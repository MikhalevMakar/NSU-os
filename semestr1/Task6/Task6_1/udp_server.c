#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main() {
    int sock_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    if (bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind() failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("UDP Server started\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        client_addr_len = sizeof(client_addr);
        ssize_t recv_len = recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (recv_len == -1) {
            perror("recvfrom() failed");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }

        printf("Received data from client: %s\n", buffer);

        if (sendto(sock_fd, buffer, recv_len, 0, (struct sockaddr*)&client_addr, client_addr_len) == -1) {
            perror("sendto() failed");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
    }

    close(sock_fd);
    return 0;
}