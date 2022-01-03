#ifndef EVENT_LOOP_THREAD_H
#define EVENT_LOOP_THREAD_H

//event_loop_thread �� reactor ���߳�ʵ�֣������׽��ֵ� read/write �¼���ⶼ��������߳�����ɵ�

#include <pthread.h>

struct event_loop_thread 
{
	struct event_loop *eventLoop;
	pthread_t thread_tid;         //�߳� ID
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	char * thread_name;
	long thread_count;            //connections handled
};

//��ʼ���Ѿ������ڴ��event_loop_thread
int event_loop_thread_init(struct event_loop_thread *, int);

//�����̵߳��ã���ʼ��һ�����̣߳����������߳̿�ʼ����event_loop
struct event_loop *event_loop_thread_start(struct event_loop_thread *);

#endif