#ifndef __CONFIG_H__
#define __CONFIG_H__

/*
	�ͻ��˳�ʱʱ��(��):
	�ڴ�ʱ����IOû���κβ����Ŀ���ʱ��
*/
#define CLIENT_TIMEOUT_SEC			10

/*
	�������˿�
*/
#define SERVER_BIND_PORT			9110

/*
	�������������еĴ�С
*/
#define SERVER_LISTEN_QUEUE_SIZE	32

/*
	�����������̳߳ؿ���
	������ʾ��
	���α�ʾ�ر�
*/
#define SERVER_THREAD_POOL_ENABLE	1

/*
	�����������̳߳ش�С��
	ȫ���̳߳��з���
*/
#define SERVER_THREAD_POOL_SIZE		1024

/*
	������ÿ�������̶߳��г���
*/
#define SERVER_WORKER_FIFO_BUF_SIZE	32


/*
	������ͬʱ����Ŀͻ�������
*/
#define CLIENT_ACCEPT_MAX_COUNT		65530

/*
	ȫ���̳߳�����
*/
#define THREAD_POOL_SIZE			SERVER_THREAD_POOL_SIZE

/*
	�������е���־�ļ�Ŀ¼
*/
#define LOG_OUT_DIR					"./out"

/*
	�������е���־�ļ�����
	stdout ��׼��������ļ�
*/
#define LOG_STDOUT_FILE				"trace_server_stdout.log"

/*
	�������е���־�ļ�����
	stderr ��׼������������ļ�
*/
#define LOG_STDERR_FILE				"trace_server_stderr.log"

/*
	�ն��ϴ��������ļ�����ĸ�Ŀ¼
*/
#define SKYLOG_ROOT_PATH			"/log"

/*
	�ն��ϴ���־������·��
*/
#define SKYLOG_REQUEST_URL_PATH		"/index"

/*
	ÿ�γɹ�������־�ļ������¼��־·��
	�ú��ʾ�ü�¼�õ�Ŀ¼
*/
#define SKYLOG_RECORD_DIR		"./record"

#endif

