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
#include "snake_list.h"


/**
1 player starts as server. Forks to server and client automatically connects itself to server. 

Server only update position of objects. Server initiates all objects for clients, so they are uniform. 
 */

/**
Threads Server:
1. Receive keystrokes from clients
2. Server sends out new positions of changed objects. Every X/seconds.
 */

const char VIABLE_INP[] = "wasdq";

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
      if(buffer[0] == '!'){
        if(!player_exists(inc_player)){
          puts(buffer);
          init_player(i, inc_player);
        }
        strcpy(buffer, "!");
        count = sendto(sockfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&inc_player, inc_player_s);
        if(count < 0){
          pexit("Sendto error: ");
        }
      }else{
        i--;
      }
    }
  }
}

void distribute_field(int sockfd){
  int height = FIELD_HEIGHT;
  int width = FIELD_WIDTH;
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


struct position *create_object(char *design, int posx, int posy){
  struct ascii_object tmp_obj;
  tmp_obj.pos.x = posx;
  tmp_obj.pos.y = posy;
  tmp_obj.height = 1;
  tmp_obj.width = 1;
  tmp_obj.twoDimArray = design;
  return add_object(tmp_obj);
}


void distribute_start_objects(int sockfd){
  int num_players = get_num_players();
  int objID, apple_id, posx, posy, height, width;
  struct position *tmp_pos;
  char *tmp_design = malloc(sizeof(MAX_DESIGN_SIZE));
  char *tmp_design_apple = malloc(sizeof(MAX_DESIGN_SIZE));
  strcpy(tmp_design, "*");
  strcpy(tmp_design_apple, "&");
  height = 1;
  width = 1;
  char *buffer = malloc(MAX_MSG_SIZE);
  for(int i = 0; i < num_players; i++){ 
    for(int j = 0; j < MAX_LENGTH; j++){ //Send all snake links.
      objID = i*MAX_LENGTH+j;
      if(j){
        posx = -1; //Make snake object hidden
        posy = -1;
      }else{
        posx = i;
        posy = i;
      }
      tmp_pos = create_object(tmp_design, posx, posy);
      set_obj(objID, tmp_pos);
      if(!j){
        append_first(i, objID);
      }
      sprintf(buffer, "2 %d %d %d %d %d %s", objID, posx, posy, height, width, tmp_design); 
      //Can collect multiple object up to MAX_MSG_SIZE in one package.
      send_all(sockfd, buffer, 0); //TODO: Collect all these together.
      ack_all(sockfd, buffer, 0, 2, objID);
      //Create snek and bläst it out.
    }
  }
  //Send all apples.
  posx = -1;
  posy = -1;
    
  for(int i = 0; i < MAX_APPLES; i++){
    apple_id = MAX_LENGTH*num_players+i;
    tmp_pos = create_object(tmp_design_apple, posx, posy); 
    set_obj(apple_id, tmp_pos); 

    sprintf(buffer, "2 %d %d %d %d %d %s", apple_id, posx, posy, height, width, tmp_design_apple);  
    send_all(sockfd, buffer, 0); //TODO: Collect all these together. 
    ack_all(sockfd, buffer, 0, 2, apple_id); 
  }
  
  free(tmp_design_apple);
  free(tmp_design);
  free(buffer);
}

int initiate_game(){ //Error  here somewhere
  int sockfd = init_socket(SOCK_DGRAM);
  bind_socket(sockfd, SERVER_PORT);
  printf("before get_players\n");
  get_players(sockfd, MAX_PLAYERS);
  printf("Before distribute field\n");
  distribute_field(sockfd);
  printf("After distribute field\n");
  distribute_start_objects(sockfd); //Should discard all 1's.
  printf("after distr objects");
  return sockfd;
}

void move_object(int sockfd, int objID, int posx, int posy){
  char *buffer = malloc(MAX_MSG_SIZE);
  sprintf(buffer, "3 %d %d %d", objID, posx, posy);
  send_all(sockfd, buffer, 0);
  free(buffer);
}


/*
Linked snake list.
Info required:
- objID
- *pos
- *nextLink


Check if head is in apple.
 - If so, create new piece as head in new position.
 - Else, move back object to the front.

Creating obj req:
- Highest ID
- position 
- height = 1
- width = 1
- design = '*'
 */

void process_inputs(int sockfd, char *latest_char){
  int *posx, *posy;
  int x_offset = 0, y_offset = 0;
  int headID;
  int appendFlag = 0;
  char *buffer = malloc(MAX_MSG_SIZE);
  int num_players = get_num_players();
  posx = malloc(sizeof(int));
  posy = malloc(sizeof(int));
  for(int i = 0; i < num_players; i++){
    if(latest_char[i] != 0){
      switch (latest_char[i]) {
      case 'w':
        if(latest_char[num_players+i] != 's'){
          y_offset = -1;
          x_offset = 0;
          latest_char[num_players+i] = 'w';
        }else{
          y_offset = 1;
          x_offset = 0;
        }
        break;
      case 'a':
        if(latest_char[num_players+i] != 'd'){
          y_offset = 0;
          x_offset = -1;
          latest_char[num_players+i] = 'a';
        }else{
          y_offset = 0;
          x_offset = 1;
        }
        break;
      case 's':
        if(latest_char[num_players+i] != 'w'){
          y_offset = 1;
          x_offset = 0;
          latest_char[num_players+i] = 's';
        }else{
          y_offset = -1;
          x_offset = 0;
        }
        break;
      case 'd':        
        if(latest_char[num_players+i] != 'a'){
          y_offset = 0;
          x_offset = 1;
          latest_char[num_players+i] = 'd';
        }else{
          y_offset = 0;
          x_offset = -1;
        }
        break;
      default:
        break;
      }
      headID = get_first_ID(i);
      get_pos(headID, posx, posy);
      if((*posx == 2) && (*posy == 2)){ //TODO: implement apples instead
        appendFlag = 1;
      }
      //Check if OOB
      if(*posx+x_offset == -1){
        x_offset = FIELD_WIDTH-1;
      }else if(*posx+x_offset == FIELD_WIDTH){
        x_offset = -(FIELD_WIDTH-1);
      }
      if(*posy+y_offset == -1){
        y_offset = FIELD_HEIGHT-1;
      }else if(*posy+y_offset == FIELD_HEIGHT){
        y_offset = -(FIELD_HEIGHT-1);
      }  
      

      if(appendFlag && (get_highest_ID(i)+1 < num_players*i+MAX_LENGTH)){
        headID = get_highest_ID(i)+1;
        append_first(i, headID);
        appendFlag = 0;
      }else{
        move_last_first(i);
        headID = get_first_ID(i);
      }
      set_pos(headID, *posx+x_offset, (*posy)+y_offset);
      move_object(sockfd, headID, (*posx)+x_offset, (*posy)+y_offset);
    } //TODO: Collect all moveobjects for all players and send to client. Needs  to be implemented in client too.
  }
  free(posx);
  free(posy);
}

//Returns ID of first apple not already spawned.
int next_apple_spawn(int *apple_decay){
  for(int i = 0; i < MAX_APPLES; i++){
    if(apple_decay[i] == 0){
      return i;
    }
  }
  return -1;
}

void game_loop(int sockfd){
  char *buffer = malloc(MAX_MSG_SIZE);
  int isRunning = 1;
  char *latest_char = calloc(get_num_players()*2+1, sizeof(char));
  ssize_t count;
  
  int selectVal, tmp_ID, posx, posy, apple_id, apple_tick = APPLE_SPAWN;
  
  int apple_decay[MAX_APPLES]; //Currently unused apples are 0
  memset(apple_decay, 0, sizeof(int)*MAX_APPLES);

  char tmp_c;
  struct timeval timeout;
  fd_set fds;
  
  //long tot_nsec = NSEC_FRAME + SEC_FRAME*1000000000;
  long sec = SEC_FRAME;
  long nsec = NSEC_FRAME; //tot_nsec*4/5%1000000000;
  
  struct timespec end_time;
  struct timespec curr_time;

  clock_gettime(CLOCK_REALTIME, &end_time);
  srand(end_time.tv_nsec); //seed

  int just_sent = 1;
  while(isRunning){
    if(just_sent){
      clock_gettime(CLOCK_REALTIME, &end_time);
      end_time.tv_sec += sec;
      end_time.tv_nsec += nsec;
      just_sent = 0;
    }
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
      if(strchr(VIABLE_INP, tmp_c)){
        latest_char[tmp_ID] = tmp_c;
      }
    }
    clock_gettime(CLOCK_REALTIME, &curr_time);
    //frames.
    if((curr_time.tv_sec > end_time.tv_sec) || ((curr_time.tv_sec == end_time.tv_sec) && (curr_time.tv_nsec >= end_time.tv_nsec))){
      process_inputs(sockfd, latest_char);
      //Spawn apple
      if(apple_tick > APPLE_SPAWN){
        apple_tick = 0;
        apple_id = next_apple_spawn(apple_decay);
        apple_decay[apple_id] = 1;
        apple_id += MAX_LENGTH*(MAX_PLAYERS);
        posx = rand() % FIELD_WIDTH;
        posy = rand() % FIELD_HEIGHT;
        move_object(sockfd, apple_id, posx, posy);
      }
      //TODO: calculate collisions. snake->snake, snake->apple

      //Despawn apple
      for(int i = 0; i < MAX_APPLES; i++){
        if(apple_decay[i] > APPLE_DESPAWN){
          apple_id = i+MAX_LENGTH*(MAX_PLAYERS);
          move_object(sockfd, apple_id, -1, -1); //Move OOB
          apple_decay[i] = 0;
        }
      }


      if(latest_char[0] == 'q'){
        isRunning = 0;
      }
      just_sent = 1;
      apple_tick++;
      for(int i = 0; i < MAX_APPLES; i++){
        if(apple_decay[i]){
          apple_decay[i]++;
        }
      }

    }
  }
  free(latest_char);
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
    printf("Server: initiation finished! \n");
    game_loop(sockfd);
    
    wait(NULL); //Wait for Client to terminate.
  }else{ //client
    static char *argvc[]={"client", address, NULL};
    execv("./client", argvc);
    pexit("Error execv: ");
  }
  return 0;
}
    

