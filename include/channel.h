#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <string.h>
#include <time.h>

#include "server.h"

typedef struct
{
	char *name;
	char *pass;
	char *modes;


    server *srv;

	struct user **users;
	size_t user_count;

} channel;

extern channel **channel_list;
extern int channel_count;

channel *get_channels();
channel *get_channel(char *chan_name);

int remove_channel(channel *chan);
channel *create_channel(char *chan_name, server *srv);

#endif
