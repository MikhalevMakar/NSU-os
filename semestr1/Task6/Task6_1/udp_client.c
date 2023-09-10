#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define SERVER_IP "127.0.0.1"
#define PORT 8888
#define BUFFER_SIZE 1024

int main() {
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton() failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    while (1) {

        printf("Enter data to send (or 'q' to quit): ");
        fgets(buffer, BUFFER_SIZE, stdin);

        if (sendto(sock_fd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            perror("sendto() failed");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }

        memset(buffer, 0, BUFFER_SIZE);
        ssize_t recv_len = recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (recv_len == -1) {
            perror("recvfrom() failed");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }

        printf("Received response from server: %s\n", buffer);
    }
}