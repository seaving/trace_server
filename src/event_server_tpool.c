#include "includes.h"

static pthread_spinlock_t _working_count_lock;
static pthread_spinlock_t _client_wait_count_lock;
static pthread_spinlock_t _client_handling_count_lock;

static evserver_tpool_t *_event_server_tpool = NULL;

static void _evserver_tpool_free(evserver_tpool_t *tpool);

static evserver_tpool_t *_evserver_tpool_new(
			int worker_size, unsigned int worker_queue_size)
{
	int i = 0;
	evserver_tpool_t *tpool = NULL;
	
	if (worker_size > 0 && worker_queue_size > 0)
	{
		tpool = calloc(sizeof(evserver_tpool_t), 1);
		if (tpool == NULL)
		{
			LOG_TRACE_PERROR("calloc error!");
			return NULL;
		}

		tpool->workers = calloc(sizeof(evserver_worker_t), worker_size);
		if (tpool->workers == NULL)
		{
			LOG_TRACE_PERROR("calloc error!");
			_evserver_tpool_free(tpool);
			return NULL;
		}

		tpool->worker_size = worker_size;
		for (i = 0; i < worker_size; i ++)
		{
			if (evserver_worker_init(
					&tpool->workers[i], 
					worker_queue_size) == false)
			{
				LOG_TRACE_NORMAL("evserver_worker_init error!\n");
				_evserver_tpool_free(tpool);
				return NULL;
			}
		}

		pthread_mutex_init(&tpool->lock, NULL);
	}

	return tpool;
}

static void _evserver_tpool_free(evserver_tpool_t *tpool)
{
	int i = 0;
	evserver_worker_t *worker = NULL;
	
	if (tpool)
	{
		if (tpool->workers)
		{
			worker = tpool->workers;
			for (i = 0; i < tpool->worker_size; i ++)
			{
				evserver_worker_free(worker + i);
			}

			free(tpool->workers);
		}
		
		pthread_mutex_destroy(&tpool->lock);
		memset(tpool, 0, sizeof(evserver_tpool_t));
		free(tpool);
	}
}

bool evserver_tpool_create(int worker_size, unsigned int worker_queue_size)
{
	int i = 0;

	pthread_spin_init(&_working_count_lock, PTHREAD_PROCESS_PRIVATE);
	pthread_spin_init(&_client_wait_count_lock, PTHREAD_PROCESS_PRIVATE);
	pthread_spin_init(&_client_handling_count_lock, PTHREAD_PROCESS_PRIVATE);
	
	_event_server_tpool = _evserver_tpool_new(worker_size, worker_queue_size);
	if (_event_server_tpool == NULL)
	{
		return false;
	}

	for (i = 0; i < worker_size; i ++)
	{
		_event_server_tpool->workers[i].tpool = _event_server_tpool;
		_event_server_tpool->workers[i].tpool_index = i;
		tpool_add_work(evserver_worker_factory, 
					&_event_server_tpool->workers[i]);
	}

	return true;
}

void evserver_tpool_destroy()
{
	pthread_spin_destroy(&_working_count_lock);
	pthread_spin_destroy(&_client_wait_count_lock);
	pthread_spin_destroy(&_client_handling_count_lock);

	_evserver_tpool_free(_event_server_tpool);
}

int evserver_tpool_get_working_count()
{
	return _event_server_tpool ? _event_server_tpool->working_count : 0;
}

int evserver_tpool_get_client_wait_count()
{
	return _event_server_tpool ? _event_server_tpool->client_wait_count : 0;
}

int evserver_tpool_get_client_handling_count()
{
	return _event_server_tpool ? _event_server_tpool->client_handling_count : 0;
}

void evserver_tpool_working_count(int c)
{
	if (_event_server_tpool)
	{
		pthread_spin_lock(&_working_count_lock);
		_event_server_tpool->working_count += c;
		pthread_spin_unlock(&_working_count_lock);
	}
}

void evserver_tpool_client_wait_count(int c)
{
	if (_event_server_tpool)
	{
		pthread_spin_lock(&_client_wait_count_lock);
		_event_server_tpool->client_wait_count += c;
		pthread_spin_unlock(&_client_wait_count_lock);
	}
}

void evserver_tpool_client_handling_count(int c)
{
	if (_event_server_tpool)
	{
		pthread_spin_lock(&_client_handling_count_lock);
		_event_server_tpool->client_handling_count += c;
		pthread_spin_unlock(&_client_handling_count_lock);
	}
}

bool evserver_tpool_add_job(
		int client_fd, 
		event_callback_fn timer_callback, 
		bufferevent_data_cb read_callback, 
		bufferevent_data_cb write_callback,
	    bufferevent_event_cb error_callback)
{
	int worker_index = 0;
	if (_event_server_tpool 
		&& _event_server_tpool->workers
		&& client_fd > 0)
	{
		pthread_mutex_lock(&_event_server_tpool->lock);
		
		worker_index = _event_server_tpool->current_worker 
				% _event_server_tpool->worker_size;
		_event_server_tpool->current_worker ++;
		//LOG_TRACE_NORMAL(">>>> worker_index: %d\n", worker_index);
		if (evserver_worker_add_job(
				&_event_server_tpool->workers[worker_index], 
				client_fd, timer_callback, 
				read_callback, write_callback, 
				error_callback) == false)
		{
			pthread_mutex_unlock(&_event_server_tpool->lock);
			return false;
		}
		pthread_mutex_unlock(&_event_server_tpool->lock);
		return true;
	}
	
	return false;
}

