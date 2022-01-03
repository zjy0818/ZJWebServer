#include "buffer.h"

#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

struct response_header 
{
	char *key;
	char *value;
};

enum HttpStatusCode 
{
	Unknown,
	OK = 200,
	MovedPermanently = 301,
	BadRequest = 400,
	NotFound = 404,
};

struct http_response 
{
	enum HttpStatusCode statusCode;
	char *statusMessage;
	char *contentType;
	char *body;
	struct response_header *response_headers;
	int response_headers_number;
	int keep_connected;
};

struct http_response *http_response_new();

//将 http_response 中的数据，根据 HTTP 协议转换为对应的字节流
void http_response_encode_buffer(struct http_response *httpResponse, struct buffer *output);

#endif