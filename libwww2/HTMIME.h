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
