#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdarg.h>

enum pos {
    INCREASE_POS = 1,
    START_POS = 0
};

enum constants {
    BUFFER_SIZE = 256,
    MAX_LEN_NAME_FILE = 256,
    ERROR = -1,
    MIN_NUMBER_CORRECT_ARGS = 2,
    OK = 0
};

/// \param count
/// \param ...
/// \return void
void override_free_memory(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; ++i) {
        if(va_arg(args, char*) != NULL) {
               free(va_arg(args, char*));
        }
    }
    va_end(args);
}

///
/// \param count
/// \param ...
/// \return void
void override_close_files(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; ++i) {
        FILE* file = va_arg(args, FILE*);
        if (file != NULL) {
            fclose(file);
        }
    }
    va_end(args);
}

/// \param sym_first
/// \param sym_second
void swap(char* sym_first, char* sym_second) {
    char sym_current = *sym_first;
	*sym_first = *sym_second;
	*sym_second = sym_current;
}

/// \param line
/// \param size
void reverse_line(char* line, const size_t size) {
    int mid_len =  size / 2;
    for (int i = 0; i < mid_len; i++) {
       swap(&line[i], &line[size - i - INCREASE_POS]);
    }
}

/// \param dir
/// \return bool
bool is_curr_or_prev_dir(char* dir) {
        return (strcmp(dir, ".") == 0 || strcmp(dir, "..") == 0);
}

/// \param path_origin_folder
/// \param path_to_folder
/// \param new_folder
/// \return STATUS ERROR/OK
ssize_t find_name_folder(char* path_origin_folder, char** path_to_folder, char** new_folder) {
    char* pos_slash = strrchr(path_origin_folder, '/');
    *new_folder = (!pos_slash) ? (char*)path_origin_folder : pos_slash + INCREASE_POS;

    size_t folder_len = pos_slash ? (size_t)(pos_slash - path_origin_folder + INCREASE_POS) : START_POS;

    *path_to_folder = strncpy(*path_to_folder, path_origin_folder, folder_len);

    if(!pos_slash) {
        int ret = snprintf(*path_to_folder, sizeof(path_to_folder) + 2, "%s", "./");
        if(ret == ERROR) {
             perror("An error occurred while calling snprintf");
             return ERROR;
        }
    }
    return OK;
}

/// \param input_path
/// \param output_path
/// \return STATUS ERROR/OK
ssize_t create_reverse_file(const char* input_path, const char* output_path) {
    FILE* input_file = fopen(input_path, "rb");
    ssize_t ret = OK;

    if (input_file == NULL) {
        perror("The input file was not opened");
        return ERROR;
    }

    FILE* output_file = fopen(output_path, "wb");
    if (output_file == NULL) {
        perror("The output file was not opened");
        fclose(input_file);
        return ERROR;
    }

    ret = fseek(input_file, 0, SEEK_END);
    if(ret == ERROR) {
        fclose(input_file);
        fclose(output_file);
        perror("Error in call file seek");
        return ERROR;
    }

    long file_size = ftell(input_file);
    ret = fseek(input_file, 0, SEEK_SET);
    if(ret == ERROR) {
        fclose(input_file);
        fclose(output_file);
        perror("Error in call file seek");
        return ERROR;
    }

    char buffer[BUFFER_SIZE];
    long bytes_left = file_size;

    while (bytes_left > 0) {
        long bytes_to_read = bytes_left < BUFFER_SIZE ? bytes_left : BUFFER_SIZE;

        size_t bytes_read = fread(buffer, 1, bytes_to_read, input_file);
        if(feof(input_file) ||  ferror(input_file)) {
             perror("Failed to read input file when call file read");
             fclose(input_file);
            fclose(output_file);
            return ERROR;
        }

        reverse_line(buffer, bytes_read);
        bytes_left -= bytes_read;

        ret = fseek(output_file, bytes_left, SEEK_SET);
        if(ret == ERROR) {
            fclose(input_file);
            fclose(output_file);
            perror("Error in call file seek");
            return ERROR;
        }

        size_t bytes_written = fwrite(buffer, 1, bytes_read, output_file);
        if (bytes_written == ERROR) {
            perror("Error call write");
            fclose(input_file);
            fclose(output_file);
            return ERROR;
        }
    }

    fclose(input_file);
    fclose(output_file);
    return OK;
}

bool is_correct_len_folder(char** name_rev_folder, char** path_rev_folder, char** path_origin_folder,
                           const struct dirent* d_entry) {
    char* name_new_rev_folder, *path_new_rev_folder, *path_new_origin_folder;

    if (strlen(d_entry->d_name) > MAX_LEN_NAME_FILE) {
        ssize_t relocation_size = strlen(d_entry->d_name) - MAX_LEN_NAME_FILE + INCREASE_POS;

        name_new_rev_folder = (char*)realloc(*name_rev_folder, relocation_size * sizeof(char));
        if (name_new_rev_folder == NULL) {
             perror("Memory allocation name_rev_folder error\n");
            return false;
        }

        *name_rev_folder = name_new_rev_folder;

        path_new_rev_folder = (char*)realloc(*path_rev_folder, relocation_size * sizeof(char));
        if (path_new_rev_folder == NULL) {
            perror("Memory allocation path_new_rev_folder error\n");
            return false;
        }
        *path_rev_folder = path_new_rev_folder;

        path_new_origin_folder = (char*)realloc(*path_origin_folder, relocation_size * sizeof(char));
        if (path_new_origin_folder == NULL) {
            perror("Error: failed to reallocate path_new_origin_folder memory\n");
            return false;
        }
        *path_origin_folder = path_new_origin_folder;
    }
    return true;
}

ssize_t create_reverse_folder(const char*, const char*);

ssize_t fill_folder(const char* path_origin_folder, const char* path_reverse_folder) {;
    struct dirent* d_entry;
    DIR* dir = opendir(path_origin_folder);
    if (dir == NULL) {
         perror("The file was not open dir");
         return ERROR;
    }

    char* name_rev_folder = (char*)malloc(MAX_LEN_NAME_FILE  * sizeof (char));
    if(name_rev_folder == NULL) {
         closedir(dir);
         perror("Error: failed to reallocate memory name_rev_folder\n");
         return ERROR;
    }

    char* path_new_rev_folder = (char*)malloc(MAX_LEN_NAME_FILE + strlen(path_reverse_folder) * sizeof(char));
     if(path_new_rev_folder == NULL) {
         closedir(dir);
         free(name_rev_folder);
         perror("Error: failed to reallocate memory path_new_rev_folder\n");
         return ERROR;
    }
    char* path_new_origin_folder = (char*)malloc((MAX_LEN_NAME_FILE + strlen(path_origin_folder)) * sizeof(char));
    if(path_new_origin_folder == NULL) {
         closedir(dir);
         free(name_rev_folder);
         free(path_new_rev_folder);
         perror("Error: failed to reallocate memory path_new_origin_folder\n");
         return ERROR;
    }

    ssize_t ret = OK;

    while ((d_entry = readdir(dir)) != NULL) {

        if(!is_correct_len_folder(&name_rev_folder,
                                  &path_new_rev_folder,
                                  &path_new_origin_folder,
                                  d_entry)) {
            free(name_rev_folder);
            free(path_new_rev_folder);
            free(path_new_origin_folder);
            closedir(dir);
            return ERROR;
        }

        name_rev_folder = strncpy(name_rev_folder, d_entry->d_name, strlen(d_entry->d_name));
        reverse_line(name_rev_folder, strlen(name_rev_folder));


        int ret_reverse = sprintf(path_new_rev_folder, "%s/%s", path_reverse_folder, name_rev_folder);
        int ret_origin = sprintf(path_new_origin_folder, "%s/%s", path_origin_folder, d_entry->d_name);

        if(ret_reverse < 0 || ret_origin < 0) {
            closedir(dir);
            free(name_rev_folder);
            free(path_new_rev_folder);
            free(path_new_origin_folder);
            perror("Error call sprintf");
            return ERROR;
        }

        if (d_entry->d_type == DT_DIR && !is_curr_or_prev_dir(d_entry->d_name)) {
            ret = create_reverse_folder(path_new_origin_folder, path_new_rev_folder);
        } else if (d_entry->d_type == DT_REG) {
            ret = create_reverse_file(path_new_origin_folder, path_new_rev_folder);
        }

        if (ret == ERROR ||
            !(d_entry->d_type == DT_DIR || d_entry->d_type == DT_REG)) {
            free(name_rev_folder);
            free(path_new_rev_folder);
            free(path_new_origin_folder);
            closedir(dir);
            return ERROR;
        }

        name_rev_folder = memset(name_rev_folder, 0, strlen(path_new_origin_folder) * sizeof(char));
        path_new_rev_folder = memset(path_new_rev_folder, 0, strlen(path_new_origin_folder) * sizeof(char));
        path_new_origin_folder = memset(path_new_origin_folder, 0, strlen(path_new_origin_folder) * sizeof(char));
    }

    free(name_rev_folder);
    free(path_new_rev_folder);
    free(path_new_origin_folder);
    closedir(dir);
    return OK;
}

/// \param path_origin_folder
/// \param path_reverse_folder
/// \return  STATUS: ERROR/OK
ssize_t create_reverse_folder(const char* path_origin_folder,
                              const char* path_reverse_folder) {

    int ret = mkdir(path_reverse_folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if(ret == ERROR) {
        perror("Error in calling mkdir");
        return ERROR;
    }

    fill_folder(path_origin_folder, path_reverse_folder);
    return OK;
}

/// \param argc
/// \param argv
/// \return max_size
size_t maximum_size_path(int argc, char** argv) {
    size_t max_size = 0, cur_len;
    for(int i = 1; i < argc; ++i) {
        cur_len = strlen(argv[i]);
        if(max_size < cur_len)
            max_size = cur_len;
    }
    return max_size;
}

/// \param argc
/// \param argv
/// \return  STATUS: ERROR/OK
ssize_t parse_command_line(int argc, char** argv) {
    if (argc < MIN_NUMBER_CORRECT_ARGS) {
        perror("Error: wrong number of arguments < MIN_NUMBER_CORRECT_ARGS = 2\n");
        return ERROR;
    }

    size_t max_size = maximum_size_path(argc, argv);
    char* rev_folder = (char*)malloc(max_size * sizeof(char));
    char* path_to_folder = (char*)malloc(max_size * sizeof(char));
    char* origin_folder = NULL;

    for (int i = 1; i < argc; ++i) {
        ssize_t ret = find_name_folder(argv[i], &path_to_folder, &origin_folder);
        if (ret == ERROR) {
            free(path_to_folder);
            free(rev_folder);
            return ERROR;
        }

        rev_folder = strncpy(rev_folder, origin_folder, strlen(origin_folder));
        reverse_line(rev_folder, strlen(rev_folder));
        strcat(path_to_folder, rev_folder);

        if (create_reverse_folder(argv[i], path_to_folder) == ERROR) {
            free(path_to_folder);
            free(rev_folder);
            return ERROR;
        }
    }

    free(path_to_folder);
    free(rev_folder);
    return OK;
}

/// \param argc
/// \param argv
/// \return EXIT_FAILURE : EXIT_SUCCESS
int main(int argc, char** argv) {
    ssize_t ret = parse_command_line(argc, argv);
    return  (ret == ERROR) ? EXIT_FAILURE : EXIT_SUCCESS;
}