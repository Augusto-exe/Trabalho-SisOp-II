#ifndef __client_tcp_hpp__
#define __client_tcp_hpp__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>
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

using namespace std;

class ClientTCP 
{
	public:
		ClientTCP(char *_perfil, char *_end_servidor, char *_porta);
		void set_sigint_callback(sighandler_t handler);
		void end_connection(int signum,string username);
		bool start_connection(string cliAdd);
		void send_message(string message, string username, int seqn, int messageType);
		void sendPendingMessages();
		vector<packet> pendingMessages;

	private:
		pthread_t clientThread;
		char* perfil;
		char* end_servidor;
		char* porta;
		static void* thread_read_client(void *socket);
		static void* waitForReconnection(void* args);
		

};

#endif