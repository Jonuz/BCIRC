CC = gcc
CFLAGS = -fPIC -shared -std=gnu11 ../libbcirc.a  -g -ldl -lcurl -lm
SOURCES = $(wildcard *.c) cJSON/cJSON.c

TARGET = $(BCIRC_PLUGINS_BIN)/weather.so

$(TARGET):
	$(CC) $(SOURCES) $(CFLAGS) -o $@

clean:
	rm -f $(BCIRC_PLUGINS_BIN)/weather.so
