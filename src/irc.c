#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>


#include "../include/irc.h"
#include "../include/plugin-handler.h"

bool is_fulldigit(char *str)
{
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (!isdigit(str[i]))
            return false;
    }
    return true;
}

int get_numeric(void **params, int argc) // FIX THIS
{

    char *buffer = malloc( sizeof( (char*) params[0]) );
    server *srv = malloc( sizeof( (server*) params[1]) );

    srv = (server*) params[0];
    buffer = (char*) params[1];

    if (buffer == NULL)
        return BCIRC_PLUGIN_STOP;
    if (srv == NULL)
        return BCIRC_PLUGIN_STOP;

    char *nums = malloc(sizeof(char) * 4);

    size_t i = 0;
    char *tokked = strtok(buffer, " ");
    while (tokked != NULL)
    {
        for (size_t y = 0; y < strlen(tokked); y++)
            if (is_fulldigit(tokked))
                printf("tokked: %s & i :%d\n", tokked, i);


        tokked = strtok(NULL, " ");
        i++;
    }

    return BCIRC_PLUGIN_CONTINUE;
    printf("\n\n numeric: %s \n\n", tokked);

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
