#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../headers/irc.h"
#include "../headers/numeric.h"
#include "../headers/server.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"

char plugin_name[] = "Channelhanbdler";
char plugin_author[] = "Joona";
char plugin_version[] = "1.0";

int get_channel_info(void **params, int argcv);

int joinded_channel(channel *chan, char *buffer);
int get_channel_title(channel *chan, char *buffer);
int get_channel_title_info(channel *chan, char *buffer);
int get_channel_users(channel *chan, char *buffer);

int remove_user(void **params, int argc);


int plugin_init(plugin *pluginptr)
{
    register_callback(CALLBACK_GOT_NUMERIC, get_channel_info, 10, pluginptr);
    register_callback(CALLBACK_CHANNEL_KICK, remove_user, 10, pluginptr);
    register_callback(CALLBACK_CHANNEL_PART, remove_user, 10, pluginptr);
    register_callback(CALLBACK_CHANNEL_QUIT, remove_user, 10, pluginptr);

    return BCIRC_PLUGIN_OK;
}


int get_channel_info(void **params, int argcv)
{
    server *srv = params[0];
    int *numeric = (int*) params[1];
    char buffer[strlen(params[2])];

    strcpy(buffer, params[2]);

    if (!buffer)
    {
        bcirc_printf("buffers is null!\n");
        return BCIRC_PLUGIN_OK;
    }

    while(1)
    {
        if (*numeric == RPL_TOPIC)
            break;
        if (*numeric == RPL_ENDOFNAMES)
            break;
        if (*numeric == RPL_TOPICWHOTIME)
            break;
        if (*numeric == RPL_NAMREPLY)
            break;
        return BCIRC_PLUGIN_OK;
    }


    char *name_start = memchr(buffer, '#', strlen(buffer)); //Todo: use CHANTYPES

    int len = 0;
    for (len; name_start[len] != ' '; len++); {};

    char *chan_name = malloc((len + 1) * sizeof(char));
    memmove(chan_name, name_start, len);
    chan_name[len] = '\0';

    channel *this_channel = NULL;
    if ((this_channel = get_channel(chan_name, srv)) == NULL)
    {
        this_channel = create_channel(chan_name, srv);
        if (!this_channel)
        {
            bcirc_printf("Failed to add channel named %s\n!", chan_name);
            return BCIRC_PLUGIN_OK;
        }
    }


    switch(*numeric)
    {
        //case RPL_ENDOFNAMES:
        //    return joinded_channel(this_channel, buffer);

        case RPL_TOPIC:
            return get_channel_title(this_channel, buffer);

        case RPL_TOPICWHOTIME:
            return get_channel_title_info(this_channel, buffer);

        case RPL_NAMREPLY:
            return get_channel_users(this_channel, buffer);

        default:
            break;
    }

    return BCIRC_PLUGIN_FAIL ;
}


int get_channel_title(channel *chan, char *buffer)
{
    if (!buffer)
    {
        puts("buffers is null!");
        return BCIRC_PLUGIN_OK;
    }

    char *title_start = memchr(buffer + 1, ':', strlen(buffer));
    size_t title_len = strlen(buffer) - strlen(title_start);

    chan->topic = malloc((title_len + 1) * sizeof(char));

    memmove(chan->topic, title_start+1, title_len);
   // printf("%s\n", chan->topic);

    return BCIRC_PLUGIN_OK;
}


/*
    :irc.localhost 333 tosibotti2 #tesm joona!~joona@127.0.0.1 1437735223
*/
int get_channel_title_info(channel *chan, char *buffer)
{
    if (!buffer)
    {
        puts("buffers is null!");
        return BCIRC_PLUGIN_OK;
    }

    char *token, *save;

    char *buf_for_tokes = malloc((strlen(buffer) + 1) * sizeof(char));
    strcpy(buf_for_tokes, buffer);

    char *info_start = memchr(buf_for_tokes, '#', strlen(buffer));

    token = strtok_r(info_start, " ", &save);

    for (int i = 0; token != NULL; i++)
    {
        if (i == 1)
        {
            chan-> topic_creator = malloc((strlen(token) + 1) * sizeof(char));
            strcpy(chan->topic_creator, token);
        }
        if (i == 2)
        {
            chan->topic_created_time = atoi(token);
            return BCIRC_PLUGIN_OK;
        }

        token = strtok_r(NULL, " ", &save);
    }

    return BCIRC_PLUGIN_OK;
}
