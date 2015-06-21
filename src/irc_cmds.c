#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/irc_cmds.h"

int privmsg(char *msg, char *target, server *srv)
{
	//PRIVMSG target :msg
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&mutex);
	usleep(75);
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
	usleep(75);
    char buf[1024];
    if (chan_pass == NULL)
    {
        sprintf(buf, "JOIN %s\r\n", chan_name);
    }
    else
    {
        sprintf(buf, "JOIN %s; %s\r\n", chan_name, chan_pass);
    }

	pthread_mutex_unlock(&mutex);
    return server_send(buf, srv);
}


int part_channel(char *reason, channel *chan)
{
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&mutex);
	usleep(75);
    char *buf = "PART %s\r\n";
    buf = malloc((strlen(buf) + strlen(chan->name)) * sizeof(char));
    sprintf(buf, buf, chan->name);
	free(buf);
	pthread_mutex_unlock(&mutex);

    return server_send(buf, chan->srv);
}
