#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../headers/server.h"
#include "../headers/irc.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"


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
	pthread_mutex_lock(&srv->mutex);
	int close_res = close(srv->s);
	pthread_mutex_unlock(&srv->mutex);

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
	pthread_mutex_lock(&srv->mutex);
    int res = send(srv->s, buf, strlen(buf), 0);
	pthread_mutex_unlock(&srv->mutex);

    if (res <= 0)
        return res;

	pthread_mutex_lock(&srv->mutex);
    srv->sent_len += res;
	pthread_mutex_unlock(&srv->mutex);

    return res;
}


/*
 todo:
	timout handling
*/
int server_recv(char *buf, server *srv)
{

	char tmpbuf[1024];

	pthread_mutex_lock(&srv->mutex);
	int res = recv(srv->s, tmpbuf, sizeof tmpbuf, 0);
	pthread_mutex_unlock(&srv->mutex);

	tmpbuf[res] = '\0';

	if (res <= 0)
	{
		server_disconnect(srv);
		return res;
	}

	if (!buf)
		buf = malloc((strlen(tmpbuf) + 1) * sizeof(char));
	else
	{
		buf = realloc(buf, (strlen(tmpbuf) + 1) * sizeof(char));
	}

	strcpy(buf, tmpbuf);

	pthread_mutex_lock(&srv->mutex);
	srv->recvd_len += res;
	pthread_mutex_unlock(&srv->mutex);

	char *save;
	char *line;

	line = strtok_r(buf, "\r\n", &save);

	while (line != NULL)
	{
		void **params = malloc(2 * sizeof(void*));

		params[0] = (void*) srv;
		params[1] = (void*) line;

		execute_callbacks(CALLBACK_SERVER_RECV, params, 2);
		free(params);

		line = strtok_r(NULL, "\r\n", &save);
  	}
	return res;
}



void server_set_timeout(time_t sec, time_t usec, server *srv)
{
	pthread_mutex_lock(&srv->mutex);
	srv->tv->tv_sec = sec;
	srv->tv->tv_usec = usec;
	pthread_mutex_unlock(&srv->mutex);

	return;
}
