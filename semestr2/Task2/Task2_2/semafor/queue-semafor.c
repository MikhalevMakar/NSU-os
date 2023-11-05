#define _GNU_SOURCE
#include <pthread.h>
#include <assert.h>
#include <stdbool.h>
#include <semaphore.h>
#include "queue-condvar.h"

sem_t semaphore;
volatile int stop_flag = false;

void *qmonitor(void *arg) {
	queue_t *q = (queue_t *)arg;

	printf("qmonitor: [%d %d %d]\n", getpid(), getppid(), gettid());

	while (!stop_flag) {
		queue_print_stats(q);
		sleep(1);
	}

	return NULL;
}

void init_sem() {
    int err = sem_init(&semaphore, 0, 1);
    if (err)
        printf("main: sem_init() failed: %s\n", strerror(err));
}

queue_t* queue_init(int max_count) {
	int err;

	queue_t *q = malloc(sizeof(queue_t));
	if (!q) {
		printf("malloc: can not allocate memory for a queue\n");
		abort();
	}

	q->first = NULL;
	q->last = NULL;
	q->max_count = max_count;
	q->count = 0;

	q->add_attempts = q->get_attempts = 0;
	q->add_count = q->get_count = 0;

	err = pthread_create(&q->qmonitor_tid, NULL, qmonitor, q);
	if (err) {
        free(q);
		printf("queue_init: pthread_create() failed: %s\n", strerror(err));
		abort();
	}

	return q;
}

void queue_destroy(queue_t *q) {
    stop_flag = true;
    void *ret_val;
    int err = pthread_join(q->qmonitor_tid, &ret_val);
    if (err)
        fprintf(stderr, "queue_destroy: pthread_join() failed %s\n", strerror(err));
    qnode_t *cur_ptr_q = q->first, *next_ptr_q;
    while (cur_ptr_q != NULL) {
        next_ptr_q = cur_ptr_q->next;
        free(cur_ptr_q);
        cur_ptr_q = next_ptr_q;
    }
}

void destroy_sem() {
    int err = sem_destroy(&semaphore);;
    if(err) fprintf(stderr, "sem_destroy: failed %s\n", strerror(err));
}

int queue_add(queue_t *q, int val) {
    sem_wait(&semaphore);
	q->add_attempts++;

	assert(q->count <= q->max_count);

	if (q->count == q->max_count) {
       sem_post(&semaphore);
        return 0;
    }

	qnode_t *new = malloc(sizeof(qnode_t));
	if (!new) {
		printf("malloc: cannot allocate memory for new node\n");
		abort();
	}

	new->val = val;
	new->next = NULL;

	if (!q->first)
		q->first = q->last = new;
	else {
		q->last->next = new;
		q->last = q->last->next;
	}

	q->count++;
	q->add_count++;
   sem_post(&semaphore);
	return 1;
}

int queue_get(queue_t *q, int *val) {
    sem_wait(&semaphore);
	q->get_attempts++;

	assert(q->count >= 0);

	if (q->count == 0) {
       sem_post(&semaphore);
        return 0;
    }

	qnode_t *tmp = q->first;

	*val = tmp->val;
	q->first = q->first->next;

	free(tmp);
	q->count--;
	q->get_count++;
    sem_post(&semaphore);
	return 1;
}

void queue_print_stats(queue_t *q) {
    sem_wait(&semaphore);
	printf("queue stats: current size %d; attempts: (%ld %ld %ld); counts (%ld %ld %ld)\n",
		q->count,
		q->add_attempts, q->get_attempts, q->add_attempts - q->get_attempts,
		q->add_count, q->get_count, q->add_count -q->get_count);
   sem_post(&semaphore);
}


