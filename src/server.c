#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "http_handler.h"

// Define a porta do servidor
#define PORT 8080

// Cria e configura o socket do servidor para aguardar conexões
int create_server_socket() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // Cria o socket do servidor. AF_INET para IPv4, SOCK_STREAM para TCP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Permite a reutilização imediata do endereço e da porta, evitando "Address already in use"
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Erro em setsockopt");
        exit(EXIT_FAILURE);
    }

    // Configura o endereço e a porta do servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Aceita conexões de qualquer IP
    address.sin_port = htons(PORT);       // Converte a porta para a ordem de bytes da rede

    // Vincula o socket ao endereço e porta definidos
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Erro de bind");
        exit(EXIT_FAILURE);
    }

    // Coloca o socket em modo de escuta, com uma fila de até 10 conexões pendentes
    if (listen(server_fd, 10) < 0) {
        perror("Erro de listen");
        exit(EXIT_FAILURE);
    }
    
    return server_fd;
}

// Ponto de entrada do programa
int main() {
    int server_fd = create_server_socket();
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    printf("Servidor rodando em http://localhost:%d\n", PORT);

    // Loop principal para aceitar e processar conexões de clientes
    while (1) {
        printf("Aguardando conexao...\n");
        // Aceita uma nova conexão e retorna um novo socket para o cliente
        int client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("Erro no accept");
            continue; // Continua para a próxima iteração
        }
        
        printf("Cliente conectado!\n");
        // Chama a função para lidar com a requisição HTTP
        handle_connection(client_socket);
    }

    // Fecha o socket do servidor (esta parte nunca é alcançada devido ao loop infinito)
    close(server_fd);
    return 0;
}