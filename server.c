// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUF_SIZE 1024
#define PORT 8080

int clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(char *message, int sender_fd) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] != 0 && clients[i] != sender_fd) {
            send(clients[i], message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUF_SIZE];
    int read_size;

    while ((read_size = recv(client_socket, buffer, BUF_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        broadcast_message(buffer, client_socket);
    }

    // Remove client
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] == client_socket) {
            clients[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    close(client_socket);
    free(arg);
    pthread_exit(NULL);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, MAX_CLIENTS);
    printf("Server listening on port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);

        pthread_mutex_lock(&clients_mutex);
        int i;
        for (i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i] == 0) {
                clients[i] = client_socket;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        if (i == MAX_CLIENTS) {
            printf("Max clients reached. Connection rejected.\n");
            close(client_socket);
        } else {
            pthread_t tid;
            int *new_sock = malloc(sizeof(int));
            *new_sock = client_socket;
            pthread_create(&tid, NULL, handle_client, new_sock);
        }
    }

    return 0;
}
