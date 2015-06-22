#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "./headers/irc.h"
#include "./headers/server.h"
#include "./headers/channel.h"
#include "./headers/user.h"
#include "./headers/plugin_handler.h"

irc_base base_client;
server server_info;
channel channel_info;
user user_info;

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

    char *this_path = malloc(100 * sizeof(char));
    this_path = getcwd(NULL, 100);
    printf("path: %s\n", this_path);
    char *plugin_path = malloc( (strlen(this_path) + 1 + 5 + 1 + 7 + 1) * sizeof(char));

    sprintf(plugin_path, "%s/build/plugins", this_path);

    free(this_path);

    get_plugins(plugin_path);
    free(plugin_path);

    int res;
    if ( (res = server_connect(&server_info) ) != 1)
    {
        printf("Cant connect to %s\n", server_info.host);
        return -1;
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
