#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <stdbool.h>
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
        printf("set_cpu: pthread_setaffinity failed for CPU %d\n", n);
        return;
    }

    printf("set_cpu: set CPU %d\n", n);
}

void reader(queue_t *q) {
    int expected = 0;
    printf("reader [%d %d %d]\n", getpid(), getppid(), gettid());

    set_cpu(1);

    while (true) {
        int val = -1;
        int ok = queue_get(q, &val);
        if (!ok) continue;

        if (expected != val)
            printf(RED "ERROR: get value is %d but expected - %d" NOCOLOR "\n", val, expected);

        expected = val + 1;
    }
}

int writer(void* arg) {
    queue_t *q = (queue_t *)arg;
    int i = 0;
    printf("writer [%d %d %d]\n", getpid(), getppid(), gettid());

    set_cpu(2);

    while (true) {
        int ok = queue_add(q, i);
        if (!ok) continue;
        i++;
    }
    return 0;
}

void free_resources(queue_t *q) {
    destroy_sem();
    queue_destroy(q);
}

int main() {
    queue_t *q;
    int err;

    printf("main [%d %d %d]\n", getpid(), getppid(), gettid());
    init_sem();

    q = queue_init(1000000);

    int stack_size = 65536;
    void *stack = malloc(stack_size);
    if (!stack) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    pid_t child_pid = clone(writer, stack + stack_size, CLONE_VM | CLONE_SIGHAND | CLONE_FS | CLONE_FILES | CLONE_SYSVSEM, q);

    if (child_pid == -1) {
        perror("clone");
        exit(EXIT_FAILURE);
    }

    reader(q);
    free_resources(q);
    free(stack);
    return EXIT_SUCCESS;
}