#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>

#include "../headers/plugin_handler.h"
#include "../headers/irc.h"
#include "../headers/server.h"

#define get_str_size(str) ( (strlen( (char*) str) + 1) * sizeof(char) )


int plugin_count;
plugin **plugin_list;

void *execute_function = NULL;

int load_plugin(char *path)
{
    plugin *new_plugin = malloc(sizeof(plugin));
    void *handle = NULL;
    int (*init_func)(plugin*);

    new_plugin->handle = handle;
    new_plugin->callback_count = 0;
    new_plugin->status = RUNNING;

    new_plugin->callback_list = malloc(sizeof(callback));

    dlerror();

    if ((handle = dlopen(path, RTLD_LAZY )) == NULL)
    {
        printf("%s\n", dlerror());
        return -1;
    }

    new_plugin->plugin_name = (char*) dlsym(handle, "plugin_name");
    if (new_plugin->plugin_name == NULL)
    {
        printf("Failed to get name of plugin!\n");
        printf("Path: %s\n", path);
        return -2;
    }

    new_plugin->plugin_author = (char*) dlsym(handle, "plugin_author");
    if (new_plugin->plugin_author == NULL)
    {
        printf("Failed to get author of plugin!\n");
        printf("Path: %s\n", path);
        return -2;
    }

    new_plugin->plugin_version = (char*) dlsym(handle, "plugin_version");
    if (new_plugin->plugin_version == NULL)
    {
        printf("Failed to get version of plugin!\n");
        printf("Path: %s\n", path);
        return -2;
    }

    init_func = dlsym(handle, "plugin_init");
    if (init_func == NULL)
    {
        printf("Failed to load function for initialization!\n");
        printf("Path: %s\n", path);
    }


    int res;
    if ((res = init_func(new_plugin)) != BCIRC_PLUGIN_OK)
    {
        printf("Plugin %s returned %d!\n", new_plugin->plugin_name, res);
        return -3;
    }

    plugin **new_list = NULL;
    new_list = realloc(plugin_list, (plugin_count + 1) * sizeof(plugin*));
    if (plugin_list == NULL)
    {
        printf("Failed to malloc new_list (%s)\n", __PRETTY_FUNCTION__);
        exit(EXIT_FAILURE);
    }

    new_list[plugin_count] = malloc(sizeof(plugin));
    new_list[plugin_count] = new_plugin;
    plugin_list = new_list;

    plugin_count++;

    printf("Added plugin %s version %s\n", new_plugin->plugin_name, new_plugin->plugin_version);


    return 1;
}


int get_plugins(char *plugin_dir)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(plugin_dir);
    if (d)
    {
        while ( (dir = readdir(d)) != NULL)
        {
            if ( dir->d_type == DT_REG ) //is file or symlink
            {
                if (strlen(dir->d_name) > 3)
                {
                    if (strstr(dir->d_name, ".so") != NULL) //fix me
                    {
                        char *plugin_to_add = malloc( (strlen(plugin_dir) + 1 + strlen(dir->d_name) + 1) * sizeof(char));
                        sprintf(plugin_to_add, "%s/%s", plugin_dir, dir->d_name);
                        load_plugin(plugin_to_add);
                        free(plugin_to_add);
                    }
                }
            }

        }
    }
    else
    {
        closedir(d);
        return -1;
    }
    closedir(d);
    return 1;
}

int pause_plugin(plugin *pluginptr)
{
    if (pluginptr == NULL)
        return -1;
    pluginptr->status = PAUSED;
    return 1;
}

int remove_plugin(plugin *pluginptr)
{


    return 1;
}

int register_callback(char *cb_name, CALLBACK_FUNC cb_func, plugin *pluginptr)
{
    if (pluginptr == NULL)
    {
        printf("pluginptr is null!\n");
        return -1;
    }
    if (cb_name == NULL)
    {
        printf("cb_name is null!\n");
        return -1;
    }
    if (cb_func == NULL)
    {
        printf("cb_func is null!\n");
        return -1;
    }

    callback *new_callback = malloc(sizeof(callback));
    new_callback->cb_name = malloc( (strlen(cb_name) + 1) * sizeof(char) );
    new_callback->cb_func = malloc(sizeof(cb_func));

    callback **new_list = NULL;

    new_list = realloc(pluginptr->callback_list, (pluginptr->callback_count + 1) * sizeof(callback));
    //pluginptr->callback_list = realloc(pluginptr->callback_list, (pluginptr->callback_count + 1) * sizeof(callback));
    if (!new_list)
    {
        printf("Failed to realloc new_list (%s)\n", __PRETTY_FUNCTION__);
        exit(EXIT_FAILURE);
    }
    pluginptr->callback_list = new_list;

    pluginptr->callback_list[pluginptr->callback_count] = malloc(sizeof(callback*));

    new_callback->cb_func = cb_func;
    new_callback->cb_name = cb_name;


    pluginptr->callback_list[pluginptr->callback_count] = new_callback;
    pluginptr->callback_count++;

    printf("Registered callback %s for plugin %s to function_ptr %p\n", cb_name, pluginptr->plugin_name, cb_func);

    return 1;
}


void execute_callbacks(char *cb_name, void **args, int argc)
{
    //printf("cb_name: %s\n", cb_name);
    for (int i = 0; i < plugin_count; i++)
    {
        for (int y = 0; y < plugin_list[i]->callback_count; y++)
        {
            if (plugin_list[i]->status == RUNNING)
            {
                if ( strcmp( plugin_list[i]->callback_list[y]->cb_name, cb_name) == 0 )
                {
                    int (*cb)(void **, int) = plugin_list[i]->callback_list[y]->cb_func;

                    int res;
                    if ( ((res = cb(args, argc)) != BCIRC_PLUGIN_OK))
                    {
                        if (res == BCIRC_PLUGIN_STOP)
                            pause_plugin(plugin_list[i]);

                        else if (res == BCIRC_PLUGIN_FAIL)
                            remove_plugin(plugin_list[i]);

                        else if (res == BCIRC_PLUGIN_BREAK)
                            break;

                        else if (res == BCIRC_PLUGIN_CONTINUE)
                            continue;
                    }
                }
            }
        }
    }
    return;
}
