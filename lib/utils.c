#include "utils.h"

void assertInSameThread(struct event_loop *eventLoop)
{
	if (eventLoop->owner_thread_id != pthread_self())
	{
		printf("not in the same thread\n");
		exit(-1);
	}
}

int isInSameThread(struct event_loop *eventLoop)
{
	return eventLoop->owner_thread_id == pthread_self();
}
