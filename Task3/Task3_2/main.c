#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "set_commands.h"

enum consts {
    CORRECT_NUMBER_ARGS = 2,
    ERROR = -1,
    SUCCESS = 0
};

char* get_name_command() {
    FILE *ptr_file_command = fopen("/proc/self/status", "rb");
    if (ptr_file_command == NULL) {
        perror("an error occurred in the fopen()\n");
        exit(1);
    }

    int skip_name = 6;
    char *name_command = NULL;
    size_t linecapp = 0;

    ssize_t ret = getline(&name_command, &linecapp, ptr_file_command);

    if(ret == ERROR) {
        fclose(ptr_file_command);
        perror("an error occurred in the getline()\n");
        exit(1);
    }

    fclose(ptr_file_command);
    return name_command + skip_name;
}

enum return_status selection_command(const char* name_command, const char* name_util) {
    if(strcmp(name_command, "mkdir\n") == SUCCESS) {
        return make_dir(name_util); //a
    } else if(strcmp(name_command, "print_dir\n") == SUCCESS) {
        return print_dir(name_util); //b
    } else if(strcmp(name_command, "rmdir\n") == SUCCESS) {
        return remove_dir(name_util); //c
    } else if(strcmp(name_command, "touch_file\n") == SUCCESS) {
       return touch_file(name_util); //d
    } else if(strcmp(name_command, "print_file\n") == SUCCESS) {
        return print_file(name_util);//e
    } else if(strcmp(name_command, "remove_file\n") == SUCCESS) {
        return remove_file(name_util); //f
    } else if(strcmp(name_command, "ln_symbol_link\n") == SUCCESS) {
        return ln_symbol_link(name_util); //g
    } else if(strcmp(name_command, "print_sym_link\n") == SUCCESS) {
        return print_sym_link(name_util); //h
    } else if(strcmp(name_command, "read_file_from_symbol_link\n") == SUCCESS) {
        return read_file_from_symbol_link(name_util); //i
    } else if(strcmp(name_command, "remove_file\n") == SUCCESS) {
        return remove_sym_link(name_util);  //j
    } else if(strcmp(name_command, "ln_hard_link\n") == SUCCESS) {
        return ln_hard_link(name_util); //k
    } else if(strcmp(name_command, "hard_unlink\n") == SUCCESS) {
        return hard_unlink(name_util); //l
    } else if(strcmp(name_command, "print_permitted\n") == SUCCESS) {
        return print_permitted(name_util); //m
    } else if(strcmp(name_command, "change_permitted\n") == SUCCESS) {
        return change_permitted(name_util); //n
    }

    fprintf(stderr, "This command wasn't found\n");
    return ERROR_HAS_OCCURRED;
}

int main(int argc, char** argv) {
    if(argc != CORRECT_NUMBER_ARGS) {
        perror("Error: wrong number of CORRECT_NUMBER_ARGS = 2\n");
        return EXIT_FAILURE;
    }

    ssize_t ret = selection_command("change_permitted\n", argv[1]);
    return ret;
}
