#ifndef __EVSERVER_CLIENT_H__
#define __EVSERVER_CLIENT_H__

#include "event_server_worker.h"

typedef void (*free_arg_callback)(void *);

typedef struct __client__ {
    int fd;
    unsigned int time_count;
    unsigned int timeout;
    struct event_base *evbase;
    struct event	event_timer;
	struct bufferevent *buf_ev;
	struct evbuffer *input_buffer;
    struct evbuffer *output_buffer;
    free_arg_callback free_callback;
    bool in_tpool;
    evserver_worker_t *worker;
    void *arg;
} client_t;


client_t *evserver_client_new(
	struct event_base *evbase, 
	bool in_tpool, 
	evserver_worker_t *worker, 
	int client_fd, 
	event_callback_fn timer_callback, 
	bufferevent_data_cb read_callback, 
	bufferevent_data_cb write_callback,
    bufferevent_event_cb error_callback);

void evserver_client_connected_count(int c);
int evserver_get_client_connected_count();

void evserver_client_release(client_t *client);

bool evserver_send_client(client_t *client, char *data, int data_len);
int evserver_recv_client(client_t *client, char *buf, int buf_size);
void evserver_client_exit(client_t *client);

#endif

