#ifndef SNAKE_LIST_H
#define SNAKE_LIST_H
#include "ascii_lib/ascii_lib.h"

//Last part of snake is moved to the front.
void move_last_first(int playerID);
 
//Appending a new part to the snake.
void append_first(int playerID, int objID);

//Position of first object
int get_first_ID(int playerID);

//Returns -1 if NULL
int get_second_ID(int playerID);

//Free all the lists.
void free_snake_list();

int get_highest_ID(int playerID);

#endif
