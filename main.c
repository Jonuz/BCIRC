#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "./headers/irc.h"
#include "./headers/server.h"
#include "./headers/channel.h"
#include "./headers/plugin_handler.h"


int server_count;
int channel_count;

char host[] = "dreamhack.se.quakenet.org";
char port[] = "6667";
int res;

void get_input()
{
	char asd[50];
	fgets(asd, 50, stdin);
	get_input();
}

int main()
{
	channel_count = 0;
	server_count = 0;

    plugin_list = malloc(sizeof(plugin*));
    init_index();

    char *plugin_dir = getenv("BCIRC_PLUGINS_BIN");
    if (!plugin_dir)
    {
        printf("Enviroment value \"PLUGINS_BIN\" doesnt exixst\n");
        exit(EXIT_SUCCESS);
    }
    get_plugins(plugin_dir);

	char *configdir = getenv("BCIRC_CONFIG_DIR");
	char *serverfile = malloc( (strlen(configdir) + 13 + 1) * sizeof(char) );

	sprintf(serverfile, "%s/servers.conf", configdir);
	load_servers(serverfile);
	free(serverfile);


	get_input();

	return 0;
}
