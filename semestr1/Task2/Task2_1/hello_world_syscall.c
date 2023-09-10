#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>

enum {
    ERROR = -1,
    STDOUT = 1
};

static ssize_t my_write(const int fd, const void* buf, const size_t count) {
    return syscall(SYS_write, fd, buf, count);
}

int main(void) {
    ssize_t ret = my_write(STDOUT, "hello world syscall", 19);
    return (ret == ERROR) ? EXIT_FAILURE : EXIT_SUCCESS;

}