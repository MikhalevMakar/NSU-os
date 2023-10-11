#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//// TODO: valgrind

enum {
    COUNT_SYMBOL_HELLO_WORLD = 12
};

void my_thread_handler(void* arg) {
    printf("my thread handler, free() hello world\n");
    char* ptr = (char*) arg;
    free(ptr);
}

void* my_thread(void *arg) {
    int old_state, count_inter = 0;
    char* arr_hello_world = malloc(sizeof(char) * COUNT_SYMBOL_HELLO_WORLD);
    strcpy(arr_hello_world, "hello world");
    if (arr_hello_world == NULL) {
        fprintf(stderr, "failed to allocate memory\n");
        pthread_exit(NULL);
    }

    pthread_cleanup_push(my_thread_handler, (void*)arg);

    printf("old state %d\n", old_state);

    while(true) {
      printf("%s", arr_hello_world);
    }

    pthread_cleanup_pop(1);
    return NULL;
}

int main() {
    pthread_t tid;

    int err = pthread_create(&tid, NULL, my_thread, NULL);
    if (err) {
        fprintf(stderr, "main: pthread_create() failed: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    err = pthread_cancel(tid);
    if (err) {
        fprintf(stderr, "main: pthread_cancel() failed %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    void* ret_val;
    int err_join = pthread_join(tid, &ret_val);
    if (err_join) {
        fprintf(stderr, "pthread_join error: %s: ", strerror(err_join));
        return EXIT_FAILURE;
    }

    if (ret_val == PTHREAD_CANCELED) {
        printf("Thread was canceled\nretval:  %d\n", (int) ret_val);
    }

    return EXIT_SUCCESS;
}
