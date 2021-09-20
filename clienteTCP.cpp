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
#include "common.h" 

#define PORT 4000

bool connected = true;
int sockfd;
void signal_callback_handler(int signum) {
	
	printf("\nDesconectando \n");
	write(sockfd, &disc_pkt, sizeof(disc_pkt));
	connected = false;
	exit(signum);
}


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
	}

	close(localsockfd);
}

int main(int argc, char *argv[])
{
    int n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    pthread_t clientThread;
	
	signal(SIGINT, signal_callback_handler);
    packet pkt;
    
    if (argc < 3) {
    	fprintf(stderr,"Para iniciar uma sessão, por favor use o comando nesse formato: %s <perfil> <endereço do servidor> <porta>\n",argv[0]);
		exit(0);
    }
	
	server = gethostbyname(argv[2]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket\n");
    
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(PORT);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     
	
    
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        printf("ERROR connecting\n");

    char *perfil = argv[1];

	pkt.type = TIPO_LOGIN;
	pkt.seqn = 0;
	pkt.length = strlen(perfil);
	pkt.timestamp = 0;
	bzero(pkt._payload, sizeof(pkt._payload));
	strcpy(pkt._payload, perfil);

    /* write in the socket */
	n = write(sockfd, &pkt, sizeof(pkt));
    if (n < 0) 
		printf("ERROR writing to socket\n");

	
	/* read from the socket */
	pthread_create(&clientThread, NULL, thread_read_client, &sockfd);

    //printf("%s\n",buffer);
    while(true)
    {
    	char comando[200];
        printf("Por favor insira um comando:\n");
        fgets(comando, 200, stdin);
        
        char delim[] = " ";

        char *ptr = strtok(comando, delim);

        bool eh_primeira_iteracao = true;
        int tipo_comando = -1; // nao sei chamar o .h, entao n consegui criar enum
        char resto_do_comando[200] = "";
        while (ptr != NULL)
        {
            if (eh_primeira_iteracao)
            {
                if (strcmp(ptr, "FOLLOW") == 0 || strcmp(ptr, "follow") == 0)
                { // fazer um tolower
                    tipo_comando = TIPO_FOLLOW;
                }
                else if (strcmp(ptr, "SEND") == 0 || strcmp(ptr, "send") == 0)
                { // fazer um tolower
                    tipo_comando = TIPO_SEND;
                }
                eh_primeira_iteracao = false;
            }
            else
            {
                char aux_string[100];
                sprintf(aux_string, " %s", ptr);
                strcat(resto_do_comando, aux_string);
            }
            ptr = strtok(NULL, delim);
        }
        if (tipo_comando == 0)
        {
            //SeguirPerfil(resto_do_comando);
        }
        else if (tipo_comando == 1)
        {
            //Tweetar(resto_do_comando);
        }
        else
        {
            printf("Comando invalido. Por favor escreva um comando em um dos seguintes formatos: \n\tFOLLOW @username\n\tSEND message_to_send\n\n");
        }
    }
	close(sockfd);
    return 0;
}