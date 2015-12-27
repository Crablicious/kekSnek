#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H
#include <sys/types.h>
#include <netinet/in.h>

int get_num_players();

void set_num_players(int num_of_players);

void init_player(int num, struct sockaddr_in player);

void send_player(int sockfd, int num, char *msg, int catID);

void send_all(int sockfd, char *msg, int catID);

void ack_player(int sockfd, int num, int ack_num);

void ack_all(int sockfd, char *resend_msg);

void free_all_players();

#endif
