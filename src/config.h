#ifndef __CONFIG_H__
#define __CONFIG_H__

/*
	客户端超时时间(秒):
	在此时间内IO没有任何操作的空闲时间
*/
#define CLIENT_TIMEOUT_SEC			10

/*
	服务器端口
*/
#define SERVER_BIND_PORT			9110

/*
	服务器监听队列的大小
*/
#define SERVER_LISTEN_QUEUE_SIZE	32

/*
	服务器工作线程池开关
	定义宏表示打开
	屏蔽表示关闭
*/
#define SERVER_THREAD_POOL_ENABLE	1

/*
	服务器工作线程池大小，
	全局线程池中分配
*/
#define SERVER_THREAD_POOL_SIZE		1024

/*
	服务器每个工作线程队列长度
*/
#define SERVER_WORKER_FIFO_BUF_SIZE	32


/*
	服务器同时处理的客户端数量
*/
#define CLIENT_ACCEPT_MAX_COUNT		65530

/*
	全局线程池数量
*/
#define THREAD_POOL_SIZE			SERVER_THREAD_POOL_SIZE

/*
	程序运行的日志文件目录
*/
#define LOG_OUT_DIR					"./out"

/*
	程序运行的日志文件名称
	stdout 标准输出到该文件
*/
#define LOG_STDOUT_FILE				"trace_server_stdout.log"

/*
	程序运行的日志文件名称
	stderr 标准错误输出到该文件
*/
#define LOG_STDERR_FILE				"trace_server_stderr.log"

/*
	终端上传过来的文件保存的根目录
*/
#define SKYLOG_ROOT_PATH			"/log"

/*
	终端上传日志的请求路径
*/
#define SKYLOG_REQUEST_URL_PATH		"/index"

/*
	每次成功保存日志文件，则记录日志路径
	该宏表示该记录用的目录
*/
#define SKYLOG_RECORD_DIR		"./record"

#endif

