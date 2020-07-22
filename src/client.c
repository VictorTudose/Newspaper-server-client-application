#include "client.h"

CLIENT* newClient(char* id)
{
	CLIENT* n=(CLIENT*)calloc(1,sizeof(CLIENT));
	if(n)
	{
		n->id=id;
		n->online=0;
	}
	return n;
}

void put_online(CLIENT* su,int socket)
{
	su->socket=socket;
	su->online=1;
	sendS(su,NULL,NULL);
}

void put_offline(CLIENT* su)
{
	su->online=0;
	su->socket=-1;
}

list findBySocket(list sList,int socket)
{
	while(sList)
	{
		if(sList->info && ((CLIENT*)sList->info)->socket==socket )
			return sList;
		sList=sList->next;
	}
	return NULL;
}

list findByID(list sList,char* id)
{
	if(!id)
		return NULL;

	while(sList)
	{
		if(sList->info && !strcmp( ((CLIENT*)sList->info)->id,id) )
			return sList;
		sList=sList->next;
	}
	return NULL;
}

void sendS(CLIENT* s,char* buffer,char* topic)
{

	
	if(s->online){
		while(s->queue)
		{
			char* mess=(char*)(s->queue->info);
			DIE(send(s->socket,mess,2000,0)<0,"send from queue");
			removeL(&(s->queue),s->queue);
		}

		if(!buffer)
			return;
		
		DIE(send(s->socket,buffer,2000,0)<0,"send");
	}
	else
	{
		if(!topic)
			return;

		if(match(s->sf,topic)){
			char* copy=(char*)calloc(1550,sizeof(char));
			strcpy(copy,buffer);
			s->queue=cons(copy,s->queue);
		}
	}
}