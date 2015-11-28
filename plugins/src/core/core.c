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

int handle_ping(void **params);
int handle_registeration(void **params);
int got_in(void **params);
void autojoin_channels();
int server_rejoin(void **params);


int handle_nick(void **params); //This function handles usage alternative nick.

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

    config_setting_t *servers_setting;
    if (!(servers_setting = config_lookup(&cfg, "servers")))
	{
		bcirc_printf("Failed to load setting \"servers\"(%s).\n", config);
		return;
	}

    unsigned int server_count = config_setting_length(servers_setting);

    for (int i = 0; i < server_count; i++)
    {
        config_setting_t *srv_setting = config_setting_get_elem(servers_setting, i);

        config_setting_t *chans_setting;

        if (!(chans_setting = config_lookup(&srv_setting, "channels")))
        {
            bcirc_printf("No channels found.\n");
            return;
        }
        unsigned int channel_count = config_setting_length(chans_setting);

        for (int y = 0; y < channel_count; y++)
        {
            config_setting_t *chan_setting = config_setting_get_elem(chans_setting, y);

            char *chan_str;
            char *key_str;

            config_setting_lookup_string(chan_setting, "chan_name", &chan_str);

            if (!config_setting_lookup_string(chan_setting, "chan_key", &key_str))
                key_str = NULL;

            bcirc_printf("chan %s\nkey: %s\n", chan_str, key_str);

            join_channel(chan_str, key_str, srv);

        }

    }

}

int got_in(void **params)
{
    server *srv = (server*) params[0];
    int *numeric = (int*) params[1];

    if (srv->motd_sent == 1)
        return BCIRC_PLUGIN_OK;

    if (*numeric != RPL_ENDOFMOTD)
        return BCIRC_PLUGIN_CONTINUE;

    bcirc_printf("Connected to %s!\n", srv->host);
    srv->motd_sent = 1;

    autojoin_channels(srv);

    return BCIRC_PLUGIN_OK;
}



int handle_ping(void **params)
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


int handle_registeration(void **params)
{

    if (params[0] == NULL)
        return BCIRC_PLUGIN_BREAK;

    server *srv = (server*) params[0];

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

int handle_nick(void **params)
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
    int res;
    while((res = server_connect(srv)) != 1337) //i had to
    {
        if (res == 1)
            break;
        sleep(15);
    }
    bcirc_printf("Reconnected to %s\n", srv->host);


    pthread_exit(&srv->thread);
    add_to_serverpool(srv);

    return NULL;
}

int server_rejoin(void **params)
{
    server *srv = params[0];
    int *reason = (int*) params[1];

    if ((!reason) || (!srv))
    {
        return -1;
    }

    pthread_t thread;
    bcirc_printf("server_rejoin called\n");

    if (*reason == SERVER_INTENTIONAL_DC)
        return BCIRC_PLUGIN_OK;

    if (pthread_create(&thread, NULL, try_rejoin, (void*) srv) == 0)
    {
        try_rejoin( (void*) srv);
    }
    else
    {
        bcirc_printf("Failed to create thread");
        return BCIRC_PLUGIN_FAIL;
    }

    return BCIRC_PLUGIN_OK;
}
