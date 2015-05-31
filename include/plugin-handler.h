#ifndef PLUGIN_HANDLER_H_
#define PLUGIN_HANDLER_H_

#include <stdlib.h>
#include <stdio.h>

#include "channel.h"
#include "server.h"

/*
#define BCIRC_PLUGIN_NAME(__name) \
	char plugin_name[] = __name;
*/

/*
#define BCIRC_PLUGIN_VERSION(__version)
	char plugin_version[] = __version;
*/

#define BCIRC_PLUGIN_CONTINUE 3
#define BCIRC_PLUGIN_BREAK 2
#define BCIRC_PLUGIN_OK 1
#define BCIRC_PLUGIN_STOP 0
#define BCIRC_PLUGIN_FAIL -1



typedef enum { RUNNING, PAUSED } PLUGIN_STATUS;


typedef struct
{
    char *cb_name;
    int (*cb_func)(void **, int); //Function to call
} callback;

typedef struct
{
	void *handle;

	char *plugin_name;
	char *plugin_version;
  char *plugin_author;

    PLUGIN_STATUS status;

    callback **callback_list;
    int callback_count;

} plugin;

extern plugin **plugin_list;
extern int plugin_count;


int load_plugin(char *path);
int pause_plugin(plugin *pluginptr);
int remove_plugin(plugin *pluginptr);
int get_plugins(char *dir);

typedef int(*CALLBACK_FUNC)(void**, int);

int register_callback(char *cb_name, CALLBACK_FUNC cb_func, plugin *pluginptr);
int main_register_callback(char *cb_name, CALLBACK_FUNC cb_func); /* Because we want also use this in main program */

void execute_callbacks(char *cb_name, void **args, int argc);


#endif /* PLUGIN_HANDLER_H_ */
