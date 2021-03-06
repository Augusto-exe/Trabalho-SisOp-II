#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <ctime>
// #include "../common.h"
#include "./clienteTCP.hpp"

bool connected = true;
bool newSocket = false;
int sockfd;
int listenSocket;

packet disc_pkt = {TIPO_DISC, 0, 5, 0, "adm", "Disc"};

using namespace std;

ClientTCP::ClientTCP(char *_perfil, char *_end_servidor, char *_porta)
{
	perfil = _perfil;
	end_servidor = _end_servidor;

	if (_porta == NULL)
	{
		strcpy(porta,"4000");
	}
	else
	{
		porta = _porta;
	}
}

void ClientTCP::set_sigint_callback(sighandler_t handler)
{
	signal(SIGINT, handler);
}

void ClientTCP::end_connection(int signum,string username)
{
	strcpy(disc_pkt.user,username.c_str());
	write(sockfd, &disc_pkt, sizeof(disc_pkt));
	connected = false;
	close(sockfd);
	exit(signum);
}
void ClientTCP::sendPendingMessages()
{
	for(packet pkt : this->pendingMessages)
	{
		write(sockfd, &pkt, sizeof(pkt));
	}
}

void ClientTCP::send_message(string message, string username, int seqn, int messageType)
{
	packet pkt;
	pkt.type = messageType;
	pkt.seqn = seqn;
	strcpy(pkt.user, username.c_str());
	strcpy(pkt._payload, message.c_str());
	pkt.length = strlen(pkt._payload);
	pkt.timestamp = std::time(0);

	if(connected)
	{
		write(sockfd, &pkt, sizeof(pkt));
	}
	else
	{
		pendingMessages.push_back(pkt);
	}
	
}
void* ClientTCP::waitForReconnection(void* args)
{
		packet localPkt;
		socklen_t clilen;
		int newsockfd;
		struct sockaddr_in serv_addr;
		// listen to the clients
		listen(sockfd, 5);
		clilen = sizeof(struct sockaddr_in);
		if ((newsockfd = accept(sockfd, (struct sockaddr *)&serv_addr, &clilen)) == -1)
			printf("ERROR on accept");
		memset(&localPkt, 0, sizeof(localPkt));

		sockfd = newsockfd;
		connected = true;

}

bool ClientTCP::start_connection(string cliAdd)
{
	struct hostent *server;
	struct sockaddr_in serv_addr,cli_addr,cli_addr2;
	cout << cliAdd << endl;
	if ((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		printf("ERROR opening socket");

	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons(2000);
	inet_aton(cliAdd.c_str(), &cli_addr.sin_addr);
	//serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(cli_addr.sin_zero), 8);

	if (bind(listenSocket, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0)
	{
		printf("1- ERROR on binding - %d\n", errno);
		exit(0);
	}
		

	server = gethostbyname(end_servidor);
	if (server == NULL)
	{
		fprintf(stderr, "ERROR, no such host\n");
		return false;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		printf("ERROR opening socket\n");
	cli_addr2.sin_family = AF_INET;
	cli_addr2.sin_port = htons(3000);
	inet_aton(cliAdd.c_str(), &cli_addr2.sin_addr);
	//serv_addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(sockfd, (struct sockaddr *)&cli_addr2, sizeof(cli_addr2)) < 0)
	{
		printf("2 - ERROR on binding - %d\n", errno);
		exit(0);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(porta));
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);
	//inicia conex??o com o servidor
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		printf("ERROR connecting\n");
	printf("server port :%d\n",serv_addr.sin_port);
	packet pkt;
	//gera pacote de login
	pkt.type = TIPO_LOGIN;
	pkt.seqn = 0;
	pkt.length = strlen(perfil);
	pkt.timestamp = std::time(0);
	bzero(pkt._payload, sizeof(pkt._payload));
	bzero(pkt.user, sizeof(pkt.user));
	strcpy(pkt._payload, perfil);
	strcpy(pkt.user, perfil);

	int n = write(sockfd, &pkt, sizeof(pkt));
	if (n < 0)
		printf("ERROR writing to socket\n");

	//le respota do servidor e verifica se sess??o pode ser iniciada
	n = read(sockfd, &pkt, sizeof(pkt));
	if (n < 0)
		printf("[start_connection] ERROR reading from socket\n");
	else
	{
		if (pkt.type == TIPO_PERMISSAO_CON)
		{
			if (atoi(pkt._payload) == 0)
			{
				printf("Voce ja esta logado em dois clientes, amigo\n");
				return false;
			}
		}
	}

	// aqui verifica se ja tem 2 conexoes

	pthread_create(&clientThread, NULL, *ClientTCP::thread_read_client, &sockfd);

	return true;
}

void *ClientTCP::thread_read_client(void *socket)
{
	pthread_t clientThread;
	int n, localsockfd, *newsockfd = (int *)socket;
	localsockfd = *newsockfd;
	
	while(true)
	{

		
		packet pkt;
		while (connected)
		{
			/* read from the socket */
			n = read(localsockfd, &pkt, sizeof(pkt));

			if (n <= 0) {
				//printf("[thread_read_client] ERROR reading from socket. Disconnecting...\n");
				connected = false;
			}
			else
			{
				//caso receba pacote de notifica????o o exibe na tela.
				if (pkt.type == TIPO_NOTI)
					printf("\n\nNEW NOTIFICATION from user %s:\n%s\n\n", pkt.user, pkt._payload);
			}
		}

		//if the connection was lost we wait for the new RM to connect
		packet localPkt;
		socklen_t clilen;
		int newsockServer;
		struct sockaddr_in serv_addr;
		n = listen(listenSocket, 5);
		clilen = sizeof(struct sockaddr_in);
		if ((newsockServer = accept(listenSocket, (struct sockaddr *)&serv_addr, &clilen)) == -1)
		{
			printf("ERROR on accept %d\n",errno);
		}		
		memset(&localPkt, 0, sizeof(localPkt));
		cout << "reconnected" << endl;
		sockfd = newsockServer;;
		localsockfd = newsockServer;

		connected = true;
	}


	return 0;
}
