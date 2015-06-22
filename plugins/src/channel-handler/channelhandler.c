#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "../headers/server.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"

char plugin_name[] = "BCIRC-Core - Channel handler";
char plugin_author[] = "Joona";
char plugin_version[] = "1.0";

int register_channel(void **params, int argc);

int plugin_init(plugin *pluginptr)
{
    register_callback(CALLBACK_SERVER_RECV, register_channel, pluginptr);

    return BCIRC_PLUGIN_OK;
}



int register_channel(void **params, int argc)
{
    server *srv = params[0];
    char buffer[strlen(params[1])];
    int *numeric = params[2];

    strcpy(buffer, params[1]);

    if ((!srv) || (!buffer) || (!numeric))
        return BCIRC_PLUGIN_BREAK;

    if (*numeric != RPL_TOPIC)
        return BCIRC_PLUGIN_OK;



}
