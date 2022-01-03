#include <lib/acceptor.h>
#include <lib/http_server.h>
#include "lib/common.h"
#include "lib/event_loop.h"


//���ݶ���buffer֮���callback
//�� parse_http_request ֮�󣬸��ݲ�ͬ�� http_request ����Ϣ�����м���ʹ���
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

	//���� http request �� URL path�������˲�ͬ�� http_response ���͡�
    //���磬������Ϊ��Ŀ¼ʱ�����ص��� 200 �� HTML ��ʽ��
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
	//���߳�event_loop
	struct event_loop *eventLoop = event_loop_init(); 

	//��ʼtcp_server������ָ���߳���Ŀ������߳���0������������߳���acceptor+i/o�������1����һ��I/O�߳�
    //tcp_server�Լ���һ��event_loop
	struct http_server *httpServer = http_server_new(eventLoop, SERV_PORT, onRequest, 2);
	http_server_start(httpServer);

	// main thread for acceptor
	event_loop_run(eventLoop);
}