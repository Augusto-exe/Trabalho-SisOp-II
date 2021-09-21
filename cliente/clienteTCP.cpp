#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <ctime>
#include "../common.h"
#include "./clienteTCP.hpp"

bool connected = true;
packet disc_pkt = {TIPO_DISC, 0, 5, 0, "adm", "Disc"};

using namespace std;

ClientTCP::ClientTCP(char *_perfil, char *_end_servidor, char *_porta)
{
	perfil = _perfil;
	end_servidor = _end_servidor;
	porta = _porta;
}

void ClientTCP::set_sigint_callback(sighandler_t handler)
{
	signal(SIGINT, handler);
}

void ClientTCP::end_connection(int signum)
{
	write(sockfd, &disc_pkt, sizeof(disc_pkt));
	connected = false;
	close(sockfd);
	exit(signum);
}

bool ClientTCP::start_connection()
{
	struct hostent *server;
	struct sockaddr_in serv_addr;

	server = gethostbyname(end_servidor);
	if (server == NULL)
	{
		fprintf(stderr, "ERROR, no such host\n");
		return false;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		printf("ERROR opening socket\n");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(porta));
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		printf("ERROR connecting\n");

	packet pkt;

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

	n = read(sockfd, &pkt, sizeof(pkt));
	if (n < 0)
		printf("ERROR reading from socket");
	else
	{
		if (pkt.type == TIPO_PERMISSAO_CON)
			printf("CAIU NO IF\n\n");
	}

	// aqui verifica se ja tem 2 conexoes

	//pthread_create(&clientThread, NULL, thread_read_client, &sockfd);

	return true;
}

void *ClientTCP::thread_read_client(void *socket)
{

	int n, localsockfd, *newsockfd = (int *)socket;
	localsockfd = *newsockfd;

	packet pkt;
	while (connected)
	{
		/* read from the socket */
		n = read(localsockfd, &pkt, sizeof(pkt));

		if (n < 0)
			printf("ERROR reading from socket");
		else
		{
			if (pkt.type == TIPO_NOTI)
				printf("\n\nNEW NOTIFICATION from user %s:\n%s\n\n", pkt.user, pkt._payload);
		}
	}
}

// int main(int argc, char *argv[])
//{
// int n;

// packet pkt;

// char *perfil = argv[1];

// pkt.type = TIPO_LOGIN;
// pkt.seqn = 0;
// pkt.length = strlen(perfil);
// pkt.timestamp = std::time(0);
// bzero(pkt._payload, sizeof(pkt._payload));
// bzero(pkt.user, sizeof(pkt.user));
// strcpy(pkt._payload, perfil);
// strcpy(pkt.user, perfil);

// /* write in the socket */
// n = write(sockfd, &pkt, sizeof(pkt));
// if (n < 0)
// 	printf("ERROR writing to socket\n");

// /* read from the socket */
// pthread_create(&clientThread, NULL, thread_read_client, &sockfd);

// //printf("%s\n",buffer);
// while (true)
// {
// 	char comando[200];
// 	//printf("Por favor insira um comando:\n");
// 	fgets(comando, 200, stdin);

// 	char delim[] = " ";

// 	char *ptr = strtok(comando, delim);
// 	printf("%s\n\n", ptr);
// 	bool eh_primeira_iteracao = true;
// 	int tipo_comando = -1; // nao sei chamar o .h, entao n consegui criar enum
// 	char resto_do_comando[200] = "";

// 	for (int i = 0; i < strlen(ptr); i++)
// 		ptr[i] = tolower(ptr[i]);
// 	printf("%sa", ptr);
// 	while (ptr != NULL)
// 	{
// 		if (eh_primeira_iteracao)
// 		{
// 			if (strcmp(ptr, "follow") == 0)
// 			{ // fazer um tolower
// 				tipo_comando = TIPO_FOLLOW;
// 			}
// 			else if (strcmp(ptr, "send") == 0)
// 			{ // fazer um tolower
// 				tipo_comando = TIPO_SEND;
// 			}
// 			eh_primeira_iteracao = false;
// 		}
// 		else
// 		{
// 			char aux_string[100];
// 			sprintf(aux_string, " %s", ptr);
// 			strcat(resto_do_comando, aux_string);
// 		}
// 		ptr = strtok(NULL, delim);
// 	}
// 	switch (tipo_comando)
// 	{
// 	case (TIPO_SEND):
// 		//SeguirPerfil(resto_do_comando);
// 		break;
// 	case (TIPO_FOLLOW):
// 		//Tweetar(resto_do_comando);
// 		break;
// 	default:
// 		printf("Comando invalido. Por favor escreva um comando em um dos seguintes formatos: \n\tFOLLOW @username\n\tSEND message_to_send\n\n");
// 		break;
// 	}
// 	return 0;
// }
//return 0;
//}