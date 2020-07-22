#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, ret;
	struct sockaddr_in serv_addr={0};
	char buffer[BUFLEN];
	char temp[BUFLEN];
	int fdmax;

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	if (argc < 3) {
		usage(argv[0]);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (SA*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;


	char mess[50];
	memset(mess,0,50);
	strcpy(mess,"ID ");
	strncat(mess,argv[1],strlen(argv[1]));
	DIE(send(sockfd,mess,50,0)<0,"send");

	while (1) {

		tmp_fds = read_fds;
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
		memset(buffer, 0, BUFLEN);

		if(FD_ISSET(sockfd,&tmp_fds)){

			DIE(recv(sockfd,buffer, BUFLEN ,0)<=0,"recv");

			if(!strncmp(buffer,"exit",4)){
				exit(0);
				break;
			}
			printf("%s\n",buffer);
		}
		else
		{
			read(STDIN_FILENO,buffer,BUFLEN-1);
			strcpy(temp,buffer);
			char* p=strtok(temp," ");
			if( !strcmp(p,"subscribe") )
			{
				p=strtok(NULL," ");
				if(!p)
					continue;
				char* s_SF=strtok(NULL," ");
				if(!s_SF)
					continue;
				char* s_null=strtok(NULL," ");
				int SF=atoi(s_SF);
				if(SF!=0&&SF!=1)
					continue;
				if(s_null)
					continue;

				DIE(send(sockfd,buffer,BUFLEN-1,0)<0,"send");
			}
			if(!strcmp(p,"unsubscribe"))
			{
				p=strtok(NULL," ");
				if(!p)
					continue;
				char* s_null=strtok(NULL," ");
				if(s_null)
					continue;

				DIE(send(sockfd,buffer,BUFLEN-1,0)<0,"send");
				
			}
			if(!(strncmp(buffer,"exit",4)))
			{
				break;
			}
		}
	}

	close(sockfd);

	return 0;
}
