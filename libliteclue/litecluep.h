/* 
LiteClueP.h - Private definitions for LiteClue widget
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
$Log: LiteClueP.h,v $
Revision 1.3  1998/07/30 16:06:01  gary
NO_FONT_SET

Revision 1.2  1997/06/15 14:08:14  gary
Support for cancel wait period

Revision 1.1  1996/10/19 16:08:51  gary
Initial

*/

/* Modified for VMS Mosaic - 5-Mar-2006 */

#ifndef _DEF_LiteClueP_h
#define _DEF_LiteClueP_h

#include <X11/ShellP.h>
/* Include public header file for this widget. */
#include "LiteClue.h"
    
/* Doubly Linked List Processing */
struct list_thread_str
{
	struct list_thread_str *forw;	/* next pointer */
	struct list_thread_str *back;	/* prev pointer */
};
typedef struct list_thread_str  ListThread; 

typedef struct _transform
{
        double  mx, bx;
        double  my, by;
} Transform;

/* Keep information for each widget we are tracking */
struct liteClue_context_str
{
	ListThread next;	/* Next in list */
	Widget watched_w;	/* The widget we are watching */
	XcgLiteClueWidget cw;	/* Pointer back to the liteClue widget */
	Boolean sensitive;	/* If False, liteClue is suppressed */
	char *text;		/* Text to display */
	int group;		/* For grouping widgets together */
	Boolean at_mouse;	/* Popup at current mouse location? */
	Position abs_x, abs_y;  /* Position to popup at */
	short text_size;	/* Its size */
};

typedef struct {
	int	nothing;	/* place holder */
} LiteClueClassPart;

/* Full class record declaration */
typedef struct _LiteClueClassRec {
	CoreClassPart	       core_class;
	CompositeClassPart     composite_class;
	ShellClassPart         shell_class;
	OverrideShellClassPart override_shell_class;
	LiteClueClassPart      LiteClue_class;
} LiteClueClassRec;

extern LiteClueClassRec xcgLiteClueClassRec;

/* New fields for the LiteClue widget record */
typedef struct {
	/* resources */
	Pixel foreground;

#if XtSpecificationRelease < 5 || defined(NO_FONT_SET)
	XFontStruct *fontset;	/* the font for text in box */
#else
	XFontSet fontset;	/* the font for text in box */
#endif
        int waitPeriod;		/* the delay resource - pointer must be
				   in watched widget this long before
				   help is poped - in millisecs
				*/
        int cancelWaitPeriod;	/* after help is popped-down - normal
				   wait period is cancelled for this
				   period - in millisecs
				*/
	int popdownDelay;	/* Popdown delay */
	Boolean shaped;		/* Shaped balloon */
	Boolean rounded;	/* Rounded balloon */

	float border_size;	/* Border thickness */

	Dimension margin_width;	/* Margins */
	Dimension margin_height;

	Boolean transparent;

	/* -------- private state --------- */
	ListThread widget_list; /* list of widgets we are liteClue-ing */
	Dimension font_width;	/* width of '1' character */
	Dimension font_height;	/* height of font, rows are spaced using this */
	Dimension font_baseline;/* relative displacement to baseline from top */
	GC text_GC;		/* for drawing text */
	GC shape_GC;		/* for drawing shape */
	XtIntervalId interval_id; /* New field, holds timer id */
	Boolean	HelpIsUp;	/* the help popup is up */
	Time	HelpPopDownTime; /* time at which help popup was popped down */
	Boolean HelpIsActive;   /* liteClue is active */
	Transform t;
	Transform maskt;
	Pixmap shape_mask;	/* window shape */
        int shape_width;        /* last shaped window width */
        int shape_height;       /* last shaped window height */
	XtIntervalId popdown_id; /* Popdown timeout id */
	struct liteClue_context_str *once; /* One time only popup */
	XtAppContext app_con;	/* Application context */
} LiteCluePart;


/*
 * Full instance record declaration
 */
typedef struct _LiteClueRec {
	CorePart	  core;
	CompositePart     composite;
	ShellPart 	  shell;
	OverrideShellPart override;
	LiteCluePart	  liteClue;
} LiteClueRec;

#endif /* _DEF_LiteClueP_h */
