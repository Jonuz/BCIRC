#ifndef IRC_H_
#define IRC_H_

#include <string.h>
#include <time.h>

#include "channel.h"
#include "user.h"
#include "server.h"


/*
    Plugin callbacks
*/

#define CALLBACK_GOT_NUMERIC "got_numeric"
/*
    Params:
        int numeric
        char *buffer
        server *srv
*/
#define CALLBACK_GOT_PRIVMSG "got_privmsg"
/*
	Params:
		server *srv
		char *nick
		char *hostmask
		char *target
		char *msg

*/


typedef struct
{
	struct user *my_info;
	struct channel **channels_info;
	struct server *server_info;

} irc_base;

int add_to_privmsg_queue(char *msg, char *target, server *srv, const int drop);
void *handle_privmsg_queue();
int get_numeric(void **params, int argc);
int get_privmsg(server *srv, char *buf);

#endif
