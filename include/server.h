#ifndef SERVER_H_
#define SERVER_H_

#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>


typedef struct {

	char *host;
	char *port;
	char *pass;

    bool motd_sent;

	int s;

	size_t sent_len;
	size_t recvd_len;

	time_t timeout_sec;
	time_t timeout_usec;

	struct timeval *tv;

} server;

int server_disconnect(server *srv);
int server_connect(server *srv);
int server_set_info(server *srv);
int server_send(char *buf, server *srv);
void server_set_timeout(time_t sec, time_t usec , server *srv);
int server_recv(char *buf, server *srv);


#endif

