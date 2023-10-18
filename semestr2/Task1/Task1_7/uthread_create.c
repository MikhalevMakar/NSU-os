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
#include <ucontext.h>
#include <pthread.h>
pthread_mutex_t mutex;
enum {
    EXC_CREATE_CLONE = 1,
    PAGE = 4096,
    SIZE_STACK = PAGE * 8,
    SUCCESSFUL = 0,
    MAX_THREAD = 200,
    SIZE_NAME_FILE = 128
};

enum error {
    ERROR_HAS_OCCURRED = -1,
    EXC_GET_CONTEXT = -1
};

typedef struct uthread {

    int number_thread;
    void(*thread_func)(void*);
    void* arg;
    ucontext_t uctx;
    bool is_running;
    bool is_finish;
    char* stack_file;
}uthread_t;

uthread_t* uthreads[MAX_THREAD];
int uthread_count = 0;
int uthread_cur = 0;
bool stop_work_checker = false;

void schedule(void);
void* create_stack(off_t size, char* stack_file);
void start_thread(void);
void uthread_create(uthread_t** thread, void *(start_routine), void *arg);
void* my_thread1(void *arg);
void* my_thread2(void *arg);
void* my_thread3(void *arg);
void* checker_finish_thread(void *arg);

int main() {
    uthread_t *ut[3];
    int arg[] = {0, 1, 2};
    uthread_create(&ut[0], my_thread1, &arg[0]);
    uthread_create(&ut[1], my_thread2, &arg[1]);
    uthread_create(&ut[2], my_thread3, &arg[2]);
    pthread_t tid;
    int err = pthread_create(&tid, NULL, checker_finish_thread, NULL);
	if (err) {
	    fprintf(stderr, "main: pthread_create() failed: %s\n", strerror(err));
		return EXIT_FAILURE;
	}

    while(uthread_count > 0) {
        schedule();
    }

    void* ret_val;
    err = pthread_join(tid, &ret_val);
    if (err) {
        fprintf(stderr, "main: pthread_join() failed %s\n", strerror(err));
    }
}

void* my_thread1(void *arg) {
    int number_thread = *(int*)arg;
    printf("my thread 1 [%d %d %d]: %d\n", getpid(), getppid(), gettid(), number_thread);
    for(int i = 0; i < 5; ++i) {
        printf("hello from thread 1\n");
        schedule();
    }
    uthreads[number_thread]->is_finish = true;
    while(stop_work_checker == false) {
        sleep(1);
    }
    return NULL;
}

void* my_thread2(void* arg) {
    int number_thread = *(int*)arg;
    printf("my thread 2 [%d %d %d]: %d\n", getpid(), getppid(), gettid(), number_thread);
    for(int i = 0; i < 5; ++i) {
        printf("hello from thread 2\n");
        schedule();
    }
    uthreads[number_thread]->is_finish = true;
    while(stop_work_checker == false) sleep(1);
    return NULL;
}

void* my_thread3(void *arg) {
    int number_thread = *(int*)arg;
    printf("my thread 3 [%d %d %d]: %d\n", getpid(), getppid(), gettid(), number_thread);
    for(int i = 0; i < 5; ++i) {
        printf("hello from thread 3\n");
        schedule();
    }
    uthreads[number_thread]->is_finish = true;
    while(stop_work_checker == false) {
        sleep(1);
    }
    return NULL;
}

void* checker_finish_thread(void *arg) {
    int count_kill_threads = 0;

    while (count_kill_threads < uthread_count) {
        for (int i = 0; i < uthread_count; ++i) {
            if (uthreads[i] != NULL && uthreads[i]->is_finish == true) {
                printf("finish thread work %d : %s\n", i, uthreads[i]->stack_file);

                int ret = remove(uthreads[i]->stack_file);
                if(ret == ERROR_HAS_OCCURRED)
                    perror("Error occurred while remove file\n");
                count_kill_threads++;
                uthreads[i] = NULL;
            }
        }
    }
    uthread_count = 0;
    stop_work_checker = true;
    return NULL;
}

void uthread_create(uthread_t** thread, void *(thread_func), void *arg) {
    char stack_file[SIZE_NAME_FILE];
    snprintf(stack_file, sizeof(stack_file), "stack-%d", uthread_count);
    printf("stack file %s\n", stack_file);
    char* stack = create_stack(SIZE_STACK, stack_file);
    if(stack == NULL) {
        fprintf(stderr, "create_stack() failed\n");
        exit(EXIT_FAILURE);
    }

    mprotect(stack, SIZE_STACK, PROT_READ | PROT_WRITE);

    uthread_t* new_ut = (uthread_t*) (stack + SIZE_STACK - sizeof(uthread_t));
    int err = getcontext(&new_ut->uctx);
    if(err == EXC_GET_CONTEXT) {
        fprintf(stderr, "get_context() failed %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    new_ut->uctx.uc_stack.ss_sp = stack;
    new_ut->uctx.uc_stack.ss_size = SIZE_STACK - sizeof(uthread_t);
    new_ut->uctx.uc_link = NULL;

    new_ut->thread_func = thread_func;
    new_ut->arg = arg;
    new_ut->number_thread = uthread_count;
    new_ut->stack_file = strdup(stack_file);

    uthreads[uthread_count++] = new_ut;

    makecontext(&new_ut->uctx, start_thread, 0);

    *thread = new_ut;
}

void start_thread(void) {
    printf("count thread %d\n", uthread_count);
    for (int i = 0; i < uthread_count; ++i) {
        if (uthreads[i]->is_running == false) {
            uthreads[i]->is_running = true;
            uthreads[i]->thread_func(uthreads[i]->arg);
        }
    }
    printf("FINISH WORK THREAD");
}

//void start_thread(void) {
//    ucontext_t* ucontext;
//    for(int i = 0; i < uthread_count; ++i) {
//        ucontext = &uthreads[i]->uctx;
//        char* stack_from = ucontext->uc_stack.ss_sp;
//        char* stack_to = ucontext->uc_stack.ss_sp + ucontext->uc_stack.ss_size;
//        printf("%p %p %p\n", stack_from, &i, stack_to);
//        if(stack_from <= (char*)&i && (char*)&i <= stack_to) {
//            printf("new thread i %d\n", i);
//            uthreads[i]->thread_func(uthreads[i]->arg);
//            return;
//        }
//    }
//}

void* create_stack(off_t size, char* stack_file) {
    int stack_fd;
    void* stack;

    stack_fd = open(stack_file, O_RDWR | O_CREAT, 0660);
    ftruncate(stack_fd, 0);
    ftruncate(stack_fd, size);

    stack = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_STACK, stack_fd, 0);
    close(stack_fd);
    memset(stack, 0x7f, size);
    return stack;
}

void schedule(void) {
    ucontext_t *cur_ctx, *next_ctx;
    cur_ctx = &(uthreads[uthread_cur]->uctx);
    uthread_cur = (uthread_cur + 1) % uthread_count;
    printf("number cur thread: %d\n", uthread_cur);
    next_ctx = &(uthreads[uthread_cur]->uctx);
    int err = swapcontext(cur_ctx, next_ctx);
    if(err == -1) {
        fprintf(stderr, "uthread scheld yield: swap context() failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}
