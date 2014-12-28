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

#define BCIRC_PLUGIN_OK 1
#define BCIRC_PLUGIN_STOP 0
#define BCIRC_PLUGIN_FAIL -1

/*
    Params:
        server *srv
*/
#define CALLBACK_SERVER_CONNECTED "server_connected"


/*
    Params:
        server *srv
*/
#define CALLBACK_SERVER_DISCONNECTED "server_disconnected"

/*
    Params:
        char *buffer
        server *srv
*/
#define CALLBACK_SERVER_RECV "server_recv"

/*
    Params:
        char *buffer
        server *srv
*/
#define CALLBACK_SERVER_SEND "server_send"




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

typedef int(*CALLBACK_FUNC)(void**, int);

int register_callback(char *cb_name, CALLBACK_FUNC cb_func, plugin *pluginptr); /* This function should be called in plugin */
void execute_callbacks(char *cb_name, void **args, int argc);


#endif /* PLUGIN_HANDLER_H_ */
