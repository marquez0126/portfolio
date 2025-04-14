#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#define MY_PORT_ID 6090

int errno;

int main()
{
   int sockid,nread,addrlen;
   struct sockaddr_in my_addr,client_addr;
   char msg[50];

   printf("Server: creating socket\n");
   if((sockid=socket(AF_INET,SOCK_DGRAM,0))<0)
   {
       printf("Server: socket error:%d\n",errno);
       perror("error string: ");
       exit(0);
   } 
  
   printf("Server: binding my local socket\n");
   bzero((char*)&my_addr,sizeof(my_addr));
   my_addr.sin_family=AF_INET;
   my_addr.sin_addr.s_addr=htons(INADDR_ANY);
   my_addr.sin_port=htons(MY_PORT_ID);

   if((bind(sockid,(struct sockaddr*)&my_addr,sizeof(my_addr)))<0)
   {
       printf("Server: bind fail:%d\n",errno);
       exit(0);
   }

   printf("Server: starting blocking message read\n");
    addrlen=sizeof(client_addr);
while(1){
   nread=recvfrom(sockid,msg,30,0,(struct sockaddr*)&client_addr,&addrlen);
   if(nread<0) printf("Server: return code from read is %d, error=%d,error reason=%s\n",nread,errno,strerror(errno));
   else printf("(127.0.0.1 , %d) said : %.15s\n",my_addr.sin_port,msg);
}
   close(sockid);
   exit(0);
}
