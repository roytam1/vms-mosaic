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
/* Copyright (C) 2006, 2007 - The VMS Mosaic Project */

#ifndef _DEF_LiteClueP_h
#define _DEF_LiteClueP_h

#include <X11/ShellP.h>
/* Include public header file for this widget. */
#include "LiteClue.h"
    
/* Doubly Linked List Processing */
typedef struct list_thread_str {
	struct list_thread_str *forw;	/* Next pointer */
	struct list_thread_str *back;	/* Prev pointer */
} ListThread;

typedef struct _transform {
        double  mx, bx;
        double  my, by;
} Transform;

/* Keep information for each widget we are tracking */
typedef struct liteClue_context_str {
	ListThread next;	/* Next in list */
	Widget watched_w;	/* The widget we are watching */
	XcgLiteClueWidget cw;	/* Pointer back to the liteClue widget */
	Boolean sensitive;	/* If False, liteClue is suppressed */
	char *text;		/* Text to display */
	int group;		/* For grouping widgets together */
	Boolean at_mouse;	/* Popup at current mouse location? */
	Position abs_x, abs_y;  /* Position to popup at */
	short text_size;	/* Its size */
} liteClueContext;

typedef struct {
	int	nothing;	/* Place holder */
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
	/* Resources */
	Pixel foreground;

#if XtSpecificationRelease < 5 || defined(NO_FONT_SET)
	XFontStruct *fontset;	/* The font for text in box */
#else
	XFontSet fontset;	/* The font for text in box */
#endif
        int waitPeriod;		/* The delay resource - pointer must be
				 * in watched widget this long before
				 * help is poped - in millisecs */
        int cancelWaitPeriod;	/* After help is popped-down - normal
				 * wait period is cancelled for this
				 * period - in millisecs */
	int popdownDelay;	/* Popdown delay */
	Boolean shaped;		/* Shaped balloon */
	Boolean rounded;	/* Rounded balloon */

	float border_size;	/* Border thickness */

	Dimension margin_width;	/* Margins */
	Dimension margin_height;

	Boolean transparent;

	/* -------- Private state --------- */
	ListThread widget_list; /* List of widgets we are liteClue-ing */
	Dimension font_width;	/* Width of '1' character */
	Dimension font_height;	/* Height of font, rows are spaced using this */
	Dimension font_baseline;/* Relative displacement to baseline from top */
	GC text_GC;		/* For drawing text */
	GC shape_GC;		/* For drawing shape */
	XtIntervalId interval_id; /* New field, holds timer id */
	Boolean	HelpIsUp;	/* The help popup is up */
	Time	HelpPopDownTime; /* Time at which help popup was popped down */
	Boolean HelpIsActive;   /* LiteClue is active */
	Transform t;
	Transform maskt;
	Pixmap shape_mask;	/* Window shape */
        int shape_width;        /* Last shaped window width */
        int shape_height;       /* Last shaped window height */
	XtIntervalId popdown_id; /* Popdown timeout id */
	liteClueContext *once; /* One time only popup */
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

#endif
