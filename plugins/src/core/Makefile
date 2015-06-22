CC = gcc
CFLAGS = -fPIC -shared -std=gnu11 ../libbcirc.a -g
SOURCES = $(wildcard *.c)
TARGET = $(PLUGINS_BIN)/core.so

$(TARGET):
	$(CC) $(SOURCES) $(CFLAGS) -o $@

clean:
	rm -rf ${TARGET}
