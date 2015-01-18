#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>


#include "../include/irc.h"
#include "../include/plugin-handler.h"


int get_numeric(void **params, int argc) // FIX THIS
{
    printf("asd??\n");

    char *buffer = malloc( sizeof( (char*) params[0]) );
    server *srv = malloc( sizeof( (server*) params[1]) );

    buffer = (char*) params[0];
    srv = (server*) params[1];

    if (buffer == NULL)
        return BCIRC_PLUGIN_STOP;
    if (srv == NULL)
        return BCIRC_PLUGIN_STOP;

    char *nums = malloc(sizeof(char) * 4);
    char *tokked = strtok(buffer, " ");

    int i = 0;
    while (tokked != NULL)
    {
        if (i == 0)
            continue;
        printf("%s\n", tokked);
        if (isdigit(tokked))
        {
            break;
        }
        return BCIRC_PLUGIN_CONTINUE;
    }

    printf("numeric: %s\n", tokked);

    int *numeric = (int*) atoi(nums); //TODO: fix warning

    void **cb_params = malloc(sizeof(void*) * 3);
    if (cb_params == NULL)
        return BCIRC_PLUGIN_CONTINUE;

    cb_params[0] = (void*) numeric;
	cb_params[1] = (void*) buffer;
    cb_params[2] = (void*) srv;

    execute_callbacks(CALLBACK_GOT_NUMERIC, cb_params, 3);

    return BCIRC_PLUGIN_OK;
}
