#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>


#include "../include/irc.h"
#include "../include/plugin-handler.h"

#define get_str_size(str) ( (strlen( (char*) str) + 1) * sizeof(char) )


bool is_fulldigit(char *str)
{
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (!isdigit(str[i]))
            return false;
    }
    return true;
}



int get_numeric(void **params, int argc)
{
    server *srv = malloc(sizeof(server));
    char *buffer = malloc(sizeof(get_str_size(params[1])));


    srv = (server*) params[0];
    buffer = (char*) params[1];

    if (buffer == NULL)
        return BCIRC_PLUGIN_STOP;
    if (srv == NULL)
        return BCIRC_PLUGIN_STOP;

    char *save = malloc(get_str_size(buffer));
    char *word = malloc(get_str_size(buffer));

    word = strtok_r(buffer, " ", &save);
    if (!word)
      return BCIRC_PLUGIN_CONTINUE;

    word = strtok_r(NULL, " ", &save);
    if (!word)
      return BCIRC_PLUGIN_CONTINUE;

    if (is_fulldigit(word))
    {
        int numeric = atoi(word);
        void **new_params = malloc(sizeof(void*) * 3);

        new_params[0] = malloc(sizeof(numeric));
        new_params[1] = malloc(get_str_size(buffer));
        new_params[2] = malloc(sizeof(server));

        new_params[0] = numeric;
        new_params[1] = buffer;
        new_params[2] = srv;

        execute_callbacks(CALLBACK_GOT_NUMERIC, new_params, 3);
    }
    return BCIRC_PLUGIN_OK;
}

int is_privmsg(server *srv, char *buf)
{
    if (!srv)
        return -1;
    if (!buf)
        return -1;

    char *nick = NULL;
    char *hostmask = NULL;
    char *target = NULL;

    char *str = malloc(get_str_size(buf));
    char *save = malloc(get_str_size(buf));
    str = strtok_r(buf, ":! ", &save);

    //:Jonuz!~Joona@178.62.198.166 PRIVMSG dadasd :lol oot homo

    for (int i = 0; str != NULL; i++)
    {
        //printf("str: %s | i: %d\n", str, i);
        if (i == 0)
        {
            nick = malloc(get_str_size(str));
            strcpy(nick, str);
        }
        if (i == 1)
        {
            hostmask =  malloc(get_str_size(str));
            strcpy(hostmask, str);
        }
        if (i == 2)
            if (strcmp(str, "PRIVMSG") != 0)
                return BCIRC_PLUGIN_OK;

        if (i == 3)
        {
            target =  malloc(get_str_size(str));
            strcpy(target, str);
            break;
        }
        str = strtok_r(NULL, ":! ", &save);
      }

      if (!nick)
          return -2;
      if (!hostmask)
          return -2;
      if (!target)
          return -2;


      char *msg = malloc(get_str_size(save));
      strcpy(msg, save);
      msg++;

      int len = strlen(msg);
      msg[len-1] = '\0';

      void **params = malloc(sizeof(void*) * 5);

      params[0] = malloc(sizeof(server));
      params[1] = malloc(get_str_size(nick));
      params[2] = malloc(get_str_size(hostmask));
      params[3] = malloc(get_str_size(target));
      params[4] = malloc(get_str_size(msg));

      params[0] = (server*) srv;
      params[1] = (char*) nick;
      params[2] = (char*) hostmask;
      params[3] = (char*) target;
      params[4] = (char*) msg;

      execute_callbacks(CALLBACK_GOT_PRIVMSG, params, 5);

      return BCIRC_PLUGIN_OK;
}
