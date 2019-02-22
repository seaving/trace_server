

#ifndef __CLIENT_TALK_OPTION_H__
#define __CLIENT_TALK_OPTION_H__

#include "http_request.h"
#include "event_server.h"

#define CLIENT_TALK_TOKEN_HEADER	"------WebKitFormBoundaryapMKTQABBP6vWIo0\r\n"
#define CLIENT_TALK_TOKEN_END		"\r\n------WebKitFormBoundaryapMKTQABBP6vWIo0--\r\n"

#define CLIENT_TALK_RESPONSE_OK		"HTTP/1.1 200 OK \r\n" \
									"Content-length:0 \r\n" \
									"\r\n"

#define CLIENT_TALK_RESPONSE_ERROR	"HTTP/1.1 400 Bad Request \r\n" \
									"Content-length:0 \r\n" \
									"\r\n"

#define client_talk_response_success(client) \
	evserver_send_client(client, CLIENT_TALK_RESPONSE_OK, \
					strlen(CLIENT_TALK_RESPONSE_OK))

#define client_talk_response_failt(client) \
	client_talk_error_debug(client); \
	evserver_send_client(client, CLIENT_TALK_RESPONSE_ERROR, \
					strlen(CLIENT_TALK_RESPONSE_ERROR))

void client_talk_error_debug(client_t *client);

httpRequest_result_t client_talk_recv_header(client_t *client);
bool client_talk_check_request_url_path(client_t *client);
httpRequest_result_t client_talk_recv_context(client_t *client);
bool client_talk_skylog_save(client_t *client);

#endif

