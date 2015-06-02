#include <stdio.h>
#include <curl/curl.h>
#include <regex.h>
#include <sys/types.h>

#include "../../include/plugin-handler.h"
#include "../../include/irc.h"
#include "../../include/irc_cmds.h"


#define get_str_size(str) ( (strlen( (char*) str) + 1) * sizeof(char) )

char plugin_name[] = "URL announcer";
char plugin_version[] = "0.01";
char plugin_author[] = "Joona";



int check_for_url(void**, int argc);
int http_request(char *url, server *srv);
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream);

server *srv_save = NULL;
char target_save[100];
char nick_save[100];

int plugin_init(plugin *pluginptr)
{
    puts("title4lyf");
    register_callback(CALLBACK_GOT_PRIVMSG, check_for_url, pluginptr);

    return BCIRC_PLUGIN_OK;
}


int check_for_url(void **params, int argv)
{
    for (int i = 0; i < argv; i++)
        if (params[i] == NULL)
        {
            printf("params[%d] is NULL!\n", i);
            return BCIRC_PLUGIN_FAIL;
        }


    char *nick = malloc(sizeof( get_str_size(params[1]) ));
    char *hostmask = malloc(sizeof(get_str_size(params[2])));
    char *target = malloc(sizeof(get_str_size(params[3])));
    char *msg = malloc(sizeof(get_str_size(params[4])));

    server *srv = malloc(sizeof(server*));
    srv = params[0];

    nick = params[1];
    hostmask = params[2];
    target = params[3];
    msg = params[4];

    regex_t regex;
    int reti;
    regmatch_t matches[1];

    reti = regcomp(&regex, "https?:\/\/[^\ ]+", REG_EXTENDED);
    if (reti != 0)
    {
        printf("Failed to compile regex!\n");
        return BCIRC_PLUGIN_CONTINUE;
    }

    reti = regexec(&regex, msg, 1, matches, 0);
    if (reti != 0)
        return BCIRC_PLUGIN_CONTINUE;

    puts("found url!");

    size_t len = matches[0].rm_eo - matches[0].rm_so + 1;
    char *url = malloc((len) * sizeof(char));

    strncpy(url, msg+matches[0].rm_so, len);
    url[len-1] = '\0';

    printf("url: %s\n", url);
    regfree(&regex);

    if (!srv_save)
        srv_save = malloc(sizeof(server));
    if (!srv_save)
        return BCIRC_PLUGIN_CONTINUE;

    *srv_save = *srv;
    strcpy(target_save, target);
    strcpy(nick_save, nick);


    http_request(url, srv);

    return BCIRC_PLUGIN_OK;
}

int http_request(char *url, server *srv)
{
    CURL *curl = NULL;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();

    if (curl)
    {

        if (strstr(url, "https") == url)
        {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            printf("curl failed: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return BCIRC_PLUGIN_CONTINUE;
        }
    }
    else
    {
        curl_easy_cleanup(curl);
        return BCIRC_PLUGIN_CONTINUE;
    }
    return BCIRC_PLUGIN_OK;
}


size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    regex_t regex;
    int reti;
    regmatch_t matches[1];

    char *response = (char*) malloc((size * nmemb + 1) * sizeof(char));
    strncat(response, (char*) ptr, size * nmemb);

    puts("title!");

    reti = regcomp(&regex, "<title[^>]*>(.*?)</title>", REG_EXTENDED);
    if (reti != 0)
    {
        printf("Failed to compile regex!\n");
        return BCIRC_PLUGIN_CONTINUE;
    }

    reti = regexec(&regex, response, 1, matches, 0);
    if (reti != 0)
    {
        puts("regex didnt match");
        //printf("%s\n", response);
        return BCIRC_PLUGIN_CONTINUE;
    }
    puts("found!");

    size_t len = matches[0].rm_eo - matches[0].rm_so;

    char *title = (char*) malloc((len + 2) * sizeof(char));
    strncpy(title, (char*) response + matches[0].rm_so , len);

    while (title[0] != '>') //till we are on title>
        title++;
    title++;

    while (title[strlen(title) - 1] != '<')
        title[strlen(title)-1] = '\0';
    title[strlen(title)-1] = '\0';

    printf("title: %s\n", title);

    printf("target_save: %s\n", target_save);
    printf("nick_save: %s\n", nick_save);

    if (target_save[0] == '#')
        privmsg(title, target_save, srv_save);
    else
        privmsg(title, nick_save, srv_save);

    srv_save = NULL;

    return size * nmemb;
}
