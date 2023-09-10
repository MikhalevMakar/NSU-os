enum  return_status {
    ERROR_HAS_OCCURRED = -1,
    OKEY = 0
};

enum permitted {
    DEFAULT_DIR = 0775,
    ALL_FOR_ALL = 0777
};

enum return_status make_dir(const char* name_dir);
enum return_status print_dir(const char* path_name_dir);
enum return_status remove_dir(const char* name_dir);
enum return_status touch_file(const char* name_file);
enum return_status print_file(const char* path_name_file);
enum return_status remove_file(const char* path_name_file);
enum return_status ln_symbol_link(const char* name_file);
enum return_status print_sym_link(const char* sym_link);
enum return_status read_file_from_symbol_link(const char* sym_link);
enum return_status remove_sym_link(const char* sym_link);
enum return_status ln_hard_link(const char* hard_link);
enum return_status hard_unlink(const char* hard_link);
enum return_status print_permitted(const char* name_file);
enum return_status change_permitted(const char* name_file);