#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

void* my_thread(void *arg) {
	printf("my_thread [%d %d %d]: Hello from my_thread!\n", getpid(), getppid(), gettid());
	return NULL;
}

int main() {
	pthread_t tid;
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	err = pthread_create(&tid, NULL, my_thread, NULL);
	if (err) {
	    fprintf(stderr, "macin: pthread_create() failed: %s\n", strerror(err));
		return EXIT_FAILURE;
	}

    void* ret_val;
    err = pthread_join(tid, &ret_val); // добавил строчку
    //sleep() можно и этот вариант

    if (err) {
        fprintf(stderr, "main: pthread_join() failed %s\n", strerror(err));
    }

	return EXIT_SUCCESS;
}