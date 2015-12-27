#define _POSIX_C_SOURCE	199309L
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


#include "ascii_lib/ascii_lib.h"
#include "network.h"
#include "defs.h"
#include "obj_list.h"


/**
Threads Client:
1. Regularly Drawing Graphics, keeping track of objects.   
2. Keyboard listener and sends it to server.
3. Receives changes in object's pos from server.
 */

int isRunning;
int myID;


void *server_talker_thread(void){
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
  while(isRunning){
    count = recvfrom(sockfd, buffer, MAX_MSG_SIZE, 0, (struct sockaddr*)&serv_addr, &serv_addr_s);
    switch (buffer[0]) {
    case '1':
      if(!isInit){
        sscanf(buffer, "1 %d %d %c %d %d", &width, &height, &backg, &num_of_objects, &playerID);
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
  return NULL;
}

void *graphics_thread(void){
  struct timespec frame_sleep;
  frame_sleep.tv_sec = 0;
  frame_sleep.tv_nsec = 1/10*pow(10,9);  
  while(myID == -1) nanosleep(&frame_sleep, NULL); //spin until field init.
  while(isRunning){
    draw_screen();
    if(nanosleep(&frame_sleep, NULL) < 0){
      pexit("Error sleeping: ");
    }
  }
  return NULL;
}


int main(int argc, char *argv[])
{
  myID = -1;
  isRunning = 1;
  char address[20];
  strcpy(address, argv[1]);
  
  pthread_t graphics_t, server_talker_t, keyboard_t;
  int r_graphics_t, r_server_talker_t, r_keyboard_t;
  
  /* thread for server_talker */
  r_server_talker_t = pthread_create( &server_talker_t, NULL, (void *) server_talker_thread, NULL);
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
  
  pthread_join(server_talker_t, NULL);
  pthread_join(graphics_t, NULL);
  return 0;
}
