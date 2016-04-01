#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <libconfig.h>
#include <pthread.h>

#include "../headers/log.h"
#include "../headers/irc.h"
#include "../headers/server.h"
#include "../headers/numeric.h"
#include "../headers/channel.h"
#include "../headers/irc_cmds.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"

#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

int autojoin_on_invite(void **params, int argc);

char plugin_name[] = "Autojoin";
char plugin_author[] = "Joona";
char plugin_version[] = "0.1";

int plugin_init(plugin *pluginptr)
{
    register_callback(CALLBACK_CHANNEL_INVITE, autojoin_on_invite, 20, pluginptr);
    return BCIRC_PLUGIN_OK;
}


int autojoin_on_invite(void **params, int argc)
{
    if (params[0] == NULL)
    {
        bcirc_printf("Channel is null\n");
        return BCIRC_PLUGIN_FAIL;
    }

    channel *chan = params[0];
    join_channel(chan->name, NULL, chan->srv);

    return BCIRC_PLUGIN_OK;
}
