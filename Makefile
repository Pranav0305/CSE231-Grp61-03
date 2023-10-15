all:
	gcc -o main main.c
	gcc -o hello helloworld.c
	gcc -o fib ib.c

clean:
	-@rm -f fib hello main