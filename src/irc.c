#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "../headers/irc.h"
#include "../headers/irc_cmds.h"
#include "../headers/plugin_handler.h"

#define get_str_size(str) ( (strlen( (char*) str) + 1) * sizeof(char) )
#define MAX_MSG_COUNT 5


typedef struct
{
    char *msg;
    char *target;
    server *srv;
    int drop; // if 1 drop from msg queue if being busy

} msg_info;

int msg_count = 0;
msg_info **msg_list;


pthread_mutex_t global_msglist_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;


int add_to_privmsg_queue(char *msg, char *target, server *srv, const int drop )
{
    static pthread_mutex_t local_mutex = PTHREAD_MUTEX_INITIALIZER;
    static pthread_t thread;

    pthread_mutex_lock(&local_mutex);

    static int handler_started = 0;

    if (msg_count >= MAX_MSG_COUNT)
    {
        printf("Got already %d messages on queue, wont take more.\n", msg_count);
        return 0;
    }

    pthread_mutex_lock(&global_msglist_mutex);

    msg_info *new_msg_info = malloc(sizeof(msg_info));
    new_msg_info->msg = malloc((strlen(msg) + 1) * sizeof(char));
    new_msg_info->target = malloc((strlen(target) + 1) * sizeof(char));
    new_msg_info->drop = drop;

    strcpy(new_msg_info->msg, msg);
    strcpy(new_msg_info->target, target);
    new_msg_info->srv = srv;


    msg_list = realloc( msg_list, (msg_count + 1) * sizeof (msg_info*) );
    msg_list[msg_count] = new_msg_info;
    if (!msg_list)
    {
        printf("Failed to realloc msg_list. (%s)\n", __PRETTY_FUNCTION__);
        exit(EXIT_FAILURE);
    }
    msg_count++;
    pthread_mutex_unlock(&global_msglist_mutex);


    if (handler_started == 0)
    {
        if ((pthread_create(&thread, NULL, &handle_privmsg_queue, NULL)))
        {
            puts("Failed to create thread!");
            exit(0);
        }
        pthread_detach(thread);
        handler_started = 1;
        pthread_mutex_unlock(&local_mutex);

        return 1;
    }

    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&local_mutex);
    return 1;
}


#define HISTORY_SIZE 5
time_t last_message_times[HISTORY_SIZE] = { 0 };

void *handle_privmsg_queue()
{
    static time_t last_message_times[HISTORY_SIZE] = { 0 };

    while(1)
    {
        if (msg_count > 0)
            pthread_mutex_lock(&global_msglist_mutex);

        while(msg_count == 0)
            pthread_cond_wait(&condition, &global_msglist_mutex);

        time_t time_now = 0;
        time_t avarage_timestamp = 0;

        for (int i = 0; i < HISTORY_SIZE; i++)
            avarage_timestamp += last_message_times[i];

        avarage_timestamp /= HISTORY_SIZE;

        struct timeval tv;
        gettimeofday(&tv, NULL);
        time_now = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

        if (time_now - avarage_timestamp <= 100000)
        {
            if (!msg_list[0]->drop)
            {
                puts("Will sleep");
                usleep(12500);
                privmsg(msg_list[0]->msg, msg_list[0]->target, msg_list[0]->srv);
            }
            else
                printf("Dropped message \"%s\" to target %s\n", msg_list[0]->msg, msg_list[0]->target);
        }
        else
            privmsg(msg_list[0]->msg, msg_list[0]->target, msg_list[0]->srv);

        msg_count--;

        free(msg_list[0]->msg);
        free(msg_list[0]->target);
        free(msg_list[0]);

        for (int i = 0; i < msg_count; i++)
            msg_list[i] = msg_list[i+1];

        msg_info **new_msg_list = malloc(msg_count * sizeof(msg_info));
        for (int i = 0; i < msg_count; i++)
            new_msg_list[i] = msg_list[i];
        free(msg_list);
        msg_list = new_msg_list;

        pthread_mutex_unlock(&global_msglist_mutex);

        for (int i = HISTORY_SIZE; i > 0; i--)
            last_message_times[i] = last_message_times[i-1];

        gettimeofday(&tv, NULL);
        last_message_times[0] = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

    }
    puts("privmsg handler loop stopped.. this should not happen..");
    exit(EXIT_FAILURE);
}
