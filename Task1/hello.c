#include "hello_from_static_lib.h"
#include "hello_from_dynamic_lib.h"

void printHello() {
    printf("Hello world\n");
}

int main() {

    printHello();
    return 0;
}
