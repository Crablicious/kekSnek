#ifndef NETWORK_H
#define NETWORK_H
#include <sys/types.h>
#include <netinet/in.h>

void set_num_players(int num_of_players);

void init_player(int num, struct sockaddr_in player);

int init_socket(int type);

void bind_socket(int sockfd, int port);

void send_player(int sockfd, int num, char *msg);

void send_all(int sockfd, char *msg);

void free_all_players();

#endif
