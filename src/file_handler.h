#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <stdio.h>

void send_file_list_page(int client_socket);
void send_file_download(int client_socket, const char* filename);
void handle_file_upload(int client_socket, const char* request_body, size_t body_len);

#endif // FILE_HANDLER_H