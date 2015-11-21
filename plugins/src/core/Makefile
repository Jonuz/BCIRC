CC = gcc
CFLAGS = -fPIC -shared -std=gnu11 ../libbcirc.a -g `pkg-config --cflags libconfig` `pkg-config --libs libconfig` -pthread
SOURCES = $(wildcard *.c)
TARGET = $(BCIRC_PLUGINS_BIN)/core.so

$(TARGET):
	$(CC) $(SOURCES) $(CFLAGS) -o $@

clean:
	rm -rf ${TARGET}
