#include "includes.h"

static void *_ev_talk_worker(void *arg)
{
	//int client_fd = -1;
	client_t *client = (client_t *) arg;
	if (client)
	{
		//client_fd = client->fd;
		
		evserver_client_connected_count(1);
		/*LOG_TRACE_NORMAL("new client connection, client_fd=%d, "
				"current total client count: %d\n", 
				client_fd, get_evserver_client_connected_count());*/
		
		if (client->evbase && client->in_tpool == false)
		{
			event_base_dispatch(client->evbase);
		}
		
		evserver_client_release(client);
		
		evserver_client_connected_count(-1);
		/*LOG_TRACE_NORMAL("released client [client_fd: %d], "
				"surplus total client count: %d\n", 
				client_fd, get_evserver_client_connected_count());*/
	}
	
	return NULL;
}

static void _ev_timer_callback(evutil_socket_t fd, short ev, void *arg)
{
	int timeout = 0;
	int client_fd = -1;
	
	client_t *client = (client_t *) arg;

	//LOG_TRACE_NORMAL("callback timer.\n");
	if (client)
	{
		if (client->time_count <= 0)
		{
			client->time_count = SYSTEM_SEC;
		}
		
		if (SYSTEM_SEC - client->time_count > client->timeout)
		{
			timeout = client->timeout;
			client_fd = client->fd;
			
			evserver_client_exit(client);
#if defined(SERVER_THREAD_POOL_ENABLE)
			evserver_tpool_client_handling_count(-1);
#endif
#if defined(SERVER_THREAD_POOL_ENABLE)
			LOG_TRACE_NORMAL("client timeout(%d sec) and stoped "
				"client event from event base [client_fd: %d].\n", 
					timeout, client_fd);
#else
			LOG_TRACE_NORMAL("client timeout(%d sec) and stoped "
				"event base for client [client_fd: %d].\n", 
					timeout, client_fd);
#endif
		}
	}
}

void ev_buffered_read_callback(struct bufferevent *bev, void *arg)
{
	client_t *client = (client_t *) arg;

	//LOG_TRACE_NORMAL("callback read.\n");
	
	if (bev && client 
		&& client->input_buffer 
		&& client->output_buffer)
	{
		client->time_count = SYSTEM_SEC;
		if (client_talk(client) == E_CLIENT_TALK_FINISHED)
		{
			evserver_client_exit(client);
#if defined(SERVER_THREAD_POOL_ENABLE)
			evserver_tpool_client_handling_count(-1);
#endif
		}
		else
		{
			//client->time_count = SYSTEM_SEC;
		}
	}
}

void ev_buffered_write_callback(struct bufferevent *bev, void *arg)
{
	client_t *client = (client_t *) arg;

	//LOG_TRACE_NORMAL("callback write.\n");
	
	if (bev && client 
		&& client->input_buffer 
		&& client->output_buffer)
	{
		client->time_count = SYSTEM_SEC;
		if (client_talk(client) == E_CLIENT_TALK_FINISHED)
		{
			evserver_client_exit(client);
#if defined(SERVER_THREAD_POOL_ENABLE)
			evserver_tpool_client_handling_count(-1);
#endif
		}
		else
		{
			//client->time_count = SYSTEM_SEC;
		}
	}
}

void ev_buffered_error_callback(struct bufferevent *bev, short what, void *arg)
{
	int client_fd = -1;
	client_t *client = (client_t *) arg;

	LOG_TRACE_NORMAL("callback error.\n");
	
	if (client)
	{
		client_fd = client->fd;
		evserver_client_exit(client);
#if defined(SERVER_THREAD_POOL_ENABLE)
		evserver_tpool_client_handling_count(-1);
#endif
#if defined(SERVER_THREAD_POOL_ENABLE)
		LOG_TRACE_NORMAL("stoped client event from event base [client_fd: %d].\n", 
					client_fd);
#else
		LOG_TRACE_NORMAL("stoped event base for client [client_fd: %d].\n", 
					client_fd);
#endif
	}
}

void ev_accept_callback(evutil_socket_t listen_fd, short ev, void *arg)
{
	int client_fd = -1;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	
	client_fd = accept(listen_fd, 
					(struct sockaddr *) &client_addr, 
					&client_len);
	if (client_fd < 0)
	{
		LOG_TRACE_PERROR("accept error!");
		evserver_destroy();
		return;
	}

	if (get_evserver_client_connected_count() > CLIENT_ACCEPT_MAX_COUNT)
	{
		LOG_TRACE_PERROR("connection denied. "
				"%d clients connected, max client counts: %d\n", 
					get_evserver_client_connected_count(), CLIENT_ACCEPT_MAX_COUNT);
		socket_close(client_fd);
		return;
	}

	if (evutil_make_socket_nonblocking(client_fd) < 0)
	{
		LOG_TRACE_NORMAL("failed to set client socket to non-blocking!\n");
		socket_close(client_fd);
		return;
	}

	if (evserver_distribute_job(
			client_fd, _ev_timer_callback, 
			ev_buffered_read_callback, 
			ev_buffered_write_callback, 
			ev_buffered_error_callback, 
			_ev_talk_worker) == false)
	{
		LOG_TRACE_NORMAL("evserver_distribute_job error!\n");
		socket_close(client_fd);
		return;
	}
}

