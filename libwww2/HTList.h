
/*              List object
**
**      The list object is a generic container for storing collections
**      of things in order.
*/
#ifndef HTLIST_H
#define HTLIST_H

#ifndef HTUTILS_H
#include "HTUtils.h"  /* for BOOL type */
#endif

typedef struct _HTList HTList;

struct _HTList {
  void   *object;
  HTList *next;
};

extern HTList  *HTList_new ();
extern void     HTList_delete (HTList *me);

/*      Add object to START of list
*/
extern void     HTList_addObject (HTList *me, void *newObject);
extern void     HTList_addObjectAtEnd (HTList *me, void *newObject);

extern BOOL     HTList_removeObject (HTList *me, void *oldObject);
extern void    *HTList_removeObjectAt (HTList *me, int position);

extern void     HTList_insertObjectAt (HTList *me, void *newObject, int pos);
extern void    *HTList_removeLastObject (HTList *me);
extern void    *HTList_removeFirstObject (HTList *me);

#define         HTList_isEmpty(me) (me ? me->next == NULL : YES)

extern int      HTList_count (HTList *me);
extern int      HTList_indexOf (HTList *me, void *object);

#define         HTList_lastObject(me) \
  (me && me->next ? me->next->object : NULL)

extern void    *HTList_objectAt (HTList *me, int position);

/* Fast macro to traverse the list.  Call it first with copy of list header :
 * it returns the first object and increments the passed list pointer.
 * Call it with the same variable until it returns NULL. */
#define HTList_nextObject(me) \
  (me && (me = me->next) ? me->object : NULL)

#endif  /* HTLIST_H */
