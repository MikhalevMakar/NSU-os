#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <sched.h>

pthread_mutex_t mutex;


void set_cpu(int n) {
	int err;
	cpu_set_t cpuset;
	pthread_t tid = pthread_self();

	CPU_ZERO(&cpuset);
	CPU_SET(n, &cpuset);

	err = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
	if (err) {
		printf("set_cpu: pthread_setaffinity failed for cpu %d\n", n);
		return;
	}

	printf("set_cpu: set cpu %d\n", n);
}


void* lock(void *arg) {
    set_cpu(1);
    printf("lock mutex\n");
    pthread_mutex_lock(&mutex);
    sleep(100);
	return NULL;
}

void* unlock(void *arg) {
    set_cpu(2);
    printf("try unlock mutex\n");
    pthread_mutex_unlock(&mutex);
    printf("SUCCESSFUL unlock mutex");
	return NULL;
}

void init_mutex() {
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
    int err = pthread_mutex_init(&mutex, &mutex_attr);
    if (err) printf("main: pthread_mutex_init() failed: %s\n", strerror(err));
}

int join_thread(pthread_t tid) {
    void* ret_val;
    int err = pthread_join(tid, &ret_val);
    if (err)
        fprintf(stderr, "queue_destroy: pthread_join() failed %s\n", strerror(err));
    return err;
}


int main() {
    pthread_t tid_reader, tid_writer;
    init_mutex();

    int err = pthread_create(&tid_reader, NULL, lock, NULL);
	if (err) {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        err = join_thread(tid_writer);
        return EXIT_FAILURE;
    }

    err = pthread_create(&tid_writer, NULL, unlock, NULL);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
		return EXIT_FAILURE;
	}

    int err_reader = join_thread(tid_reader);
    int err_writer = join_thread(tid_writer);

    return EXIT_SUCCESS;
}
