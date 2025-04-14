#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(void){
	pid_t pid;
	pid=fork();
	if(pid==0){
		int pidsum=0;
		for(int i=0;i<=10;i++){
			pidsum+=i;
		}
		exit(pidsum);
	}else{
		int stat;
		wait(&stat);
		if(WIFEXITED(stat)){
			int sum=WEXITSTATUS(stat);
			printf("%d\n",sum);	
		}
	}
	return 0;
}
