#ifndef THREAD_POOL_H
#define THREAD_POOL_H

//  thread_pool 维护了一个 sub-reactor 的线程列表，它可以提供给主 reactor 线程使用，每次当有新的连接建立时，
// 可以从 thread_pool 里获取一个线程，以便用它来完成对新连接套接字的 read/write 事件注册，将 I/O 线程和主 reactor 线程分离

#include "event_loop.h"
#include "event_loop_thread.h"

struct thread_pool
{
	struct event_loop *mainLoop;   //创建thread_pool的主线程
	int started;                   //是否已经启动
	int thread_number;             //线程数目
	struct event_loop_thread *eventLoopThreads;
	                               //数组指针，指向创建的event_loop_thread数组	
	int position;                  //表示在数组里的位置，用来决定选择哪个event_loop_thread服务
};

struct thread_pool *thread_pool_new(struct event_loop *mainLoop, int threadNumber);

void thread_pool_start(struct thread_pool *);

struct event_loop *thread_pool_get_loop(struct thread_pool *);

#endif