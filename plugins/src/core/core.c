#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <libconfig.h>
#include <pthread.h>

#include "../headers/log.h"
#include "../headers/irc.h"
#include "../headers/server.h"
#include "../headers/numeric.h"
#include "../headers/irc_cmds.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"

#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

int handle_ping(void **params, int argc);
int handle_registeration(void **params, int argc);
int got_in(void **params, int argc);
void autojoin_channels();
int server_rejoin(void **params, int);


int handle_nick(void **params, int argc); //This function handles usage alternative nick.

char plugin_name[] = "BCIRC-Core plugin";
char plugin_author[] = "Joona";
char plugin_version[] = "0.1";

int plugin_init(plugin *pluginptr)
{
    register_callback(CALLBACK_GOT_NUMERIC, got_in, 5, pluginptr);
    register_callback(CALLBACK_SERVER_RECV, handle_ping, 5, pluginptr);
    register_callback(CALLBACK_SERVER_CONNECTED, handle_registeration, 5, pluginptr);
    register_callback(CALLBACK_GOT_NUMERIC, handle_nick, 5, pluginptr);
    register_callback(CALLBACK_SERVER_DISCONNECTED, server_rejoin, 5, pluginptr);

    return BCIRC_PLUGIN_OK;
}


void autojoin_channels(server *srv)
{
    config_t cfg;
	config_init(&cfg);

    char config[256];
    sprintf(config, "%s/servers.conf", getenv("BCIRC_CONFIG_DIR"));

    if (!config_read_file(&cfg, config))
	{
		bcirc_printf("Failed to load config!\n");
		bcirc_printf("%d\n%s\n", config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);

		return;
	}
    config_setting_t *servers = NULL;
    servers = config_lookup(&cfg, "servers");

    if (!servers)
    {
        bcirc_printf("Failed to load servers from config file!\n");
        return;
    }
    int servers_count = config_setting_length(servers);

    for (int i = 0; i < servers_count; i++)
    {
        config_setting_t *this_server = config_setting_get_elem(servers, i);

        char *network_name = NULL;
        config_setting_lookup_string(this_server, "network_name", &network_name);

        if (strcmp(srv->network_name, network_name) != 0)
            continue;

        config_setting_t *channels = config_setting_lookup(this_server, "channels");
        if (!channels)
        {
            bcirc_printf("Failed to load channels(no channels?)\n");
            continue;
        }

        size_t channels_count = config_setting_length(channels);

        for (int y = 0; y < channels_count; y++)
        {
            config_setting_t *this_channel = config_setting_get_elem(channels, y);

            char *channel_to_join = NULL;
            char *channel_key = NULL;

            if (!(config_setting_lookup_string(this_channel, "chan_name", &channel_to_join)))
            {
                bcirc_printf("Failed to load chan_name\n");
                continue;
            }

            if (!(config_setting_lookup_string(this_channel, "chan_key", &channel_key)))
                channel_key = NULL;

            join_channel(channel_to_join, channel_key, srv);
        }
    }
}

int got_in(void **params, int argc)
{
    server *srv = (server*) params[0];
    int *numeric = (int*) params[1];

    if (srv->motd_sent == 1)
        return BCIRC_PLUGIN_OK;

    if (*numeric != RPL_ENDOFMOTD)
        return BCIRC_PLUGIN_CONTINUE;

    bcirc_printf("Connected to %s!\n", srv->host);
    srv->motd_sent = 1;
    srv->rejoin_tries = 0;

    autojoin_channels(srv);

    return BCIRC_PLUGIN_OK;
}



int handle_ping(void **params, int argc)
{
    server *srv = (server*) params[0];
    char *buf = (char*) params[1];

    if ((buf == NULL) || (srv == NULL))
        return BCIRC_PLUGIN_CONTINUE;


    if (strlen(buf) < 7)
    {
        return BCIRC_PLUGIN_CONTINUE;
    }

    char *tmp = malloc( (strlen(buf) + 1) * sizeof(char));
    strcpy(tmp, buf);

    char *maybe_ping;
    maybe_ping = strtok(tmp, " ");

    if (strcmp(maybe_ping, "PING") != 0)
    {
        free(tmp);
        return BCIRC_PLUGIN_CONTINUE;
    }

    char *pong = malloc( ((strlen(buf) + 2 + 1) * sizeof(char)) );
    sprintf(pong, "%s\r\n", buf);

    pong[1] = 'O';

    bcirc_printf("PONG!\n");
    server_send(pong, srv);

    free(pong);
    free(tmp);

    return BCIRC_PLUGIN_OK;
}


int handle_registeration(void **params, int argc)
{

    if (params[0] == NULL)
        return BCIRC_PLUGIN_BREAK;

    server *srv = (server*) params[0];
    time(&srv->time_connected);

    char key_buf[512];
    char username_buf[512];

    if (!srv)
    {
        bcirc_printf("srv is null!\n");
        return BCIRC_PLUGIN_CONTINUE;
    }

    if (srv->pass)
        sprintf(key_buf, "PASS %s\r\n", srv->pass);
    else
        sprintf(key_buf, "PASS %s\r\n", "adasdasda");

    sprintf(username_buf, "USER %s 8 * :%s\r\n", srv->realname, srv->username);

    server_send(key_buf, srv);
    server_send(username_buf, srv);
    nick(srv->nick, srv);

    return BCIRC_PLUGIN_OK;
}

int handle_nick(void **params, int argc)
{
    server *srv = (server*) params[0];
    int *numeric = (int*) params[1];
    char buf[strlen(params[2])+1];
    strcpy(buf, params[2]);


    int numerics[]= { ERR_NONICKNAMEGIVEN, ERR_ERRONEUSNICKNAME , ERR_NICKNAMEINUSE, ERR_NICKCOLLISION, ERR_UNAVAILRESOURCE };
    size_t numerics_count = sizeof(numerics) / sizeof(int);

    for (int i = 0;i < numerics_count; i++)
    {
        if (*numeric == numerics[i])
            break;
        if (i == numerics_count-1)
            return BCIRC_PLUGIN_OK;
    }

    if (strcmp(srv->nick, srv->alt_nick) != 0)
        nick(srv->alt_nick, srv);
    else
    {
        char *new_nick = malloc(strlen(srv->nick + 2));
        strcpy(new_nick, srv->nick);
        strcat(new_nick, "_");

        nick(new_nick, srv);
        free(new_nick);
    }

    return BCIRC_PLUGIN_OK;
}


void *try_rejoin(void *srv_void)
{
    server *srv = srv_void;

    time_t timenow;
    time(&timenow);

    #define WAIT_TIME 30

    time_t timediff = timenow - srv->time_connected;
    if ((timediff < WAIT_TIME) && (srv->rejoin_tries != 0))
    {
        size_t sleeptime = WAIT_TIME * (srv->rejoin_tries + 1);
        printf("Sleeping for %d seconds before trying rejoin to %s.\n", sleeptime, srv->host);
        sleep(sleeptime);
    }
    srv->rejoin_tries++;

    int res;
    while((res = server_connect(srv)) != 1337) //i had to
    {
        if (res == 1)
            break;
        sleep(15);
    }
    bcirc_printf("Reopened connection to %s\n", srv->host);
    add_to_serverpool(srv);

    return NULL;
}

int server_rejoin(void **params, int argc)
{
    server *srv = params[0];
    int *reason = (int*) params[1];

    if ((!reason) || (!srv))
    {
        return -1;
    }

    bcirc_printf("Trying to rejoin to server %s\n", srv->host);

    if (*reason == SERVER_INTENTIONAL_DC)
        return BCIRC_PLUGIN_OK;

    pthread_t thread;

    if (pthread_create(&thread, NULL, try_rejoin, (void*) srv) == 0)
    {
        return BCIRC_PLUGIN_OK;
    }
    else
    {
        bcirc_printf("Failed to create thread");
        return BCIRC_PLUGIN_FAIL;
    }

    return BCIRC_PLUGIN_OK;
}
