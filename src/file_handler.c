#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>

// Define o diretório onde os arquivos serão armazenados
#define FILES_DIR "./files"
// Tamanho do buffer para leitura/escrita de arquivos
#define BUFFER_SIZE 4096

// Função utilitária para buscar uma sequência de bytes (agulha) em outra (palheiro)
void* find_mem(const void* haystack, size_t haystack_len, const void* needle, size_t needle_len) {
    if (needle_len == 0) return (void*)haystack; // Se a agulha é vazia, retorna o início
    if (haystack_len < needle_len) return NULL;  // Se o palheiro é menor que a agulha, não pode conter

    const char* h = (const char*)haystack;
    const char* n = (const char*)needle;

    // Itera através do palheiro para encontrar a primeira ocorrência da agulha
    for (size_t i = 0; i <= haystack_len - needle_len; i++) {
        // memcmp compara blocos de memória
        if (memcmp(h + i, n, needle_len) == 0) {
            return (void*)(h + i);
        }
    }
    return NULL;
}

// Retorna o tipo MIME baseado na extensão do arquivo
const char* get_mime_type(const char* filename) {
    const char* dot = strrchr(filename, '.'); // Encontra o último ponto
    if (!dot) return "application/octet-stream"; // Se não tem extensão
    if (strcmp(dot, ".html") == 0) return "text/html";
    if (strcmp(dot, ".css") == 0) return "text/css";
    if (strcmp(dot, ".jpg") == 0) return "image/jpeg";
    if (strcmp(dot, ".png") == 0) return "image/png";
    return "application/octet-stream"; // Tipo genérico para binários desconhecidos
}

// Gera e envia a página HTML com a lista de arquivos e o formulário de upload
void send_file_list_page(int client_socket) {
    char buffer[BUFFER_SIZE];
    char file_list_html[BUFFER_SIZE] = "";
    DIR *dir;
    struct dirent *entry;

    // Cria o diretório de arquivos se ele não existir
    mkdir(FILES_DIR, 0755);
    // Abre o diretório para leitura
    dir = opendir(FILES_DIR);

    if (dir) {
        // Itera sobre as entradas do diretório
        while ((entry = readdir(dir)) != NULL) {
            // Se for um arquivo regular (não diretório)
            if (entry->d_type == DT_REG) {
                char item[512];
                // Cria um item de lista HTML para o arquivo
                snprintf(item, sizeof(item), "<li><a href=\"/%s\" download>%s</a></li>", entry->d_name, entry->d_name);
                strcat(file_list_html, item); // Adiciona ao HTML da lista
            }
        }
        closedir(dir);
    }

    // Monta a resposta HTTP completa
    snprintf(buffer, sizeof(buffer),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n\r\n"
        "<!DOCTYPE html><html><head><title>Servidor C</title>"
        "<style>body{font-family:sans-serif;background:#f0f0f0;color:#333;}"
        "div{background:white;padding:20px;margin:20px auto;max-width:800px;border-radius:5px;}"
        "ul{list-style:none;padding:0;} li a{text-decoration:none;color:blue;}"
        "form{margin-top:20px;}</style></head>"
        "<body><div><h1>Arquivos Disponiveis</h1><ul>%s</ul>"
        "<h2>Upload de Arquivo</h2>"
        "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">"
        "<input type=\"file\" name=\"file\" required>"
        "<button type=\"submit\">Enviar</button></form>"
        "</div></body></html>",
        strlen(file_list_html) > 0 ? file_list_html : "<li>Nenhum arquivo encontrado.</li>"
    );

    // Envia a resposta HTTP para o cliente
    send(client_socket, buffer, strlen(buffer), 0);
}

// Envia um arquivo para download
void send_file_download(int client_socket, const char* filename) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", FILES_DIR, filename);

    // Tenta abrir o arquivo para leitura em modo binário
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        // Se o arquivo não for encontrado, envia uma resposta 404 Not Found
        char response[] = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, response, strlen(response), 0);
        return;
    }

    // Obtém o tamanho do arquivo
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Prepara e envia os cabeçalhos de resposta HTTP
    char headers[512];
    snprintf(headers, sizeof(headers),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Content-Disposition: attachment; filename=\"%s\"\r\n\r\n",
        get_mime_type(filename), file_size, filename);
    send(client_socket, headers, strlen(headers), 0);

    // Lê o arquivo em blocos e envia o conteúdo para o cliente
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }
    fclose(file); // Fecha o arquivo
}

// Processa o corpo de uma requisição de upload (multipart/form-data)
void handle_file_upload(int client_socket, const char* request_body, size_t body_len) {
    // A requisição de upload usa o formato "multipart/form-data",
    // onde os dados são divididos em partes por um "boundary".

    // Encontra o nome do arquivo na parte do cabeçalho de upload
    char* filename_ptr = strstr(request_body, "filename=\"");
    if (!filename_ptr) return;
    filename_ptr += 10;
    char* filename_end_ptr = strchr(filename_ptr, '"');
    if (!filename_end_ptr) return;

    // Copia o nome do arquivo para uma nova string
    char filename[256];
    strncpy(filename, filename_ptr, filename_end_ptr - filename_ptr);
    filename[filename_end_ptr - filename_ptr] = '\0';
    
    // Encontra o início dos dados binários do arquivo (após o \r\n\r\n)
    char* data_start = strstr(request_body, "\r\n\r\n");
    if (!data_start) return;
    data_start += 4; // Avança 4 bytes para pular o \r\n\r\n

    size_t header_len = data_start - request_body;
    
    // Encontra o "boundary" que marca o fim do arquivo
    char boundary[128];
    char* boundary_end = strstr(request_body, "\r\n");
    if(!boundary_end) return;
    strncpy(boundary, request_body, boundary_end - request_body);
    boundary[boundary_end - request_body] = '\0';
    
    // Usa a nossa função find_mem para encontrar a ocorrência do boundary de fechamento
    char* end_data_ptr = find_mem(data_start, body_len - header_len, boundary, strlen(boundary));
    
    size_t data_len = body_len - header_len;
    if (end_data_ptr) {
        // Calcula o tamanho exato dos dados do arquivo
        data_len = end_data_ptr - data_start - 2; // -2 para remover o \r\n antes do boundary
    }

    // Constrói o caminho completo onde o arquivo será salvo
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", FILES_DIR, filename);

    // Salva os dados do arquivo em disco
    FILE* file = fopen(filepath, "wb");
    if (file) {
        fwrite(data_start, 1, data_len, file);
        fclose(file);
        printf("Arquivo '%s' salvo com sucesso.\n", filename);
    }

    // Envia uma resposta de redirecionamento (303 See Other)
    // Isso faz com que o navegador recarregue a página inicial com a lista de arquivos atualizada
    char response[] = "HTTP/1.1 303 See Other\r\nLocation: /\r\n\r\n";
    send(client_socket, response, strlen(response), 0);
}