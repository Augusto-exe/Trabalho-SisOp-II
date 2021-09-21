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

class ClientTCP 
{
	public:
		ClientTCP(char *_perfil, char *_end_servidor, char *_porta);
		void set_sigint_callback(sighandler_t handler);
		void end_connection(int signum);
		bool start_connection();

	private:
		int sockfd;
		pthread_t clientThread;
		char* perfil;
		char* end_servidor;
		char* porta;
		static void *thread_read_client(void *socket);

};
// int main(int argc, char *argv[]);