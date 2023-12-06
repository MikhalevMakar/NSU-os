//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <unistd.h>
//#include <arpa/inet.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <errno.h>
//
//#define MAX_BUFFER_SIZE 4096
//#define PORT 8080
//
//void handle_client(int client_socket) {
//    char buffer[MAX_BUFFER_SIZE];
//    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
//
//    if (bytes_received <= 0) {
//        perror("Error receiving data from client");
//        close(client_socket);
//        return;
//    }
//
//    buffer[bytes_received] = '\0';
//
//    // Парсим URL из запроса клиента
//    char* url_start = strstr(buffer, "http://") + strlen("http://");
//    char* url_end = strchr(url_start, '/');
//
//    if (url_start == NULL || url_end == NULL) {
//        perror("Error parsing URL");
//        close(client_socket);
//        return;
//    }
//
//    // Формируем запрос для сервера
//    char request[MAX_BUFFER_SIZE];
//    sprintf(request, "GET %s HTTP/1.1\r\nHost: %.*s\r\n\r\n", url_end, (int)(url_end - url_start), url_start);
//
//    // Создаем сокет для соединения с удаленным сервером
//    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
//    if (server_socket == -1) {
//        perror("Error creating server socket");
//        close(client_socket);
//        return;
//    }
//
//    // Настраиваем адрес удаленного сервера
//    struct sockaddr_in server_address;
//    memset(&server_address, 0, sizeof(server_address));
//    server_address.sin_family = AF_INET;
//    server_address.sin_port = htons(80);  // Используем порт 80 для HTTP
//    inet_pton(AF_INET, "ya.ru", &server_address.sin_addr);  // Укажите нужный IP-адрес
//
//    // Подключаемся к удаленному серверу
//    if (connect(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
//        perror("Error connecting to the server");
//        fprintf(stderr, "Connect error code: %d\n", errno);  // Print the error code
//        close(client_socket);
//        close(server_socket);
//        return;
//    }
//
//    // Пересылаем запрос на сервер
//    ssize_t bytes_sent = send(server_socket, request, strlen(request), 0);
//    if (bytes_sent == -1) {
//        perror("Error sending data to server");
//        close(client_socket);
//        close(server_socket);
//        return;
//    }
//
//    // Получаем ответ от сервера и пересылаем его клиенту
//    while ((bytes_received = recv(server_socket, buffer, sizeof(buffer), 0)) > 0) {
//        bytes_sent = send(client_socket, buffer, bytes_received, 0);
//        if (bytes_sent == -1) {
//            perror("Error sending data to client");
//            close(client_socket);
//            close(server_socket);
//            return;
//        }
//    }
//
//    // Закрываем сокеты
//    close(client_socket);
//    close(server_socket);
//}
//
//int main() {
//    int server_socket, client_socket;
//    struct sockaddr_in server_address, client_address;
//    socklen_t client_address_len = sizeof(client_address);
//
//    // Создаем сокет сервера
//    server_socket = socket(AF_INET, SOCK_STREAM, 0);
//    if (server_socket == -1) {
//        perror("Error creating server socket");
//        exit(EXIT_FAILURE);
//    }
//
//    // Настраиваем адрес сервера
//    memset(&server_address, 0, sizeof(server_address));
//    server_address.sin_family = AF_INET;
//    server_address.sin_addr.s_addr = INADDR_ANY;
//    server_address.sin_port = htons(PORT);
//
//    // Привязываем сокет к адресу и порту
//    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
//        perror("Error binding server socket");
//        exit(EXIT_FAILURE);
//    }
//
//    // Начинаем слушать входящие соединения
//    if (listen(server_socket, 5) == -1) {
//        perror("Error listening on server socket");
//        exit(EXIT_FAILURE);
//    }
//
//    printf("Proxy server is listening on port %d...\n", PORT);
//
//    while (1) {
//        // Принимаем входящее соединение
//        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
//        if (client_socket == -1) {
//            perror("Error accepting connection");
//            continue;
//        }
//
//        // Обрабатываем запрос от клиента
//        handle_client(client_socket);
//    }
//
//    // Закрываем сокет сервера (выход из цикла никогда не наступит)
//    close(server_socket);
//
//    return 0;
//}
