#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdatomic.h>

enum {
    SIZE_WORD = 100,
    SIZE_STORAGE = 10,
    STOPPING_ORDER = 10000000,
    COUNT_THREADS = 7
};

#define THREAD_CHECK_INCR "check_and_move_increasing"
#define THREAD_CHECK_DECR "check_and_move_decreasing"
#define THREAD_CHECK_EQ "check_and_move_equals"

typedef struct _Node {
    char value[SIZE_WORD];
    struct _Node* next;
    pthread_rwlock_t sync;
}Node;

typedef struct _Storage {
    Node *first;
}Storage;

_Atomic int count_increment = ATOMIC_VAR_INIT(0);
_Atomic int stop_flag = ATOMIC_VAR_INIT(0);

Node* create_node(char* line) {
    Node* node = malloc(sizeof(Node));
    assert(node != NULL);
    strcpy(node->value, line);
    node->next= NULL;
    pthread_rwlock_init(&(node->sync), NULL);
    return node;
}

void append_new_node(Storage* storage, Node* new_node) {
    assert(storage != NULL);

    if(storage->first == NULL) {
        storage->first = new_node;
        return;
    }
    Node* current = storage->first;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
}

Storage* create_storage() {
    Storage* storage = malloc(sizeof(Storage));
    assert(storage != NULL);
    storage->first= NULL;
    return storage;
}

void init_storage(Storage* storage) {
    Node* node;
    char new_value[SIZE_WORD];
    for(int i = 1; i <= SIZE_STORAGE; ++i) {
        snprintf(new_value, sizeof(new_value), "create new node %d", SIZE_STORAGE - i +1);
        node = create_node(new_value);
        append_new_node(storage, node);
    }
}

void print_storage(Storage* storage) {
    Node* node = storage->first;
    while(node != NULL) {
         printf("%s \n", node->value);
         node = node->next;
    }
}

bool try_lock_rdlock(pthread_rwlock_t* rwlock, char* name_thread) {
   int err = pthread_rwlock_tryrdlock(rwlock);
   if(err) printf("failed lock spinlock: %s\n", name_thread);

   return err ? false : true;
}

void* find_increasing(void* args) {
    Storage* storage = (Storage*)args;

    while(true) {
        Node* node = storage->first;
        if (node == NULL || node->next == NULL) {
            fprintf(stderr, "warn: storage is empty\n");
            continue;
        }

        size_t previous_length = strlen(node->value);
        int counter = 0;

        while (node != NULL) {
            pthread_rwlock_rdlock(&(node->sync));
            size_t size_line = strlen(node->value);
            pthread_rwlock_unlock(&(node->sync));
            if (previous_length < size_line) ++counter;
            node = node->next;
            previous_length = size_line;
        }
//        printf("number of pairs of strings "
//               "in ascending order of length %d\n", counter);
        ++count_increment;
    }
    return NULL;
}

void* find_decreasing(void* args) {
    Storage* storage = (Storage*)args;

    while(true) {
        Node* node = storage->first;
        if (node == NULL || node->next == NULL) {
            fprintf(stderr, "warn: storage is empty\n");
            continue;
        }
        size_t previous_length = strlen(node->value);
        int counter = 0;
        while (node != NULL) {
            pthread_rwlock_rdlock(&(node->sync));
            size_t size_line = strlen(node->value);
            pthread_rwlock_unlock(&(node->sync));
            if (previous_length > size_line) ++counter;
            node = node->next;
            previous_length = size_line;
        }
//        printf("number of pairs of strings "
//               "in descending order of length %d\n", counter);
        ++count_increment;
    }
}

void* find_equals(void* args) {
    Storage* storage = (Storage*)args;

    while(true) {
        Node* node = storage->first;
        if (node == NULL || node->next == NULL) {
            fprintf(stderr, "warn: storage is empty\n");
            continue;
        }
        size_t previous_length = strlen(node->value);
        int counter = 0;
        while (node != NULL) {
            pthread_rwlock_rdlock(&(node->sync));
            size_t size_line = strlen(node->value);
            pthread_rwlock_unlock(&(node->sync));
            if (previous_length == size_line) ++counter;
            node = node->next;
            previous_length = size_line;
        }
//        printf("number of pairs of strings"
//               " having the same length %d\n", counter);
        ++count_increment;
    }
}

void* check_and_move_increasing(void* args) {
    Storage* storage = (Storage*)args;
    Node *future_node, *prev_node;

    while(true) {
         Node* node = storage->first;
         if (node == NULL || node->next == NULL) {
            fprintf(stderr, "warn: storage is empty\n");
            continue;
        }

        future_node = node->next;
        prev_node = node;
        while (future_node != NULL) {

            pthread_rwlock_rdlock(&(node->sync));
            if (try_lock_rdlock(&(future_node->sync), THREAD_CHECK_INCR) &&
                strlen(node->value) > strlen(future_node->value)) {
                ++count_increment;

                if (node == storage->first) {
                    storage->first = future_node;
                } else if (try_lock_rdlock(&(prev_node->sync), THREAD_CHECK_INCR)) {

                    pthread_rwlock_wrlock(&(prev_node->sync));
                    prev_node->next = future_node;
                    pthread_rwlock_unlock(&(prev_node->sync));

                    pthread_rwlock_wrlock(&(node->sync));
                    node->next = future_node->next;
                    pthread_rwlock_unlock(&(prev_node->sync));

                    pthread_rwlock_wrlock(&(future_node->sync));
                    future_node->next = node;
                    pthread_rwlock_unlock(&(future_node->sync));
                }
            }
            pthread_rwlock_unlock(&(node->sync));
            pthread_rwlock_unlock(&(future_node->sync));
            prev_node = node;
            node = node->next;
            future_node = future_node->next;
        }
          //printf("%s \n", THREAD_CHECK_INCR);
    }
}

void* check_and_move_decreasing(void* args) {
    Storage* storage = (Storage*)args;
    Node *future_node, *prev_node;

    while(true) {
         Node* node = storage->first;
         if (node == NULL || node->next == NULL) {
            fprintf(stderr, "warn: storage is empty\n");
            continue;
        }

        future_node = node->next;
        while (future_node != NULL) {
            pthread_rwlock_rdlock(&(node->sync));
            if (try_lock_rdlock(&(future_node->sync), THREAD_CHECK_INCR) &&
                strlen(node->value) < strlen(future_node->value)) {
                ++count_increment;

                if (node == storage->first) {
                    storage->first = future_node;
                }  else if (try_lock_rdlock(&(prev_node->sync), THREAD_CHECK_INCR)) {

                    pthread_rwlock_wrlock(&(prev_node->sync));
                    prev_node->next = future_node;
                    pthread_rwlock_unlock(&(prev_node->sync));

                    pthread_rwlock_wrlock(&(node->sync));
                    node->next = future_node->next;
                    pthread_rwlock_unlock(&(prev_node->sync));

                    pthread_rwlock_wrlock(&(future_node->sync));
                    future_node->next = node;
                    pthread_rwlock_unlock(&(future_node->sync));
                }
                node->next = future_node->next;
                future_node->next = node;
            }

            pthread_rwlock_unlock(&(node->sync));
            pthread_rwlock_unlock(&(future_node->sync));

            prev_node = node;
            node = future_node;
            future_node = future_node->next;
        }
         //printf("%s \n", THREAD_CHECK_DECR);
    }
}

//void* check_and_move_equals(void* args) {
//    Storage* storage = (Storage*)args;
//
//    Node *future_node, *cur_node, *node;
//    while(true) {
//         node = storage->first;
//         if (node == NULL || node->next == NULL) {
//            fprintf(stderr, "warn: storage is empty\n");
//            continue;
//        }
//
//        cur_node = node->next;
//        future_node = node->next->next;
//
//        while (future_node != NULL) {
//            pthread_spin_lock(&(node->sync));
//            if(!try_lock_spinlock(&cur_node->sync, THREAD_CHECK_EQ)) {
//                pthread_spin_unlock(&(node->sync));
//                usleep(random() % STOPPING_ORDER);
//                continue;
//            }
//            if(!try_lock_spinlock(&(future_node->sync), THREAD_CHECK_EQ)) {
//                pthread_spin_unlock(&(node->sync));
//                pthread_spin_unlock(&(cur_node->sync));
//                usleep(random() % STOPPING_ORDER);
//                continue;
//            }
//            if (strlen(node->value) == strlen(future_node->value)) {
//                ++count_increment;
//
//                node->next = future_node;
//                cur_node->next = future_node->next;
//                future_node->next = cur_node;
//
//            }
//
//            pthread_spin_unlock(&(node->sync));
//            pthread_spin_unlock(&(cur_node->sync));
//            pthread_spin_unlock(&(future_node->sync));
//
//            node= node->next;
//            cur_node = node->next;
//            future_node = cur_node->next;
//        }
//        //printf("%s \n", THREAD_CHECK_EQ);
//    }
//}

void join_thread(pthread_t tid) {
    void* ret_val;
    int err = pthread_join(tid, &ret_val);
    if (err) perror("pthread_join() failed");
}

void* monitor(void *arg) {
	while (true) {
        printf("count %d\n", count_increment);
		sleep(1);
	}
	return NULL;
}

int main() {
    Storage *storage = create_storage();

    init_storage(storage);
    print_storage(storage);

    pthread_t tids[COUNT_THREADS];

//    pthread_create(&tids[0], NULL, find_increasing, storage);
//    pthread_create(&tids[1], NULL, find_decreasing, storage);
//    pthread_create(&tids[2], NULL, find_equals, storage);
//    pthread_create(&tids[3], NULL, check_and_move_decreasing, storage);
//    pthread_create(&tids[4], NULL, check_and_move_equals, storage);
    pthread_create(&tids[5], NULL, check_and_move_increasing, storage);
    pthread_create(&tids[6], NULL, monitor, storage);

    join_thread(tids[6]);
    return EXIT_FAILURE;
}