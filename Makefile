TARGET = irc
CFLAGS = -g -Wall -ldl -std=gnu11
CC = gcc

SOURCES = $(wildcard src/*.c) main.c
HEADERS = $(wildcard headers/*.h)
OBJCETS = $(wilcard objs/*.o)

default: $(TARGET)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -fPIC $< -o $@

$(TARGET):
	$(CC) $(SOURCES) $(CFLAGS) -o $@
	$(CC) $(SOURCES) $(CFLAGS) -fPIC -c


	ar rcs ./plugins/libbcirc.a *.o
	rm *.o
