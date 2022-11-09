/*                       ASSOCIATION LIST FOR STORING NAME-VALUE PAIRS
                                             
   Lookups from assosiation list are not case-sensitive.
   
 */

#ifndef HTASSOC_H
#define HTASSOC_H

#ifndef HTUTILS_H
#include "HTUtils.h"
#endif
#ifndef HTLIST_H
#include "HTList.h"
#endif

typedef HTList HTAssocList;

typedef struct {
    char *name;
    char *value;
} HTAssoc;

PUBLIC HTAssocList *HTAssocList_new ();
PUBLIC void HTAssocList_delete (HTAssocList *alist);

PUBLIC void HTAssocList_add (HTAssocList    *alist,
                             WWW_CONST char *name,
                             WWW_CONST char *value);

PUBLIC char *HTAssocList_lookup (HTAssocList *alist, WWW_CONST char *name);

#endif  /* HTASSOC_H */
