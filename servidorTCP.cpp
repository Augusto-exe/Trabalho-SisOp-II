#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include "common.h"

#define PORT 4000

bool connected = true;

void* thread_read_client(void* socket){
	
	int n, localsockfd, *newsockfd = (int*)socket;
	localsockfd = *newsockfd;

	packet pkt;
	while(connected)
	{
		/* read from the socket */
		n = read(localsockfd, &pkt, sizeof(pkt));
		if (n < 0) 
			printf("ERROR reading from socket");

			printf("type: %d\n", pkt.type);
			printf("socket: %d\n",localsockfd);
			printf("seqn: %d\n", pkt.seqn);
			printf("length: %d\n", pkt.length);
			printf("timestamp: %d\n", pkt.timestamp);
			printf("payload: %s\n", pkt._payload);
		switch(pkt.type)
		{
			case(TIPO_DISC):
				connected = false;
				break;
			case (TIPO_LOGIN):
				printf("\nuser %s loged in\n", pkt._payload);
			default:
			break;
		}
			
	}

	close(localsockfd);
}

void* thread_write_client(void* socket){
	
	int n=1, localsockfd, *newsockfd = (int*)socket;
	localsockfd = *newsockfd;

	while(connected)
	{
		if(false){//needsToSend(user))
			packet pkt;
			//pkt = consume(user);
			pkt.type = 1;
			pkt.seqn = 2;
			pkt.length = strlen(pkt._payload);
			pkt.timestamp = 0;
			n = write(localsockfd,&pkt, sizeof(pkt));

			if (n < 0) 
				printf("ERROR writing to socket");
		}
		
		sleep(2);
	}
}



int main(int argc, char *argv[])
{
	int sockfd, newsockfd, n;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	packet pkt;
	pthread_t clientThread;
    
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding");

	while(true) {
		// listen to the clients
		listen(sockfd, 5);
		
		clilen = sizeof(struct sockaddr_in);
		if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
			printf("ERROR on accept");
		memset (&pkt, 0, sizeof (pkt));	

		pthread_create(&clientThread, NULL, thread_read_client, &newsockfd);
		pthread_create(&clientThread, NULL, thread_write_client, &newsockfd);
	}	


	close(sockfd);
	return 0; 
}