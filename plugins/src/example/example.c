#include "../headers/irc.h"
#include "../headers/log.h"
#include "../headers/server.h"
#include "../headers/irc_cmds.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"

char plugin_name[] = "Example plugin";
char plugin_version[] = "1.0";
char plugin_author[] = "Joona";


int hello(void **params, int argc)
{
    for (int i = 0; i < argc; i++)
    {
        if (params[i] == NULL)
            return BCIRC_PLUGIN_FAIL;
    }
    server *srv = params[0]; //See callback_defines.h
    char *nick = params[1];
    char *hostmask = params[2];
    char *target = params[3];
    char *msg = params[4];

    bcirc_printf("Nick %s sent message %s to %s\n", nick, msg, target);

    if (strstr(msg, "!hoi") == msg)
    {
        char reply[512];
        sprintf(reply, "Hey you, you there, %s!", nick);
        privmsg(msg, nick, srv);
    }

    bcirc_printf("Initalized example plugin\n");

    return BCIRC_PLUGIN_OK;
}


int plugin_init(plugin *pluginptr)
{
    if (pluginptr == NULL)
        return BCIRC_PLUGIN_FAIL;

    register_callback(CALLBACK_GOT_PRIVMSG, hello, 20, pluginptr); //callbacks are sorted by priority, callback with "lowest" priority are called first.

    return BCIRC_PLUGIN_OK;
}
