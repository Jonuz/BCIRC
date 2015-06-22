#include <stdlib.h>
#include <stdio.h>

#include "../headers/channel.h"
#include "../headers/server.h"

channel **channel_list;

int get_channel_count()
{
    int count = 0;
    while(channel_list[count]->next_channel)
        count++;

    return count;
}

channel *create_channel(char *chan_name, server *srv)
{
    if ((!chan_name) || (!srv))
        return NULL;

    int channel_count = get_channel_count();

    channel *new_channel = NULL;

    new_channel->name = malloc((strlen(chan_name) + 1) * sizeof(char));
    strcpy(new_channel->name, chan_name);

    if (!new_channel->name)
        return NULL;

    new_channel->modes = NULL;
    new_channel->next_channel = NULL;
    new_channel->users = NULL;
    new_channel->key = NULL;
    new_channel->srv = srv;
    new_channel->topic = NULL;

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
    channel_list[channel_count]->next_channel = NULL;

    return new_channel;
}

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

channel *get_channel(char *chan_name)
{
    if (!chan_name)
        return NULL;
    for (int i = 0; channel_list[i]->next_channel; i++ )
    {
        if ( strcmp( channel_list[i]->name, chan_name) == 0)
            return channel_list[i];
    }
    return NULL;
}
