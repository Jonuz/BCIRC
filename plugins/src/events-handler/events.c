#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>

#include "events.h"
#include "../headers/log.h"
#include "../headers/irc.h"
#include "../headers/server.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"

char plugin_name[] = "BCIRC-Core - Event handler";
char plugin_author[] = "Joona";
char plugin_version[] = "1.0";

bool is_fulldigit(char *str)
{
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (!isdigit(str[i]))
            return false;
    }
    return true;
}


int plugin_init(plugin *pluginptr)
{
    register_callback(CALLBACK_SERVER_RECV, get_privmsg, 3, pluginptr);
    register_callback(CALLBACK_SERVER_RECV, get_numeric, 3, pluginptr);
    register_callback(CALLBACK_SERVER_RECV, get_chan_event, 3, pluginptr);


    return BCIRC_PLUGIN_OK;
}


int get_privmsg(void **params, int argc)
{
    char *srv = params[0];
    char buf[strlen(params[1])];
    strcpy(buf, params[1]);

    char *hostmask = NULL;
    char *target = NULL;
    char *nick = NULL;

    char *str;
    char *save;

    str = strtok_r(buf, ":! ", &save);

    int i = 0;
    for (i = 0; str != NULL; i++)
    {
        if (i == 0)
        {
            nick = malloc((strlen(str) + 1) * sizeof(char));
            strcpy(nick, str);
        }
        if (i == 1)
        {
            hostmask =  malloc((strlen(str) + 1) * sizeof(char));
            strcpy(hostmask, str);

        }
        if (i == 2)
            if (strcmp(str, "PRIVMSG") != 0)
            {
                free(nick);
                free(hostmask);

                return BCIRC_PLUGIN_OK;
            }
        if (i == 3)
        {
            target =  malloc((strlen(str) + 1) * sizeof(char));
            strcpy(target, str);
            break;
        }
        str = strtok_r(NULL, ":! ", &save);
    }

    if (i < 3) // In case of PING or something like that since loop might end before PRIVMSG check.
    {
        free(nick);
        free(hostmask);

        return BCIRC_PLUGIN_OK;
    }

    int msg_len = strlen(save);
    char *msg = malloc((msg_len + 1) * sizeof(char) );
    strncpy(msg, save+1, msg_len);
    msg[msg_len] = '\0';

    void **new_params = malloc(sizeof(void*) * 5);

    new_params[0] = (server*) srv;
    new_params[1] = (char*) nick;
    new_params[2] = (char*) hostmask;
    new_params[3] = (char*) target;
    new_params[4] = (char*) msg;

    execute_callbacks(CALLBACK_GOT_PRIVMSG, new_params, 5);

    free(new_params);

    free(nick);
    free(hostmask);
    free(target);
    free(msg);

    return BCIRC_PLUGIN_OK;
}


int get_numeric(void **params, int argc)
{

    server *srv = (server*) params[0];
    char buffer[strlen(params[1])];
    strcpy(buffer, params[1]);

    if (buffer == NULL)
        return BCIRC_PLUGIN_OK;
    if (srv == NULL)
        return BCIRC_PLUGIN_OK;

    char *save;
    char *word;

    word = strtok_r(buffer, " ", &save);
    if (!word)
      return BCIRC_PLUGIN_OK;
    word = strtok_r(NULL, " ", &save);

    if (!word)
      return BCIRC_PLUGIN_OK;


    if (is_fulldigit(word))
    {
        int *numeric = malloc(sizeof(int));
        *numeric = atoi(word);

        strcpy(buffer, params[1]);
        void **new_params = malloc(sizeof(void*) * 3);

        new_params[0] = srv;
        new_params[1] = (int*) numeric;
        new_params[2] = buffer;

        execute_callbacks(CALLBACK_GOT_NUMERIC, new_params, 3);

        free(numeric);
        free(new_params);
    }
    return BCIRC_PLUGIN_OK;
}



int get_chan_event(void **params, int argv)
{
    server *srv = params[0];
    char *buffer = params[1];

    if (srv->motd_sent == 0)
        return 0;

    char *str = malloc((strlen(buffer) + 1) * sizeof(char));
    strcpy(str, buffer);

    char *save, *token;

    char *nick, *hostmask;
    char *reason = NULL;
    channel *chan = NULL;

    int event_type = 0; // 0 = part, 1 = kick

    token = strtok_r(str, " ", &save);

    //joona!~joona@127.0.0.1 PART #tesm :WeeChat 1.2
    for (int i = 0; token != NULL; i++)
    {
        if (i == 1)
        {
            if (strcmp(token, "JOIN") == 0)
                event_type = CHAN_JOIN;
            else if (strcmp(token, "KICK") == 0)
                event_type = CHAN_KICK;
            else if (strcmp(token, "PART") == 0)
                event_type = CHAN_PART;
            else if (strcmp(token, "QUIT") == 0)
                event_type = CHAN_QUIT;
            else
            {
                free(str);
                return BCIRC_PLUGIN_OK;
            }

            char *nick_end = memchr(buffer, '!', strlen(buffer));
            if (nick_end == NULL)
                nick_end = memchr(buffer, '*', strlen(buffer));
            if (nick_end == NULL)
            {
                bcirc_printf("nick_end is still NULL(%s)\n", __PRETTY_FUNCTION__);
                return BCIRC_PLUGIN_OK;
            }

            size_t nick_len = strlen(buffer) - strlen(nick_end) - 1;

            nick = malloc((nick_len + 1) * sizeof(char));
            memmove(nick, buffer+1, nick_len);
            nick[nick_len] = '\0';

            char *mask_end = memchr(buffer, ' ', strlen(buffer));
            size_t mask_len = strlen(buffer) - strlen(mask_end) - nick_len - 2;

            hostmask = malloc((mask_len + 1) * sizeof(char));
            memmove(hostmask, buffer + nick_len + 2, mask_len);
            hostmask[mask_len] = '\0';
        }

        if (i == 2)
        {
            chan = (channel*) get_channel(token, (server*) srv);
            if (!chan)
                chan = create_channel(token,srv);
        }
        if (i == 3)
        {
            reason = malloc(( strlen(token) + 1 + strlen(save) + 1) * sizeof(char));
            strcpy(reason, token+1);
            strcat(reason, save);

            break;
        }
        token = strtok_r(NULL, " ", &save);
    }


    void **params2 = malloc(4 * sizeof(void*));
    params2[0] = chan;
    params2[1] = nick;
    params2[2] = hostmask;
    params2[3] = reason;


    if (event_type == CHAN_JOIN)
        execute_callbacks(CALLBACK_CHANNEL_JOIN, params2, 4);
    else if (event_type == CHAN_KICK)
        execute_callbacks(CALLBACK_CHANNEL_KICK, params2, 4);
    else if (event_type == CHAN_PART)
        execute_callbacks(CALLBACK_CHANNEL_PART, params2, 4);
    else if (event_type == CHAN_QUIT)
        execute_callbacks(CALLBACK_CHANNEL_QUIT, params2, 4);

    free(str);
    free(params2);
    free(nick);
    free(hostmask);
    free(reason);

    return BCIRC_PLUGIN_OK;
}
