/* 
LiteClue.h - Public definitions for LiteClue widget
	See LiteClue documentation

Copyright 1996 COMPUTER GENERATION, INC.,

The software is provided "as is", without warranty of any kind, express
or implied, including but not limited to the warranties of
merchantability, fitness for a particular purpose and noninfringement.
In no event shall Computer Generation, inc. nor the author be liable for
any claim, damages or other liability, whether in an action of contract,
tort or otherwise, arising from, out of or in connection with the
software or the use or other dealings in the software.

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation.

Author:
Gary Aviv 
Computer Generation, Inc.,
gary@compgen.com
*/
/* Revision History:

Revision 1.5  1998/09/07 14:06:24  gary
Added const to prototype of XcgLiteClueAddWidget at request from user

Revision 1.4  1997/06/15 14:07:56  gary
Added XcgLiteClueDispatchEvent

Revision 1.3  1997/04/14 13:03:25  gary
Added XgcNwaitperiod XgcNcancelWaitPeriod and c++ wrappers

Revision 1.2  1996/10/20 13:39:25  gary
Version 1.2 freeze

Revision 1.1  1996/10/19 16:08:04  gary
Initial

*/

/* Modified for VMS Mosaic - 28-Feb-2006 */

#ifndef _DEF_LiteClue_h
#define _DEF_LiteClue_h

#include <X11/StringDefs.h>

/*
 * New resource names
 */

#define XcgNcancelWaitPeriod	"cancelWaitPeriod"
#define XcgNwaitPeriod		"waitPeriod"
#define XcgNtransparent		"transparent"
#define XcgNpopdownDelay	"popdownDelay"
#define XcgNshaped		"shaped"
#define XcgNrounded		"rounded"
#define XcgNborderSize		"borderSize"
/*
 * New resource classes
 */
#define XcgCCancelWaitPeriod	"CancelWaitPeriod"
#define XcgCWaitPeriod		"WaitPeriod"
#define XcgCTransparent		"Transparent"
#define XcgCPopdownDelay	"PopdownDelay"
#define XcgCShaped		"Shaped"
#define XcgCRounded		"Rounded"
#define XcgCBorderSize		"BorderSize"

extern WidgetClass xcgLiteClueWidgetClass; 
typedef struct _LiteClueClassRec *XcgLiteClueWidgetClass;
typedef struct _LiteClueRec      *XcgLiteClueWidget;

void XcgLiteClueAddWidget(Widget w, Widget watch, const char *text, int size,
			  int group);
void XcgLiteClueDeleteWidget(Widget w, Widget watch);
void XcgLiteClueDeleteGroup(Widget w, int group);
void XcgLiteClueSetSensitive(Widget w, Widget watch, Boolean sensitive);
Boolean XcgLiteClueGetSensitive(Widget w, Widget watch);
Boolean XcgLiteClueDispatchEvent(Widget w, XEvent *event);
void XcgLiteClueSetActive(Widget w, Boolean active);
Boolean XcgLiteClueGetActive(Widget w);
void XcgLiteClueResetWait(Widget w);
void XcgLiteCluePopup(Widget w, Widget popin, char *text, int x, int y,
		      Boolean at_mouse);
void XcgLiteCluePopdown(Widget w);
Boolean XcgLiteCluePopupIsUp(Widget w);

#endif
