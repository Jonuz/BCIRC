CFLAGS = -Wall -ldl -std=gnu11 -pthread -Wl,--export-dynamic -g
CC = gcc

SOURCES = $(wildcard src/*.c) main.c
HEADERS = $(wildcard include/*.h)
OBJETCTS = $(*.o)

#TARGET_DIR

#PLUGIN_DIR
#PLUGINS_SRC
#PLUGINS_BIN

STATIC = $(PLUGINS_SRC)/libbcirc.a

default: irc

irc:
	$(CC) $(SOURCES) $(CFLAGS) -I. -o $@
	$(CC) $(SOURCES) $(CFLAGS) -fPIC -c

	ar rcs $(STATIC) *.o
	rm *.o

clean:
	-rm -f $(PLUGINS_BIN)/*.so
	-rm -f $(PLUGINS_DIR)/libbcirc.a
	-rm $(TARGET_DIR)/irc
