#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/irc_cmds.h"

int privmsg(char *msg, char *target, server *srv)
{
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&mutex);
	char *buf = malloc(( 7 + 1 + strlen(target) + 2 + strlen(msg) + 2 + 1) * sizeof(char));
	sprintf(buf,"PRIVMSG %s :%s\r\n", target, msg);
	int res = server_send(buf, srv);
	free(buf);
	pthread_mutex_unlock(&mutex);

	return res;
}

int join_channel(char *chan_name, char *chan_pass, server *srv)
{
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&mutex);
	char *buf = NULL;
	if (chan_pass == NULL)
	{
		buf = malloc((4 + 1 + strlen(chan_name) + 2 + 1) * sizeof(char));
		sprintf(buf, "JOIN %s\r\n", chan_name);
	}
	else
	{
		buf = malloc((4 + 1 + strlen(chan_name) + 2 + strlen(chan_pass) + 2 + 1) * sizeof(char));
		sprintf(buf, "JOIN %s; %s\r\n", chan_name, chan_pass);
	}

	sprintf(buf, buf, chan_name);

	int len = strlen(buf);
	char buf_copy[len];
	strcpy(buf_copy, buf);
	free(buf);

	pthread_mutex_unlock(&mutex);
	return server_send(buf_copy, srv);
}


int part_channel(char *reason, channel *chan)
{
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&mutex);
	char *buf = "PART %s\r\n";
	buf = malloc((strlen(buf) + strlen(chan->name)) * sizeof(char));
	sprintf(buf, buf, chan->name);

	int len = strlen(buf);
	char buf_copy[len];
	strcpy(buf_copy, buf);
	free(buf);

	pthread_mutex_unlock(&mutex);

	return server_send(buf_copy, chan->srv);
}
