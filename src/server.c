#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../include/server.h"
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

	//freeaddrinfo(res);

	return 1;
}

int server_disconnect(server *srv)
{
	return close(srv->s);
}


/* todo:
	timeout handling
*/
int server_send(char *buf, server *srv)
{

	int res = send(srv->s, buf, sizeof buf, 0);
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
    char tmpbuf[2048];
	int res = recv(srv->s, tmpbuf, sizeof tmpbuf, 0);

	if (res <= 0)
	{
		free(buf);
        //run_server_disconnect_cbs(srv);
        return res;
	}

	for (size_t i = 0; i < strlen(tmpbuf); i++) /* Buffer may contain some unwanted stuff after end of the message */
	{
        if ((tmpbuf[i] == '\n') && (i != strlen(tmpbuf)))
            tmpbuf[i+1] = '\0';
	}

    buf = realloc(buf, strlen(tmpbuf) + 1 * (sizeof(char)));
    strcpy(buf, tmpbuf);
	srv->recvd_len += res;

    void **params;
    params = malloc(sizeof(server*));
    params[0] = (void*)srv;

    execute_callbacks(CALLBACK_SERVER_CONNECTED, params, 1);

	return res;
}


void server_set_timeout(time_t sec, time_t usec, server *srv)
{
	srv->tv->tv_sec = sec;
	srv->tv->tv_usec = usec;
	return;
}
