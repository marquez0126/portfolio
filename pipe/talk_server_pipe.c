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
    int read_res;
    char client_fifo[256];
    char *tmp_char_ptr;

	printf("---Input 'q' to exit---\n");
	printf("waiting for client response...\n");
    mkfifo(SERVER_FIFO_NAME, 0777);
    server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY);
    if (server_fifo_fd == -1) {
        fprintf(stderr, "Server fifo failure\n");
        exit(EXIT_FAILURE);
    }
    do {
        read_res = read(server_fifo_fd, &my_data, sizeof(my_data));
		if(strcmp(my_data.some_data,"q")==0){
    		close(server_fifo_fd);
    		unlink(SERVER_FIFO_NAME);
			printf("Client has left the chat...\n");
    		exit(EXIT_SUCCESS);	
		}
        if (read_res > 0) {
            sprintf(client_fifo, CLIENT_FIFO_NAME, my_data.client_pid);
            client_fifo_fd = open(client_fifo, O_WRONLY);
            if (client_fifo_fd != -1) {
             	printf("received:%s\n",my_data.some_data);
				printf("Input messages:\n");
				fgets(my_data.some_data,sizeof(my_data.some_data),stdin);
				my_data.some_data[strcspn(my_data.some_data,"\n")] = '\0';
                write(client_fifo_fd, &my_data, sizeof(my_data));
				close(client_fifo_fd);
				printf("waiting for client response...\n");
            }
        }
    } while (strcmp(my_data.some_data,"q")!=0);
    close(server_fifo_fd);
    unlink(SERVER_FIFO_NAME);
	printf("Server has left the chat...\n");
    exit(EXIT_SUCCESS);
}

