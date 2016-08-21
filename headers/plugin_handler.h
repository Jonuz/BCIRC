#ifndef PLUGIN_HANDLER_H_
#define PLUGIN_HANDLER_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

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
	int priority;
} callback;

typedef struct
{
	void *handle;

	char *plugin_name;
	char *plugin_version;
	char *plugin_author;

	char *path;

	PLUGIN_STATUS status;

	callback **callback_list;
	int callback_count;

} plugin;

typedef struct
{
	char *cb_name;
	callback **callbacks;
	size_t cb_count;

	void *next_index;
} callback_index;

extern callback_index **index_list;
extern int index_count;

extern plugin **plugin_list;
extern int plugin_count;

extern pthread_mutex_t plugins_global_mutex;

int get_plugins(char *dir);
int load_plugin(char *path);
int pause_plugin(plugin *pluginptr);
int remove_plugin(plugin *pluginptr);

typedef int(*CALLBACK_FUNC)(void**, int);

int register_callback(char *cb_name, CALLBACK_FUNC cb_func, int priority, plugin *pluginptr);
int execute_callbacks(char *cb_name, void **args, int argc);

// Returns paramameter "index" when it's null, -1 when none of params are null.
int callback_params_null_check(void **params, int argc);

int init_index();
int index_callback(callback *callback_ptr);
int get_index_count();
int get_cb_index(char *cb_name);
int remove_index(callback *cb_ptr);

#endif /* PLUGIN_HANDLER_H_ */
