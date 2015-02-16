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
/*
    Params:
        int numeric
        char *buffer;
        server *srv
*/
#define CALLBACK_GOT_NUMERIC "got_numeric"

typedef struct
{
	struct user *my_info;
	struct channel **channels_info;
	struct server *server_info;

} irc_base;


int get_numeric(void **params, int argc);

//int get_numeric_response(server *srv);


#endif
