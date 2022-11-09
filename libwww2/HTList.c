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
extern int www2Trace;
#endif

HTList *HTList_new NOARGS
{
  HTList *newList = (HTList *)malloc(sizeof(HTList));

  if (!newList)
    outofmem(__FILE__, "HTList_new");
  newList->object = NULL;
  newList->next = NULL;
  return newList;
}

void HTList_delete ARGS1(HTList *, me)
{
  HTList *current;

  while (current = me) {
    me = me->next;
    free (current);
  }
}

void HTList_addObject ARGS2(HTList *, me, void *, newObject)
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
  else if (www2Trace) {
    fprintf(stderr, "HTList: Trying to add object %p to a nonexisting list\n",
	    newObject);
  }
#endif
}

void HTList_addObjectAtEnd ARGS2(HTList *, me, void *, newObject)
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
  else if (www2Trace) {
    fprintf(stderr, "HTList: Trying to add object %p to a nonexisting list\n",
	    newObject);
  }
#endif
}

BOOL HTList_removeObject ARGS2(HTList *, me, void *, oldObject)
{
  if (me) {
    HTList *previous;

    while (me->next) {
      previous = me;
      me = me->next;
      if (me->object == oldObject) {
	previous->next = me->next;
	free (me);
	return YES;  /* Success */
      }
    }
  }
  return NO;  /* object not found or NULL list */
}

void *HTList_removeLastObject ARGS1 (HTList *, me)
{
  if (me && me->next) {
    HTList *lastNode = me->next;
    void *lastObject = lastNode->object;

    me->next = lastNode->next;
    free (lastNode);
    return lastObject;
  } else {  /* Empty list */
    return NULL;
  }
}

void *HTList_removeFirstObject ARGS1 (HTList *, me)
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

int HTList_count ARGS1 (HTList *, me)
{
  int count = 0;

  if (me)
    while (me = me->next)
      count++;
  return count;
}

int HTList_indexOf ARGS2(HTList *, me, void *, object)
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

void *HTList_objectAt ARGS2(HTList *, me, int, position)
{
  if (position < 0)
    return NULL;
  if (me) {
    while (me = me->next) {
      if (position == 0)
	return me->object;
      position--;
    }
  }
  return NULL;  /* Reached the end of the list */
}

/*	Remove object at a given position in the list, where 0 is the
**	object pointed to by the head (returns a pointer to the element
**	(->object) for the object, and NULL if the list is empty, or
**	if it doesn't exist - Yuk!).
*/
PUBLIC void *HTList_removeObjectAt ARGS2(HTList *, me,	int, position)
{
    HTList *temp = me;
    HTList *prevNode;
    int pos = position;

    if (!temp || pos < 0)
	return NULL;

    prevNode = temp;
    while ((temp = temp->next)) {
	if (pos == 0) {
	    prevNode->next = temp->next;
	    prevNode = temp;
	    free(temp);
	    return prevNode->object;
	}
	prevNode = temp;
	pos--;
    }

    return NULL;  /* Reached the end of the list */
}

/*	Insert an object into the list at a specified position.
**      If position is 0, this places the object at the head of the list
**      and is equivalent to HTList_addObject().
*/
PUBLIC void HTList_insertObjectAt ARGS3(
	HTList *,	me,
	void *,		newObject,
	int,		pos)
{
    HTList *newNode;
    HTList *temp = me;
    HTList *prevNode;
    int Pos = pos;

    if (!temp) {
#ifndef DISABLE_TRACE
	if (www2Trace)
	    fprintf(stderr,
		    "HTList: Trying to add object %p to a nonexisting list\n",
		    newObject);
#endif
	return;
    }
    if (Pos < 0) {
	Pos = 0;
#ifndef DISABLE_TRACE
	if (www2Trace)
	    fprintf(stderr,
		    "HTList: Treating negative object position %d as %d.\n",
		    pos, Pos);
#endif
    }

    prevNode = temp;
    while ((temp = temp->next)) {
	if (Pos == 0) {
	    if ((newNode = (HTList *)calloc(1, sizeof(HTList))) == NULL)
	        outofmem(__FILE__, "HTList_addObjectAt");
	    newNode->object = newObject;
	    newNode->next = temp;
	    if (prevNode)
	        prevNode->next = newNode;
	    return;
	}
	prevNode = temp; 
	Pos--;
    }
    if (Pos >= 0)
        HTList_addObject(prevNode, newObject);

    return;
}
