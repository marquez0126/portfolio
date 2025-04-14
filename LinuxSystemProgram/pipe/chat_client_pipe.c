#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#define SERVER_FIFO_NAME "/tmp/serv_fifo"
#define CLIENT_FIFO_NAME "/tmp/cli_%d_fifo"
#define BUFFER_SIZE 1024

struct data_to_pass_st {
    pid_t client_pid;
    char some_data[BUFFER_SIZE - 1];
};

int main() {
    int server_fifo_fd, client_fifo_fd;
    struct data_to_pass_st my_data;
    char client_fifo[256];
    int read_res;

    printf("--- Input 'q' to exit ---\n");

    server_fifo_fd = open(SERVER_FIFO_NAME, O_WRONLY);
    if (server_fifo_fd == -1) {
        fprintf(stderr, "Sorry, no server\n");
        exit(EXIT_FAILURE);
    }

    my_data.client_pid = getpid();
    sprintf(client_fifo, CLIENT_FIFO_NAME, my_data.client_pid);
    if (mkfifo(client_fifo, 0777) == -1) {
        fprintf(stderr, "Sorry, can't make %s\n", client_fifo);
        exit(EXIT_FAILURE);
    }

    if (fork() == 0) {
        while (1) {
            client_fifo_fd = open(client_fifo, O_RDONLY);
            if (client_fifo_fd != -1) {
                read_res = read(client_fifo_fd, &my_data, sizeof(my_data));
                if (read_res > 0) {
                    my_data.some_data[strcspn(my_data.some_data, "\n")] = '\0';
                    if (strcmp(my_data.some_data, "q") == 0) {
                        printf("Server has left the chat...\n");
                        break;
                    }
                    printf("Received from pid[%d]: %s\n", my_data.client_pid, my_data.some_data);
                }
                close(client_fifo_fd);
            }
        }
        unlink(client_fifo);
        exit(EXIT_SUCCESS);
    }

    while (1) {
        printf("Enter message: ");
        fgets(my_data.some_data, sizeof(my_data.some_data), stdin);
        my_data.some_data[strcspn(my_data.some_data, "\n")] = '\0';

        write(server_fifo_fd, &my_data, sizeof(my_data));

        if (strcmp(my_data.some_data, "q") == 0) {
            printf("Client has left the chat...\n");
            break;
        }
    }

    close(server_fifo_fd);
    unlink(client_fifo);
    exit(EXIT_SUCCESS);
}

