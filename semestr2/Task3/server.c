#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>

#include "enum_const.h"

typedef struct context_request {
    char request[MAX_BUFFER_SIZE];
    char host[SIZE_URL];
    char path[SIZE_URL];
    char* url;
} ContextRequest;

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void server_http_response(int client_socket, const char* content) {
    const char* response_template = "HTTP/1.0 200 OK\r\n"
                                    "Content-Length: %zu\r\n"
                                    "Content-Type: text/html\r\n"
                                    "\r\n"
                                    "%s";

    char response[SIZE_RESPONSE];
    unsigned int response_length = snprintf(response, sizeof(response),
                                            response_template, strlen(content), content);

    write(client_socket, response, response_length);
}

void server_http_request(int server_socket, ContextRequest* contextRequest) {
    char request[MAX_BUFFER_SIZE];
    snprintf(request, MAX_BUFFER_SIZE, "GET / HTTP/1.0\r\nHost: %s\r\n\r\n", contextRequest->host);

    if (send(server_socket, request, strlen(request), START_POS_WRITE) == SEND_ERROR)
        perror("Error sending HTTP request to the server");
    else
        printf("SUCCESSFUL request: get packet\n");
}

void receive_response(int sock_fd) {
    char response[MAX_BUFFER_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = recv(sock_fd, response, MAX_BUFFER_SIZE - 1, 0)) > 0) {
        response[bytes_received] = '\0';
        printf("%s", response);
    }

    if (bytes_received < 0)
        error("Error receiving response");
}

int create_socket_server(struct sockaddr_in* server_address) {
    int serverSocket;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR)
        error("Error creating socket");

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

int connect_to_remote_server(char* host) {
    struct hostent *server = gethostbyname(host);
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        error("Error opening socket");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(PORT);

    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        error("Error connecting");

    return socket_fd;
}