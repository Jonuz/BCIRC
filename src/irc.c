#include <string.h>
#include <stdio.h>

#include "../include/irc.h"

int privmsg(char *msg, char *target, server *srv)
{
	char *buf = "PRIVMSG %s: %s\r\n";
	sprintf(buf, target, msg);
	int res = server_send(buf, srv);
	return res;

	/*
		server_send(char *buf, server *srv);
	*/
}


int get_numeric_response(server *srv)
{
    return 1;
}
