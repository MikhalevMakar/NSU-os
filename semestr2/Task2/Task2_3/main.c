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
    SIZE_STORAGE = 100,
    COUNT_THREADS = 7,
    MIN_SIZE_LIST = 3,
};

enum num_thread {
    INCREASING ,
    DECREASING ,
    EQUALS,
    RANDOM_SWAP_1,
    RANDOM_SWAP_2,
    RANDOM_SWAP_3,
    MONITOR
};

typedef struct _Node {
    char value[SIZE_WORD];
    struct _Node* next;
    pthread_mutex_t sync;
}Node;

typedef struct _Storage {
    Node *first;
}Storage;

typedef struct Context {
    enum num_thread thread;
    Storage* storage;
}Context;

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
        //printf(new_value, sizeof(new_value), "create new node %d", SIZE_STORAGE - i + 1);
        node = create_node(new_value);
        append_new_node(storage, node);
    }
}

void print_storage(Storage* storage) {
    Node* node = storage->first;
    while(node != NULL) {
         //printf("%s %p\n", node->value, &(node->sync));
         node = node->next;
    }
}

void* find_increasing(void* args) {
    Storage* storage = (Storage*)args;

    while(true) {
        int counter = 0;
        Node* node = storage->first, *next, *tmp;
        if (node == NULL || node->next == NULL) {
            //printf(stderr, "warn: storage is empty\n");
            continue;
        }

        while (true) {
            pthread_mutex_lock(&(node->sync));
            //printf("lock current node %d, %p\n", INCREASING, &(node->sync));
            tmp = node;

            next = node->next;
            if(next == NULL) {
                pthread_mutex_unlock(&(node->sync));
                break;
            }

            pthread_mutex_lock(&(next->sync));

            //("lock future node %d, %p\n", INCREASING, &(next->sync));

            if (strlen(node->value) < strlen(next->value)) ++counter;

            node = node->next;

            //printf("UNlock current node %d, %p\n", INCREASING, &(next->sync));
            pthread_mutex_unlock(&(next->sync));

            //printf("UNlock future node %d, %p\n", INCREASING, &(tmp->sync));
            pthread_mutex_unlock(&(tmp->sync));
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
            //printf(stderr, "warn: storage is empty\n");
            continue;
        }
        while (true) {
            pthread_mutex_lock(&(node->sync));
            //printf("lock current node %d, %p\n", DECREASING, &(node->sync));

            tmp = node;
            next = node->next;
            if(next == NULL) {
                pthread_mutex_unlock(&(node->sync));
                break;
            }
            pthread_mutex_lock(&(next->sync));
            //printf("lock future node %d, %p\n", DECREASING, &(next->sync));

            if (strlen(node->value) < strlen(next->value)) ++counter;

            node = node->next;

            //printf("UNlock current node %d, %p\n", DECREASING, &(next->sync));
            pthread_mutex_unlock(&(next->sync));

            //printf("UNlock future node %d, %p\n", DECREASING, &(tmp->sync));
            pthread_mutex_unlock(&(tmp->sync));
        }
        ++count_increment[DECREASING];
    }
}

void* find_equals(void* args) {
    Storage* storage = (Storage*)args;

    while(true) {
        int counter = 0;
        Node* node = storage->first, *next, *tmp;
        if (node == NULL || node->next == NULL) {
            //printf(stderr, "warn: storage is empty\n");
            continue;
        }
        while (true) {

            pthread_mutex_lock(&(node->sync));
            //printf("lock current node %d %p\n", EQUALS, &(node->sync));

            tmp = node;
            next = node->next;

            if(next == NULL) {
                pthread_mutex_unlock(&(node->sync));
                break;
            }

            pthread_mutex_lock(&(next->sync));
            //printf("lock future node %d, %p\n", EQUALS, &(next->sync));

            if (strlen(node->value) == strlen(next->value)) ++counter;

            node = node->next;
            //printf("UNlock current node %d, %p\n", EQUALS, &(next->sync));
            pthread_mutex_unlock(&(next->sync));

            //printf("UNlock future node %d, %p\n", EQUALS, &(tmp->sync));
            pthread_mutex_unlock(&(tmp->sync));
        }
        ++count_increment[EQUALS];
    }
}

void swap_nodes(Node* prev, Node* cur, Node* future) {
    //printf("swap nodes\n");
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
    Context* context = (Context*)args;
    Storage* storage = context->storage;
    //printf("number thread %d\n", context->thread);
    assert(storage != NULL);

    while(true) {
         Node *prev = storage->first;
         if (prev == NULL || prev->next == NULL) {
            //printf(stderr, "warn: there are less, than two items in the value\n");
            continue;
         }

         int index = number_random_node();

         Node *current = prev->next, *future = prev->next->next, *node;

         for (int i = 0; i < index; ++i) {
             pthread_mutex_lock(&(prev->sync));
//             //printf("lock previous node %d %p\n", context->thread, &(prev->sync));

             if(prev->next == NULL) {
//                 //printf("NULL prev->next\n");
                 pthread_mutex_unlock(&(prev->sync));
                 break;
             }

             pthread_mutex_lock(&(prev->next->sync));
//             //printf("lock current node %d %p\n", context->thread, &(prev->next->sync));

             if(prev->next->next == NULL) {
//                 //printf("NULL prev->next->next\n");
                 pthread_mutex_unlock(&(prev->next->sync));
                 pthread_mutex_unlock(&(prev->sync));
                 break;
             }

             pthread_mutex_lock(&(prev->next->next->sync));
             //printf("lock future node %d %p\n", context->thread, &(prev->next->sync));
             node = prev;
                 prev = prev->next;
                 current = current->next;
                 future = future->next;

//             //printf("UNlock future node %d %p\n", context->thread, &(node->next->next->sync));
             pthread_mutex_unlock(&(node->next->next->sync));

//             //printf("UNlock current node %d %p\n", context->thread, &(node->next->sync));
             pthread_mutex_unlock(&(node->next->sync));

//             //printf("UNlock previous node %d %p\n", context->thread, &(node->sync));
             pthread_mutex_unlock(&(node->sync));
         }

         ++count_increment[context->thread];
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
               " RANDOM_SWAP_1 %d,"
               " RANDOM_SWAP_2 %d,"
               " RANDOM_SWAP_3 %d\n", count_increment[INCREASING],
                                      count_increment[DECREASING],
                                      count_increment[EQUALS],
                                      count_increment[RANDOM_SWAP_1],
                                      count_increment[RANDOM_SWAP_2],
                                      count_increment[RANDOM_SWAP_3]);
		sleep(1);
	}
}

int main() {
    Storage* storage = create_storage();

    init_storage(storage);
    print_storage(storage);

    pthread_t tids[COUNT_THREADS];

    pthread_create(&tids[INCREASING], NULL, find_increasing, storage);
    pthread_create(&tids[DECREASING], NULL, find_decreasing, storage);
    pthread_create(&tids[EQUALS], NULL, find_equals, storage);

    Context context1 = {RANDOM_SWAP_1, storage};
    Context context2 = {RANDOM_SWAP_2, storage};
    Context context3 = {RANDOM_SWAP_3, storage};

    pthread_create(&tids[RANDOM_SWAP_1], NULL, random_swap, (void*) &context1);
    pthread_create(&tids[RANDOM_SWAP_2], NULL, random_swap, (void*) &context2);
    pthread_create(&tids[RANDOM_SWAP_3], NULL, random_swap, (void*) &context3);

    pthread_create(&tids[MONITOR], NULL, monitor, storage);

    join_thread(tids[MONITOR]);
    return EXIT_SUCCESS;
}