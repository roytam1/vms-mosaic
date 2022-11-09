/*			MAILTO WINDOW				HTMailTo.c
 **			=============
 ** Authors:
 **  Mike Peter Bretz (bretz@zdv.uni-tuebingen.de)
 **  Alan Braverman (alanb@ncsa.uiuc.edu)
 **
 ** History:
 **	07 Jul 94   First version  (MPB)
 **     07 Mar 95   Stuck it in NCSA Mosaic for X 2.6 (AMB)
 */

#include "../config.h"
#include "HTAccess.h"
#include "HTUtils.h"
#include "HTFile.h"
#include "HTML.h"
#include "HTFormat.h"
#include "HTAlert.h"
#include "../libnut/str-tools.h"

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/* From LIBHTMLW */
extern void GetMailtoKludgeInfo();

struct _HTStream {
    HTStreamClass *isa;
};

PUBLIC int HTSendMailTo (WWW_CONST char *arg,
			 HTParentAnchor *anAnchor,
			 HTFormat format_out,
			 HTStream *stream)
{
    char *mailtoURL, *mailtoSubject;
    char *p1 = arg;

#ifndef DISABLE_TRACE
    if (www2Trace) 
        fprintf(stderr, "HTMailto: Mailing to %s\n", p1);
#endif

    /*	We will ask for the document, omitting the host name and anchor.
     **
     **	Syntax of address is
     **		xxx@yyy		User xxx at site yyy (xxx is optional).
     */
    if (!my_strncasecmp(p1, "mailto:", 7))
        p1 += 7;		/* Skip "mailto:" prefix */

    if (!*p1) {			/* No email address */
	HTStream *target = HTStreamStack(WWW_HTML, format_out, COMPRESSED_NOT,
					 stream, anAnchor);
	char *msg = "<H1>ERROR</H1>Email address missing.";

	if (!target)
	    return(HT_NOT_LOADED);

	(*target->isa->put_block)(target, msg, strlen(msg));
        (*target->isa->free)(target);
	HTProgress("Could not find email address");
	return HT_NOT_LOADED;
    }

    GetMailtoKludgeInfo(&mailtoURL, &mailtoSubject);
    (void) mo_post_mailto_win(p1, mailtoSubject);
    return HT_LOADED;
}

PUBLIC HTProtocol HTMailto = { "mailto", HTSendMailTo, NULL };
