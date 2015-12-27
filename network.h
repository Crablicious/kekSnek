#ifndef NETWORK_H
#define NETWORK_H
#include <sys/types.h>
#include <netinet/in.h>

int init_socket(int type);

void bind_socket(int sockfd, int port);

#endif
