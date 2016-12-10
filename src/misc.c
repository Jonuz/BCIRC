#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>

#include "../headers/misc.h"
#include "../headers/log.h"
#include "../headers/server.h"


//http://man7.org/linux/man-pages/man3/backtrace.3.html#NOTES
void bcirc_print_callstack()
{
	pthread_mutex_lock(&servers_global_mutex);

	int j, nptrs;
	void *buffer[512];
	char **strings;

	nptrs = backtrace(buffer, 1024);
	printf("backtrace() returned %d addresses\n", nptrs);

	strings = backtrace_symbols(buffer, nptrs);
	if (strings == NULL) {
		perror("backtrace_symbols");
		pthread_mutex_unlock(&servers_global_mutex);
		return;
	}

	for (j = 0; j < nptrs; j++)
		bcirc_printf("%s\n", strings[j]);

	free(strings);

	pthread_mutex_unlock(&servers_global_mutex);

	return;
}
