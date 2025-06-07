#include "common.h"               
#include <stdio.h>             
#include <stdlib.h>               
#include <string.h>               
#include <unistd.h>               
#include <time.h>                 
#include <sys/types.h>
#include <sys/socket.h>           

#define BUFSZ 1024               

// Função para exibir o uso correto do programa e encerrar a execução
void usage(int argc, char **argv){
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    exit(EXIT_FAILURE);
}
//Nomes das jogadas possiveis
char nomes_ataque[5][20] = {
    "Nuclear Attack", "Intercept Attack", "Cyber Attack", "Drone Strike", "Bio Attack"
};
//Função para retornar o nome da jogada escolhida pelo cliente
char* verifica_nome_acao_cliente(GameMessage msg){
    if(msg.client_action == 0){
        return nomes_ataque[0];
    }
    else if(msg.client_action == 1){
        return  nomes_ataque[1];
    }
    else if(msg.client_action == 2){
        return nomes_ataque[2];
    }
    else if(msg.client_action == 3){
        return nomes_ataque[3];
    }
    else{
        return nomes_ataque[4];
    }
}
//Função para retornar o nome da jogada escolhida pelo servidor
char* verifica_nome_acao_servidor(GameMessage msg){
    if(msg.server_action == 0){
        return nomes_ataque[0];
    }
    else if(msg.server_action == 1){
        return  nomes_ataque[1];
    }
    else if(msg.server_action == 2){
        return nomes_ataque[2];
    }
    else if(msg.server_action == 3){
        return nomes_ataque[3];
    }
    else{
        return nomes_ataque[4];
    }
}
//Retorna o resultado da partida como string
char* resultado(GameMessage msg){
    if(msg.result == 1){
        return "Vitoria";
    }
    else if(msg.result == -1){
        return "Empate";
    }
    else{
        return "Derrota";
    }

}

//Função para verificar o ganhador da rodada
int verificar_resultado(int cliente, int servidor) {
    //Caso seja empate
    if(cliente == servidor){
        return -1;
    }
    //Caso seja vitoria ou derrota
    else{
        switch(cliente){
            //Nuclear Attack
            case 0: 
                //Vence Cyber Attack e Drone Strike
                if(servidor == 2 || servidor == 3) {
                    return 1;
                }
                //Perde para Intercept Attack e Bio Attack
                else {
                    return 0;
                }
            //Intercept Attack
            case 1:
                //Vence de Nuclear Attack e Bio Attack
                if(servidor == 0 || servidor == 4) {
                    return 1;
                }
                //Perde para Cyber Attack e Drone Strike
                else {
                    return 0;
                }
            //Cyber Attack 
            case 2:
                //Vence de Intercept Attack e Drone Strike
                if(servidor == 1 || servidor == 3) {
                    return 1;
                }
                //Perde para Nuclear Attack e Bio Attack 
                else {
                    return 0;
                }
            //Drone Strike
            case 3:
                //Vence de Intercept Attack e Bio Attack
                if(servidor == 1 || servidor == 4) {
                    return 1;
                }
                //Perde para NUclear Attack e Cyber Attack
                else {
                    return 0;
                }
            //Bio Attack
            case 4:
                //Vence de NUclear Attack e Cyber Attack
                if(servidor == 0 || servidor == 2) {
                    return 1;
                }
                //Perde para Intercept Attack e Drone Strike
                else {
                    return 0;
                }
            default: return -2;
            
        }
    }
}

//Variavel auxiliar para indicar o numero de vitorias do cliente
int vitorias_cliente = 0;
//Variavel auxiliar para indicar o numero de vitorias do servidor
int vitorias_servidor = 0;
//Variavel auxiliar para indicar o resultado
int result = 0;
//Variavel auxiliar para indicar o nome da ação do cliente
char* nome_acao_cliente;
//Variavel auxiliar para indicar o nome da ação do servidor
char* nome_acao_servidor;
//Variavel auxiliar para indicar o resultado em forma de string
char* string_resultado;

int main(int argc, char **argv){
    if(argc < 3){
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if(0 != server_sockaddr_init(argv[1], argv[2], &storage)){
        usage(argc, argv);
    }
    // Criação do socket TCP
    int s = socket(storage.ss_family, SOCK_STREAM, 0);
    if(s == -1){
        logexit("socket");
    }
    // Permite reuso do endereço em caso de reconexão
    int enable = 1;
    if(0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))){
        logexit("setsockopt");
    }
    // Associa o socket ao endereço e porta
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if(0 != bind(s, addr, sizeof(storage))){
        logexit("bind");
    }
    // Coloca o socket em modo escuta
    if(0 != listen(s, 10)){
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("Servidor iniciado em modo %.4s na porta %s. Aguardando conexão...\n", addrstr,addrstr +strlen(addrstr)-5);

    struct sockaddr_storage cstorage;
    struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
    socklen_t caddrlen = sizeof(cstorage);

     // Aceita conexão de um cliente
    int csock = accept(s, caddr, &caddrlen);
    if(csock == -1){
        logexit("accept");
    }

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("Cliente conectado.\n");
    // Inicializa estrutura da mensagem
    GameMessage msg; 
    //set dos valores para 0
    memset(&msg, 0, sizeof(msg));

    // Semente para geração aleatória
    srand(time(NULL));
    
    while(1){
        // Solicita jogada ao cliente
        msg.type = MSG_REQUEST;
        snprintf(msg.message, MSG_SIZE,"Escolha sua jogada:\n0 - Nuclear Attack\n1 - Intecept Attack\n2 - Cyber Attack\n3 - Drone Strike\n4 - Bio Attack\n");
        printf("Apresentando as opções para o cliente\n");
        size_t count = send(csock, &msg, sizeof(msg), 0);
        if(count != sizeof(msg)){
            logexit("send");
        }
        //Recebe a jogada do cliente
        recv(csock, &msg, sizeof(msg), 0);
        
        // Verifica se a jogada é válida
        if (msg.client_action < 0 || msg.client_action > 4) {
        
            msg.type = MSG_ERROR;
            printf("Cliente escolheu %d.\n",msg.client_action);
            printf("Erro: Opção inválida de jogada.\n");
            snprintf(msg.message, MSG_SIZE, "Por favor, selecione um valor de 0 a 4.\n");
            send(csock, &msg, sizeof(msg), 0);
            continue;
        }

        // Gera jogada aleatória do servidor
        msg.server_action = rand() % 5;
        printf("Cliente escolheu %d.\n",msg.client_action);
        printf("Servidor escolheu aleatoriamente %d.\n",msg.server_action);
        
        //Verificação do resultado
        result = verificar_resultado(msg.client_action, msg.server_action);
        nome_acao_cliente = verifica_nome_acao_cliente(msg);
        nome_acao_servidor = verifica_nome_acao_servidor(msg);
        
        //Caso o resultado seja de empate solicita ao cliente mais uma escolha
        if(result == -1){
            msg.type = MSG_RESULT;
            msg.result = -1;
            printf("Jogo empatado.\n");
            printf("Solicitando ao cliente mais uma escolha.\n");
            snprintf(msg.message, MSG_SIZE, "Você escolheu: %s\nServidor escolheu: %s\nResultado: Empate!\n",nome_acao_cliente,nome_acao_servidor);
            send(csock, &msg, sizeof(msg), 0);
            // volta ao início para solicitar nova jogada
            continue; 
        }

        //Caso o resultado seja de vitoria ou derrota, envia resultado da jogada
        msg.type = MSG_RESULT;
        msg.result = result;
        if(msg.result == 0){
            string_resultado = "Derrota";
        }
        else if(msg.result == 1){
            string_resultado = "Vitoria";
        }
        
        snprintf(msg.message, MSG_SIZE, "Você escolheu: %s\nServidor escolheu: %s\nResultado: %s!\n",nome_acao_cliente,nome_acao_servidor,string_resultado);
        send(csock, &msg, sizeof(msg), 0);
        if(result == 1){
            vitorias_cliente++;
        } else {
            vitorias_servidor++;
        }
        printf("Placar atualizado: Cliente %d x %d Servidor\n",vitorias_cliente,vitorias_servidor);
        
        //Variavel auxilia para verificar se escolha de nova jogada é valida(0 ou 1)
        int resposta_valida = 0;
        // Pergunta ao cliente se deseja jogar novamente
        while (!resposta_valida) {
            msg.type = MSG_PLAY_AGAIN_REQUEST;
            snprintf(msg.message, MSG_SIZE,"Deseja jogar novamente?\n1 - Sim\n0 - Não\n");
            send(csock, &msg, sizeof(msg), 0);

            count = recv(csock, &msg, sizeof(msg), 0);
            if (count == 0 || count != sizeof(msg)) {
                logexit("recv");
            }
            //caso seja uma escolha valida
            if (msg.client_action == 0 || msg.client_action == 1) {
                resposta_valida = 1; 
            } 
            //caso seja uma escolha errada pede novamente para digitar 0 ou 1
            else {
                msg.type = MSG_ERROR;
                 printf("Perguntando se o cliente deseja jogar novamente.\n");
                printf("Erro: resposta inválida para jogar novamente.\n");
                snprintf(msg.message, MSG_SIZE, "Por favor, digite 1 para jogar novamente ou 0 para encerrar.\n");
                send(csock, &msg, sizeof(msg), 0);
            }
        }
        if(count != sizeof(msg)){
            logexit("recv");
        }
        // Encerra o jogo e envia o placar final  
        if(msg.client_action == 0){
            msg.type = MSG_END;
            msg.client_wins = vitorias_cliente;
            msg.server_wins = vitorias_servidor;
            printf("Cliente não deseja jogar novamente\nEnviando placar final.\nEncerrando conexão.\n");
            snprintf(msg.message, MSG_SIZE, "Fim de jogo!\nPlacar final: Você %d x %d Servidor\nObrigado por jogar!", msg.client_wins,msg.server_wins);
            count = send(csock, &msg, sizeof(msg), 0);
            break;
        }
        printf("Cliente deseja jogar novamente\n");
        
    }
    // Fecha o  socket e finaliza
    close(csock);
    close(s);
    printf("Cliente desconectado.\n");

    return 0;
}
