#include <stdio.h>

#include "../headers/log.h"
#include "../headers/irc.h"
#include "../headers/server.h"
#include "../headers/channel.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"

char plugin_version[] = "0.01";
char plugin_author[] = "Joona";
char plugin_name[] = "Logger";


int on_server_send(void **paramams, int argc);
int on_connect(void **params, int argc);

int on_privmsg(void **params, int argc);

int on_recv(void **params, int argc);
int on_join(void **params, int argc);
int on_part(void **params, int argc);
int on_kick(void **params, int argc);
int on_quit(void **params, int argc);


int on_numeric(void **params, int argc);

int plugin_init(plugin *pluginptr)
{
	register_callback(CALLBACK_SERVER_RECV, on_recv, 0, pluginptr);

	register_callback(CALLBACK_GOT_PRIVMSG, on_privmsg, 0, pluginptr);

	register_callback(CALLBACK_CHANNEL_JOIN, on_join, 0, pluginptr);

	register_callback(CALLBACK_CHANNEL_PART, on_part, 0, pluginptr);
	register_callback(CALLBACK_CHANNEL_KICK, on_kick, 0, pluginptr);
	register_callback(CALLBACK_CHANNEL_QUIT, on_quit, 0, pluginptr);

	return BCIRC_PLUGIN_OK;
}


int on_recv(void **params, int argc)
{
	server *srv = params[0];
	char *buffer = params[1];

	if ((!srv) || (!buffer))
		return BCIRC_PLUGIN_CONTINUE;

	bcirc_log("%s\n", "raw", buffer);

	return BCIRC_PLUGIN_OK;
}

int on_numeric(void **params, int argc)
{

	for (int i = 0; i < argc; i++)
		if (params[i] == NULL)
			return BCIRC_PLUGIN_CONTINUE;

	server *srv = params[0];
	char *buf = params[2];

	if (!srv->motd_sent == 1)
		return BCIRC_PLUGIN_OK;

	if (!srv->network_name)
		return BCIRC_PLUGIN_OK;
	bcirc_printf("%s: %s\n", srv->network_name, buf);

	return BCIRC_PLUGIN_STOP;
}

int on_privmsg(void **params, int argc)
{

	char *nick = params[1];
	char *target = params[3];
	char *msg = params[4];

	bcirc_printf("%s <%s>: %s\n", target, nick, msg);

	return BCIRC_PLUGIN_OK;
}

int on_server_send(void **paramams, int argc)
{
	server *srv = (server*) paramams[0];
	char *buf = (char*) paramams[1];

	if ((srv == NULL) || (buf == NULL))
		return BCIRC_PLUGIN_BREAK;

	bcirc_printf("%s\n", buf);
	bcirc_log(buf, NULL);

	return BCIRC_PLUGIN_OK;
}


int on_join(void **params, int argc)
{
	channel *chan = params[0];
	char *nick = params[1];

	if ((!chan) || (!nick))
	{
		puts("wut");
		return BCIRC_PLUGIN_BREAK;
	}

	bcirc_printf("User %s joined channel %s\n", nick, chan->name);

	return BCIRC_PLUGIN_OK;
}

int on_part(void **params, int argc)
{
	channel *chan = params[0];
	char *nick = params[1];
	char *hostmask = params[2];
	char *reason = params[3];


	if (reason)
		bcirc_printf("User %s left channel %s(%s)\n", nick, chan->name, reason);
	else if (reason)
		bcirc_printf("User %s left channel %s\n", nick, chan->name);

	return BCIRC_PLUGIN_OK;
}

int on_quit(void **params, int argc)
{
	channel *chan = params[0];
	char *nick = params[1];
	char *hostmask = params[2];
	char *reason = params[3];

	if (reason)
		bcirc_printf("User %s left server %s(%s)\n", nick, chan->srv->network_name, reason);
	else
		bcirc_printf("User %s left server %s\n", nick, chan->srv->network_name);

	return BCIRC_PLUGIN_OK;
}

int on_kick(void **params, int argc)
{
	channel *chan = params[0];
	char *nick = params[1];
	char *hostmask = params[2];
	char *reason = params[3];

	if (reason)
		bcirc_printf("User %s was kicked from channel %s(%s)\n", nick, chan->name, reason);
	else
		bcirc_printf("User %s was kicked from channel %s\n", nick, chan->name);

	return BCIRC_PLUGIN_OK;
}
