#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <libconfig.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../headers/server.h"
#include "../headers/irc.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"


server **server_list;
int server_count;

#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"


int server_connect(server *srv)
{
	pthread_mutex_lock(&srv->mutex);
	int *s = &srv->s;
	if (s == NULL)
	{
		pthread_mutex_unlock(&srv->mutex);
        return -1;
	}
	pthread_mutex_unlock(&srv->mutex);

	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(srv->host, srv->port, &hints, &res);
	pthread_mutex_lock(&srv->mutex);
	*s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (*s == -1)
		return -2;

	if (connect(*s, res->ai_addr, res->ai_addrlen) != 0)
        return -3;

	srv->motd_sent = 0;
	pthread_mutex_unlock(&srv->mutex);


    void **cb_params = malloc( sizeof(void*) );
    cb_params[0] = (void*) srv;
    execute_callbacks( CALLBACK_SERVER_CONNECTED, cb_params, 1 );
	free(cb_params);

	freeaddrinfo(res);


	return 1;
}

int server_disconnect(server *srv)
{
	int close_res = close(srv->s);

	void **cb_params = malloc( sizeof(server) );
    cb_params[0] = srv;
    execute_callbacks( CALLBACK_SERVER_DISCONNECTED, cb_params, 1 );
	free(cb_params);

	return close_res;
}


/* todo:
	timeout handling
*/
int server_send(char *buf, server *srv)
{
	void **params = malloc(2* sizeof(void*));
	params[0] = srv;
	params[1] = buf;

	if (execute_callbacks(CALLBACK_SERVER_SEND, params, 2) != BCIRC_PLUGIN_OK)
	{
		free(params);
		return -2;
	}
	free(params);

	int res = send(srv->s, buf, strlen(buf), 0);

    if (res <= 0)
        return res;

	pthread_mutex_lock(&srv->mutex);
    srv->sent_len += res;
	pthread_mutex_unlock(&srv->mutex);

    return res;
}

void *server_recv(void *srv_void)
{
	char tmpbuf[2048];
	char *buf = NULL;

	server *srv = (server*) srv_void;

	if (!srv)
	{
		puts("srv is null!");
		return NULL;
	}
/*
	struct timeval tv;
	fd_set readfs;

	int n;

	tv.tv_sec = 0;
	tv.tv_usec = 25000;


	for (;;)
	{
		FD_ZERO(&readfs);
		FD_SET(srv->s, &readfs);

		n = select(srv->s+1, &readfs, NULL, NULL, &tv);

		if (n == 2) //timeout
			continue;
*/
	int res;
	while( (res = recv(srv->s, tmpbuf, sizeof tmpbuf, 0) ) >= 0 )
	{

		tmpbuf[res] = '\0';

		if (res <= 0)
		{
			server_disconnect(srv);
			return NULL;
		}

		if (!buf)
			buf = malloc((strlen(tmpbuf) + 1) * sizeof(char));
		else
			buf = realloc(buf, (strlen(tmpbuf) + 1) * sizeof(char));

		strcpy(buf, tmpbuf);

		srv->recvd_len += res;

		char *save;
		char *line;

		line = strtok_r(buf, "\r\n", &save);
		while (line != NULL)
		{
			void **params = malloc(2 * sizeof(void*));

			params[0] = (void*) srv;
			params[1] = (void*) line;

			puts("asd");
			printf("%s\n", srv->network_name);
			execute_callbacks(CALLBACK_SERVER_RECV, params, 2);
			printf("%s\n", srv->network_name);
			puts("das");
			free(params);

			line = strtok_r(NULL, "\r\n", &save);
		}
		puts("end");
	}
	return NULL;
}

int load_servers(char *config)
{
	config_t cfg;
	config_setting_t *setting;
	config_init(&cfg);

	if (!config_read_file(&cfg, config))
	{
		printf("Failed to load config!\n");
		printf("%d\n%s\n", config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);

		return -2;
	}


	if (! (setting = config_lookup(&cfg, "servers")))
	{
		printf("Failed to load setting \"servers\".(%s)\n", config);
		return -1;
	}

	server *srv = malloc(sizeof(server));
	if (!srv)
	{
		puts("failed to malloc server");
		exit(EXIT_FAILURE);
	}
	pthread_mutex_init(&srv->mutex, NULL);

	unsigned int server_count = config_setting_length(setting);
	printf("server_count: %d\n", server_count);

	for (int i = 0; i < server_count; i++)
	{
		config_setting_t *srv_setting = config_setting_get_elem(setting, i);

		if(!config_setting_lookup_string(srv_setting ,"network_name", &srv->network_name))
			return -1;
		if(!config_setting_lookup_string(srv_setting, "server_host", &srv->host))
			return -1;
		if(!config_setting_lookup_string(srv_setting, "server_port", &srv->port))
			return -1;
		if(!config_setting_lookup_string(srv_setting, "realname", &srv->realname))
			return -1;
		if(!config_setting_lookup_string(srv_setting, "username", &srv->username))
			return -1;
		if(!config_setting_lookup_string(srv_setting, "nick", &srv->nick))
			return -1;
		if(!config_setting_lookup_string(srv_setting, "alt_nick", &srv->alt_nick))
			return -1;
		config_setting_lookup_string(srv_setting, "server_pass", &srv->pass);

		if (server_connect(srv) != 1)
		{
			printf("Failed connect to %s\n", srv->host);
			return 0;
		}
		puts("oink");
		add_to_serverpool(srv);
	}

	return 1;
}


int add_to_serverpool(server *srv)
{
	if (!srv)
	{
		printf("srv is null!\n");
		return -1;
	}

	if (server_list)
		server_list = realloc(server_list, (server_count + 1) * sizeof(server*));
	else
		server_list = malloc(sizeof(server*));

	server_list[server_count] = malloc(sizeof(server*));
	server_list[server_count] = srv;

	server_count++;

	int ret = pthread_create(&srv->thread, NULL, server_recv, (void*) srv);

	if (ret)
	{
		printf("pthread_create failed, %d\n", ret);
		return -2;
	}
	pthread_detach(srv->thread);

	return 1;
}
