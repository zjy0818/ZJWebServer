#ifndef YOLANDA_COMMON_H
#define YOLANDA_COMMON_H

#include "tcp_server.h"
#include "channel_map.h"
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/time.h>
#include <time.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sysctl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/epoll.h>

int tcp_server(int port);

int tcp_server_listen(int port);

int tcp_nonblocking_server_listen(int port);

void make_nonblocking(int fd);

int tcp_client(char *address, int port);

#define    SERV_PORT      4869
#define    MAXLINE        4096
#define    UNIXSTR_PATH   "/var/lib/unixstream.sock"
#define    LISTENQ        1024
#define    BUFFER_SIZE    4096

#endif //YOLANDA_COMMON_H