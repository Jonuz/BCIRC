#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "../include/irc.h"
#include "../include/irc_cmds.h"
#include "../include/plugin-handler.h"

#define get_str_size(str) ( (strlen( (char*) str) + 1) * sizeof(char) )
#define MAX_MSG_COUNT 15


bool is_fulldigit(char *str)
{
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (!isdigit(str[i]))
            return false;
    }
    return true;
}

typedef struct
{
    char *msg;
    char *target;
    server *srv;

} msg_info;
int msg_count = 0;

msg_info **msg_list;


static pthread_mutex_t global_msglist_mutex = PTHREAD_MUTEX_INITIALIZER;

int add_to_privmsg_queue(char *msg, char *target, server *srv)
{
    static pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
    static pthread_t thread;

    static int handler_started = 0;

    pthread_mutex_trylock(&mutex1);

    if (msg_count >= MAX_MSG_COUNT)
    {
        return 0;
    }

    msg_info *new_msg_info = malloc(sizeof(msg_info));
    new_msg_info->msg = malloc((strlen(msg) + 1) * sizeof(char));
    new_msg_info->target = malloc((strlen(target) + 1) * sizeof(char));

    strcpy(new_msg_info->msg, msg);
    strcpy(new_msg_info->target, target);
    new_msg_info->srv = srv;


    pthread_mutex_trylock(&global_msglist_mutex);
    msg_list = realloc( msg_list, (msg_count + 1) * sizeof (msg_info*) );
    msg_list[msg_count] = new_msg_info;
    msg_count++;
    pthread_mutex_unlock(&global_msglist_mutex);


    if (handler_started == 0)
    {
        if ((pthread_create(&thread, NULL, &handle_privmsg_queue, NULL)))
        {
            puts("Failed to crate thread!");
            exit(0);
        }
        pthread_detach(thread);
        handler_started = 1;
    }

    pthread_mutex_unlock(&mutex1);

    return 1;
}

void *handle_privmsg_queue()
{
    int history_size = 12;
    double last_message_times[history_size];

    for (int i = 0; i < history_size; i++)
        last_message_times[i] =  0;

    double time_now = 0;
    double avarage_timestamp = 0;

    struct timeval tv;

    while(1)
    {
        for (int i = 0; i < history_size; i++)
            avarage_timestamp += last_message_times[i];

        pthread_mutex_lock(&global_msglist_mutex);
        if (msg_count == 0)
            continue;

        avarage_timestamp /= history_size;
        gettimeofday(&tv, NULL);

        time_now = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;


        double delay = (avarage_timestamp / time_now) * 1000000;
        printf("delay: %f\n", delay);

        if (delay > 750000)
        {
            printf("Will sleep!\n");
            usleep(delay);
        }
        if (!msg_list[0]->srv)
            puts("Server is now available anymore.");

        privmsg(msg_list[0]->msg, msg_list[0]->target, msg_list[0]->srv);
        msg_count--;

        for (int i = history_size; i > 0 ; i--)
            last_message_times[i] = last_message_times[i-1];

        gettimeofday(&tv, NULL);

        last_message_times[0] = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;


        free(msg_list[0]->msg);
        free(msg_list[0]->target);

        for (int i = 1; i < msg_count; i++)
            msg_list[i-1] = msg_list[i];


        pthread_mutex_unlock(&global_msglist_mutex);
    }
    puts("privmsg hanlder loop stopped.. this should not happen..");
    exit(0);
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

        new_params[0] = (int*) numeric;
        new_params[1] = buffer;
        new_params[2] = srv;

        execute_callbacks(CALLBACK_GOT_NUMERIC, new_params, 3);

        free(numeric);
        free(new_params);
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
