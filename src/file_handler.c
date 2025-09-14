#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>

#define FILES_DIR "./files"
#define BUFFER_SIZE 16384

void* find_mem(const void* haystack, size_t haystack_len, const void* needle, size_t needle_len) {
    if (needle_len == 0) return (void*)haystack;
    if (haystack_len < needle_len) return NULL;

    const char* h = (const char*)haystack;
    const char* n = (const char*)needle;

    for (size_t i = 0; i <= haystack_len - needle_len; i++) {
        if (memcmp(h + i, n, needle_len) == 0) {
            return (void*)(h + i);
        }
    }
    return NULL;
}

const char* get_mime_type(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot) return "application/octet-stream";
    if (strcmp(dot, ".html") == 0) return "text/html";
    if (strcmp(dot, ".css") == 0) return "text/css";
    if (strcmp(dot, ".jpg") == 0) return "image/jpeg";
    if (strcmp(dot, ".png") == 0) return "image/png";
    return "application/octet-stream";
}

void send_file_list_page(int client_socket) {
    char buffer[BUFFER_SIZE];
    char file_list_html[BUFFER_SIZE] = "";
    DIR *dir;
    struct dirent *entry;

    mkdir(FILES_DIR, 0755);
    dir = opendir(FILES_DIR);

    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            char path[512];
            snprintf(path, sizeof(path), "%s/%s", FILES_DIR, entry->d_name);

            struct stat st;
            if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
                char item[512];
                snprintf(item, sizeof(item),
                         "<li><a href=\"/%s\" download>%s</a></li>",
                         entry->d_name, entry->d_name);

                strncat(file_list_html, item,
                        sizeof(file_list_html) - strlen(file_list_html) - 1);
            }
        }
        closedir(dir);
    }

    snprintf(buffer, sizeof(buffer),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n\r\n"
        "<!DOCTYPE html><html><head><title>Servidor C</title>"
        "<style>body{font-family:sans-serif;background:#f0f0f0;color:#333;}"
        "div{background:white;padding:20px;margin:20px auto;max-width:800px;border-radius:5px;}"
        "ul{list-style:none;padding:0;} li a{text-decoration:none;color:blue;}"
        "form{margin-top:20px;}</style></head>"
        "<body><div><h1>Arquivos Disponíveis</h1><ul>%s</ul>"
        "<h2>Upload de Arquivo</h2>"
        "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">"
        "<input type=\"file\" name=\"file\" required>"
        "<button type=\"submit\">Enviar</button></form>"
        "</div></body></html>",
        strlen(file_list_html) > 0 ? file_list_html : "<li>Nenhum arquivo encontrado.</li>"
    );

    if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
        perror("Erro ao enviar lista de arquivos");
    }
}

void send_file_download(int client_socket, const char* filename) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", FILES_DIR, filename);

    FILE *file = fopen(filepath, "rb");
    if (!file) {
        char response[] = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char headers[512];
    snprintf(headers, sizeof(headers),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Content-Disposition: attachment; filename=\"%s\"\r\n\r\n",
        get_mime_type(filename), file_size, filename);

    if (send(client_socket, headers, strlen(headers), 0) < 0) {
        perror("Erro ao enviar cabecalho do arquivo");
        fclose(file);
        return;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) < 0) {
            perror("Erro ao enviar arquivo");
            break;
        }
    }
    fclose(file);
}

void handle_file_upload(int client_socket, const char* request_body, size_t body_len) {
    char* filename_ptr = strstr(request_body, "filename=\"");
    if (!filename_ptr) return;
    filename_ptr += 10;
    char* filename_end_ptr = strchr(filename_ptr, '"');
    if (!filename_end_ptr) return;

    char filename[256];
    strncpy(filename, filename_ptr, filename_end_ptr - filename_ptr);
    filename[filename_end_ptr - filename_ptr] = '\0';
    
    char* data_start = strstr(request_body, "\r\n\r\n");
    if (!data_start) return;
    data_start += 4;

    size_t header_len = data_start - request_body;
    
    char boundary[128];
    char* boundary_end = strstr(request_body, "\r\n");
    if(!boundary_end) return;
    strncpy(boundary, request_body, boundary_end - request_body);
    boundary[boundary_end - request_body] = '\0';
    
    char* end_data_ptr = find_mem(data_start, body_len - header_len, boundary, strlen(boundary));
    
    size_t data_len = body_len - header_len;
    if (end_data_ptr) {
        data_len = end_data_ptr - data_start - 2;
    }

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", FILES_DIR, filename);

    FILE* file = fopen(filepath, "wb");
    if (!file) {
        char response[] = "HTTP/1.1 500 Internal Server Error\r\n"
                          "Content-Type: text/plain\r\n\r\n"
                          "Erro: Nao foi possivel salvar o arquivo.\n";
        send(client_socket, response, strlen(response), 0);
        return;
    }

    if (fwrite(data_start, 1, data_len, file) < data_len) {
        char response[] = "HTTP/1.1 500 Internal Server Error\r\n"
                          "Content-Type: text/plain\r\n\r\n"
                          "Erro: Falha ao gravar o arquivo no servidor.\n";
        send(client_socket, response, strlen(response), 0);
        fclose(file);
        return;
    }

    fclose(file);
    printf("Arquivo '%s' salvo com sucesso.\n", filename);

    char response[] = "HTTP/1.1 303 See Other\r\nLocation: /\r\n\r\n";
    send(client_socket, response, strlen(response), 0);
}
