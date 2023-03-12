#include <sys/syscall.h>
#include <unistd.h>

static int my_write(const int fd, const void* buf, const size_t count) {
    return syscall(SYS_write, fd, buf, count); // SYS_write = 4
}

int main(void) {
    int ret = my_write(1, "hello world syscall", 19);
    return (ret == -1) ? -1 : 0;
}