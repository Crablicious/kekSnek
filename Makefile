CC=gcc
CFLAGS=-Wall -std=c99 

all: main client

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<


ascii_lib.o: ascii_lib/ascii_lib.c
	$(COMPILE) -c ascii_lib/ascii_lib.c

main: main.c ascii_lib/ascii_lib.o defs.o network.o network_server.o 
	$(CC) $(CFLAGS) ascii_lib/ascii_lib.o defs.o network.o network_server.o main.c -o main 

client: ascii_lib/ascii_lib.o defs.o	
	$(CC) $(CFLAGS) ascii_lib/ascii_lib.o defs.o client.c -o client -lm

.PHONY: clean
clean:
	rm *.o
	rm *~ 
