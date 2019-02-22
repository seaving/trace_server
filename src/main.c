#include "includes.h"

static inline void _delay(int sec)
{
	int i = 0;
	for (i = 0; i < sec; i ++)
	{
		//LOG_TRACE_NORMAL("please wait %d sec .\n", sec - i);
		sleep(1);
	}
}

static inline bool _create_server()
{
	if (evserver_create() == false)
	{
		LOG_TRACE_NORMAL("create server failt!\n");
		return false;
	}

	LOG_TRACE_NORMAL("create server success.\n");
	return true;
}

static inline bool _repeat_init(int repeat_count)
{
	int i;
	for (i = 0; evserver_status_ok() == false && i < repeat_count; i ++)
	{
		LOG_TRACE_NORMAL("repeat init server ... %d\n", i);
		
		if (_create_server())
		{
			break;
		}

		sleep(1);
	}

	if (i >= repeat_count)
	{
		return false;
	}

	return true;
}

static inline void *_server_monitor_worker(void *arg)
{
	pthread_detach(pthread_self());
	
	for ( ; ; )
	{
		if (_repeat_init(30) == false)
		{
			LOG_TRACE_NORMAL("exit!\n");
			exit(-1);
		}
		
		sleep(3);
	}

	return NULL;
}

static inline void _display_status()
{
	static int tpool_working_count = 0;
	static int tpool_worker_count = 0;

#if defined(SERVER_THREAD_POOL_ENABLE)
	static int evserver_working_count = 0;
	static int client_wait_count = 0;
	static int client_handling_count = 0;
#else
	static int client_count = 0;
#endif
	
	if (tpool_working_count != tpool_get_working_count()
#if defined(SERVER_THREAD_POOL_ENABLE)
		|| evserver_working_count != evserver_tpool_get_working_count()
		|| client_wait_count != evserver_tpool_get_client_wait_count()
		|| client_handling_count != evserver_tpool_get_client_handling_count()
#else
		|| client_count != get_evserver_client_connected_count()
#endif
		)
	{
		tpool_worker_count = tpool_get_worker_count();
		tpool_working_count = tpool_get_working_count();
		
#if defined(SERVER_THREAD_POOL_ENABLE)
		evserver_working_count = evserver_tpool_get_working_count();
		client_wait_count = evserver_tpool_get_client_wait_count();
		client_handling_count = evserver_tpool_get_client_handling_count();
#else
		client_count = get_evserver_client_connected_count();
#endif

		LOG_TRACE_NORMAL("\n");
		LOG_TRACE_NORMAL("-----------------------------------------------\n");
		LOG_TRACE_NORMAL("-  tpool worker count: %d\n", tpool_worker_count);
		LOG_TRACE_NORMAL("- tpool working count: %d\n", tpool_working_count);
#if defined(SERVER_THREAD_POOL_ENABLE)
		LOG_TRACE_NORMAL("       server workers: %d\n", evserver_working_count);
		LOG_TRACE_NORMAL("-    server pool mode: %s\n", "enable");
		LOG_TRACE_NORMAL("          client wait: %d\n", client_wait_count);
		LOG_TRACE_NORMAL("      client handling: %d\n", client_handling_count);
#else
		LOG_TRACE_NORMAL("   client connections: %d\n", client_count);
#endif
		LOG_TRACE_NORMAL("-----------------------------------------------\n");
		LOG_TRACE_NORMAL("\n");
	}
}

static inline void _monitor_loop()
{
	for ( ; ; )
	{
		_display_status();
		sleep(1);
	}
}

static inline void _usage()
{
	char sys_time[256] = {0};
	system_time("current date", sys_time, sizeof(sys_time));
	LOG_TRACE_NORMAL("\n");
	LOG_TRACE_NORMAL("------------------------------------------------------\n");
	LOG_TRACE_NORMAL("\t %s version: %s\n", PROGRAME_NAME, VERSION);
	LOG_TRACE_NORMAL("\t %s\n", sys_time);
	LOG_TRACE_NORMAL("\t    thread pool size: %d\n", THREAD_POOL_SIZE);
	LOG_TRACE_NORMAL("\t         server port: %d\n", SERVER_BIND_PORT);
	LOG_TRACE_NORMAL("\t      client timeout: %d sec\n", CLIENT_TIMEOUT_SEC);
	LOG_TRACE_NORMAL("\t    listen max count: %d\n", CLIENT_ACCEPT_MAX_COUNT);
#if defined(SERVER_THREAD_POOL_ENABLE)
	LOG_TRACE_NORMAL("\t    server pool mode: %s\n", "enable");
	LOG_TRACE_NORMAL("\t      server workers: %d\n", SERVER_THREAD_POOL_SIZE);
	LOG_TRACE_NORMAL("\t server worker queue: %d\n", SERVER_WORKER_FIFO_BUF_SIZE);
#endif
	LOG_TRACE_NORMAL("------------------------------------------------------\n");
	LOG_TRACE_NORMAL("\n");
}

int main()
{
	pthread_t tid;
	
	signal(SIGPIPE, SIG_IGN);
	
	time_init();

	log_trace_stdout_init(LOG_OUT_DIR, LOG_STDOUT_FILE);
	log_trace_stderr_init(LOG_OUT_DIR, LOG_STDERR_FILE);

	_usage();
	
	if (tpool_create(THREAD_POOL_SIZE) < 0)
	{
		LOG_TRACE_NORMAL("create thread pool failt!\n");
		return -1;
	}

	//_delay(10);
	for ( ; tpool_get_worker_count() != THREAD_POOL_SIZE; )
	{
		LOG_TRACE_NORMAL("please wait thread pool init (%d).\n", 
				tpool_get_worker_count());
		sleep(1);
	}
	LOG_TRACE_NORMAL("thread pool init (%d) success.\n", 
					tpool_get_worker_count());

#if defined(SERVER_THREAD_POOL_ENABLE)
	if (evserver_tpool_create(SERVER_THREAD_POOL_SIZE, 
			SERVER_WORKER_FIFO_BUF_SIZE) == false)
	{
		LOG_TRACE_NORMAL("evserver_tpool_create error! exit!!!\n");
		return -1;
	}
#endif

	LOG_TRACE_NORMAL("\n");
	LOG_TRACE_NORMAL("------ loop start ------\n");
	
	if (pthread_create(&tid, NULL, 
			_server_monitor_worker, NULL) != 0)
	{
		LOG_TRACE_PERROR("pthread_create error!");
		return -1;
	}
	
	for ( ; ; )
	{
		_monitor_loop();

		sleep(3);
	}

	tpool_destroy();
	log_trace_stdout_exit();
	log_trace_stderr_exit();
	return 0;
}


