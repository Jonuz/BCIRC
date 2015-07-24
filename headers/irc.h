#ifndef IRC_H_
#define IRC_H_

#include <string.h>
#include <time.h>

#include "channel.h"
#include "server.h"


typedef struct
{
	struct channel **channels_info;
	struct server *server_info;

} irc_base;

int add_to_privmsg_queue(char *msg, char *target, server *srv, const int drop);
void *handle_privmsg_queue();

#endif
