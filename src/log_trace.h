#ifndef __LOG_TRACE_H__
#define __LOG_TRACE_H__

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "version.h"
#include "user_time.h"

#define DEBUG_POINT			LOG_TRACE_NORMAL("\n")

#define LOG(first...)		LOG_TRACE_NORMAL(first)
#define PERROR(first...)	LOG_TRACE_PERROR(first)

#define LOG_TRACE_NORMAL(first...) \
	do { \
		log_trace_lock(); \
		printf("[fun: %s] [line: %d] [tick_sec: %u]: ", __FUNCTION__, __LINE__, SYSTEM_SEC); \
		printf(first); \
		fflush(stdout); \
		log_trace_unlock();\
	} while (0)

#define LOG_TRACE_PERROR(first...) \
	do { \
		log_trace_lock(); \
		printf("[fun: %s] [line: %d] [tick_sec: %u]: ", __FUNCTION__, __LINE__, SYSTEM_SEC); \
		printf(first); \
		printf(" [error info(errno=%d): %s]\n", errno, strerror(errno)); \
		fflush(stdout); \
		log_trace_unlock();\
	} while (0)

void log_trace_stdout_init(
		char *log_dir_path, 
		char *log_file_name);
void log_trace_stderr_init(
		char *log_dir_path, 
		char *log_file_name);

void log_trace_stdout_exit();
void log_trace_stderr_exit();

void log_trace_lock();
void log_trace_unlock();

#endif

