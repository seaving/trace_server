#include "includes.h"

static inline bool _evserver_worker_job_init(
		evserver_worker_job_t *job, 
		int connect_fd, 
		event_callback_fn timer_callback, 
		bufferevent_data_cb read_callback, 
		bufferevent_data_cb write_callback, 
		bufferevent_event_cb error_callback)
{
	if (job && connect_fd > 0)
	{
		memset(job, 0, sizeof(evserver_worker_job_t));
		
		job->connect_fd = connect_fd;
		job->read_callback = read_callback;
		job->write_callback = write_callback;
		job->timer_callback = timer_callback;
		job->error_callback = error_callback;

		return true;
	}

	return false;
}

static void _evserver_worker_add_event(evserver_worker_t *worker)
{
	int i = 0;
	item_t *item = NULL;
	evserver_worker_job_t *job;
	client_t *client = NULL;

	for (i = 0; worker && i < 1; i ++)
	{
		item = NULL;
		job = NULL;
		client = NULL;

		item = queue_pop(worker->queue);
		job = (evserver_worker_job_t *) item_get_data(worker->queue, item);
		if (job == NULL)
		{
			//LOG_TRACE_NORMAL("queue_pop error!\n");
			item_free(worker->queue, item);
			return;
		}
		job->processing = false;
		
		evserver_tpool_client_wait_count(-1);

		//LOG_TRACE_NORMAL("------------------------\n");
		//LOG_TRACE_NORMAL("connect_fd: %d\n", job.connect_fd);
		//LOG_TRACE_NORMAL("timer_callback: %u\n", (unsigned int) job.timer_callback);
		//LOG_TRACE_NORMAL("read_callback: %u\n", (unsigned int) job.read_callback);
		//LOG_TRACE_NORMAL("write_callback: %u\n", (unsigned int) job.write_callback);
		//LOG_TRACE_NORMAL("error_callback: %u\n", (unsigned int) job.error_callback);
		//LOG_TRACE_NORMAL("worker_callback: %u\n", (unsigned int) job.worker_callback);
		//LOG_TRACE_NORMAL("------------------------\n");

		if (job->connect_fd > 0)
		{
			/*
				in_tpool设置为false的情况下:
				evserver_client_new成功，evbase的free将由client处理
				evserver_client_new失败，evbase则需要free
			*/
			client = evserver_client_new(worker->evbase, true, worker, 
						job->connect_fd, job->timer_callback, 
						job->read_callback, job->write_callback, 
						job->error_callback);
			if (client == NULL)
			{
				//LOG_TRACE_NORMAL("evserver_client_new error!\n");
				//socket_close(job->connect_fd);
			}
			else
			{
				job->processing = true;
				evserver_tpool_client_handling_count(1);
			}
		}

		item_free(worker->queue, item);
	}
}

static void _evserver_worker_read_callback(
				evutil_socket_t fd, short ev, void *arg)
{
	char buf[2] = {0};
	evserver_worker_t *worker = (evserver_worker_t *) arg;
	if (worker == NULL)
	{
		return;
	}

	if (fd != worker->read_fd)
	{
		LOG_TRACE_NORMAL("OH MY GOD! The fd(%d) not my read fd(%d)! exit!!!!\n", 
					fd, worker->read_fd);
		exit(-1);
	}

	/*LOG_TRACE_NORMAL("[worker_index=%d] new notification processing ...\n", 
			worker->tpool_index);*/
	
	recv(worker->read_fd, buf, 1, 0);
	_evserver_worker_add_event(worker);
}

static inline void _evserver_worker_job_free(void *data)
{
	evserver_worker_job_t *job = (evserver_worker_job_t *) data;

	if (job)
	{
		if (job->processing == false)
		{
			socket_close(job->connect_fd);
		}
		
		free(job);
	}
}

bool evserver_worker_init(evserver_worker_t *worker, unsigned int queue_size)
{
	int fd[2] = {-1, -1};
	if (worker == NULL)
	{
		return false;
	}

	memset(worker, 0, sizeof(evserver_worker_t));
	
	worker->queue = queue_create();
	if (worker->queue == NULL)
	{
		LOG_TRACE_NORMAL("queue_create error!\n");
		return false;
	}
	
	queue_set_depth(worker->queue, queue_size);
	queue_set_hook(worker->queue, NULL, _evserver_worker_job_free);
	
	pthread_mutex_init(&worker->lock, NULL);
	
	if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1)
	{
		LOG_TRACE_PERROR("evutil_socketpair error!");
		return false;
	}

	evutil_make_socket_closeonexec(fd[0]);
	evutil_make_socket_closeonexec(fd[1]);

	evutil_make_socket_nonblocking(fd[0]);
	evutil_make_socket_nonblocking(fd[1]);
	
	worker->write_fd = fd[0];
	worker->read_fd = fd[1];
	worker->evbase = event_base_new();
	if (worker->evbase == NULL)
	{
		LOG_TRACE_NORMAL("event_base_new error!\n");
		return false;
	}

	if (event_assign(&worker->event, 
			worker->evbase, worker->read_fd,
			EV_READ | EV_PERSIST, 
			_evserver_worker_read_callback, worker) < 0)
	{
		LOG_TRACE_NORMAL("event_assign error!\n");
		return false;
	}

	if (event_add(&worker->event, NULL) < 0)
	{
		LOG_TRACE_NORMAL("event_add error!\n");
		return false;
	}
	
	return true;
}

evserver_worker_t *evserver_worker_new(unsigned int fifo_size)
{
	evserver_worker_t *worker = calloc(sizeof(evserver_worker_t), 1);
	if (worker == NULL)
	{
		LOG_TRACE_PERROR("calloc error!");
		return NULL;
	}

	if (evserver_worker_init(worker, fifo_size) == false)
	{
		evserver_worker_free(worker);
		return NULL;
	}

	worker->create_by_malloc = true;
	
	return worker;
}

void evserver_worker_free(evserver_worker_t *worker)
{
	if (worker)
	{
		if (worker->evbase)
		{
			event_del(&worker->event);
			event_base_loopexit(worker->evbase, NULL);
			event_base_free(worker->evbase);
			worker->evbase = NULL;
		}

		if (worker->read_fd)
			socket_close(worker->read_fd);
		if (worker->write_fd)
			socket_close(worker->write_fd);

		if (worker->queue)
			queue_destroy(worker->queue);
		
		pthread_mutex_destroy(&worker->lock);

		memset(worker, 0, sizeof(evserver_worker_t));

		if (worker->create_by_malloc)
		{
			free(worker);
		}
	}
}

void *evserver_worker_factory(void *arg)
{
	int tpool_index = 0;
	evserver_tpool_t *tpool = NULL;
	evserver_worker_t *worker = (evserver_worker_t *) arg;
	if (worker == NULL
		|| worker->evbase == NULL)
	{
		return NULL;
	}

	tpool = (evserver_tpool_t *) worker->tpool;
	if (tpool == NULL)
	{
		return NULL;
	}
	
	worker->tid = pthread_self();

	tpool_index = worker->tpool_index;
	/*LOG_TRACE_NORMAL("server tpool worker"
			" [tpool_index=%d] start ...\n", tpool_index);*/

	evserver_tpool_working_count(1);

	event_base_dispatch(worker->evbase);
	event_del(&worker->event);
	event_base_free(worker->evbase);
	worker->evbase = NULL;

	evserver_tpool_working_count(-1);
	
	LOG_TRACE_NORMAL("server tpool worker"
			" [tpool_index=%d] exit ...\n", tpool_index);
	
	return NULL;
}

bool evserver_worker_add_job(
			evserver_worker_t *worker, 
			int client_fd, 
			event_callback_fn timer_callback, 
			bufferevent_data_cb read_callback, 
			bufferevent_data_cb write_callback,
		    bufferevent_event_cb error_callback)
{
	evserver_worker_job_t job;
	
	if (worker == NULL 
		|| client_fd < 0
		|| worker->write_fd <= 0
		|| worker->queue == NULL)
	{
		return false;
	}
	
	pthread_mutex_lock(&worker->lock);

	_evserver_worker_job_init(
			&job, client_fd, timer_callback, 
			read_callback, write_callback, 
			error_callback);

	item_t *item = item_alloc(worker->queue, 
				&job, sizeof(evserver_worker_job_t));
	if (item == NULL)
	{
		LOG_TRACE_NORMAL("item_alloc error!\n");
		pthread_mutex_unlock(&worker->lock);
		return false;
	}
	
	if (queue_push(worker->queue, item) < 0)
	{
		LOG_TRACE_NORMAL("queue_push error!\n");
		item_free(worker->queue, item);
		pthread_mutex_unlock(&worker->lock);
		return false;
	}

	evserver_tpool_client_wait_count(1);

	send(worker->write_fd, " ", 1, 0);

	pthread_mutex_unlock(&worker->lock);
	return true;
}

