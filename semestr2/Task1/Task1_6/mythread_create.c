#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <memory.h>
#include <signal.h>
#include <stdbool.h>

enum {
    EXC_CREATE_CLONE = 1,
    PAGE = 4096,
    SIZE_STACK = PAGE * 8,
    SUCCESSFUL = 0
};

typedef void*(*start_routine_t)(void*);

typedef struct _my_thread {
    int id;
    start_routine_t start_routine;
    void* arg;
    void* ret_val;
    volatile bool joined;
    volatile bool exited;
} _my_thread_struct_t;

typedef _my_thread_struct_t* _my_thread;

void* create_stack(off_t size, int thread_num) {
    int stack_fd;
    void* stack;
    char stack_file[128];

    snprintf(stack_file, sizeof(stack_file), "stack-%d", thread_num);

    stack_fd = open(stack_file, O_RDWR | O_CREAT, 0660);
    ftruncate(stack_fd, 0);
    ftruncate(stack_fd, size);

    stack = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, stack_fd, 0);
    close(stack_fd);

    return stack;
}

int my_thread_start_up(void* arg) {
    _my_thread_struct_t* my_thread = (_my_thread_struct_t*)arg;
    my_thread->ret_val = my_thread->start_routine(my_thread->arg);
    my_thread->exited = true;

    while(!my_thread->joined)
        sleep(1);

    return SUCCESSFUL;
}

int my_thread_join(_my_thread my_tid, void** ret_val) {
    _my_thread_struct_t* my_thread = my_tid;
    while(!my_thread->exited)
        sleep(1);

    *ret_val = my_thread->ret_val;
    my_thread->joined = true;
    return SUCCESSFUL;
}

int my_thread_create(_my_thread* thread, start_routine_t start_routine, void *arg) {
    static int thread_num = 0;

    void* child_stack = create_stack(SIZE_STACK, ++thread_num);
    mprotect(child_stack + PAGE, SIZE_STACK - PAGE, PROT_READ | PROT_WRITE);
    memset(child_stack + PAGE, 0x7f, SIZE_STACK - PAGE);

    _my_thread_struct_t* my_thread = (_my_thread_struct_t*)(child_stack + SIZE_STACK - sizeof(_my_thread_struct_t));
    my_thread->id = thread_num;
    my_thread->start_routine = start_routine;
    my_thread->arg = arg;
    my_thread->joined = false;
    my_thread->exited = false;
    my_thread->ret_val = NULL;

    child_stack = (void*) my_thread;

    int child_pid = clone(my_thread_start_up, child_stack, CLONE_VM | CLONE_FILES | CLONE_THREAD | CLONE_SIGHAND | SIGCHLD, (void*)my_thread);
    if(child_pid == -1) {
        printf("clone failed %s\n", strerror(errno));
        return EXC_CREATE_CLONE;
    }

    *thread = my_thread;
    return SUCCESSFUL;
}

void* my_thread(void *arg) {
    char *str = (char *) arg;
    printf("my thread [%d %d %d]: %s\n", getpid(), getppid(), gettid(), str);
    return NULL;
}

int main() {
     _my_thread my_tid;
    void* ret_val;

    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());
    my_thread_create(&my_tid, my_thread, "Hello from my thread!");
    my_thread_join(my_tid, &ret_val);
    return EXIT_SUCCESS;
}