CC = gcc
CFLAGS = -fPIC -shared -std=gnu11 ../libbcirc.a -g -O1
SOURCES = $(wildcard *.c)
TARGET = $(PLUGINS_BIN)/core.so

$(TARGET):
	$(CC) $(SOURCES) $(CFLAGS) -o $@

clean:
	rm -rf ${TARGET}
