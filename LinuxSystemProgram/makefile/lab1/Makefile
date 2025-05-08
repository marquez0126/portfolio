hello:hello.o hello_func.o
	gcc -o hello hello.o hello_func.o
hello.o:hello.c hello_api.h
	gcc -c hello.c
hello_func.o:hello_func.c hello_api.h
	gcc -c hello_func.c
