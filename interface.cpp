#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "interface.h" ????

void IniciarSessao(char *perfil, char *end_servidor, char *porta)
{
    printf("Iniciar sessao usando perfil %s, endereco de servidor %s e porta %s\n", perfil, end_servidor, porta);
}

void Tweetar(char *mensagem)
{
    printf("Tweetar a mensagem: %s\n", mensagem);
}

void SeguirPerfil(char *perfil)
{
    printf("Seguir perfil: %s\n", perfil);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Para iniciar uma sessão, por favor use o comando nesse formato: ./app_cliente <perfil> <endereço do servidor> <porta>\n");
    }
    else
    {
        char *perfil = argv[1];
        char *end_servidor = argv[2];
        char *porta = argv[3];
        IniciarSessao(perfil, end_servidor, porta);

        while (true)
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
                SeguirPerfil(resto_do_comando);
            }
            else if (tipo_comando == 1)
            {
                Tweetar(resto_do_comando);
            }
            else
            {
                printf("Comando invalido. Por favor escreva um comando em um dos seguintes formatos: \n\tFOLLOW @username\n\tSEND message_to_send\n\n");
            }
        }
    }
    return 0;
}
