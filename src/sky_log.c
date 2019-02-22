#include "includes.h"
#include "skylog_record.h"

static pthread_mutex_t _skylog_lock = PTHREAD_MUTEX_INITIALIZER;

static void _skylog_info_init(skylog_t *skylog)
{
	if (skylog)
	{
		memset(skylog, 0, sizeof(skylog_t));
		snprintf(skylog->hw, sizeof(skylog->hw), "UnkownProduct");
		snprintf(skylog->ver, sizeof(skylog->ver), "UnkownVersion");
		snprintf(skylog->sn, sizeof(skylog->sn), "UnkownSn");
		skylog->ch = 1;
		skylog->type = E_SKY_LOG_TYPE_UNKOWN;
	}
}

static bool _skylog_check_create_data(skylog_t *skylog)
{
	int i = 0;
	char *create_data = NULL;
	if (skylog)
	{
		create_data = skylog->create_data;
		for (i = 0; i < strlen(create_data); i ++)
		{
			if (! (0 <= i && i <= 3 && '0' <= create_data[i] && create_data[i] <= '9'))
			{
				break;
			}
			else if (! (i == 4 && create_data[i] == '-'))
			{
				break;
			}
			else if (! (5 <= i && i <= 6 && '0' <= create_data[i] && create_data[i] <= '9'))
			{
				break;
			}
			else if (! (i == 7 && create_data[i] == '-'))
			{
				break;
			}
			else if (! (8 <= i && i <= 9 && '0' <= create_data[i] && create_data[i] <= '9'))
			{
				break;
			}
		}

		if (i >= strlen(create_data))
		{
			return false;
		}

		return true;
	}

	return false;
}

static bool _create_and_open_skylog_file(skylog_t *skylog)
{
	char *p = SKYLOG_ROOT_PATH;
	char dir_path[255] = {0};
	char file_path[255] = {0};
	int Year, Mon, Data, Hour, Min, Sec;
	char time_str[64] = {0};
	bool ret = false;
	
	if (skylog == NULL)
	{
		return false;
	}

	if (strlen(skylog->file_path) > 0 
		&& create_file(skylog->file_path))
	{
		clear_file(skylog->file_path);
		skylog->f = open_file(skylog->file_path);
		LSEEK_TO_END(skylog->f);
		return skylog->f != NULL;
	}

	GetSystemUTCTime(&Year, &Mon, &Data, &Hour, &Min, &Sec);
	
	if (_skylog_check_create_data(skylog) == false)
	{
		snprintf(skylog->create_data, 
			sizeof(skylog->create_data) - 1, 
			"20%d-%02d-%02d", 
			Year, Mon, Data);
	}
	
	snprintf(time_str, sizeof(time_str) - 1, 
			"20%d-%02d-%02d_%02d_%02d_%02d_%lu",
			Year, 
			Mon, 
			Data, 
			Hour, 
			Min, 
			Sec, GetTimestrip_us());
	
	snprintf(dir_path, 
		sizeof(dir_path) - 1, 
		"%s%s%s/%s/%s/%s/%s", 
		SKYLOG_ROOT_PATH,
		p[strlen(p) - 1] == '/' ? "" : "/", 
		get_skylog_type_name(skylog->type), 
		skylog->hw, 
		skylog->ver, 
		skylog->sn, 
		skylog->create_data);

	pthread_mutex_lock(&_skylog_lock);
	ret = create_dir(dir_path);
	pthread_mutex_unlock(&_skylog_lock);
	if (ret)
	{
		if(skylog->type == E_SKY_LOG_TYPE_CRASH)
		{
			snprintf(file_path, 
				sizeof(file_path) - 1, 
				"%s/%s_%s_%s.info", 
				dir_path, 
				get_skylog_type_name(skylog->type), 
				skylog->sn, 
				time_str);
		}
		else
		{
			snprintf(file_path, 
				sizeof(file_path) - 1, 
				"%s/%s_%s_%s.gz", 
				dir_path, 
				get_skylog_type_name(skylog->type), 
				skylog->sn, 
				time_str);
		}

		snprintf(skylog->file_path, 
			sizeof(skylog->file_path), "%s", file_path);
		if (create_file(file_path))
		{
			clear_file(file_path);
			skylog->f = open_file(file_path);
			LSEEK_TO_END(skylog->f);
			return skylog->f != NULL;
		}
	}

	return false;
}

bool parse_skylog_info(skylog_t *skylog, httpRequest_t *httpRequest)
{
	char value[1024] = {0};
	if (skylog && httpRequest)
	{
		_skylog_info_init(skylog);

		memset(value, 0, sizeof(value));
		if (httpRequest_find_field(
				httpRequest, "Create-Data", 
				value, sizeof(value) - 1) == true)
		{
			snprintf(skylog->create_data, 
				sizeof(skylog->create_data) - 1, "%s", value);
		}
		
		memset(value, 0, sizeof(value));
		if (httpRequest_get_url_parmer_value(
				httpRequest, "Hw", strlen("Hw"), 
				value, sizeof(value) - 1) == true)
		{
			snprintf(skylog->hw, sizeof(skylog->hw) - 1, "%s", value);
		}

		memset(value, 0, sizeof(value));
		if (httpRequest_get_url_parmer_value(
				httpRequest, "Ver", strlen("Ver"), 
				value, sizeof(value) - 1) == true)
		{
			snprintf(skylog->ver, sizeof(skylog->ver) - 1, "%s", value);
		}

		memset(value, 0, sizeof(value));
		if (httpRequest_get_url_parmer_value(
				httpRequest, "Sn", strlen("Sn"), 
				value, sizeof(value) - 1) == true)
		{
			snprintf(skylog->sn, sizeof(skylog->sn) - 1, "%s", value);
		}

		memset(value, 0, sizeof(value));
		if (httpRequest_get_url_parmer_value(
				httpRequest, "Ch", strlen("Ch"), 
				value, sizeof(value) - 1) == true)
		{
			if (isdigit_str(value))
			{
				skylog->ch = atoi(value);
			}
		}

		memset(value, 0, sizeof(value));
		if (httpRequest_get_url_parmer_value(
				httpRequest, "Type", strlen("Type"), 
				value, sizeof(value) - 1) == true)
		{
			if (isdigit_str(value))
			{
				skylog->type = atoi(value);
			}
		}

		if(strstr(skylog->hw, "SolisHome"))
		{
			memset(skylog->hw, 0, sizeof(skylog->hw));
			strcpy(skylog->hw, "SolisHome");
		}

		/*LOG_TRACE_NORMAL("\nhw: %s\n"
						"ver: %s\n"
						"sn: %s\n"
						"ch: %d\n"
						"type: %d\n",
						"create_data: %s\n"
						skylog->hw,
						skylog->ver,
						skylog->sn,
						skylog->ch,
						skylog->typem,
						skylog->create_data);*/

		return true;
	}

	return false;
}

static inline bool _skylog_file_create(skylog_t *skylog)
{
	if (skylog == NULL)
	{
		return false;
	}

	if (skylog->f)
	{
		return true;
	}

	return _create_and_open_skylog_file(skylog);
}

static inline bool _skylog_write_to_file(skylog_t *skylog, char *data, int data_len)
{
	if (data && data_len > 0)
	{
		if (skylog && skylog->f == NULL)
		{
			if (_skylog_file_create(skylog) == false)
			{
				return false;
			}
		}

		if (skylog && skylog->f)
		{
			if (write_file(skylog->f, data, data_len) == data_len)
			{
				return true;
			}
		}
	}

	return false;
}

static inline void _skylog_write_finish(skylog_t *skylog)
{
	if (skylog)
	{
		file_close(skylog->f);
		skylog->f = NULL;
	}
}

bool skylog_save(skylog_t *skylog, char *data, int data_len)
{
	int i = 0;
	int len = 0;
	char record_data[512] = {0};
	bool ret = false;
	if (skylog == NULL
		|| data == NULL
		|| data_len <= 0)
	{
		return false;
	}

	for (i = 0; i < 3; i ++)
	{
		if (i > 0)
		{
			LOG_TRACE_NORMAL("Number.%d: repeat write to file %s ...\n", 
					i + 1, skylog->file_path);
		}
		
		ret = _skylog_write_to_file(skylog, data, data_len);
		if (ret == true)
		{
			len = snprintf(record_data, 
				sizeof(record_data) - 1, 
				"%s\r\n", skylog->file_path);
			skylog_record(record_data, len);
			break;
		}
		else
		{
			_skylog_write_finish(skylog);
		}
	}

	if (ret == false)
	{
		LOG_TRACE_NORMAL("writed %d times file %s, but still failt!!!\n", 
				i, skylog->file_path);
	}
	
	return ret;
}

void skylog_save_finished(skylog_t *skylog)
{
	_skylog_write_finish(skylog);
}

