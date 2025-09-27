CC := gcc
CFLAGS := -Wall -Werror -g

all: aesdsocket

aesdsocket: aesdsocket.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f aesdsocket aesdsocket.o
