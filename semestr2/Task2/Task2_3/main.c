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
    SIZE_STORAGE = 20000,
    STOPPING_ORDER = 10000,
    COUNT_THREADS = 7,
    MIN_SIZE_LIST = 3,
    LOCK_SUCCESS = 0
};
enum num_thread {
    INCREASING = 0,
    DECREASING = 1,
    EQUALS = 2,
    CHECK_INCREASING = 3,
    CHECK_DECREASING = 4,
    CHECK_EQUALS = 5
};

typedef struct _Node {
    char value[SIZE_WORD];
    struct _Node* next;
    pthread_mutex_t sync;
}Node;

typedef struct _Storage {
    Node *first;
}Storage;

_Atomic int count_increment[COUNT_THREADS] = {0};

Node* create_node(char* line) {
    Node* node = malloc(sizeof(Node));
    assert(node != NULL);
    strcpy(node->value, line);
    node->next= NULL;
    pthread_mutex_init(&(node->sync), NULL);
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
        snprintf(new_value, sizeof(new_value), "create new node %d", SIZE_STORAGE - i + 1);
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

void permit_live_lock() {
    usleep(random() % STOPPING_ORDER);
}

bool try_lock_mutex(pthread_mutex_t* mutex) {
   int err = pthread_mutex_trylock(mutex);
   if(err != LOCK_SUCCESS) printf("failed lock mutex\n");

   return err == LOCK_SUCCESS;
}

void* find_increasing(void* args) {
    Storage* storage = (Storage*)args;

    while(true) {
        int counter = 0;
        Node* node = storage->first, *next, *tmp;
        if (node == NULL || node->next == NULL) {
            fprintf(stderr, "warn: storage is empty\n");
            continue;
        }
        while (node->next != NULL) {
            tmp = node;
            if(try_lock_mutex(&(tmp->sync))) {
                next = node->next;
                if (try_lock_mutex(&(next->sync))) {
                    if (strlen(node->value) < strlen(next->value)) ++counter;
                    node = node->next;
                    pthread_mutex_unlock(&(next->sync));
                }
                pthread_mutex_unlock(&(tmp->sync));
            }
        }
        ++count_increment[INCREASING];
    }
}

void* find_decreasing(void* args) {
    Storage* storage = (Storage*)args;

    while(true) {
        int counter = 0;
        Node* node = storage->first, *next, *tmp;
        if (node == NULL || node->next == NULL) {
            fprintf(stderr, "warn: storage is empty\n");
            continue;
        }
        while (node->next != NULL) {
            tmp = node;
            pthread_mutex_lock(&(tmp->sync));
            next = node->next;
            if(try_lock_mutex(&(next->sync))) {
                if (strlen(node->value) > strlen(next->value)) ++counter;
                pthread_mutex_unlock(&(next->sync));
            }
            node = node->next;
            pthread_mutex_unlock(&(tmp->sync));
        }
        ++count_increment[EQUALS];
    }
}

void* find_equals(void* args) {
    Storage* storage = (Storage*)args;

    while(true) {
        int counter = 0;
        Node* node = storage->first, *next, *tmp;
        if (node == NULL || node->next == NULL) {
            fprintf(stderr, "warn: storage is empty\n");
            continue;
        }
        while (node->next != NULL) {
            tmp = node;
            pthread_mutex_lock(&(tmp->sync));
            next = node->next;
            if(try_lock_mutex(&(next->sync))) {
                if (strlen(node->value) == strlen(next->value)) ++counter;
                pthread_mutex_unlock(&(next->sync));
            }
            node = node->next;
            pthread_mutex_unlock(&(tmp->sync));
        }
        ++count_increment[EQUALS];
    }
}

void swap_nodes(Node* prev, Node* cur, Node* future) {
    assert(prev != NULL && cur != NULL && future != NULL);
    prev->next = future;
    Node* tmp = future->next;
    future->next = cur;
    cur->next = tmp;
}

int number_random_node() {
    int index;
    do {
        unsigned int seed = (unsigned int) time(NULL);
        index = rand_r(&seed) % (SIZE_STORAGE - 2);
    } while (index < MIN_SIZE_LIST);
    return index;
}

void* random_swap(void* args) {
     Storage* storage = (Storage*)args;
     assert(storage != NULL);

    while(true) {
         Node *prev = storage->first;
         if (prev == NULL || prev->next == NULL) {
            fprintf(stderr, "warn: there are less, than two items in the value\n");
            continue;
         }

         int index = number_random_node();

         Node *current = prev->next, *future = prev->next->next;

         for (int i = 0; i < index && future != NULL; ++i) {
             Node* node = prev;
             pthread_mutex_lock(&(node->sync));
             if (try_lock_mutex(&(node->next->sync))) {
                 if (try_lock_mutex(&(node->next->next->sync))) {
                     prev = current;
                     current = current->next;
                     future = future->next;
                     pthread_mutex_unlock(&(node->next->next->sync));
                 }
                 pthread_mutex_unlock(&(node->next->sync));
             }
             pthread_mutex_unlock(&(node->sync));
         }

         swap_nodes(prev, current, future);
    }
}

void join_thread(pthread_t tid) {
    void* ret_val;
    int err = pthread_join(tid, &ret_val);
    if (err) perror("pthread_join() failed");
}

void* monitor(void *arg) {
	while (true) {
        printf("count INCREASING: %d,"
               " DECREASING %d, EQUALS %d,"
               " CHECK_INCREASING %d,"
               " CHECK_DECREASING %d,"
               " CHECK_EQUALS %d\n", count_increment[INCREASING],
                                     count_increment[DECREASING],
                                     count_increment[EQUALS],
                                     count_increment[CHECK_INCREASING],
                                     count_increment[CHECK_DECREASING],
                                     count_increment[CHECK_EQUALS]);
		sleep(1);
	}
}

int main() {
    Storage* storage = create_storage();

    init_storage(storage);
    print_storage(storage);

    pthread_t tids[COUNT_THREADS];

    pthread_create(&tids[0], NULL, find_increasing, storage);
    //pthread_create(&tids[1], NULL, find_decreasing, storage);
//    pthread_create(&tids[2], NULL, find_equals, storage);
    pthread_create(&tids[1], NULL, find_decreasing, storage);

    pthread_create(&tids[3], NULL, random_swap, storage);
//    pthread_create(&tids[4], NULL, random_swap, storage);
//    pthread_create(&tids[5], NULL, random_swap, storage);

    pthread_create(&tids[6], NULL, monitor, storage);

    join_thread(tids[6]);
    return EXIT_FAILURE;
}