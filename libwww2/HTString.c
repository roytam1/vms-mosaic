/*		String manipulation routines		HTString.c
**
**	Original version came with listserv implementation.
**	Version TBL Oct 91 replaces one which modified the strings.
**	02-Dec-91 (JFG) Added stralloccopy and stralloccat
**	23 Jan 92 (TBL) Changed strallocc* to 8 char HTSAC* for VM and suchlike
**	 6 Oct 92 (TBL) Moved WWW_TraceFlag in here to be in library
*/
#include "../config.h"
#include <ctype.h>
#include "HTUtils.h"

PUBLIC WWW_CONST char *HTLibraryVersion = "2.12_Mosaic";

/*
 *	Allocate a new copy of a string, and returns it
 */
PUBLIC char *HTSACopy (char **dest, WWW_CONST char *src)
{
    if (!dest)
        return NULL;
    if (*dest)
        free(*dest);
    if (!src) {
        *dest = NULL;
    } else {
        *dest = (char *) malloc(strlen(src) + 1);
        if (!*dest)
            outofmem(__FILE__, "HTSACopy");
        strcpy(*dest, src);
    }
    return *dest;
}

/*
 *	String Allocate and Concatenate
 */
PUBLIC char *HTSACat (char **dest, WWW_CONST char *src)
{
    if (src && *src) {
        if (*dest) {
            int length = strlen(*dest);

            *dest = (char *) realloc(*dest, length + strlen(src) + 1);
            if (!*dest)
	        outofmem(__FILE__, "HTSACat");
            strcpy(*dest + length, src);
        } else {
            *dest = (char *) malloc(strlen(src) + 1);
            if (!*dest)
	        outofmem(__FILE__, "HTSACat");
            strcpy(*dest, src);
        }
    }
    return *dest;
}


/*	Find next Field
**	---------------
**
** On entry,
**	*pstr	points to a string containing white space separated
**		field, optionally quoted.
**
** On exit,
**	*pstr	has been moved to the first delimiter past the field.
**		THE STRING HAS BEEN MUTILATED by a 0 terminator
**
**	returns	a pointer to the first field
*/
PUBLIC char *HTNextField (char **pstr)
{
    char *p = *pstr;
    char *start;		/* Start of field */
    
    while (*p && WHITE(*p))
	p++;			/* Strip white space */
    if (!*p) {
	*pstr = p;
        return NULL;		/* No first field */
    }
    if (*p == '"') {		/* Quoted field */
	start = ++p;
	for (; *p && *p != '"'; p++) {
	    if (*p == '\\' && p[1])
		p++;		/* Skip escaped chars */
	}
    } else {
	start = p;
	while (*p && !WHITE(*p))
	    p++;		/* Skip first field */
    }
    if (*p)
	*p++ = '\0';
    *pstr = p;
    return start;
}
