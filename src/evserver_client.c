#include "includes.h"

static inline void _evserver_client_free(client_t *client)
{
	if (client)
	{
		event_del(&client->event_timer);
		
		if (client->free_callback)
		{
			client->free_callback(client->arg);
			client->free_callback = NULL;
		}
		if (client->input_buffer)
		{
			evbuffer_free(client->input_buffer);
			client->input_buffer = NULL;
		}
		if (client->output_buffer)
		{
			evbuffer_free(client->output_buffer);
			client->output_buffer = NULL;
		}
		if (client->buf_ev)
		{
			bufferevent_disable(client->buf_ev, EV_READ);
			bufferevent_free(client->buf_ev);
			client->buf_ev = NULL;
		}
		if (client->evbase && client->in_tpool == false)
		{
			event_base_free(client->evbase);
			client->evbase = NULL;
		}

		free(client);
	}
}

static inline void _evserver_client_base_exit(client_t *client)
{
	if (client && client->evbase 
		&& client->in_tpool == false)
	{
		event_base_loopexit(client->evbase, NULL);
	}
}

void evserver_client_release(client_t *client)
{
	int client_fd = -1;
	if (client)
	{
		client_fd = client->fd;
		_evserver_client_free(client);
		socket_close(client_fd);
	}
}

void evserver_client_exit(client_t *client)
{
	if (client && client->in_tpool)
	{
		evserver_client_release(client);
	}
	else
	{
		_evserver_client_base_exit(client);
	}
}

client_t *evserver_client_new(
	struct event_base *evbase, 
	bool in_tpool, 
	evserver_worker_t *worker, 
	int client_fd, 
	event_callback_fn timer_callback, 
	bufferevent_data_cb read_callback, 
	bufferevent_data_cb write_callback,
    bufferevent_event_cb error_callback)
{
	struct timeval tv;
	client_t *client = NULL;

	//in_tpool 标记client由worker处理
	//如果此时worker为NULL，则异常
	if (in_tpool && worker == NULL)
	{
		return NULL;
	}
	
	if (client_fd > 0 && evbase)
	{
		client = calloc(sizeof(client_t), 1);
		if (client == NULL)
		{
			LOG_TRACE_PERROR("calloc error!");
			return NULL;
		}

		client->in_tpool = in_tpool;
		client->worker = in_tpool ? worker : NULL;
		client->fd = client_fd;
		if ((client->input_buffer = evbuffer_new()) == NULL)
		{
	        LOG_TRACE_NORMAL("client input buffer allocation failed!\n");
	        _evserver_client_free(client);
	        return NULL;
	    }

	    if ((client->output_buffer = evbuffer_new()) == NULL)
	    {
	        LOG_TRACE_NORMAL("client output buffer allocation failed!\n");
	        _evserver_client_free(client);
	        return NULL;
    	}

		client->timeout = CLIENT_TIMEOUT_SEC;
		client->time_count = SYSTEM_SEC;

		if (timer_callback 
			&& event_assign(&client->event_timer, 
				evbase, client->fd, 
				EV_PERSIST, 
				timer_callback, client) < 0)
		{
			LOG_TRACE_NORMAL("event_assign error!\n");
			_evserver_client_free(client);
			return NULL;
		}

		evutil_timerclear(&tv);
		tv.tv_sec = 5;
		if (event_add(&client->event_timer, &tv) < 0)
		{
	        LOG_TRACE_NORMAL("event_add failed!\n");
	        _evserver_client_free(client);
	        return NULL;
		}

#if 1
		client->buf_ev = bufferevent_socket_new(
				evbase, client_fd, BEV_OPT_CLOSE_ON_FREE);
		if (client->buf_ev == NULL) {
			LOG_TRACE_NORMAL("client bufferevent creation failed!\n");
			_evserver_client_free(client);
			return NULL;
		}
		
		bufferevent_setcb(client->buf_ev, 
					read_callback, 
					write_callback,
					error_callback, client);
		
		if (bufferevent_enable(client->buf_ev, EV_READ) < 0)
		{
			LOG_TRACE_NORMAL("enable bufferevent failed!\n");
			_evserver_client_free(client);
			return NULL;
		}
#endif
		client->evbase = evbase;
	}

	return client;
}

bool evserver_send_client(client_t *client, char *data, int data_len)
{
	if (data
		&& data_len > 0 
		&& client 
		&& client->buf_ev
		&& client->output_buffer)
	{
		if (evbuffer_add(client->output_buffer, data, data_len) < 0)
		{
			LOG_TRACE_NORMAL("evbuffer_add error!\n");
			return false;
		}
		if (bufferevent_write_buffer(
				client->buf_ev, client->output_buffer) < 0)
		{
			LOG_TRACE_NORMAL("bufferevent_write_buffer error!\n");
			return false;
		}

		return true;
	}

	return false;
}

int evserver_recv_client(client_t *client, char *buf, int buf_size)
{
	int len = 0;
	
	if (buf 
		&& buf_size > 0 
		&& client 
		&& client->buf_ev 
		&& client->input_buffer)
	{
		bufferevent_read_buffer(
				client->buf_ev, client->input_buffer);
		len = evbuffer_get_length(client->input_buffer);
		if (len > 0)
		{
			len = evbuffer_remove(client->input_buffer, 
					buf, min(len, buf_size));
		}
	}

	return len;
}

