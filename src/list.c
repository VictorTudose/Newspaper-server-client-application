#include "list.h"

list cons(void* info,list node)
{
	list newL=(list)calloc(1,sizeof(struct genCell));
	if(newL){
		newL->info=info;
	}
	if(node){
		list aux=node;
		while(aux->next)
			aux=aux->next;
		aux->next=newL;
		return node;
	}
	return newL;
}

void removeL(list* al,list cell)
{
	list l=*al;
	if(!cell)
		return;
	list ant=NULL;
	while(l && l!=cell){
		ant=l;
		l=l->next;
	}
	// is null
	if(!l)
		return;
	//is cell
	if(ant)
		ant->next=l->next;
	else
	{
		*al=l->next;
	}
	free(l);
}

list match(list l,char* info)
{
	if(!l || !info)
		return NULL;

	while(l)
	{
		if(!strcmp((char*)(l->info),info))
			return l;
		l=l->next;
	}
	return NULL;
}