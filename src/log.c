#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

 #include <sys/types.h>
 #include <sys/stat.h>

#include "../headers/log.h"

#pragma GCC diagnostic ignored "-Wvarargs" //but it works


int bcirc_escape_buf(char *str, char *new_str)
{
	#define MAX_LEN 2048

	char str_copy[MAX_LEN];

	//int str_len = strlen(str);

	int i = 0;
	int str_copy_len = 0;

	while (str[i] != '\0')
	{
		if (str_copy_len >= MAX_LEN)
			return -1;

		//if (i == str_len)
		//	break;

		str_copy[str_copy_len] = str[i];
		str_copy_len++;

		if (str[i] == '%')
		{
			str_copy[str_copy_len] = '%';
			str_copy_len++;
			if (str[i+1] == '%')
				i++;
		}
		i++;
	}
	str_copy[str_copy_len] = '\0';

	strcpy(new_str, str_copy);

	return 1;
}

int bcirc_printf(char *str, ...)
{
	char *output = NULL;
	size_t len = 0;

	size_t size = (strlen(str) + 1) * sizeof(char);

	va_list args;
	va_start(args, size);
	len = vsnprintf(0, 0, str, args);
	va_end(args);

	output = malloc(len + 1);

	va_start(args, size);
	vsnprintf(output, len+1, str, args);
	va_end(args);


	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	const int max_time_len = 20;
	char time_prefix[max_time_len];

	strftime(time_prefix, max_time_len, "%H:%M:%S", timeinfo);

	char *new_output = malloc(strlen(output) + 3 + strlen(time_prefix) + 1); //hh:mm:ss | text\n\0
	sprintf(new_output, "%s | %s", time_prefix, output);

	if (!new_output)
	{
		return -1;
	}
	free(output);
	output = malloc((strlen(new_output) + 1) * (sizeof(char)));
	strcpy(output, new_output);

	bcirc_log(output, NULL);

	printf(output);
	free(output);

	return 1;
}

int bcirc_log(char *str, char *file, ...)
{
	char *output = NULL;
	size_t len = 0;

	size_t size = (strlen(str) + 1) * sizeof(char);

	va_list args;
	va_start(args, size);
	len = vsnprintf(0, 0, str, args);
	va_end(args);

	output = malloc(len + 1);

	va_start(args, size);
	vsnprintf(output, len+1, str, args);
	va_end(args);

	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	const int max_time_len = 20;

	char log_name_time[max_time_len]; //dd.mm.yy
	strftime(log_name_time, 20, "%d-%m-%Y", timeinfo);

	char *log_dir = getenv("BCIRC_LOG_DIR");
	if (!log_dir)
	{
		bcirc_printf("BCIRC_LOG_DIR is not set!\n");
		return -1;
	}

	char file2[8];
	if (file == NULL)
	{
		strcpy(file2, "mainlog");
		file = file2;
	}

	// /logdir/logname.dd.mm.yy.log
	char *file_to_write = malloc((strlen(log_dir) + 1 + strlen(file) + 1 + strlen(log_name_time) + 4 + 1) * sizeof(char));
	sprintf(file_to_write, "%s/%s.%s.log", log_dir, file, log_name_time);


	struct stat sb;
	if (stat(log_dir, &sb))
	{
		int status = mkdir(log_dir, S_IRWXU);

		if (status != 0)
		{
			 printf("failed to create dir %s, errno: %d\n", log_dir, errno);
			 exit(EXIT_FAILURE);
		}
	}

	FILE *logfile = fopen(file_to_write, "a");
	fprintf(logfile, "%s", output);
	fclose(logfile);

	free(file_to_write);
	free(output);

	return 1;
}
