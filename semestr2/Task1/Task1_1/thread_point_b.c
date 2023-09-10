#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

enum {
    COUNT_THREAD = 5
};

void* my_thread(void *arg) {
	printf("my_thread [pid: %d, ppid: %d, tid: %d, pthread_self %lu]: Hello from my_thread!\n", getpid(), getppid(), gettid(), (unsigned long)pthread_self());
	return NULL;
}

int main() {
   pthread_t tid[COUNT_THREAD];
   int err;

   printf("main [pid: %d, ppid: %d, tid: %d]: Hello from main!\n", getpid(), getppid(), gettid());

   for(int i = 0; i < COUNT_THREAD; ++i) {
       err = pthread_create(&tid[i], NULL, my_thread, NULL);
       if (err) {
           fprintf(stderr, "main: pthread_create() failed: %s\n", strerror(err));
           return EXIT_FAILURE;
       }
   }

   void* ret_val;
   for(int i = 0; i < COUNT_THREAD; ++i) {
       err = pthread_join(tid[i], &ret_val);
       if (err) {
           fprintf(stderr, "main: pthread_join() failed %s\n", strerror(err));
           return EXIT_FAILURE;
       }
   }
   return EXIT_SUCCESS;
}
