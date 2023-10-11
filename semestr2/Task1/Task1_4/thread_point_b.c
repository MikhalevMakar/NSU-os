#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void my_thread_handler(void *arg) {
    printf("my thread handler\n");
}

void* my_thread(void *arg) {
    pthread_cleanup_push(my_thread_handler, (void*)arg);

    int old_state, count_inter = 0;
    int err = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old_state);
    printf("old state %d\n", old_state);
    if (err) {
        fprintf(stderr, "main: pthread_setcancelstate() failed %s\n", strerror(err));
        pthread_exit(NULL);
    }

    while(true) {
      ++count_inter;
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

    sleep(1);

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
        printf("Thread was canceled, ret_val:  %d\n", (int) ret_val);
    }

    return EXIT_SUCCESS;
}