#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "./headers/irc.h"
#include "./headers/server.h"
#include "./headers/channel.h"
#include "./headers/plugin_handler.h"


pthread_t **server_threads;
int server_count;

int channel_count;

char host[] = "dreamhack.se.quakenet.org";
char port[] = "6667";
int res;

int main()
{
	channel_count = 0;

	server *srv = malloc(sizeof(server));

    srv->host = malloc(strlen(host) + 1 * sizeof(char));
    srv->port = malloc(strlen(port) + 1 * sizeof(char));

	pthread_mutex_init(&srv->mutex, NULL);

    strcpy(srv->host, host);
    strcpy(srv->port, port);

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
    if ( (res = server_connect(srv) ) != 1)
    {
        printf("Cant connect to %s\n", srv->host);
        exit(EXIT_SUCCESS);
    }

	add_to_serverpool(srv);

	while(1)
	{}


/*    char *buffer = NULL;
    while(server_recv(buffer, &server_info) >= 1)
    {
        if (buffer)
            free(buffer);
    }
*/
    puts("Connection closed");

	return 0;
}
