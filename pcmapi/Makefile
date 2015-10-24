CC=gcc
CFLAGS=-I.

all: test.o p_mmap.o
	$(CC) -static -o test test.o p_mmap.o -I.

clean:
	@rm -rf *.o test


