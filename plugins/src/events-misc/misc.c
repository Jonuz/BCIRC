#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>

#include "misc.h"
#include "../headers/irc.h"
#include "../headers/server.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"

char plugin_name[] = "BCIRC-Core - Event handler - Misc";
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
    register_callback(CALLBACK_SERVER_RECV, get_privmsg, 20, pluginptr);
    register_callback(CALLBACK_SERVER_RECV, get_numeric, 20, pluginptr);

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

    printf("msg: %s\n", (char*) msg);

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

        new_params[0] = (int*) numeric;
        new_params[1] = buffer;
        new_params[2] = srv;

        execute_callbacks(CALLBACK_GOT_NUMERIC, new_params, 3);

        free(numeric);
        free(new_params);
    }
    return BCIRC_PLUGIN_OK;
}
