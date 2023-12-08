#include "proxy.c"

int main() {
    run_proxy();
}
// http://127.0.0.1/
// lsof -i -P -n | grep LISTEN
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <netdb.h>
//
//#define MAX_BUFFER_SIZE 4096
//
//void send_request(int sockfd, const char *hostname, int port) {
//    char request[MAX_BUFFER_SIZE];
//    snprintf(request, MAX_BUFFER_SIZE, "GET / HTTP/1.0\r\nHost: %s\r\n\r\n", hostname);
//    if (send(sockfd, request, strlen(request), 0) < 0) {
//        error("Error sending request");
//    }
//}
//
//void receive_response(int sockfd) {
//    char response[MAX_BUFFER_SIZE];
//    ssize_t bytes_received;
//    while ((bytes_received = recv(sockfd, response, MAX_BUFFER_SIZE - 1, 0)) > 0) {
//        response[bytes_received] = '\0';
//        printf("%s", response);
//    }
//
//    if (bytes_received < 0) {
//        error("Error receiving response");
//    }
//}
//
//int main(int argc, char *argv[]) {
//    if (argc < 3) {
//        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
//        exit(1);
//    }
//
//    char *hostname = argv[1];
//    int port = atoi(argv[2]);
//
//    struct hostent *server = gethostbyname(hostname);
//    if (server == NULL) {
//        fprintf(stderr, "Error: No such host\n");
//        exit(1);
//    }
//
//    // Create a socket
//    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//    if (sockfd < 0) {
//        error("Error opening socket");
//    }
//
//    struct sockaddr_in server_addr;
//    memset(&server_addr, 0, sizeof(server_addr));
//    server_addr.sin_family = AF_INET;
//    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
//    server_addr.sin_port = htons(port);
//
//    // Connect to the server
//    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
//        error("Error connecting");
//    }
//
//    // Send an HTTP GET request
//    send_request(sockfd, hostname, port);
//
//    // Receive and print the response
//    receive_response(sockfd);
//
//    // Close the socket
//    close(sockfd);
//
//    return 0;
//}

//size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
//    size_t realsize = size * nmemb;
//    printf("%.*s", (int)realsize, (char *)contents);
//    return realsize;
//}

//int main(void) {
//    CURL *curl;
//    CURLcode res;
//
//    curl_global_init(CURL_GLOBAL_DEFAULT);
//
//    curl = curl_easy_init();
//    if(curl) {
//        // Установка URL для запроса
//        curl_easy_setopt(curl, CURLOPT_URL, "https://ya.ru/?utm_referrer=https%3A%2F%2Fwww.google.com%2F");
//
//        // Установка колбэка для обработки полученных данных
//        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
//
//        // Выполнение HTTP-запроса
//        res = curl_easy_perform(curl);
//
//        // Проверка на ошибки
//        if(res != CURLE_OK)
//            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
//
//        // Очистка ресурсов
//        curl_easy_cleanup(curl);
//    }
//
//    curl_global_cleanup();
//
//    return 0;
//}