#include <unistd.h>

int main(void) {
    int ret = write(1, "hello from write", 16);
    return (ret == -1) ? -1 : 0;
}