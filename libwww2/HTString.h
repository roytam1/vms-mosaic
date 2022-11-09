/*                             String handling for libwww
                                         STRINGS
                                             
   String allocations with copies, etc.
   
 */
#ifndef HTSTRING_H
#define HTSTRING_H

#ifndef HTUTILS_H
#include "HTUtils.h"
#endif

extern WWW_CONST char *HTLibraryVersion;   /* String for help screen etc */

/*
 * Malloced string manipulation
 */
#define StrAllocCopy(dest, src) HTSACopy(&(dest), src)
#define StrAllocCat(dest, src) HTSACat(&(dest), src)
extern char *HTSACopy (char **dest, WWW_CONST char *src);
extern char *HTSACat (char **dest, WWW_CONST char *src);

/*
 * Next word or quoted string
 */
extern char *HTNextField (char **pstr);


#endif
