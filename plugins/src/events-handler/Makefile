CC = gcc
CFLAGS = -fPIC -shared -std=gnu11 ../libbcirc.a -g -std=gnu11
SOURCES = $(wildcard *.c)
TARGET = $(BCIRC_PLUGINS_BIN)/events-misc.so

$(TARGET):
	$(CC) $(SOURCES) $(CFLAGS) -o $@

clean:
	rm -rf ${TARGET}
