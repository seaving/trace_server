#ifndef __FIFO_H__
#define __FIFO_H__

#include "common.h"

/**
kfifo�ṩ���¶��⹦�ܹ��
1.ֻ֧��һ�����ߺ�һ�����߲�������
2.�������Ķ�д����������ռ䲻�����򷵻�ʵ�ʷ��ʿռ�
*/

#define FIFO_DATA_LEN(fifo) 	(fifo->in - fifo->out)
#define FIFO_FREE_SIZE(fifo) 	(fifo->size - fifo->in + fifo->out)
#define FIFO_BUFF_SIZE(fifo) 	(fifo->size)

typedef void (*copy_fn)(void *, void *, int);
typedef pthread_mutex_t fifo_lock_t;

typedef struct __fifo__
{
	char 		*name;		/* fifo������ */
    void	 *buffer;  /* ���ڴ�����ݵĻ��� */
    unsigned int size;    	/* buffer�ռ�Ĵ�С���ڳ���ʱ������������չ��2���� */
    unsigned int in;    	/* inָ��buffer�ж�ͷ */
    unsigned int out;    	/* ����outָ��buffer�еĶ�β */
    fifo_lock_t lock;    	/* ���ʹ�ò��ܱ�֤�κ�ʱ�����ֻ��һ�����̺߳�д�̣߳���Ҫʹ�ø�lockʵʩͬ�� */
	bool 		disable;	/* ����fifo���� */
	copy_fn		copy_fn;
} fifo_t;

typedef fifo_t FT_FIFO;

unsigned int fifo_cal_buffer_size(unsigned int size);

bool fifo_create_by_array(fifo_t *fifo, char *fifo_name, 
		void *buffer, unsigned int size, copy_fn copy_fn);

unsigned int fifo_put(fifo_t *fifo,
             void *buffer, unsigned int len);

unsigned int fifo_read(fifo_t *fifo,
             void *buffer, unsigned int len);

unsigned int fifo_seek_read(fifo_t *fifo,
             void *buffer, unsigned int offset, unsigned int len);

unsigned int fifo_safe_put(fifo_t *fifo,
             void *buffer, unsigned int len);

unsigned int fifo_set_offset(fifo_t *fifo, unsigned int offset);

void fifo_clean(fifo_t *fifo);

void fifo_enable(fifo_t *fifo);

void fifo_disable(fifo_t *fifo);

#endif





