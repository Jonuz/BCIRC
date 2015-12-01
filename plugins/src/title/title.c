#include <stdio.h>
#include <curl/curl.h>
#include <regex.h>
#include <sys/types.h>
#include <time.h>

#include  "entities.h"

#include "../headers/irc.h"
#include "../headers/log.h"
#include "../headers/server.h"
#include "../headers/irc_cmds.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"


#define get_str_size(str) ( (strlen( (char*) str) + 1) * sizeof(char) )

char plugin_name[] = "Title announcer";
char plugin_version[] = "0.1";
char plugin_author[] = "Joona";


#define TITLE_MAX_LEN 250

typedef struct ark
{
    server *srv_save;
    char *target_save;
    char *nick_save;
    int tries;

	clock_t start_time;
} ark;


int add_url(void **, int);
int check_for_url(void**, int argc);
int http_request(char *url, ark *arkptr);
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream);

int has_filter = 0; // 1 if true


int plugin_init(plugin *pluginptr)
{
    has_filter = get_urls(); //1 if urls in urls.txt
    curl_global_init(CURL_GLOBAL_DEFAULT);

    register_callback(CALLBACK_GOT_PRIVMSG, check_for_url, 20, pluginptr);
    register_callback(CALLBACK_GOT_PRIVMSG, add_url, 20, pluginptr);

    return BCIRC_PLUGIN_OK;
}


int check_for_url(void **params, int argv)
{
    for (int i = 0; i < argv; i++)
        if (params[i] == NULL)
        {
            bcirc_printf("params[%d] is NULL!\n", i);
            return BCIRC_PLUGIN_FAIL;
        }

    server *srv = params[0];
    char *nick = params[1];
    char *hostmask = params[2];
    char *target = params[3];
    char *msg = params[4];

	ark *new_ark = malloc(sizeof(ark));
	new_ark->start_time = clock();
    new_ark->tries = 0;
    regex_t regex;
    int reti;
    regmatch_t matches[1];

    reti = regcomp(&regex, "https?:\/\/[^\ ]+", REG_EXTENDED);
    if (reti != 0)
    {
        bcirc_printf("Failed to compile regex!\n");
        regfree(&regex);
		free(new_ark);
        return BCIRC_PLUGIN_FAIL;
    }

    reti = regexec(&regex, msg, 1, matches, 0);
    regfree(&regex);
    if (reti != 0)
    {
		free(new_ark);
        return BCIRC_PLUGIN_CONTINUE;
    }

    size_t len = matches[0].rm_eo - matches[0].rm_so + 1;
    char *url = malloc((len + 1) * sizeof(char));

    strncpy(url, msg+matches[0].rm_so, len);
    url[len-1] = '\0';

    new_ark->srv_save = srv;

    new_ark->target_save = malloc(strlen(target) + 1);
    strcpy(new_ark->target_save, target);
    new_ark->nick_save = malloc(strlen(nick) + 1);
    strcpy(new_ark->nick_save, nick);


    if (has_filter == 1)
        if (check_url(url) == 0)
		{
			free(new_ark);
            return BCIRC_PLUGIN_OK;
		}

    http_request(url, new_ark);
    free(url);
    free(new_ark->target_save);
    free(new_ark->nick_save);
    free(new_ark);

    return BCIRC_PLUGIN_OK;
}

int http_request(char *url, ark *arkptr)
{
    CURLcode res;
    CURL *curl;

    curl = curl_easy_init();
    if (!curl)
    {
        printf("Failed to initalize curl!\n");
        return BCIRC_PLUGIN_STOP;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, arkptr);


    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);


    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        if (arkptr->tries >= 3)
        {
            printf("Tried %d times already, stopping now.\n", arkptr->tries);
            return BCIRC_PLUGIN_OK;
        }

        bcirc_printf("curl failed: %s\n", curl_easy_strerror(res));
        arkptr->tries++;

        http_request(url, arkptr);

        return BCIRC_PLUGIN_OK;
    }

    return BCIRC_PLUGIN_OK;
}


size_t write_callback(void *ptr, size_t size, size_t nmemb, void *ark_param)
{
    regex_t regex;
    int reti;
    regmatch_t matches[2];

    ark *arkptr = (ark*) ark_param;

    char *response = (char*) malloc((size * nmemb + 2) * sizeof(char));
    strncpy(response, (char*) ptr, size * nmemb);
    response[size*nmemb] = '\0';


    char regex_str[] = "<\\s*title[^>]*>\\s*([^<]+\\s*)";
    reti = regcomp(&regex, regex_str, REG_EXTENDED);
    if (reti != 0)
    {
        bcirc_printf("Failed to compile regex!\n");
        free(response);
        regfree(&regex);
        return size * nmemb;;
    }

    reti = regexec(&regex, response, 2, matches, 0);
    regfree(&regex);
    if (reti != 0)
    {
        free(response);
        return size * nmemb;
    }

    size_t len = matches[1].rm_eo - matches[1].rm_so;

    if (len > TITLE_MAX_LEN)
    {
        bcirc_printf("Title is too long, wont send it.\n");
        return size * nmemb;
    }

    char *title = (char*) malloc((len + 2) * sizeof(char));
    strncpy(title, (char*) response + matches[1].rm_so, len);
    title[len] = '\0';

    free(response);

    decode_html_entities_utf8(title, NULL);


    char *target = malloc((strlen(arkptr->target_save)+1) * sizeof(char));
    char *nick = malloc(( strlen(arkptr->nick_save)+1) * sizeof(char));

    strcpy(target, arkptr->target_save);
    strcpy(nick, arkptr->nick_save);

    if (arkptr->target_save[0] == '#') //In future: Check if target is channel.
        add_to_privmsg_queue(title, target, arkptr->srv_save, 0);
    else
        add_to_privmsg_queue(title, nick, arkptr->srv_save, 0);

    free(title);

    return size * nmemb;
}
