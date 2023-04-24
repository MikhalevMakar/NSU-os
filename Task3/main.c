#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>

enum pos {
    INCREASE_POS = 1,
    START_POS = 0
};

enum constants {
    BUFFER_SIZE = 256,
    MAX_LEN_NAME_FILE = 256,
    ERROR = -1,
    MIN_NUMBER_CORRECT_ARGS = 2,
};

void swap(char* sym_first, char* sym_second) {
    char sym_current = *sym_first;
	*sym_first = *sym_second;
	*sym_second = sym_current;
}

void reverse_line(char* line, const size_t size) {
    int mid_len =  size / 2;
    for (int i = 0; i < mid_len; i++) {
       swap(&line[i], &line[size - i - INCREASE_POS]);
    }
}

bool is_curr_or_prev_dir(char* dir) {
        return (strcmp(dir, ".") == 0 || strcmp(dir, "..") == 0);
}

void find_name_folder(const char* path_origin_folder, char** path_to_folder, char** new_folder) {
    char* pos_slash = strrchr(path_origin_folder, '/'); // поиск первого вхождения символа
    *new_folder = (!pos_slash) ? (char*)path_origin_folder : pos_slash + INCREASE_POS;

    size_t folder_len = pos_slash ? (size_t)(pos_slash - path_origin_folder + INCREASE_POS) : START_POS;

    strncpy(*path_to_folder, path_origin_folder, folder_len);

    if(!pos_slash)
        strncpy(*path_to_folder, "./", sizeof("./"));
}

ssize_t create_reverse_file(const char* input_path, const char* output_path) {
    FILE* input_file = fopen(input_path, "rb");
    if (!input_file) {
        perror("failed to open input file");
        return EXIT_FAILURE;
    }

    FILE* output_file = fopen(output_path, "wb");
    if (!output_file) {
        fclose(input_file);
        fclose(output_file);
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

        reverse_line(buffer, bytes_read);
        bytes_left -= bytes_read;

        fseek(output_file, bytes_left, SEEK_SET);
        size_t bytes_written = fwrite(buffer, 1, bytes_read, output_file);
        if (bytes_written == 0) {
            perror("failed to write output file");
            fclose(input_file);
            fclose(output_file);
            return ERROR;
        }
    }

    fclose(input_file);
    fclose(output_file);
    return EXIT_SUCCESS;
}

bool is_correct_len_folder(char* name_rev_folder, char* path_new_rev_folder, char* path_new_origin_folder,
                           const struct dirent* d_entry) {
      if(strlen(d_entry->d_name) > MAX_LEN_NAME_FILE) {
            ssize_t relocation_size = strlen(d_entry->d_name - MAX_LEN_NAME_FILE + INCREASE_POS);

            name_rev_folder = (char*)realloc(name_rev_folder, relocation_size * sizeof(char));
            path_new_rev_folder = (char*)realloc(path_new_rev_folder, relocation_size * sizeof(char));
            path_new_origin_folder = (char*)realloc(path_new_origin_folder, relocation_size * sizeof(char));

            if(name_rev_folder == NULL || path_new_rev_folder == NULL || path_new_origin_folder == NULL) {
                 fprintf(stderr, "Error: failed to reallocate memory\n");
                 free(name_rev_folder);
                 free(path_new_rev_folder);
                 free(path_new_origin_folder);
                 return false;
            }
       }
      return true;
}

ssize_t create_reverse_folder(const char*, const char*);

ssize_t fill_folder(const char* path_origin_folder, const char* path_reverse_folder) {
    DIR *dir;
    struct dirent* d_entry;

    if ((dir = opendir(path_origin_folder)) == NULL) {
         perror(path_origin_folder);
         return ERROR;
    }

    char* name_rev_folder = (char*)malloc(MAX_LEN_NAME_FILE  * sizeof (char));
    char* path_new_rev_folder = (char*)malloc(MAX_LEN_NAME_FILE + strlen(path_reverse_folder) * sizeof(char));
    char* path_new_origin_folder = (char*)malloc((MAX_LEN_NAME_FILE + strlen(path_origin_folder)) * sizeof(char));
    
    if(name_rev_folder == NULL || path_new_rev_folder == NULL || path_new_origin_folder == NULL) {
        fprintf(stderr, "Error: failed to reallocate memory\n");
        free(name_rev_folder);
        free(path_new_rev_folder);
        free(path_new_origin_folder);
        return ERROR;
    }

    ssize_t ret = START_POS;

    while ((d_entry = readdir(dir)) != NULL) {

        if(!is_correct_len_folder(name_rev_folder,
                                 path_new_rev_folder,
                                 path_new_origin_folder,
                                 d_entry)) {
            return ERROR;
        }

        name_rev_folder = strncpy(name_rev_folder, d_entry->d_name, strlen(d_entry->d_name));
        reverse_line(name_rev_folder, strlen(name_rev_folder));

        sprintf(path_new_rev_folder, "%s/%s", path_reverse_folder, name_rev_folder);
        sprintf(path_new_origin_folder, "%s/%s", path_origin_folder, d_entry->d_name);

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

        memset(name_rev_folder, 0, strlen(path_new_origin_folder) * sizeof(char));
        memset(path_new_rev_folder, 0, strlen(path_new_origin_folder) * sizeof(char));
        memset(path_new_origin_folder, 0, strlen(path_new_origin_folder) * sizeof(char));
    }

    free(name_rev_folder);
    free(path_new_rev_folder);
    free(path_new_origin_folder);
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
        return ERROR;

    size_t max_size = maximum_size_path(argc, argv);
    char* rev_folder = (char*)malloc(max_size * sizeof(char));
    char* path_to_folder = (char*)malloc(max_size * sizeof(char));
    char* origin_folder = NULL;

    for (int i = 1; i < argc; ++i) {
        find_name_folder(argv[i], &path_to_folder, &origin_folder);

        strncpy(rev_folder, origin_folder, strlen(origin_folder));
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
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    ssize_t ret = parse_command_line(argc, argv);

    if (ret == ERROR) {
        fprintf(stderr, "Error: wrong number of arguments < MIN_NUMBER_CORRECT_ARGS = 2\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}