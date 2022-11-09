/*              MIME Parser                     HTMIME.h
**              -----------
**
**   The MIME parser stream presents a MIME document.
**
**
*/

#ifndef HTMIME_H
#define HTMIME_H

#ifndef HTSTREAM_H
#include "HTStream.h"
#endif
#ifndef HTANCHOR_H
#include "HTAnchor.h"
#endif

#ifdef MOTIF
#include <Xm/XmP.h>
# ifdef MOTIF1_2
#  include <Xm/ManagerP.h>
# endif /* MOTIF1_2 */
#else
#include <X11/IntrinsicP.h>
#include <X11/ConstrainP.h>
#endif /* MOTIF */

typedef struct mime_rec {
	char *last_modified;
	char *expires;
	char *refresh;
	char *charset;
} MIMEInfo;

extern int HTMIME_get_header_length(HTStream *me);
extern int HTMIME_get_content_length(HTStream *me);
extern Boolean HTMIME_header_done(HTStream *me);

extern HTStream *HTMIMEConvert(HTPresentation *pres,
                               HTParentAnchor *anchor,
                               HTStream       *sink,
                               HTFormat        format_in,
                               int             compressed);

#endif
