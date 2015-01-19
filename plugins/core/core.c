#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>

#include "../../include/irc.h"
#include "../../include/server.h"
#include "../../include/plugin-handler.h"

int handle_ping(void **params, int argc);
int handle_registeration(void **params, int argc);

int test_numeric(void **params, int argc);

char plugin_name[] = "BCIRC-Core plugin";
char plugin_author[] = "Joona";
char plugin_version[] = "0.1";

int plugin_init(plugin *pluginptr)
{
    register_callback(CALLBACK_SERVER_RECV, handle_ping, pluginptr);
    register_callback(CALLBACK_SERVER_CONNECTED, handle_registeration, pluginptr);
	register_callback(CALLBACK_GOT_NUMERIC, test_numeric, pluginptr);

    return BCIRC_PLUGIN_OK;
}


int test_numeric(void **params, int argc)
{
	int *numeric = (int*) params[0];
	printf("%d\n", *numeric);

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

    if (strstr(buf, "PING :") == NULL) //fix me!
    {
        return BCIRC_PLUGIN_CONTINUE;
    }

    int len = strlen(buf) + 1   ;
    char pong[len];
    strcpy(pong, buf);

    pong[1] = 'O';

    server_send(pong, srv);
    printf("%s", pong);

    return BCIRC_PLUGIN_OK;
}


int handle_registeration(void **params, int argc)
{

    if (params[0] == NULL)
        return BCIRC_PLUGIN_BREAK;
    server *srv = (server*) params[0];

    char password_msg[] = "PASS passu\r\n";
    char username_msg[] = "USER quest  asd Testimies :Tosimies\r\n";
    char nickname_msg[] = "NICK dadasd\r\n";

    usleep(10000);

    server_send(password_msg, srv);
    usleep(1000);
    server_send(username_msg, srv);
    usleep(1000);
    server_send(nickname_msg, srv);
    usleep(1000);

    return BCIRC_PLUGIN_OK;
}
