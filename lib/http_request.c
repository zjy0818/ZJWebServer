#include "common.h"
#include "http_request.h"

#define INIT_REQUEST_HEADER_SIZE 128

const char *HTTP10 = "HTTP/1.0";
const char *HTTP11 = "HTTP/1.1";
const char *KEEP_ALIVE = "Keep-Alive";
const char *CLOSE = "close";

//��ʼ��һ��request����
struct http_request *http_request_new()
{
	struct http_request *httpRequest = malloc(sizeof(struct http_request));
	httpRequest->method = NULL;
	httpRequest->current_state = REQUEST_STATUS;
	httpRequest->version = NULL;
	httpRequest->url = NULL;
	httpRequest->request_headers = malloc(sizeof(struct http_request) * INIT_REQUEST_HEADER_SIZE);
	httpRequest->request_headers_number = 0;
	return httpRequest;

}

//���һ��request����
void http_request_clear(struct http_request *httpRequest)
{
	if (httpRequest->request_headers != NULL)
	{
		for (int i = 0; i < httpRequest->request_headers_number; i++)
		{
			free(httpRequest->request_headers[i].key);
			free(httpRequest->request_headers[i].value);
		}
		free(httpRequest->request_headers);
	}
	free(httpRequest);
}

//����һ��request����
void http_request_reset(struct http_request *httpRequest)
{
	httpRequest->method = NULL;
	httpRequest->current_state = REQUEST_STATUS;
	httpRequest->version = NULL;
	httpRequest->url = NULL;
	httpRequest->request_headers_number = 0;
}

//��request����header
void http_request_add_header(struct http_request *httpRequest, char * key, char * value)
{
	httpRequest->request_headers[httpRequest->request_headers_number].key = key;
	httpRequest->request_headers[httpRequest->request_headers_number].value = value;
	httpRequest->request_headers_number++;
}

//����keyֵ��ȡheader����
char *http_request_get_header(struct http_request *httpRequest, char *key)
{
	if (httpRequest->request_headers != NULL)
	{
		for (int i = 0; i < httpRequest->request_headers_number; i++)
		{
			if (strncmp(httpRequest->request_headers[i].key, key, strlen(key)) == 0) 
			{
				return httpRequest->request_headers[i].value;
			}
		}
	}

	return NULL;
}

//���request�����ĵ�ǰ״̬
enum http_request_state http_request_current_state(struct http_request *httpRequest)
{
	return httpRequest->current_state;
}

//����request�����ж��Ƿ���Ҫ�رշ�����-->�ͻ��˵�������
int http_request_close_connection(struct http_request *httpRequest)
{
	char *connection = http_request_get_header(httpRequest, "Connection");

	if (connection != NULL && strncmp(connection, CLOSE, strlen(CLOSE)) == 0) 
	{
		return 1;
	}

	if (httpRequest->version != NULL && strncmp(httpRequest->version, HTTP10, strlen(HTTP10)) == 0 &&
		strncmp(connection, KEEP_ALIVE, strlen(KEEP_ALIVE)) == 1) 
	{
		return 1;
	}
	return 0;
}
