#include <stdio.h>

#include "../headers/irc.h"
#include "../headers/server.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"

char plugin_name[] = "Logger";
char plugin_version[] = "0.01";
char plugin_author[] = "Joona";

int on_server_send(void **paramams, int argc);
int on_connect(void **params, int argc);

int on_privmsg(void **params, int argc);

int on_join(void **params, int argc);
int on_part(void **params, int argc);
int on_kick(void **params, int argc);

int plugin_init(plugin *pluginptr)
{
    register_callback(CALLBACK_SERVER_CONNECTED, on_connect, 20, pluginptr);
	register_callback(CALLBACK_GOT_PRIVMSG, on_privmsg, 20, pluginptr);

	//register_callback(CALLBACK_CHANNEL_JOIN, on_join, 20, pluginptr);
	//register_callback(CALLBACK_CHANNEL_PART, on_part, 20, pluginptr);
	//register_callback(CALLBACK_CHANNEL_KICK, on_kick, 20, pluginptr);

    return BCIRC_PLUGIN_OK;
}


int on_privmsg(void **params, int argc)
{

	char *nick = params[1];
	char *target = params[3];
	char *msg = params[4];

	printf("%s <%s>: %s\n", target, nick, msg);

	return BCIRC_PLUGIN_OK;
}

int on_connect(void **params, int argc)
{
    printf("on_connect..\n");
    server *srv = (server*) params[0];
    printf("Connected to %s:%s\n", srv->host, srv->port);

    return BCIRC_PLUGIN_OK;
}

int on_server_send(void **paramams, int argc)
{
    server *srv = (server*) paramams[0];
    char *buf = (char*) paramams[1];

    if ((srv == NULL) || (buf == NULL))
        return BCIRC_PLUGIN_BREAK;

    printf("%s\n", buf);

    return BCIRC_PLUGIN_OK;
}


int on_join(void **params, int argc)
{



	return BCIRC_PLUGIN_OK;
}
