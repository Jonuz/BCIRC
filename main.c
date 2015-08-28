#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "./headers/irc.h"
#include "./headers/server.h"
#include "./headers/channel.h"
#include "./headers/plugin_handler.h"

irc_base base_client;
server server_info;
channel channel_info;

char host[] = "dreamhack.se.quakenet.org";
char port[] = "6667";
int res;

int main()
{
    server_info.host = malloc(strlen(host) + 1 * sizeof(char));
    server_info.port = malloc(strlen(port) + 1 * sizeof(char));

    strcpy(server_info.host, host);
    strcpy(server_info.port, port);

    plugin_list = malloc(sizeof(plugin*));
    init_index();

    char *plugin_dir = getenv("PLUGINS_BIN");
    if (!plugin_dir)
    {
        printf("Enviroment value \"PLUGINS_BIN\" doesnt exixst\n");
        exit(EXIT_SUCCESS);
    }
    get_plugins(plugin_dir);

    int res;
    if ( (res = server_connect(&server_info) ) != 1)
    {
        printf("Cant connect to %s\n", server_info.host);
        exit(EXIT_SUCCESS);
    }

    char *buffer = NULL;
    while(server_recv(buffer, &server_info) >= 1)
    {
        if (buffer)
            free(buffer);
    }

    puts("Connection closed");

	return 0;
}
