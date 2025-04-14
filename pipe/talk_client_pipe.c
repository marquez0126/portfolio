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
#define BUFFER_SIZE 20

struct data_to_pass_st {
	pid_t client_pid;
	char some_data[BUFFER_SIZE - 1];
};

int main()
{
    int server_fifo_fd, client_fifo_fd;
    struct data_to_pass_st my_data;
    char client_fifo[256];
	printf("---Input 'q' to exit---\n");
	do{
	printf("Input message to server:\n");
	fgets(my_data.some_data,sizeof(my_data.some_data),stdin);
	my_data.some_data[strcspn(my_data.some_data, "\n")] = '\0';
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

	write(server_fifo_fd, &my_data, sizeof(my_data));
	if(strcmp(my_data.some_data,"q") == 0){
		printf("Client has left the chat...\n");
		exit (EXIT_SUCCESS);
	}
	client_fifo_fd = open(client_fifo, O_RDONLY);
	printf("waiting for server response...\n");
	if (client_fifo_fd != -1) {
		if (read(client_fifo_fd, &my_data, sizeof(my_data)) > 0) {
	my_data.some_data[strcspn(my_data.some_data, "\n")] = '\0';
	if(strcmp(my_data.some_data,"q") == 0){
		printf("Server has left the chat...\n");
		exit (EXIT_SUCCESS);
	}
                printf("received:%s\n", my_data.some_data);
		}
        close(client_fifo_fd);
    	unlink(client_fifo);
	}
    }while (1);
    close(server_fifo_fd);
    exit(EXIT_SUCCESS);
}
