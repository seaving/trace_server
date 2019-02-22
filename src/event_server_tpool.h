#ifndef __EVENT_SERVER_TPOOL_H__
#define __EVENT_SERVER_TPOOL_H__

#include "common.h"
#include "event_server_worker.h"

typedef struct __event_server_tpool__
{
	evserver_worker_t *workers;
	int worker_size;
	int working_count;
	unsigned int current_worker;
	
	int client_wait_count;		//等待处理的数量
	int client_handling_count;	//正在处理的数量
	pthread_mutex_t lock;
} evserver_tpool_t;


int evserver_tpool_get_working_count();
int evserver_tpool_get_client_wait_count();
int evserver_tpool_get_client_handling_count();
void evserver_tpool_working_count(int c);
void evserver_tpool_client_wait_count(int c);
void evserver_tpool_client_handling_count(int c);

bool evserver_tpool_create(
		int worker_size, unsigned int worker_queue_size);
void evserver_tpool_destroy();
bool evserver_tpool_add_job(
		int client_fd, 
		event_callback_fn timer_callback, 
		bufferevent_data_cb read_callback, 
		bufferevent_data_cb write_callback,
	    bufferevent_event_cb error_callback);

#endif

