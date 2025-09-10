#include "http_handler.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define REQUEST_BUFFER_SIZE 8192

void handle_connection(int client_socket) {
    char request_buffer[REQUEST_BUFFER_SIZE];
    ssize_t bytes_read;
    char method[16], path[256];
    char* body = NULL;
    long content_length = 0;

    // 1. Lê a primeira parte da requisição (geralmente os cabeçalhos)
    bytes_read = recv(client_socket, request_buffer, sizeof(request_buffer) - 1, 0);
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    request_buffer[bytes_read] = '\0';
    
    // Extrai o método e o caminho
    sscanf(request_buffer, "%15s %255s", method, path);
    printf("Requisicao Recebida: %s %s\n", method, path);

    // Se a requisição for um POST para /upload
    if (strcmp(method, "POST") == 0 && strcmp(path, "/upload") == 0) {
        // Encontra o cabeçalho Content-Length
        char* cl_header = strstr(request_buffer, "Content-Length: ");
        if (cl_header) {
            content_length = atol(cl_header + 16);
        }

        // Encontra o início do corpo da requisição
        char* body_start_ptr = strstr(request_buffer, "\r\n\r\n");
        if (body_start_ptr) {
            body_start_ptr += 4;
            
            // Calcula quantos bytes do corpo já foram lidos no primeiro recv
            size_t body_already_read = bytes_read - (body_start_ptr - request_buffer);

            // Aloca memória para armazenar o corpo completo da requisição
            body = malloc(content_length + 1);
            if (!body) {
                close(client_socket);
                return;
            }
            
            // Copia a parte do corpo já lida
            memcpy(body, body_start_ptr, body_already_read);
            
            // Loop para ler o restante do corpo
            size_t total_read = body_already_read;
            while (total_read < content_length) {
                bytes_read = recv(client_socket, body + total_read, content_length - total_read, 0);
                if (bytes_read <= 0) {
                    free(body);
                    close(client_socket);
                    return;
                }
                total_read += bytes_read;
            }
            body[total_read] = '\0'; // Adiciona o terminador nulo para segurança
            
            // Chama a função de upload com o corpo completo
            handle_file_upload(client_socket, body, total_read);
            free(body);
        }

    } else if (strcmp(method, "GET") == 0) {
        if (strcmp(path, "/") == 0) {
            send_file_list_page(client_socket);
        } else {
            send_file_download(client_socket, path + 1);
        }
    } else {
        char response[] = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
    }
    
    close(client_socket);
    printf("Conexao fechada.\n\n");
}