#define TIPO_DISC		0
#define TIPO_SEND		1
#define TIPO_LOGIN		2
#define TIPO_FOLLOW		3
#define TIPO_NOTI		4


typedef struct __packet{
    uint16_t type; //Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn; //Número de sequência
    uint16_t length; //Comprimento do payload
    uint16_t timestamp; // Timestamp do dado
    char user[16];
    char _payload [128]; //Dados da mensagem
} packet;

packet disc_pkt = {TIPO_DISC,0,5,0,"adm","Disc"};