#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../include/server.h"
#include "../include/irc.h"
#include "../include/plugin-handler.h"

int server_connect(server *srv)
{
	int *s = &srv->s;
	if (s == NULL)
        return -1;
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(srv->host, srv->port, &hints, &res);
	*s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (*s == -1)
		return -2;

	if (connect(*s, res->ai_addr, res->ai_addrlen) != 0)
        return -3;

    void **cb_params = malloc( sizeof(server*) );
    cb_params[0] = (void*) srv;
    execute_callbacks( CALLBACK_SERVER_CONNECTED, cb_params, 1 );

	//freeaddrinfo(res);

	return 1;
}

int server_disconnect(server *srv)
{
    void **cb_params = malloc( sizeof(server) );
    cb_params[0] = (void*) srv;
    execute_callbacks( CALLBACK_SERVER_DISCONNECTED, cb_params, 1 );

	return close(srv->s);
}


/* todo:
	timeout handling
*/
int server_send(char *buf, server *srv)
{
    int res = send(srv->s, buf, strlen(buf), 0);
    if (res <= 0)
        return res;
    srv->sent_len += res;
    return res;
}


/*
 todo:
	timout handling
*/
int server_recv(char *buf, server *srv)
{
	char tmpbuf[1024];
	int res = recv(srv->s, tmpbuf, sizeof tmpbuf, 0);
	tmpbuf[res] = '\0';

	if (res <= 0)
	{
		//free(buf);
		server_disconnect(srv);
		return res;
	}

	if (!buf)
		buf = malloc(strlen(tmpbuf) * sizeof(char));
	else
		buf = realloc(buf, (strlen(tmpbuf) + 1) * (sizeof(char)));

	if (buf == NULL)
		return -2;

	char *buf_copy = malloc(sizeof(tmpbuf));
	strcpy(buf_copy, tmpbuf);
	strcpy(buf, tmpbuf);
	srv->recvd_len += res;

	is_privmsg(srv, buf_copy);

	char *save = malloc(sizeof(buf));
	char *line = malloc((strlen(buf) + 1) * sizeof(char));
	line = strtok_r(buf, "\r\n", &save);

	while (line != NULL)
	{
		void **params = malloc(2 * sizeof(void*));
		strcat(line, "\0");			params[0] = (void*) srv;
		params[1] = (void*) line;
		execute_callbacks(CALLBACK_SERVER_RECV, params, 2);
		line = strtok_r(NULL, "\r\n", &save);
  	}
	return res;
}



void server_set_timeout(time_t sec, time_t usec, server *srv)
{
	srv->tv->tv_sec = sec;
	srv->tv->tv_usec = usec;
	return;
}
