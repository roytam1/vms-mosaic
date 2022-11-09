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
#include "HTML.h"
#include "HTFormat.h"
#include "HTAlert.h"
#include "../libnut/str-tools.h"

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

extern void GetMailtoKludgeInfo();

PUBLIC int HTSendMailTo (WWW_CONST char *arg,
			 HTParentAnchor *anAnchor,
			 HTFormat format_out,
			 HTStream *stream)
{
    char *mailtoURL, *mailtoSubject;
    WWW_CONST char *p1 = arg;

#ifndef DISABLE_TRACE
    if (www2Trace) 
        fprintf(stderr, "HTMailto: Mailing to %s\n", arg);
#endif
    
    /*	We will ask for the document, omitting the host name and anchor.
     **
     **	Syntax of address is
     **		xxx@yyy		User xxx at site yyy (xxx is optional).
     */        
    if (!my_strncasecmp(arg, "mailto:", 7))
        p1 = arg + 7;		/* Skip "mailto:" prefix */
    
    if (!*arg) {
	HTProgress("Could not find email address");
	return HT_NOT_LOADED;	/* Ignore if no name */
    }

    GetMailtoKludgeInfo(&mailtoURL, &mailtoSubject);
    (void) mo_post_mailto_win(p1, mailtoSubject);
    return HT_LOADED;
}

PUBLIC HTProtocol HTMailto = { "mailto", HTSendMailTo, NULL };
