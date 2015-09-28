#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <string.h>
#include <time.h>
#include <pthread.h>

struct server;

typedef struct channel
{
	char *name;
	char *key;
	char *modes;

	char *topic;

	int topic_created_time;
	char *topic_creator;

	char *users;

	pthread_mutex_t mutex;

    struct server *srv;
} channel;

extern channel **channel_list;
extern int channel_count;

channel *get_channel(char *chan_name, struct server *srv);

int remove_channel(channel *channel_ptr); //Removes channel, return new channel count.
channel *create_channel(char *chan_name, struct server *srv); //Creates new channel, adds it to channel list and returns new channel.

extern pthread_mutex_t channel_global_mutex;

#endif
