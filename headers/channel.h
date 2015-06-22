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

channel *get_channel(char *chan_name);

int remove_channel(channel *channel_ptr); //Removes channel, return new channel count.
channel *create_channel(char *chan_name, server *srv); //Creates new channel, adds it to channel list and returns new channel.

#endif
