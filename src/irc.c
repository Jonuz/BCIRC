#include <string.h>
#include <stdio.h>
#include <ctype.h>


#include "../include/irc.h"
#include "../include/plugin-handler.h"


int privmsg(char *msg, char *target, server *srv)
{
	char *buf = "PRIVMSG %s: %s\r\n";
	sprintf(buf, target, msg);
	int res = server_send(buf, srv);

	return res;
}

int get_numeric(void **params, int argc)
{
    char *buffer = (char*) params[0];
    server *srv = (server*) params[1];

    if (buffer == NULL)
        return BCIRC_PLUGIN_STOP;
    if (srv == NULL)
        return BCIRC_PLUGIN_STOP;

    if (strlen(buffer) < 3)
    {
        for (int i = 0; i < 3; i++)
        {
            if (!isdigit(buffer[i]))
                return BCIRC_PLUGIN_CONTINUE;
        }
    }
    else
    {
        return BCIRC_PLUGIN_BREAK;
    }

    char nums[4];
    memcpy(nums, buffer, 3);
    nums[3] = '\0';

    int *numeric = (int*) atoi(nums); //TODO: fix warning

    void **cb_params = malloc(sizeof(server*));
    cb_params = realloc(cb_params, sizeof(cb_params) + sizeof(int*));

    if (cb_params == NULL)
        return BCIRC_PLUGIN_CONTINUE;

    cb_params[0] = (void*) numeric;
    cb_params[1] = (void*) srv;

    execute_callbacks(CALLBACK_GOT_NUMERIC, cb_params, 2);

    return BCIRC_PLUGIN_OK;
}
