#include <assert.h>
#include "common.h"
#include "tcp_server.h"
#include "thread_pool.h"
#include "utils.h"
#include "tcp_connection.h"

struct TCPserver *
	tcp_server_init(struct event_loop *eventLoop, struct acceptor *acceptor,
		connection_completed_call_back connectionCompletedCallBack,
		message_call_back messageCallBack,
		write_completed_call_back writeCompletedCallBack,
		connection_closed_call_back connectionClosedCallBack,
		int threadNum)
{
	struct TCPserver *tcpServer = malloc(sizeof(struct TCPserver));
	tcpServer->eventLoop = eventLoop;
	tcpServer->acceptor = acceptor;
	tcpServer->connectionCompletedCallBack = connectionCompletedCallBack;
	tcpServer->messageCallBack = messageCallBack;
	tcpServer->writeCompletedCallBack = writeCompletedCallBack;
	tcpServer->connectionClosedCallBack = connectionClosedCallBack;
	tcpServer->threadNum = threadNum;
	tcpServer->threadPool = thread_pool_new(eventLoop, threadNum);
	tcpServer->data = NULL;

	return tcpServer;
}

//���������ѽ����Ļص�����
int handle_connection_established(void *data)
{
	struct TCPserver *tcpServer = (struct TCPserver *) data;
	struct acceptor *acceptor = tcpServer->acceptor;
	int listenfd = acceptor->listen_fd;

	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	//��ȡ����ѽ������׼��֣�����Ϊ�������׼���
	int connected_fd = accept(listenfd, (struct sockaddr *) &client_addr, &client_len);
	make_nonblocking(connected_fd);

	printf("new connection established, socket == %d\n", connected_fd);

	//���̳߳���ѡ��һ��eventloop����������µ������׽���
	struct event_loop *eventLoop = thread_pool_get_loop(tcpServer->threadPool);

	// Ϊ����½����׽��ִ���һ��tcp_connection���󣬲���Ӧ�ó����callback�������ø����tcp_connection����
	struct tcp_connection *tcpConnection = tcp_connection_new(connected_fd, eventLoop, 
		tcpServer->connectionCompletedCallBack, 
		tcpServer->connectionClosedCallBack, 
		tcpServer->messageCallBack, tcpServer->writeCompletedCallBack);

	//callback�ڲ�ʹ��
	if (tcpServer->data != NULL)
	{
		tcpConnection->data = tcpServer->data;
	}
	return 0;
}

//��������
void tcp_server_start(struct TCPserver *tcpServer)
{
	struct acceptor *acceptor = tcpServer->acceptor;
	struct event_loop *eventLoop = tcpServer->eventLoop;

	//��������߳�
	thread_pool_start(tcpServer->threadPool);

	//acceptor���̣߳� ͬʱ��tcpServer��Ϊ��������channel����
	struct channel *channel = channel_new(acceptor->listen_fd, EVENT_READ, handle_connection_established, NULL, tcpServer);

	event_loop_add_channel_event(eventLoop, channel->fd, channel);

	return;
}

//����callback����
void tcp_server_set_data(struct TCPserver *tcpServer, void * data)
{
	if (data != NULL) 
	{
		tcpServer->data = data;
	}
}

int tcp_server(int port)
{
	int listenfd;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	int on = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	int rt1 = bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (rt1 < 0)
	{
		printf("bind failed\n ");
	}

	int rt2 = listen(listenfd, LISTENQ);

	if (rt2 < 0)
	{
		printf("listen failed\n ");
	}

	signal(SIGPIPE, SIG_IGN);

	int connfd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	if ((connfd = accept(listenfd, (struct sockaddr *) &client_addr, &client_len)) < 0) 
	{
		printf("accept failed\n ");
	}

	return connfd;
}

int tcp_server_listen(int port)
{
	int listenfd;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	int on = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	int rt1 = bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (rt1 < 0)
	{
		printf("bind failed\n ");
	}

	int rt2 = listen(listenfd, LISTENQ);

	if (rt2 < 0)
	{
		printf("listen failed\n ");
	}

	signal(SIGPIPE, SIG_IGN);

	return listenfd;
}

int tcp_nonblocking_server_listen(int port)
{
	int listenfd;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	make_nonblocking(listenfd);

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	int on = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	int rt1 = bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (rt1 < 0)
	{
		printf("bind failed\n ");
	}

	int rt2 = listen(listenfd, LISTENQ);

	if (rt2 < 0)
	{
		printf("listen failed\n ");
	}

	signal(SIGPIPE, SIG_IGN);

	return listenfd;
}



void make_nonblocking(int fd)
{
	fcntl(fd, F_SETFL, O_NONBLOCK);
}
	