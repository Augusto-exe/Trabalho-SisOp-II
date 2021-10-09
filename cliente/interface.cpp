#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
// #include "../common.h"
#include "clienteTCP.hpp"

using namespace std;

ClientTCP *clienteTCP;

void signal_callback_handler(int signum)
{
    printf("\nDesconectando \n");
    clienteTCP->end_connection(signum);
}

bool IniciarSessao(char *perfil, char *end_servidor, char *porta)
{
    printf("Iniciar sessao usando perfil %s, endereco de servidor %s e porta %s\n", perfil, end_servidor, porta);
    // chama algum metodo do clienteTCP, que vai tentar fazer a conexao e retornar != -1 se der certo.
    clienteTCP = new ClientTCP(perfil, end_servidor, porta);
    bool success = clienteTCP->start_connection();
    if (!success)
    {
        return false;
    }
    signal(SIGINT, signal_callback_handler);

    return true;
}

void Tweetar(char *mensagem, char* username, int seqn)
{
    printf("Tweetar a mensagem: %s\n", mensagem);

    // string uses 2 extra bytes
    if (string(mensagem).length() > 130) 
    {
        cout << "The character limit is 128" << endl;
        return;
    }
    string msg_str = string(mensagem);
    msg_str.erase(0, 1); // Remove o espaço q ta ficando no primeiro caracter
    msg_str.pop_back();
    clienteTCP->send_message(msg_str, string(username), seqn, TIPO_SEND);
}

void SeguirPerfil(char *perfilQueSegue, char *perfilSeguido, int seqn)
{
    if (string(perfilQueSegue) == string(perfilSeguido)) {
        cout << "Nao eh possivel seguir a si mesmo" << endl;
    }

    printf("Seguir perfil: %s\n", perfilSeguido);
    string msg_str = string(perfilSeguido);
    msg_str.erase(0, 1); // Remove o espaço q ta ficando no primeiro caracter
    msg_str.pop_back();
    clienteTCP->send_message(msg_str, string(perfilQueSegue), seqn, TIPO_FOLLOW);
}

int main(int argc, char *argv[])
{
    //set_sigint_callback
    if (argc < 3)
    {
        printf("Para iniciar uma sessão, por favor use o comando nesse formato: ./app_cliente <perfil> <endereço do servidor> <porta>\n");
    }
    else
    {
        char *perfil = argv[1];
        char *end_servidor = argv[2];
        char *porta = argv[3];

        if (string(perfil).length() > 16)
        {
            cout << "Username nao pode exceder 16 caracteres." << endl;
            return 1;
        }

        bool login_success = IniciarSessao(perfil, end_servidor, porta);

        if (!login_success)
        {
            return 1;
        }

        while (true)
        {
            char comando[200];
            printf("Por favor insira um comando:\n");
            fgets(comando, 200, stdin);

            char delim[] = " ";

            char *ptr = strtok(comando, delim);

            bool eh_primeira_iteracao = true;
            int tipo_comando = -1;
            char resto_do_comando[200] = "";
            while (ptr != NULL)
            {
                if (eh_primeira_iteracao)
                {
                    if (strcmp(ptr, "FOLLOW") == 0 || strcmp(ptr, "follow") == 0)
                    { // fazer um tolower
                        tipo_comando = 0;
                    }
                    else if (strcmp(ptr, "SEND") == 0 || strcmp(ptr, "send") == 0)
                    { // fazer um tolower
                        tipo_comando = 1;
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
                SeguirPerfil(perfil, resto_do_comando, 0);
            }
            else if (tipo_comando == 1)
            {
                Tweetar(resto_do_comando, perfil, 0);
            }
            else
            {
                printf("Comando invalido. Por favor escreva um comando em um dos seguintes formatos: \n\tFOLLOW @username\n\tSEND message_to_send\n\n");
            }
        }
    }
    return 0;
}
