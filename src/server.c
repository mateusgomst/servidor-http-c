#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "http_handler.h"

#define PORT 8080

int create_server_socket() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Erro em setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Erro de bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Erro de listen");
        exit(EXIT_FAILURE);
    }
    
    return server_fd;
}

int main() {
    int server_fd = create_server_socket();
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    printf("Servidor rodando em http://localhost:%d\n", PORT);

    while (1) {
        printf("Aguardando conexao...\n");
        int client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("Erro no accept");
            continue;
        }
        
        printf("Cliente conectado!\n");
        handle_connection(client_socket);
    }

    close(server_fd);
    return 0;
}
