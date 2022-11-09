#ifndef HTML_H
#define HTML_H

#ifndef HTUTILS_H
#include "HTUtils.h"
#endif
#ifndef HTANCHOR_H
#include "HTAnchor.h"
#endif
#ifndef HTMLDTD_H
#include "HTMLDTD.h"
#endif

extern WWW_CONST HTStructuredClass HTMLPresentation;

extern HTStructured *HTML_new (HTParentAnchor *anchor,
        		       HTFormat        format_out,
        		       HTStream       *stream);

/*      Names for selected internal representations:
*/
typedef enum _HTMLCharacterSet {
        HTML_ISO_LATIN1,
        HTML_NEXT_CHARS,
        HTML_PC_CP950
} HTMLCharacterSet;

extern void HTMLUseCharacterSet (HTMLCharacterSet i);

/*

Record error message as a hypertext object

   The error message should be marked as an error so that it can be
   reloaded later.  This implementation just throws up an error message
   and leaves the document unloaded.
   
 */
/* On entry,
**      sink    is a stream to the output device if any
**      number  is the HTTP error number
**      message is the human readable message.
** On exit,
**      a retrun code like HT_LOADED if object exists else 60; 0
*/

extern int HTLoadError (HTStream *sink, int number, WWW_CONST char *message);

#endif
