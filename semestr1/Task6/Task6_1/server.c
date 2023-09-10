#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>

enum {
    ERROR = -1,
    SIZE_BUFFER = 5,
    CHILD_PROC = 0,
    STATUS_RUN = 1,
    INCREASE = 1
};

void sigchld_handler(int signum) {
    int exit_status;
    pid_t child_status = wait(&exit_status);

    if (child_status == ERROR)
        perror("Error when calling waitpid");
}

void communicate_client(int clt_sock) {
    while (STATUS_RUN) {
        int ret;
        char buff[SIZE_BUFFER];

        ret = read(clt_sock, buff, sizeof(buff));
        printf("Send data: %s", buff);
        if (ret == ERROR) {
            printf("Stopping\n");
            break;
        }

        ret = write(clt_sock, buff, ret);
        if (ret == ERROR) {
            printf("Stopping\n");
            break;
        }
        memset(buff, 0, SIZE_BUFFER);
    }
}

int main() {
    int sock_fd, clt_sock;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_addr_len;

    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == ERROR) {
        perror("socket() failed\n");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, "socket_path", sizeof(server_addr.sun_path) - INCREASE);
    strcpy(server_addr.sun_path, "./dsock_file");

    int returnValueBind = bind(sock_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr));

    if (returnValueBind == -1){
        perror("Error: bind in server");
        exit(EXIT_FAILURE);
    }


    if (bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == ERROR) {
        perror("bind() failed\n");
        exit(EXIT_FAILURE);
    }

    if (listen(sock_fd, 5) == ERROR) {
        perror("Error call listen()\n");
        exit(EXIT_FAILURE);
    }

    printf("UNIX Domain Server started\n");

    signal(SIGCHLD, sigchld_handler);

    while (STATUS_RUN) {
        client_addr_len = sizeof(client_addr);
        memset(&client_addr, 0, sizeof(struct sockaddr_un));
        clt_sock = accept(sock_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (clt_sock == ERROR) {
            perror("Error call accept()\n");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }

        int pid = fork();

        if (pid == ERROR) {
            perror("Error call fork()\n");
            close(clt_sock);
            close(sock_fd);
            exit(EXIT_FAILURE);
        } else if (pid == CHILD_PROC) {
            break;
        }

        close(clt_sock);
    }

    close(sock_fd);

    communicate_client(clt_sock);

    close(clt_sock);

    unlink(server_addr.sun_path);
    return EXIT_SUCCESS;
}