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
	int res = server_send(srv, buf);
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
		sprintf(buf, "JOIN %s %s\r\n", chan_name, chan_pass);
	}

	sprintf(buf, buf, chan_name);

	int len = strlen(buf);
	char buf_copy[len];
	strcpy(buf_copy, buf);
	free(buf);

	return server_send(srv, buf_copy);
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

	return server_send((server*) chan->srv, buf_copy);
}


int nick(char *nick, server *srv)
{
	#define BUF_BASE "NICK %s\r\n"
	char *buf = NULL;
	buf = malloc(( strlen(BUF_BASE) + 1 + strlen(nick) + 3) * sizeof(char));

	sprintf(buf, BUF_BASE, nick);
	//printf("buf: %s", buf);

	srv->nick = malloc((strlen(nick)+1) * sizeof(char));
	strcpy(srv->nick, nick);

	int res = server_send(srv, buf);
	free(buf);

	return res;
}

int quit(char *reason, server *srv)
{
	#define BUF_QUIT_BASE "QUIT :%s\r\n"
	char *buf = NULL;
	buf = malloc(( strlen(BUF_QUIT_BASE) + 1 + strlen(reason) + 3) * sizeof(char));
	sprintf(buf, BUF_QUIT_BASE, reason);

	int res = server_send(srv, buf);
	free(buf);

	return res;
}
