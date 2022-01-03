#ifndef ACCEPTOR_H
#define ACCEPTOR_H

//acceptor 对象表示的是服务器端监听器，acceptor 对象最终会作为一个 channel 对象，注册到 event_loop 上，
//以便进行连接完成的事件分发和检测

#include "common.h"

struct acceptor 
{
	int listen_port;
	int listen_fd;
};

struct acceptor *acceptor_init(int port);

#endif