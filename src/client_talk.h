#ifndef __CLIENT_TALK_H__
#define __CLIENT_TALK_H__

#include "event_server.h"

typedef enum __client_talk_result__
{
	E_CLIENT_TALK_CONTINUE,
	E_CLIENT_TALK_FINISHED
} client_talk_result_t;

client_talk_result_t client_talk(client_t *client);


#endif

