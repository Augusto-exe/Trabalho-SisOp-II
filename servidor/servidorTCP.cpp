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
#include <map>
#include <cstdlib>
#include <ctime>
#include <string>
#include <signal.h>
#include <memory>
#include <list>
#include <mutex>
#include "../common.h"
#include "./notificationManager.hpp"

#define PORT 4000
#define ELECTION_TIMEOUT 3

using namespace std;

bool connected = true;
bool leader = false;
bool electionStarted = false;
bool answerd = false;

mutex write_mtx;
mutex thread_mtx;
map <int,bool> writed;

int id = 0;
int leaderId = -1;
list<int> serverSocketList;
string read_server_addr;
list<string> serverIps;
map<int,int> socketToId;

NotificationManager *notificationManager;

typedef struct new_thread_args
{
	int socket;
	int sessionID;
	bool connected;
	char* username;
	struct sockaddr_in socketAddress;
} new_thread_args;

void *thread_tweet_to_client(void *args);
void *thread_read_client(void *args);

void sendAnsMsg(int socket)
{
	int n;
	packet pkt;
	pkt.type = TIPO_SERVER_ANS;
	strcpy(pkt._payload, to_string(id).c_str());
	strcpy(pkt.user, "server");
	//cout << "mandando msg de coord - socket: "<< socket << endl;
	
	write_mtx.lock();
	//writed[socket] = true;
	write(socket, &pkt, sizeof(pkt));
	write_mtx.unlock();
	if (n < 0)
		printf("ERROR writing to socket");
}
void reconnectToClients()
{
	struct hostent *client;
	struct sockaddr_in cli_addr;
	new_thread_args* arg = (new_thread_args*) malloc(sizeof(new_thread_args));
	int sockfd;
	string aux_porta,aux_ip,aux_session,localUserName,aux_string,lim = "#";
	pthread_t clientThread;

	client = gethostbyname("localhost");
	if (client == NULL)
	{
		fprintf(stderr, "ERROR, no such host\n");
	}

	for(auto itMap : notificationManager->sessionAddresses)
	{
		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			printf("ERROR opening socket\n");
		aux_string = itMap.first;
		localUserName = aux_string.substr(0, aux_string.find(lim));
		aux_string.erase(0, aux_string.find(lim) + lim.length());
		aux_session = aux_string.substr(0, aux_string.find(lim));
		

		cli_addr.sin_family = AF_INET;
		cli_addr.sin_port = htons(2000);
		cli_addr.sin_addr = *((struct in_addr *)client->h_addr);
		cli_addr.sin_addr.s_addr = itMap.second.sin_addr.s_addr;
		bzero(&(cli_addr.sin_zero), 8);

		//inicia conexão com o servidor
		if (connect(sockfd, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0)
			printf("ERROR connecting\n");

		arg->username =(char*) malloc(16*sizeof(char));
		printf("\nUser %s logged in.\n", localUserName.c_str());
		strcpy(arg->username, localUserName.c_str());
		arg->sessionID = stoi(aux_session);
		thread_mtx.lock();
		arg->socketAddress = cli_addr;
		arg->socket = sockfd;
		arg->connected = true;
		arg->sessionID = stoi(aux_session);

		cout <<"user: " << localUserName <<" socket: " <<sockfd << endl;
		pthread_create(&clientThread, NULL, thread_tweet_to_client, (void*) arg);
		pthread_create(&clientThread, NULL, thread_read_client, (void*) arg);
		arg = (new_thread_args*) malloc(sizeof(new_thread_args));
		
	}
	
}

void sendCoordMsg(int socket)
{
	int n;
	packet pkt;
	pkt.type = TIPO_SERVER_COORD;
	strcpy(pkt._payload, to_string(id).c_str());
	strcpy(pkt.user, "server");
	//cout << "mandando msg de coord - socket: "<< socket << endl;
	
	write_mtx.lock();
	//writed[socket] = true;
	write(socket, &pkt, sizeof(pkt));
	write_mtx.unlock();
	if (n < 0)
		printf("ERROR writing to socket");
}
void sendMsgCoordAll()
{
	int n;
	packet pkt;
	pkt.type = TIPO_SERVER_COORD;
	strcpy(pkt._payload, to_string(id).c_str());
	strcpy(pkt.user, "server");
	//cout << "mandando msg de coord - socket: "<< socket << endl;
	for(auto socket : serverSocketList)
	{
		write_mtx.lock();
		writed[socket] = true;
		n = write(socket, &pkt, sizeof(pkt));
		write_mtx.unlock();
		if (n < 0)
			printf("ERROR writing to socket");
	}


}

void sendMsgElection()
{
	int n;
	packet pkt;
	pkt.type = TIPO_SERVER_ELECTION;
	strcpy(pkt._payload, to_string(id).c_str());
	strcpy(pkt.user, "server");
	//cout << "mandando msg de coord - socket: "<< socket << endl;
	for(auto socket : serverSocketList)
	{

		write_mtx.lock();
		writed[socket] = true;
		n = write(socket, &pkt, sizeof(pkt));
		write_mtx.unlock();
		if (n < 0)
			printf("ERROR writing to socket");
		
		
	}
}
void sendLoginToservers(packet pkt,int session_id)
{
	int n;
	pkt.type = TIPO_SERVER_ADD_SES;
	string payload = "";
	payload +=to_string(session_id)+"-";
	payload +=to_string(notificationManager->sessionAddresses[string(pkt.user) +"#"+to_string(session_id)].sin_addr.s_addr)+"-";
	payload +=to_string(notificationManager->sessionAddresses[string(pkt.user) +"#"+to_string(session_id)].sin_port);
	strcpy(pkt._payload, payload.c_str());
	pkt.length = strlen(pkt._payload);
	
	//cout << "mandando msg de coord - socket: "<< socket << endl;
	for(auto socket : serverSocketList)
	{

		write_mtx.lock();
		//writed[socket] = true;
		n = write(socket, &pkt, sizeof(pkt));
		write_mtx.unlock();
		if (n < 0)
			printf("ERROR writing to socket");
		
		
	}

}

void forwardPacketsToServers(packet pkt)
{
	int n;
	
	//cout << "mandando msg de coord - socket: "<< socket << endl;
	for(auto socket : serverSocketList)
	{

		write_mtx.lock();
		//writed[socket] = true;
		n = write(socket, &pkt, sizeof(pkt));
		write_mtx.unlock();
		if (n < 0)
			printf("ERROR writing to socket");
		
		
	}
}

void sendLogoutToServers(packet pkt,int session_id,string user)
{
	int n;
	pkt.type = TIPO_SERVER_RMV_SES;
	strcpy(pkt.user,user.c_str());
	string payload = "";
	payload +=to_string(session_id);
	strcpy(pkt._payload, payload.c_str());
	pkt.length = strlen(pkt._payload);
	
	//cout << "mandando msg de coord - socket: "<< socket << endl;
	for(auto socket : serverSocketList)
	{

		write_mtx.lock();
		//writed[socket] = true;
		n = write(socket, &pkt, sizeof(pkt));
		write_mtx.unlock();
		if (n < 0)
			printf("ERROR writing to socket");
		
		
	}
}

void *thread_tweet_to_client(void *args)
{
	struct new_thread_args *arg = (struct new_thread_args *)args;
	int n = 1, localsockfd = arg->socket, session_id = arg->sessionID;
	string username = string(arg->username);
	//cout << "socket " << localsockfd << " session id " << session_id << " user " << username << endl;
	packet pkt;

	while (arg->connected)
	{
		sleep(1);
		//verifica se precisa enviar tweet ao cliente
		if (leader && notificationManager->needsToSend(username,session_id))
		{ 
			//consome tweet e envia ao cliente
			pkt = notificationManager->consumeTweet(username,session_id);
			write_mtx.lock();
			writed[localsockfd] = true;
			cout << localsockfd << endl;
			n = write(localsockfd, &pkt, sizeof(pkt));
			write_mtx.unlock();
			cout << n << endl;
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
	int n, localsockfd;
	localsockfd =arg->socket;
	int electionID;
	char *sessionUser;
	Sockaddr_in socketAddress = arg->socketAddress;
	string aux_addr,aux_port,aux_sess,lim,line;
	int session_id;
	int j;
	char localUserName[16];
	if(arg->username !=NULL)
		strcpy(localUserName,arg->username);
	if(arg->sessionID >0)
		session_id = arg->sessionID;
	cout << "socket read " << localsockfd << endl;
	thread_mtx.unlock();
	
	packet pkt, rcvPkt;
	while (arg->connected)
	{
		pkt.type = -1;
		/* read from the socket */
		//cout << "waiting to read" << endl;
		n = read(localsockfd, &pkt, sizeof(pkt));
		//cout << "readed - " << n << " , " << writed<< endl;
		if (n <= 0)
		{
			//cout << "entrou" << endl;
			write_mtx.lock();
			if(writed[localsockfd])
			{
				
				writed[localsockfd] = false;
				
			}
			else
			{
				if(socketToId[localsockfd] == leaderId)
				{
					electionStarted = true;
					cout << "leader disconnected" << endl;
				}
					
				cout << "ERROR reading from socket with id" << socketToId[localsockfd]<< endl;
			}
			write_mtx.unlock();
			arg->connected = false;
		}
		rcvPkt = pkt;

		//cout <<"pkt received " << pkt.type<<endl;

		//verifica tipo de pacote recebido e trata de acordo
		switch (pkt.type)
		{
		case (TIPO_DISC):
			//caso seja de desconexão coloca connected em 0, deleta a sessão 
			cout << "User " << localUserName << " logged out." << endl;
			arg->connected = false;
			sendLogoutToServers(pkt,session_id,string(pkt.user));
			if (sessionUser != NULL)
			{

			notificationManager->del_session(string(pkt.user),session_id);
			}
			break;
		case (TIPO_SEND):
			//produz uma notificação de acordo com o pacote recebido
			if(leader)
				forwardPacketsToServers(pkt);
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
				sendLoginToservers(pkt,session_id);
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
			
			write_mtx.lock();
			//writed[localsockfd] = true;
			n = write(localsockfd, &pkt, sizeof(pkt));
			write_mtx.unlock();
			break;
		}
		case (TIPO_FOLLOW):
			// dar um follow = se adicionar a lista de seguires de alguem
			// payload diz o 'alguem' e o user ja vem no pacote
			if(leader)
				forwardPacketsToServers(pkt);
			notificationManager->follow(string(pkt.user), string(pkt._payload),leader);
			break;
		case(TIPO_SERVER):
			serverSocketList.push_back(localsockfd);
			socketToId[localsockfd] = atoi(pkt._payload);
			cout<<"new server connected and added to list. ID = " << pkt._payload << " socket :"<< localsockfd<< endl;
			if(leader)
			{
				sendCoordMsg(localsockfd);
			}
			break;
		case(TIPO_SERVER_CONSUME):
			break;
		case(TIPO_SERVER_COORD):
			leaderId = atoi(pkt._payload);
			cout<< "LEADER: " << leaderId << endl;
			break;
		case(TIPO_SERVER_ANS):
			
			answerd = true;
			break;
		case(TIPO_SERVER_ELECTION):
			electionID = atoi(pkt._payload);
			if(electionID < id)
			{
				sendAnsMsg(localsockfd);
			}
				
			break;
		case (TIPO_SERVER_ADD_SES):
			
			lim="-";
			line = string(pkt._payload);
			//cout << pkt._payload << endl;
			aux_sess = line.substr(0, line.find("-"));
			line.erase(0, line.find("-") + lim.length());
			aux_addr = line.substr(0, line.find("-"));
			line.erase(0, line.find("-") + lim.length());
			aux_port = line.substr(0,line.find("-"));
			notificationManager->add_session_from_server(string(pkt.user),stoi(aux_sess),stoi(aux_addr),stoi(aux_port));
			break;
		case (TIPO_SERVER_RMV_SES):
			cout << "deleting " << pkt.user <<" - " << pkt._payload << endl;
			notificationManager->del_session(string(pkt.user),atoi(pkt._payload));
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
	while(true)
	{
		if(electionStarted)
		{
			sendMsgElection();
			sleep(ELECTION_TIMEOUT);
			if(!answerd)
			{
				leaderId = id;
				leader = true;
				answerd = false;
				sendMsgCoordAll();
				reconnectToClients();
			}
			electionStarted = false;

		}

	}
}

void tryToConnectToServerGroup()
{
	struct hostent *server;
	struct sockaddr_in serv_addr;
	int sockfd;
	int countConnected =0;
	packet pkt;
	pthread_t clientThread;
	pkt.type = TIPO_SERVER;
	strcpy(pkt.user,"server");
	pkt.seqn = -1;
	new_thread_args *args;
	
	ifstream serverFile("serverList.txt");
	string addr_str,id_str,lim = "-";

	for (std::string line; getline(serverFile, line); ) 
	{
		
		//cout << args <<" - ";
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
		{
			//printf("ERROR connecting\n");
			close(sockfd);
		}			
		else
		{
			//cria thread de leitura que lida com mensagens vindas do server
			thread_mtx.lock();
			args = (new_thread_args*) malloc(sizeof(new_thread_args));
			args->socket = sockfd;
			args->connected = true;	
			pthread_create(&clientThread, NULL, thread_read_client, (void*) args);
			
			countConnected +=1;
			serverSocketList.push_back(sockfd);
			socketToId[sockfd] = stoi(id_str);
			
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
		strcpy(pkt._payload,to_string(id).c_str());
		write_mtx.lock();
		//writed[sock] = true;
		write(sock, &pkt, sizeof(pkt));
		write_mtx.unlock();
	}
	pthread_create(&clientThread, NULL, electionTimeoutManager, NULL);
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
		printf("ERROR on binding\n");
	else
	{	

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
		thread_mtx.lock();
		args->socketAddress = cli_addr;
		args->socket = newsockfd;
		args->connected = true;
		args->sessionID = -1;
		args->username = NULL;
		//ver um jeito de se for server lançar a thread de leitura própria -> pode ser aqui, ou dps de fazer a conexão mandar mensagem, enfim

		//cria thread de leitura que lida com mensagens vindas do cliente

		pthread_create(&clientThread, NULL, thread_read_client, (void*) args);
		
	}
	}
	close(sockfd);
	return 0;
}