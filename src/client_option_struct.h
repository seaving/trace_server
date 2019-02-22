#ifndef __CLIENT_OPTION_STRUCT_H__
#define __CLIENT_OPTION_STRUCT_H__

#include "http_request.h"
#include "event_server.h"

typedef enum __client_option_step__
{
	E_CLIENT_OPTION_STEP_READ_HEADER = 0,
	E_CLIENT_OPTION_STEP_CHECK_URL_PATH,
	E_CLIENT_OPTION_STEP_SKYLOG_SAVE,
	E_CLIENT_OPTION_STEP_READ_CONTEXT,
	E_CLIENT_OPTION_STEP_SEND_SUCCESS,
	E_CLIENT_OPTION_STEP_SEND_ERROR,
	E_CLIENT_OPTION_STEP_FINISHED
} client_option_step_t;

typedef struct __client_option__
{
	skylog_t skylog;
	httpRequest_t httpRequest;
	client_option_step_t step;
} client_option_t;

void client_option_free(void *cop);
bool client_option_arg_init(client_t *client);

#endif

