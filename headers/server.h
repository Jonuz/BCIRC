#ifndef SERVER_H_
#define SERVER_H_

#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>


typedef struct {

	char *host;
	char *port;
	char *pass;

    int motd_sent;

	int s;

	size_t sent_len;
	size_t recvd_len;

	time_t timeout_sec;
	time_t timeout_usec;

	pthread_mutex_t mutex;

	struct timeval *tv;

} server;

extern server **server_list;
extern int server_count;

void handle_servers();

int server_disconnect(server *srv);
int server_connect(server *srv);
int server_set_info(server *srv);

int server_recv(char *buf, server *srv);
int server_send(char *buf, server *srv);

void server_set_timeout(time_t sec, time_t usec , server *srv);



#endif
