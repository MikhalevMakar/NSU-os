#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

enum {
    ERROR = -1,
    SIZE_BUFFER = 128,
    INCREASE = 1
};

void communicate_server(int sock_fd) {
    printf("Connected to the UNIX Domain Server. Enter messages:\n");

    ssize_t ret;
    char input[SIZE_BUFFER];

    while (1) {
        memset(input, 0, sizeof(input));
        fgets(input, sizeof(input), stdin);

        ret = write(sock_fd, input, strlen(input));
        if (ret == ERROR) {
            perror("Error call write()\n");
            break;
        }

        ret = read(sock_fd, input, sizeof(input));
        if (ret == ERROR) {
            perror("Error call read()\n");
            break;
        }

        printf("Server response: %s\n", input);
    }
}

int main() {
    int sock_fd;
    struct sockaddr_un server_addr;

    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == ERROR) {
        perror("Error call socket()\n");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, "./dsock_file", sizeof(server_addr.sun_path) - INCREASE);

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == ERROR) {
        perror("Error call connect()\n");
        exit(EXIT_FAILURE);
    }

    communicate_server(sock_fd);

    close(sock_fd);

    return EXIT_SUCCESS;
}