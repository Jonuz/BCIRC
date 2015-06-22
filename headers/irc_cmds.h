#ifndef IRC_CMDS_H_
#define IRC_CMDS_H_

#include "irc.h"
#include "server.h"
#include "channel.h"

int privmsg(char *msg, char *target, server *srv);
int join_channel(char *chan_name, char *chan_pass, server *srv);
int part_channel(char *reason, channel *chan);


#endif // IRC_CMDS_H_INCLUDED
