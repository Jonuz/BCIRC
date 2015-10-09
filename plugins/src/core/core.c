#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <libconfig.h>

#include "../headers/irc.h"
#include "../headers/server.h"
#include "../headers/numeric.h"
#include "../headers/irc_cmds.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"


int handle_ping(void **params, int argc);
int handle_registeration(void **params, int argc);
int got_in(void **params, int argc);
void autojoin_channels();


char plugin_name[] = "BCIRC-Core plugin";
char plugin_author[] = "Joona";
char plugin_version[] = "0.1";

int plugin_init(plugin *pluginptr)
{
    register_callback(CALLBACK_GOT_NUMERIC, got_in, 5, pluginptr);
    register_callback(CALLBACK_SERVER_RECV, handle_ping, 5, pluginptr);
    register_callback(CALLBACK_SERVER_CONNECTED, handle_registeration, 5, pluginptr);

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
		printf("Failed to load config!\n");
		printf("%d\n%s\n", config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);

		return;
	}

    config_setting_t *servers_setting;
    if (!(servers_setting = config_lookup(&cfg, "servers")))
	{
		printf("Failed to load setting \"servers\"(%s).\n", config);
		return;
	}

    unsigned int server_count = config_setting_length(servers_setting);

    for (int i = 0; i < server_count; i++)
    {
        config_setting_t *srv_setting = config_setting_get_elem(servers_setting, i);

        config_setting_t *chans_setting;

        if (!(chans_setting = config_lookup(&srv_setting, "channels")))
        {
            printf("No channels found.\n");
            return;
        }
        unsigned int channel_count = config_setting_length(chans_setting);

        for (int y = 0; y < channel_count; y++)
        {
            config_setting_t *chan_setting = config_setting_get_elem(chans_setting, y);

            const char *chan_str;
            char *key_str;

            config_setting_lookup_string(chan_setting, "chan_name", &chan_str);

            if (!config_setting_lookup_string(chan_setting, "chan_key", &key_str))
                key_str = NULL;

            printf("chan %s\nkey: %s\n", chan_str, key_str);

            join_channel(chan_str, key_str, srv);

        }

    }

}

int got_in(void **params, int argc)
{
    int *numeric = (int*) params[0];
    server *srv = (server*) params[2];


    if (srv->motd_sent == 1)
        return BCIRC_PLUGIN_OK;

    if (*numeric != RPL_ENDOFMOTD)
        return BCIRC_PLUGIN_CONTINUE;

    printf("Connected to %s!\n", srv->host);
    srv->motd_sent = 1;

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

    puts("PONG!");
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

    char key_buf[512];
    char username_buf[512];
    char nickname_buf[512];

    if (srv->pass)
        sprintf(key_buf, "PASS %s\r\n", srv->pass);
    else
        sprintf(key_buf, "PASS %s\r\n", "adasdasda");

    sprintf(username_buf, "USER %s 8 * :%s\r\n", srv->realname, srv->username);
    sprintf(nickname_buf, "NICK %s\r\n", srv->nick);

    server_send(key_buf, srv);
    server_send(username_buf, srv);
    server_send(nickname_buf, srv);

    return BCIRC_PLUGIN_OK;
}
