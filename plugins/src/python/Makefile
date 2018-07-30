CC = gcc
CFLAGS = -fPIC -shared -std=gnu11 -g $(shell python3-config --cflags) $(shell python3-config --ldflags) ../libbcirc.a -Wstrict-prototypes
#CFLAGS = -fPIC -shared -std=gnu11 -g -I/usr/include/python3.6 -lpython3.6 ../libbcirc.a -Wstrict-prototypes

SOURCES = $(wildcard *.c)
TARGET = $(BCIRC_PLUGINS_BIN)/python.so

$(TARGET):
	$(CC) $(SOURCES) $(CFLAGS) -o $@
	@echo $CFLAGS

clean:
	rm -rf ${TARGET}
