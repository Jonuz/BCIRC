#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"


char *urls = NULL;

int get_urls();
int check_url(char *);
int add_url(void **, int);



int get_urls()
{
    FILE *urlstxt = NULL;
    char url_file[512];

    sprintf(url_file, "%s/urls.txt", getenv("BCIRC_CONFIG_DIR"));


    urlstxt = fopen(url_file, "a");
    if (urlstxt == NULL)
    {
        printf("Failed to open %s\n", url_file);
        if (urls)
            free(urls);
        return 0;
    }

    size_t len = 0;
    ssize_t read;

    char *line = NULL;
    while ( (read = getline(&line, &len, urlstxt)) != -1 )
    {
        size_t new_size = 0;
        if (urls)
        {
            new_size = (strlen(urls) + strlen(line) + 1) * sizeof(char);
            urls = realloc(urls, new_size);
        }
        else
        {
            new_size = strlen(line) + 1;
            urls = malloc(new_size);
        }
        strcat(urls, line);
    }
    fclose(urlstxt);
    return 1;
}

int check_url(char *url) // 1 if true, othetwise 0.
{

    if (!url)
    {
        printf("Url is NULL!\n");
        return 0;
    }

    char *token, *save;

    token = strtok_r(url, "\n", &save);

    while( token != NULL )
    {
        if (strcmp(token, url) == 0) //url.com
        {
            return 1;
        }
        if (strcmp(token+4, url) == 0) //www.url.com
        {
            return 1;
        }
        if (strcmp(token+7, url) == 0) //http://www.url.com
        {
            return 1;
        }
        if (strcmp(token+8, url) == 0) //https://www.url.com
        {
            return 1;
        }

        strtok_r(NULL, "\n", &save);
    }
    return 0;
}

int add_url(void **params, int argc)
{
    char *nick = params[1];
    char *msg = params[4];

    if ((!msg) || (!nick))
    {
        puts("msg or nick NULL");
        return BCIRC_PLUGIN_CONTINUE;
    }

    puts("oink");
    if (strstr(msg, "!addurl") != msg) //TODO: Make actual system for adding commands and permission system.
    {
        return BCIRC_PLUGIN_CONTINUE;
    }

    if (strlen(msg) > 255)
        return BCIRC_PLUGIN_CONTINUE;

    char url[255];
    strtok(url, msg+8);
    strncpy(url, msg+8, strlen(msg) - 8);

    size_t new_size = (strlen(url) + 1) * sizeof(char);
    if (urls)
    {
        new_size += strlen(urls) * sizeof(char);
        urls = realloc(urls, new_size);
        strcat(urls, url);
    }
    else
    {
        urls = malloc(new_size);
        strcpy(urls, url);
    }

    FILE *urlstxt = NULL;
    char url_file[512];

    sprintf(url_file, "%s/urls.txt", getenv("BCIRC_CONFIG_DIR"));

    urlstxt = fopen(url_file, "a");
    if (!urlstxt)
    {
        printf("failed to open %s\n", url_file);
        fclose(urlstxt);
        return BCIRC_PLUGIN_FAIL;
    }

    fprintf(urlstxt, "%s\n", url);
    fclose(urlstxt);

    return BCIRC_PLUGIN_OK;
}
