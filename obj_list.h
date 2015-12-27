#ifndef OBJ_LIST_H
#define OBJ_LIST_H

#include "defs.h"
#include "ascii_lib/ascii_lib.h"

void set_num_obj(int num);

void set_obj(int objID, struct position *pos);

void set_pos(int objID, int posx, int posy);

void get_pos(int objID, int *posx, int *posy);

int obj_exists(int objID);

void free_list();

#endif
