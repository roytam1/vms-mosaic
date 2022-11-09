/*			Atoms: Names to numbers			HTAtom.c
**			=======================
**
**	Atoms are names which are given representative pointer values
**	so that they can be stored more efficiently, and comparisons
**	for equality done more efficiently.
**
**	Atoms are kept in a hash table consisting of an array of linked lists.
**
** Authors:
**	TBL	Tim Berners-Lee, WorldWideWeb project, CERN
**	(c) Copyright CERN 1991 - See Copyright.html
**
*/
#include "../config.h"
#define HASH_SIZE	101		/* Tunable */
#include "HTAtom.h"

#include <stdio.h>			/* joe@athena, TBL 921019 */
#include "HTUtils.h"
#include "tcp.h"

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

PRIVATE HTAtom *hash_table[HASH_SIZE];
PRIVATE BOOL initialised = NO;

PUBLIC HTAtom *HTAtom_for(char *string)
{
    int hash = 0;
    char *p;
    HTAtom *a;

    /* Bug hack. */
    if (!string)
        string = strdup("blargh");
    
    /*		First time around, clear hash table
    */
    if (!initialised) {
        int i;

	for (i = 0; i < HASH_SIZE; i++)
	    hash_table[i] = NULL;
	initialised = YES;
    }
    
    /*		Generate hash function
    */
    for (p = string; *p; p++)
        hash = (hash * 3 + *p) % HASH_SIZE;
    
    /*		Search for the string in the list
    */
    for (a = hash_table[hash]; a; a = a->next) {
	if (!strcmp(a->name, string)) {
#ifndef DISABLE_TRACE
    	    if (www2Trace)
		fprintf(stderr,	"HTAtom: Found atom for `%s'\n", string);
#endif
	    return a;			/* Found: return it */
	}
    }
    
    /*		Generate a new entry
    */
    if (!(a = (HTAtom *)malloc(sizeof(*a))))
	outofmem(__FILE__, "HTAtom_for");
    if (!(a->name = (char *)malloc(strlen(string) + 1)))
	outofmem(__FILE__, "HTAtom_for");
    strcpy(a->name, string);
    a->next = hash_table[hash];		/* Put onto the head of list */
    hash_table[hash] = a;
#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr, "HTAtom: Created atom for `%s'\n", string);
#endif
    return a;
}


PUBLIC HTAtom *HTAtom_exists(char *string)
{
    int hash = 0;
    char *p;
    HTAtom *a;
    
    if (!initialised)
        return NULL;
    
    /*		Generate hash function
    */
    for (p = string; *p; p++)
        hash = (hash * 3 + *p) % HASH_SIZE;
    
    /*		Search for the string in the list
    */
    for (a = hash_table[hash]; a; a = a->next) {
	if (!strcmp(a->name, string))
	    return a;				/* Found: return it */
    }
    return NULL;
}
