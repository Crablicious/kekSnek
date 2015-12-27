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
