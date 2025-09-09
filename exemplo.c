#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 33333
/*
Escuta na porta 33333.

Quando recebe uma requisição HTTP, extrai o nome do arquivo pedido (a partir do método GET).

Tenta abrir o arquivo com esse nome no sistema (fopen).

Se o arquivo existir, lê seu conteúdo e envia como resposta.

Se o arquivo não existir, responde com um erro 404 simples.

Fecha a conexão e fica esperando nova conexão.
*/
char *lerArq(char *vetor)
{
    char *vet = malloc(100);
    char c;   
    FILE *arq;
    arq = fopen(vetor,"r");
    if(arq == NULL)
    {
        strcpy(vet, "HTTP/1.1 200 OK Content-Type: text/plain\n\nERRO 404");
    }
    else
    {
        int i = 0;
        c = fgetc(arq);
        while(c != EOF)
        {
            vet[i] = c;
            c = fgetc(arq);
            i++;
        }
        fclose(arq);
    }
    return vet;
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket; 
    long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind()");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 10) < 0)
    {
        perror("listen()");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        printf("\nServidor Iniciado\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr )&address, (socklen_t)&addrlen)) < 0)
        {
            perror("accept()");
            exit(EXIT_FAILURE);
        }
        
        char buffer[30000] = {0};
        valread = read(new_socket , buffer, 30000);
        printf("%s\n", buffer);

        char *vet = malloc(64);
        for(int i = 0, j = 5; buffer[j] != ' '; i++, j++)
        {
            vet[i] = buffer[j];
        }

        char *hello = lerArq(vet);
        write(new_socket, hello, strlen(hello));
        printf("Resposta Enviada");
        close(new_socket);
        if(hello)
            free(hello);
        if(vet)        
            free(vet);
    }
    return 0;
}