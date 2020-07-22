#ifndef MYTOPIC__
#define MYTOPIC__ 1

#include <string.h>
#include "list.h"
#include "client.h"

#define TOPIC struct topic
#define HANDLE struct handle

struct topic
{
	char* name;
	list sbs;
};
// topic constructor
TOPIC* newTopic(char* name);
// find the topic by name
list findByName(list tList,char* name);
void sendtoclients(TOPIC* t,char* buffer);
// add and remove
void addSubsriber(TOPIC* t,CLIENT* s);
void removeSubsriber(TOPIC* t,CLIENT* s);

// abstraction of topic to subsriber list and client list
struct handle
{
	list lTp;
	list lSb;
	list lID;
};

void handle_log(HANDLE* H);
int newID(HANDLE* H,char* id);
// message "ID <id>" from socket
int add_new_sbs(HANDLE* H,char* id,int socket);
void subsribe(HANDLE* H,int socket,char* topic,int SF);
void unsubsribe(HANDLE* H,int socket,char* topic);
void disconnect(HANDLE* H,int socket);

int handle_message(HANDLE* H,char* message,int socket,list* aqueueID);
void handle_udp(HANDLE* H,char* topic,char type,char* payload,SAi * cli_addr);

#endif