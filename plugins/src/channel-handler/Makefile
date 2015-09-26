CC = gcc
CFLAGS = -fPIC -shared -std=gnu11 ../libbcirc.a -g
SOURCES = $(wildcard *.c)
TARGET = $(BCIRC_PLUGINS_BIN)/channel.so

$(TARGET):
	$(CC) $(BCIRC_SOURCES) $(CFLAGS) -o $@

clean:
	rm -rf ${TARGET}
