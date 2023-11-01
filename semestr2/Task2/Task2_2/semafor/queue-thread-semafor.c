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

#include "queue-semafor.h"

#define RED "\033[41m"
#define NOCOLOR "\033[0m"

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

void* reader(void *arg) {
	int expected = 0;
	queue_t *q = (queue_t *)arg;
	printf("reader [%d %d %d]\n", getpid(), getppid(), gettid());

	set_cpu(1);

	while (true) {
		int val = -1;
		int ok = queue_get(q, &val);
		if (!ok) continue;

		if (expected != val)
			printf(RED"ERROR: get value is %d but expected - %d" NOCOLOR "\n", val, expected);

		expected = val + 1;
	}

	return NULL;
}

void* writer(void *arg) {
	int i = 0;
	queue_t *q = (queue_t *)arg;
	printf("writer [%d %d %d]\n", getpid(), getppid(), gettid());

	set_cpu(1);

	while (true) {
		int ok = queue_add(q, i);
		if (!ok) continue;
		i++;
	}
	return NULL;
}

void free_resources(queue_t *q) {
     destroy_sem();
     queue_destroy(q);
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
	queue_t *q;
	int err;

	printf("main [%d %d %d]\n", getpid(), getppid(), gettid());

	q = queue_init(1000);
    init_sem();
    err = pthread_create(&tid_writer, NULL, writer, q);
	if (err) {
        free_resources(q);
		printf("main: pthread_create() failed: %s\n", strerror(err));
		return EXIT_FAILURE;
	}

//	sched_yield();

    err = pthread_create(&tid_reader, NULL, reader, q);
	if (err) {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        free_resources(q);
        err = join_thread(tid_writer);
		return EXIT_FAILURE;
	}

    err = join_thread(tid_reader);
    err = join_thread(tid_writer);
    free_resources(q);
	return err ? EXIT_FAILURE : EXIT_SUCCESS;
}
