all: test

test.o: test.c mini_filesystem.h 
	gcc -c test.c -o test.o

mini_filesystem.o: mini_filesystem.c mini_filesystem.h
	gcc -c mini_filesystem.c -o mini_filesystem.o

	

test: test.o mini_filesystem.o
	gcc test.o mini_filesystem.o -o test
	

clean: 
	rm -f *.o
	rm -f test test.o mini_filesystem.o
