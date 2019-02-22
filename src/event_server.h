#ifndef __EVENT_SERVER_H__
#define __EVENT_SERVER_H__

#include "common.h"

typedef void *(*job_process_cb)(void *);

bool evserver_create();

void evserver_destroy();

bool evserver_status_ok();

bool evserver_distribute_job(
	int client_fd, 
	event_callback_fn timer_callback, 
	bufferevent_data_cb read_callback, 
	bufferevent_data_cb write_callback,
    bufferevent_event_cb error_callback,
    job_process_cb job_process_callback);

void evserver_client_connected_count(int c);
int get_evserver_client_connected_count();

#endif

