#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>


#include "../include/irc.h"
#include "../include/plugin-handler.h"

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
    char *buffer = malloc( sizeof( (char*) params[0]) );
    server *srv = malloc( sizeof( (server*) params[1]) );

    srv = (server*) params[0];
    buffer = (char*) params[1];

    if (buffer == NULL)
        return BCIRC_PLUGIN_STOP;
    if (srv == NULL)
        return BCIRC_PLUGIN_STOP;

    char *save = malloc(strlen(buffer) * sizeof(char));
    char *word = malloc(strlen(buffer) * sizeof(char));

    word = strtok_r(buffer, " ", &save);
    if (!word)
      return BCIRC_PLUGIN_CONTINUE;
    word = strtok_r(NULL, " ", &save);
    if (!word)
      return BCIRC_PLUGIN_CONTINUE;

    if (is_fulldigit(word))
    {
      int numeric = atoi(word);

      void **params = malloc(sizeof(void*) * 3);
      for (int i = 0; i < 3; i++)
        params[i] = malloc(sizeof(void*));

      params[0] = (void*) numeric;
      params[1] = (void*) buffer;
      params[2] = (void*) srv;

      execute_callbacks(CALLBACK_GOT_NUMERIC, params, 3);
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
    char *msg = NULL;

    char *save = malloc(sizeof(buf));
    char *str = strtok_r(buf, ":! ", &save);

    //:Jonuz!~Joona@178.62.198.166 PRIVMSG dadasd :lol oot homo

    for (int i = 0; str != NULL; i++)
    {
        //printf("str: %s | i: %d\n", str, i);
        if (i == 0)
        {
            nick = malloc((strlen(str) + 1) * sizeof(char));
            strcpy(nick, str);
        }
        if (i == 1)
        {
            hostmask = malloc((strlen(str) + 1) * sizeof(char));
            strcpy(hostmask, str);
        }
        if (i == 2)
            if (strcmp(str, "PRIVMSG") != 0)
                return BCIRC_PLUGIN_OK;

        if (i == 3)
        {
            target = malloc((strlen(str) + 1) * sizeof(char));
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


      msg = malloc(sizeof(save));
      strcpy(msg, save);
      msg++;

      int len = strlen(msg);
      msg[len-1] = '\0';

      void **params = malloc(sizeof(void*) * 5);
      params[0] = srv;
      params[1] = nick;
      params[2] = hostmask;
      params[3] = target;
      params[4] = msg;

      execute_callbacks(CALLBACK_GOT_PRIVMSG, params, 5);

      return BCIRC_PLUGIN_OK;
}
