#include "hello_from_static_lib.h"

void printHello() {
    printf("Hello world\n");
}

int main() {
    hello_from_static_lib();
    printHello();
    return 0;
}
