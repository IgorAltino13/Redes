#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


// Função que exibe o uso correto do programa e encerra a execução
//recebe como parametro o ip do servidor e o porto
void usage(int argc, char **argv){
    printf("usage: %s <server IP> <server port>",argv[0]);
    printf("example: %s 127.0.0.1 51511",argv[0]);
    exit(EXIT_FAILURE);
}

#define BUFSZ 1024




int main(int argc, char **argv){
    //verificação se a pessoa chamou o programa certo
    if(argc < 3){
        usage(argc, argv);
    }
    // Armazena o endereço do servidor (IPv4 ou IPv6)
    struct sockaddr_storage storage;
    // Converte o IP e a porta passados como argumento para estrutura de endereço
    if(0 != addrparse(argv[1],argv[2], &storage)){
        usage(argc,argv);
    }
    //criação do socket
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);

    if(s == -1){
        // Encerra se a criação do socket falhar
        logexit("socket");
    }
    
    // Faz o cast da estrutura para o tipo esperado pela função connect
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    // Tenta estabelecer conexão com o servidor
    if(0 != connect(s, addr, sizeof(storage))){
        // Encerra se a conexão falhar
        logexit("connect");
    }

    // Converte o endereço binário do servidor para string e imprime a conexão
    char addrstr[BUFSZ];
    addrtostr(addr,addrstr, BUFSZ);
    printf("Conectado ao servidor.\n");

    
   // Declaração da estrutura de mensagem usada no protocolo do jogo
   GameMessage msg;
   // Variavel auxiliar que armazena a jogada escolhida pelo cliente
   int opção = 0;
   // Variavel auxiliar que armazena se cliente quer jogar novamente
   int novamente = 0;

   // Loop principal de comunicação com o servidor
    while(1){
        // Recebe mensagem do servidor
        size_t count = recv(s, &msg, sizeof(msg), 0);
        
        //Mostra as jogadas disponiveis
        if(msg.type == 0){
            printf("%s",msg.message);
            msg.type = MSG_RESPONSE;
            //leitura da opção escolhida pelo cliente
            scanf("%d",&opção);
            msg.client_action = opção;
            //envio da escolha
            send(s,&msg,sizeof(msg),0);
        }
        else if(msg.type == 2){
            //print das açoes dos jogadores e do resultado atual
            printf("%s",msg.message);
        }
        //verifica se quer jogar novamente
        else if(msg.type == 3){
            printf("%s\n",msg.message);
            scanf("%d",&novamente);
            msg.type = MSG_PLAY_AGAIN_RESPONSE;
            msg.client_action = novamente;
            send(s,&msg,sizeof(msg),0);
            
        }
        //Exibe a mensagem de erro
        else if(msg.type == 5){
            printf("%s",msg.message);
        }
        //exibe a mensagem final(Resultado final da partida )
        else if(msg.type == 6){
            printf("%s\n",msg.message);
            break;
        }
        
        if(count == 0){
            //conexao terminada
            break;
        }
        
    }
    //Fecha o socket
    close(s);
    exit(EXIT_SUCCESS);
}