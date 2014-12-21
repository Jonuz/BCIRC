#include <stdio.h>

#include "../include/plugin-handler.h"
#include "../include/server.h"

char plugin_name[] = "Logger";
char plugin_version[] = "0.01";
char plugin_author[] = "Joona";

//int register_callback(char *cb_name, CALLBACK_FUNC cb_func, plugin *pluginptr);

int on_connect(void **params, int argc)
{
    printf("on_connect");
    server *srv = (server*) params[0];
    printf("Connected to %s:%s\n", srv->host, srv->port);
    return BCIRC_PLUGIN_OK;
}


int plugin_init(plugin *pluginptr)
{
    printf("plugin_init()\n");
    register_callback(CALLBACK_SERVER_CONNECTED, on_connect, pluginptr);
    return BCIRC_PLUGIN_OK;
}

/*
int on_server_recv(void **params)
{
    for (int i = 0; i < strlen(buf) + 1; i++)
    {
        if (buf[i] == '\0')
            break;
        if (buf[i] == '\r')
            continue;
        if (buf[i] == '\377')
            continue;
        printf("%c", buf[i]);
    }
    return 1;
}
*/
