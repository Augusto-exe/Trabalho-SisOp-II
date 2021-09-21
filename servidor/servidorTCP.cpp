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
#include <ctime>
#include <signal.h>
#include "../common.h"
#include "./sessionManager.hpp"

#define PORT 4000

bool connected[1024];
int counter;
SessionManager *sessionManager;

typedef struct new_thread_args
{
	void *socket;
	int counter;
} new_thread_args;

void *thread_read_client(void *args)
{
	struct new_thread_args *arg = (struct new_thread_args *)args;
	int counter = arg->counter;
	void *socket = arg->socket;
	char *sessionUser;

	printf("counter %d\n", counter);

	int n, localsockfd, *newsockfd = (int *)socket;
	localsockfd = *newsockfd;
	char localUserName[16];

	packet pkt;
	while (connected[counter])
	{
		/* read from the socket */
		// printf("\n--waiting to read--\n");
		n = read(localsockfd, &pkt, sizeof(pkt));
		if (n < 0)
		{
			printf("ERROR reading from socket");
			connected[counter] = false;
		}

		switch (pkt.type)
		{
		case (TIPO_DISC):
			printf("\nUser %s loged out.\n", localUserName);
			connected[counter] = false;
			if (sessionUser != NULL)
			{
				sessionManager->del_session(sessionUser);
			}
			break;
		case (TIPO_SEND):
			//insertMessage(pkt.user,pkt._payload);
			break;
		case (TIPO_LOGIN):
		{
			sessionUser = strdup(pkt.user);
			bool login_success = sessionManager->add_session(sessionUser);

			strcpy(localUserName, pkt.user);
			pkt.type = TIPO_PERMISSAO_CON;
			pkt.seqn = 2;
			strcpy(pkt._payload, login_success ? "1" : "0");
			pkt.length = strlen(pkt._payload);
			pkt.timestamp = std::time(0);
			n = write(localsockfd, &pkt, sizeof(pkt));

			if (!login_success)
			{
				printf("Login error\n");
				break;
			}
			else
			{
				printf("\nUser %s loged in.\n", pkt.user);
			}

			if (n < 0)
			{
				printf("ERROR writing to socket\n");
			}

			break;
		}
		case (TIPO_FOLLOW):
			//insertFollow(pkt.user,pkt._payload);
			break;
		default:
			break;
		}
	}

	close(localsockfd);
}

void *thread_write_client(void *socket)
{

	int n = 1, localsockfd, *newsockfd = (int *)socket;
	localsockfd = *newsockfd;
	packet pkt;
	while (connected)
	{
		if (false)
		{ //needsToSend(user))

			//pkt = consume(user);
			pkt.type = TIPO_NOTI;
			pkt.seqn = 2;
			strcpy(pkt.user, "user_1");
			strcpy(pkt._payload, "tst_messagem_1");
			pkt.length = strlen(pkt._payload);
			pkt.timestamp = std::time(0);
			n = write(localsockfd, &pkt, sizeof(pkt));

			if (n < 0)
				printf("ERROR writing to socket");
		}

		sleep(10);

		pkt.type = TIPO_NOTI;
		pkt.seqn = 2;
		strcpy(pkt.user, "user_1");
		strcpy(pkt._payload, "tst_messagem_1");
		pkt.length = strlen(pkt._payload);
		pkt.timestamp = std::time(0);
		//n = write(localsockfd,&pkt, sizeof(pkt));
	}
}

int main(int argc, char *argv[])
{
	sessionManager = new SessionManager();
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

	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		printf("ERROR on binding");

	while (true)
	{
		// listen to the clients
		listen(sockfd, 5);

		clilen = sizeof(struct sockaddr_in);
		if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1)
			printf("ERROR on accept");
		memset(&pkt, 0, sizeof(pkt));

		new_thread_args args;
		args.counter = counter;
		args.socket = &newsockfd;
		connected[counter] = true;

		counter += 1;

		pthread_create(&clientThread, NULL, thread_read_client, (void *)&args);
		pthread_create(&clientThread, NULL, thread_write_client, &newsockfd);
	}

	close(sockfd);
	return 0;
}