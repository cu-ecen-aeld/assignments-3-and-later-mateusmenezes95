CC ?= gcc
AR ?= ar
CFLAGS += -Wall -Werror -g
LDFLAGS ?=
INCLUDES := -I include


vpath %.c src
vpath %.h include

.PHONY: all clean


all: aesdsocket

aesdsocket: main.c libaesdserver.a libbecomedaemon.a
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $^

libaesdserver.a: server.o
	$(AR) rcs $@ $<

server.o: server.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

libbecomedaemon.a: become_daemon.o
	$(AR) rcs $@ $<

become_daemon.o: become_daemon.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f aesdsocket server.o libaesdserver.a libbecomedaemon.a become_daemon.o

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