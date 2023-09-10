#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum size_buffer {
    SIZE_BUFFER = 100
};

enum  return_status {
    ERROR_HAS_OCCURRED = -1,
    Ok = 0
};

enum return_status allocate_memory_heap(size_t size) {
    char* buffer_1 = (char*) calloc(size, sizeof(char));
    if(buffer_1 == NULL) {
        perror("Error allocate buffer_1 on the heap\n");
        return ERROR_HAS_OCCURRED;
    }

    buffer_1 = strcpy(buffer_1, "hello world\0");

    printf("%s\n", buffer_1);

    free(buffer_1);
    printf("%s\n", buffer_1);

    char* buffer_2 = (char*) calloc(size, sizeof(char));
    if(buffer_2 == NULL) {
        perror("Error allocate buffer_2 on the heap\n");
        return ERROR_HAS_OCCURRED;
    }

    buffer_2 = strcpy(buffer_2, "hello world\0");
    printf("%s\n", buffer_2);

    buffer_2 += size * sizeof(char) / 2;
    free(buffer_2);
    printf("%s\n", buffer_2);
    return Ok;
}


int main() {
    enum return_status ret = allocate_memory_heap(SIZE_BUFFER);
    return (ret == Ok) ? EXIT_SUCCESS : EXIT_FAILURE;
}