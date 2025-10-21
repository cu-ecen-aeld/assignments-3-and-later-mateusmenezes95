CC := gcc
CFLAGS := -Wall -Werror -g -I include


vpath %.c src
vpath %.h include

.PHONY: all clean


all: aesdsocket

aesdsocket: main.c libaesdserver.a libdaemon.a
	$(CC) $(CFLAGS) -o $@ $^

libaesdserver.a: server.o
	ar rcs $@ $<

server.o: server.c
	$(CC) $(CFLAGS) -c -o $@ $<

libdaemon.a: become_daemon.o
	ar rcs $@ $<

become_daemon.o: become_daemon.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f aesdsocket server.o libaesdserver.a l

# Automatic variables:
# $@ The filename representing the target.
# $% The filename element of an archive member specification.
# $< The filename of the first prerequisite.
# $? The names of all prerequisites that are newer than the target, separated by spaces.
# $^ The filenames of all the prerequisites, separated by spaces. This list has dupli-
# cate filenames removed since for most uses, such as compiling, copying, etc.,
# duplicates are not wanted.
# $+ Similar to $^ except that duplicates are not removed.
# $* The stem with which an implicit rule matches. For example, if the target is foo.o
# and the implicit rule is for %.o, then the stem is foo.