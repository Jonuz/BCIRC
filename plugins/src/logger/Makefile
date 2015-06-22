CC = gcc
CFLAGS = -fPIC -shared -std=gnu11 ../libbcirc.a  -g
SOURCES = $(wildcard *.c)
TARGET = $(PLUGINS_BIN)/logger.so

$(TARGET):
	$(CC) $(SOURCES) $(CFLAGS) -o $@

clean:
	rm -f $(PLUGIN_PATH)/logger.so
