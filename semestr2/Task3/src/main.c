#include "proxy.c"

int main() {
    logg("SERVER START", GREEN);

    sem_init(&thread_semaphore, 0, MAX_USERS_COUNT);
    run_proxy();
    sem_destroy(&thread_semaphore);
    exit(EXIT_SUCCESS);
}