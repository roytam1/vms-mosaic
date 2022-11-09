/* Copyright (C) 2005 - The VMS Mosaic Project */

#ifndef NUT_STR_TOOLS_H
#define NUT_STR_TOOLS_H

char *getFileName(char *file_src);
char *strcasechr(char *src, char srch);
char *strrcasechr(char *src, char srch);
char *strstrdup(char *src, char *srch, char *rplc);
char **string_to_token_array(char *str, char *delimiter);
char *my_strndup(char *str, int num);
char *my_chop(char *str);
int my_strcasecmp(char *, char *);
int my_strncasecmp(char *, char *, int);
int removeblanks(char *str);

#endif
