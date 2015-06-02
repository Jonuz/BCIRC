CFLAGS = -g -Wall -ldl -std=gnu11
CC = gcc

SOURCES = $(wildcard src/*.c) main.c
HEADERS = $(wildcard include/*.h)
OBJETCTS = $(*.o)

PLUGIN_SRC_PATH = plugins

STATIC = $(PLUGIN_SRC_PATH)/libbcirc.a

default: irc

irc:
	$(CC) $(SOURCES) $(CFLAGS) -I. -o $@
	$(CC) $(SOURCES) $(CFLAGS) -fPIC -c

	ar rcs ./plugins/libbcirc.a *.o
	rm *.o

clean:
	-rm -f build/plugins/*.so
	-rf $(PLUGIN_SRC_DIR)/libbcirc.a
	-rm irc