#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


void pexit(char *error){
  perror(error);
  exit(EXIT_FAILURE);
}
