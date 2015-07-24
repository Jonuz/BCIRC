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
    char *buffer
    server *srv
*/


#define CALLBACK_SERVER_SEND "server_send"
/*
Params:
    char *buffer
    server *srv
*/





#define CHANNEL_JOINED "channel_joined"
/*
    Params:
        channel *
        user *

        Callback to call after someone joined to channel.
*/


#define CHANNEL_PART "channel_part"
/*
    Params:
        channel *
        char *reason

        Callback to call after someone parted from channel.

*/

#endif
