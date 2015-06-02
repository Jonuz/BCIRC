CC = gcc
CFLAGS = -fPIC -shared -std=gnu11 ../libbcirc.a -g -lcurl
SOURCES = $(wildcard *.c)
PLUGIN_PATH = ../../build/plugins
TARGET = $(PLUGIN_PATH)/title.so

$(TARGET):
	$(CC) $(SOURCES) $(CFLAGS) -o $@

clean:
	rm -f $(TARGET)
