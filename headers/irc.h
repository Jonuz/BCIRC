#ifndef IRC_H_
#define IRC_H_

#include <string.h>
#include <time.h>

#include "channel.h"
#include "server.h"


typedef struct
{
	char *nick;
	char *hostmask;

	server *srv;
} user;

user *get_user(char *str, server *srv);

int add_to_privmsg_queue(char *msg, char *target, server *srv, const int drop);
void *handle_privmsg_queue();

#endif
