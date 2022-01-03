#include "tcp_connection.h"
#include "utils.h"

int handle_connection_closed(struct tcp_connection *tcpConnection)
{
	struct event_loop *eventLoop = tcpConnection->eventLoop;
	struct channel *channel = tcpConnection->channel;
	event_loop_remove_channel_event(eventLoop, channel->fd, channel);

	if (tcpConnection->connectionClosedCallBack != NULL) 
	{
		tcpConnection->connectionClosedCallBack(tcpConnection);
	}
}

//�׽��ֽ������ݣ����� buffer_socket_read �������������׽��ֵ��������������仺�嵽 buffer �����С�
int handle_read(void *data)
{
	struct tcp_connection *tcpConnection = (struct tcp_connection *) data;
	struct buffer *input_buffer = tcpConnection->input_buffer;
	struct channel *channel = tcpConnection->channel;

	if (buffer_append_socket_read(input_buffer, channel->fd) > 0)
	{
		//Ӧ�ó���������ȡBuffer�������
		if (tcpConnection->messageCallBack != NULL)
		{
			tcpConnection->messageCallBack(input_buffer, tcpConnection);
		}
	}
	else
	{
		handle_connection_closed(tcpConnection);
	}
}

//���ͻ�������������д
//��channel��Ӧ��output_buffer�������ⷢ��
int handle_write(void *data)
{
	struct tcp_connection *tcpConnection = (struct tcp_connection *) data;
	struct event_loop *eventLoop = tcpConnection->eventLoop;
	assertInSameThread(eventLoop);

	struct buffer *output_buffer = tcpConnection->output_buffer;
	struct channel *channel = tcpConnection->channel;

	ssize_t nwrited = write(channel->fd, output_buffer->data + output_buffer->readIndex, buffer_readable_size(output_buffer));

	if (nwrited > 0)
	{
		//�Ѷ�nwrited�ֽ�
		output_buffer->readIndex += nwrited;
		//���������ȫ���ͳ�ȥ���Ͳ���Ҫ������
		if (buffer_readable_size(output_buffer) == 0)
		{
			channel_write_event_disable(channel);
		}

		//�ص�writeCompletedCallBack
		if (tcpConnection->writeCompletedCallBack != NULL) 
		{
			tcpConnection->writeCompletedCallBack(tcpConnection);
		}
	}
	else
	{
		printf("handle_write for tcp connection %s\n", tcpConnection->name);
	}
}

struct tcp_connection * tcp_connection_new(int connected_fd, struct event_loop *eventLoop,
	connection_completed_call_back connectionCompletedCallBack,
	connection_closed_call_back connectionClosedCallBack,
	message_call_back messageCallBack, write_completed_call_back writeCompletedCallBack)
{
	struct tcp_connection *tcpConnection = malloc(sizeof(struct tcp_connection));
	tcpConnection->writeCompletedCallBack = writeCompletedCallBack;
	tcpConnection->messageCallBack = messageCallBack;
	tcpConnection->connectionCompletedCallBack = connectionCompletedCallBack;
	tcpConnection->connectionClosedCallBack = connectionClosedCallBack;
	tcpConnection->eventLoop = eventLoop;
	tcpConnection->input_buffer = buffer_new();
	tcpConnection->output_buffer = buffer_new();

	char *buf = malloc(16);
	sprintf(buf, "connection-%d\0", connected_fd);
	tcpConnection->name = buf;

	// add event read for the new connection
	struct channel *channel1 = channel_new(connected_fd, EVENT_READ, handle_read, handle_write, tcpConnection);
	tcpConnection->channel = channel1;

	//��ɶ�connectionCompletedCallBack�ĺ����ص�
	if (tcpConnection->connectionCompletedCallBack != NULL)
	{
		tcpConnection->connectionCompletedCallBack(tcpConnection);
	}

	//�Ѹ��׼��ֶ�Ӧ��channel����ע�ᵽevent_loop�¼��ַ�����
	event_loop_add_channel_event(tcpConnection->eventLoop, connected_fd, tcpConnection->channel);
	return tcpConnection;
}

//Ӧ�ò�������
int tcp_connection_send_data(struct tcp_connection *tcpConnection, void *data, int size)
{
	size_t nwrited = 0;
	size_t nleft = size;
	int fault = 0;

	struct channel *channel = tcpConnection->channel;
	struct buffer *output_buffer = tcpConnection->output_buffer;

	//�����׽��ֳ��Է�������
	// ������ֵ�ǰ channel û��ע�� WRITE �¼������ҵ�ǰ tcp_connection ��Ӧ�ķ��ͻ�����������Ҫ����
	// ��ֱ�ӵ��� write ���������ݷ��ͳ�ȥ
	if (!channel_write_event_is_enabled(channel) && buffer_readable_size(output_buffer) == 0)
	{
		nwrited = write(channel->fd, data, size);
		if (nwrited >= 0)
		{
			nleft = nleft - nwrited;
		}
		else
		{
			nwrited = 0;
			if (errno != EWOULDBLOCK)
			{
				if (errno == EPIPE || errno == ECONNRESET) 
				{
					fault = 1;
				}
			}
		}
	}

	//�����һ�η��Ͳ��꣬�ͽ�ʣ����Ҫ���͵����ݿ�������ǰ tcp_connection ��Ӧ�ķ��ͻ������У�
	//���� event_loop ע�� WRITE �¼����������ݾ��ɿ�ܽӹܣ�Ӧ�ó����ͷ��ⲿ�����ݡ�
	if (!fault && nleft > 0)
	{
		//������Buffer�У�Buffer�������ɿ�ܽӹ�
		buffer_append(output_buffer, data + nwrited, nleft);
		if (!channel_write_event_is_enabled(channel))
		{
			channel_write_event_enable(channel);
		}
	}

	return nwrited;
}

//�� buffer �������ͨ���׽��ֻ��������ͳ�ȥ
int tcp_connection_send_buffer(struct tcp_connection *tcpConnection, struct buffer * buffer)
{
	int size = buffer_readable_size(buffer);
	int result = tcp_connection_send_data(tcpConnection, buffer->data + buffer->readIndex, size);
	buffer->readIndex += size;
	return result;
}

void tcp_connection_shutdown(struct tcp_connection * tcpConnection)
{
	if (shutdown(tcpConnection->channel->fd, SHUT_WR) < 0)
	{
		printf("tcp_connection_shutdown failed, socket == %d\n", tcpConnection->channel->fd);
	}
}
