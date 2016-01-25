#ifndef DEFS_H
#define DEFS_H

#define CLIENT_PORT 5001
#define SERVER_PORT 5002

#define MAX_APPLES 20
#define MAX_LENGTH 10
#define START_LENGTH 1
#define MAX_MSG_SIZE 512
#define MAX_DESIGN_SIZE 2
#define MAX_PLAYERS 1

#define SEC_FRAME 0
#define NSEC_FRAME 75000000

#define ACK_TO 3
#define SERV_TALK_TO 5

#define SEC_READ_TO 0
#define USEC_READ_TO 200000  

#define FIELD_HEIGHT 20
#define FIELD_WIDTH 20

//Number of frames
#define APPLE_SPAWN 20
#define APPLE_DESPAWN 30

extern const char VIABLE_INP[];

void pexit(char *error);

#endif

/**
Communication protocol:
1. init field. "1 width height backg num_of_objects playerID"
 - Needs an ack "1 playerID"
2. init object "2 objID posx posy height width design(char*)"
 - Needs an ack "2 playerID objID"
3. move object "3 objID posx posy" 
 */

//All objects have a uniform ID in all clients.  
//New objects have to be able to be added while playing. NO, everything is sent at the start.

/*
Loop around screen? Yes, can be fixed in server.
No calculations for client side? Start with all calculations in server.
 - Moving the snake client side would mean it has to predicts the movement and correct it if wrong. Less stress on server and enables more clients.

Order shouldn't be a problem since time is split into discrete moments of frames.
 */

/*Snake is composed of one object per link meaning whole snake is multiple objects.   
- Snake is moved by moving last object to second to last object's position and so on until first link object. First link object is moved one unit in the direction of players last input.

CHEATING THE SYSTEM: Move the last link object to the front and keep the rest. will work. If apple is eaten, just add a new object in front instead.
 - Snakes for players are categorized by their objID.
N = MAX_PLAYERS
Player0 Snake: 0->MAX_LENGTH-1
Player1 Snake: MAX_LENGTH->2*MAX_LENGTH-1.
PlayerN Snake: MAX_LENGTH*N->MAX_LENGTH*(N+1)-1
Apples: MAX_LENGTH(N+1)->maxInt? 
 */

/*
All changes in objects have to be deployed every frame. Frame speed has to be adjusted depending on latency to server. Can be tested before start after everything else works.
 */

