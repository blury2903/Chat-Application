.PHONY := clean all

all:
	gcc -o chat main.c

clean:
	rm -rf chat

