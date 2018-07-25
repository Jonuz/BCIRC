CFLAGS = -Wall -ldl -std=gnu11 -pthread -Wl,--export-dynamic -g `pkg-config --cflags libconfig` `pkg-config --libs libconfig`

CC = gcc

SOURCES = $(wildcard src/*.c) main.c
HEADERS = $(wildcard include/*.h)


STATIC = $(BCIRC_PLUGINS_SRC)/libbcirc.a

default: irc

irc:
	$(CC) $(SOURCES) $(CFLAGS) $(shell python3-config --ldflags) -o $@ 
	$(CC) $(SOURCES) $(CFLAGS)  -fPIC -c

	ar rcs $(STATIC) *.o
	rm *.o

clean:
	-rm -f $(BCIRC_PLUGINS_BIN)/*.so
	-rm -f $(BCIRC_PLUGINS_DIR)/libbcirc.a
	-rm $(BCIRC_TARGET_DIR)/irc
	-rm *.out
