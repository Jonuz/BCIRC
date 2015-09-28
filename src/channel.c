#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "../headers/numeric.h"
#include "../headers/server.h"
#include "../headers/channel.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"


channel **channel_list;
int channel_count;

pthread_mutex_t channel_global_mutex;

channel *create_channel(char *chan_name, struct server *srv)
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

	pthread_mutex_init(&new_channel->mutex, NULL);

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


channel *get_channel(char *chan_name, struct server *srv)
{
    if (!chan_name)
        return NULL;

    for (int i = 0; i < channel_count; i++)
    {
        pthread_mutex_lock(&channel_global_mutex);
        if ((strcmp(chan_name, channel_list[i]->name) == 0 )
            &&
            (channel_list[i]->srv == srv))
        {
            pthread_mutex_unlock(&channel_global_mutex);
            return channel_list[i];
        }
        pthread_mutex_unlock(&channel_global_mutex);
    }
    return NULL;
}

int remove_user(void **params, int arcv)
{
    channel *chan = params[0];
    char *nick = params[1];

    if ((!chan) || (!nick))
        return BCIRC_PLUGIN_BREAK;

    char *new_users = NULL;

    char *token, *save;

    token = strtok_r(chan->users, " ", &save);
    size_t len = 0;

    while (token != NULL)
    {
        if (strcmp(token, nick) == 0)
        {
            len += strlen(token);
            continue;
        }
        else
        {
            size_t new_len = 0;

            if (chan->users)
                new_len = strlen(chan->users) -  strlen(token);
            else
                new_len = strlen(token);

            if (new_len == 0)
            {
                free(chan->users);
                chan->users = 0;
                return BCIRC_PLUGIN_OK;
            }

            size_t second_start = len + 1 + strlen(token);
            new_users = malloc(new_len * sizeof(char));


            strncpy(new_users, chan->users, len);
            strncat(new_users, chan->users+second_start, strlen(chan->users - second_start));

            printf("new_users: %s\n", new_users);

        }

        token = strtok_r(NULL, " ", &save);
    }
    free(chan->users);
    chan->users = new_users;

    return BCIRC_PLUGIN_OK;
}


int get_channel_users(channel *chan, char *buffer)
{
    if (!buffer)
    {
        puts("buffers is null!");
        return BCIRC_PLUGIN_OK;
    }


	char *users = NULL;
	size_t users_len = 0;

	users = strstr(buffer+1, ":");
	users++;

	//printf("user_start: %s\n", users);

	if (chan->users)
		chan->users = realloc(chan->users, (strlen(chan->users) +  strlen(users) + 1) * sizeof(char));
	else
		chan->users = malloc((strlen(users) + 1) * sizeof(char));

	if (!chan->users)
	{
		printf("Failed to realloc chan->users(%s)\n", __PRETTY_FUNCTION__);
		exit(EXIT_FAILURE);
	}

    strcat(chan->users, users);

    return BCIRC_PLUGIN_OK;
}
