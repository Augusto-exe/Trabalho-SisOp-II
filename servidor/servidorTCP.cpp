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
#include "./notificationManager.hpp"

#define PORT 4000

using namespace std;

bool connected = true;
SessionManager *sessionManager;
NotificationManager *notificationManager;

typedef struct new_thread_args
{
	void *socket;
	int sessionID;
	bool connected;
	string username;
} new_thread_args;

void *thread_read_client(void *args)
{
	struct new_thread_args *arg = (struct new_thread_args *)args;

	void *socket = arg->socket;
	char *sessionUser;

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
			cout << "User " << localUserName << "logged out." << endl;
			arg->connected = false;
			if (sessionUser != NULL)
			{
				cout << "entrou aqui" << endl;
				sessionManager->del_session(sessionUser);
			}
			break;
		case (TIPO_SEND):
			//insertMessage(pkt.user,pkt._payload);
			cout << "recebi uma msg " << pkt._payload << " do usuario " << pkt.user << endl;
			// vai popular alguma coisa que o needsToSend vai ler
			break;
		case (TIPO_LOGIN):
		{
			sessionUser = strdup(pkt.user);
			int session_id = sessionManager->add_session(string(sessionUser));

			strcpy(localUserName, pkt.user);
			pkt.type = TIPO_PERMISSAO_CON;
			pkt.seqn = 2;
			strcpy(pkt._payload, session_id != -1 ? "1" : "0");
			pkt.length = strlen(pkt._payload);
			pkt.timestamp = std::time(0);
			n = write(localsockfd, &pkt, sizeof(pkt));

			if (session_id == -1)
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
			// dar um follow = se adicionar a lista de seguires de alguem
			// payload diz o 'alguem' e o user ja vem no pacote
			cout << "chegou um pacote do tipo follow do user" << pkt.user << endl;
			notificationManager->follow(string(pkt.user), "FULANO");
			//insertFollow(pkt.user,pkt._payload);
			break;
		default:
			break;
		}
	}

	close(localsockfd);
}

void *thread_write_client(void *args)
{
	struct new_thread_args *arg = (struct new_thread_args *)args;
	int n = 1, localsockfd, *newsockfd = (int *)arg->socket;
	localsockfd = *newsockfd;
	packet pkt;
	while (arg->connected)
	{
		if (false)
		{ 
			// todo needsToSend(arg->username)
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
	}

	cout << "saindo do while no read" << endl;
}

bool needsToSend(string username) {
	// vai consultar alguma coisa (um mapa de username -> file de notificacoes pendentes)
	return true;
}

int main(int argc, char *argv[])
{
	sessionManager = new SessionManager();
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

		clilen = sizeof(struct sockaddr_in);
		if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1)
			printf("ERROR on accept");
		memset(&pkt, 0, sizeof(pkt));

		args = (new_thread_args*) malloc(sizeof(new_thread_args));
		args->socket = &newsockfd;
		args->connected = true;

		pthread_create(&clientThread, NULL, thread_read_client, (void*) args);
		pthread_create(&clientThread, NULL, thread_write_client, (void*) args);
	}

	close(sockfd);
	return 0;
}