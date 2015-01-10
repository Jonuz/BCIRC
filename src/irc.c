#include <string.h>
#include <stdio.h>

#include "../include/irc.h"

int privmsg(char *msg, char *target, server *srv)
{
	char *buf = "PRIVMSG %s: %s\r\n";
	sprintf(buf, target, msg);
	int res = server_send(buf, srv);

	return res;
}

int get_numeric(void **params, int argc)
{


}
