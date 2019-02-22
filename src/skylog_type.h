#ifndef __SKYLOG_TYPE_H__
#define __SKYLOG_TYPE_H__


typedef enum __sky_log_type__
{
	E_SKY_LOG_TYPE_CRASH = 0,
	E_SKY_LOG_TYPE_ERROR,
	E_SKY_LOG_TYPE_LOG,
	E_SKY_LOG_TYPE_DEBUG,
	E_SKY_LOG_TYPE_FLOW,
	E_SKY_LOG_TYPE_DNS,

	/*
		在此处增加其他类型，
		在源文件中新增对应名称
	*/
	
	E_SKY_LOG_TYPE_UNKOWN
} skylog_type_t;


const char *get_skylog_type_name(skylog_type_t type);


#endif


