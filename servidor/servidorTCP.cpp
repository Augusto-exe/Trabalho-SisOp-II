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
#include "./notificationManager.hpp"

#define PORT 4000

using namespace std;

bool connected = true;

NotificationManager *notificationManager;

typedef struct new_thread_args
{
	void *socket;
	int sessionID;
	bool connected;
	char* username;
} new_thread_args;

void *thread_tweet_to_client(void *args)
{
	struct new_thread_args *arg = (struct new_thread_args *)args;
	int n = 1, localsockfd, *newsockfd = (int *)arg->socket, session_id = arg->sessionID;
	localsockfd = *newsockfd;
	packet pkt;
	string username = string(arg->username);

	while (arg->connected)
	{
		sleep(1);
		if (notificationManager->needsToSend(username,session_id))
		{ 
			pkt = notificationManager->consumeTweet(username,session_id);;
			n = write(localsockfd, &pkt, sizeof(pkt));

			if (n < 0)
				printf("ERROR writing to socket");
		}
	}

	return 0;
}

void *thread_read_client(void *args)
{
	struct new_thread_args *arg = (struct new_thread_args *)args;
	pthread_t clientThread;

	void *socket = arg->socket;
	char *sessionUser;
	int session_id;

	int n, localsockfd, *newsockfd = (int *)socket;
	localsockfd = *newsockfd;
	char localUserName[16];

	packet pkt;
	while (arg->connected)
	{
		/* read from the socket */
		n = read(localsockfd, &pkt, sizeof(pkt));
		if (n < 0)
		{
			cout << "ERROR reading from socket" << endl;
			arg->connected = false;
		}

		switch (pkt.type)
		{
		case (TIPO_DISC):
			cout << "User " << localUserName << " logged out." << endl;
			arg->connected = false;
			if (sessionUser != NULL)
			{

			notificationManager->del_session(sessionUser,session_id);
			}
			break;
		case (TIPO_SEND):
			//insertMessage(pkt.user,pkt._payload);
			//cout << "recebi uma msg " << pkt._payload << " do usuario " << pkt.user << endl;
			notificationManager->tweetReceived(string(pkt.user),string(pkt._payload),pkt.timestamp);
			// vai popular alguma coisa que o needsToSend vai ler
			break;
		case (TIPO_LOGIN):
		{
			sessionUser = strdup(pkt.user);
			session_id = notificationManager->add_session(string(sessionUser));


			strcpy(localUserName, pkt.user);
			pkt.type = TIPO_PERMISSAO_CON;
			pkt.seqn = 2;
			strcpy(pkt._payload, session_id != -1 ? "1" : "0");
			pkt.length = strlen(pkt._payload);
			pkt.timestamp = std::time(0);
			
			if (session_id == -1)
			{
				printf("Login error\n");	
			}
			else
			{

				arg->username =(char*) malloc(16*sizeof(char));
				printf("\nUser %s logged in.\n", pkt.user);
				strcpy(arg->username, localUserName);
				arg->sessionID = session_id;
				
				pthread_create(&clientThread, NULL, thread_tweet_to_client, (void*) arg);
				
			}

			if (n < 0)
			{
				printf("ERROR writing to socket\n");
			}
			n = write(localsockfd, &pkt, sizeof(pkt));
			break;
		}
		case (TIPO_FOLLOW):
			// dar um follow = se adicionar a lista de seguires de alguem
			// payload diz o 'alguem' e o user ja vem no pacote
			
			notificationManager->follow(string(pkt.user), string(pkt._payload));
			break;
		default:
			break;
		}
	}

	close(localsockfd);
	return 0;
}


int main(int argc, char *argv[])
{

	notificationManager = new NotificationManager();
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

	new_thread_args *args;

	while (true)
	{
		// listen to the clients
		listen(sockfd, 5);
		args = (new_thread_args*) malloc(sizeof(new_thread_args));
		clilen = sizeof(struct sockaddr_in);
		if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1)
			printf("ERROR on accept");
		memset(&pkt, 0, sizeof(pkt));


		args->socket = &newsockfd;
		args->connected = true;

		pthread_create(&clientThread, NULL, thread_read_client, (void*) args);
		
	}

	close(sockfd);
	return 0;
}