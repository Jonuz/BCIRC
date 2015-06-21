#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <string.h>
#include <time.h>

#include "server.h"




typedef struct
{
	char *name;
	char *key;
	char *modes;
	char *topic;

    server *srv;
	struct user **users;

    void *next_channel;
} channel;

extern channel **channel_list;

channel *get_channels();
channel *get_channel(char *chan_name);

int remove_channel(char *chan_name);
channel *create_channel(char *chan_name, server *srv);

#endif
