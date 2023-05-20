#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

enum constant {
    ERROR = -1,
    CHILDREN_PROC = 0
};

//watch -d -n1 cat /proc/pid/maps

//watch -d -n1 ps  --pid pid_parent --pid pid_children

int global_value = 10;

void run_task() {
    int local_value = 5;
    printf("Global variable address: %p and value: %d\n", (void*)&global_value, global_value);
    printf("Local  variable address: %p and value: %d\n", (void*)&local_value, local_value);

    pid_t pid_process = getpid();
    printf("Parent pid: %d\n", pid_process);
    sleep(10);

    pid_t child_pid = fork();

    if (child_pid <= ERROR) {
        perror("Error when calling fork()\n");
        exit(EXIT_FAILURE);
    } else if(child_pid == CHILDREN_PROC) {
        printf("Child's PID: %d\n", getpid());
        printf("Child's parent PID: %d\n", getppid());

        printf("Child's, global variable address: %p and value: %d\n", (void*)&global_value, global_value);
        printf("Child's, local  variable address: %p and value: %d\n", (void*)&local_value, local_value);

        local_value = 30;
        global_value = 40;

        printf("Child's, global variable address: %p and value: %d\n", (void*)&global_value, global_value);
        printf("Child's, local  variable address: %p and value: %d\n", (void*)&local_value, local_value);
        sleep(5);
        exit(5);
    } else {

        printf("Parent, global variable address: %p and value: %d\n", (void*)&global_value, global_value);
        printf("Parent, local  variable address: %p and value: %d\n", (void*)&local_value, local_value);

        sleep(15);

        int exit_status;
        pid_t child_status = waitpid(child_pid, &exit_status, 0);
        if (child_status == ERROR) {
            perror("Error when calling waitpid");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(child_status)) {
            printf("Child process with PID: %d,  ended with the code %d\n", child_status, WEXITSTATUS(child_status));
        }

        printf("Parent, global variable address: %p and value: %d\n", (void*)&global_value, global_value);
        printf("Parent, local  variable address: %p and value: %d\n", (void*)&local_value, local_value);

        sleep(5);
    }
}

int main() {
    run_task();
    return EXIT_SUCCESS;
}
