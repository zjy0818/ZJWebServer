#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

//完成报文的解析，将解析的报文转化为 http_request 对象

#include "common.h"
#include "tcp_server.h"
#include "http_request.h"
#include "http_response.h"

typedef int(*request_callback)(struct http_request *httpRequest, struct http_response *httpResponse);

// http_server 本质上就是一个 TCPServer，只不过暴露给应用程序的回调函数更为简单，只需要 http_request 和 http_response 结构
struct http_server
{
	struct TCPserver *tcpServer;
	request_callback requestCallback;
};

struct http_server *http_server_new(struct event_loop *eventLoop, int port, request_callback requestCallback, int threadNum);

void http_server_start(struct http_server *httpServer);

int parse_http_request(struct buffer *input, struct http_request *httpRequest);

#endif