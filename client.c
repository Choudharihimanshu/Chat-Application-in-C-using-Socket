// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024
#define PORT 8080

int sock;

void *receive_handler(void *arg) {
    char buffer[BUF_SIZE];
    int len;

    while ((len = recv(sock, buffer, BUF_SIZE, 0)) > 0) {
        buffer[len] = '\0';
        printf("%s", buffer);
    }

    return NULL;
}

int main() {
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    pthread_t recv_thread;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket error");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        return 1;
    }

    printf("Connected to chat server.\n");

    pthread_create(&recv_thread, NULL, receive_handler, NULL);

    while (1) {
        fgets(buffer, BUF_SIZE, stdin);
        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
    return 0;
}
