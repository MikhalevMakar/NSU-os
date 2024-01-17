#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include "../header/cache_list.h"

#include <assert.h>
#include "server.c"

typedef struct {
    int client_socket;
    char *request;
} context;

int server_is_on = 1;
Cache *cache;
sem_t thread_semaphore;

int init_cache() {
    logg("Initializing cache", GREEN);
    cache = malloc(sizeof(Cache));
    if (cache == NULL) {
        return EXIT_FAILURE;
    }
    cache->next = NULL;
    return EXIT_SUCCESS;
}

void destroy_cache() {
    logg("Destroying cache", GREEN);
    pthread_mutex_lock(&cache_mutex);
    Cache *cur = cache;
    while (cur != NULL) {
        delete_cache_record(cur);
        Cache *next = cur->next;
        free(cur);
        cur = next;
    }
    pthread_mutex_unlock(&cache_mutex);
}

int send_from_cache(char *request, int client_socket) {
    char *cache_record = calloc(CACHE_BUFFER_SIZE, sizeof(char));
    ssize_t len = find_in_cache(cache, request, cache_record);

    if (len != NOT_FOUND_CACHE) {
        ssize_t send_bytes = write(client_socket, cache_record, len);
        if (send_bytes == WRITE_ERROR) {
            perror("Error while sending cached data");
            close(client_socket);
            free(cache_record);
            return EXIT_FAILURE;
        }
        free(cache_record);
        logg_int("Send cached response to the client, len = ", send_bytes, PURPLE);
        printf("\n");
        close(client_socket);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

void* execute_client_request(void *arg) {
    context *ctx = (context *) arg;
    int client_socket = ctx->client_socket;
    char *request0 = ctx->request;
    char request[MAX_BUFFER_SIZE];
    strcpy(request, request0);

    Cache *record = malloc(sizeof(Cache));
    init_cache_record(record);
    add_request(record, request0);

    unsigned char host[HOST_SIZE];
    const unsigned char *host_result = memccpy(host, strstr((char *) request, HOST) + 6, '\r', sizeof(host));
    host[host_result - host - 1] = END_STR;
    logg_char("Remote server host name: ", (char *) host, GREEN);

    int dest_socket = connect_to_remote((char *) host);
    if (dest_socket == SOCKET_ERROR) {
        close(client_socket);
    }
    logg("Create new connection with remote server", GREEN);

    ssize_t bytes_sent = write(dest_socket, request, strlen(request));
    if (bytes_sent == WRITE_ERROR) {
        perror("Error while sending request to remote server");
        close(client_socket);
        close(dest_socket);
        return NULL;
    }
    logg_int("Send request to remote server, len = ", bytes_sent, GREEN);

    char *buffer = calloc(MAX_BUFFER_SIZE, sizeof(char));
    ssize_t bytes_read, all_bytes_read = 0;
    while ((bytes_read = read(dest_socket, buffer, MAX_BUFFER_SIZE)) > 0) {
        bytes_sent = write(client_socket, buffer, bytes_read);
        if (bytes_sent == SEND_ERROR) {
            perror("Error while sending data to client");
            close(client_socket);
            close(dest_socket);
            return NULL;
        } else
            add_response(record, buffer, all_bytes_read, bytes_read);

        all_bytes_read += bytes_read;
    }
    add_size(record, all_bytes_read);
    push_record(cache, record);

    close(client_socket);
    close(dest_socket);
    free(buffer);
    free(request0);

    sem_post(&thread_semaphore);
    return NULL;
}

void accept_new_client(int server_socket) {
    int client_socket;
     while (server_is_on) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_size);
        if (client_socket == SOCKET_ERROR) {
            perror("Error to accept");
            close(server_socket);
            destroy_cache();
            exit(EXIT_FAILURE);
        }
        char *buff = calloc(MAX_BUFFER_SIZE, sizeof(char));
        sprintf(buff, "Client connected from %s:%d", inet_ntoa(client_addr.sin_addr),
                                                     ntohs(client_addr.sin_port));
        logg(buff, BLUE);
        free(buff);

        char *request = calloc(MAX_BUFFER_SIZE, sizeof(char));
        assert(request != NULL);
        int err = read_request(client_socket, request);
        if (err == EXIT_FAILURE) {
            logg("Failed to read request", RED);
            free(request);
            close(client_socket);
            continue;
        }

        if (send_from_cache(request, client_socket) == EXIT_SUCCESS) {
            free(request);
            close(client_socket);
            continue;
        } else {
            sem_wait(&thread_semaphore);
            logg("Init new connection", PURPLE);
            context ctx = {client_socket, request};
            pthread_t handler_thread;
            err = pthread_create(&handler_thread, NULL, execute_client_request, &ctx);
            if (err == PTHREAD_ERROR) {
                perror("Failed to create thread");
                close(client_socket);
                close(server_socket);
                destroy_cache();
                exit(EXIT_FAILURE);
            }
        }
    }
}

void run_proxy() {
    int server_socket = create_server_socket();
    if (server_socket == SOCKET_ERROR) {
        perror("Error to create server socket");
        exit(EXIT_FAILURE);
    }

    int err = init_cache();
    if (err == EXIT_FAILURE) {
        perror("Error to init cache");
        destroy_cache();
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    logg_int("Server listening on port ", PORT, GREEN);

    accept_new_client(server_socket);
    close(server_socket);
    destroy_cache();
}