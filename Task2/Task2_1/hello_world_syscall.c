#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>

enum {
    ERROR = -1,
    STDOUT = 1
};

static int my_write(const int fd, const void* buf, const size_t count) {
    return syscall(SYS_write, fd, buf, count); // SYS_write = 4
}

int main(void) {
    int ret = my_write(STDOUT, "hello world syscall", 19);
    return (ret == ERROR) ? EXIT_FAILURE : EXIT_SUCCESS;
}