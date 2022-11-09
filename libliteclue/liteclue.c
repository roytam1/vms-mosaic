/* 
LiteClue.c - LiteClue widget
	See LiteClue documentation
	Version 1.5

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
www.compgen.com/widgets

Thanks to Contributers:
J Satchell
Eric Marttila
Addy Klos

*/
/* Revision History:
$Log: LiteClue.c,v $
Revision 1.16  1998/09/07 14:06:19  gary
Added const to prototype of XcgLiteClueAddWidget at request from user

Revision 1.15  1998/07/30 16:05:16  gary
Add NO_FONT_SET to use FontStruct rather than FontSet

Revision 1.14  1998/01/06 15:30:33  gary
If font specified by resource can not be converted, use fixed
font as fallback. If no font at all can be converted, prevent
crash, just disable widget entirely.

Revision 1.13  1997/07/07 14:55:04  gary
Cancel timeouts when XcgLiteClueDeleteWidget is called to prevent
errant timeout event on deleted widget.

Revision 1.12  1997/06/20 20:09:09  gary
Add XcgLiteClueDispatchEvent to enable clues for insensitive widgets.

Revision 1.11  1997/06/15 14:10:24  gary
Add XcgLiteClueDispatchEvent to enable clues for insensitive widgets.

Revision 1.10  1997/04/14 13:02:33  gary
Attempt to fix problem when we get multiple enter events bu no leave event.

Revision 1.9  1997/03/10 14:42:41  gary
Attempt to fix problem when we get multiple enter events bu no leave event.
Add C++ wrapper to allow linking with C++ programs. (In HView.h)

Revision 1.8  1997/01/17 13:44:14  gary
Support of cancelWaitPeriod resource: this is a period from the point
a help popdown occurs in which the normal waitPeriod is suspended
for the next popup

Revision 1.7  1996/12/16 22:35:38  gary
Fix double entry problem

Revision 1.6  1996/11/18 14:52:21  gary
remove some compile warnings pointed out by a user

Revision 1.5  1996/11/12 20:56:43  gary
remove some compile warnings

Revision 1.4  1996/10/20  13:38:16  gary
Version 1.2 freeze

Revision 1.3  1996/10/19 16:16:30  gary
Compile warning removed with cast

Revision 1.2  1996/10/19 16:07:38  gary
a) R4 back compatibility
b) Delay before pop up of help, waitPeriod resource (def 500 ms).
	Thanks to J Satchell for this.
c) Button press in watched widget pops down help

Revision 1.1  1996/10/18 23:14:58  gary
Initial

*/

/*
 * Modified for VMS mosaic
 * Added Group support - 28-Feb-2006
 * Added enable/disable support - 5-Mar-2006
 * Added shaped support from xmHTML - 10-Mar-2006
 * Added popup once support - 15-Mar-2006
 * Added rounded support - 6-Apr-2006
 */

/* Copyright (C) 2006, 2007 - The VMS Mosaic Project */

#define NO_FONT_SET 1

#ifndef VMS
#include <unistd.h>
#else
#include <stdlib.h>
#endif
#include <signal.h>
#include <X11/IntrinsicP.h> 
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <X11/Shape.h>
#include <Xmu/Converters.h>
#include "LiteClueP.h"

#include <stdio.h>

#define CheckWidgetClass(routine) \
	if (XtClass(w) != xcgLiteClueWidgetClass) \
		wrong_widget(routine)

#define ATTR(field)  cw->liteClue.field

static Boolean setValues(Widget _current, Widget _request, Widget _new,
			 ArgList args, Cardinal *num_args);
static void Initialize(Widget treq, Widget tnew, ArgList args,
		       Cardinal *num_args);

/*
 * Widget resources: for example to set LiteClue box background to yellow:
 *	*XcgLiteClue.background: yellow
 */

/* Set if server is missing shape extension */
static int no_shape_ext = 0;

#define offset(field) XtOffsetOf(LiteClueRec, field)

static XtResource LC_resources[] = {
	{XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
		offset(liteClue.foreground), XtRString, "black" },

#if XtSpecificationRelease < 5 || defined(NO_FONT_SET)
	{XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
		offset(liteClue.fontset), XtRString, 
             "-adobe-new century schoolbook-bold-r-normal-*-12-*-*-*-*-*-*-*" },
#else
	{XtNfontSet, XtCFontSet, XtRFontSet, sizeof(XFontSet),
		offset(liteClue.fontset), XtRString,
		"-adobe-new century schoolbook-bold-r-normal-*-12-*" },
#endif
        {XmNmarginWidth, XmCMarginWidth, XtRDimension,
        	sizeof(Dimension), offset(liteClue.margin_width),
                XtRImmediate, (XtPointer)2 },

        {XmNmarginHeight, XmCMarginHeight, XtRDimension,
                sizeof(Dimension), offset(liteClue.margin_height),
                XtRImmediate, (XtPointer)2 },

        {XcgNtransparent, XcgCTransparent, XtRBoolean, sizeof(Boolean),
		offset(liteClue.transparent), XtRString, "False" },

 	{XcgNwaitPeriod, XcgCWaitPeriod, XtRInt, sizeof(int),
		offset(liteClue.waitPeriod), XtRString, "500" },

	{XcgNcancelWaitPeriod, XcgCCancelWaitPeriod, XtRInt, sizeof(int),
		offset(liteClue.cancelWaitPeriod), XtRString, "2000" },

        {XcgNpopdownDelay, XcgCPopdownDelay, XtRInt, sizeof(int),
		offset(liteClue.popdownDelay), XtRString, "4000" },

	{XcgNshaped, XcgCShaped, XtRBoolean, sizeof(Boolean),
		offset(liteClue.shaped), XtRString, "False" },

	{XcgNrounded, XcgCRounded, XtRBoolean, sizeof(Boolean),
		offset(liteClue.rounded), XtRString, "False" },

	{XcgNborderSize, XcgCBorderSize, XtRFloat, sizeof(float),
		offset(liteClue.border_size), XtRString, "0.075" }
};

#undef offset


LiteClueClassRec xcgLiteClueClassRec = {
    {
	(WidgetClass)&overrideShellClassRec,	/* superclass */
	"XcgLiteClue",				/* class_name */
	(Cardinal)sizeof(LiteClueRec),		/* widget size */
	NULL,					/* class_init */
	(XtWidgetClassProc)NULL,		/* class_part_init */
	(XtEnum)FALSE,				/* class_inited */
	(XtInitProc)Initialize,			/* initialize */
	(XtArgsProc)NULL,			/* init_hook */
	XtInheritRealize,			/* realize */
	(XtActionList)NULL,			/* actions */
	(Cardinal)0,				/* num_actions */
	(XtResourceList)LC_resources,		/* resources */
	(Cardinal)XtNumber(LC_resources),	/* num_resources */
	NULLQUARK,				/* xrm_class */
	TRUE,					/* compress_motion */
	(XtEnum)FALSE,				/* compress_exposur */
	TRUE,					/* compress enterleave */
	FALSE,					/* visibility_interest */
	(XtWidgetProc)NULL,			/* destroy */
	XtInheritResize,
	XtInheritExpose,			/* expose, */
	(XtSetValuesFunc)setValues,		/* set_values */
	(XtArgsFunc)NULL,			/* set_values_hook */
	XtInheritSetValuesAlmost,		/* set_values_almost */
	(XtArgsProc)NULL,			/* get_values_hook */
	XtInheritAcceptFocus,			/* accept_focus */
	XtVersion,				/* version */
	(XtPointer)NULL,			/* callback_private */
	XtInheritTranslations,
	XtInheritQueryGeometry,			/* query_geometry */
	XtInheritDisplayAccelerator,		/* display_accelerator */
	(XtPointer)NULL,			/* extension */
    },
    { /*** composite-Class ***/
	XtInheritGeometryManager,		/* geometry_manager */
	XtInheritChangeManaged,			/* change_managed   */
	XtInheritInsertChild,			/* insert_child	    */
	XtInheritDeleteChild,			/* delete_child	    */
	NULL					/* extension	    */
    }, 
	{ /* Shell */
	(XtPointer)NULL,       			/* extension record pointer */
	},
	{ /* Override Shell */
	0,
	},
	{ /* LiteClue */
	0,
	},
};

WidgetClass xcgLiteClueWidgetClass = (WidgetClass) &xcgLiteClueClassRec;


/* Doubly linked list processing */

/*
 * Initialize header - both pointers point to it
 */
static void xcgListInit(ListThread *newbuf)
{
	newbuf->back = newbuf->forw = newbuf;
}


/*
 * Insert newbuf before posbuf
 */
static void xcgListInsertBefore(ListThread *newlist, ListThread *poslist)
{
	ListThread *prevbuf = poslist->back;

	poslist->back = newlist;
	newlist->forw = poslist;
	newlist->back = prevbuf;
	prevbuf->forw = newlist;
}


/*
 * Remove rembuf from queue
 */
static ListThread *xcgListRemove(ListThread *rembuf)
{
	ListThread *prevbuf = rembuf->back;
	ListThread *nextbuf = rembuf->forw;

	prevbuf->forw = nextbuf;
	nextbuf->back = prevbuf;

	rembuf->back = (ListThread *) NULL;	/* For safety to cause trap */
	rembuf->forw = (ListThread *) NULL;	/* if mistakenly refed */
	return rembuf;
}


/*
 * Font information
 */

#if XtSpecificationRelease < 5 || defined(NO_FONT_SET)

/* R4 and below code */
/*
 * Get XFontSet for passed font_string and return status
 */
static int string_to_FontSet(XcgLiteClueWidget cw, char *font_string,
			     XFontStruct **out) 
{
	Boolean sts;
	XrmValue from, to;
	
	to.size = sizeof(out);
	to.addr = (void *) out;
	from.size = strlen(from.addr = font_string);
	sts = XtConvertAndStore((Widget) cw, XtRString, &from, XtRFontStruct,
				&to);
	return sts;
}

static void compute_font_info(XcgLiteClueWidget cw)
{
	if (!ATTR(fontset)) {
		string_to_FontSet(cw, "fixed", &cw->liteClue.fontset);
		if (!ATTR(fontset)) {
			fprintf(stderr,	"LiteClue: cannot find font\n");
			return;
		}
		fprintf(stderr, "LiteClue: using fixed font\n");
	}
	ATTR(font_baseline) = ATTR(fontset)->max_bounds.ascent;
	ATTR(font_height) = ATTR(fontset)->max_bounds.ascent +
			    ATTR(fontset)->max_bounds.descent;
	ATTR(font_width) = ATTR(fontset)->max_bounds.rbearing -
			   ATTR(fontset)->min_bounds.lbearing;
}

#else
/*
 * Return XFontSet for passed font_string. 
 */
static int string_to_FontSet(XcgLiteClueWidget cw, char *font_string,
			     XFontSet *out) 
{
	Boolean sts;
	XrmValue from, to;
	
	to.size = sizeof(out);
	to.addr = (void *) out;
	from.size = strlen(from.addr = font_string);
	sts = XtConvertAndStore((Widget) cw, XtRString, &from, XtRFontSet, &to);
	return sts;
}

/* R5 and above code */
static void compute_font_info(XcgLiteClueWidget cw)
{
	XRectangle ink, logical;

	if (!ATTR(fontset)) {
		string_to_FontSet(cw, "fixed", &cw->liteClue.fontset);
		if (!ATTR(fontset)) {
			fprintf(stderr, "LiteClue: cannot find font\n");
			return;
		}
		fprintf(stderr, "LiteClue: using fixed font\n");
	}
	XmbTextExtents(ATTR(fontset), "1", 1, &ink, &logical);

	/* y offset from top to baseline, returned as negative */
	ATTR(font_baseline) = -logical.y;
	ATTR(font_height) = logical.height;
}
#endif

/*
 * Creates the various graphic contexts we will need 
 */
static void create_GC(XcgLiteClueWidget cw)
{
	XtGCMask valuemask = GCForeground | GCBackground | GCFillStyle;
	XGCValues myXGCV;

	myXGCV.foreground = ATTR(foreground);
	myXGCV.background = cw->core.background_pixel;
	myXGCV.fill_style = FillSolid; 

#if XtSpecificationRelease < 5	|| defined(NO_FONT_SET)
	valuemask |= GCFont;
	myXGCV.font = ATTR(fontset->fid); 
#endif
	if (ATTR(text_GC))
		XtReleaseGC((Widget) cw, ATTR(text_GC));
	ATTR(text_GC) = XtGetGC((Widget)cw, valuemask, &myXGCV);
}


/* A routine to halt execution and force a core dump for debugging analysis	
 * when a public routine is called with the wrong class of widget
 */
static void wrong_widget(char *routine)
{
#ifndef VMS
	int mypid = getpid();
#endif

	fprintf(stderr, "Wrong class of widget passed to %s\n", routine);
	fflush(stderr); 
#ifndef VMS
	kill(mypid, SIGABRT);
#else
	abort();
#endif
}

/*
 * Find the target in the widget list.  Return context pointer if found,
 * NULL if not
 */
static liteClueContext *find_watched_widget(XcgLiteClueWidget cw, Widget target)
{
	liteClueContext *obj;

	for (obj = (liteClueContext *)cw->liteClue.widget_list.forw;
	     obj != (liteClueContext *)&cw->liteClue.widget_list; 
	     obj = (liteClueContext *)obj->next.forw) {
		if (target == obj->watched_w)
			return obj;
	}
	return NULL;
}

/*
 * Allocate and initialize a widget context
 */
static liteClueContext *alloc_liteClue_context(void)
{
	liteClueContext *out = (liteClueContext *) XtCalloc(1,
						       sizeof(liteClueContext));

	xcgListInit(&out->next);	
	return out;
}

/*
 * Allocate, initialize and link a liteClue context to the list
 */
static liteClueContext *alloc_link_liteClue_context(XcgLiteClueWidget cw)
{
	liteClueContext *out = alloc_liteClue_context();

	/* Link as new last */
	xcgListInsertBefore(&out->next, &cw->liteClue.widget_list);
	out->cw = cw;	/* Initialize this member - it's always the same */
	return out;
}

/*
 * Free a widget context
 */
static void free_widget_context(liteClueContext *obj)
{
	xcgListRemove((ListThread *)obj);

	/* Free up all things object points to */
	obj->sensitive = False;
	if (obj->text)
		XtFree(obj->text);
	XtFree((char *) obj);
}

/* -------------------- Shaped support ---------------------- */

#define BORDER_SIZE(w)    ((w)->liteClue.border_size)
#define WINDOW_WIDTH(w)   (2.0 - BORDER_SIZE(w) * 2)
#define WINDOW_HEIGHT(w)  (2.0 - BORDER_SIZE(w) * 2)
#define Xx(x, t)          ((int)((t)->mx * (x) + (t)->bx + 0.5))
#define Xy(y, t)          ((int)((t)->my * (y) + (t)->by + 0.5))
#define Xwidth(w, t)      ((int)((t)->mx * (w) + 0.5))
#define Xheight(h, t)     ((int)((t)->my * (h) + 0.5))

/* Copyright (C) 1994-1997 by Ripley Software Development
 *
 * This section from the XmHTML Widget Library.
 */

static void popdownBalloon(XtPointer client_data, XtIntervalId *id)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget)client_data;

	/* Remove timeout if called from within the widget code */
	if ((id == (XtIntervalId)0) && ATTR(popdown_id))
		XtRemoveTimeOut(ATTR(popdown_id));

	/* Always remove any outstanding popupDelay */
	if (ATTR(interval_id))
		XtRemoveTimeOut(ATTR(interval_id));

	ATTR(interval_id) = (XtIntervalId)0;
	ATTR(popdown_id) = (XtIntervalId)0;

	/* Pop it down */
	if (ATTR(HelpIsUp)) {
		XtPopdown((Widget)cw);
		ATTR(HelpIsUp) = False;
	}
	ATTR(once)->watched_w = NULL;
}

static void FillArc(Display *dpy, Drawable d, GC gc, Transform *t, double x,
		    double y, double width, double height, int angle1,
		    int angle2)
{
	int xx, xy, xw, xh;

	xx = Xx(x, t);
	xy = Xy(y, t);
	xw = Xwidth(width, t);
	xh = Xheight(height, t);
	if (xw < 0) {
		xx += xw;
		xw = -xw;
	}
	if (xh < 0) {
		xy += xh;
		xh = -xh;
	}
	XFillArc(dpy, d, gc, xx, xy, xw, xh, angle1, angle2);
}

static void setTransform(Transform *t, int xx1, int xx2, int xy1, int xy2,
			 double tx1, double tx2, double ty1, double ty2)
{
	t->mx = ((double) xx2 - xx1) / (tx2 - tx1);
	t->bx = ((double) xx1) - t->mx * tx1;
	t->my = ((double) xy2 - xy1) / (ty2 - ty1);
	t->by = ((double) xy1) - t->my * ty1;
}

static void drawText(Display *dpy, liteClueContext *obj,Drawable drawable,
		     GC gc, int x_offset, int y_offset)
{
	XcgLiteClueWidget cw = obj->cw;

	/* Draw the text in the window */
#if XtSpecificationRelease < 5 || defined(NO_FONT_SET)
	XDrawString(dpy, drawable, gc,
#else
	XmbDrawString(dpy, drawable, ATTR(fontset), gc,
#endif
		      (2 * ATTR(margin_width)) + x_offset,
		      ATTR(margin_height) + ATTR(font_baseline) + y_offset,
		      obj->text, obj->text_size);
}


/*****
* Name:			drawShapedBalloon
* Return Type:	void
* Description:	pops up the balloon widget as a shaped window
* In:
*	obj:			liteClueContext;
*	x:			absolute x popup position;
*	y:			absolute y popup position;
*	width:			desired widget width;
* Returns:
*	nothing
* Note:
*	This routine composes a *clipmask* for the widget to use when
*	it is displayed.  The clipmask is initially empty and gets filled
*	according to the selected options.  Once it is filled, the text
*	is rendered in the selected color.
*****/
static void drawShapedBalloon(liteClueContext *obj, Position x, Position y,
			      int width)
{
	XcgLiteClueWidget cw = obj->cw;
	int face_width, face_height, x_offset, y_offset;
	Dimension bwidth, bheight;
	Pixmap shape_mask;
	Display *dpy = XtDisplay((Widget)cw);
	Window win = XtWindow((Widget)cw);

	if (!win) {
		XtRealizeWidget((Widget)cw);
		win = XtWindow((Widget)cw);
		if (ATTR(transparent)) {
			XSetWindowAttributes attrs;
			unsigned long valueMask = 0;

			attrs.background_pixel = ATTR(foreground);
			valueMask |= CWBackPixel;
			valueMask &= ~CWBackPixmap;
			XChangeWindowAttributes(dpy, win, valueMask, &attrs);
		}
	}

	/* Horizontal offset for text rendering */
	x_offset = ATTR(margin_width) + ATTR(font_width);
	y_offset = 0.25 * ATTR(font_height);

	bwidth = 2 * ATTR(margin_width) + width + 2 * x_offset;
	bheight = 2 * ATTR(margin_height) + ATTR(font_height) + 2 * y_offset;
	if (width > 240)
		x_offset += ATTR(font_width);

	/* Resize to fit */
	XtResizeWidget((Widget)cw, bwidth, bheight, cw->core.border_width);

	/* Compute desired border size */
	setTransform(&ATTR(maskt), 0, bwidth, bheight, 0, -1.0, 1.0, -1.0, 1.0);

	face_width = abs(Xwidth(BORDER_SIZE(cw), &(ATTR(maskt))));
	face_height = abs(Xheight(BORDER_SIZE(cw), &(ATTR(maskt))));

	setTransform(&ATTR(t), face_width, bwidth - face_width,
		     bheight - face_height, face_height, 
		     -WINDOW_WIDTH(cw) / 2, WINDOW_WIDTH(cw) / 2,
		     -WINDOW_HEIGHT(cw) / 2, WINDOW_HEIGHT(cw) / 2);

	/* Free up previous clipmask if the size differs */
	if (ATTR(shape_mask) &&
	    ((ATTR(shape_width) != cw->core.width) ||
	     (ATTR(shape_height) != cw->core.height))) {
		XFreePixmap(dpy, ATTR(shape_mask));
		ATTR(shape_mask) = None;
	}

	/* Allocate a clipmask (bitmap of depth one) */
	if (!ATTR(shape_mask)) {
		ATTR(shape_mask) = XCreatePixmap(dpy, win, bwidth, bheight, 1);
		ATTR(shape_width) = bwidth;
		ATTR(shape_height) = bheight;
	}
	shape_mask = ATTR(shape_mask);

	/* Simple GC */
	if (!ATTR(shape_GC)) {
		XtGCMask valuemask;
		XGCValues xgc;

		ATTR(shape_GC) = XCreateGC(dpy, shape_mask, 0, &xgc);
#if XtSpecificationRelease < 5  || defined(NO_FONT_SET)
		valuemask = GCFont;
		xgc.font = ATTR(fontset->fid);
		XChangeGC(dpy, ATTR(shape_GC), valuemask, &xgc);
#endif
	}

	/* Make it fully transparent */
	XSetForeground(dpy, ATTR(shape_GC), 0);
	XFillRectangle(dpy, shape_mask, ATTR(shape_GC), 0, 0, bwidth, bheight);
	XSetForeground(dpy, ATTR(shape_GC), 1);

	/*****
	* Fill in the border bits if we have a border.  If we aren't transparent
	* a filled arc is created.
	*****/
	if (ATTR(border_size) > 0.0 || !ATTR(transparent))
		FillArc(dpy, shape_mask, ATTR(shape_GC), &(ATTR(maskt)),
			-1.0, -1.0, 2.0, 2.0, 0, 360 * 64);

	/*****
	* If we are being transparent, erase the inner part of the disk
	* and fill the bits for the text.  If we aren't transparent we don't
	* have to do this 'cause the bits set for the disk already cover the
	* bits that cover the text.
	*****/
	if (ATTR(transparent)) {
		if (ATTR(border_size) > 0.0) {
			XSetForeground(dpy, ATTR(shape_GC), 0);
			FillArc(dpy, shape_mask, ATTR(shape_GC), &(ATTR(maskt)),
				-WINDOW_WIDTH(cw) / 2, -WINDOW_HEIGHT(cw) / 2,
				WINDOW_WIDTH(cw), WINDOW_HEIGHT(cw),
				0, 360 * 64);
			XSetForeground(dpy, ATTR(shape_GC), 1);
		}
		drawText(dpy, obj, shape_mask, ATTR(shape_GC),
			 x_offset, y_offset);
	}
	/* The highest enclosing widget is the widget itself */
	XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, shape_mask, ShapeSet);

	/* Erase clipmask */
	XSetForeground(dpy, ATTR(shape_GC), 0); 
	XFillRectangle(dpy, shape_mask, ATTR(shape_GC), 0, 0, bwidth, bheight);
	XSetForeground(dpy, ATTR(shape_GC), 1);

	/* Draw clip shape */
	if (ATTR(transparent)) {
		drawText(dpy, obj, shape_mask, ATTR(shape_GC),
			 x_offset, y_offset);
	} else {
		FillArc(dpy, shape_mask, ATTR(shape_GC), &(ATTR(t)),
			-WINDOW_WIDTH(cw) / 2, -WINDOW_HEIGHT(cw) / 2,
			WINDOW_WIDTH(cw), WINDOW_HEIGHT(cw),
			0, 360 * 64);
	}

	/* Compose final clipmask */
	XShapeCombineMask(dpy, win, ShapeClip, 0, 0, shape_mask, ShapeSet);

	/* Move to correct location */
	XtMoveWidget((Widget)cw, x, y);

	/* Pop it up */
	XtPopup((Widget)cw, XtGrabNone);

	/* Draw the text */
	drawText(dpy, obj, win, ATTR(text_GC), x_offset, y_offset);
}

/* -------------------- Widget Methods ---------------------- */
/* Initialize method */
static void Initialize(Widget treq, Widget tnew, ArgList args, 
		       Cardinal *num_args)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget) tnew;
	int shape_event_base, shape_error_base;
	liteClueContext *obj;

	ATTR(text_GC) = NULL;
	ATTR(HelpIsUp) = False;
	ATTR(HelpIsActive) = False;
	ATTR(HelpPopDownTime) = 0;
	ATTR(interval_id) = (XtIntervalId)0;
	ATTR(popdown_id) = (XtIntervalId)0;
	ATTR(shape_mask) = None;
	ATTR(shape_GC) = NULL;
	ATTR(shape_width) = 0;
	ATTR(shape_height) = 0;
	ATTR(once) = obj = alloc_liteClue_context();
	ATTR(app_con) = XtWidgetToApplicationContext((Widget)cw);
	obj->watched_w = NULL;
	obj->cw = cw;
	obj->text = NULL;
	obj->sensitive = True;
	obj->at_mouse = True;

	tnew->core.x = tnew->core.y = 0;
	tnew->core.width = tnew->core.height = 1;

	xcgListInit(&cw->liteClue.widget_list);	 /* Initialize empty list */
	compute_font_info(cw);
	create_GC(cw);

	/* Only one or the other */
	if (ATTR(shaped) && ATTR(rounded))
		ATTR(shaped) = False;

	if ((ATTR(shaped) || ATTR(rounded)) &&
            !XShapeQueryExtension(XtDisplay(cw), &shape_event_base,
				  &shape_error_base)) {
		fprintf(stderr,
		       "LiteClue: Shape extension not supported by XServer.\n");
		fprintf(stderr, "          Turning off shaped tooltips.\n");
		ATTR(rounded) = ATTR(shaped) = False;
		no_shape_ext = 1;
	}
}

static Boolean setValues(Widget _current, Widget _request, Widget _new,
			 ArgList args, Cardinal *num_args)
{
	XcgLiteClueWidget cw_req = (XcgLiteClueWidget) _request;
	XcgLiteClueWidget cw_cur = (XcgLiteClueWidget) _current;
	XcgLiteClueWidget cw_new = (XcgLiteClueWidget) _new;
	static int check_shape = 0;

	if (!check_shape && !no_shape_ext &&
	    (cw_req->liteClue.shaped || cw_req->liteClue.rounded)) {
		int shape_event_base, shape_error_base;

        	if (!XShapeQueryExtension(XtDisplay(cw_cur), &shape_event_base,
				  	  &shape_error_base)) {
		    fprintf(stderr,
		       "LiteClue: Shape extension not supported by XServer.\n");
		    fprintf(stderr, "          Turning off shaped tooltips.\n");
		    cw_new->liteClue.shaped = cw_new->liteClue.rounded = False;
		    no_shape_ext = 1;
		}
		check_shape = 1;
	}

	/* Values of cw_new->liteClue.cancelWaitPeriod and
	 * cw_new->liteClue.waitPeriod are accepted without checking
	 */
	if (cw_req->liteClue.fontset != cw_cur->liteClue.fontset)
		compute_font_info(cw_new);

	if (cw_req->liteClue.foreground != cw_cur->liteClue.foreground ||
	    cw_req->core.background_pixel != cw_cur->core.background_pixel ||
	    cw_req->liteClue.fontset != cw_cur->liteClue.fontset)
		create_GC(cw_new);

	if (no_shape_ext &&
	    (cw_req->liteClue.shaped || cw_req->liteClue.rounded))
		cw_new->liteClue.shaped = cw_new->liteClue.rounded = False;

	if (cw_req->liteClue.shaped && cw_req->liteClue.rounded) {
		cw_new->liteClue.shaped = False;
		fprintf(stderr,
		       "LiteClue: Both Oval and Rounded Tooltips requested.\n");
		fprintf(stderr, "          Defaulting to Rounded Tooltips.\n");
	}
	return False;
}

/* ----------------- Event handlers ------------------------*/


/* At this point the help may be popup 
*/
static void timeout_event(XtPointer client_data, XtIntervalId *id)
{
#define OFFSET_X 4	
#define OFFSET_Y 4
#define BorderPix 2
	liteClueContext *obj = (liteClueContext *) client_data;
	XcgLiteClueWidget cw = obj->cw;
	Position x, y, abs_x, abs_y, w_height;
	Dimension clue_width, clue_height;
	Dimension w_offset = 0;
	XRectangle ink, logical;
	Widget w;

	/* Variables to retrieve info about the screen size */
	Display *display;
	int screen_num, display_width, display_height;
	Position clue_x, clue_y;

	if (ATTR(interval_id) == (XtIntervalId)0)
		return;	  /* Timeout was removed but callback happened anyway */
	ATTR(interval_id) = (XtIntervalId)0;

	if ((obj->sensitive == False) || !ATTR(HelpIsActive))
		return;

	w = obj->watched_w;
	display = XtDisplay(w);
	if (!obj->at_mouse) {
		if (obj != ATTR(once)) {
			XtVaGetValues(w, XtNheight, &w_height, NULL);
			/* Position just below the widget */
			x = y = 0;
		} else {
			w_height = 10;
			x = obj->abs_x;
			y = obj->abs_y;
		}
	} else {
		Window root, child;
		int win_x, win_y, root_x, root_y;
		unsigned int mask_return;

		/* Must get current pointer position */
		if (!XQueryPointer(display, XtWindow(w), &root, &child,
				   &root_x, &root_y, &win_x, &win_y,
				   &mask_return))
	 		return;
		/* Position it below the pointer */
		w_height = ATTR(font_height);
		x = (Position)win_x + 8;
		y = (Position)win_y;
	}
	XtTranslateCoords(w, x, y, &abs_x, &abs_y);

#if XtSpecificationRelease < 5	|| defined(NO_FONT_SET)
	{
	    int direction_return;
	    int font_ascent_return, font_descent_return; 
	    XCharStruct oret;

	    XTextExtents(ATTR(fontset), obj->text, obj->text_size,
			 &direction_return, &font_ascent_return,
			 &font_descent_return, &oret); 
	    logical.width = oret.width;
	}
#else
	XmbTextExtents(ATTR(fontset), obj->text, obj->text_size, &ink,
		       &logical);
#endif
	if (ATTR(rounded))
		w_offset = 4;

	clue_width = (2 * (BorderPix + w_offset)) + logical.width;
	clue_height =  (2 * BorderPix) + ATTR(font_height);

	if (ATTR(shaped) && (logical.width > 240))
		clue_width += 2 * ATTR(font_width);

	screen_num  = DefaultScreen(display);
  
	display_width  = DisplayWidth(display, screen_num);
	display_height = DisplayHeight(display, screen_num);

	/* Deal with the Y coordinate */
	/* Default position below watched widget */
	clue_y = abs_y + w_height + OFFSET_Y;
	/* Off bottom of screen? */
	if (clue_y + clue_height > display_height)
		/* Then place on top of watched widget */
		clue_y = abs_y - clue_height - OFFSET_Y;

	/* Now deal with the X coordinate */
	/* Default position a little right of watched widget */ 
	clue_x = abs_x + OFFSET_X;
	if (clue_x < 0) {	/* Off left of screen? */
		clue_x = 0;
	} else if (clue_x + clue_width > display_width) {
		/* Off right of screen? */
		clue_x = display_width - clue_width - 1;
		if (ATTR(shaped))
			clue_x -= (4 * ATTR(margin_width)) +
				  (2 * ATTR(font_width));
		if (clue_x < 0)
			clue_x = 0;
	}

        if (ATTR(shaped)) {
		drawShapedBalloon(obj, clue_x, clue_y, clue_width);
	} else {
		XtResizeWidget((Widget)cw, clue_width, 
			       clue_height, cw->core.border_width);
		XtMoveWidget((Widget)cw, clue_x, clue_y);
		XtPopup((Widget)cw, XtGrabNone);

		if (ATTR(rounded))
			XmuReshapeWidget((Widget)cw, XmuShapeOval, 8, 8);

#if XtSpecificationRelease < 5 || defined(NO_FONT_SET)
		XDrawImageString(display, XtWindow((Widget)cw), 
#else
		XmbDrawImageString(display, XtWindow((Widget)cw), 
				   ATTR(fontset),
#endif
				   ATTR(text_GC), BorderPix + w_offset,
				   BorderPix + ATTR(font_baseline),
				   obj->text, obj->text_size);
	}

	ATTR(HelpIsUp) = True;

	/* If we have a popdown timeout, add it */
	if (ATTR(popdownDelay))
		ATTR(popdown_id) = XtAppAddTimeOut(ATTR(app_con),
						   ATTR(popdownDelay),
						   popdownBalloon,
						   (XtPointer)cw);
#undef OFFSET_X
#undef OFFSET_Y
}

/*
 * Pointer enters watched widget, set a timer at which time it will
 * popup the help
 */
static void Enter_event(Widget w, XtPointer client_data, XEvent *xevent,
			Boolean *continue_to_dispatch )
{
	liteClueContext *obj = (liteClueContext *) client_data;
	XcgLiteClueWidget cw = obj->cw;
	XEnterWindowEvent *event = &xevent->xcrossing;
	int current_waitPeriod;

	if (!ATTR(HelpIsActive) || (obj->sensitive == False) || !ATTR(fontset))
		return;
	/*
	 * Check for two enters in a row - happens when widget is
	 * exposed under a pop-up.  Also ignore if the window does
	 * not have focus.
	 */
	if ((ATTR(interval_id) != (XtIntervalId)0) ||
	    (event->mode != NotifyNormal) || !event->focus)
		return;

	/* If clue was recently popped down, only wait short time to
	 * popup clue for next watched widget
	 */
	if ((event->time - ATTR(HelpPopDownTime)) > 
	    ATTR(cancelWaitPeriod)) {
		current_waitPeriod = ATTR(waitPeriod);
	} else {
		current_waitPeriod = 100;
	}
	ATTR(interval_id) = XtAppAddTimeOut(ATTR(app_con), current_waitPeriod,
					    timeout_event, client_data);
}

/*
 * Remove timers, if pending.  Then popdown help.
 */
static void Leave_event(Widget w, XtPointer client_data, XEvent *xevent,
			Boolean *continue_to_dispatch)
{
	liteClueContext *obj = (liteClueContext *) client_data;
	XcgLiteClueWidget cw = obj->cw;
	XEnterWindowEvent *event = &xevent->xcrossing;

	if (ATTR(interval_id) != (XtIntervalId)0) {
		XtRemoveTimeOut(ATTR(interval_id));
		ATTR(interval_id) = (XtIntervalId)0;
	}
	if (ATTR(popdown_id) != (XtIntervalId)0) {
		XtRemoveTimeOut(ATTR(popdown_id));
		ATTR(popdown_id) = (XtIntervalId)0;
	}
	ATTR(once)->watched_w = NULL;

	if (obj->sensitive == False)
		return;
	if (ATTR(HelpIsUp)) {
		XtPopdown((Widget) cw);
		ATTR(HelpIsUp) = False;
		ATTR(HelpPopDownTime) = event->time;
	}
}

/* -------------------------- Widget API ---------------------------- */

/*
;+
XcgLiteClueAddWidget -- Add a widget to be watched.  LiteClue will be given
			for this widget

Func:	A widget is added to the LiteClue watched list.  Clues are given for
	sensitive watched widgets when the pointer enters its window.  If the
	widget is already watched, the passed text replaces its current clue
	text.  If text is null, the widget is still added, if it is not already
	in the list, but no clue will appear.  Text may be specified with
	XcgLiteClueAddWidget in a subsequent call.  When text is null and the
	widget is already in the list, its text is not changed.  When a widget
	is added to the watched list, it automatically becomes sensitive.
	Otherwise, its sensitivity is not changed.  A watched widget which is
	not sensitive retains its context but clues are suppressed.
	None of this affects the behaviour of the watched widget itself.
	LiteClue monitors enter and leave events of the watched widget's
	window passively.

Input:	w - LiteClue widget
	watch - the widget to give liteClues for
	text - pointer to liteClue text. (May be NULL)
	size - size of text.  May be zero
	       in which case a strlen will be done.
	group - used for deleting groups
Output: 

Return:	

;-
*/
void XcgLiteClueAddWidget(Widget w, Widget watch, const char *text, int size,
			  int group)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;
	liteClueContext *obj;
	Boolean exists = False;

	/* Make sure we are called with a LiteClue widget */
	CheckWidgetClass("XcgLiteClueAddWidget");

	obj = find_watched_widget(cw, watch);
	if (obj) {
		exists = True;
		if (text) {
			if (obj->text)
				XtFree(obj->text);
			obj->text = NULL;
		}
	} else {
		obj = alloc_link_liteClue_context(cw);
		obj->watched_w = watch;
	}
	obj->group = group;
	obj->at_mouse = False;

	if (text && !obj->text) {
		if (!size)
			size = strlen(text);
		obj->text = XtMalloc(size + 1);
		memcpy(obj->text, text, size);
		obj->text[size] = '\0';
		obj->text_size = size;
	}
	if (!exists) {	/* Was created */
		XtAddEventHandler(watch, EnterWindowMask, False,
				  Enter_event, (XtPointer) obj);
		XtAddEventHandler(watch, LeaveWindowMask | ButtonPressMask,
				  False, Leave_event, (XtPointer) obj);
		obj->sensitive = True;
	}
}


/*
;+
XcgLiteClueDeleteWidget -- Delete a widget that is watched. 

Func:	A widget is deleted from the watched list and its resources are
	freed.  LiteClue is no longer given for the widget.
	If the widget is not watched, nothing is done.

Input:	w - LiteClue widget
	watch - the widget to delete
Output: 

Return:	

;-
*/
void XcgLiteClueDeleteWidget(Widget w, Widget watch)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;
	liteClueContext *obj;

	CheckWidgetClass("XcgLiteClueDeleteWidget");
	obj = find_watched_widget(cw, watch);
	if (obj) {
		XtRemoveEventHandler(watch, EnterWindowMask, False,
				     Enter_event, (XtPointer) obj);
		XtRemoveEventHandler(watch, LeaveWindowMask | ButtonPressMask,
				     False, Leave_event, (XtPointer) obj);
		if (ATTR(interval_id) != (XtIntervalId)0) {
			XtRemoveTimeOut(ATTR(interval_id));
			ATTR(interval_id) = (XtIntervalId)0;
		}
		free_widget_context(obj);
	}
}

/* Group deletion */
void XcgLiteClueDeleteGroup(Widget w, int group)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;
	liteClueContext *obj, *prev;

	CheckWidgetClass("XcgLiteClueDeleteGroup");

	for (obj = (liteClueContext *)cw->liteClue.widget_list.forw;
	     obj != (liteClueContext *)&cw->liteClue.widget_list; 
	     obj = (liteClueContext *)obj->next.forw) {
		if (group == obj->group) {
			XtRemoveEventHandler(obj->watched_w,
					     EnterWindowMask, False,
					     Enter_event, (XtPointer) obj);
			XtRemoveEventHandler(obj->watched_w,
					     LeaveWindowMask | ButtonPressMask, 
					     False, Leave_event,
					     (XtPointer) obj);
			if (ATTR(interval_id) != (XtIntervalId)0) {
				XtRemoveTimeOut(ATTR(interval_id));
				ATTR(interval_id)= (XtIntervalId)0;
			}
			prev = (liteClueContext *)obj->next.back;
			free_widget_context(obj);
			obj = prev;
		}
	}
}


/*
;+
XcgLiteClueSetSensitive -- Enable/disable sensitivity for watched widget. 

Func:	When a watched widget is sensitive, a clue is popped up when the pointer
	enters its window.  When a watched widget is insensitive, the widget is
	retained in the watched list but no clue is popped.  The sensitivity of
	a watched widget relative to clues is set or reset by this function.
	The Xt sensitivity of the watched widget is not altered by this
	function.

Input:	w - LiteClue widget
	watch - the widget to make sensitive or insensitive or NULL
		to change all watched widgets
	sensitive - True or False
Output: 

Return:	

;-
*/
void XcgLiteClueSetSensitive(Widget w, Widget watch, Boolean sensitive)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;
	liteClueContext *obj;

	CheckWidgetClass("XcgLiteClueSetSensitive");
	if (watch) {
		obj = find_watched_widget(cw, watch);
		if (obj) 
			obj->sensitive = sensitive;
		return;
	}

	/* Do them all */
	for (obj = (liteClueContext *)cw->liteClue.widget_list.forw;
	     obj != (liteClueContext *)&cw->liteClue.widget_list; 
	     obj = (liteClueContext *)obj->next.forw)
		obj->sensitive = sensitive;
}

/*
;+
XcgLiteClueGetSensitive -- Get sensitivity mode for watched widget. 

Func:	When a watched widget is sensitive, a clue is popped up when the pointer
	enters its window.  When a watched widget is insensitive, the widget is
	retained in the watched list but no clue is popped.  The sensitivity
	state of a watched widget relative to clues is returned by this
	function.  The Xt sensitivity of a widget is a totally independent
	concept.

Input:	w - LiteClue widget
	watch - the widget for which to get sensitivity state.  If NULL
		first watched widget is used.  If there are no watched widgets,
		False is returned.
Output: 

Return:	sensitive - True or False

;-
*/
Boolean XcgLiteClueGetSensitive(Widget w, Widget watch)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;
	liteClueContext *obj;

	CheckWidgetClass("XcgLiteClueGetSensitive");
	if (watch) {
		obj = find_watched_widget(cw, watch);
		if (obj) {
			return obj->sensitive;
		} else {
			return False;
		}
	}
	/* Do the first one */
	obj = (liteClueContext *) cw->liteClue.widget_list.forw; 
	if (obj != (liteClueContext *) &cw->liteClue.widget_list) {
		return obj->sensitive;
	} else {
		return False;
	}
}


/*
;+
XcgLiteClueDispatchEvent -- Dispatch event from main X event loop

Func:	This function may be used to enable clues for insensitive
	watched widgets.  Normally, XtAppMainLoop (which calls
	XtDispatchEvent) will not deliver EnterNotify and LeaveNotify
	events to widgets that are not sensitive (XtSetSensitive).  This
	prevents clues from poping up for these widgets.  To bypass this
	limitation, you can break out XtAppMainLoop and add a call to
	XcgLiteClueDispatchEvent as follows:

	MyXtAppMainLoop(XtAppContext app) 
	{
	    XEvent event;

	    for (;;) {
	        XtAppNextEvent(app, &event);
		XcgLiteClueDispatchEvent(w, event);
	        XtDispatchEvent(&event);
	    }
	} 

Input:	w - LiteClue widget
	event - received event, normally from call to XtAppNextEvent.

Output: void

Return:	True - event was dispatched to non-sensitive watched widget.
	False - not a EnterNotify or LeaveNotify event or window in
		event is not a non-sensitive watched widget.

;-
*/
Boolean XcgLiteClueDispatchEvent(Widget w, XEvent *event)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;
	liteClueContext *obj;
	Boolean continue_to_dispatch;

	if (event->type != EnterNotify && event->type != LeaveNotify)
		return False;
	CheckWidgetClass("XcgLiteClueDispatchEvent");

	/* Scan list */
	for (obj = (liteClueContext *)cw->liteClue.widget_list.forw;
	     obj != (liteClueContext *)&cw->liteClue.widget_list; 
	     obj = (liteClueContext *)obj->next.forw) {
		if ((XtWindow(obj->watched_w) != event->xany.window) ||
		    (XtIsSensitive(obj->watched_w)))
			continue;
		/* Found one */
		if (event->type == EnterNotify) {
			Enter_event(obj->watched_w, (XtPointer)obj, event,
				    &continue_to_dispatch);
		} else {
			Leave_event(obj->watched_w, (XtPointer)obj, event,
				    &continue_to_dispatch);
		}
		return True;
	}
	return False;
}

void XcgLiteClueSetActive(Widget w, Boolean active)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;

	ATTR(HelpIsActive) = active;
}

Boolean XcgLiteClueGetActive(Widget w)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;

	return ATTR(HelpIsActive);
}

void XcgLiteClueResetWait(Widget w)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;

	ATTR(HelpPopDownTime) = 0;
}

/*************************** Popup one time **********************/

void XcgLiteCluePopup(Widget w, Widget popin, char *text, int x, int y,
		      Boolean at_mouse)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;
	liteClueContext *obj;
	int size = strlen(text);

	if (ATTR(HelpIsUp) || ATTR(interval_id))
		popdownBalloon((XtPointer)cw, (XtIntervalId)0);

	if (!ATTR(HelpIsActive) || !ATTR(fontset))
		return;

	obj = ATTR(once);
	obj->watched_w = popin;

	if (obj->text)
		XtFree(obj->text);
	obj->text = XtMalloc(size + 1);
	memcpy(obj->text, text, size);
	obj->text[size] = '\0';
	obj->text_size = size;
	obj->abs_x = x;
	obj->abs_y = y;
	obj->at_mouse = at_mouse;
	
	ATTR(interval_id) = XtAppAddTimeOut(ATTR(app_con), ATTR(waitPeriod),
					    timeout_event, (XtPointer)obj);
}

void XcgLiteCluePopdown(Widget w)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;

	if (!XcgLiteCluePopupIsUp(w))
		return;
	popdownBalloon((XtPointer)cw, (XtIntervalId)0);
}

Boolean XcgLiteCluePopupIsUp(Widget w)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;

	if (ATTR(once)->watched_w)
		return True;

	return False;
}
