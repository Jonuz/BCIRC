#ifndef IRC_CMDS_H_
#define IRC_CMDS_H_

#include <string.h>

#include "../headers/server.h"
#include "../headers/channel.h"
#include "../headers/irc_cmds.h"

int privmsg(char *msg, char *target, server *srv);
int join_channel(char *chan_name, char *chan_pass, server *srv);
int part_channel(char *reason, channel *chan);
int nick(char *nick, server *srv);
int quit(char *reason, server *srv);

#endif // IRC_CMDS_H_INCLUDED
