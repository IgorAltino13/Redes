#pragma once
#include <stdlib.h>
#include <arpa/inet.h>



#define MSG_SIZE 256
//enum de MessageType
typedef enum {
    MSG_REQUEST,
    MSG_RESPONSE,
    MSG_RESULT,
    MSG_PLAY_AGAIN_REQUEST,
    MSG_PLAY_AGAIN_RESPONSE,
    MSG_ERROR,
    MSG_END
} MessageType;

//Estrutura disponibilizada
typedef struct {
    int type;                // Tipo da mensagem
    int client_action;      // Ação escolhida pelo cliente
    int server_action;      // Ação escolhida pelo servidor
    int result;             // Resultado: 1 (vitória), 0 (derrota), -1 (empate)
    int client_wins;        // Vitórias do cliente
    int server_wins;        // Vitórias do servidor
    char message[MSG_SIZE]; // Mensagem textual
} GameMessage;



void logexit(const char *msg);
int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage);
void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);
int server_sockaddr_init(const char *proto, const char* portstr, struct sockaddr_storage *storage);