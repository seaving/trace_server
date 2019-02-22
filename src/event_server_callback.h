#ifndef __EVENT_SERVER_CALLBACK_H__
#define __EVENT_SERVER_CALLBACK_H__

#include "common.h"


void ev_accept_callback(evutil_socket_t listen_fd, short ev, void *arg);
void ev_buffered_error_callback(struct bufferevent *bev, short what, void *arg);
void ev_buffered_write_callback(struct bufferevent *bev, void *arg);
void ev_buffered_read_callback(struct bufferevent *bev, void *arg);

#endif


