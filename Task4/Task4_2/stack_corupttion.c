#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//watch -d -n1 cat /proc/pid/maps
//ulimit -a

enum constant {
    SIZE_BUFFER = 4096
};

int i = 0;

void stack_corruption() {
    char buffer[SIZE_BUFFER];
    printf("%d\n", ++i);
    usleep(100000);

    stack_corruption();
}

int main(int argc, char** argv) {
    pid_t pid_process = getpid();
    printf("process pid: %d\n", pid_process);
    sleep(10);

    stack_corruption();

    return EXIT_SUCCESS;
}