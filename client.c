#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int sockfd;

void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            printf("Disconnected from server.\n");
            exit(1);
        }
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
        fflush(stdout);
    }
    return NULL;
}

int main() {
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char name[50];

    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0'; // remove newline

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Send name to server
    send(sockfd, name, strlen(name), 0);

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, NULL);

    while (1) {
        fgets(buffer, sizeof(buffer), stdin);
        send(sockfd, buffer, strlen(buffer), 0);
    }

    return 0;
}
