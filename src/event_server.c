#include "includes.h"

static pthread_spinlock_t _client_connected_count_lock;
static int _evserver_client_connected_count = 0;

typedef void (*ev_callback_f)(evutil_socket_t, short, void *);

typedef struct __server__
{
	int listen_fd;
	int listen_port;
	int listen_count;
	struct event event_accept;
	struct event_base *evbase_server;
	bool server_ok;
} server_t;

static server_t _server = {
	.listen_fd = -1,
	.evbase_server = NULL,
	.server_ok = false,
	.listen_port = SERVER_BIND_PORT,
	.listen_count = SERVER_LISTEN_QUEUE_SIZE,
};

static inline void _evserver_destroy();

static inline bool _evserver_get_server_status()
{
	return _server.server_ok;
}

static int _evserver_socket_create(int server_port, int listen_count)
{
	int listen_fd = socket_listen(NULL, server_port, listen_count);
	if (listen_fd > 0)
	{
		if (evutil_make_listen_socket_reuseable(listen_fd) < 0)
		{
			LOG_TRACE_NORMAL("evutil_make_listen_socket_reuseable error!\n");
			socket_close(listen_fd);
			return -1;
		}

	    if (evutil_make_socket_nonblocking(listen_fd) < 0)
	    {
			LOG_TRACE_NORMAL("evutil_make_socket_nonblocking error!\n");
			socket_close(listen_fd);
			return -1;
	    }
	}
	
    return listen_fd;
}

static bool _evserver_create(event_callback_fn ev_callback)
{
	_server.evbase_server = event_base_new();
	if (_server.evbase_server == NULL)
	{
		LOG_TRACE_NORMAL("event_base_new error!\n");
		return false;
	}

	_server.listen_fd = _evserver_socket_create(
		_server.listen_port, _server.listen_count);
	if (_server.listen_fd < 0)
	{
		_evserver_destroy();
		return false;
	}
	
	if (event_assign(&_server.event_accept, 
			_server.evbase_server, 
			_server.listen_fd, 
			EV_READ | EV_PERSIST, 
			ev_callback, &_server) < 0)
	{
		LOG_TRACE_NORMAL("event_assign error!\n");
		socket_close(_server.listen_fd);
		return false;
	}
	
	if (event_add(&_server.event_accept, NULL) < 0)
	{
		LOG_TRACE_NORMAL("event_add error!\n");
		socket_close(_server.listen_fd);
		return false;
	}

	return true;
}

static inline void _evserver_destroy()
{	
	if (_server.evbase_server)
	{
		event_del(&_server.event_accept);
		event_base_loopexit(_server.evbase_server, NULL);
		event_base_free(_server.evbase_server);
		shutdown(_server.listen_fd, SHUT_RDWR);
		socket_close(_server.listen_fd);
		_server.listen_fd = -1;
		_server.evbase_server = NULL;
		_server.server_ok = false;
	}
}

static inline void *_evserver_factory(void *arg)
{
	pthread_detach(pthread_self());
	
	if (_server.evbase_server)
	{
		_server.server_ok = true;
		event_base_dispatch(_server.evbase_server);
		_server.server_ok = false;
	}

	return NULL;
}

static inline bool _evserver_job_process(
		job_process_cb job_process_callback, client_t *client)
{
	if (job_process_callback == NULL 
		|| client == NULL)
	{
		return false;
	}
	
	if (tpool_add_work(job_process_callback, client) < 0)
	{
		LOG_TRACE_NORMAL("tpool_add_work failt!\n");
		return false;
	}

	return true;
}

#if !defined(SERVER_THREAD_POOL_ENABLE)
static bool _evserver_distribute_job_in_thread(
				int client_fd, 
				event_callback_fn timer_callback, 
				bufferevent_data_cb read_callback, 
				bufferevent_data_cb write_callback,
			    bufferevent_event_cb error_callback,
			    job_process_cb job_process_callback)
{
	client_t *client = NULL;
	struct event_base *evbase = NULL;

	if (client_fd > 0 && job_process_callback)
	{
		evbase = event_base_new();
		if (evbase == NULL)
		{
			LOG_TRACE_NORMAL("event_base_new error!\n");
			return false;
		}

		/*
			in_tpool设置为false的情况下:
			evserver_client_new成功，evbase的free将由client处理
			evserver_client_new失败，evbase则需要free
		*/
		client = evserver_client_new(evbase, false, NULL, 
					client_fd, timer_callback, read_callback, 
					write_callback, error_callback);
		if (client == NULL)
		{
			LOG_TRACE_NORMAL("evserver_client_new error!\n");
			event_base_free(evbase);
			return false;
		}

		if (_evserver_job_process(
				job_process_callback, client) == false)
		{
			evserver_client_release(client);
			return false;
		}

		return true;
	}

	return false;
}
#endif

bool evserver_create()
{
	pthread_t tid;

	pthread_spin_init(&_client_connected_count_lock, 
			PTHREAD_PROCESS_PRIVATE);
	
	if (_evserver_create(ev_accept_callback) == false)
	{
		evserver_destroy();
		return false;
	}

	if (pthread_create(&tid, NULL, 
			_evserver_factory, NULL) != 0)
	{
		LOG_TRACE_PERROR("pthread_create error!");
		return false;
	}

	return true;
}

void evserver_destroy()
{	
	pthread_spin_destroy(&_client_connected_count_lock);
	_evserver_destroy();
}

bool evserver_status_ok()
{
	return _evserver_get_server_status();
}

bool evserver_distribute_job(
	int client_fd, 
	event_callback_fn timer_callback, 
	bufferevent_data_cb read_callback, 
	bufferevent_data_cb write_callback,
    bufferevent_event_cb error_callback,
    job_process_cb job_process_callback)
{
#if defined(SERVER_THREAD_POOL_ENABLE)
	return evserver_tpool_add_job(
				client_fd, timer_callback, 
				read_callback, write_callback, 
				error_callback);
#else
	return _evserver_distribute_job_in_thread(
				client_fd, timer_callback, 
				read_callback, write_callback, 
				error_callback, job_process_callback);
#endif
}

void evserver_client_connected_count(int c)
{
	pthread_spin_lock(&_client_connected_count_lock);
	_evserver_client_connected_count += c;
	pthread_spin_unlock(&_client_connected_count_lock);
}

int get_evserver_client_connected_count()
{
	return _evserver_client_connected_count;
}

