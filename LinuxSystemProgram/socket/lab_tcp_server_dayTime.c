#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>

#define SERVER_PORT 20000
#define LENGTH_OF_LISTEN_QUEUE 10
#define BUFFER_SIZE 255
#define WELCOME_MESSAGE "Welcome to connect the server\n"

int main(int argc, char **argv) {
    int servfd, clifd;
    struct sockaddr_in servaddr, cliaddr;

    if ((servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Create socket error\n");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);

    if (bind(servfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        printf("Bind error");
        exit(1);
    }

    if (listen(servfd, LENGTH_OF_LISTEN_QUEUE) < 0) {
        printf("Listen error");
        exit(1);
    }

    while (1) {
		char buf[BUFFER_SIZE];
		long timestamp;
		
		socklen_t length=sizeof(cliaddr);
        clifd = accept(servfd, (struct sockaddr *)&cliaddr, &length);
        if (clifd < 0) {
            perror("Accept error");
            break;
        }
		strcpy(buf,WELCOME_MESSAGE);
        printf("From client, IP: %s, Port: %d\n",
               inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
		timestamp=time(NULL);
		strcat(buf,"timestamp in server:");
		strcat(buf,ctime(&timestamp));
        send(clifd, buf, BUFFER_SIZE, 0);
    	close(clifd);
    }
    close(servfd);
    return 0;
}
