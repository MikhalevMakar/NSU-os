#include <stdio.h>
#include <stdlib.h>

int* get_local_variable() {
    int local_variable = 10;
    return &local_variable;
}

int main(int argc, char** argv) {
    printf("%p", get_local_variable());
    return EXIT_SUCCESS;
}