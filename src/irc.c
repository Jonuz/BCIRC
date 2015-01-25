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

    char *lines_save = NULL;
    char *words_save = NULL;

    char *line = strtok_r(buffer, "\r", &lines_save);
    char *word = NULL;

    for (int line_count = 0; line != NULL; line_count++)
    {
        word = strtok_r(line, " ", &words_save);
        for (int word_count = 0; word != NULL; word_count++)
        {
            if (word_count == 1)
            {
                if (!is_fulldigit(word))
                    return BCIRC_PLUGIN_CONTINUE;

                int numeric = atoi(word);

                void **params = NULL;
                params = malloc(3 * sizeof(void*));

                if (params == NULL)
                {
                    puts("Failed to alloc memory in get_numeric()!");
                    return BCIRC_PLUGIN_FAIL;
                }
                params[0] = (void*) numeric;
                params[1] = (void*) buffer;
                params[2] = (void*) srv;

                execute_callbacks(CALLBACK_GOT_NUMERIC, params, 3);

                continue;
            }
            word = strtok_r(NULL, " ", &words_save);
        }
        line = strtok_r(NULL, "\r", &lines_save);
    }

    return BCIRC_PLUGIN_OK;
}
