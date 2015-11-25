CC = gcc
CFLAGS = -fPIC -shared -std=gnu11 ../libbcirc.a -g  -pthread
SOURCES = $(wildcard *.c)
TARGET = $(BCIRC_PLUGINS_BIN)/autoload.so

$(TARGET):
	$(CC) $(SOURCES) $(CFLAGS) -o $@

clean:
	rm -rf ${TARGET}
