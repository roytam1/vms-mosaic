/****************************************************************************
 * NCSA Mosaic for the X Window System                                      *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 * mosaic@ncsa.uiuc.edu                                                     *
 *                                                                          *
 * Copyright (C) 1993, Board of Trustees of the University of Illinois      *
 *                                                                          *
 * NCSA Mosaic software, both binary and source (hereafter, Software) is    *
 * copyrighted by The Board of Trustees of the University of Illinois       *
 * (UI), and ownership remains with the UI.                                 *
 *                                                                          *
 * The UI grants you (hereafter, Licensee) a license to use the Software    *
 * for academic, research and internal business purposes only, without a    *
 * fee.  Licensee may distribute the binary and source code (if released)   *
 * to third parties provided that the copyright notice and this statement   *
 * appears on all copies and that no charge is associated with such         *
 * copies.                                                                  *
 *                                                                          *
 * Licensee may make derivative works.  However, if Licensee distributes    *
 * any derivative work based on or derived from the Software, then          *
 * Licensee will (1) notify NCSA regarding its distribution of the          *
 * derivative work, and (2) clearly notify users that such derivative       *
 * work is a modified version and not the original NCSA Mosaic              *
 * distributed by the UI.                                                   *
 *                                                                          *
 * Any Licensee wishing to make commercial use of the Software should       *
 * contact the UI, c/o NCSA, to negotiate an appropriate license for such   *
 * commercial use.  Commercial use includes (1) integration of all or       *
 * part of the source code into a product for sale or license by or on      *
 * behalf of Licensee to third parties, or (2) distribution of the binary   *
 * code or source code to third parties that need it to utilize a           *
 * commercial product sold or licensed by or on behalf of Licensee.         *
 *                                                                          *
 * UI MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR   *
 * ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED          *
 * WARRANTY.  THE UI SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE    *
 * USERS OF THIS SOFTWARE.                                                  *
 *                                                                          *
 * By using or copying this Software, Licensee agrees to abide by the       *
 * copyright law and all other applicable laws of the U.S. including, but   *
 * not limited to, export control laws, and the terms of this license.      *
 * UI shall have the right to terminate this license immediately by         *
 * written notice upon Licensee's breach of, or non-compliance with, any    *
 * of its terms.  Licensee may be held legally responsible for any          *
 * copyright infringement that is caused or encouraged by Licensee's        *
 * failure to abide by the terms of this license.                           *
 *                                                                          *
 * Comments and questions are welcome and can be sent to                    *
 * mosaic-x@ncsa.uiuc.edu.                                                  *
 ****************************************************************************/

/* Copyright (C) 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"
/*
 * The following XPM hashing code if from the libXpm code, which I
 * am free to use as long as I include the following copyright:
 */
/*
 * Copyright 1990-93 GROUPE BULL
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of GROUPE BULL not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  GROUPE BULL makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * GROUPE BULL disclaims all warranties with regard to this software,
 * including all implied warranties of merchantability and fitness,
 * in no event shall GROUPE BULL be liable for any special,
 * indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits,
 * whether in an action of contract, negligence or other tortious
 * action, arising out of or in connection with the use 
 * or performance of this software.
 *
 */
/*****************************************************************************\
*                                                                             *
*  XPM library                                                                *
*                                                                             *
*  Developed by Arnaud Le Hors                                                *
*  this originaly comes from Colas Nahaboo as a part of Wool                  *
*                                                                             *
\*****************************************************************************/

#include "xpm.h"
#include <stdlib.h>
#ifdef VMS
#include <string.h>
#endif

/* Make an atom */
static xpmHashAtom AtomMake(char *name, void *data)
{
    xpmHashAtom object = (xpmHashAtom) XpmMalloc(sizeof(struct _xpmHashAtom));

    if (object) {
	object->name = name;
	object->data = data;
    }
    return object;
}

/************************\
* 			 *
*  Hash table routines 	 *
* 			 *
\************************/

/*
 * Hash function definition:
 * HASH_FUNCTION: hash function, hash = hashcode, hp = pointer on char,
 *				 hash2 = temporary for hashcode.
 * INITIAL_TABLE_SIZE in slots
 * HASH_TABLE_GROWS how hash table grows.
 */

/* Mock lisp function */
#define HASH_FUNCTION 	  hash = (hash << 5) - hash + *hp++;
/* #define INITIAL_HASH_SIZE 2017 */
#define INITIAL_HASH_SIZE 256		/* Should be enough for colors */
#define HASH_TABLE_GROWS  size = size * 2;

/* aho-sethi-ullman's HPJ (sizes should be primes) */
#ifdef notdef
#define HASH_FUNCTION	hash <<= 4; hash += *hp++; \
    if (hash2 = hash & 0xf0000000) hash ^= (hash2 >> 24) ^ hash2;
#define INITIAL_HASH_SIZE 4095		/* Should be 2^n - 1 */
#define HASH_TABLE_GROWS  size = size << 1 + 1;
#endif

/* GNU emacs function */
/*
#define HASH_FUNCTION 	  hash = (hash << 3) + (hash >> 28) + *hp++;
#define INITIAL_HASH_SIZE 2017
#define HASH_TABLE_GROWS  size = size * 2;
*/

/* End of hash functions */

/*
 * The hash table is used to store atoms via their NAME:
 *
 * NAME --hash--> ATOM |--name--> "foo"
 *		       |--data--> any value which has to be stored
 *
 */

/*
 * xpmHashSlot gives the slot (pointer to xpmHashAtom) of a name
 * (slot points to NULL if it is not defined)
 */
xpmHashAtom *xpmHashSlot(xpmHashTable *table, char *s)
{
    xpmHashAtom *atomTable = table->atomTable;
    xpmHashAtom *p;
    unsigned int hash = 0;
    char *hp = s;
    char *ns;

    while (*hp) {			/* Computes hash function */
	HASH_FUNCTION
    }
    p = atomTable + hash % table->size;
    while (*p) {
	ns = (*p)->name;
	if (*ns == *s && !strcmp(ns, s))
	    break;
	if (--p < atomTable)
	    p = atomTable + table->size - 1;
    }
    return p;
}

static int HashTableGrows(xpmHashTable *table)
{
    xpmHashAtom *atomTable = table->atomTable;
    xpmHashAtom *t, *p, *ps;
    int size = table->size;
    int oldSize = size;
    int i;

    t = atomTable;
    HASH_TABLE_GROWS
    table->size = size;
    table->limit = size / 3;
    atomTable = (xpmHashAtom *) XpmCalloc(size, sizeof(*atomTable));
    if (!atomTable)
	return(XpmNoMemory);
    table->atomTable = atomTable;
    /** calloc zeros it
    for (p = atomTable + size; p > atomTable; )
	*--p = NULL;
    **/
    for (i = 0, p = t; i < oldSize; i++, p++) {
	if (*p) {
	    ps = xpmHashSlot(table, (*p)->name);
	    *ps = *p;
	}
    }
    XpmFree(t);
    return(XpmSuccess);
}

/*
 * an xpmHashAtom is created if name doesn't exist, with the given data.
 */
int xpmHashIntern(xpmHashTable *table, char *tag, void *data)
{
    xpmHashAtom *slot;

    if (!*(slot = xpmHashSlot(table, tag))) {
	/* Undefined, make a new atom with the given data */
	if (!(*slot = AtomMake(tag, data)))
	    return(XpmNoMemory);
	if (table->used >= table->limit) {
	    int ErrorStatus = HashTableGrows(table);

	    if (ErrorStatus != XpmSuccess)
		return(ErrorStatus);
	    table->used++;
	    return(XpmSuccess);
	}
	table->used++;
    }
    return(XpmSuccess);
}

/*
 *  Must be called before allocating any atom
 */
int xpmHashTableInit(xpmHashTable *table)
{
    xpmHashAtom *p, *atomTable;

    table->size = INITIAL_HASH_SIZE;
    table->limit = table->size / 3;
    table->used = 0;
    atomTable = (xpmHashAtom *) XpmCalloc(table->size, sizeof(*atomTable));
    if (!atomTable)
	return(XpmNoMemory);
    /** calloc zeros it
    for (p = atomTable + table->size; p > atomTable; )
	*--p = NULL;
    **/
    table->atomTable = atomTable;
    return(XpmSuccess);
}

/*
 *   Frees a hashtable and all the stored atoms
 */
void xpmHashTableFree(xpmHashTable *table)
{
    xpmHashAtom *p;
    xpmHashAtom *atomTable = table->atomTable;

    if (!atomTable)
	return;
    for (p = atomTable + table->size; p > atomTable; ) {
	if (*--p)
	    XpmFree(*p);
    }
    XpmFree(atomTable);
    table->atomTable = NULL;
}
