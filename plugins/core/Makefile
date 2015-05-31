CC = gcc
CFLAGS = -fPIC -shared -std=gnu11 ../libbcirc.a -g
SOURCES = $(wildcard *.c)
PLUGIN_PATH = ../../build/plugins
TARGET = $(PLUGIN_PATH)/core.so

$(TARGET):
	$(CC) $(SOURCES) $(CFLAGS) -o $@

