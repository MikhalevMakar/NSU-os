#include <stdio.h>
#include <assert.h>
#include <netinet/in.h>

#include "server.c"

#define REGEX_URL "https://%255[^/]/%255[^\r\n]"

typedef struct context_request {
    char request[MAX_BUFFER_SIZE];
    char host[SIZE_URL];
    char path[SIZE_URL];
    char url[SIZE_URL];
} context_request;

context_request* find_url_request(int client_socket) {
    char buffer[MAX_BUFFER_SIZE];

    unsigned int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, START_POS_READ);
    printf("%s", buffer);

    buffer[bytes_received] = '\0';

    char* url_start = strstr(buffer, TYPE_GET) + SIZE_SPACE;
    assert(url_start != NULL);

    url_start += sizeof(TYPE_GET);

    char* url_end = strstr(url_start, VERSION_HTTP);
    assert(url_end != NULL);

    size_t url_length = url_end - url_start;
    char* url = strndup(url_start, url_length);
    assert(url != NULL);

    char host[256];
    char path[256];
    sscanf(url, REGEX_URL, host, path);
    assert(sscanf(url, REGEX_URL, host, path) == 2);

    context_request* context_request = malloc(sizeof(context_request));
    assert(context_request != NULL);
    return context_request;
}

void handle_client(int client_socket) {
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received;
    context_request* context_Request= find_url_request(client_socket);

    int server_socket = connect_to_remote_server(host);
    if(server_socket == SOCKET_ERROR) return;

    printf("url: %s, host: %s, path: %s\n", url, host, path);
    server_http_request(server_socket, host, path);

    printf("57\n");
    while ((bytes_received = recv(server_socket, buffer, sizeof(buffer), 0)) > 0) {
        server_http_response(client_socket, buffer);
        printf("Response from server:\n%.*s\n", (int)bytes_received, buffer);
    }
    printf("67");
    close(client_socket);
    close(server_socket);
}

void run_proxy() {
    printf("Server is listening on port %d...\n", PORT);

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int client_socket, server_socket;

    server_socket = create_socket_server(&serverAddr);

    init_socket(server_socket, &serverAddr);

     while (1) {

         if ((client_socket = accept(server_socket, (struct sockaddr *) &clientAddr, &clientLen)) == ACCEPT_ERROR) {
             perror("Error accepting connection");
             continue;
         }

         printf("Accepted connection from on socket %d\n", client_socket);

         handle_client(client_socket);
         close(client_socket);
     }
}