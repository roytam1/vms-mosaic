/*              Plain text object                       HTPlain.h
**              -----------------
**
**
*/

#ifndef HTPLAIN_H
#define HTPLAIN_H

#ifndef HTSTREAM_
#include "HTStream.h"
#endif
#ifndef HTANCHOR_H
#include "HTAnchor.h"
#endif

extern HTStream *HTPlainPresent (HTPresentation *pres,
				 HTParentAnchor *anchor,
        			 HTStream       *sink, 
        			 HTFormat       format_in,
        			 int            compressed);

#endif
