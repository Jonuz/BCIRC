#include <string.h>
#include <stdio.h>
#include <ctype.h>


#include "../include/irc.h"
#include "../include/plugin-handler.h"


int get_numeric(void **params, int argc) // FIX THIS
{
    char *buffer = malloc( sizeof( (char*) params[0]) / sizeof(char*) );
    server *srv = malloc( sizeof( (server*) params[1]) / sizeof(server*) );

    buffer = (char*) params[0];
    srv = (server*) params[1];

    srv = (server*) params[1];

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

    void **cb_params = malloc(sizeof(server*) + sizeof(char*) + sizeof(int*)); 


    if (cb_params == NULL)
        return BCIRC_PLUGIN_CONTINUE;

    cb_params[0] = (void*) numeric;
	cb_params[1] = (void*) buffer;
    cb_params[2] = (void*) srv;

    execute_callbacks(CALLBACK_GOT_NUMERIC, cb_params, 3);

    return BCIRC_PLUGIN_OK;
}
