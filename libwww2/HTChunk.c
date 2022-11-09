/*		Chunk handling:	Flexible arrays
**		===============================
**
*/
#include "../config.h"
#include "HTUtils.h"
#include "HTChunk.h"
#include <stdio.h>

/*	Create a chunk with a certain allocation unit
**	--------------
*/
PUBLIC HTChunk *HTChunkCreate (int growby)
{
    HTChunk *ch = (HTChunk *) calloc(1, sizeof(HTChunk));

    if (!ch)
	outofmem(__FILE__, "creation of chunk");

    ch->growby = growby;
    /** calloc zeros
    ch->data = 0;
    ch->size = 0;
    ch->allocated = 0;
    **/
    return ch;
}


/*	Clear a chunk of all data
**	--------------------------
*/
PUBLIC void HTChunkClear (HTChunk *ch)
{
    if (ch->data) {
	free(ch->data);
	ch->data = 0;
    }
    ch->size = 0;
    ch->allocated = 0;
}


/*	Free a chunk
**	------------
*/
PUBLIC void HTChunkFree (HTChunk *ch)
{
    if (ch->data)
	free(ch->data);
    free(ch);
}


/*	Append a character
**	------------------
*/
PUBLIC void HTChunkPutc (HTChunk *ch, char c)
{
    if (ch->size >= ch->allocated) {
	ch->allocated = ch->allocated + ch->growby;
        ch->data = ch->data ? (char *)realloc(ch->data, ch->allocated) :
			      (char *)malloc(ch->allocated);
        if (!ch->data)
	    outofmem(__FILE__, "HTChunkPutc");
    }
    ch->data[ch->size++] = c;
}


/*	Ensure a certain size
**	---------------------
*/
PUBLIC void HTChunkEnsure (HTChunk *ch, int needed)
{
    if (needed <= ch->allocated)
	return;
    							     /* Round up */
    ch->allocated = needed - 1 - ((needed - 1) % ch->growby) + ch->growby;
    ch->data = ch->data ? (char *)realloc(ch->data, ch->allocated) :
			  (char *)malloc(ch->allocated);
    if (!ch->data)
	outofmem(__FILE__, "HTChunkEnsure");
}


/*	Terminate a chunk
**	-----------------
*/
PUBLIC void HTChunkTerminate (HTChunk *ch)
{
    HTChunkPutc(ch, '\0');
}


/*	Append a string
**	---------------
*/
PUBLIC void HTChunkPuts (HTChunk *ch, WWW_CONST char *str)
{
    WWW_CONST char *p;

    for (p = str; *p; p++)
        HTChunkPutc(ch, *p);
}
