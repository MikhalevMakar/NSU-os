#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>

#include "enum_const.h"

void server_http_response(int client_socket, const char* content) {
    const char* response_template = "HTTP/1.1 200 OK\r\n"
                                    "Content-Length: %zu\r\n"
                                    "Content-Type: text/html\r\n"
                                    "\r\n"
                                    "%s";

    char response[SIZE_RESPONSE];
    unsigned int response_length = snprintf(response, sizeof(response),
                                            response_template, strlen(content), content);

    write(client_socket, response, response_length);
}

void server_http_request(int server_socket, char* host, char* path) {
char request_buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_sent;

    snprintf(request_buffer, sizeof(request_buffer), "GET / HTTP/1.1\r\nHost: ya.ru\r\nContent-Type: application/x-www-form-\\r\n\r\n");

    bytes_sent = send(server_socket, request_buffer, strlen(request_buffer), START_POS_WRITE);

    if (bytes_sent == -1)
        perror("Error sending HTTP request to the server");
    else
        printf("SUCCESSFUL request: get packet\n");
}

int create_socket_server(struct sockaddr_in* server_address) {
    int serverSocket;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(server_address, 0, sizeof(*server_address));
    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(PORT);
    server_address->sin_addr.s_addr = INADDR_ANY;
    return serverSocket;
}

void init_socket(int serverSocket, struct sockaddr_in* serverAddr) {
    if (bind(serverSocket, (struct sockaddr*)serverAddr, sizeof(*serverAddr)) == -1) {
        perror("Error binding socket");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, BACKLOG) == SOCKET_ERROR) {
        perror("Error listening on socket");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
}

int connect_to_remote_server(char *host) {
    struct addrinfo hints, *res0;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo((char *) host, "http", &hints, &res0);
    if (status != 0) {
        freeaddrinfo(res0);
        return -1;
    }
    int dest_socket = socket(res0->ai_family, res0->ai_socktype, res0->ai_protocol);
    if (dest_socket == SOCKET_ERROR) return SOCKET_ERROR;
1
    int err = connect(dest_socket, res0->ai_addr, res0->ai_addrlen);
    if (err == SOCKET_ERROR) {
        close(dest_socket);
        freeaddrinfo(res0);
        return SOCKET_ERROR;
    }
    return dest_socket;
}


