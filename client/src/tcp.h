#ifndef _TCP_H
#define _TCP_H

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define TCP_MAX_RECV_LEN 8192

typedef int socket_t;

void
tcp_annihilate_socket(int socket);

int
tcp_open(char *ip, int port);

#endif
