#include "helpers.h"
#include "topic.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{

	setvbuf(stdout,NULL,_IONBF,0);

	int tcplisten,udplisten, newsockfd, portno;
	
	char buffer[BUFLEN];
	
	struct sockaddr_in serv_addr, cli_addr;
	
	int n, i, j;
	
	int ret;
	socklen_t clilen;

	list queueID=NULL;

	fd_set read_fds;
	fd_set tmp_fds;
	int fdmax;

	if (argc < 2) {
		usage(argv[0]);
	}

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	HANDLE H={0};

	tcplisten = socket(AF_INET, SOCK_STREAM, 0);
	DIE(tcplisten < 0, "socket tcp");

	udplisten = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(udplisten < 0, "socket tcp");

	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	memset((char *) &serv_addr, 0, sizeof(SAi));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(tcplisten, (SA *) &serv_addr, sizeof(SA));
	DIE(ret < 0, "bind tcp");

	ret = bind(udplisten, (SA *) &serv_addr, sizeof(SA));
	DIE(ret < 0, "bind udp");

	ret = listen(tcplisten, MAX_CLIENTS);
	DIE(ret < 0, "tcplisten");

	FD_SET(tcplisten, &read_fds);
	FD_SET(udplisten, &read_fds);
	FD_SET(0, &read_fds);
	fdmax = tcplisten>udplisten ? tcplisten:udplisten;

	for(;;) {

		tmp_fds = read_fds;

		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
					if(i==0)
					{
						scanf("%s",buffer);
						if(!strncmp(buffer,"exit",4))
						{
							for(j=5;j<=fdmax;j++)
							{
								if(i!=j && FD_ISSET(j,&read_fds))
								{
									DIE( send(j,buffer,sizeof(buffer),0)<0,"send");
								}
							}
						}

						goto exit_1;
					}

					if( i == tcplisten ){

						clilen = sizeof(cli_addr);
						newsockfd = accept(tcplisten, (SA *) &cli_addr, &clilen);
						DIE(newsockfd < 0, "accept");
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) {
							fdmax = newsockfd;
						}

						char* result=calloc(20,sizeof(char));
						sprintf(result,"%s",inet_ntoa(cli_addr.sin_addr));
						sprintf(result,"%s:%d",result,ntohs(cli_addr.sin_port));

						queueID = cons(result,queueID);

						sprintf(buffer,"%d",i);
						continue;
					}
					if( i== udplisten){

						DIE((ret=recvfrom(udplisten,(void*)buffer,BUFLEN,0,(SA *) &cli_addr, &clilen))<0,"recv udp" ); 
						
						unsigned char type=buffer[50];
						char* topic=calloc(50,sizeof(char));
						strncpy(topic,buffer,50);

						char* payload;
						
						if(type<0||type>3)
							goto out_of_udp;						

						int count= type==0 ? 5: (type==1 ? 2: (type==2 ? 6 : 1500 ) )  ;
						
						payload=calloc(count,sizeof(char));

						memcpy(payload,buffer+51,count);
						handle_udp(&H,topic,type,payload,&cli_addr);
						handle_log(&H);
out_of_udp:
						continue;
					}

					memset(buffer, 0, BUFLEN);
					
					n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");

					if (n == 0)
					{
						list l=findBySocket(H.lSb,i);
						printf("Client %s disconnected\n", ((CLIENT*)(l->info))->id);
						close(i);
						disconnect(&H,i);							
						FD_CLR(i, &read_fds);
					} else {
						ret=handle_message(&H,buffer,i,&queueID);
						if(ret==-1)
						{
							DIE(send(i,"exit",4,0)<0,"send fraud");
							close(i);						 	
							FD_CLR(i, &read_fds);
							continue;
						}
					}
			}
		}
	}

exit_1:
	close(tcplisten);

	return 0;
}
