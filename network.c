#define _BSD_SOURCE

#include <sys/types.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "defs.h"
#include "network.h"


//static struct sockaddr_in player0;
//static struct sockaddr_in player1;

static struct sockaddr_in *players;

void set_num_players(int num_of_players){
  players = calloc((num_of_players+1), sizeof(struct sockaddr_in));
}

void init_player(int num, struct sockaddr_in player){
  players[num].sin_family = player.sin_family;
  players[num].sin_port = player.sin_port;
  players[num].sin_addr.s_addr = player.sin_addr.s_addr;
}

int init_socket(int type){
  int sockfd;
  //Set up socket
  if ((sockfd=socket(AF_INET, type, 0))==-1){
    pexit("Error opening socket");
  }  
  //Make socket reusable 
  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    pexit("setsockopt(SO_REUSEADDR) failed");
  return sockfd;
}

void bind_socket(int sockfd, int port){
  struct sockaddr_in serv_addr;
  socklen_t serv_len = sizeof(struct sockaddr_in);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if((bind(sockfd, (struct sockaddr*)&serv_addr, serv_len)) < 0){
    pexit("Error bind: ");
  }
}

void send_player(int sockfd, int num, char *msg){
  ssize_t count;
  count = sendto(sockfd, msg, strlen(msg)+1, 0, (struct sockaddr*)&players[num], sizeof(struct sockaddr_in));
  if(count < 0){
    pexit("Error sending: ");
  }
}

void send_all(int sockfd, char *msg){
  ssize_t count;  
  for(int i = 0; players[i].sin_port != 0; i++){
    count = sendto(sockfd, msg, strlen(msg)+1, 0, (struct sockaddr*)&players[i], sizeof(struct sockaddr_in));
    if(count < 0){
      pexit("Error sending: ");
    }
  }
}

void free_all_players(){
  free(players);
}
