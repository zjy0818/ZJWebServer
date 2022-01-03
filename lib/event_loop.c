#include <assert.h>
#include "event_loop.h"
#include "common.h"
#include "event_dispatcher.h"
#include "channel.h"
#include "utils.h"


//初始化
struct event_loop *event_loop_init()
{
    return event_loop_init_with_name(NULL);
}

struct event_loop *event_loop_init_with_name(char *thread_name)
{
	struct event_loop *eventLoop = malloc(sizeof(struct event_loop));
	pthread_mutex_init(&eventLoop->mutex, NULL);
	pthread_cond_init(&eventLoop->cond, NULL);
	
	if (thread_name != NULL)
	{
		eventLoop->thread_name = thread_name;
	}
	else
	{
		eventLoop->thread_name = "main thread";
	}
	
	eventLoop->quit = 0;
    eventLoop->channelMap = malloc(sizeof(struct channel_map));
    map_init(eventLoop->channelMap);
	
	printf("set epoll as dispatcher, %s\n", eventLoop->thread_name);
    eventLoop->eventDispatcher = &epoll_dispatcher;
	
	eventLoop->event_dispatcher_data = eventLoop->eventDispatcher->init(eventLoop);
	
    //这里创建的是套接字对，目的是为了唤醒子线程
	//往这个套接字的一端写时，另外一端就可以感知到读的事件，类似于管道
    eventLoop->owner_thread_id = pthread_self();
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, eventLoop->socketPair) < 0) 
	{
		printf("socketpair set failed\n");
	}
	
    eventLoop->is_handle_pending = 0;
    eventLoop->pending_head = NULL;
    eventLoop->pending_tail = NULL;
	
	//注册了 socketPair[1]描述字上的 READ 事件，如果有 READ 事件发生，就调用 handleWakeup 函数来完成事件处理
    struct channel *channel = channel_new(eventLoop->socketPair[1], EVENT_READ, handleWakeup, NULL, eventLoop);
    event_loop_add_channel_event(eventLoop, eventLoop->socketPair[1], channel);

    return eventLoop;
}


/*
 * 1.参数验证
 * 2.调用dispatcher来进行事件分发,分发完回调事件处理函数
 */
int event_loop_run(struct event_loop *eventLoop)
{
	assert(eventLoop != NULL);
	
	struct event_dispatcher *dispatcher = eventLoop->eventDispatcher;
	
    if (eventLoop->owner_thread_id != pthread_self()) 
    {
        exit(1);
    }
	
	printf("event loop run, %s\n", eventLoop->thread_name);
	struct timeval timeval;
	timeval.tv_sec = 1;
	
	while (!eventLoop->quit)
	{
		//阻塞此处以等待I/O事件，并获取channels
        dispatcher->dispatch(eventLoop, &timeval);

        //处理挂起的channels，如果是子线程被唤醒，这个部分也会立即执行到
        event_loop_handle_pending_channel(eventLoop);
	}
	
    printf("event loop end, %s\n", eventLoop->thread_name);
    return 0;
}


void event_loop_wakeup(struct event_loop *eventLoop)
{
	char one = 'a';
	ssize_t n = write(eventLoop->socketPair[0], &one, sizeof(one));
	if (n != sizeof(one))
	{
		printf("wakeup event loop thread failed\n");
	}
}

//从 socketPair[1]描述字上读取一个字符, 可以让子线程从 dispatch 的阻塞中苏醒
int handleWakeup(void *data)
{
	struct event_loop *eventLoop = (struct event_loop *) data;
	char one;
	ssize_t n = read(eventLoop->socketPair[1], &one, sizeof one);
	if (n != sizeof(one)) 
	{
		printf("handleWakeup  failed\n");
	}

	printf("wakeup, %s\n", eventLoop->thread_name);
}

//在当前的 I/O 线程中执行，
//遍历当前 event loop 里挂起的 channel event 列表，
//将它们和 event_dispatcher 关联起来，从而修改感兴趣的事件集合
int event_loop_handle_pending_channel(struct event_loop *eventLoop)
{
	pthread_mutex_lock(&eventLoop->mutex);
	eventLoop->is_handle_pending = 1;

	struct channel_element *channelElement = eventLoop->pending_head;
	while (channelElement != NULL)
	{
		struct channel *channel = channelElement->channel;
		int fd = channel->fd;
		if (channelElement->type == 1)
		{
			event_loop_handle_pending_add(eventLoop, fd, channel);
		}
		else if (channelElement->type == 2)
		{
			event_loop_handle_pending_remove(eventLoop, fd, channel);
		}
		else if (channelElement->type == 3)
		{
			event_loop_handle_pending_update(eventLoop, fd, channel);
		}
		channelElement = channelElement->next;
	}

	eventLoop->pending_head = eventLoop->pending_tail = NULL;
	eventLoop->is_handle_pending = 0;

	pthread_mutex_unlock(&eventLoop->mutex);

	return 0;
}

void event_loop_channel_buffer_nolock(struct event_loop *eventLoop, int fd, struct channel *channel1, int type)
{
	struct channel_element *channelElement = malloc(sizeof(struct channel_element));
	channelElement->channel = channel1;
	channelElement->type = type;
	channelElement->next = NULL;

	//第一个元素
	if (eventLoop->pending_head == NULL) 
	{
		eventLoop->pending_head = eventLoop->pending_tail = channelElement;
	}
	else 
	{
		eventLoop->pending_tail->next = channelElement;
		eventLoop->pending_tail = channelElement;
	}
}

int event_loop_do_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1, int type)
{
	pthread_mutex_lock(&eventLoop->mutex);
	assert(eventLoop->is_handle_pending == 0);
	//往该线程的channel列表里增加需要处理的 channel event 对象
	//所有增加的 channel 对象以列表的形式维护在子线程的数据结构中。
	event_loop_channel_buffer_nolock(eventLoop, fd, channel1, type);
	pthread_mutex_unlock(&eventLoop->mutex);

	//如果是主线程发起操作，则调用event_loop_wakeup唤醒子线程
	if (!isInSameThread(eventLoop)) 
	{
		event_loop_wakeup(eventLoop);
	}
	else 
	{
		//如果是子线程自己，则直接调用 event_loop_handle_pending_channel 处理新增加的 channel event 事件列表。
		event_loop_handle_pending_channel(eventLoop);
	}

	return 0;
}

//入口函数，用于增加、删除和修改 channel event 事件
int event_loop_add_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1)
{
	return event_loop_do_channel_event(eventLoop, fd, channel1, 1);
}

int event_loop_remove_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1)
{
	return event_loop_do_channel_event(eventLoop, fd, channel1, 2);
}

int event_loop_update_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1)
{
	return event_loop_do_channel_event(eventLoop, fd, channel1, 3);
}

/*
 *在当前 event_loop 的 channel_map 里增加一个新的 key-value 对
 *在当前的 I/O 线程中执行
*/
int event_loop_handle_pending_add(struct event_loop *eventLoop, int fd, struct channel *channel)
{
	printf("add channel fd == %d, %s\n", fd, eventLoop->thread_name);
	struct channel_map *map = eventLoop->channelMap;

	if (fd < 0)
	{
		return 0;
	}

	if (fd >= map->nentries)
	{
		if (map_make_space(map, fd, sizeof(struct channel *)) == -1)
			return -1;
	}

	//第一次创建，增加
	if (map->entries[fd] == NULL)
	{
		map->entries[fd] = channel;

		//调用 event_dispatcher 对象的 add 方法增加 channel event 事件。注意这个方法总在当前的 I/O 线程中执行
		struct event_dispatcher *eventDispatcher = eventLoop->eventDispatcher;
		eventDispatcher->add(eventLoop, channel);
		return 1;
	}

	return 0;
}

/*
 *在当前 event_loop 的 channel_map 里删除一个新的 key-value 对
 *在当前的 I/O 线程中执行
*/
int event_loop_handle_pending_remove(struct event_loop *eventLoop, int fd, struct channel *channel1)
{
	printf("remove channel fd == %d, %s\n", fd, eventLoop->thread_name);
	struct channel_map *map = eventLoop->channelMap;
	assert(fd == channel1->fd);

	if (fd < 0)
		return 0;

	if (fd >= map->nentries)
		return -1;

	struct channel *channel2 = map->entries[fd];

	//update dispatcher(multi-thread)here
	struct event_dispatcher *eventDispatcher = eventLoop->eventDispatcher;

	int retval = 0;
	if (eventDispatcher->del(eventLoop, channel2) == -1)
	{
		retval = -1;
	}
	else
	{
		retval = 1;
	}

	map->entries[fd] = NULL;
	return retval;
}


/*
 *在当前 event_loop 的 channel_map 里修改一个新的 key-value 对
 *在当前的 I/O 线程中执行
*/
int event_loop_handle_pending_update(struct event_loop *eventLoop, int fd, struct channel *channel)
{
	printf("update channel fd == %d, %s\n", fd, eventLoop->thread_name);
	struct channel_map *map = eventLoop->channelMap;

	if (fd < 0)
		return 0;

	if (map->entries[fd] == NULL) 
	{
		return -1;
	}

	//update channel
	struct event_dispatcher *eventDispatcher = eventLoop->eventDispatcher;
	eventDispatcher->update(eventLoop, channel);

	return 1;
}

// dispather派发完事件之后，调用该方法通知event_loop执行对应事件的相关callback方法
// res: EVENT_READ | EVENT_READ等
int channel_event_activate(struct event_loop *eventLoop, int fd, int revents)
{
	struct channel_map *map = eventLoop->channelMap;
	printf("activate channel fd == %d, revents=%d, %s\n", fd, revents, eventLoop->thread_name);

	if (fd < 0)
		return 0;

	if (fd >= map->nentries)
		return -1;

	struct channel *channel = map->entries[fd];
	assert(fd == channel->fd);

	if (revents & (EVENT_READ)) 
	{
		if (channel->eventReadCallback) 
			channel->eventReadCallback(channel->data);
	}
	if (revents & (EVENT_WRITE)) 
	{
		if (channel->eventWriteCallback) 
			channel->eventWriteCallback(channel->data);
	}

	return 0;
}
