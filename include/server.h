#ifndef SERVER_H_
#define SERVER_H_

#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>


/*
    Plugin callbacks
*/
/*
    Params:
        server *srv
*/
#define CALLBACK_SERVER_CONNECTED "server_connected"


/*
    Params:
        server *srv
*/
#define CALLBACK_SERVER_DISCONNECTED "server_disconnected"

/*
    Params:
        char *buffer
        server *srv
*/
#define CALLBACK_SERVER_RECV "server_recv"

/*
    Params:
        char *buffer
        server *srv
*/
#define CALLBACK_SERVER_SEND "server_send"

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
