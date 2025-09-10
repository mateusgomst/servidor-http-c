#include "http_handler.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

// Tamanho do buffer para ler a requisição HTTP
#define REQUEST_BUFFER_SIZE 8192

// Processa uma conexão de cliente
void handle_connection(int client_socket) {
    char* request_buffer = malloc(REQUEST_BUFFER_SIZE);
    if (!request_buffer) {
        return;
    }

    // Lê toda a requisição HTTP do socket
    ssize_t bytes_read = recv(client_socket, request_buffer, REQUEST_BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) {
        free(request_buffer);
        close(client_socket);
        return;
    }
    request_buffer[bytes_read] = '\0';
    
    // Extrai o método (GET, POST) e o caminho (/ ou /caminho) da requisição
    char method[16], path[256];
    sscanf(request_buffer, "%15s %255s", method, path);
    printf("Requisicao Recebida: %s %s\n", method, path);

    // Lógica para lidar com diferentes tipos de requisições
    if (strcmp(method, "GET") == 0) {
        // Se o caminho for "/", envia a página com a lista de arquivos
        if (strcmp(path, "/") == 0) {
            send_file_list_page(client_socket);
        } else {
            // Se for outro caminho, tenta enviar o arquivo solicitado
            send_file_download(client_socket, path + 1); // path + 1 para remover a '/' inicial
        }
    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/upload") == 0) {
        // Se for um POST para "/upload", lida com o upload do arquivo
        
        // Encontra o início do corpo da requisição (após a linha em branco)
        char* body = strstr(request_buffer, "\r\n\r\n");
        if (body) {
            body += 4; // Avança 4 bytes para pular o \r\n\r\n
            
            // Procura o cabeçalho Content-Length para saber o tamanho exato do corpo
            char* cl_header = strstr(request_buffer, "Content-Length: ");
            if (cl_header) {
                // Extrai o valor do Content-Length e chama a função de upload
                long content_length = atol(cl_header + 16);
                handle_file_upload(client_socket, body, content_length);
            }
        }
    } else {
        // Resposta para métodos ou caminhos não suportados
        char response[] = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
    }
    
    // Libera a memória e fecha o socket do cliente
    free(request_buffer);
    close(client_socket);
    printf("Conexao fechada.\n\n");
}