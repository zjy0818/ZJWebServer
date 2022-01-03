#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

/*event_loop:和一个线程绑定的，无限循环着的事件分发器*/

#include <pthread.h>
#include "channel.h"
#include "event_dispatcher.h"
#include "common.h"

extern const struct event_dispatcher epoll_dispatcher;

struct channel_element 
{
	int type; //1: add  2: delete  3: update
	struct channel *channel;
	struct channel_element *next;
};

//event_loop结构体
struct event_loop
{
	int quit;
	const struct event_dispatcher *eventDispatcher;
	
	/* 对应的event_dispatcher的数据 */
	void *event_dispatcher_data;
	struct channel_map *channelMap;
	
    int is_handle_pending;
	//保留在子线程内的需要处理的新事件
    struct channel_element *pending_head;
    struct channel_element *pending_tail;
	
    pthread_t owner_thread_id;  //保留每个 event loop 的线程 ID
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int socketPair[2];    //父线程用来通知子线程有新的事件需要处理
    char *thread_name;    //线程名
};

struct event_loop *event_loop_init();

struct event_loop *event_loop_init_with_name(char * thread_name);

int event_loop_run(struct event_loop *eventLoop);

void event_loop_wakeup(struct event_loop *eventLoop);

int handleWakeup(void *data);

int event_loop_handle_pending_channel(struct event_loop *eventLoop);

void event_loop_channel_buffer_nolock(struct event_loop *eventLoop, int fd, struct channel *channel1, int type);

int event_loop_do_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1, int type);

int event_loop_add_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1);

int event_loop_remove_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1);

int event_loop_update_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1);

int event_loop_handle_pending_add(struct event_loop *eventLoop, int fd, struct channel *channel);

int event_loop_handle_pending_remove(struct event_loop *eventLoop, int fd, struct channel *channel);

int event_loop_handle_pending_update(struct event_loop *eventLoop, int fd, struct channel *channel);

// dispather派发完事件之后，调用该方法通知event_loop执行对应事件的相关callback方法
// res: EVENT_READ | EVENT_READ等
int channel_event_activate(struct event_loop *eventLoop, int fd, int res);


#endif