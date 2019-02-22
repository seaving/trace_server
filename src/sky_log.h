#ifndef __SKY_LOG_H__
#define __SKY_LOG_H__

#include "skylog_type.h"
#include "http_request.h"

typedef struct __sky_log__
{
	char hw[64];
	char ver[64];
	char sn[64];
	int ch;
	char create_data[32];
	char file_path[255];
	skylog_type_t type;
	file_t *f;
} skylog_t;

bool parse_skylog_info(skylog_t *skylog, httpRequest_t *httpRequest);
bool skylog_save(skylog_t *skylog, char *data, int data_len);
void skylog_save_finished(skylog_t *skylog);

#endif

