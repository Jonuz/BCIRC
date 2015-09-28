#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../headers/server.h"
#include "../headers/channel.h"
#include "../headers/irc_cmds.h"

int privmsg(char *msg, char *target, server *srv)
{
	char *buf = malloc(( 7 + 1 + strlen(target) + 2 + strlen(msg) + 2 + 1) * sizeof(char));
	sprintf(buf,"PRIVMSG %s :%s\r\n", target, msg);
	int res = server_send(buf, srv);
	free(buf);

	return res;
}

int join_channel(char *chan_name, char *chan_pass, server *srv)
{
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

	return server_send(buf_copy, srv);
}


int part_channel(char *reason, channel *chan)
{
	char *buf = "PART %s\r\n";
	buf = malloc((strlen(buf) + strlen(chan->name) + 1) * sizeof(char));
	sprintf(buf, buf, chan->name);

	int len = strlen(buf);
	char buf_copy[len];
	strcpy(buf_copy, buf);
	free(buf);

	return server_send(buf_copy, (server*) chan->srv);
}
