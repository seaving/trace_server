#include "includes.h"

typedef enum __client_option_step_result__
{
	/*
		执行当前step，不立即执行下一个step
	*/
	E_CLIENT_OPTION_STEP_RESULT_ONCE = 0,

	/*
		执行完当前step，立即再次执行下一个step
	*/
	E_CLIENT_OPTION_STEP_RESULT_AGAIN,

	/*
		执行完所有步骤
	*/
	E_CLIENT_OPTION_STEP_RESULT_FINISHED
} cli_option_res_t;

static cli_option_res_t _client_option_step(client_t *client)
{
	httpRequest_result_t httpRequest_result = E_HTTP_REQUEST_RESULT_SOCKET_ERROR;
	client_option_t * client_option = (client_option_t *) client->arg;
	switch (client_option->step)
	{
		case E_CLIENT_OPTION_STEP_READ_HEADER:
		{
			//LOG_TRACE_NORMAL("client_talk_recv_header\n");
			httpRequest_result = client_talk_recv_header(client);
			if (httpRequest_result == 
					E_HTTP_REQUEST_RESULT_HEADER_RECV_SUCCESS)
			{
				client_option->step = E_CLIENT_OPTION_STEP_CHECK_URL_PATH;
				return E_CLIENT_OPTION_STEP_RESULT_AGAIN;
			}
			else if (httpRequest_result == 
						E_HTTP_REQUEST_RESULT_RECV_EMPTY)
			{
				return E_CLIENT_OPTION_STEP_RESULT_ONCE;
			}
			break;
		}
		case E_CLIENT_OPTION_STEP_CHECK_URL_PATH:
		{
			//LOG_TRACE_NORMAL("client_talk_check_request_url_path\n");
			if (client_talk_check_request_url_path(client) == true)
			{
				client_option->step = E_CLIENT_OPTION_STEP_READ_CONTEXT;
				return E_CLIENT_OPTION_STEP_RESULT_AGAIN;
			}
			else
			{
				client_option->step = E_CLIENT_OPTION_STEP_SEND_ERROR;
				return E_CLIENT_OPTION_STEP_RESULT_AGAIN;
			}
			break;
		}
		case E_CLIENT_OPTION_STEP_READ_CONTEXT:
		{
			//LOG_TRACE_NORMAL("client_talk_recv_context\n");
			httpRequest_result = client_talk_recv_context(client);
			if (httpRequest_result == 
					E_HTTP_REQUEST_RESULT_CONTEXT_RECV_SUCCESS)
			{
				client_option->step = E_CLIENT_OPTION_STEP_SKYLOG_SAVE;
				return E_CLIENT_OPTION_STEP_RESULT_AGAIN;
			}
			else if (httpRequest_result == 
						E_HTTP_REQUEST_RESULT_RECV_EMPTY)
			{
				return E_CLIENT_OPTION_STEP_RESULT_ONCE;
			}
			break;
		}
		case E_CLIENT_OPTION_STEP_SKYLOG_SAVE:
		{
			//LOG_TRACE_NORMAL("client_talk_skylog_save\n");
			if (client_talk_skylog_save(client) == true)
			{
				client_option->step = E_CLIENT_OPTION_STEP_SEND_SUCCESS;
				return E_CLIENT_OPTION_STEP_RESULT_AGAIN;
			}
			else
			{
				client_option->step = E_CLIENT_OPTION_STEP_SEND_ERROR;
				return E_CLIENT_OPTION_STEP_RESULT_AGAIN;
			}
			break;
		}
		case E_CLIENT_OPTION_STEP_SEND_SUCCESS:
		{
			//LOG_TRACE_NORMAL("client_talk_response_success\n");
			client_talk_response_success(client);
			client_option->step = E_CLIENT_OPTION_STEP_FINISHED;

			//必须返回once也不能返回finished，否则消息会发不出去
			return E_CLIENT_OPTION_STEP_RESULT_ONCE;
		}
		case E_CLIENT_OPTION_STEP_SEND_ERROR:
		{
			//LOG_TRACE_NORMAL("client_talk_response_failt\n");
			client_talk_response_failt(client);
			client_option->step = E_CLIENT_OPTION_STEP_FINISHED;

			//必须返回once也不能返回finished，否则消息会发不出去
			return E_CLIENT_OPTION_STEP_RESULT_ONCE;
		}
		case E_CLIENT_OPTION_STEP_FINISHED:
		default:
		{
			//结束所有流程
			return E_CLIENT_OPTION_STEP_RESULT_FINISHED;
		}
	}

	//默认结束所有流程
	return E_CLIENT_OPTION_STEP_RESULT_FINISHED;
}

client_talk_result_t client_talk(client_t *client)
{
	cli_option_res_t ret = E_CLIENT_OPTION_STEP_RESULT_FINISHED;
	if (client_option_arg_init(client) == false)
	{
		return E_CLIENT_TALK_FINISHED;
	}
	
	for ( ; ; )
	{
		ret = _client_option_step(client);
		if (ret == E_CLIENT_OPTION_STEP_RESULT_AGAIN)
		{
			continue;
		}
		if (ret == E_CLIENT_OPTION_STEP_RESULT_ONCE)
		{
			return E_CLIENT_TALK_CONTINUE;
		}
		if (ret == E_CLIENT_OPTION_STEP_RESULT_FINISHED)
		{
			return E_CLIENT_TALK_FINISHED;
		}

		break;
	}

	return E_CLIENT_TALK_FINISHED;
}

