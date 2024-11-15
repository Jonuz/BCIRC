#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/log.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"


char *urls = NULL;

bool check_url(char *str)
{
	if ((!str) || (!urls))
	{
		return false;
	}

	char *token, *save;
	char urls_save[strlen(urls)+1];
	strcpy(urls_save, urls);

	token = strtok_r(urls, "\n", &save);
	while( token != NULL )
	{
		//printf("token: %s\n", token);
		if (strstr(str, token) != NULL)
		{
			strcpy(urls, urls_save);
			return true;
		}
		token = strtok_r(NULL, "\n", &save);
	}
	strcpy(urls, urls_save);
	return true;
}


int get_urls(void)
{
	FILE *urlstxt = NULL;
	char url_file[512];

	sprintf(url_file, "%s/urls.txt", getenv("BCIRC_CONFIG_DIR"));

	urlstxt = fopen(url_file, "r");
	if (urlstxt == NULL)
	{
		bcirc_printf("%s doesnt exists, url filtering disabled.\n", url_file);
		if (urls)
			free(urls);
		return 0;
	}

	size_t len = 0;
	ssize_t read;

	char *line = NULL;
	while ( (read = getline(&line, &len, urlstxt)) != -1 )
	{
		if (urls)
		{
			urls = realloc(urls, (strlen(urls) + len + 1) * sizeof(char) );
			strcat(urls, line);
		}
		else
		{
			urls = malloc((len + 1) * sizeof(char));
			strcpy(urls, line);
		}
	}
	fclose(urlstxt);
	return 1;
}


int add_url(void **params, int argc)
{
	char *nick = params[1];
	char *msg = params[4];

	if ((!msg) || (!nick))
	{
		bcirc_printf("msg or nick NULL\n");
		return BCIRC_PLUGIN_CONTINUE;
	}

	if (strstr(msg, "!addurl ") != msg) //TODO: Make actual system for adding commands with permission system.
	{
		return BCIRC_PLUGIN_CONTINUE;
	}

	if (strlen(msg) > 255)
		return BCIRC_PLUGIN_CONTINUE;

	char str[255];
	strncpy(str, msg+8, strlen(msg) - 8);
	str[ strlen(msg) - 8 ] = '\0';

	if ((check_url(str)) == true)
		return BCIRC_PLUGIN_CONTINUE;

	FILE *urlstxt = NULL;
	char url_file[512];

	sprintf(url_file, "%s/urls.txt", getenv("BCIRC_CONFIG_DIR"));

	urlstxt = fopen(url_file, "a");
	if (!urlstxt)
	{
		bcirc_printf("failed to open %s\n", url_file);
		fclose(urlstxt);
		return BCIRC_PLUGIN_FAIL;
	}

	fprintf(urlstxt, "%s\n", str);
	fclose(urlstxt);


	free(urls);
	urls = NULL;

	get_urls();

	return BCIRC_PLUGIN_OK;
}
