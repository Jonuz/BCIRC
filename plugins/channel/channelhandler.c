#include <stdio.h>

#include "../../include/plugin-handler.h"
#include "../../include/server.h"
#include "../../include/channel.h"
#include "../../include/irc.h"
#include "../../include/irc_cmds.h"



#include "../../include/numeric.h"


char plugin_name[] = "BCIRC-Channel plugin";
char plugin_author[] = "Joona";
char plugin_version[] = "0.1";

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

    int numeric =  (int) params[0];

    char *buf = malloc(get_str_size(params[1]));
    buf = params[1];

    server *srv = malloc(sizeof(server));
    *srv = params[2];

    switch(numeric)
    {
        case RPL_TOPIC:
            set_topic(params);
            break;

        case RPL_ENDOFOMOTD:
            join_channel("#jonuz.test", NULL, srv);
            break;

        case RPL_NAMREPLY:
            //add_users(params);
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

    printf("BUF: %s\n\n", buf);

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
        }
        word = strtok_r(NULL, " ", &word_save);
    }
}
