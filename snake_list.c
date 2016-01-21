#include "defs.h"
#include "snake_list.h"
#include <stdlib.h>


struct linked_list{
  int objID;
  struct linked_list *next_link; //NULL if last
};

struct linked_list *snake_lists[MAX_PLAYERS];

void move_last_first(int playerID){
  if(snake_lists[playerID]->next_link == NULL){
    return;
  }
  struct linked_list *curr = snake_lists[playerID];
  struct linked_list *previous = NULL;
  while(curr->next_link){
    previous = curr;
    curr = curr->next_link;
  }
  if(previous){
    previous->next_link = NULL;
  }
  curr->next_link = snake_lists[playerID];
  snake_lists[playerID] = curr;
}

void append_first(int playerID, int objID){
  struct linked_list *new_link = malloc(sizeof(struct linked_list));
  new_link->objID = objID;
  new_link->next_link = snake_lists[playerID];
  snake_lists[playerID] = new_link;
}


int get_first_ID(int playerID){
  return snake_lists[playerID]->objID;
}

void free_snake_list(){
  for(int i = 0; i < MAX_PLAYERS; i++){
    struct linked_list *next = snake_lists[i];
    struct linked_list *previous = NULL;
    while(next != NULL && next->next_link){
      previous = next;
      next = next->next_link;
      free(previous);
    }
    free(next);
  }
}

int get_highest_ID(int playerID){
  int highest = 0;
  if(snake_lists[playerID] != NULL){
    struct linked_list *curr = snake_lists[playerID]; 
    while(curr){
      if(curr->objID > highest){
        highest = curr->objID;
      }
      curr = curr->next_link;
    }
  }
  return highest;
}
