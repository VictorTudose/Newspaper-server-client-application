#include "topic.h"

TOPIC* newTopic(char* name)
{
	TOPIC* n=calloc(1,sizeof(TOPIC));
	if(n)
		n->name=name;
	return n;
}

list findByName(list tList,char* name)
{
	if(!name)
		return NULL;

	if(!tList)
		return NULL;

	while(tList)
	{
		if(tList->info && !strcmp(((TOPIC*)tList->info)->name,name) )
			return tList;
		tList=tList->next;
	}
	// segfault 25
	return NULL;
}

void sendtoclients(TOPIC* t,char* buffer)
{
	list c=t->sbs;
	while(c)
	{
		sendS((CLIENT*)c->info,buffer,t->name);
		c=c->next;
	}
}

void addSubsriber(TOPIC* t,CLIENT* s)
{
	t->sbs = cons((void*)s,t->sbs);
}

void removeSubsriber(TOPIC* t,CLIENT* s)
{
	removeL(&(t->sbs),findByID(t->sbs,s->id));
}

//--------------------------------- handle start here ----------------------------------------------------------------


int newID(HANDLE* H,char* id)
{
	list found=match(H->lID,id);
	if(!found)
		return 1;
	return 0;
}

int add_new_sbs(HANDLE* H,char* id,int socket)
{
	CLIENT* su=NULL;
	if(newID(H,id)){

		char* ID=(char*)calloc(50,sizeof(char));
		strcpy(ID,id);
		H->lID=cons(ID,H->lID);

		list found=findBySocket(H->lSb,socket);		
		if(!found || ((CLIENT*)(found->info))->online==0 ){
			su=newClient(ID);
			H->lSb=cons(su,H->lSb);
		}
		else{
			su=(CLIENT*)(found->info);
		}
	}
	put_online(su,socket);
	return 1;
}

void disconnect(HANDLE* H,int socket)
{
	// segfault ?
	CLIENT* su=(CLIENT*)findBySocket(H->lSb,socket)->info;
	put_offline(su);
}

void subsribe(HANDLE* H,int socket,char* topic,int SF)
{
	CLIENT* su=(CLIENT*)findBySocket(H->lSb,socket)->info;
	list t=findByName(H->lTp,topic);
	TOPIC* tt;

	if(!t)
	{
		char* Topic=(char*)calloc(50,sizeof(char));
		strcpy(Topic,topic);
		tt=newTopic(Topic);
		H->lTp=cons(tt,H->lTp);
		tt->sbs=cons(su,tt->sbs);
		
		char* topic_name=calloc(50,sizeof(char));
		strcpy(topic_name,tt->name);
		if(SF)
			su->sf=cons(topic_name,su->sf);
		
		return;
	}
	else{
		tt=(TOPIC*)t->info;
		// add client if not already used
		if(!findByID(tt->sbs,su->id)){
			tt->sbs=cons(su,tt->sbs);

			char* topic_name=calloc(50,sizeof(char));
			strcpy(topic_name,tt->name);
			if(SF)
				su->sf=cons(topic_name,su->sf);

		}
	}

}

void unsubsribe(HANDLE* H,int socket,char* topic)
{
	// segfault 25

	topic[strlen(topic)-1]=0;
	char* to_look_for=(char*)calloc(50,sizeof(char));
	strcpy(to_look_for,topic);

	list tl=findByName(H->lTp,to_look_for);
	if(!tl){	return; }

	TOPIC* t=(TOPIC*)tl->info;
	
	list e=findBySocket(t->sbs,socket);
	removeL(&(t->sbs),e);
}

int handle_message(HANDLE* H,char* message,int socket,list* aqueueID)
{

	char* command=strtok(message," ");
	
	if(!strcmp(command,"ID")){
		char* id=strtok(NULL," ");
		
		list found=match(H->lID,id);
		
		if(found){
			found=findByID(H->lSb,id);

			if( ( ( (CLIENT*)found->info )->online ==1 ) )
			{
				return -1;
			}
			else
			{
				put_online((CLIENT*)found->info,socket);
				return 1;
			}
		}
		else{
			add_new_sbs(H,id,socket);
			printf("New client %s connected from %s\n",id,(char*)(*aqueueID)->info);
			removeL(aqueueID,(*aqueueID));
			return 1;
		}
	}
	
	if(!strcmp(command,"subscribe")){
		char* topic =strtok(NULL," ");
		char* s_SF=strtok(NULL," ");
		int SF=atoi(s_SF);
		subsribe(H,socket,topic,SF);
		handle_log(H);
		return 1;
	}

	if(!strcmp(command,"unsubscribe")){
		char* topic =strtok(NULL," ");
		unsubsribe(H,socket,topic);
		
		return 1;
	}
	
	return 0;
}

void handle_udp(HANDLE* H,char* topic,char type,char* payload,SAi* cli_addr)
{

	char* type_string=calloc(10,sizeof(char));
	char* result=calloc(1600,sizeof(char));

	switch(type)
	{
		case 0:
			strcpy(type_string,"INT");
			break;
		case 1:
			strcpy(type_string,"SHORT_REAL");
			break;
		case 2:
			strcpy(type_string,"FLOAT");
			break;
		case 3:
			strcpy(type_string,"STRING");
			break;
		default:
			break;
	}

	sprintf(result,"%s",inet_ntoa(cli_addr->sin_addr));
	sprintf(result,"%s:%d",result,ntohs(cli_addr->sin_port));
	sprintf(result,"%s - %s",result,topic);
	sprintf(result,"%s - %s",result,type_string);
	
	switch(type)
	{
		case 0:
		{
			unsigned int res= ntohl(*(uint32_t*)(payload+1));
			unsigned char sign=payload[0];
			sprintf(result,"%s - %s%u",result, sign?"-":"" ,res);
			break;
		}
		case 1:
		{
			uint16_t resa=ntohs(*(uint16_t*)payload);
			float res= ((float)resa)/100.0;
			sprintf(result,"%s - %.2f",result,res);
			break;
		}
		case 2:
		{
			unsigned char sign=payload[0];
			int ibase=ntohl(*(uint32_t*)(payload+1));
			float base=(float)ibase;
			unsigned char power=payload[5];
			while(power)
			{
				power--;
				base/=10;
			}
			sprintf(result,"%s - %s%f",result, sign?"-":"",base);
			break;
		}
		case 3:
			sprintf(result,"%s - %s",result,payload);
			break;
		default:
			break;
	}

	list t=findByName(H->lTp,topic);
	TOPIC* tt=NULL;
	if(t){
		tt=(TOPIC*)t->info;
		sendtoclients(tt,result);
	}
	else
	{
		tt=newTopic(topic);
		H->lTp=cons(tt,H->lTp);
	}

}

void handle_log(HANDLE* H)
{
	#if DEBUG_THE_MEMORY_STRUCTURE
	printf("ID : ");
	for(list it=H->lID ; it ; it=it->next)
	{
		printf("%s -> ",(char*)it->info);

	}
	printf("\n");
	printf("clients : ");
	for(list it=H->lSb ; it ; it=it->next)
	{
		printf("<%s,%d,%d: { ",((CLIENT*)(it->info))->id,((CLIENT*)(it->info))->socket,((CLIENT*)(it->info))->online);
		
		for(list jt= ( (CLIENT*)(it->info) )->sf ; jt ; jt=jt->next)
		{
			printf(" %s",(char*)(jt->info));
		}
		printf("}\n");
	}
	printf("\n");
	printf("Topics :");
	for(list it=H->lTp ; it ; it=it->next)
	{
		printf(" %s: {",((TOPIC*)(it->info))->name);
		for(list jt=((TOPIC*)(it->info))->sbs ; jt ; jt=jt->next)
		{
			printf("<%s,%d> -> ",((CLIENT*)(jt->info))->id,((CLIENT*)(jt->info))->socket) ;
		}
		printf(" }\n");
	}
	printf("\n");
	#endif
}