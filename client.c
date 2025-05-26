#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>



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
    struct sockaddr_storage storage;
    //argv[1] = endereço recebido do servido, argv[2] = port
    if(0 != addrparse(argv[1],argv[2], &storage)){
        usage(argc,argv);
    }
    //criação do socket
    int s;
    //socket da internet e tcp
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    //verificação de erros(problemas no dispositivo remoto e com isso nao comunica...)
    //toda vez que dá erro socket retorna -1
    if(s == -1){
        //função para imprimir a mensagem de erro e sair
        logexit("socket");
    }
    
    
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    //função connect
    //se retornar valor diferente de 0 é que houve algum erro
    //para a função conect temos que passar o socket, o endereço e o tamanho do endereço
    if(0 != connect(s, addr, sizeof(storage))){
        logexit("connect");
    }
    //Criação do vetor de endereço do servidor
    char addrstr[BUFSZ];
    addrtostr(addr,addrstr, BUFSZ);
    printf("connected to %s\n",addrstr);
    //vetor de mensagem
    char buf[BUFSZ];
    //inicialização do buffer com 0
    memset(buf, 0, BUFSZ);
    printf("mensagem> ");
    fgets(buf,BUFSZ-1, stdin);
    //envio do dado para o servidor
    //fala o numero de bytes que mandamos transmitir
    size_t count = send(s, buf, strlen(buf)+1,0);
    // se numero de bytes transmitidos na rede != do tamanho de buf deu erro
    if(count != strlen(buf)+1){
        logexit("send");
    }
    memset(buf, 0, BUFSZ);
    unsigned total = 0;
    //para ficarmos recebendo dados do servidor
    while(1){
        //recepção do dado(resposta enviada) pelo servidor
        //o dado irá chegar no socket s e irá ser armazenado no buf e o tanto de dado que irei receber é ate BUFSZ
        //importante notar que a variavel total foi adicionada pois o servidor nao manda o dado todo de uma vez 
        count = recv(s, buf + total, BUFSZ - total, 0);
        //significa que não recebemos nada, logo a conexao está fechada
        if(count == 0){
            //conexao terminada
            break;
        }
        total += count;
    }
    close(s);
    printf("received %u bytes\n",total);
    puts(buf);
    exit(EXIT_SUCCESS);
}