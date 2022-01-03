#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

struct request_header 
{
	char *key;
	char *value;
};

enum http_request_state 
{
	REQUEST_STATUS,    //�ȴ�����״̬��
	REQUEST_HEADERS,   //�ȴ�����headers
	REQUEST_BODY,      //�ȴ���������body
	REQUEST_DONE       //�������
};

struct http_request 
{
	char *version;
	char *method;
	char *url;
	enum http_request_state current_state;
	struct request_header *request_headers;
	int request_headers_number;
};

//��ʼ��һ��request����
struct http_request *http_request_new();

//���һ��request����
void http_request_clear(struct http_request *httpRequest);

//����һ��request����
void http_request_reset(struct http_request *httpRequest);

//��request����header
void http_request_add_header(struct http_request *httpRequest, char * key, char * value);

//����keyֵ��ȡheader��Ϥ
char *http_request_get_header(struct http_request *httpRequest, char *key);

//���request�����ĵ�ǰ״̬
enum http_request_state http_request_current_state(struct http_request *httpRequest);

//����request�����ж��Ƿ���Ҫ�رշ�����-->�ͻ��˵�������
int http_request_close_connection(struct http_request *httpRequest);

#endif