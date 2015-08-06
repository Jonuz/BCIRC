CC = gcc
CFLAGS = -fPIC -shared -std=gnu11 ../libbcirc.a -g -lcurl -g
SOURCES = $(wildcard *.c)
TARGET = $(PLUGINS_BIN)/title.so

$(TARGET):
	$(CC) $(SOURCES) $(CFLAGS) -o $@

clean:
	rm -f $(TARGET)
