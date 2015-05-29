#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>

#include "../include/plugin-handler.h"
#include "../include/irc.h"
#include "../include/server.h"

int plugin_count;
plugin **plugin_list;

int load_plugin(char *path)
{
    plugin *new_plugin = malloc(sizeof(plugin));
    void *handle = NULL;
    int (*init_func)(plugin*);

    new_plugin->handle = handle;
    new_plugin->callback_count = 0;
    new_plugin->status = RUNNING;

    new_plugin->callback_list = malloc(sizeof(callback*));

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

    plugin_list = realloc(plugin_list, plugin_count * sizeof(plugin*) );
    if (plugin_list == NULL)
    {
        return -4;
    }

    plugin_list[plugin_count] = new_plugin;
    plugin_count++;

    printf("oink?\n");

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
                        char *plugin_to_add = malloc(strlen(plugin_dir) + 1 + strlen(dir->d_name) + 1 * sizeof(char));
                        sprintf(plugin_to_add, "%s/%s", plugin_dir, dir->d_name);
                        load_plugin(plugin_to_add);
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
    plugin **new_list = NULL;
    new_list = malloc(0);
    int new_count = 0;

    for (int i = 0; i < plugin_count; i++)
    {
        if (plugin_list[i] != pluginptr)
        {
            new_list = realloc(new_list, new_count * (sizeof(plugin) / sizeof (new_list)));
            new_list[new_count] = plugin_list[i];
            new_count++;
        }
    }
    plugin_list = new_list;
    plugin_count = new_count;

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

    callback *new_callback = malloc(sizeof(callback*));
    new_callback->cb_name = malloc(sizeof(cb_name));

    new_callback->cb_func = cb_func;
    new_callback->cb_name = cb_name;
    //pluginptr->callback_list = realloc(pluginptr->callback_list, pluginptr->callback_count * sizeof(callback*));

    pluginptr->callback_list[pluginptr->callback_count] = new_callback;
    pluginptr->callback_count++;

    return 1;
}

int main_register_callback(char *cb_name, CALLBACK_FUNC cb_func)
{
    /*
     Since register_callback() wants plugin as parameter but we would also like to use
     our awesome callback system in main program we do this "plugin" and give it as parameter.
    */
    static plugin *cb_plugin = NULL;

    if (cb_plugin == NULL)
    {
        cb_plugin = malloc(sizeof(plugin));
        cb_plugin->callback_list = malloc(sizeof(callback*));

        cb_plugin->callback_count = 0;
        cb_plugin->handle = NULL;
        cb_plugin->status = RUNNING;

        cb_plugin->plugin_name = "CB-Handler";
        cb_plugin->plugin_version = "Joona";
        cb_plugin->plugin_version = "0";

        plugin **new_list = NULL;

        new_list = realloc(plugin_list, plugin_count + 1 * sizeof(plugin*));
        if (new_list == NULL)
        {
            printf("Failed to reserve memory in main_register_callback !\n");
            return -1;
        }
        new_list[plugin_count] = cb_plugin;
        plugin_list = new_list;

        plugin_count++;
    }

    return register_callback(cb_name, cb_func, cb_plugin);
}

void execute_callbacks(char *cb_name, void **args, int argc)
{
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
