/*                             String handling for libwww
                                         STRINGS
                                             
   String allocations with copies, etc.
   
 */
#ifndef HTSTRING_H
#define HTSTRING_H

#include "HTUtils.h"

extern WWW_CONST char *HTLibraryVersion;   /* String for help screen etc */

/*
 * Malloced string manipulation
 */
#define StrAllocCopy(dest, src) HTSACopy(&(dest), src)
#define StrAllocCat(dest, src)  HTSACat(&(dest), src)
extern char *HTSACopy PARAMS ((char **dest, WWW_CONST char *src));
extern char *HTSACat  PARAMS ((char **dest, WWW_CONST char *src));

/*
 * Next word or quoted string
 */
extern char *HTNextField PARAMS ((char **pstr));


#endif
