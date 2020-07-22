#ifndef MYclient__
#define MYclient__ 1

#include <string.h>
#include "helpers.h"
#include "list.h"

#define CLIENT struct client

struct client
{
	char* id;
	int socket;
	int online;
	list queue;
	list sf;
};

// subscribe constructor
CLIENT* newClient(char* id);
void put_online(CLIENT* su,int socket);
void put_offline(CLIENT* su);
// find client in list
list findBySocket(list sList,int socket);
list findByID(list sList,char* id);
// send function
//		send message(s) if online
//		put in  queue if offline
void sendS(CLIENT* s,char* buffer,char* topic);
// create a client
void sendS(CLIENT* s,char* buffer,char* topic);

#endif