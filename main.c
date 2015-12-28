#define _POSIX_C_SOURCE	199309L

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <sys/select.h>


#include "ascii_lib/ascii_lib.h"
#include "defs.h"
#include "network.h"
#include "network_server.h"
#include "obj_list.h"


/**
1 player starts as server. Forks to server and client automatically connects itself to server. 

Server only update position of objects. Server initiates all objects for clients, so they are uniform. 
 */

/**
Threads Server:
1. Receive keystrokes from clients
2. Server sends out new positions of changed objects. Every X/seconds.
 */



void get_players(int sockfd, int num_of_players){
  struct sockaddr_in inc_player;
  socklen_t inc_player_s;
  int init_msg_len = 20;
  char buffer[init_msg_len];
  ssize_t count;
  set_num_players(num_of_players);
  
  for(int i = 0; i < num_of_players; i++){
    count = recvfrom(sockfd, buffer, init_msg_len, 0, (struct sockaddr*)&inc_player, &inc_player_s);
    if(count < 0){
      pexit("Recvfrom error: ");
    }else{
      if(!player_exists(inc_player)){
        puts(buffer);
        init_player(i, inc_player);
      }
      strcpy(buffer, "!");
      count = sendto(sockfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&inc_player, inc_player_s);
      if(count < 0){
        pexit("Sendto error: ");
      }
    }
  }
}

void distribute_field(int sockfd){
  int height = 20;
  int width = 20;
  char backg = '-';
  int num_of_objects = get_num_players()*MAX_LENGTH+MAX_APPLES;
  char *buffer = malloc(100);
  initiate_field(width, height, backg, num_of_objects);
  set_num_obj(num_of_objects);
  sprintf(buffer, "1 %d %d %c %d", width, height, backg, num_of_objects);
  send_all(sockfd, buffer, 1);
  ack_all(sockfd, buffer, 1, 1, 0);
  free(buffer);
  //Field is now distributed to all players. Doesn't deal with disconnected clients atm.
}

void distribute_start_objects(int sockfd){
  int num_players = get_num_players();
  int objID, posx, posy, height, width;
  struct ascii_object tmp_obj;
  struct position *tmp_pos;
  char *tmp_design = malloc(sizeof(MAX_DESIGN_SIZE));
  strcpy(tmp_design, "*");
  height = 1;
  width = 1;
  char *buffer = malloc(MAX_MSG_SIZE);
  for(int i = 0; i < num_players; i++){
    objID = i*MAX_LENGTH;
    posx = i;
    posy = i;
    tmp_obj.pos.x = posx;
    tmp_obj.pos.y = posy;
    tmp_obj.height = height;
    tmp_obj.width = width;
    tmp_obj.twoDimArray = tmp_design;
    tmp_pos = add_object(tmp_obj);
    set_obj(objID, tmp_pos);
    sprintf(buffer, "2 %d %d %d %d %d %s", objID, posx, posy, height, width, tmp_design); 
    //Can collect multiple object up to MAX_MSG_SIZE in one package.
    send_all(sockfd, buffer, 0);
    ack_all(sockfd, buffer, 0, 2, objID);
    //Create snek and bläst it out.
    //Need to keep track of the linked snake.
  }
  free(tmp_design);
  free(buffer);
}

int initiate_game(){
  int sockfd = init_socket(SOCK_DGRAM);
  bind_socket(sockfd, SERVER_PORT);
  get_players(sockfd, MAX_PLAYERS);
  distribute_field(sockfd);
  distribute_start_objects(sockfd); //Should discard all 1's.
  return sockfd;
}

void move_object(int sockfd, int objID, int posx, int posy){
  char *buffer = malloc(MAX_MSG_SIZE);
  sprintf(buffer, "3 %d %d %d", objID, posx, posy);
  send_all(sockfd, buffer, 0);
}

/*
void collect_char(int sockfd, char *buffer, char *latest_char, int sec, int nsec){
  ssize_t count;
  
  count = read(sockfd, buffer, MAX_MSG_SIZE);
  
}*/



void game_loop(int sockfd){
  char *buffer = malloc(MAX_MSG_SIZE);
  int isRunning = 1;
  char *latest_char = calloc(MAX_PLAYERS, sizeof(char));
  ssize_t count;
  
  int selectVal, tmp_ID;
  char tmp_c;
  struct timeval timeout;
  fd_set fds;
  
  //long tot_nsec = NSEC_FRAME + SEC_FRAME*1000000000;
  long sec = SEC_FRAME;
  long nsec = NSEC_FRAME; //tot_nsec*4/5%1000000000;
  
  struct timespec end_time;
  struct timespec curr_time;
  while(isRunning){
    puts("in game loop");
    clock_gettime(CLOCK_REALTIME, &end_time);
    end_time.tv_sec += sec;
    end_time.tv_nsec += nsec;
    
    timeout.tv_sec = SEC_READ_TO;
    timeout.tv_usec = USEC_READ_TO;  
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);  
    
    selectVal = select(sockfd + 1, &fds, NULL, NULL, &timeout);
    if(selectVal == -1){
      pexit("Error select");
    }else if(selectVal == 1){
      count = recv(sockfd, buffer, MAX_MSG_SIZE, 0);
      if(count < 0){
        pexit("error recv game_loop");
      }
      sscanf(buffer, "%c %d", &tmp_c, &tmp_ID);
      latest_char[tmp_ID] = tmp_c;
    }
    clock_gettime(CLOCK_REALTIME, &curr_time);
    if((curr_time.tv_sec > end_time.tv_sec) || (curr_time.tv_sec == end_time.tv_sec && curr_time.tv_nsec >= end_time.tv_nsec)){
      //process_inputs();
      printf("CHARACTER IS: %c\n",latest_char[0]);
      isRunning = 0;
    }
  }
}

int main(int argc, char *argv[])
{
  int is_server = 0;
  pid_t pid = 0;
  static char address[20];
  if(argc == 2 && strcmp(argv[1], "1") == 0){
    is_server = 1;
    strcpy(address, "127.0.0.1");
  }else if(argc == 2){
    strcpy(address, argv[1]);
  }else{
    puts("Wrong number of arguments");
    exit(EXIT_FAILURE);
  }
  if(is_server){
    if((pid = fork()) < 0){
      pexit("Error creating child: ");
    }
  }
  if(pid){ //server
    int sockfd = initiate_game();
    game_loop(sockfd);
    
    wait(NULL); //Wait for Client to terminate.
  }else{ //client
    static char *argvc[]={"client", address, NULL};
    execv("./client", argvc);
    pexit("Error execv: ");
  }
  return 0;
}
    

