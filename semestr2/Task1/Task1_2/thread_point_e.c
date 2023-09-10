#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
// pthread_self стал randomый так как освобоэдается
void* my_thread(void *arg) {
	printf("my_thread [%d %d %d %lu]: Hello from my_thread!\n", getpid(), getppid(), gettid(), (unsigned long)pthread_self());

    err = pthread_detach(pthread_self());
    if (err) {
        fprintf(stderr, "pthread_detach() failed: %s\n", strerror(err));
        return EXIT_FAILURE;
    }
	return NULL;
}

int main() {
	pthread_t tid;
	int err;
    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());
    while(1) {
        err = pthread_create(&tid, NULL, my_thread, NULL);
        if (err) {
            fprintf(stderr, "main: pthread_create() failed: %s\n", strerror(err));
            return EXIT_FAILURE;
        }
    }
	return EXIT_SUCCESS;
}