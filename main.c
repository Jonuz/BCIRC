#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "./include/irc.h"
#include "./include/server.h"
#include "./include/channel.h"
#include "./include/user.h"
#include "./include/plugin-handler.h"

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

    get_plugins("/home/joona/Ohjelmointi/Irc/build/plugins");

    int res;
    if ( (res = server_connect(&server_info) ) != 1)
    {
        printf("Cant connect to %s\n", server_info.host);
        return -1;
    }

    char *buffer = malloc(0); // So realloc() can be used.

    if (main_register_callback(CALLBACK_SERVER_RECV, get_numeric) != 1)
    {
        printf("Failed to add main_cb\n");
    }

    while(server_recv(buffer, &server_info) >= 1)
    {
    }

	puts("Connection closed");

	return 0;
}
