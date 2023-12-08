#include <stdio.h>
#include <assert.h>
#include <netinet/in.h>

#include "server.c"

#define REGEX_URL "https://%255[^/]/%255[^\r\n]"

ContextRequest* find_url_request(int client_socket) {
    ContextRequest* context_request = malloc(sizeof(ContextRequest));
    assert(context_request != NULL);

    unsigned int bytes_received = recv(client_socket, context_request->request, sizeof(context_request->request) - 1, START_POS_READ);

    context_request->request[bytes_received] = '\0';
    printf("request %s\n", context_request->request);

    char* url_start = strstr(context_request->request, TYPE_GET) + SIZE_SPACE;
    assert(url_start != NULL);

    url_start += sizeof(TYPE_GET);

    char* url_end = strstr(url_start, VERSION_HTTP);
    assert(url_end != NULL);

    size_t url_length = url_end - url_start;

    context_request->url = strndup(url_start, url_length);
    assert(context_request->url != NULL);

    sscanf(context_request->url, REGEX_URL, context_request->host, context_request->path);
//    assert(sscanf(context_request->url, REGEX_URL, context_request->host, context_request->path) == 2);

    return context_request;
}

void send_request(int sockfd, const char *hostname) {
    char request[MAX_BUFFER_SIZE];
    snprintf(request, MAX_BUFFER_SIZE, "GET / HTTP/1.0\r\nHost: %s\r\n\r\n", hostname);
    if (send(sockfd, request, strlen(request), 0) < 0)
        error("Error sending request");
}

void receive_response1(int sockfd) {
    char response[MAX_BUFFER_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = recv(sockfd, response, MAX_BUFFER_SIZE - 1, 0)) > 0) {
        response[bytes_received] = '\0';
        printf("%s", response);
    }

    if (bytes_received < 0) {
        error("Error receiving response");
    }
}

void handle_client(int client_socket) {
    ContextRequest* context_request = find_url_request(client_socket);
    printf("url: %s, host: %s, path: %s\n", context_request->url, context_request->host, context_request->path);

    int server_socket = connect_to_remote_server("ya.ru");
    if (server_socket == SOCKET_ERROR) {
        perror("Error connecting to remote server");
        free(context_request);
        close(client_socket);
        return;
    }
    send_request(server_socket, context_request->host);
    receive_response1(server_socket);

    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received;
    printf("78\n");
    while ((bytes_received = recv(server_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        write(client_socket, buffer, (int) bytes_received);
        printf("Response from server:\n%.*s\n", (int)bytes_received, buffer);
    }

    close(client_socket);
    close(server_socket);
    free(context_request);
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


