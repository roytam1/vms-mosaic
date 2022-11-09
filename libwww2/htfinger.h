/* Finger protocol module for the WWW library */
/* History:
**      21 Apr 94       Andrew Brooks
*/

#ifndef HTFINGER_H
#define HTFINGER_H

#include "HTAccess.h"
#include "HTAnchor.h"

extern HTProtocol HTFinger;

extern int HTLoadFinger (WWW_CONST char *arg,
			 HTParentAnchor *anAnchor,
			 HTFormat        format_out,
			 HTStream       *stream);

#endif  /* HTFINGER_H */
