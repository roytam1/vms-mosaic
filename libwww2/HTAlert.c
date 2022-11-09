/*	Displaying messages and getting input for LineMode Browser
**	==========================================================
**
**	REPLACE THIS MODULE with a GUI version in a GUI environment!
**
** History:
**	   Jun 92 Created May 1992 By C.T. Barker
**	   Feb 93 Simplified, portablised (ha!) TBL
**
*/

#include "../config.h"
#include "HTAlert.h"


PUBLIC void HTAlert (WWW_CONST char *Msg)
{
  mo_gui_notify_progress(Msg);
  return;
}

PUBLIC char *HTPrompt (WWW_CONST char *Msg, WWW_CONST char *deflt)
{
  char *Tmp = prompt_for_string(Msg);
  char *rep = NULL;

  StrAllocCopy(rep, (Tmp && *Tmp) ? Tmp : deflt);
  if (Tmp)
      XtFree(Tmp);
  return rep;
}
