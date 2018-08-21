#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "./headers/irc.h"
#include "./headers/server.h"
#include "./headers/channel.h"
#include "./headers/plugin_handler.h"
#include "./headers/callback_defines.h"

//Global variables
int server_count;
int channel_count;
pthread_mutex_t servers_global_mutex;

int res;

void get_input()
{
	char input[1024];
	fgets(input, 1024, stdin);

	char *input_copy = malloc(strlen(input)+1);
	strcpy(input_copy, input);

	void **params;
	params = malloc(sizeof(void*));
	params[0] = input_copy;

	execute_callbacks(CALLBACK_GOT_INPUT, params, 1);

	free(params);
	free(input_copy);

	get_input();
}

int main()
{
	channel_count = 0;
	server_count = 0;

	pthread_mutex_init(&servers_global_mutex, NULL);

	plugin_list = malloc(sizeof(plugin*));
	init_index();

	char *plugin_dir = getenv("BCIRC_PLUGINS_BIN");
	if (!plugin_dir)
	{
		printf("Enviroment value \"PLUGINS_BIN\" doesnt exist.\n");
		exit(EXIT_SUCCESS);
	}
	get_plugins(plugin_dir);

	char *configdir = getenv("BCIRC_CONFIG_DIR");
	char *serverfile = malloc( (strlen(configdir) + 13 + 1) * sizeof(char) );
	sprintf(serverfile, "%s/bcirc.conf", configdir);
	load_servers(serverfile);
	free(serverfile);


	get_input();

	return 0;
}
