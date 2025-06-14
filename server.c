#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int clients[MAX_CLIENTS];
char client_names[MAX_CLIENTS][50];
int client_count = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(char *message, int sender_fd) {
    pthread_mutex_lock(&lock);
    for (int i = 0; i < client_count; ++i) {
        if (clients[i] != sender_fd) {
            send(clients[i], message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&lock);
}

void remove_client(int client_fd) {
    pthread_mutex_lock(&lock);
    for (int i = 0; i < client_count; ++i) {
        if (clients[i] == client_fd) {
            for (int j = i; j < client_count - 1; ++j) {
                clients[j] = clients[j + 1];
                strcpy(client_names[j], client_names[j + 1]);
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&lock);
}

void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    char name[50];
    char buffer[BUFFER_SIZE];

    // Receive client name
    recv(client_fd, name, sizeof(name), 0);

    pthread_mutex_lock(&lock);
    strcpy(client_names[client_count], name);
    clients[client_count++] = client_fd;
    printf("Client '%s' connected. Total clients: %d\n", name, client_count);
    pthread_mutex_unlock(&lock);

    while (1) {
        int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            printf("Client '%s' disconnected. Total clients: %d\n", name, client_count - 1);
            remove_client(client_fd);
            close(client_fd);
            break;
        }

        buffer[bytes_received] = '\0';

        char message[BUFFER_SIZE + 50];
        snprintf(message, sizeof(message), "[%s]: %s", name, buffer);
        broadcast_message(message, client_fd);
    }

    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, MAX_CLIENTS);

    printf("Server listening on port 8080...\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, &client_fd);
        pthread_detach(tid);
        usleep(10000); // to ensure client_fd is copied before overwritten
    }

    return 0;
}
