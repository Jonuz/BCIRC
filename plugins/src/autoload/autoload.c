#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/inotify.h>

#include "../headers/log.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

void *hook_change();

plugin **plugin_list; //Variables from plugin_handler.h
int plugin_count;

pthread_mutex_t plugins_global_mutex;
pthread_t this_thread;


char plugin_name[] = "Plugin auto(re)load";
char plugin_author[] = "Joona";
char plugin_version[] = "0.1";

int plugin_init(plugin *pluginptr) //TODO: Make this plugin more safer.
{
    int res = pthread_create(&this_thread, NULL, hook_change, NULL);
    if (res)
    {
        bcirc_printf("Failed to spawn new thread\n");
        return BCIRC_PLUGIN_FAIL;
    }

    pthread_detach(this_thread);
    return BCIRC_PLUGIN_OK;
}

void load_new_plugin(char *name)
{
    pthread_mutex_lock(&plugins_global_mutex);

    char *dir_path = getenv("BCIRC_PLUGINS_BIN");
    if (!dir_path)
    {
        bcirc_printf("dir_path is NULL!");
        pthread_detach(this_thread);
    }

    size_t pathlen = strlen(dir_path) + 1 + strlen(name) + 1;
    char path[ pathlen ];
    sprintf(path, "%s/%s", dir_path, name);

    for (int i = 0; i < plugin_count; i++)
    {
        if (strcmp(path, plugin_list[i]->path) == 0)
        {
            bcirc_printf("Removing old plugin..\n");
            pthread_mutex_unlock(&plugins_global_mutex);
            remove_plugin(plugin_list[i]);
            pthread_mutex_lock(&plugins_global_mutex);
        }
    }
    pthread_mutex_unlock(&plugins_global_mutex);
    int res = load_plugin(path);
    if (res == 1)
        bcirc_printf("Automatically loaded plugin %s\n", path);
    else
        bcirc_printf("Failed to load plugin %s(%d)\n", path, res);
}

void *hook_change()
{

    int len, fd, wd, i = 0;
    char buffer[BUF_LEN];

    if ( (fd = inotify_init()) < 0 )
    {
        bcirc_printf("Failed to initalize inotify\n");
        pthread_exit(&this_thread);
    }

    wd = inotify_add_watch(fd, getenv("BCIRC_PLUGINS_BIN"), IN_CLOSE_WRITE | IN_MOVED_TO);
    len = read(fd, buffer, BUF_LEN);


    if (len < 0)
    {
        bcirc_printf("read fails");
        perror("read");
        pthread_exit(&this_thread);
    }

    while ( i < len )
    {
        struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];

        if (event->mask & IN_ISDIR)
            break;

        if ( event->mask & IN_MODIFY )
            break;

        if (event->len)
        {
            bcirc_printf("New file: %s\n", event->name);
            load_new_plugin(event->name);
            break;
        }
        i += EVENT_SIZE + event->len;
    }
    inotify_rm_watch( fd, wd );
    close( fd );
    hook_change();
}
