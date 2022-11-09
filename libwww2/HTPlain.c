/*		Plain text object		HTPlain.c
**		=================
**
**	This version of the stream object just writes to a socket.
**	The socket is assumed open and left open.
**
**	Bugs:
**		strings written must be less than buffer size.
*/
#include "../config.h"
#include "HTPlain.h"
#include "HTUtils.h"
#include "HText.h"
#include "HTFile.h"
#include "HTCompressed.h"

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/*		HTML Object
**		-----------
*/

struct _HTStream {
	WWW_CONST HTStreamClass	*isa;
	HText *text;
        int compressed;
	int interrupted;
};

/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/

PRIVATE void HTPlain_put_character (HTStream *me, char c)
{
  HText_appendCharacter(me->text, c);
}


/*	String handling
**	---------------
**
*/
PRIVATE void HTPlain_put_string (HTStream *me, WWW_CONST char *s)
{
  HText_appendText(me->text, s);
}


PRIVATE void HTPlain_write (HTStream *me, WWW_CONST char *s, int l)
{
  HText_appendBlock(me->text, s, l);
}


/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created object is
**	not, as it takes on an existence of its own unless explicitly freed.
*/
PRIVATE void HTPlain_free (HTStream *me)
{
  if ((me->compressed != COMPRESSED_NOT) && !me->interrupted) {
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "[HTPlain_free] going to decompress HText\n");
#endif
      HTCompressedHText(me->text, me->compressed, 1);
  }

  free(me);
}

/*	End writing
*/

PRIVATE void HTPlain_end_document (HTStream *me)
{
  HText_endAppend(me->text);
}

PRIVATE void HTPlain_handle_interrupt (HTStream *me)
{
  HText_doAbort(me->text);
  me->interrupted = 1;
}


/*		Structured Object Class
**		-----------------------
*/
PRIVATE WWW_CONST HTStreamClass HTPlain = {
	"SocketWriter",
	HTPlain_free,
	HTPlain_end_document,
	HTPlain_put_character, HTPlain_put_string, HTPlain_write,
        HTPlain_handle_interrupt
}; 


/*		New object
**		----------
*/
PUBLIC HTStream *HTPlainPresent (HTPresentation *pres,
				 HTParentAnchor *anchor,	
				 HTStream *sink,
        			 HTFormat format_in,
        			 int compressed)
{
  HTStream *me = (HTStream *)malloc(sizeof(*me));

  me->isa = &HTPlain;       
  
#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf(stderr, "[HTPlainPresent] format_in is '%s' and compressed is %d\n",
	    HTAtom_name(format_in), compressed);
#endif  
  me->text = HText_new();
  me->compressed = compressed;
  me->interrupted = 0;

  HText_beginAppend(me->text);
  if (me->compressed == COMPRESSED_NOT)
    HText_appendText(me->text, "<PLAINTEXT>\n");

  return (HTStream *)me;
}
