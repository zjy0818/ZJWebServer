#ifndef CHANNEL_H
#define CHANNEL_H

#include "common.h"
#include "event_loop.h"
#include "buffer.h"

/*channel:注册到 event_loop 上的监听事件，注册到 event_loop 上的套接字读写事件等，都抽象成channel来表示*/

#define EVENT_TIMEOUT    0x01
/* Wait for a socket or FD to become readable */
#define EVENT_READ        0x02
/* Wait for a socket or FD to become writeable */
#define EVENT_WRITE    0x04
/* Wait for a POSIX signal to be raised*/
#define EVENT_SIGNAL    0x08

typedef int (*event_read_callback)(void *data);

typedef int (*event_write_callback)(void *data);

struct channel 
{
    int fd;
    int events;   //表示event类型

    //绑定了事件处理函数 event_read_callback 和 event_write_callback。
    event_read_callback eventReadCallback;
    event_write_callback eventWriteCallback;
    void *data; //callback data, 可能是event_loop，也可能是tcp_server或者tcp_connection
};

struct channel *channel_new(int fd, int events, event_read_callback eventReadCallback, event_write_callback eventWriteCallback, void *data);

int channel_write_event_is_enabled(struct channel *channel);    //是否可写

int channel_write_event_enable(struct channel *channel);       //置为可写

int channel_write_event_disable(struct channel *channel);      //置为不可写


#endif