#include "includes.h"

static char _skylog_record_filename[128] = {0};
static file_t *_skylog_record_f = NULL;
static pthread_mutex_t _skylog_record_lock = PTHREAD_MUTEX_INITIALIZER;

static inline void _skylog_record_exit()
{
	if (_skylog_record_f)
		file_close(_skylog_record_f);
	_skylog_record_f = NULL;
	memset(_skylog_record_filename, 0, 
		sizeof(_skylog_record_filename));
}

static inline bool _skylog_record_create_and_open_file()
{
	int y, m, d, h, min, s;
	char filename[128] = {0};
	char path[255] = {0};

	if (_skylog_record_f == NULL)
	{
		_skylog_record_exit();
	}
	
	if (create_dir(SKYLOG_RECORD_DIR) == false)
	{
		_skylog_record_exit();
		return false;
	}

	GetSystemUTCTime(&y, &m, &d, &h, &min, &s);
	snprintf(filename, sizeof(filename) - 1, 
		"skylog_record_20%d-%02d-%02d.txt", 
		y, m, d);

	if (strcmp(filename, _skylog_record_filename) == 0
		&& _skylog_record_f != NULL)
	{
		return true;
	}

	_skylog_record_exit();

	memcpy(path, SKYLOG_RECORD_DIR, 
			strlen(SKYLOG_RECORD_DIR));
	snprintf(path + strlen(path), 
		sizeof(path) - strlen(path) - 1, 
		"%s%s", 
		path[strlen(path) - 1] == '/' ? "" : "/",
		filename);

	if (create_file(path) == false)
	{
		return false;
	}

	_skylog_record_f = open_file(path);
	if (_skylog_record_f == NULL)
	{
		return false;
	}

	LSEEK_TO_END(_skylog_record_f);
	memcpy(_skylog_record_filename, filename, strlen(filename));
	return true;
}

static inline bool _skylog_record_write(char *data, int data_len)
{
	if (_skylog_record_f)
	{
		if (write_file(_skylog_record_f, 
				data, data_len) != data_len)
		{
			return false;
		}

		return true;
	}

	return false;
}

bool skylog_record(char *data, int data_len)
{
	if (data && data_len > 0)
	{
		pthread_mutex_lock(&_skylog_record_lock);
		if (_skylog_record_create_and_open_file() == false)
		{
			_skylog_record_exit();
			pthread_mutex_unlock(&_skylog_record_lock);
			return false;
		}
		
		if (_skylog_record_write(data, data_len) == false)
		{
			_skylog_record_exit();
			pthread_mutex_unlock(&_skylog_record_lock);
			return false;
		}

		pthread_mutex_unlock(&_skylog_record_lock);
		return true;
	}

	return false;
}

void skylog_record_exit()
{
	pthread_mutex_lock(&_skylog_record_lock);
	_skylog_record_exit();
	pthread_mutex_unlock(&_skylog_record_lock);
}


