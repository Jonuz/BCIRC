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

	struct user **users;
	size_t user_count;

} channel;


int join_channel(char *name, server *srv);
int part_channel(char *name, char *reason, server *srv);

int kick(char *name, char *reason, channel *chan);


#endif
