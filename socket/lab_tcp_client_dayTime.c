#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define RCVBUFSIZE 128

void DieWithError(char *errorMessage){
	perror(errorMessage);
	exit(1);
}
int main(int argc, char** argv){
	int sock;
	struct sockaddr_in echoServAddr;
	unsigned short echoServPort=20000;
	char *servIP ="127.0.0.1";
	char *echoString="Hi Server"; 
	char echoBuffer[RCVBUFSIZE];
	unsigned int echoStringLen;
	int bytesRcvd, totalBytesRcvd;
	
	if((sock=socket(PF_INET,SOCK_STREAM,0))<0){
		DieWithError("socket() failed");
	}

	memset(&echoServAddr,0,sizeof(echoServAddr));
	echoServAddr.sin_family=AF_INET;
	echoServAddr.sin_addr.s_addr=inet_addr(servIP);
	echoServAddr.sin_port=htons(echoServPort);
	
	if(connect(sock,(struct sockaddr *)&echoServAddr,sizeof(echoServAddr))<0){
		DieWithError("connect() failed");
	}
	echoStringLen=strlen(echoString);

	if(send(sock,echoString,echoStringLen,0)!=echoStringLen){
		DieWithError("send() sent a different number of bytes than expected");
	}
	totalBytesRcvd=0;
	printf("Reciver:");

	while(totalBytesRcvd<echoStringLen){
		if((bytesRcvd=recv(sock,echoBuffer,RCVBUFSIZE-1,0))<=0){
			DieWithError("recv() failed or connection closed premanturely");
		}
		totalBytesRcvd+=bytesRcvd;
		echoBuffer[bytesRcvd]='\0';
		printf("%s",echoBuffer);
	}
	printf("\n");
	close(sock);
	exit(0);

}
