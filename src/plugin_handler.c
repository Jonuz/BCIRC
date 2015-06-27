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

callback_index **index_list;
int index_count;


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
        free(new_plugin->callback_list);
        free(new_plugin);
        return -1;
    }

    new_plugin->plugin_name = (char*) dlsym(handle, "plugin_name");
    if (new_plugin->plugin_name == NULL)
    {
        printf("Failed to get name of plugin!\n");
        printf("Path: %s\n", path);
        free(new_plugin->callback_list);
        free(new_plugin);
        return -2;
    }

    new_plugin->plugin_author = (char*) dlsym(handle, "plugin_author");
    if (new_plugin->plugin_author == NULL)
    {
        printf("Failed to get author of plugin!\n");
        printf("Path: %s\n", path);
        free(new_plugin->callback_list);
        free(new_plugin);
        return -2;
    }

    new_plugin->plugin_version = (char*) dlsym(handle, "plugin_version");
    if (new_plugin->plugin_version == NULL)
    {
        printf("Failed to get version of plugin!\n");
        printf("Path: %s\n", path);
        free(new_plugin->callback_list);
        free(new_plugin);
        return -2;
    }

    init_func = dlsym(handle, "plugin_init");
    if (init_func == NULL)
    {
        printf("Failed to load function for initialization!\n");
        printf("Path: %s\n", path);
        free(new_plugin->callback_list);
        free(new_plugin);
        return -2;
    }


    int res;
    if ((res = init_func(new_plugin)) != BCIRC_PLUGIN_OK)
    {
        printf("Plugin %s returned %d on init!\n", new_plugin->plugin_name, res);
        free(new_plugin->callback_list);
        free(new_plugin);
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
                    size_t name_len = strlen(dir->d_name);
                    if (strstr(dir->d_name, ".so") == dir->d_name+name_len-3)
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

int register_callback(char *cb_name, CALLBACK_FUNC cb_func, int priority, plugin *pluginptr)
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
    new_callback->priority = priority;


    index_callback(new_callback);

    pluginptr->callback_list[pluginptr->callback_count] = new_callback;
    pluginptr->callback_count++;

    return 1;
}

int init_index()
{
    index_count = 0;
    index_list = malloc(sizeof(callback_index*));
    index_list[0] = NULL;

    return 1;
}


int is_callback_indexed(char *cb_name)
{
    for (int i = 0; i < index_count; i++)
    {
        if (strcmp(index_list[i]->cb_name, cb_name) == 0)
            return i;
    }
    return -1;
}

int compare_index (const void *a, const void *b)
{
    const callback *cb1 = *(callback**) a;
    const callback *cb2 = *(callback**) b;

    //printf("cb1: %d\n", (int*) cb1->priority);

    return (cb1->priority) < (cb2->priority);

}

int index_callback(callback *callback_ptr)
{
    char *cb_name = callback_ptr->cb_name;
    int index_point = is_callback_indexed(cb_name);


    if (index_point >= 0)
    {
        int callbacks_count = index_list[index_point]->cb_count;
        callback **callbacks = index_list[index_point]->callbacks;

        if (callbacks_count > 0)
            callbacks = realloc(callbacks, sizeof(callback*) * (callbacks_count + 1) );
        else
            callbacks = malloc(sizeof(callback*));

        if (!callbacks)
        {
            printf("Failed to realloc callback_list(%s)!\n", __PRETTY_FUNCTION__);
            exit(EXIT_SUCCESS);
        }
        callbacks[callbacks_count] = callback_ptr;
        index_list[index_point]->cb_count++;

        if (callbacks_count > 1)
            qsort(callbacks, callbacks_count + 1, sizeof(callback*), compare_index);

        index_list[index_point]->callbacks = callbacks;

    }
    else
    {
        callback_index *new_index = malloc(sizeof(callback_index));
        if (!new_index)
        {
            printf("Failed to realloc new_index(%s)\n", __PRETTY_FUNCTION__);
            exit(EXIT_SUCCESS);
        }

        new_index->cb_name = cb_name;
        new_index->next_index = NULL;
        new_index->cb_count = 1;
        new_index->callbacks = malloc(sizeof(callback*));
        new_index->callbacks[0] = callback_ptr;

        index_list = realloc(index_list, (index_count + 1) * sizeof(callback_index*));
        if (!index_list)
        {
            printf("Failed to realloc index_list(%s)\n", __PRETTY_FUNCTION__);
            exit(EXIT_SUCCESS);
        }

        index_list[index_count] = new_index;
        index_count++;

    }

    return 1;
}


void execute_callbacks(char *cb_name, void **args, int argc)
{
    //printf("cb_name: %s\n", cb_name);
    int index_point = is_callback_indexed(cb_name);
    int cb_count = index_list[index_point]->cb_count;

    for (int i = 0; i < cb_count; i++)
    {
        int (*cb)(void **, int) = index_list[index_point]->callbacks[i]->cb_func;

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
    return;
}
