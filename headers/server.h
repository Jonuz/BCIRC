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

	time_t time_connected;
	int rejoin_tries;

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

int server_disconnect(server *srv, int reason);
int server_connect(server *srv);

void *server_recv(void *srv);
int server_send(server *srv, char *buf);

int load_servers(char *);

int add_to_serverpool(server *srv);
int remove_from_serverpool(server *srv);
int free_server(server *srv);

#endif
