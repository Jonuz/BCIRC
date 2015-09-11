#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../headers/numeric.h"
#include "../headers/server.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"

char plugin_name[] = "BCIRC-Core - Channel handler";
char plugin_author[] = "Joona";
char plugin_version[] = "0.1";

int get_channel_info(void **params, int argcv);

int get_channel_event(void **params, int argc);

int joinded_channel(channel *chan, char *buffer);
int get_channel_title(channel *chan, char *buffer);
int get_channel_title_info(channel *chan, char *buffer);
int get_channel_users(channel *chan, char *buffer);

int remove_user(void **params, int argc);


int plugin_init(plugin *pluginptr)
{
	register_callback(CALLBACK_SERVER_RECV, get_channel_event, 5, pluginptr); //Takes care of parts, joins and kicks

    register_callback(CALLBACK_GOT_NUMERIC, get_channel_info, 10, pluginptr);
    register_callback(CALLBACK_CHANNEL_KICK, remove_user, 10, pluginptr);
    register_callback(CALLBACK_CHANNEL_PART, remove_user, 10, pluginptr);

    return BCIRC_PLUGIN_OK;
}


int get_channel_event(void **params, int argc)
{

	char *str = malloc(strlen( (char*) params[0] + 1) * sizeof(char));
	strcpy(str, params[0]);
	server *srv = params[1];

	char *event, *channel_str, *reason, *nick;
	channel *channel_ptr = NULL;

	char *token, *save;
	event = strtok_r(str, " ", &save);
	channel_str = strtok_r(NULL, " ", &save);
	reason = save;

	channel_ptr = get_channel(channel_str, srv);

	void **params2 = malloc(4 * sizeof(void*));
	params2[0] = channel_ptr;


	free(params2);
	free(str);

	return BCIRC_PLUGIN_OK;
}


int get_channel_info(void **params, int argcv)
{
    int *numeric = (int*) params[0];
    char buffer[strlen(params[1])];
    server *srv = params[2];

    strcpy(buffer, params[1]);

    if (!buffer)
    {
        puts("buffers is null!");
        return BCIRC_PLUGIN_OK;
    }

    while(1)
    {
        if (*numeric == RPL_TOPIC)
            break;
        if (*numeric == RPL_ENDOFNAMES)
            break;
        if (*numeric == RPL_TOPICWHOTIME)
            break;
        if (*numeric == RPL_NAMREPLY)
            break;
        return BCIRC_PLUGIN_OK;
    }


    char *name_start = memchr(buffer, '#', strlen(buffer)); //Todo: use CHANTYPES

    int len = 0;
    for (len; name_start[len] != ' '; len++); {};

    char *chan_name = malloc((len + 1) * sizeof(char));
    memmove(chan_name, name_start, len);
    chan_name[len] = '\0';

    channel *this_channel = NULL;
    if ((this_channel = get_channel(chan_name, srv)) == NULL)
    {
        this_channel = create_channel(chan_name, srv);
        if (!this_channel)
        {
            printf("Failed to add channel named %s\n!", chan_name);
            return BCIRC_PLUGIN_OK;
        }
    }


    switch(*numeric)
    {
        //case RPL_ENDOFNAMES:
        //    return joinded_channel(this_channel, buffer);

        case RPL_TOPIC:
            return get_channel_title(this_channel, buffer);

        case RPL_TOPICWHOTIME:
            return get_channel_title_info(this_channel, buffer);

        case RPL_NAMREPLY:
            return get_channel_users(this_channel, buffer);

        default:
            break;
    }

    return BCIRC_PLUGIN_FAIL ;
}

int joinded_channel(channel *chan, char *buffer)
{

    //printf("\nJoined to channel %s\n", chan->name);
    if (chan->topic)
        printf("Channel topic: %s\n", chan->topic);
    if (chan->topic_created_time)
        printf("Topic created %d by %s\n", chan->topic_created_time, chan->topic_creator);
    if (chan->users)
        printf("Channel users: %s\n\n", chan->users);

    return BCIRC_PLUGIN_OK;
}


int get_channel_title(channel *chan, char *buffer)
{
    if (!buffer)
    {
        puts("buffers is null!");
        return BCIRC_PLUGIN_OK;
    }


    char *title_start = memchr(buffer + 1, ':', strlen(buffer));

    size_t title_len = strlen(buffer) - strlen(title_start);

    chan->topic = malloc((title_len + 1) * sizeof(char));

    memmove(chan->topic, title_start+1, title_len);
   // printf("%s\n", chan->topic);

    return BCIRC_PLUGIN_OK;
}


/*
    :irc.localhost 333 tosibotti2 #tesm joona!~joona@127.0.0.1 1437735223
*/
int get_channel_title_info(channel *chan, char *buffer)
{
    if (!buffer)
    {
        puts("buffers is null!");
        return BCIRC_PLUGIN_OK;
    }

    char *token, *save;

    char *buf_for_tokes = malloc((strlen(buffer) + 1) * sizeof(char));
    strcpy(buf_for_tokes, buffer);

    char *info_start = memchr(buf_for_tokes, '#', strlen(buffer));

    token = strtok_r(info_start, " ", &save);

    for (int i = 0; token != NULL; i++)
    {
        if (i == 1)
        {
            chan-> topic_creator = malloc((strlen(token) + 1) * sizeof(char));
            strcpy(chan->topic_creator, token);
        }
        if (i == 2)
        {
            chan->topic_created_time = atoi(token);
            return BCIRC_PLUGIN_OK;
        }

        token = strtok_r(NULL, " ", &save);
    }

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

int remove_user(void **params, int arcv)
{
    channel *chan = params[0];
    char *nick = params[1];

    if ((!chan) || (!nick))
        return BCIRC_PLUGIN_BREAK;

    char *new_users = NULL;

    char *token, *save;

    token = strtok_r(chan->users, " ", &save);

    while (token != NULL)
    {
        if (strcmp(token, nick) == 0)
            continue;

        size_t new_size = (strlen(new_users) + strlen(token) + 2) * sizeof(char);
        new_users = realloc(new_users, new_size);
        if (new_users == NULL)
        {
            printf("failed to realloc new_users at %s\n", __PRETTY_FUNCTION__);
            exit(EXIT_FAILURE);
        }
        strcat(new_users, " ");
        strcat(new_users, token);

        token = strtok_r(NULL, " ", &save);
    }
    free(chan->users);
    chan->users = new_users;

    return BCIRC_PLUGIN_OK;
}
