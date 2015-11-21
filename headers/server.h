#ifndef SERVER_H_
#define SERVER_H_

#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>


#define SERVER_INTENTIONAL_DC 0
#define SERVER_CLOSED_CONNECTION 1
#define SERVER_TIMEOUT 2


typedef struct server
{
	char *network_name;

	char *host;
	char *port;
	char *pass;

	int autorejoin; //1 or 0

	char *realname;
	char *username;
	char *nick;
	char *alt_nick;

    int motd_sent;

	int s;

	size_t sent_len;
	size_t recvd_len;

	time_t timeout_sec;
	time_t timeout_usec;

	pthread_t thread;
	pthread_mutex_t mutex;


	int connected;

	struct timeval *tv;

} server;


extern server **server_list;
extern int server_count;

extern pthread_mutex_t servers_global_mutex;

int add_to_serverpool();

int server_disconnect(server *srv);
int server_connect(server *srv);
int server_set_info(server *srv);

void *server_recv(void *srv);
int server_send(char *buf, server *srv);

int load_servers(char *);


#endif
