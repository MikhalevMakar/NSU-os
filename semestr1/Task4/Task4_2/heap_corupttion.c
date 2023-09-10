#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>

//watch -d -n1 cat /proc/pid/maps
//ulimit -a

enum constant {
    SIZE_BLOCK = 4096

};

void free_array_allocation(char** array, int size) {
    for(int i = 0; i < size; ++i) {
        if(array[i] != NULL)
            free(array[i]);
    }
    free(array);
}

void signal_callback_handler(int sig_num, char** array, int size) {
  printf("Caught signal %d\n", sig_num);
  free_array_allocation(array, size);
  exit(sig_num);
}

int i = 0;
void heap_corruption() {
    long int size = 0;

    int size_allocation = SIZE_BLOCK;
    char** array_allocation = calloc(size_allocation, sizeof(char*));
    if(array_allocation == NULL) {
        perror("Error allocation memory");
        exit(1);
    }

    signal(SIGINT, signal_callback_handler);
    signal(SIGSEGV, signal_callback_handler);

    while(1) {
        char* array = (char *) malloc(sizeof(char) * SIZE_BLOCK);
        if (array == NULL) {
            free_array_allocation(array_allocation, size_allocation);
            perror("Error allocation memory");
            exit(1);
        }

        array_allocation[i++] = array;
        size += SIZE_BLOCK;
        printf("%d %ld\n", i, size);

        if(i >= size_allocation) {
             size_allocation += SIZE_BLOCK;
             char** new_array_allocation = realloc(array_allocation, size_allocation);
             if(new_array_allocation == NULL) {
                 free_array_allocation(array_allocation, size_allocation);
                 perror("Error allocation memory");
                 exit(1);
             }
             array_allocation = new_array_allocation;
        }
        usleep(100000);

        char* p =
            mmap(NULL, 10 * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        munmap(p + 4 * 4096, 2 * 4096);
        sleep(10);

        mprotect(p, 4096, PROT_READ);
    }
}

int main(int argc, char** argv) {
    pid_t pid_process = getpid();
    printf("process pid: %d\n", pid_process);
    sleep(10);

    heap_corruption();
    return EXIT_SUCCESS;
}