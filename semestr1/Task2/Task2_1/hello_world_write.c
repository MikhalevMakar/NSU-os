#include <unistd.h>
#include <stdlib.h>

enum {
    ERROR = -1,
    STDOUT = 1
};

int main(void) {
    ssize_t ret = write(STDOUT, "hello from write", 16);
    return (ret == ERROR) ? EXIT_FAILURE : EXIT_SUCCESS;
}