#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
	int ch,count=0;
	int in;
	
	if(isatty(fileno(stdin))){
		in=open("file.in",O_RDONLY);
		if(in==-1){
			printf("Input error\n");
			exit(1);
		};
	}else{
		in=fileno(stdin);
	};
	while(read(in,&ch,1)==1){
		count++;
	};
	if(in!=fileno(stdin)){
		printf("File(file.in) has %d characters\n",count);
		close(in);
	}else{	
		printf("File(stdin) has %d characters\n",count);
	};
	return 0;
}
