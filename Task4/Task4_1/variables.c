#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/// proc/<pid>/maps

enum constants_utility {
    VALUE = 20,
    TIME_SLEEP = 10,
    OKEY = 0
};

int my_global_init = VALUE;
int my_global_not_init;
const int my_global_const = VALUE;

void display_addr_variables() {
    int my_local_init = VALUE;
    int my_local_not_init;

    static int my_static_init = VALUE;
    static int my_static_not_init;

    const int my_const_init = VALUE;

    printf("my_local_init -> PTR: %p VALUE: %d\n", &my_local_init, my_local_init);
    printf("my_local_not_init -> PTR: %p VALUE: %d\n", &my_local_not_init, my_local_not_init);

    printf("my_static_init -> PTR: %p VALUE: %d\n", &my_static_init, my_static_init);
    printf("my_static_not_init -> PTR: %p VALUE: %d\n", &my_static_not_init, my_static_not_init);

    printf("my_const_init -> PTR: %p VALUE: %d\n", &my_const_init, my_const_init);

    printf("global_init -> PTR: %p VALUE: %d\n", &my_global_init, my_global_init);
    printf("global_not_init -> PTR: %p VALUE: %d\n", &my_global_not_init, my_global_not_init);
    printf("global_const -> PTR: %p VALUE: %d\n", &my_global_const, my_global_const);

    unsigned int ret = sleep(TIME_SLEEP);
    if(ret != OKEY) {
        fprintf(stderr, "Error in func sleep!\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv) {
    pid_t pid_process = getpid();
    printf("process pid: %d\n", pid_process);

    display_addr_variables();
    return EXIT_SUCCESS;
}
