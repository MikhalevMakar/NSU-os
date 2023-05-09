#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv) {
    pid_t pid_process = getpid();
    printf("process pid: %d\n", pid_process);

    execv(argv[0], argv);
    return EXIT_SUCCESS;
}