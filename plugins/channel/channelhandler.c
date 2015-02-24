#include <stdio.h>

#include "../../include/plugin-handler.h"
#include "../../include/server.h"
#include "../../include/channel.h"
#include "../../include/irc.h"


#include "../../include/numeric.h"


int handle_ping(void **params, int argc);
int handle_registeration(void **params, int argc);
int got_in(void **params, int argc);


int check_numeric(void **params, int argc);


int set_topic(void **params);
int add_users(void **params);

int wanted_responses[] = {
                            ERR_NEEDMOREPARAMS, ERR_BANNEDFROMCHAN,
                            ERR_INVITEONLYCHAN, ERR_BADCHANNELKEY,
                            ERR_CHANNELISFULL, ERR_BADCHANMASK,
                            ERR_NOSUCHCHANNEL, ERR_TOOMANYCHANNELS,
                            ERR_TOOMANYTARGETS, ERR_UNAVAILRESOURCE,
                            RPL_NAMREPLY
                        };


int plugin_init(plugin *pluginptr)
{
    register_callback(CALLBACK_GOT_NUMERIC, check_numeric, pluginptr);

    return BCIRC_PLUGIN_OK;
}



int check_numeric(void **params, int argc)
{
    for (int i = 0; i < argc; i++)
        if (params[i] == NULL);
            return BCIRC_PLUGIN_BREAK;

    int numeric =  (int) params[0];
    char *buf = params[1];
    server *srv = params[2];

    switch(numeric)
    {
        case RPL_TOPIC:
            set_topic(params);
            break;

        case RPL_NAMREPLY:
            add_users(params);
            break;

        case ERR_NEEDMOREPARAMS:
            break;

        case ERR_BANNEDFROMCHAN:

            break;


        case ERR_INVITEONLYCHAN:

            break;

        case ERR_CHANNELISFULL:

            break;

        case ERR_BADCHANMASK:

            break;

        case ERR_NOSUCHCHANNEL:

            break;

        case ERR_TOOMANYCHANNELS:


            break;

        case ERR_TOOMANYTARGETS:


            break;

        case ERR_UNAVAILRESOURCE:

                break;
    }

}


int set_topic(void **params)
{
    char *buf = params[1];
    server *srv = params[2];

    channel *chan = NULL;
    char *chan_name = NULL;

    char *topic = NULL;

    char *word_save = NULL;
    char *word = strtok_r(buf, " ", &word_save);

    for (int i = 0; word != NULL; i++)
    {
        if (i == 3)
            if ( (chan = get_channel(word)) == NULL )
                create_channel(word, srv);

        if (i > 3)
        {
            chan->topic = realloc(chan->topic, (strlen(chan->topic) + strlen(word)) / sizeof(char));
            if (chan->topic = NULL)
            {
                printf("Failed to add memory on %s\n", __PRETTY_FUNCTION__);
                exit(EXIT_FAILURE);
            }
            sprintf(chan->topic, "%s%s", chan->topic, word);
            printf("Topic: %s\n", chan->topic);
        }
        word = strtok_r(buf, " ", &word_save);

    }

}
