#include "obj_list.h"
#include <stdlib.h>
#include <pthread.h>

struct client_object{
  int objID;
  struct position *pos;
};


pthread_mutex_t obj_lock;
static struct client_object *obj_list;

void set_num_obj(int num){
   if (pthread_mutex_init(&obj_lock, NULL) != 0){
     pexit("Mutex init failed: ");
   }
   if ((obj_list = calloc(num ,sizeof(struct client_object))) == NULL){
     pexit("Callocing failed: ");
   }
   obj_list[0].objID = -1; //To enable checking for ID 0 in obj_exists
}

void set_obj(int objID, struct position *pos){
  pthread_mutex_lock(&obj_lock);
  obj_list[objID].objID = objID;
  obj_list[objID].pos = pos;
  pthread_mutex_unlock(&obj_lock);
}

void set_pos(int objID, int posx, int posy){
  pthread_mutex_lock(&obj_lock);
  obj_list[objID].pos->x = posx;
  obj_list[objID].pos->y = posy;
  pthread_mutex_unlock(&obj_lock);
}

void get_pos(int objID, int *posx, int *posy){
  pthread_mutex_lock(&obj_lock);
  *posx = obj_list[objID].pos->x;
  *posy = obj_list[objID].pos->y;
  pthread_mutex_unlock(&obj_lock);
}

int obj_exists(int objID){
  if(obj_list[objID].objID == objID){
    return 1;
  }else{
    return 0;
  }
}

void free_list(){
  pthread_mutex_destroy(&obj_lock);
  free(obj_list);
}
