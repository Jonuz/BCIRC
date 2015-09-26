#include <stdio.h>
#include <curl/curl.h>
#include <regex.h>
#include <sys/types.h>

#include  "entities.h"

#include "../headers/irc.h"
#include "../headers/server.h"
#include "../headers/irc_cmds.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"


#define get_str_size(str) ( (strlen( (char*) str) + 1) * sizeof(char) )

char plugin_name[] = "Title announcer";
char plugin_version[] = "0.01";
char plugin_author[] = "Joona";


int add_url(void **, int);


int check_for_url(void**, int argc);
int http_request(char *url, server *srv);
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream);

server *srv_save = NULL;
char target_save[100];
char nick_save[100];

int has_filter = 0; // 1 if true

int plugin_init(plugin *pluginptr)
{
    has_filter = get_urls();

    register_callback(CALLBACK_GOT_PRIVMSG, check_for_url, 20, pluginptr);
    register_callback(CALLBACK_GOT_PRIVMSG, add_url, 20, pluginptr);

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

    server *srv = params[0];
    char *nick = params[1];
    char *hostmask = params[2];
    char *target = params[3];
    char *msg = params[4];

    //puts("Check for url called!");

    regex_t regex;
    int reti;
    regmatch_t matches[1];

    reti = regcomp(&regex, "https?:\/\/[^\ ]+", REG_EXTENDED);
    if (reti != 0)
    {
        printf("Failed to compile regex!\n");
        regfree(&regex);
        return BCIRC_PLUGIN_FAIL;
    }

    reti = regexec(&regex, msg, 1, matches, 0);
    regfree(&regex);
    if (reti != 0)
    {
        return BCIRC_PLUGIN_CONTINUE;
    }

    size_t len = matches[0].rm_eo - matches[0].rm_so + 1;
    char *url = malloc((len + 1) * sizeof(char));

    strncpy(url, msg+matches[0].rm_so, len);
    url[len-1] = '\0';

    srv_save = srv;

    strcpy(target_save, target);
    strcpy(nick_save, nick);

    if (has_filter == 1)
        if (check_url(url) == 0)
            return BCIRC_PLUGIN_OK;

    http_request(url, srv);
    free(url);

    return BCIRC_PLUGIN_OK;
}

int http_request(char *url, server *srv)
{
    CURL *curl;
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
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            printf("curl failed: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            curl_global_cleanup();

            return BCIRC_PLUGIN_CONTINUE;
        }
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return BCIRC_PLUGIN_OK;
}


size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    regex_t regex;
    int reti;
    regmatch_t matches[1];
    CURL *curl = stream;

    char *response = (char*) malloc((size * nmemb + 2) * sizeof(char));
    strncpy(response, (char*) ptr, size * nmemb);
    response[size*nmemb] = '\0';

    reti = regcomp(&regex, "<title[^>]*>(.*?)</title>", REG_EXTENDED);
    if (reti != 0)
    {
        printf("Failed to compile regex!\n");
        free(response);
        regfree(&regex);
        return size * nmemb;;
    }

    reti = regexec(&regex, response, 1, matches, 0);
    regfree(&regex);
    if (reti != 0)
    {
        free(response);
        return size * nmemb;
    }

    puts("Title found");

    size_t len = matches[0].rm_eo - matches[0].rm_so;

    char *title = (char*) malloc((len + 2) * sizeof(char));
    strncpy(title, (char*) response + matches[0].rm_so, len);
    title[len] = '\0';

    free(response);

    size_t title_start = 0;
    size_t title_end = strlen(title);

    for (int i = 0; title[i] != '>'; i++) //till we are on the end of <title>
        title_start++;
    title_start++;

    for (int i = strlen(title); title[i - 1] != '<'; i--)
        title_end--;
    title_end--;

    size_t title_len = title_end - title_start;
    char *new_title = malloc((title_len + 1) * sizeof(char));

    strncpy(new_title, title+title_start, title_len );
    new_title[title_len] = '\0';


    title = realloc(title, (strlen(new_title) + 1) * sizeof(char));

    // http://stackoverflow.com/a/1082191/2279808 <3
    decode_html_entities_utf8(title, new_title);
    free(new_title);

    char *unwanted_chars =  "\r\n\t\a\b\f\r\v ";

    int remove_front = 0;
    int remove_back = 0;

    title_len = strlen(title);

    char *ret;
    while(1)
    {
        ret = strpbrk(title, unwanted_chars);
        if (!ret)
            break;

        if ( (title[0 + remove_front]) == (char) *ret)
            remove_front++;
        else if (( title[title_len - remove_back - 1]) == (char) *ret)
            remove_back++;
        else
            break;
    }

    if ((remove_front != 0) || (remove_back != 0))
    {
        char *new_title2 = malloc( (title_len - remove_back - remove_front + 1) * sizeof(char) );
        strncpy(new_title2, title+remove_front, title_len - remove_front - remove_back);
        new_title2[title_len - remove_front - remove_back] = '\0';

        free(title);
        title = new_title2;
    }

    if (target_save[0] == '#') //In future: Check if target is channel.
        add_to_privmsg_queue(title, target_save, srv_save, 0);
    else
        add_to_privmsg_queue(title, nick_save, srv_save, 1);

    srv_save = NULL;
    free(title);

    //curl_easy_pause(curl, CURLPAUSE_ALL);

    return size * nmemb;
}
