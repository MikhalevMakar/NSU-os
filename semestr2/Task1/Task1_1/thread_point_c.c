#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

//watch -d -n1 cat /proc/pid/maps

static int global_value = 10;

void* my_thread(void *arg) {
   const int const_local = 5;
   static int static_local = 10;
   int local = 15;

   printf("my_thread [pid: %d, ppid: %d, tid: %d, pthread_self %lu]: Hello from my_thread!\n", getpid(), getppid(), gettid(), (unsigned long)pthread_self());
   printf("my_thread: address local: %p, static_local: %p, const_local: %p, global_value: %p\n", &local, &static_local, &const_local, &global_value );
   return NULL;
}

int main() {
   printf("main [pid: %d, ppid: %d, tid: %d]: Hello from main!\n", getpid(), getppid(), gettid());

   const int const_local = 5;
   static int static_local = 10;
   int local = 15;
   printf("main: address local: %p, static_local: %p, const_local: %p, global_value: %p\n", &local, &static_local, &const_local, &global_value );

   pthread_t tid;
   int err;
   for(int i = 0; i < COUNT_THREAD; ++i) {
       err = pthread_create(&tid, NULL, my_thread, NULL);
       if (err) {
           fprintf(stderr, "main: pthread_create() failed: %s\n", strerror(err));
           return EXIT_FAILURE;
       }
   }

   void* ret_val;
   err = pthread_join(tid, &ret_val);
   if (err) {
       fprintf(stderr, "main: pthread_join() failed %s\n", strerror(err));
       return EXIT_FAILURE;
   }

   return EXIT_SUCCESS;
}
