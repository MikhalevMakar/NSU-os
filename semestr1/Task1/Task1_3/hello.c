#include <stdio.h>
#include "hello_from_dynamic_lib.h"

void printHello() {
    printf("Hello world\n");
}

int main() {
    hello_from_dynamic_lib();
    printHello();
    return 0;
}
