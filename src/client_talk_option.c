#include "includes.h"

void client_talk_error_debug(client_t *client)
{
	int len = 0;
	int offset = 0;
	char buf[1024] = {0};
	char str_info[4048] = {0};
	client_option_t *client_option = NULL;

	if (client)
	{
		client_option = (client_option_t *) client->arg;
		len = snprintf(str_info + offset, 
					sizeof(str_info) - offset, 
					"-------------error info-----------\n");
		offset += len;
		
		len = snprintf(str_info + offset, 
					sizeof(str_info) - offset, 
					"http header: %s", 
					httpRequest_get_header(
						&client_option->httpRequest));
		offset += len;

		memset(buf, 0, sizeof(buf));
		httpRequest_get_url_path(
			&client_option->httpRequest, 
			buf, sizeof(buf) - 1);
		len = snprintf(str_info + offset, 
					sizeof(str_info) - offset, 
					"url path: %s\n", buf);
		offset += len;

		len = snprintf(str_info + offset, 
					sizeof(str_info) - offset, 
					"content-length: %d\n", 
					httpRequest_get_context_len(
						&client_option->httpRequest));
		offset += len;

		len = snprintf(str_info + offset, 
					sizeof(str_info) - offset, 
					"----------------------------------\n");
		offset += len;
		LOG_TRACE_NORMAL("\n%s\n", str_info);
	}
}

/*
	接收http请求头部
*/
httpRequest_result_t client_talk_recv_header(client_t *client)
{
	httpRequest_result_t httpRequest_result;
	client_option_t *client_option = NULL;
	client_option = (client_option_t *) client->arg;

	httpRequest_result = httpRequest_recv_header(
					client, &client_option->httpRequest);
	if (httpRequest_result != E_HTTP_REQUEST_RESULT_HEADER_RECV_SUCCESS)
	{
		return httpRequest_result;
	}

	//LOG_TRACE_NORMAL(">>>>>>>> %s", 
		//httpRequest_get_header(&client_option->httpRequest));
	return httpRequest_result;
}

/*
	校验客户端请求过来的url 路径是否合法
*/
bool client_talk_check_request_url_path(client_t *client)
{
	char buf[255] = {0};
	client_option_t *client_option = NULL;
	client_option = (client_option_t *) client->arg;

	httpRequest_get_url_path(&client_option->httpRequest, buf, sizeof(buf) - 1);
	//LOG_TRACE_NORMAL("url path: %s\n", buf);
	return strcmp(buf, SKYLOG_REQUEST_URL_PATH) == 0;
}

/*
	接收正文
*/
httpRequest_result_t client_talk_recv_context(client_t *client)
{
	client_option_t *client_option = NULL;
	client_option = (client_option_t *) client->arg;
	return httpRequest_recv_context(client, &client_option->httpRequest);
}

/*
	把接收到的正文保存到文件
*/
bool client_talk_skylog_save(client_t *client)
{
	int file_len = 0;
	int tail_len = 0;
	char *p = NULL;
	const char gzip_header[] = {0x1f, 0x8b, 0x08, 0x00};

	client_option_t *client_option = NULL;
	client_option = (client_option_t *) client->arg;

	if (httpRequest_get_context_buf(
			&client_option->httpRequest) == NULL
		|| httpRequest_get_context_data_len(
			&client_option->httpRequest) <= 0)
	{
		return false;
	}
	
	if (parse_skylog_info(&client_option->skylog, 
			&client_option->httpRequest) == false)
	{
		return false;
	}

	//LOG_TRACE_NORMAL("%s\n", client_option->httpRequest.context_buf);
	
	p = strstr(httpRequest_get_context_buf(
					&client_option->httpRequest), gzip_header);
	if (p)
	{
		tail_len = strlen(CLIENT_TALK_TOKEN_END);
		file_len = httpRequest_get_context_buf(
						&client_option->httpRequest) + 
					httpRequest_get_context_data_len(
						&client_option->httpRequest) - p - tail_len;
		
		skylog_save(&client_option->skylog, p, file_len);
		skylog_save_finished(&client_option->skylog);
		return true;
	}
	else
	{
		LOG_TRACE_NORMAL("stream is not gzip format!\n");
	}

	return false;
}


