#ifndef THREAD_POOL_H
#define THREAD_POOL_H

//  thread_pool ά����һ�� sub-reactor ���߳��б��������ṩ���� reactor �߳�ʹ�ã�ÿ�ε����µ����ӽ���ʱ��
// ���Դ� thread_pool ���ȡһ���̣߳��Ա���������ɶ��������׽��ֵ� read/write �¼�ע�ᣬ�� I/O �̺߳��� reactor �̷߳���

#include "event_loop.h"
#include "event_loop_thread.h"

struct thread_pool
{
	struct event_loop *mainLoop;   //����thread_pool�����߳�
	int started;                   //�Ƿ��Ѿ�����
	int thread_number;             //�߳���Ŀ
	struct event_loop_thread *eventLoopThreads;
	                               //����ָ�룬ָ�򴴽���event_loop_thread����	
	int position;                  //��ʾ���������λ�ã���������ѡ���ĸ�event_loop_thread����
};

struct thread_pool *thread_pool_new(struct event_loop *mainLoop, int threadNumber);

void thread_pool_start(struct thread_pool *);

struct event_loop *thread_pool_get_loop(struct thread_pool *);

#endif