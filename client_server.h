#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H

#define MSG_SIZE 256

typedef enum {
    MSG_REQUEST,    
    MSG_RESPONSE,   
    MSG_RESULT, 
    MSG_PLAY_AGAIN_REQUEST, 
    MSG_PLAY_AGAIN_RESPONSE,     
    MSG_ERROR,       
    MSG_END         
} MessageType;

typedef struct { 
    int type;            // Tipo da mensagem (usando MessageType)
    int client_action;   // Escolha do cliente (0-4)
    int server_action;   // Escolha do servidor (0-4)
    int result;          // Resultado: 1 vitória, 0 derrota, -1 empate
    int client_wins;     // Número de vitórias do cliente
    int server_wins;     // Número de vitórias do servidor
    char message[MSG_SIZE];  // Texto extra (mensagem para cliente)
} GameMessage;

#endif
