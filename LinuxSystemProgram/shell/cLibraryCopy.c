#include <stdio.h>
#include <stdlib.h>

int main(int argc,char *argv[]){
	int ch;
	FILE *in,*out;
	if((in=fopen(argv[1],"r"))==NULL){
		if((in=fopen("./file.in","r"))==NULL){
			printf("File(file.in) not found\n");
			return 1;
		}
	}

	out=fopen("file.out","w");
	while((ch=getc(in))!=EOF){
		putc(ch,out);
	}
	fclose(in);
	fclose(out);
	return 0;
}
