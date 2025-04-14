#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#define MY_PORT_ID 6089
#define SERVER_PORT_ID 6090
#define SERV_HOST_ADDR "127.0.0.1"

int main()
{
    int sockid,retcode;
    struct sockaddr_in my_addr,server_addr;
    char msg[15];

    printf("Client: creating socket\n");
    if((sockid=socket(AF_INET,SOCK_DGRAM,0))<0)
    {
        printf("Clientr: socket error:%d\n",errno);
        perror("error string: ");
        exit(0);
    } 

    printf("Client: binding my local socket\n");
    bzero((char*)&my_addr,sizeof(my_addr));
    my_addr.sin_family=AF_INET;
    my_addr.sin_addr.s_addr=htons(INADDR_ANY);
    my_addr.sin_port=htons(MY_PORT_ID);

    if((bind(sockid,(struct sockaddr*)&my_addr,sizeof(my_addr)))<0)
    {
        printf("Client: bind fail:%d\n",errno);
        exit(0);
    }

    printf("Client: creating addr structure for server\n");
    bzero((char*)&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=inet_addr(SERV_HOST_ADDR);
    server_addr.sin_port=htons(SERVER_PORT_ID);

    printf("Client: initializing message and sending\n");
    
	char inputMeg[100];
	while(1){
	printf("input:");
	fgets(inputMeg,sizeof(inputMeg),stdin);
	inputMeg[strcspn(inputMeg,"\n")]='\0';
	strcpy(msg, inputMeg);
	//sprintf(msg,inputMeg);
    //printf("\n***%s***\n",msg);
    retcode=sendto(sockid,msg,sizeof(msg),0,(struct sockaddr*)&server_addr,sizeof(server_addr));

    if(retcode<=-1)
    {
        printf("client:sendto faild:%d\n",errno);
        exit(0);
    }
    }
    close(sockid);
    return 0;
}
