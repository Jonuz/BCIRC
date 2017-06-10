#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/irc.h"
#include "../headers/log.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"


char plugin_name[] = "Seeker of benis :D";
char plugin_author[] = "Joona";
char plugin_version[] = "0.1";

int seek_for_benis(void **params, int argv);

int plugin_init(plugin *pluginptr)
{
	register_callback(CALLBACK_GOT_PRIVMSG, seek_for_benis, 20, pluginptr);

	return BCIRC_PLUGIN_OK;
}



int seek_for_benis(void **params, int argv)
{
	for (int i = 0; i < argv; i++)
		if (params[i] == NULL)
		{
			bcirc_printf("params[%d] is NULL!\n", i);
			return BCIRC_PLUGIN_FAIL;
		}

	server *srv = params[0];
	char *nick = params[1];
	char *hostmask = params[2];
	char *target = params[3];
	char *msg = params[4];

	if (strstr(msg, "benis") != NULL)
	{
		bcirc_printf("%s said benis :DD\n", nick);

		srand(time(NULL));

		int lucky_number = rand() % 10;
		bcirc_printf("Lucky number is %d:DDD\n", lucky_number);

		if (lucky_number == 0)
		{
			bcirc_printf("we are lucky :DDD\n");

			char *benis = malloc(255);
			strcpy(benis, "benis :D");

			int benis_len = strlen(benis);

			int i = 0;
			for (i = 0; i < 240 && (rand() % 110) > 5; i++)
			{
				benis[benis_len + i] = (char) 'D';
			}
			benis[benis_len+i] = '\0';

			add_to_privmsg_queue(benis, target, srv, 0);
			free(benis); // sed id free :DDD
		}

	}

}
