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

    bytes_read = recv(client_socket, request_buffer, sizeof(request_buffer) - 1, 0);
    if (bytes_read <= 0) {
        char response[] = "HTTP/1.1 408 Request Timeout\r\n"
                          "Content-Type: text/plain\r\n"
                          "Connection: close\r\n\r\n"
                          "Erro: Conexao perdida com o servidor.\n";
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return;
    }
    request_buffer[bytes_read] = '\0';
    
    sscanf(request_buffer, "%15s %255s", method, path);
    printf("Requisicao Recebida: %s %s\n", method, path);

    if (strcmp(method, "POST") == 0 && strcmp(path, "/upload") == 0) {
        char* cl_header = strstr(request_buffer, "Content-Length: ");
        if (cl_header) {
            content_length = atol(cl_header + 16);
        }

        char* body_start_ptr = strstr(request_buffer, "\r\n\r\n");
        if (body_start_ptr) {
            body_start_ptr += 4;
            size_t body_already_read = bytes_read - (body_start_ptr - request_buffer);

            body = malloc(content_length + 1);
            if (!body) {
                char response[] = "HTTP/1.1 500 Internal Server Error\r\n"
                                  "Content-Type: text/plain\r\n\r\n"
                                  "Erro: Falha de memoria no servidor.\n";
                send(client_socket, response, strlen(response), 0);
                close(client_socket);
                return;
            }
            
            memcpy(body, body_start_ptr, body_already_read);
            size_t total_read = body_already_read;
            while (total_read < content_length) {
                bytes_read = recv(client_socket, body + total_read, content_length - total_read, 0);
                if (bytes_read <= 0) {
                    char response[] = "HTTP/1.1 408 Request Timeout\r\n"
                                      "Content-Type: text/plain\r\n\r\n"
                                      "Erro: Conexao perdida durante upload.\n";
                    send(client_socket, response, strlen(response), 0);
                    free(body);
                    close(client_socket);
                    return;
                }
                total_read += bytes_read;
            }
            body[total_read] = '\0';
            
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
