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

	int topic_created_time;
	char *topic_creator;

	char *users;

    server *srv;
} channel;

extern channel **channel_list;
extern int channel_count;

channel *get_channel(char *chan_name, server *srv);

int remove_channel(channel *channel_ptr); //Removes channel, return new channel count.
channel *create_channel(char *chan_name, server *srv); //Creates new channel, adds it to channel list and returns new channel.

#endif
