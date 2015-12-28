CC=gcc
CFLAGS=-Wall -std=c99 

all: main client

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<


ascii_lib.o: ascii_lib/ascii_lib.c
	$(COMPILE) -c ascii_lib/ascii_lib.c

main: main.c ascii_lib/ascii_lib.o defs.o network.o network_server.o defs.h obj_list.o 
	$(CC) $(CFLAGS) ascii_lib/ascii_lib.o defs.o network.o network_server.o obj_list.o main.c -o main -ggdb 

client: client.c ascii_lib/ascii_lib.o defs.o network.o obj_list.o defs.h	
	$(CC) $(CFLAGS) ascii_lib/ascii_lib.o defs.o network.o obj_list.o client.c -o client -lm -pthread

.PHONY: clean
clean:
	rm *.o



