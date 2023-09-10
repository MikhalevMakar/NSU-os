#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

enum {
    COUNT_SYMBOL_HELLO_WORLD = 11
};

void* my_thread(void* arg) {
    char* value = (char*)malloc(sizeof(char)*COUNT_SYMBOL_HELLO_WORLD);
    if (value == NULL) {
        perror("my_thread: malloc failed");
        pthread_exit(NULL);
    }

    strcpy(value, "hello world");
    pthread_exit(value);
}

int main() {
    pthread_t tid;
    int err;

    err = pthread_create(&tid, NULL, my_thread, NULL);
    if (err) {
        fprintf(stderr, "main: pthread_create() failed: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    void* ret_val;
    err = pthread_join(tid, &ret_val);
    if (err) {
        fprintf(stderr, "main: pthread_join() failed %s\n", strerror(err));
        return EXIT_FAILURE;
    }
    printf("ret_val_hello_world %s\n", (char*)ret_val);
    free(ret_val_hello_world);
    return EXIT_SUCCESS;
}
