#define VERSION_HTTP " HTTP/1.1"
#define TYPE_GET "GET"

enum {
    MAX_BUFFER_SIZE = 4096,
    SIZE_RESPONSE = 512,
    SIZE_URL = 256,
    PORT = 80,
    BACKLOG = 10,
    SIZE_SPACE = 1,
    START_POS_READ = 0,
    START_POS_WRITE = 0,
    ZERO_READ_BYTE = 0,
    SUCCESSFUL_PARSE_ADR = 0
};

enum error {
    SOCKET_ERROR = -1,
    SEND_ERROR = -1,
    CONNECTION_ERROR = -1,
    ACCEPT_ERROR = -1
};

//http://127.0.0.1/https://ya.ru/?utm_referrer=https%3A%2F%2Fwww.google.com%2F