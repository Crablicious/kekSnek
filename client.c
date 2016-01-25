#define _POSIX_C_SOURCE	199309L
#define _BSD_SOURCE

#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ascii_lib/ascii_lib.h"
#include "network.h"
#include "defs.h"
#include "obj_list.h"
#include "kb_hit.h"


/**
Threads Client:
1. Regularly Drawing Graphics, keeping track of objects.   
2. Keyboard listener and sends it to server.
3. Receives changes in object's pos from server.
 */

const char VIABLE_INP[] = "wasdq";
int isRunning;
int myID;

void *server_talker_thread(char *address){
  int sockfd = init_socket(SOCK_DGRAM);
  bind_socket(sockfd, CLIENT_PORT);
  char *buffer = malloc(MAX_MSG_SIZE);
  char designbuffer[MAX_DESIGN_SIZE];
  ssize_t count;
  int width, height, num_of_objects, playerID, posx, posy, objID;
  int isInit = 0;
  struct sockaddr_in serv_addr;
  socklen_t serv_addr_s;
  char  backg;
  struct ascii_object tmp_obj;
  struct position *tmp_pos;
  
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVER_PORT);
  inet_aton(address, &serv_addr.sin_addr);
  serv_addr_s = sizeof(struct sockaddr);
  int selectVal;  
  struct timeval timeout;
  fd_set fds;
  strcpy(buffer, "!");
  int got_ack = 0;
  while(!got_ack && isRunning){ 
    count = sendto(sockfd, buffer, strlen(buffer)+1, 0, (struct sockaddr *)&serv_addr, serv_addr_s);
    if(count < 0){
      pexit("Error sendto ACK: ");
    }
    timeout.tv_sec = ACK_TO;
    timeout.tv_usec = 0;  
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);  
    
    selectVal = select(sockfd + 1, &fds, NULL, NULL, &timeout);
    if(selectVal == -1){
      pexit("Error select");
    }else if(selectVal == 1){
      strcpy(buffer, "!");
      count = recv(sockfd, buffer, MAX_MSG_SIZE, 0);
      if(count < 0){
        pexit("Error recv: ");
      }
      got_ack = 1;
    }
  }
  
  //Receive info from server
  while(isRunning){
    timeout.tv_sec = SERV_TALK_TO;
    timeout.tv_usec = 0;  
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);  
    
    selectVal = select(sockfd + 1, &fds, NULL, NULL, &timeout);
    if(selectVal == -1){
      pexit("Error select");
    }else if(selectVal == 1){
      count = recvfrom(sockfd, buffer, MAX_MSG_SIZE, 0, (struct sockaddr*)&serv_addr, &serv_addr_s);
      switch (buffer[0]) {
      case '1':
        if(!isInit){
          sscanf(buffer, "1 %d %d %c %d %d", &width, &height, &backg, &num_of_objects, &playerID);
          if(backg == 'W'){
            backg = ' ';
          }
          initiate_field(width, height, backg, num_of_objects);
          set_num_obj(num_of_objects);
          myID = playerID;
          isInit = 1;
        }
        sprintf(buffer, "1 %d", myID);
        count = sendto(sockfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&serv_addr, serv_addr_s);
        if(count < 0){
          pexit("Error sendto: ");
        }
        break;
      case '2':
        sscanf(buffer, "2 %d %d %d %d %d %s", &objID, &posx, &posy, &height, &width, designbuffer);
        
        if(!obj_exists(objID)){

          tmp_obj.pos.x = posx;
          tmp_obj.pos.y = posy;
          tmp_obj.height = height;
          tmp_obj.width = width;
          tmp_obj.twoDimArray = designbuffer;
      
          tmp_pos = add_object(tmp_obj);
          set_obj(objID, tmp_pos);
        }
        //TODO: Make it able to receive multiple changes in one packet.
        sprintf(buffer, "2 %d %d", myID, objID); 
        count = sendto(sockfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&serv_addr, serv_addr_s);
        if(count < 0){
          pexit("Error sendto: ");
        }
        break;
      case '3':
        sscanf(buffer, "3 %d %d %d", &objID, &posx, &posy);
        set_pos(objID, posx, posy);
        break;
      default:
        printf("Unknown message identifier\n");
      }
    }
  }
  return NULL;
}

void *graphics_thread(void){
  struct timespec frame_sleep;
  frame_sleep.tv_sec = SEC_FRAME;
  frame_sleep.tv_nsec = NSEC_FRAME;  
  while(myID == -1 && isRunning) nanosleep(&frame_sleep, NULL); //spin until field init.
  while(isRunning){
    draw_screen();
    if(nanosleep(&frame_sleep, NULL) < 0){
      pexit("Error sleeping: ");
    }
  }
  return NULL;
}

void *keyboard_thread(char *address){
  int c, sockfd = init_socket(SOCK_DGRAM);
  ssize_t count = 1;
  char *buffer = malloc(2);
  struct sockaddr_in serv_addr;
  socklen_t serv_addr_s;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVER_PORT);
  inet_aton(address, &serv_addr.sin_addr);
  serv_addr_s = sizeof(struct sockaddr_in);
  
  struct timespec samp_rate;  //sleep(10);

  samp_rate.tv_sec = SEC_FRAME/4;
  samp_rate.tv_nsec = NSEC_FRAME/4;  
  init_keyboard();
  while(isRunning){
    
    while(!kbhit());
    c = readch();
    if(strchr(VIABLE_INP, c)){
      //printf("%d Key read: %c\n" ,myID,c);
      sprintf(buffer, "%c %d", c, myID);
      count = sendto(sockfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&serv_addr, serv_addr_s);
      if(count < 0){
        pexit("Error sendto keyboard_thread");
      }
      nanosleep(&samp_rate, NULL);
    }
    if(c == 'q'){
      isRunning = 0;
    }
    c = 0;
  }
  free(buffer);
  close_keyboard();
  return NULL;
}

int main(int argc, char *argv[])
{
  myID = -1;
  isRunning = 1;
  char *address = malloc(sizeof(char)*20);
  strcpy(address, argv[1]);
  pthread_t graphics_t, server_talker_t, keyboard_t;
  int r_graphics_t, r_server_talker_t, r_keyboard_t;
  
  /* thread for server_talker */
  r_server_talker_t = pthread_create( &server_talker_t, NULL, (void *) server_talker_thread, address);
  if(r_server_talker_t) {
    fprintf(stderr,"Error - pthread_create() return code: %d\n",r_server_talker_t);
    exit(EXIT_FAILURE);
  }
  
  /* thread for graphics */
  r_graphics_t = pthread_create( &graphics_t, NULL, (void *) graphics_thread, NULL);
  if(r_graphics_t) {
    fprintf(stderr,"Error - pthread_create() return code: %d\n",r_graphics_t);
    exit(EXIT_FAILURE);
  }
  
  /* thread for kbhit */
  r_keyboard_t = pthread_create( &keyboard_t, NULL, (void *) keyboard_thread, address);
  if(r_keyboard_t) {
    fprintf(stderr,"Error - pthread_create() return code: %d\n",r_keyboard_t);
    exit(EXIT_FAILURE);
  }
  puts("bef joining");
  pthread_join(server_talker_t, NULL);
  puts("joining");
  pthread_join(graphics_t, NULL);
  puts("joining");
  pthread_join(keyboard_t, NULL);
  puts("joining");
  free(address);
  return 0;
}
