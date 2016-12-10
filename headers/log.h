#ifndef LOG_H_
#define LOG_H

int bcirc_escape_buf(char *str, char *new_str);


int bcirc_printf(char *str, ...);
int bcirc_log(char *str, char *file, ...);

#endif
