#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>

#include "events.h"
#include "../headers/log.h"
#include "../headers/irc.h"
#include "../headers/log.h"
#include "../headers/server.h"
#include "../headers/plugin_handler.h"
#include "../headers/callback_defines.h"

char plugin_name[] = "BCIRC-Core - Event handler";
char plugin_author[] = "Joona";
char plugin_version[] = "1.0";

bool is_fulldigit(char *str)
{
	for (size_t i = 0; i < strlen(str); i++)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}


int plugin_init(plugin *pluginptr)
{
	register_callback(CALLBACK_SERVER_RECV, get_privmsg, 3, pluginptr);
	register_callback(CALLBACK_SERVER_RECV, get_numeric, 3, pluginptr);
	register_callback(CALLBACK_SERVER_RECV, get_chan_event, 2, pluginptr);


	return BCIRC_PLUGIN_OK;
}


int get_privmsg(void **params, int argc)
{
	char *srv = params[0];
	char buf[strlen(params[1])];
	strcpy(buf, params[1]);

	if (strstr(buf, "PRIVMSG") == NULL) {
		return BCIRC_PLUGIN_OK;
	}

	char *hostmask = NULL;
	char *target = NULL;
	char *nick = NULL;

	char *str;
	char *save;

	str = strtok_r(buf, " ", &save);

	int i = 0;
	for (i; str != NULL; i++)
	{
		if (i == 0)
		{
			char nick_and_mask[strlen(str) + 1];
			strcpy(nick_and_mask, str);

			char *nick_part = NULL;
			char *mask_part = NULL;

			nick_part = strtok_r(nick_and_mask, "!", &mask_part);

			if (nick_part && strlen(nick_part)) {
				nick = malloc(strlen(nick_part));
				nick_part++; // Remove ':'
				strcpy(nick, nick_part);
			} else {
				return BCIRC_PLUGIN_OK;
			}

			if (mask_part && strlen(mask_part)) {
				hostmask = malloc(strlen(mask_part) + 1);
				strcpy(hostmask, mask_part);
			} else {
				free(nick);
				return BCIRC_PLUGIN_CONTINUE;
			}
		}
		if (i == 1)
			if (strcmp(str, "PRIVMSG") != 0)
			{
				free(nick);
				free(hostmask);

				return BCIRC_PLUGIN_OK;
			}
		if (i == 2)
		{
			target =  malloc((strlen(str) + 1));
			strcpy(target, str);
			break;
		}
		str = strtok_r(NULL, " ", &save);
	}

	int msg_len = strlen(save);
	char *msg = malloc(msg_len + 1);
	strncpy(msg, save+1, msg_len);
	msg[msg_len] = '\0';

	void **new_params = malloc(sizeof(void*) * 5);

	new_params[0] = (server*) srv;
	new_params[1] = (char*) nick;
	new_params[2] = (char*) hostmask;
	new_params[3] = (char*) target;
	new_params[4] = (char*) msg;

	execute_callbacks(CALLBACK_GOT_PRIVMSG, new_params, 5);

	free(new_params);

	free(nick);
	free(hostmask);
	free(target);
	free(msg);

	return BCIRC_PLUGIN_OK;
}


int get_numeric(void **params, int argc)
{
	server *srv = (server*) params[0];

	char buffer[strlen(params[1])+1];
	strcpy(buffer, params[1]);

	if (buffer == NULL)
		return BCIRC_PLUGIN_OK;
	if (srv == NULL)
		return BCIRC_PLUGIN_OK;

	char *save;
	char *word;

	word = strtok_r(buffer, " ", &save);
	if (!word)
	  return BCIRC_PLUGIN_OK;
	word = strtok_r(NULL, " ", &save);

	if (!word)
	  return BCIRC_PLUGIN_OK;


	if (is_fulldigit(word))
	{
		int *numeric = malloc(sizeof(int));
		*numeric = atoi(word);

		char *buf = malloc(strlen(params[1]) + 1);
		strcpy(buf, params[1]);


		void **new_params = malloc(sizeof(void*) * 3);

		new_params[0] = srv;
		new_params[1] = (int*) numeric;
		new_params[2] = buf;

		execute_callbacks(CALLBACK_GOT_NUMERIC, new_params, 3);

		free(numeric);
		free(new_params);
		free(buf);
	}
	return BCIRC_PLUGIN_OK;
}



int get_chan_event(void **params, int argv)
{
	if (!params[0] || !params[1])
		return BCIRC_PLUGIN_BREAK;

	size_t buf_len = strlen(params[1]) + 1;

	if (buf_len <= 20) //This is strange but some times happens in channel where are many users and leads to seg fault.
	{
		if (strstr(params[1], "PING"))
			return BCIRC_PLUGIN_CONTINUE;
		bcirc_printf("Buf is: %s\n", params[1]);
		//bcirc_print_callstack();
		return BCIRC_PLUGIN_CONTINUE;
	}

	server *srv = params[0];

	if (srv->motd_sent == 0)
		return 0;

	char *str = malloc(buf_len);
	strcpy(str, params[1]);

	char buf_orig[buf_len];
	strcpy(buf_orig, params[1]);

	char *save, *token;

	char *nick = NULL, *hostmask = NULL;
	char *reason = NULL;
	channel *chan = NULL;

	int event_type = 0; // 0 = part, 1 = kick

	char to_tokenized[1024];
	strcpy(to_tokenized, str);

	token = strtok_r(to_tokenized, " ", &save);

	//joona!~joona@127.0.0.1 PART #tesm :WeeChat 1.2
	for (int i = 0; token != NULL; i++)
	{
		if (i == 1)
		{
			if (strcmp(token, "JOIN") == 0)
				event_type = CHAN_JOIN;
			else if (strcmp(token, "KICK") == 0)
				event_type = CHAN_KICK;
			else if (strcmp(token, "PART") == 0)
				event_type = CHAN_PART;
			else if (strcmp(token, "QUIT") == 0)
				event_type = CHAN_QUIT;
			else if (strcmp(token, "INVITE") == 0)
				event_type = CHAN_INVITE;
			else
			{
				//bcirc_printf("No action\n");
				free(str);
				return BCIRC_PLUGIN_OK;
			}

			char *nick_end = memchr(buf_orig, '!', strlen(buf_orig));

			if (nick_end == NULL)
			{
				bcirc_printf("Can't find end of nick..\n");
				free(str);
				return BCIRC_PLUGIN_CONTINUE;
			}

			size_t nick_len = strlen(buf_orig) - strlen(nick_end) - 1;

			nick = malloc(nick_len + 1);
			memmove(nick, buf_orig+1, nick_len);
			nick[nick_len] = '\0';

			char *mask_end = memchr(buf_orig, ' ', strlen(buf_orig));

			if (mask_end == NULL)
			{
				bcirc_printf("Can't find end of nick..\n");
				free(str);
				free(nick);
				return BCIRC_PLUGIN_CONTINUE;
			}

			size_t mask_len = strlen(buf_orig) - strlen(mask_end) - nick_len - 2;

			hostmask = malloc(mask_len + 1);
			memmove(hostmask, buf_orig + nick_len + 2, mask_len);
			hostmask[mask_len] = '\0';

		}
		if (i == 2)
		{
			if (event_type == CHAN_INVITE)
				continue;
                
			chan = (channel*) get_channel(token, (server*) srv);
			if (!chan) {
				if (event_type == CHAN_QUIT) {
                    continue;
                }
                chan = create_channel(token,srv);
            }
			if (!chan || !chan->srv)
			{
				bcirc_printf("Failed get channel");
				return BCIRC_PLUGIN_CONTINUE;
			}
		}
		if (i == 3)
		{
			if (event_type == CHAN_INVITE)
			{
				chan = (channel*) get_channel(save, (server*) srv);
				if (!chan)
					chan = create_channel(save,srv);

				if (!chan || !chan->srv)
				{
					bcirc_printf("Failed get channel");
					return BCIRC_PLUGIN_CONTINUE;
				}

				break;
			}

			reason = malloc(( strlen(token) + 1 + strlen(save) + 1) * sizeof(char));
			if (event_type != CHAN_QUIT)
				strcpy(reason, token+1);
			else
				strcpy(reason, token);

			if (strlen(save) > 0) {
				strcat(reason, " ");
				strcat(reason, save);
			}

			break;
		}
		token = strtok_r(NULL, " ", &save);
	}

	void **params2 = malloc(5 * sizeof(void*));
	params2[0] = chan;
	params2[1] = nick;
	params2[2] = hostmask;
	params2[3] = reason;
    params2[4] = srv;

	if (event_type == CHAN_JOIN)
		execute_callbacks(CALLBACK_CHANNEL_JOIN, params2, 4);
	else if (event_type == CHAN_KICK)
		execute_callbacks(CALLBACK_CHANNEL_KICK, params2, 4);
	else if (event_type == CHAN_PART)
		execute_callbacks(CALLBACK_CHANNEL_PART, params2, 4);
	else if (event_type == CHAN_QUIT)
		execute_callbacks(CALLBACK_CHANNEL_QUIT, params2, 4);
	else if (event_type == CHAN_INVITE)
		execute_callbacks(CALLBACK_CHANNEL_INVITE, params2, 4);

	free(str);
	free(nick);
	free(hostmask);
	free(reason);
	free(params2);

	return BCIRC_PLUGIN_OK;
}
