#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>

#include "../../include/irc.h"
#include "../../include/irc_cmds.h"
#include "../../include/server.h"
#include "../../include/plugin-handler.h"
#include "../../include/numeric.h"

int handle_ping(void **params, int argc);
int handle_registeration(void **params, int argc);
int got_in(void **params, int argc);

int test_numeric(void **params, int argc);

char plugin_name[] = "BCIRC-Core plugin";
char plugin_author[] = "Joona";
char plugin_version[] = "0.1";

int plugin_init(plugin *pluginptr)
{
    register_callback(CALLBACK_SERVER_RECV, handle_ping, pluginptr);
    register_callback(CALLBACK_SERVER_CONNECTED, handle_registeration, pluginptr);
    register_callback(CALLBACK_GOT_NUMERIC, got_in, pluginptr);

    return BCIRC_PLUGIN_OK;
}



int handle_ping(void **params, int argc)
{
    server *srv = (server*) params[0];
    char *buf = (char*) params[1];

    if ((buf == NULL) || (srv == NULL))
        return BCIRC_PLUGIN_BREAK;


    if (strlen(buf) < 7)
    {
        return BCIRC_PLUGIN_CONTINUE;
    }

    char *tmp = malloc( (strlen(buf) + 1) * sizeof(char));
    strcpy(tmp, buf);
    char *maybe_ping = malloc( (strlen(tmp) + 1) * sizeof(char));
    maybe_ping = strtok(tmp, " ");

    if (strcmp(maybe_ping, "PING") != 0)
        return BCIRC_PLUGIN_CONTINUE;

    char *pong = malloc( ((strlen(buf) + 2 + 1) * sizeof(char)) );
    sprintf(pong, "%s\r\n", buf);

    pong[1] = 'O';

    puts("PONG!");
    server_send(pong, srv);

    return BCIRC_PLUGIN_OK;
}


int handle_registeration(void **params, int argc)
{

    if (params[0] == NULL)
        return BCIRC_PLUGIN_BREAK;
    server *srv = (server*) params[0];

    char password_msg[] = "PASS passu\r\n";
    char username_msg[] = "USER quest dsd Tositestimies :Tosimies\r\n";
    char nickname_msg[] = "NICK dadasd\r\n";

    usleep(100);

    server_send(password_msg, srv);
    usleep(1000);
    server_send(username_msg, srv);
    usleep(1000);
    server_send(nickname_msg, srv);
    usleep(1000);

    return BCIRC_PLUGIN_OK;
}

int got_in(void **params, int argc)
{
    int numeric = (int) &params[0];
    server *srv = malloc(sizeof(server));
    srv = (server*) params[2];

    if (numeric != RPL_ENDOFMOTD)
        return BCIRC_PLUGIN_CONTINUE;

    printf("Connected to %s!\n", srv->host);

    return BCIRC_PLUGIN_OK;
}
