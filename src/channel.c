#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "../headers/numeric.h"
#include "../headers/server.h"
#include "../headers/channel.h"

channel **channel_list;
int channel_count;

pthread_mutex_t channel_global_mutex;

channel *create_channel(char *chan_name, server *srv)
{
    if ((!chan_name) || (!srv))
        return NULL;

    channel *new_channel = NULL;
    new_channel = malloc(sizeof(channel));

    new_channel->name = malloc((strlen(chan_name) + 1) * sizeof(char));
    strcpy(new_channel->name, chan_name);

    if (!new_channel->name)
        return NULL;

    new_channel->modes = NULL;
    new_channel->users = NULL;
    new_channel->key = NULL;
    new_channel->srv = srv;
    new_channel->topic = NULL;
    new_channel->topic_creator = NULL;
    new_channel->topic_created_time = 0;
    //new_channel->mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&channel_global_mutex);
    channel **new_channel_list = realloc(channel_list, (channel_count + 1) * sizeof(channel*) );
    if (!new_channel_list)
    {
        printf("Failed to realloc new_channel_list (%s)\n", __PRETTY_FUNCTION__);
        exit(EXIT_FAILURE);
    }
    new_channel_list[channel_count] = malloc(sizeof(channel));
    if (!new_channel_list)
    {
        printf("Failed to malloc!(%s)\n", __PRETTY_FUNCTION__);
        exit(EXIT_FAILURE);
    }

    new_channel_list[channel_count] = new_channel;
    channel_count++;

    channel_list = new_channel_list;

    pthread_mutex_unlock(&channel_global_mutex);

    return new_channel;
}
/*
int remove_channel(channel *channel_ptr)
{
    if (!channel_ptr)
        return -1;

    channel **new_list = NULL;
    channel *last_chan = NULL;

    size_t new_count = 0;

    for (int i = 0; channel_list[i]->next_channel; i++)
    {
        if (channel_list[i] == channel_ptr)
        {
            if (realloc(new_list, (new_count + 1) * sizeof(channel*)) == NULL )
            {
                printf("realloc() failed at %s\n", __PRETTY_FUNCTION__ );
                exit(EXIT_FAILURE);
            }
            new_list[new_count] = malloc(sizeof(channel));

            new_list[new_count] = channel_list[i];
            new_list[new_count]->next_channel = NULL;

            last_chan->next_channel = new_list[new_count];
            last_chan = new_list[new_count];


            new_count++;
        }
    }
    return new_count;
}
*/

channel *get_channel(char *chan_name, server *srv)
{
    if (!chan_name)
        return NULL;

    pthread_mutex_lock(&channel_global_mutex);
    pthread_mutex_lock(&srv->mutex);

    for (int i = 0; i < channel_count; i++)
    {
        if ((strcmp(chan_name, channel_list[i]->name) == 0 )
            && (channel_list[i]->srv == srv))
        {
            pthread_mutex_unlock(&channel_global_mutex);
            pthread_mutex_unlock(&srv->mutex);
            return channel_list[i];
        }
    }
    pthread_mutex_unlock(&channel_global_mutex);
    pthread_mutex_unlock(&srv->mutex);
    return NULL;
}
