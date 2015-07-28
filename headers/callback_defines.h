#ifndef CALLBACKS_H_
#define CALLBACKS_H_


#define CALLBACK_GOT_NUMERIC "got_numeric"
/*
    Params:
        int *numeric
        char *buffer
        server *src
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



#define CALLBACK_SERVER_CONNECTED "server_connected"
/*
Params:
    server *srv
*/


#define CALLBACK_SERVER_DISCONNECTED "server_disconnected"
/*
Params:
    server *srv
*/


#define CALLBACK_SERVER_RECV "server_recv"
/*
Params:
    server *srv
    char *buffer
*/


#define CALLBACK_SERVER_SEND "server_send"
/*
Params:
    char *buffer
    server *srv
*/





#define CALLBACK_CHANNEL_JOIN "channel_joined"
/*
    Params:
        channel *

*/


#define CALLBACK_CHANNEL_PART "channel_part"
/*
    Params:
        channel *
        char *nick
        char *hostmask
        char *reason

*/

#define CALLBACK_CHANNEL_KICK "channel_kick"
/*
    Params:
        channel *
        char *nick
        char *hostmask
        char *reason
*/


#endif
