#include "void  hello_from_dyn_runtime_lib.h"
#include <stdio.h>
#include <dlfcn.h>
#include <gnu/lib-names.h>

void print_hello() {
    printf("print_hello\n");
}
void  hello_from_dyn_runtime_lib() {
    printf("hello_from_dyn_runtime_lib\n");
}

void load_lib_run_function(void) {
    void(*dyn_print_hello)(void);
    char* error;
    void* h = dlopen("./libhello_from_dynamic_lib.so", RTLD_LAZY);
    if(!h) {
        printf("dlopen() failed: %s\n", dlerror());
        return;
    }

    dyn_print_hello = (void (*)(void)) dlsym(h, "hello_from_dynamic_lib");

    error = dyn_print_hello();
    if(error != nullptr) {
        printf("dlsyn() failed: %s\n", dlerror());
        return;
    }
    dyn_print_hello();
    dlclose(h);
}

int main(int argc, char* argv[]) {
    print_hello();
    hello_from_dyn_runtime_lib();
    load_lib_run_function();


}