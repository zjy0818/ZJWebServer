#include <assert.h>
#include "http_server.h"

//���ӽ���֮���callback
int http_onConnectionCompleted(struct tcp_connection *tcpConnection)
{
	printf("connection completed\n");
	struct http_request *httpRequest = http_request_new();
	tcpConnection->request = httpRequest;
	return 0;
}

//���������С�ʾ����GET /index.html HTTP/1.1
int process_status_line(char *start, char *end, struct http_request *httpRequest)
{
	int size = end - start;
	//method
	char *space = memmem(start, end - start, " ", 1);
	assert(space != NULL);
	int method_size = space - start;
	httpRequest->method = malloc(method_size + 1);
	strncpy(httpRequest->method, start, space - start);
	httpRequest->method[method_size + 1] = '\0';

	//url
	start = space + 1;
	space = memmem(start, end - start, " ", 1);
	assert(space != NULL);
	int url_size = space - start;
	httpRequest->url = malloc(url_size + 1);
	strncpy(httpRequest->url, start, space - start);
	httpRequest->url[url_size + 1] = '\0';

	//version
	start = space + 1;
	httpRequest->version = malloc(end - start + 1);
	strncpy(httpRequest->version, start, end - start);
	httpRequest->version[end - start + 1] = '\0';
	assert(space != NULL);
	return size;
}

/*
* ����http����
* Ѱ�ұ��ĵı߽磬ͬʱ��¼�µ�ǰ��������������״̬��
* ���ݽ���������ǰ��˳�򣬰ѱ��Ľ����Ĺ����ֳ� 
* REQUEST_STATUS��REQUEST_HEADERS��REQUEST_BODY �� REQUEST_DONE �ĸ��׶Σ�
* ÿ���׶ν����ķ������в�ͬ.
* �ڽ���״̬��ʱ����ͨ����λ CRLF �س����з���λ����Ȧ��״̬�У�����״̬�н���ʱ���ٴ�ͨ�����ҿո��ַ�����Ϊ�ָ��߽�
* �ڽ���ͷ������ʱ��Ҳ����ͨ����λ CRLF �س����з���λ����Ȧ��һ�� key-value �ԣ���ͨ������ð���ַ�����Ϊ�ָ��߽硣
* ������û���ҵ�ð���ַ���˵������ͷ���Ĺ������
*/
int parse_http_request(struct buffer *input, struct http_request *httpRequest)
{
	int ok = 1;
	while (httpRequest->current_state != REQUEST_DONE)
	{
		if (httpRequest->current_state == REQUEST_STATUS)
		{
			char *crlf = buffer_find_CRLF(input);
			if (crlf)
			{
				int request_line_size = process_status_line(input->data + input->readIndex, crlf, httpRequest);
				if (request_line_size)
				{
					input->readIndex += request_line_size;  // request line size
					input->readIndex += 2;  //CRLF size
					httpRequest->current_state = REQUEST_HEADERS;
				}
			}
		}
		else if (httpRequest->current_state == REQUEST_HEADERS)
		{
			char *crlf = buffer_find_CRLF(input);
			if (crlf)
			{
				/*  �������ĸ����ײ��ֶ�
				 *    <start>-------<colon>:-------<crlf>
				 */
				char *start = input->data + input->readIndex;
				int request_line_size = crlf - start;
				char *colon = memmem(start, request_line_size, ": ", 2);
				if (colon != NULL)
				{
					char *key = malloc(colon - start + 1);
					strncpy(key, start, colon - start);
					key[colon - start] = '\0';
					char *value = malloc(crlf - colon - 2 + 1);
					strncpy(value, colon + 2, crlf - colon - 2);
					value[crlf - colon - 2] = '\0';

					http_request_add_header(httpRequest, key, value);

					input->readIndex += request_line_size;  //request line size
					input->readIndex += 2;  //CRLF size
				}
				else
				{
					//��������˵��:û�ҵ�����˵����������һ��
					input->readIndex += 2;  //CRLF size
					httpRequest->current_state = REQUEST_DONE;
				}
			}
		}
	}

	return ok;
}

// ��ɱ��ĵĽ������������ı���ת��Ϊ http_request ����
// buffer�ǿ�ܹ����õģ������Ѿ��յ��������ݵ������
// ע���������û���յ�ȫ�����ݣ�����Ҫ�������ݲ���������
int http_onMessage(struct buffer *input, struct tcp_connection *tcpConnection)
{
	printf("get message from tcp connection %s\n", tcpConnection->name);

	struct http_request *httpRequest = (struct http_request *) tcpConnection->request;
	struct http_server *httpServer = (struct http_server *) tcpConnection->data;

	if (parse_http_request(input, httpRequest) == 0)
	{
		char *error_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
		tcp_connection_send_data(tcpConnection, error_response, sizeof(error_response));
		tcp_connection_shutdown(tcpConnection);
	}

	//�����������е�request���ݣ����������б���ͷ���
	if (http_request_current_state(httpRequest) == REQUEST_DONE)
	{
		struct http_response *httpResponse = http_response_new();

		//httpServer��¶��requestCallback�ص�
		if (httpServer->requestCallback != NULL)
		{
			httpServer->requestCallback(httpRequest, httpResponse);
		}
		struct buffer *buffer = buffer_new();
		http_response_encode_buffer(httpResponse, buffer);
		tcp_connection_send_buffer(tcpConnection, buffer);

		if (http_request_close_connection(httpRequest)) 
		{
			tcp_connection_shutdown(tcpConnection);
		}

		http_request_reset(httpRequest);
	}
}

//����ͨ��bufferд��֮���callback
int http_onWriteCompleted(struct tcp_connection *tcpConnection)
{
	printf("write completed\n");
	return 0;
}

//���ӹر�֮���callback
int http_onConnectionClosed(struct tcp_connection *tcpConnection)
{
	printf("connection closed\n");
	if (tcpConnection->request != NULL) 
	{
		http_request_clear(tcpConnection->request);
		tcpConnection->request = NULL;
	}
	return 0;
}

struct http_server *http_server_new(struct event_loop *eventLoop, int port, request_callback requestCallback, int threadNum)
{
	struct http_server *httpServer = malloc(sizeof(struct http_server));
	httpServer->requestCallback = requestCallback;
	//��ʼ��acceptor
	struct acceptor *acceptor = acceptor_init(SERV_PORT);

	httpServer->tcpServer = tcp_server_init(eventLoop, acceptor, http_onConnectionCompleted, http_onMessage,
		http_onWriteCompleted, http_onConnectionClosed, threadNum);

	// for callback
	httpServer->tcpServer->data = httpServer;

	return httpServer;
}

void http_server_start(struct http_server *httpServer)
{
	tcp_server_start(httpServer->tcpServer);
}
