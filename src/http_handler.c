#include "http_handler.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define REQUEST_BUFFER_SIZE 8192

void handle_connection(int client_socket) {
    char* request_buffer = malloc(REQUEST_BUFFER_SIZE);
    if (!request_buffer) return;

    // Lê toda a requisição
    ssize_t bytes_read = recv(client_socket, request_buffer, REQUEST_BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) {
        free(request_buffer);
        close(client_socket);
        return;
    }
    request_buffer[bytes_read] = '\0';
    
    // Extrai método e caminho
    char method[16], path[256];
    sscanf(request_buffer, "%15s %255s", method, path);
    printf("Requisicao Recebida: %s %s\n", method, path);

    if (strcmp(method, "GET") == 0) {
        if (strcmp(path, "/") == 0) {
            send_file_list_page(client_socket);
        } else {
            send_file_download(client_socket, path + 1); // Remove a '/' inicial
        }
    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/upload") == 0) {
        // Encontra o corpo da requisição
        char* body = strstr(request_buffer, "\r\n\r\n");
        if (body) {
            body += 4;
            // O Content-Length é necessário para saber o tamanho exato do corpo
            char* cl_header = strstr(request_buffer, "Content-Length: ");
            if (cl_header) {
                long content_length = atol(cl_header + 16);
                handle_file_upload(client_socket, body, content_length);
            }
        }
    } else {
        // Resposta para métodos não suportados
        char response[] = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
    }
    
    free(request_buffer);
    close(client_socket);
    printf("Conexao fechada.\n\n");
}