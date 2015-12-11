#ifndef CALLBACKS_H_
#define CALLBACKS_H_


#define CALLBACK_GOT_NUMERIC "got_numeric"
/*
    Params:
        server *srv
        int *numeric
        char *buffer
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
    int reason, SERVER_INTENTIONAL_DC or SERVER_TIMEOUT
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


#define CALLBACK_CHANNEL_JOIN "channel_join"
/*
    Params:
        channel *
        char *nick
        char *hostmask
        int event_type
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

#define CALLBACK_CHANNEL_QUIT "channel_quit"
/*
    Params:
        channel *
        char *nick
        char *hostmask
        char *reason

*/
#define CALLBACK_CALLBACKS_EXECUTED "callback_callbacks_executed"
/*
    Params:
        void **arguments
        int count

    Note:
        When CALLBACK_CALLBACKS_EXECUTED are executed, name of callback which are being executed is added to end of aguments list.
        So you can get name of callback like this:

        char *cb_name = arguments[count-1];
*/


#endif
