#ifndef __interface_h__
#define __interface_h__

typedef enum TipoComando {
    INVALIDO = -1,
    FOLLOW = 0,
    TWEET = 1
} TipoComando;

void IniciarSessao(char *perfil, char *end_servidor, char *porta);
int main(int argc, char *argv[]);

#endif 