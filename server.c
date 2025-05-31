#include "common.h"               
#include <stdio.h>             
#include <stdlib.h>               
#include <string.h>               
#include <unistd.h>               
#include <time.h>                 
#include <sys/types.h>
#include <sys/socket.h>           

#define BUFSZ 1024                // Tamanho do buffer para strings e mensagens

// Função que imprime o modo de uso correto do servidor e encerra
void usage(int argc, char **argv){
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    exit(EXIT_FAILURE);
}

// Retorna o nome da ação de ataque correspondente ao número recebido
const char* action_name(int action) {
    const char *actions[] = {"Nuclear Attack", "Intercept Attack", "Cyber Attack", "Drone Strike", "Bio Attack"};
    return actions[action];
}

// Função que decide o vencedor da rodada
// Retorna:
//   -1 -> empate
//    0 -> vitória do servidor
//    1 -> vitória do cliente
int decide_winner(int client, int server) {
    if (client == server) return -1;

    // Matriz de quem perde para quem
    int win_map[5][2] = {
        {2, 3}, // Nuclear vence de Cyber e Drone
        {0, 4}, // Intercept vence de Nuclear e Bio
        {1, 3}, // Cyber vence de Intercept e Drone
        {1, 4}, // Drone vence de Intercept e Bio
        {0, 2}  // Bio vence de Nuclear e Cyber
    };

    return (client == win_map[server][0] || client == win_map[server][1]) ? 0 : 1;
}

int main(int argc, char **argv){
    // Verifica se o usuário passou os argumentos necessários
    if(argc < 3){
        usage(argc, argv);
    }

    struct sockaddr_storage storage;

    // Inicializa o endereço do servidor com base no protocolo e porta
    if(0 != server_sockaddr_init(argv[1], argv[2], &storage)){
        usage(argc, argv);
    }

    // Cria o socket do servidor (TCP)
    int s = socket(storage.ss_family, SOCK_STREAM, 0);
    if(s == -1){
        logexit("socket");
    }

    // Permite reuso do endereço/porta (evita erro ao reiniciar)
    int enable = 1;
    if(0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))){
        logexit("setsockopt");
    }

    // Faz o bind do socket ao endereço
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if(0 != bind(s, addr, sizeof(storage))){
        logexit("bind");
    }

    // Coloca o socket em modo de escuta (aguardando conexões)
    if(0 != listen(s, 10)){
        logexit("listen");
    }

    // Imprime mensagem indicando que o servidor está pronto
    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("Servidor iniciado em %s. Aguardando conexão...\n", addrstr);

    // Loop principal para aceitar múltiplas conexões (uma por vez)
    while(1){
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        // Aceita uma conexão de cliente
        int csock = accept(s, caddr, &caddrlen);
        if(csock == -1){
            logexit("accept");
        }

        // Converte e exibe o endereço do cliente conectado
        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] Conexão de %s\n", caddrstr);

        GameMessage msg; // Estrutura para mensagens do protocolo
        int client_wins = 0, server_wins = 0;

        // Define a semente do gerador de números aleatórios
        srand(time(NULL));

        // Loop da partida 
        while(1){
            // Envia uma solicitação de jogada para o cliente
            memset(&msg, 0, sizeof(msg));
            msg.type = MSG_REQUEST;
            send(csock, &msg, sizeof(msg), 0);

            // Recebe a jogada do cliente
            recv(csock, &msg, sizeof(msg), 0);

            // Verifica se o valor recebido está no intervalo permitido
            if(msg.client_action < 0 || msg.client_action > 4){
                msg.type = MSG_ERROR;
                snprintf(msg.message, MSG_SIZE, "Por favor, selecione um valor de 0 a 4.");
                send(csock, &msg, sizeof(msg), 0);
                continue; // Pede novamente
            }

            // Gera jogada aleatória do servidor
            msg.server_action = rand() % 5;

            // Verifica o resultado da partida
            msg.result = decide_winner(msg.client_action, msg.server_action);

            // Atualiza placar
            if(msg.result == 1) client_wins++;
            else if(msg.result == 0) server_wins++;

            // Envia o resultado da rodada
            msg.type = MSG_RESULT;
            msg.client_wins = client_wins;
            msg.server_wins = server_wins;
            snprintf(msg.message, MSG_SIZE, "Você: %s | Servidor: %s | %s",
                     action_name(msg.client_action),
                     action_name(msg.server_action),
                     msg.result == 1 ? "Vitória!" : msg.result == 0 ? "Derrota!" : "Empate!");
            send(csock, &msg, sizeof(msg), 0);

            // Se for empate, repete a jogada
            if(msg.result == -1) continue;

            // Pergunta ao cliente se deseja jogar novamente
            memset(&msg, 0, sizeof(msg));
            msg.type = MSG_PLAY_AGAIN_REQUEST;
            send(csock, &msg, sizeof(msg), 0);

            // Recebe a resposta do cliente
            recv(csock, &msg, sizeof(msg), 0);
            if(msg.result != 1 && msg.result != 0){
                msg.type = MSG_ERROR;
                snprintf(msg.message, MSG_SIZE, "Por favor, digite 1 para jogar novamente ou 0 para encerrar.");
                send(csock, &msg, sizeof(msg), 0);
                continue;
            }

            // Se o cliente escolheu encerrar
            if(msg.result == 0){
                msg.type = MSG_END;
                msg.client_wins = client_wins;
                msg.server_wins = server_wins;
                snprintf(msg.message, MSG_SIZE, "Placar final: Você %d x %d Servidor", client_wins, server_wins);
                send(csock, &msg, sizeof(msg), 0);
                break; // termina a conexão
            }
        }

        // Fecha o socket do cliente
        close(csock);
    }

    
    close(s);
    return 0;
}
