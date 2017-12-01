#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/irc.h"
#include "../headers/log.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"


char plugin_name[] = "Nick changed";
char plugin_author[] = "Joona";
char plugin_version[] = "0.1";

int handle_nick(void **params, int argv);

int plugin_init(plugin *pluginptr)
{
	register_callback(CALLBACK_GOT_PRIVMSG, handle_nick, 20, pluginptr);

	return BCIRC_PLUGIN_OK;
}



int handle_nick(void **params, int argv)
{
	for (int i = 0; i < argv; i++)
		if (params[i] == NULL)
		{
			bcirc_printf("params[%d] is NULL!\n", i);
			return BCIRC_PLUGIN_FAIL;
		}

	server *srv = params[0];
	char *msg = params[4];

	if (strpos(msg, "!nick") == 0)
	{
		char * nick_token, *save;
		nick_token = strtok_r(msg, " ", &save);
		if (nick_token) {
			nick_token = strtok_r(NULL, " ", &save);
			if (nick_token) {
				nick(nick_token, srv);
			}
		}
	}

}
