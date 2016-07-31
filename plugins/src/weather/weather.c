#include <stdio.h>
#include <dlfcn.h>

#include <curl/curl.h>

#include "cJSON/cJSON.h"

#include "../headers/log.h"
#include "../headers/irc.h"
#include "../headers/server.h"
#include "../headers/channel.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"

char plugin_version[] = "1.0";
char plugin_author[] = "Joona";
char plugin_name[] = "Weather service";

int http_request(char *url, void **params, int argc);
int on_privmsg(void **params, int argc);
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *info);

#define URL_BASE "http://api.openweathermap.org/data/2.5/weather?q=%s&APPID=%s&units=metric"

/*
    Please get your own API key from http://openweathermap.org/
*/
#define API_KEY ""

#define MSG "Paikkakunnan %s tiedot; lämpötila noin %.2f°C, ilmankosteus: %.0f%%, tuulen nopeus %.2f m/s"
//#define MSG "Weather data of place %s; tempature %.2f°C, lowest tempature: %.2f°C, highest tempature %.2f°C, humidity: %.0f%%, speed of wind %.2f m/s"

typedef struct target_info
{
    server *srv;
	char *target;
} target_info;

int plugin_init(plugin *pluginptr)
{
    if (strlen(API_KEY) == 0)
    {
        bcirc_printf("API key is not set, disabling %s\n", plugin_name);
        return BCIRC_PLUGIN_STOP;
    }
    
	register_callback(CALLBACK_GOT_PRIVMSG, on_privmsg, 20, pluginptr);
    curl_global_init(CURL_GLOBAL_DEFAULT);

    return BCIRC_PLUGIN_OK;
}

int on_privmsg(void **params, int argc)
{
    #define COMMANDS_COUNT 2
    char *commands[COMMANDS_COUNT] = {"!sää", ".sää"} ;
    //char *commands[COMMANDS_COUNT] = {"!weather", ".weather"} ;

    static unsigned long last_call = 0;

    unsigned long call_now = time(NULL);
    char *str = params[4];
    int is_command = 0;

    for (int i = 0; i < COMMANDS_COUNT; i++)
    {
        if (strstr(str, commands[i]) == 0)
        {
            is_command = 1;
            break;
        }
    }
    if (is_command == 0)
        return BCIRC_PLUGIN_CONTINUE;

    char *save = NULL;
    char *city = strtok_r(str, " ", &save);

    if (city == NULL)
        return BCIRC_PLUGIN_OK;
    city = strtok_r(NULL, " ", &save);

    if (city == NULL)
        return BCIRC_PLUGIN_OK;

    #define WAIT_FETCH_TIME 7
    #define TIME_ANNOUNCE_TIME 3

    if (call_now <= last_call + WAIT_FETCH_TIME)
    {
        if (call_now <= last_call + TIME_ANNOUNCE_TIME)
        {
            //char *tmp = "Still %d seconds untill weather data can be fetched.";
            char *tmp = "Vielä %d sekunttia jotta säätietoja voidaan hakea.";

            char *msg = malloc(strlen(tmp) + 2);
            sprintf(msg, tmp, (call_now - (last_call + WAIT_FETCH_TIME)) * -1);

            add_to_privmsg_queue(msg, params[3], params[0], 0);
            free(msg);
        }
        return BCIRC_PLUGIN_OK;
    }
    last_call = time(NULL) ;

    char *url = malloc(strlen(URL_BASE) + strlen(city) + strlen(API_KEY) + 1);
    sprintf(url, URL_BASE, city, API_KEY);

	bcirc_printf("User %s(%s) asked weather info for place %s.\n", params[1], params[2], city);

    http_request(url, params, argc);

    free(url);

    return BCIRC_PLUGIN_OK;
}

int http_request(char *url, void **params, int argc)
{
    CURLcode res;

    CURL *curl = curl_easy_init();

    if (!curl)
    {
        bcirc_printf("Failed to initalize curl!\n");
        return BCIRC_PLUGIN_STOP;
    }

	target_info *info = malloc(sizeof(target_info));

	info->target = malloc(strlen(params[3]) + 1);
	info->srv = params[0];
	strcpy(info->target, params[3]);

	curl_easy_setopt(curl, CURLOPT_WRITEDATA, info);

    curl_easy_setopt(curl, CURLOPT_URL, url);

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

	res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        bcirc_printf("curl failed, url: %s\n", url);
    }

    free(info->target);
    free(info);

    return 1;
}

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *infotmp)
{
    char *response = (char*) malloc((size * nmemb + 2) * sizeof(char));
    strncpy(response, (char*) ptr, size * nmemb);
    response[size*nmemb] = '\0';

	target_info *info = (target_info*) infotmp;

	bcirc_printf("%s\n", response);

	cJSON *root = cJSON_Parse(response);

	cJSON *main = cJSON_GetObjectItem(root, "main");
	cJSON *wind = cJSON_GetObjectItem(root, "wind");
	cJSON *weather = cJSON_GetObjectItem(root, "weather");


	char *city = cJSON_GetObjectItem(root, "name")->valuestring;

	double humidity = cJSON_GetObjectItem(main, "humidity")->valuedouble;

	double temp = cJSON_GetObjectItem(main, "temp")->valuedouble;
	double temp_max = cJSON_GetObjectItem(main, "temp_max")->valuedouble;
	double temp_min = cJSON_GetObjectItem(main, "temp_min")->valuedouble;

	double windspeed = cJSON_GetObjectItem(wind, "speed")->valuedouble;

	char msg_tmp[1024];

	sprintf(msg_tmp, MSG, city, temp, humidity, windspeed);

	char *msg = malloc(strlen(msg_tmp) + 1);
	strcpy(msg, msg_tmp);


	add_to_privmsg_queue(msg, info->target, info->srv, 0);

    free(msg);

	return size * nmemb;
}
