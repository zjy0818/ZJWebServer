#ifndef ACCEPTOR_H
#define ACCEPTOR_H

//acceptor �����ʾ���Ƿ������˼�������acceptor �������ջ���Ϊһ�� channel ����ע�ᵽ event_loop �ϣ�
//�Ա����������ɵ��¼��ַ��ͼ��

#include "common.h"

struct acceptor 
{
	int listen_port;
	int listen_fd;
};

struct acceptor *acceptor_init(int port);

#endif