#ifndef __EVENT_SERVER_WORKER_H__
#define __EVENT_SERVER_WORKER_H__

#include "common.h"
#include "queue.h"

typedef void (*worker_callback_fn)(void *);

typedef struct __event_server_tpool_worker_job__
{
	int connect_fd;
	bool processing;
	event_callback_fn timer_callback;
	bufferevent_data_cb read_callback;
	bufferevent_data_cb write_callback;
	bufferevent_event_cb error_callback;
	worker_callback_fn worker_callback;
} evserver_worker_job_t;

typedef struct __event_server_tpool_worker__
{
	pthread_t tid;
	int read_fd;
    int write_fd;
    int connect_fd;
	struct event_base *evbase;
	struct event event;
	int tpool_index;
	void *tpool;
	
	queue_t *queue;

	bool create_by_malloc;

	pthread_mutex_t lock;
} evserver_worker_t;

bool evserver_worker_init(evserver_worker_t *worker, unsigned int queue_size);

evserver_worker_t *evserver_worker_new(unsigned int fifo_size);
void evserver_worker_free(evserver_worker_t *worker);

void *evserver_worker_factory(void *arg);
bool evserver_worker_add_job(
		evserver_worker_t *worker, 
		int client_fd, 
		event_callback_fn timer_callback, 
		bufferevent_data_cb read_callback, 
		bufferevent_data_cb write_callback, 
	    bufferevent_event_cb error_callback);

#endif

