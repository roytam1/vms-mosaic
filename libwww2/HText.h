/*
 *                                RICH HYPERTEXT OBJECT
 */

#ifndef HTEXT_H
#define HTEXT_H

#ifndef HTANCHOR_H
#include "HTAnchor.h"
#endif
#ifndef HTSTREAM_H
#include "HTStream.h"
#endif

typedef struct _HText HText;

extern HText *HTMainText;              /* Pointer to current main text */

/*                      Creation and deletion
**
**      Create hypertext object
*/
extern HText *HText_new();

/*                      Object Building methods
**                      -----------------------
**
**      These are used by a parser to build the text in an object.
**      HText_beginAppend must be called, then any combination of other
**      append calls, then HText_endAppend.  This allows optimised
**      handling using buffers and caches which are flushed at the end.
*/
extern void HText_beginAppend(HText *text);

extern void HText_endAppend(HText *text);
extern void HText_doAbort(HText *text);
extern void HText_clearOutForNewContents(HText *text);

/*      Add one character
*/
extern void HText_appendCharacter(HText *text, char ch);

/*      Add a zero-terminated string
*/
extern void HText_appendText(HText *text, WWW_CONST char *str);

/*      Add a block.
*/
extern void HText_appendBlock(HText *text, WWW_CONST char *str, int len);

/*      Start/end sensitive text
**
** The anchor object is created and passed to HText_beginAnchor.
** The senstive text is added to the text object, and then HText_endAnchor
** is called.  Anchors may not be nested.
*/

extern void HText_beginAnchor(HText *text, char *anc);
extern void HText_endAnchor(HText *text);

extern char *HText_getText(HText *me);
extern int HText_getTextLength(HText *me);

#endif  /* HTEXT_H */
