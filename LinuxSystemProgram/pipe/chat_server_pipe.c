#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>

#define SERVER_FIFO_NAME "/tmp/serv_fifo"
#define CLIENT_FIFO_NAME "/tmp/cli_%d_fifo"
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

struct data_to_pass_st {
    pid_t client_pid;
    char some_data[BUFFER_SIZE - 1];
};

pid_t clients[MAX_CLIENTS];
int client_count = 0;     

void add_client(pid_t pid) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == pid) return; 
    }
    if (client_count < MAX_CLIENTS) {
        clients[client_count++] = pid;
    }
}

void remove_client(pid_t pid) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == pid) {
            clients[i] = clients[--client_count]; 
            return;
        }
    }
}

void broadcast_message(struct data_to_pass_st *msg) {
    char client_fifo[256];
    int client_fifo_fd;

    for (int i = 0; i < client_count; i++) {
        if (clients[i] != msg->client_pid) {  
            sprintf(client_fifo, CLIENT_FIFO_NAME, clients[i]);
            client_fifo_fd = open(client_fifo, O_WRONLY);
            if (client_fifo_fd != -1) {
                write(client_fifo_fd, msg, sizeof(*msg));
                close(client_fifo_fd);
            }
        }
    }
}

int main() {
    int server_fifo_fd;
    struct data_to_pass_st my_data;
    int read_res;

    printf("--- Server Started ---\n");

    mkfifo(SERVER_FIFO_NAME, 0777);
    server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY);
    if (server_fifo_fd == -1) {
        fprintf(stderr, "Server fifo failure\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        read_res = read(server_fifo_fd, &my_data, sizeof(my_data));
        if (read_res > 0) {
            printf("Received from pid[%d]: %s\n", my_data.client_pid, my_data.some_data);

            if (strcmp(my_data.some_data, "q") == 0) {
                remove_client(my_data.client_pid);
                printf("Client [%d] has left.\n", my_data.client_pid);
                continue;
            }

            add_client(my_data.client_pid);  
            broadcast_message(&my_data);     
        }
    }

    close(server_fifo_fd);
    unlink(SERVER_FIFO_NAME);
    printf("Server has shut down.\n");
    exit(EXIT_SUCCESS);
}

