#ifndef BUFFER_H
#define BUFFER_H

#define INIT_BUFFER_SIZE 65536
//���ݻ�����
struct buffer 
{
	char *data;          //ʵ�ʻ���
	int readIndex;       //�����ȡλ��
	int writeIndex;      //����д��λ��
	int total_size;      //�ܴ�С
};

struct buffer *buffer_new();

void buffer_free(struct buffer *buffer1);

int buffer_writeable_size(struct buffer *buffer);

int buffer_readable_size(struct buffer *buffer);

//����������
int buffer_front_spare_size(struct buffer *buffer);

//��buffer��д����
int buffer_append(struct buffer *buffer, void *data, int size);

//��buffer��д����
int buffer_append_char(struct buffer *buffer, char data);

//��buffer��д����
int buffer_append_string(struct buffer*buffer, char * data);

//��socket���ݣ���buffer��д
int buffer_append_socket_read(struct buffer *buffer, int fd);

//��buffer����
char buffer_read_char(struct buffer *buffer);

//��ѯbuffer����
char * buffer_find_CRLF(struct buffer * buffer);

void make_room(struct buffer *buffer, int size);

#endif