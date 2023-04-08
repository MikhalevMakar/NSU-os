#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

enum POS {
    INCREASE_POS = 1,
    START_POS = 0
};

enum {
    BUFFER_SIZE = 256,
    MAX_LEN_NAME_FILE = 256,
    ERROR = -1,
    MIN_NUMBER_CORRECT_ARGS = 2,
};

void freeing_memory(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        free(va_arg(args, char*));
    }
    va_end(args);
}

void close_files(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        fclose(va_arg(args, FILE*));
    }
    va_end(args);
}

char* memory_allocation(size_t size) {
    char* ptr = malloc(size);
    assert(ptr);
    return ptr;
}

char* reallocation_memory(const char* previous_ptr, size_t size) {
    char* new_ptr = realloc(previous_ptr, size);
    assert(new_ptr);
    return new_ptr;
}

void swap(char* sym_first, char* sym_second) {
    char sym_current = *sym_first;
	*sym_first = *sym_second;
	*sym_second = sym_current;
}

void reverse_line(const char* file_name, char* rev_line) {
    size_t len_file_name = strlen(file_name);

    strncpy(rev_line, file_name, len_file_name);

    int mid_len =  len_file_name / 2;
    for (int i = 0; i < mid_len; i++) {
       swap(&rev_line[i], &rev_line[len_file_name - i - INCREASE_POS]);
    }
}

bool is_curr_or_prev_dir(char* dir) {
    return (strcmp(dir, ".") == 0 || strcmp(dir, "..") == 0);

}

void find_name_folder(const char* path_origin_folder, char** path_to_folder, char** new_folder) {
    char* pos_slash = strrchr(path_origin_folder, '/');
    *new_folder = (!pos_slash) ? path_origin_folder : pos_slash + INCREASE_POS;

    size_t folder_len = pos_slash ? (size_t)(pos_slash - path_origin_folder + INCREASE_POS) : START_POS;

    strncpy(*path_to_folder, path_origin_folder, folder_len);

    if(!pos_slash)
        strncpy(*path_to_folder, "./", sizeof("./"));
}

ssize_t create_reverse_file(const char* input_path, const char* output_path) {
    FILE* input_file = fopen(input_path, "r");
    if (!input_file) {
        perror("failed to open input file");
        return EXIT_FAILURE;
    }

    FILE* output_file = fopen(output_path, "w");
    if (!output_file) {
        close_files(1, input_file);
        perror("failed to open output file");
        return EXIT_FAILURE;
    }

    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    char buffer[BUFFER_SIZE];
    long bytes_left = file_size;

    while (bytes_left > 0) {
        long bytes_to_read = bytes_left < BUFFER_SIZE ? bytes_left : BUFFER_SIZE;

        size_t bytes_read = fread(buffer, 1, bytes_to_read, input_file);
        if (bytes_read == 0) {
            perror("failed to read input file");
            fclose(input_file);
            fclose(output_file);
            return EXIT_FAILURE;
        }

        for (int i = 0; i < bytes_read / 2; i++) {
            char temp = buffer[i];
            buffer[i] = buffer[bytes_read - i - 1];
            buffer[bytes_read - i - 1] = temp;
        }
        bytes_left -= bytes_read;

        fseek(output_file, bytes_left, SEEK_SET);
        size_t bytes_written = fwrite(buffer, 1, bytes_read, output_file);
        if (bytes_written == 0) {
            perror("failed to write output file");
            close_files(2, input_file, output_file);
            return ERROR;
        }
    }

    close_files(2, input_file, output_file);
    return EXIT_SUCCESS;

}

ssize_t create_reverse_folder(const char*, const char*);

ssize_t fill_folder(const char* path_origin_folder, const char* path_reverse_folder) {
    DIR *dir;
    struct dirent* d_entry;

    if ((dir = opendir(path_origin_folder)) == NULL) {
         perror(path_origin_folder);
         return ERROR;
    }

    char* new_rev_folder = memory_allocation(MAX_LEN_NAME_FILE  * sizeof (char));
    char* path_new_rev_folder = memory_allocation(MAX_LEN_NAME_FILE + strlen(path_reverse_folder) * sizeof(char));
    char* path_new_folder = memory_allocation((MAX_LEN_NAME_FILE + strlen(path_origin_folder)) * sizeof(char));

    ssize_t ret = START_POS;

    while ((d_entry = readdir(dir)) != NULL) {

        if(strlen(d_entry->d_name) > MAX_LEN_NAME_FILE) {
            ssize_t relocation_size = strlen((d_entry->d_name) - MAX_LEN_NAME_FILE + INCREASE_POS);

            new_rev_folder = reallocation_memory(new_rev_folder, relocation_size);
            path_new_rev_folder = reallocation_memory(path_reverse_folder, relocation_size);
            path_new_folder = reallocation_memory(path_new_folder, relocation_size);
        }

        reverse_line(d_entry->d_name, new_rev_folder);
        sprintf(path_new_rev_folder, "%s/%s", path_reverse_folder, new_rev_folder);
        sprintf(path_new_folder, "%s/%s", path_origin_folder, d_entry->d_name);

        if (d_entry->d_type == DT_DIR && !is_curr_or_prev_dir(d_entry->d_name)) {
            ret = create_reverse_folder(path_new_folder, path_new_rev_folder);
        } else if (d_entry->d_type == DT_REG) {
            printf("\n%s\n",path_new_rev_folder);
            ret = create_reverse_file(path_new_folder, path_new_rev_folder);
        }

        if (ret == ERROR ||
            !(d_entry->d_type == DT_DIR || d_entry->d_type == DT_REG)) {
            freeing_memory(3, new_rev_folder, path_new_rev_folder, path_new_folder);
            closedir(dir);
            return ERROR;
        }
        memset(new_rev_folder, 0, strlen(path_new_folder) * sizeof(char));
        memset(path_new_rev_folder, 0, strlen(path_new_folder) * sizeof(char));
        memset(path_new_folder, 0, strlen(path_new_folder) * sizeof(char));
    }

    freeing_memory(3, new_rev_folder, path_new_rev_folder, path_new_folder);
    closedir(dir);
    return EXIT_SUCCESS;
}

ssize_t create_reverse_folder(const char* path_origin_folder,
                              const char* path_reverse_folder) {

    int ret = mkdir(path_reverse_folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if(ret == ERROR)
        return ERROR;

    fill_folder(path_origin_folder, path_reverse_folder);
    return EXIT_SUCCESS;
}

size_t maximum_size_path(int argc, char** argv) {
    size_t max_size = 0, cur_len;
    for(int i = 1; i < argc; ++i) {
        cur_len = strlen(argv[i]);
        if(max_size < cur_len)
            max_size = cur_len;
    }
    return max_size;
}

ssize_t parse_command_line(int argc, char** argv) {

    if (argc < MIN_NUMBER_CORRECT_ARGS)
        return EXIT_FAILURE;

    size_t max_size = maximum_size_path(argc, argv);
    char* rev_folder = memory_allocation(max_size * sizeof(char));
    char* path_to_folder = memory_allocation(max_size * sizeof(char));
    char* origin_folder = NULL;

    for (int i = 1; i < argc; ++i) {
        find_name_folder(argv[i], &path_to_folder, &origin_folder);
        reverse_line(origin_folder, rev_folder);
        strcat(path_to_folder, rev_folder);

        if (create_reverse_folder(argv[i], path_to_folder) == ERROR) {
            freeing_memory(2, path_to_folder, rev_folder);
            return ERROR;
        }
    }

    freeing_memory(2, path_to_folder, rev_folder);
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    ssize_t ret = parse_command_line(argc, argv);

    if (ret == ERROR) {
        perror(strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}