/* Copyright (C) 2004, 2005, 2007 - The VMS Mosaic Project */

#ifndef HTCOOKIES_H
#define HTCOOKIES_H

#ifndef HTLIST_H
#include "HTList.h"
#endif

extern void HTSetCookie (WWW_CONST char *SetCookie,
			 WWW_CONST char *SetCookie2,
			 WWW_CONST char *address);

extern char *HTCookie (WWW_CONST char *hostname,
		       WWW_CONST char *path,
		       int	       port,
		       BOOL	       secure);

extern void HTLoadCookies (char *cookie_file, char *perm_file);

extern void HTStoreCookies (char *cookie_file, char *perm_file);

typedef enum { ACCEPT_ALWAYS,
	       REJECT_ALWAYS,
	       QUERY_USER,
	       FROM_FILE
} behaviour_t;

typedef enum { INVCHECK_QUERY,
               INVCHECK_STRICT,
               INVCHECK_LOOSE
} invcheck_type;

typedef enum { FLAG_ACCEPT_ALWAYS,
               FLAG_REJECT_ALWAYS,
               FLAG_QUERY_USER,
               FLAG_FROM_FILE,
               FLAG_INVCHECK_QUERY,
               FLAG_INVCHECK_STRICT,
               FLAG_INVCHECK_LOOSE
} cookie_domain_flags;

struct _domain_entry {
    char       *domain;  /* Domain for which these cookies are valid */
    behaviour_t	bv;
    invcheck_type invcheck_bv;
    HTList     *cookie_list;
};

typedef struct _domain_entry domain_entry;

#endif  /* HTCOOKIES_H */
