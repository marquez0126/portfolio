#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(void) {
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address, client_address;
    fd_set readfds, testfds;
    int fd, result;
    char buffer[BUFFER_SIZE];

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(9734);
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, 5);

    FD_ZERO(&readfds);
    FD_SET(server_sockfd, &readfds);

    printf("Server started on port %d\n", 9734);

    while (1) {
        testfds = readfds;
        printf("Waiting for connections...\n");
        result = select(FD_SETSIZE, &testfds, NULL, NULL, NULL);

        if (result < 1) {
            perror("select failed");
            exit(1);
        }

        for (fd = 0; fd < FD_SETSIZE; fd++) {
            if (FD_ISSET(fd, &testfds)) {
                if (fd == server_sockfd) {
                    client_len = sizeof(client_address);
                    client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
                    FD_SET(client_sockfd, &readfds);
                    printf("Client connected on fd %d\n", client_sockfd);
                } else {
                    int nread = read(fd, buffer, sizeof(buffer) - 1);
                    if (nread <= 0) {
                        close(fd);
                        FD_CLR(fd, &readfds);
                        printf("Client disconnected on fd %d\n", fd);
                    } else {
                        buffer[nread] = '\0';
                        printf("Message from fd %d: %s", fd, buffer);
                        for (int i = 0; i < FD_SETSIZE; i++) {
                            if (FD_ISSET(i, &readfds) && i != server_sockfd && i != fd) {
                                write(i, buffer, nread);
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

