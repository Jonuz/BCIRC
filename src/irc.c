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

    server *srv = (server*) params[0];
    char *buffer = (char*) params[1];

    if (buffer == NULL)
        return BCIRC_PLUGIN_STOP;
    if (srv == NULL)
        return BCIRC_PLUGIN_STOP;

    char *save;
    char *word;

    word = strtok_r(buffer, " ", &save);
    if (!word)
      return BCIRC_PLUGIN_CONTINUE;

    word = strtok_r(NULL, " ", &save);
    if (!word)
      return BCIRC_PLUGIN_CONTINUE;

    if (is_fulldigit(word))
    {
        int *numeric = malloc(sizeof(int));
        *numeric = atoi(word);

        void **new_params = malloc(sizeof(void*) * 3);

        new_params[0] = (void*) numeric;
        new_params[1] = buffer;
        new_params[2] = srv;

        execute_callbacks(CALLBACK_GOT_NUMERIC, new_params, 3);

        free(numeric);
    }
    return BCIRC_PLUGIN_OK;
}

int get_privmsg(server *srv, char *buf)
{
    if (!srv)
        return -1;
    if (!buf)
        return -1;

    char *nick = NULL;
    char *hostmask = NULL;
    char *target = NULL;

    char *str;
    char *save;

    str = strtok_r(buf, ":! ", &save);

    int i = 0;
    for (i = 0; str != NULL; i++)
    {
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
            {
                free(nick);
                free(hostmask);

                return BCIRC_PLUGIN_OK;
            }
        if (i == 3)
        {
            target =  malloc(get_str_size(str));
            strcpy(target, str);
            break;
        }
        str = strtok_r(NULL, ":! ", &save);
      }


      if (i < 3) // In case of PING or something like that.
      {
          return -1;
      }


      char *msg = malloc((strlen(save) - 3 + 1) * sizeof(char));
      strncpy(msg, save+1, strlen(save) - 3); //Skip : at start and \r\n from end
      msg[strlen(save) - 3] = '\0';

      void **params = malloc(sizeof(void*) * 5);

      params[0] = (server*) srv;
      params[1] = (char*) nick;
      params[2] = (char*) hostmask;
      params[3] = (char*) target;
      params[4] = (char*) msg;

      execute_callbacks(CALLBACK_GOT_PRIVMSG, params, 5);

      free(params);

      free(nick);
      free(hostmask);
      free(target);
      free(msg);

      return BCIRC_PLUGIN_OK;
}
