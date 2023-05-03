#include "set_commands.h"

#include <stdio.h>
#include  <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>

enum constant_read {
    BUFFER_SIZE = 256,
};

enum return_status make_dir(const char* name_dir) {
    int ret = mkdir(name_dir, DEFAULT_DIR);

    if(ret == ERROR_HAS_OCCURRED) {
        perror("Error occurred while creating dir");
        return ERROR_HAS_OCCURRED;
    }
    return OKEY;
}

enum return_status is_dir(const char* name_dir) {
    struct stat stat_dir;

    int ret = stat(name_dir, &stat_dir);
    if (ret == ERROR_HAS_OCCURRED) {
        perror("Error in call stat\n");
        return ERROR_HAS_OCCURRED;
    }

    if(!S_ISDIR(stat_dir.st_mode)) {
        perror("The file is not a dir\n");
         return ERROR_HAS_OCCURRED;
    }
    return OKEY;
}

enum return_status print_dir(const char* path_name_dir) {

    DIR* dir = opendir(path_name_dir);

    if (dir == NULL) {
         perror("The file wasn't open dir\n");
         return ERROR_HAS_OCCURRED;
    }
    struct dirent* d_entry  = readdir(dir);

    while (d_entry != NULL) {
        printf("%s\n", d_entry->d_name); //maybe call write file
        d_entry = readdir(dir);
    }

    ssize_t ret = closedir(dir);
    if(ret == ERROR_HAS_OCCURRED) {
        perror("Error occurred while close dir\n");
        return ERROR_HAS_OCCURRED;
    }
    return OKEY;
}

enum return_status remove_dir(const char* name_dir) {
    int ret = rmdir(name_dir);
    if(ret == ERROR_HAS_OCCURRED) {
        perror("Error occurred while remove dir\n");
        return ERROR_HAS_OCCURRED;
    }
    return OKEY;
}

enum return_status touch_file(const char* name_file) {
    FILE* ptr_file = fopen(name_file, "a+");
    if(ptr_file == NULL) {
        perror("Error occurred while touch file\n");
        return ERROR_HAS_OCCURRED;
    }

    int ret  = fclose(ptr_file);
    if(ret != OKEY) {
        perror("Error occurred while close file\n");
        return ERROR_HAS_OCCURRED;
    }
    return OKEY;
}

enum return_status is_file_reg(const char* name_file) {
    struct stat stat_file;

    int ret = stat(name_file, &stat_file);
    if (ret == ERROR_HAS_OCCURRED) {
        perror("Error in call stat\n");
        return ERROR_HAS_OCCURRED;
    }

    if(!S_ISREG(stat_file.st_mode)) {
        perror("The file is not a reg\n");
         return ERROR_HAS_OCCURRED;
    }
    return OKEY;
}

enum return_status write_to_file_implements(const char* buffer, const size_t size_written, FILE* stream) {
    ssize_t sum_written = 0;

    while (sum_written < size_written) {
        int written = fwrite(buffer + sum_written, sizeof(char), size_written - sum_written, stream);

        if (written <= ERROR_HAS_OCCURRED) {
            perror("Error in call fwrite");
            return ERROR_HAS_OCCURRED;
        }
        sum_written += written;
    }
    return OKEY;
}

enum return_status write_file(FILE* ptr_file) {
    long file_size = ftell(ptr_file);
    printf("%ld", file_size);
    ssize_t ret = fseek(ptr_file, 0, SEEK_SET);

    if(ret == ERROR_HAS_OCCURRED) {
        fclose(ptr_file);
        perror("Error in call file seek\n");
        return ERROR_HAS_OCCURRED;
    }

    char buffer[BUFFER_SIZE];
    long bytes_left = file_size;
    while (bytes_left > 0) {
        long bytes_to_read = bytes_left < BUFFER_SIZE ? bytes_left : BUFFER_SIZE;

        size_t bytes_read = fread(buffer, 1, bytes_to_read, ptr_file);
        if(feof(ptr_file) ||  ferror(ptr_file)) {
            perror("Failed to read input file when call file read\n");
            return ERROR_HAS_OCCURRED;
        }
        bytes_left -= bytes_read;

        enum return_status writing_status = write_to_file_implements(buffer, bytes_read, stdout);
        if (writing_status == ERROR_HAS_OCCURRED) {
            return ERROR_HAS_OCCURRED;
        }
    }

    return OKEY;
}

enum return_status print_file(const char* path_name_file) {
     if(is_file_reg(path_name_file) == ERROR_HAS_OCCURRED)
         return ERROR_HAS_OCCURRED;

     FILE* ptr_file = fopen(path_name_file, "a+");
     if(ptr_file == NULL) {
        perror("Error occurred while open file\n");
        return ERROR_HAS_OCCURRED;
     }

     enum return_status ret = write_file(ptr_file);
     fclose(ptr_file);
     return ret;
}

enum return_status remove_file(const char* path_name_file) {
    int ret = remove(path_name_file);
    if(ret == ERROR_HAS_OCCURRED) {
        perror("Error occurred while remove file\n");
        return ERROR_HAS_OCCURRED;
    }
    return OKEY;
}

enum return_status is_sym_link(const char* sym_link) {
    struct stat stat_link;

    int ret = stat(sym_link, &stat_link);
    if (ret == ERROR_HAS_OCCURRED) {
        perror("Error in call stat\n");
        return ERROR_HAS_OCCURRED;
    }

    if(!S_ISLNK(stat_link.st_mode)) {
        perror("The file is not a symbolic link\n");
         return ERROR_HAS_OCCURRED;
    }
    return OKEY;
}

enum return_status ln_symbol_link(const char* name_file) {
    int ret = symlink(name_file, "symbol_link");
    if(ret == ERROR_HAS_OCCURRED) {
        perror("Error occurred while symlink\n");
        return ERROR_HAS_OCCURRED;
    }
    return OKEY;
}

enum return_status print_sym_link(const char* sym_link) {
    if(is_sym_link(sym_link) == ERROR_HAS_OCCURRED) {
        return ERROR_HAS_OCCURRED;
    }

    char buffer[BUFFER_SIZE];
    ssize_t sum_written = readlink(sym_link, buffer, BUFFER_SIZE);
    if (sum_written == ERROR_HAS_OCCURRED) {
        perror("Error in call readlink");
        return ERROR_HAS_OCCURRED;
    }

    enum return_status writing_status = write_to_file_implements(buffer, sum_written, stdout);
    if (writing_status == ERROR_HAS_OCCURRED) {
        return ERROR_HAS_OCCURRED;
    }
    return OKEY;
}

enum return_status read_file_from_symbol_link(const char* sym_link) {
    if(is_sym_link(sym_link) == ERROR_HAS_OCCURRED) {
        return ERROR_HAS_OCCURRED;
    }

    char buffer[BUFFER_SIZE];
    ssize_t ret = readlink(sym_link, buffer, BUFFER_SIZE);
    if (ret == ERROR_HAS_OCCURRED) {
        perror("Error in call readlink");
        return ERROR_HAS_OCCURRED;
    }

    FILE* ptr_file = fopen(buffer, "a+"); //TODO change rb
    if (ptr_file == NULL) {
        perror("Error in call fopen");
        return ERROR_HAS_OCCURRED;
    }

    ret = write_file(ptr_file);
    fclose(ptr_file);
    return ret;
}

enum return_status remove_sym_link(const char* sym_link) {
    enum return_status ret = is_sym_link(sym_link);
    if(ret == ERROR_HAS_OCCURRED)
        return ERROR_HAS_OCCURRED;

    return remove_file(sym_link);
}

enum return_status ln_hard_link(const char* hard_link) {
    int ret = link(hard_link, "hard_link");
    if (ret == ERROR_HAS_OCCURRED) {
        perror("Error in call link\n");
        return ERROR_HAS_OCCURRED;
    }
    return OKEY;
}

enum return_status hard_unlink(const char* hard_link) {
    int ret = unlink(hard_link);
    if (ret == ERROR_HAS_OCCURRED) {
        perror("Error in call unlink\n");
        return ERROR_HAS_OCCURRED;
    }
    return OKEY;
}

enum return_status print_permitted(const char* name_file) {
    struct stat buff;

    int ret =  stat(name_file, &buff);
    if (ret == ERROR_HAS_OCCURRED) {
        perror("Error in call stat\n");
        return ERROR_HAS_OCCURRED;
    }

    printf("permissions: %o\n", buff.st_mode & ALL_FOR_ALL);
    printf("count hard links: %hu\n", buff.st_nlink);

    return OKEY;
}

enum return_status change_permitted(const char* name_file) {
    struct stat buff;
    int ret =  stat(name_file, &buff);
    if (ret == ERROR_HAS_OCCURRED) {
        perror("Error in call stat\n");
        return ERROR_HAS_OCCURRED;
    }

    mode_t new_mode = (mode_t) (rand() % ALL_FOR_ALL);

    ret = chmod(name_file, new_mode);
    if (ret == ERROR_HAS_OCCURRED) {
        perror("Error changing file permissions\n");
        return ERROR_HAS_OCCURRED;
    }
    return OKEY;
}