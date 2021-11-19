#ifndef __common_h__
#define __common_h__

#define TIPO_DISC		        0
#define TIPO_SEND		        1
#define TIPO_LOGIN		        2
#define TIPO_FOLLOW		        3
#define TIPO_NOTI		        4
#define TIPO_PERMISSAO_CON      5 // provisorio
#define TIPO_SERVER             6
#define TIPO_SERVER_CONSUME     7
#define TIPO_SERVER_COORD       8
#define TIPO_SERVER_ANS         9
#define TIPO_SERVER_ELECTION    10
#define TIPO_SERVER_ADD_SES     11
#define TIPO_SERVER_RMV_SES     12


typedef struct __packet{
    uint16_t type; //Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn; //Número de sequência
    uint16_t length; //Comprimento do payload
    uint16_t timestamp; // Timestamp do dado
    char user[16];
    char _payload [256]; //Dados da mensagem
} packet;

#endif