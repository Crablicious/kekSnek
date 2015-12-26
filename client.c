#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "ascii_lib/ascii_lib.h"

/**
Threads Client:
1. Regularly Drawing Graphics, keeping track of objects.   
2. Keyboard listener and sends it to server.
3. Receives changes in object's pos from server.
 */

int main(int argc, char *argv[])
{
  char address[20];
  strcpy(address, argv[1]);
  initiate_field(20, 20, ' ', 10);  
  
  
  pthread_t r_graphics_t, r_keyboard_t, r_server_talker_t;
  
  
  return 0;
}

void *graphics_thread(void){
  
}
