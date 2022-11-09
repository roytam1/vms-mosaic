/*	A small List class					      HTList.c
**	==================
**
**	A list is represented as a sequence of linked nodes of type HTList.
**	The first node is a header which contains no object.
**	New nodes are inserted between the header and the rest of the list.
*/
#include "../config.h"
#include "HTList.h"

#include <stdio.h>				/* joe@athena, TBL 921019 */

#ifndef DISABLE_TRACE
extern int reportBugs;
#endif

HTList *HTList_new (void)
{
  HTList *newList = (HTList *)calloc(1, sizeof(HTList));

  if (!newList)
    outofmem(__FILE__, "HTList_new");
  /** calloc does it
  newList->object = NULL;
  newList->next = NULL;
  **/
  return newList;
}

void HTList_delete (HTList *me)
{
  HTList *current;

  while (current = me) {
    me = me->next;
    free(current);
  }
}

void HTList_addObject (HTList *me, void *newObject)
{
  if (me) {
    HTList *newNode = (HTList *)malloc(sizeof(HTList));

    if (!newNode)
      outofmem(__FILE__, "HTList_addObject");
    newNode->object = newObject;
    newNode->next = me->next;
    me->next = newNode;
  }
#ifndef DISABLE_TRACE
  else if (reportBugs) {
    fprintf(stderr, "HTList: Trying to add object %p to a nonexisting list\n",
	    newObject);
  }
#endif
}

void HTList_addObjectAtEnd (HTList *me, void *newObject)
{
  if (me) {
    HTList *newNode = (HTList *)malloc(sizeof(HTList));

    if (!newNode)
      outofmem(__FILE__, "HTList_addObject");
    newNode->object = newObject;
    newNode->next = NULL;
    while (me->next) 
      me = me->next;
    me->next = newNode;
  }
#ifndef DISABLE_TRACE
  else if (reportBugs) {
    fprintf(stderr, "HTList: Trying to add object %p to a nonexisting list\n",
	    newObject);
  }
#endif
}

BOOL HTList_removeObject (HTList *me, void *oldObject)
{
  if (me) {
    HTList *previous;

    while (me->next) {
      previous = me;
      me = me->next;
      if (me->object == oldObject) {
	previous->next = me->next;
	free(me);
	return YES;  /* Success */
      }
    }
  }
  return NO;  /* object not found or NULL list */
}

void *HTList_removeLastObject (HTList *me)
{
  if (me && me->next) {
    HTList *lastNode = me->next;
    void *lastObject = lastNode->object;

    me->next = lastNode->next;
    free(lastNode);
    return lastObject;
  } else {  /* Empty list */
    return NULL;
  }
}

void *HTList_removeFirstObject (HTList *me)
{
  if (me && me->next) {
    HTList *prevNode;
    void *firstObject;

    while (me->next) {
      prevNode = me;
      me = me->next;
    }
    firstObject = me->object;
    prevNode->next = NULL;
    free(me);
    return firstObject;
  } else {  /* Empty list */
    return NULL;
  }
}

int HTList_count (HTList *me)
{
  int count = 0;

  if (me) {
    while (me = me->next)
      count++;
  }
  return count;
}

int HTList_indexOf (HTList *me, void *object)
{
  if (me) {
    int position = 0;

    while (me = me->next) {
      if (me->object == object)
	return position;
      position++;
    }
  }
  return -1;  /* Object not in the list */
}

void *HTList_objectAt (HTList *me, int position)
{
  if (!me || (position < 0))
    return NULL;

  while (me = me->next) {
    if (!position--)
       return me->object;
  }
  return NULL;  /* Reached the end of the list */
}

/*	Remove object at a given position in the list, where 0 is the
**	object pointed to by the head (returns a pointer to the element
**	(->object) for the object, and NULL if the list is empty, or
**	if it doesn't exist - Yuk!).
*/
PUBLIC void *HTList_removeObjectAt (HTList *me, int position)
{
    HTList *previous = me;

    if (!me || (position < 0))
	return NULL;

    while ((me = me->next)) {
	if (!position--) {
	    void *object = me->object;

	    previous->next = me->next;
	    free(me);
	    return object;
	}
	previous = me;
    }
    return NULL;  /* Reached the end of the list */
}

/*	Insert an object into the list at a specified position.
**      If position is 0, this places the object at the head of the list
**      and is equivalent to HTList_addObject().
*/
PUBLIC void HTList_insertObjectAt (HTList *me, void *newObject, int pos)
{
    HTList *newNode, *prevNode;
    HTList *temp = me;
    int Pos = pos;

    if (!temp) {
#ifndef DISABLE_TRACE
	if (reportBugs)
	    fprintf(stderr,
		    "HTList: Trying to add object %p to a nonexisting list\n",
		    newObject);
#endif
	return;
    }
    if (Pos < 0) {
	Pos = 0;
#ifndef DISABLE_TRACE
	if (reportBugs)
	    fprintf(stderr,
		    "HTList: Treating negative position %d as 0.\n", pos);
#endif
    }
    prevNode = temp;
    while ((temp = temp->next)) {
	if (!Pos--) {
	    if (!(newNode = (HTList *)malloc(sizeof(HTList))))
	        outofmem(__FILE__, "HTList_addObjectAt");
	    newNode->object = newObject;
	    newNode->next = temp;
	    if (prevNode)
	        prevNode->next = newNode;
	    return;
	}
	prevNode = temp; 
    }
    if (Pos >= 0)
        HTList_addObject(prevNode, newObject);
    return;
}
