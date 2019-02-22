#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__
 
#include <pthread.h>
 
/* Ҫִ�е��������� */
typedef struct __tpool_work__
{
	void*				(*routine)(void*);		 /* ������ */
	void				*arg;					 /* �����������Ĳ��� */
	struct __tpool_work__	*next;					  
} tpool_work_t;

typedef struct __tpools__
{
	int 			shutdown;					 /* �̳߳��Ƿ����� */
	int 			max_thr_num;				/* ����߳��� */
	int				worker_counts;				/* �����ɹ��߳����� */
	int 			working_counts;				/* ����ִ��������߳����� */
	pthread_t		*thr_id;					/* �߳�ID���� */
	tpool_work_t	*queue_head;				/* �߳����� */
	pthread_mutex_t queue_lock;					/* �̵߳��ź��� */
	pthread_cond_t	queue_ready;
	pthread_mutex_t put_lock;					/* ����/ȡ������ */
} tpool_t;


int tpool_create(int max_thr_num);
int tpool_destroy();
int tpool_add_work(void *(*routine)(void*), void *arg);

int display_worker_counts();
int tpool_get_working_count();
int tpool_get_worker_count();

#endif

