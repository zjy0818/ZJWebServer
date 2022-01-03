#include <lib/acceptor.h>
#include <lib/http_server.h>
#include "lib/common.h"
#include "lib/event_loop.h"


//数据读到buffer之后的callback
//在 parse_http_request 之后，根据不同的 http_request 的信息，进行计算和处理
int onRequest(struct http_request *httpRequest, struct http_response *httpResponse) 
{
    char *url = httpRequest->url;
    char *question = memmem(url, strlen(url), "?", 1);
    char *path = NULL;
    if (question != NULL) 
	{
        path = malloc(question - url);
        strncpy(path, url, question - url);
    } 
	else 
	{
        path = malloc(strlen(url));
        strncpy(path, url, strlen(url));
    }

	//根据 http request 的 URL path，返回了不同的 http_response 类型。
    //比如，当请求为根目录时，返回的是 200 和 HTML 格式。
    if (strcmp(path, "/") == 0) 
	{
        httpResponse->statusCode = OK;
        httpResponse->statusMessage = "OK";
        httpResponse->contentType = "text/html";
        httpResponse->body = "<html><head><title>This is network programming</title></head><body><h1>Hello, network programming</h1></body></html>\r\n";
    } 
	else if (strcmp(path, "/network") == 0) 
	{
        httpResponse->statusCode = OK;
        httpResponse->statusMessage = "OK";
        httpResponse->contentType = "text/plain";
        httpResponse->body = "hello, network programming";
    } 
	else 
	{
        httpResponse->statusCode = NotFound;
        httpResponse->statusMessage = "Not Found";
        httpResponse->keep_connected = 1;
    }

    return 0;
}


int main(int c, char **v) 
{
	//主线程event_loop
	struct event_loop *eventLoop = event_loop_init(); 

	//初始tcp_server，可以指定线程数目，如果线程是0，就是在这个线程里acceptor+i/o；如果是1，有一个I/O线程
    //tcp_server自己带一个event_loop
	struct http_server *httpServer = http_server_new(eventLoop, SERV_PORT, onRequest, 2);
	http_server_start(httpServer);

	// main thread for acceptor
	event_loop_run(eventLoop);
}