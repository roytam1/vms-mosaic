
/*      Displaying messages and getting input for WWW Library
**      =====================================================
**
**         May 92 Created By C.T. Barker
**         Feb 93 Portablized etc TBL
*/

#ifndef HTAlert_H
#define HTAlert_H

#include "HTUtils.h"
#include "tcp.h"

/*      Display a message and get the input
**
**      On entry,
**              Msg is the message.
**
**      On exit,
**              Return value is malloc'd string which must be freed.
*/
extern char *HTPrompt (WWW_CONST char *Msg, WWW_CONST char *deflt);
#define HTPromptPassword prompt_for_password


/*      Display a message, don't wait for input
**
**      On entry,
**              The input is a list of parameters for printf.
*/
extern void HTAlert (WWW_CONST char *Msg);


/*      Display a progress message for information (and diagnostics) only
*/
#define HTProgress mo_gui_notify_progress

#define HTCheckActiveIcon mo_gui_check_icon
#define HTClearActiveIcon mo_gui_clear_icon
#define HTMeter mo_gui_update_meter
#define HTDoneWithIcon mo_gui_done_with_icon

/*      Display a message, then wait for 'yes' or 'no'.
**
**      On entry,
**              Takes text string.
**
**      On exit,
**              If the user enters 'YES', returns TRUE, returns FALSE
**              otherwise.
*/
#define HTConfirm prompt_for_yes_or_no

#endif
