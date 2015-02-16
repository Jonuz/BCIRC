#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <string.h>
#include <time.h>

#include "server.h"

/*
    Plugin callbacks

*/


#define CHANNEL_JOINED "channel_joined"

/*
    Params:
        channel *
        user *

        Callback to call after someone joined to channel.
*/


#define CHANNEL_PART "channel_part"
/*
    Params:
        channel *
        char *reason

        Callback to call after someone parted from channel.

*/


#define CHANNEL_KICKED
/*
    Params:
        channel *
        char *reason

        Callback to call after someone kicked from channel.

*/

typedef struct
{
	char *name;
	char *key;
	char *modes;

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
