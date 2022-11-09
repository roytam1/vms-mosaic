#ifndef __INET_LOADED
#define __INET_LOADED	1

#include <in.h>

/* Some of these functions are not properly prototyped because
   the interface is poorly defined, and many programs use int's
   and struct in_addr's interchangeably. */
int inet_addr(char *cp);
int inet_network(char *cp);
/* char *inet_ntoa(struct in_addr in); */
char *inet_ntoa();
struct in_addr inet_makeaddr(int net, int lna);
/* int inet_lnaof(struct in_addr in); */
int inet_lnaof();
/* int inet_netof(struct in_addr in); */
int inet_netof();

#endif					/* __INET_LOADED */
