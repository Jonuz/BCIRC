#ifndef USER_H_
#define USER_H_

#include <string.h>
#include <time.h>

typedef struct
{
	char *nick;
	char *realname;
	char *username;

	char *hostmask;

} user;


int set_user_nick(char *nick, user *usr);
int set_user_realname(char *realname, user *usr);
int set_user_username(char *username, user *usr);
int set_user_hostnask(char *hostmask, user *usr);


#endif
