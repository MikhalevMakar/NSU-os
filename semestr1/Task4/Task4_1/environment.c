#include <stdio.h>
#include <stdlib.h>

enum  return_status {
    ERROR_HAS_OCCURRED = -1,
    Ok = 0
};

enum consts {
    CORRECT_NUMBER_ARGS = 2,
    FLAG_OVERRIDE = 1
};

enum return_status set_environment(char* name_env, char*  new_value_env) {
    char* env = getenv(name_env);
    if(env == NULL) {
        perror("If the variable name is not in the current environment\n");
        return ERROR_HAS_OCCURRED;
    }

    int result = setenv(env, new_value_env, FLAG_OVERRIDE);
    if (result == ERROR_HAS_OCCURRED) {
        perror("Error setting environment variable\n");
        return ERROR_HAS_OCCURRED;
    }
    return Ok;
}

enum return_status print_environment(char* name_env) {
    char* env = getenv(name_env);
    if(env == NULL) {
        perror("If the variable name is not in the current environment\n");
        return ERROR_HAS_OCCURRED;
    }
    printf("Print env: %s", env);
    return Ok;
}

enum return_status  run_task(char* name_env) {
    enum return_status ret = print_environment(name_env);
    if(ret == ERROR_HAS_OCCURRED) {
        return ERROR_HAS_OCCURRED;
    }

    ret = set_environment(name_env, "TEST");
    if(ret == ERROR_HAS_OCCURRED) {
        return ERROR_HAS_OCCURRED;
    }

    ret = print_environment(name_env);
    if(ret == ERROR_HAS_OCCURRED) {
        return ERROR_HAS_OCCURRED;
    }

}


int main(int argc, char** argv) {
    if(argc != CORRECT_NUMBER_ARGS) {
        fprintf(stderr, "Error: wrong number of CORRECT_NUMBER_ARGS = 2\n");
        return EXIT_FAILURE;
    }

    enum return_status ret = run_task(argv[1]);
    return (ret == Ok) ? EXIT_SUCCESS : EXIT_FAILURE;
}
