#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 1024  // Tamanho máximo do buffer de mensagens

// Exibe o modo correto de uso do cliente
void usage(int argc, char **argv){
    printf("usage: %s <server IP> <server port>\n", argv[0]);
    exit(EXIT_FAILURE);
}

// Função com validação para ler um número inteiro do usuário
int ler_inteiro_seguro(const char *prompt, int *valor) {
    char input[BUFSZ];
    printf("%s", prompt);
    if (fgets(input, sizeof(input), stdin) == NULL)
        return 0; // erro ao ler
    return sscanf(input, "%d", valor) == 1; // retorna 1 se conseguiu ler um número inteiro
}

int main(int argc, char **argv){
    if(argc < 3){
        usage(argc, argv); // exige IP e porta como argumentos
    }

    struct sockaddr_storage storage;
    // Converte os argumentos em um endereço de socket
    if(0 != addrparse(argv[1], argv[2], &storage)){
        usage(argc, argv); // erro na conversão
    }

    // Criação do socket TCP
    int s = socket(storage.ss_family, SOCK_STREAM, 0);
    if(s == -1){
        logexit("socket"); // erro na criação do socket
    }

    // Conecta ao servidor
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if(0 != connect(s, addr, sizeof(storage))){
        logexit("connect"); // erro ao conectar
    }

    // Converte o endereço do servidor em string para exibição
    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("Conectado ao servidor: %s\n", addrstr);

    GameMessage msg; // estrutura de mensagem compartilhada com o servidor

    while(1){
        // Aguarda mensagem do servidor
        recv(s, &msg, sizeof(msg), 0);

        // Solicitação de jogada do servidor
        if(msg.type == MSG_REQUEST){
            int opcao;
            // Laço até o usuário digitar uma entrada válida
            while (!ler_inteiro_seguro("\nEscolha sua jogada:\n"
                                       "0 - Nuclear Attack\n"
                                       "1 - Intercept Attack\n"
                                       "2 - Cyber Attack\n"
                                       "3 - Drone Strike\n"
                                       "4 - Bio Attack\n> ", &opcao)) {
                printf("[Erro] Entrada inválida. Digite um número entre 0 e 4.\n");
            }

            // Envia a jogada para o servidor
            msg.type = MSG_RESPONSE;
            msg.client_action = opcao;
            send(s, &msg, sizeof(msg), 0);
        }
        // Resultado da rodada enviado pelo servidor
        else if(msg.type == MSG_RESULT){
            printf("%s\n", msg.message); // exibe resultado: vitória, derrota ou empate
        }
        // Servidor pergunta se o cliente quer jogar novamente
        else if(msg.type == MSG_PLAY_AGAIN_REQUEST){
            int novamente;
            while (!ler_inteiro_seguro("\nDeseja jogar novamente?\n1 - Sim\n0 - Não\n> ", &novamente)) {
                printf("[Erro] Entrada inválida. Digite 0 ou 1.\n");
            }

            msg.type = MSG_PLAY_AGAIN_RESPONSE;
            msg.result = novamente;
            send(s, &msg, sizeof(msg), 0); // envia decisão ao servidor
        }
        // Servidor detecta erro na entrada
        else if(msg.type == MSG_ERROR){
            printf("[Erro] %s\n", msg.message); // exibe mensagem de erro do servidor
        }
        // Fim do jogo
        else if(msg.type == MSG_END){
            printf("%s\n", msg.message); // exibe placar final
            break; // encerra o loop
        }
    }

    close(s); // fecha o socket
    return 0;
}
