#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>

#include "../headers/irc.h"
#include "../headers/server.h"
#include "../headers/numeric.h"
#include "../headers/irc_cmds.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"


int handle_ping(void **params, int argc);
int handle_registeration(void **params, int argc);
int got_in(void **params, int argc);


char plugin_name[] = "BCIRC-Core plugin";
char plugin_author[] = "Joona";
char plugin_version[] = "0.1";

int plugin_init(plugin *pluginptr)
{
    register_callback(CALLBACK_GOT_NUMERIC, got_in, pluginptr);
    register_callback(CALLBACK_SERVER_RECV, handle_ping, pluginptr);
    register_callback(CALLBACK_SERVER_CONNECTED, handle_registeration, pluginptr);

    return BCIRC_PLUGIN_OK;
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

    join_channel("#tesm", NULL, srv );


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
        return BCIRC_PLUGIN_CONTINUE;

    char *pong = malloc( ((strlen(buf) + 2 + 1) * sizeof(char)) );
    sprintf(pong, "%s\r\n", buf);

    pong[1] = 'O';

    puts("PONG!");
    server_send(pong, srv);

    free(tmp);

    return BCIRC_PLUGIN_OK;
}


int handle_registeration(void **params, int argc)
{

    if (params[0] == NULL)
        return BCIRC_PLUGIN_BREAK;

    server *srv = (server*) params[0];

    char password_msg[] = "PASS passu\r\n";
    char username_msg[] = "USER quest dsd Tositestimies :Tosimies\r\n";
    char nickname_msg[] = "NICK tosibotti\r\n";


    server_send(password_msg, srv);
    server_send(username_msg, srv);
    server_send(nickname_msg, srv);

    return BCIRC_PLUGIN_OK;
}
