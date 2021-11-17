#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <signal.h>
#include <memory>
#include <list>
#include "../common.h"
#include "./notificationManager.hpp"

#define PORT 4000

using namespace std;

bool connected = true;
bool leader = false;
int id = 0;
int leaderId = -1;
list<int> serverSocketList;
string read_server_addr;
list<string> serverIps;

NotificationManager *notificationManager;

typedef struct new_thread_args
{
	void *socket;
	int sessionID;
	bool connected;
	char* username;
	struct sockaddr_in socketAddress;
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
		//verifica se precisa enviar tweet ao cliente
		if (notificationManager->needsToSend(username,session_id))
		{ 
			//consome tweet e envia ao cliente
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
	Sockaddr_in socketAddress = arg->socketAddress;
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

		//verifica tipo de pacote recebido e trata de acordo
		switch (pkt.type)
		{
		case (TIPO_DISC):
			//caso seja de desconexão coloca connected em 0, deleta a sessão 
			cout << "User " << localUserName << " logged out." << endl;
			arg->connected = false;
			if (sessionUser != NULL)
			{

			notificationManager->del_session(sessionUser,session_id);
			}
			break;
		case (TIPO_SEND):
			//produz uma notificação de acordo com o pacote recebido
			notificationManager->tweetReceived(string(pkt.user),string(pkt._payload),pkt.timestamp);

			break;
		case (TIPO_LOGIN):
		{
			//verifica se o usuário pode logar
			sessionUser = strdup(pkt.user);

			session_id = notificationManager->add_session(string(sessionUser), socketAddress);

			//gera pacote de resposta
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
				//Caso usuário possa se conectar inicializa thread para envio de tweets
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

//Deve ser lnaçado e verificar se a eleição iniciou, caso tenha iniciado inicia o timer
// e após o tempo do timer verifica se algum servidor respondeu. Se nenhum respondeu esse é o novo líder. Define a Flag como 1 e 
//manda mensagem pra todos os servers 
void* electionTimeoutManager(void *args)
{

}


//essa thread deve ficar lendo e lidando com mensagens vindas de OUTROS SERVERS
void* threadReadFromServerGroup(void *args)
{

}

void tryToConnectToServerGroup()
{
	struct hostent *server;
	struct sockaddr_in serv_addr;
	int sockfd;
	int countConnected =0;

	ifstream serverFile("serverList.txt");
	string addr_str,id_str,lim = "-";

	for (std::string line; getline(serverFile, line); ) 
	{
		addr_str = line.substr(0, line.find("-"));
		line.erase(0, line.find("-") + lim.length());
		id_str = line.substr(0,line.find("-"));
		if(addr_str == read_server_addr)
			id = stoi(id_str);
		else
			serverIps.push_back( addr_str);

		server = gethostbyname(addr_str.c_str());
		if (server == NULL)
		{
			fprintf(stderr, "ERROR, no such host\n");
			
		}

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			printf("ERROR opening socket\n");

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(4000);
		serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
		bzero(&(serv_addr.sin_zero), 8);
		//inicia conexão com o servidor
		if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
			printf("ERROR connecting\n");
		else
		{
			cout <<"connected to " << addr_str << endl;
			countConnected +=1;
			serverSocketList.push_back(sockfd);
		}
			
		
	}
	if(countConnected == 0)
	{
		leader = true;
		leaderId = id;
	}
	cout << endl <<"id encontrado: " << id << endl;
	cout <<"Sou líder?" <<leader << endl;
	for (auto sock :serverSocketList)
	{
		cout << sock << endl;
	}
	
}

int main(int argc, char *argv[])
{
	read_server_addr = string(argv[1]);
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
	inet_aton(read_server_addr.c_str(), &serv_addr.sin_addr);
	//serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);

	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		printf("ERROR on binding");

	new_thread_args *args;
	tryToConnectToServerGroup();

	while (true)
	{
		// listen to the clients
		listen(sockfd, 5);
		args = (new_thread_args*) malloc(sizeof(new_thread_args));
		clilen = sizeof(struct sockaddr_in);
		if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1)
			printf("ERROR on accept");
		memset(&pkt, 0, sizeof(pkt));

		args->socketAddress = cli_addr;
		args->socket = &newsockfd;
		args->connected = true;
		//ver um jeito de se for server lançar a thread de leitura própria -> pode ser aqui, ou dps de fazer a conexão mandar mensagem, enfim

		//cria thread de leitura que lida com mensagens vindas do cliente
		pthread_create(&clientThread, NULL, thread_read_client, (void*) args);
		
	}

	close(sockfd);
	return 0;
}