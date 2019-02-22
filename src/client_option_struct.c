#include "includes.h"

void client_option_free(void *cop)
{
	client_option_t *client_option = (client_option_t *) cop;
	if (client_option)
	{
		skylog_save_finished(&client_option->skylog);
		httpRequest_context_free(&client_option->httpRequest);
		memset(client_option, 0, sizeof(client_option_t));
		free(client_option);
	}
}

bool client_option_arg_init(client_t *client)
{
	client_option_t *client_option = NULL;
	
	if (client)
	{
		if (client->arg == NULL)
		{
			client->arg = calloc(sizeof(client_option_t), 1);
			if (client->arg == NULL)
			{
				LOG_TRACE_PERROR("calloc error!");
				return false;
			}

			client->free_callback = client_option_free;
			client_option = (client_option_t *) client->arg;
			client_option->step = E_CLIENT_OPTION_STEP_READ_HEADER;
		}
		
		return true;
	}

	return false;
}

