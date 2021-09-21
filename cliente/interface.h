#ifndef __interface_h__
#define __interface_h__

typedef enum TipoComando
{
    INVALIDO = -1,
    FOLLOW = 0,
    TWEET = 1
} TipoComando;

bool IniciarSessao(char *perfil, char *end_servidor, char *porta);
void signal_callback_handler(int signum);
int main(int argc, char *argv[]);

#endif