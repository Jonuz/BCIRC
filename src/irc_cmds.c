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
	int res = server_send(buf, srv);
	free(buf);

	return res;
}


int part_channel(char *reason, channel *chan)
{
	#define BUF_BASE "PART %s\r\n"
	char *buf = NULL;
	buf = malloc((strlen(BUF_BASE) + 1 + strlen(chan->name) + 3) * sizeof(char));
	sprintf(buf, BUF_BASE, chan->name);

	int res = server_send(buf, (server*) chan->srv);
	free(buf);

	return res;
}

int nick(char *nick, server *srv)
{
	#define BUF_BASE "NICK %s\r\n"
	char *buf = NULL;
	buf = malloc(( strlen(BUF_BASE) + 1 + strlen(nick) + 3) * sizeof(char));
	sprintf(buf, BUF_BASE, nick);

	if (srv->last_tried_nick)
		free(srv->last_tried_nick);

	srv->last_tried_nick = malloc((strlen(nick) + 1) * sizeof(char));
	strcpy(srv->last_tried_nick, nick);

	int res = (buf, srv);
	free(buf);

	return res;
}
