/****************************************************************************
 * NCSA Mosaic for the X Window System                                      *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 * mosaic@ncsa.uiuc.edu                                                     *
 *                                                                          *
 * Copyright (C) 1993, Board of Trustees of the University of Illinois      *
 *                                                                          *
 * NCSA Mosaic software, both binary and source (hereafter, Software) is    *
 * copyrighted by The Board of Trustees of the University of Illinois       *
 * (UI), and ownership remains with the UI.                                 *
 *                                                                          *
 * The UI grants you (hereafter, Licensee) a license to use the Software    *
 * for academic, research and internal business purposes only, without a    *
 * fee.  Licensee may distribute the binary and source code (if released)   *
 * to third parties provided that the copyright notice and this statement   *
 * appears on all copies and that no charge is associated with such         *
 * copies.                                                                  *
 *                                                                          *
 * Licensee may make derivative works.  However, if Licensee distributes    *
 * any derivative work based on or derived from the Software, then          *
 * Licensee will (1) notify NCSA regarding its distribution of the          *
 * derivative work, and (2) clearly notify users that such derivative       *
 * work is a modified version and not the original NCSA Mosaic              *
 * distributed by the UI.                                                   *
 *                                                                          *
 * Any Licensee wishing to make commercial use of the Software should       *
 * contact the UI, c/o NCSA, to negotiate an appropriate license for such   *
 * commercial use.  Commercial use includes (1) integration of all or       *
 * part of the source code into a product for sale or license by or on      *
 * behalf of Licensee to third parties, or (2) distribution of the binary   *
 * code or source code to third parties that need it to utilize a           *
 * commercial product sold or licensed by or on behalf of Licensee.         *
 *                                                                          *
 * UI MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR   *
 * ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED          *
 * WARRANTY.  THE UI SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE    *
 * USERS OF THIS SOFTWARE.                                                  *
 *                                                                          *
 * By using or copying this Software, Licensee agrees to abide by the       *
 * copyright law and all other applicable laws of the U.S. including, but   *
 * not limited to, export control laws, and the terms of this license.      *
 * UI shall have the right to terminate this license immediately by         *
 * written notice upon Licensee's breach of, or non-compliance with, any    *
 * of its terms.  Licensee may be held legally responsible for any          *
 * copyright infringement that is caused or encouraged by Licensee's        *
 * failure to abide by the terms of this license.                           *
 *                                                                          *
 * Comments and questions are welcome and can be sent to                    *
 * mosaic-x@ncsa.uiuc.edu.                                                  *
 ****************************************************************************/

/* Copyright (C) 1998, 1999, 2000, 2003, 2004, 2005, 2006, 2007
 * The VMS Mosaic Project
 */

#include "../config.h"

#include <stdio.h>
#include <stdlib.h>

#include "HTMLparse.h"
#include "HTMLP.h"
#include "HTMLPutil.h"
#include "HTMLfont.h"
#include "HTMLframe.h"
#include "HTMLwidgets.h"

#include "../src/prefs.h"

#include <Xm/DrawingA.h>
#include <Xm/ScrollBar.h>
#ifndef MOTIF1_2
extern void _XmDrawShadow();  /* Motif 1.1 .h files are broken */
#undef XtIsRealized	      /* Motif 1.1 definition causes build failure */
#else
#include <Xm/DrawP.h>
#endif
#include <Xm/ToggleB.h>
#include <X11/cursorfont.h>
#include <Xmu/StdSel.h>

#ifdef VMS
/* DEC extensions */
#include <X11/decspecific.h>
#endif

#if defined(MULTINET) && defined(__DECC) && (__VMS_VER >= 70000000)
#define strdup  decc$strdup
#endif /* VMS V7, VRH, GEC, MPJZ */
extern char *strdup();

#include "../libnut/str-tools.h"

#define	CLICK_TIME		500
#define	SELECT_THRESHOLD	3
#define	MAX_UNDERLINES		3
#define DEFAULT_INCREMENT       18

#define VERT_SCROLL_WIDTH	14
#define HORIZ_SCROLL_HEIGHT	14

#ifndef ABS
#define ABS(x)  (((x) > 0) ? (x) : ((x) * -1))
#endif

extern unsigned char map_table[];
#define TOLOWER(x) map_table[x]

#ifndef DISABLE_TRACE
int htmlwTrace;
extern int reportBugs;
extern int refreshTrace;
#endif

/* For selective image loading */
extern Boolean currently_delaying_images;

/* For image processing in idle work process */
static int DidAllImages;

static void		SelectStart(Widget w, XEvent *event,
				    String *params, Cardinal *num_params);
static void		ExtendStart(Widget w, XEvent *event,
				    String *params, Cardinal *num_params);
static void		ExtendAdjust(Widget w, XEvent *event,
				     String *params, Cardinal *num_params);
static void		ExtendEnd(Widget w, XEvent *event,
				  String *params, Cardinal *num_params);
static void 		TrackMotion(Widget w, XEvent *event,
				    String *params, Cardinal *num_params);
static Boolean 		ConvertSelection(Widget w, Atom *selection,
					 Atom *target, Atom *type,
					 XtPointer *value,
				         unsigned long *length, int *format);
static void 		LoseSelection(Widget w, Atom *selection);
static void 		SelectionDone(Widget w, Atom *selection, Atom *target);

static void		_HTMLInput(Widget w, XEvent *event, ElemInfo *eptr,
				   Boolean is_label);
static void             Initialize(HTMLWidget request, HTMLWidget nw);
static void             Redisplay(HTMLWidget hw, XEvent *event, Region region);
static void             Destroy(HTMLWidget hw);
static void             Resize(HTMLWidget hw);
static Boolean          SetValues(HTMLWidget current, HTMLWidget request,
				  HTMLWidget nw);
static XtGeometryResult GeometryManager(Widget w, XtWidgetGeometry *request,
				        XtWidgetGeometry *reply);
static Dimension        VbarWidth(HTMLWidget hw);
static void		ViewRedisplay(HTMLWidget hw, int x, int y,
				      int width, int height);
static Boolean 		html_accept_focus(Widget w, Time *t);
static ElemInfo 	*HTMLAnchorToEptr(HTMLWidget hw, char *name);
static void 		Realize(Widget w, XtValueMask *valueMask, 
				XSetWindowAttributes *attrs); 

/* In HTMLwidgets.c */
extern int skip_traversal_current;

/* From gui.h and img.h */
extern void mo_gui_done_with_icon(void);
extern void mo_gui_clear_icon(void);
extern void ProcessImageData(Widget w, ImageInfo *img_info, XColor *colrs);

extern int force_image_load;
extern int loading_inlined_images;
extern int Vclass;

extern char *currentURL;

extern int XmxBlockTimers;

/* Exported to HTMLimages.c */
GC maskGC = NULL;   /* PGE, transparent on solid background speedup */

int Select_GC_Reset = 0;

/* Default translations
 * Selection of text, and activate anchors.
 * If motif, add manager translations.
 * 4/5/96 TPR added traversal_current to make sure if user presses
 * button in view the hotkeys get control.
 */
static char defaultTranslations[] = " \
 <Btn1Down>:	select-start() ManagerGadgetArm()\n\
 <Btn1Motion>:	extend-adjust() ManagerGadgetButtonMotion()\n\
 <Btn1Up>:	extend-end(PRIMARY, CUT_BUFFER0) ManagerGadgetActivate() traversal_current()\n\
 <Btn2Down>:	select-start()\n\
 <Btn2Motion>:	extend-adjust()\n\
 <Btn2Up>:	extend-end(PRIMARY, CUT_BUFFER0)\n\
 <Motion>:      track-motion()\n\
 <Leave>:       track-motion()\n\
 <Expose>:      track-motion()\
";

static XtActionsRec actionsList[] = {
   { "select-start",      (XtActionProc) SelectStart },
   { "extend-start",      (XtActionProc) ExtendStart },
   { "extend-adjust",     (XtActionProc) ExtendAdjust },
   { "extend-end",        (XtActionProc) ExtendEnd },
   { "track-motion",      (XtActionProc) TrackMotion },
   { "traversal_back",    (XtActionProc) traversal_back },
   { "traversal_forward", (XtActionProc) traversal_forward },
   { "traversal_end",     (XtActionProc) traversal_end },
   { "traversal_current", (XtActionProc) traversal_current }
};

/*
 * For some reason, in Motif1.2/X11R5 the actionsList above gets corrupted
 * When the parent HTML widget is created.  This means we can't use
 * it later with XtAppAddActions to add to the viewing area.
 * So, we make a spare copy here to use with XtAppAddActions.
 */
static XtActionsRec SpareActionsList[] = {
   { "select-start",      (XtActionProc) SelectStart },
   { "extend-start",      (XtActionProc) ExtendStart },
   { "extend-adjust",     (XtActionProc) ExtendAdjust },
   { "extend-end",        (XtActionProc) ExtendEnd },
   { "track-motion",      (XtActionProc) TrackMotion },
   { "traversal_back",    (XtActionProc) traversal_back },
   { "traversal_forward", (XtActionProc) traversal_forward },
   { "traversal_end",     (XtActionProc) traversal_end },
   { "traversal_current", (XtActionProc) traversal_current },
};

/*
 *  Resource definitions for HTML widget
 */
static XtResource resources[] = {
        /* Without Motif we need to override the borderWidth to 0 (from 1). */
	{ WbNmarginWidth, WbCMarginWidth, XtRDimension, sizeof(Dimension),
	  XtOffset(HTMLWidget, html.margin_width),
	  XtRImmediate, (XtPointer) MARGIN_DEFAULT
	},
	{ WbNmarginHeight, WbCMarginHeight, XtRDimension, sizeof(Dimension),
	  XtOffset(HTMLWidget, html.margin_height),
	  XtRImmediate, (XtPointer) MARGIN_DEFAULT
	},
	{ WbNanchorCallback, XtCCallback, XtRCallback, sizeof(XtCallbackList),
	  XtOffset(HTMLWidget, html.anchor_callback),
	  XtRImmediate, (XtPointer) NULL
	},
	{ WbNbaseCallback, XtCCallback, XtRCallback, sizeof(XtCallbackList),
	  XtOffset(HTMLWidget, html.base_callback),
	  XtRImmediate, (XtPointer) NULL
	},
	{ WbNtitleCallback, XtCCallback, XtRCallback, sizeof(XtCallbackList),
	  XtOffset(HTMLWidget, html.title_callback),
	  XtRImmediate, (XtPointer) NULL
	},
	{ WbNsubmitFormCallback, XtCCallback, XtRCallback,
	  sizeof(XtCallbackList),
	  XtOffset(HTMLWidget, html.form_callback), XtRImmediate,
	  (XtPointer) NULL
	},
	{ WbNtext, WbCText, XtRString, sizeof(char *),
	  XtOffset(HTMLWidget, html.raw_text), XtRString, (char *) NULL
	},
	{ WbNheaderText, WbCHeaderText, XtRString, sizeof(char *),
	  XtOffset(HTMLWidget, html.header_text), XtRString, (char *) NULL
	},
	{ WbNfooterText, WbCFooterText, XtRString, sizeof(char *),
	  XtOffset(HTMLWidget, html.footer_text), XtRString, (char *) NULL
	},
	{ WbNcharSet, WbCCharSet, XtRString, sizeof(char *),
	  XtOffset(HTMLWidget, html.charset), XtRString, (char *) NULL
	},
	{ WbNtitleText, WbCTitleText, XtRString, sizeof(char *),
	  XtOffset(HTMLWidget, html.title), XtRString, (char *) NULL	
	},
	{ WbNbodyColors, WbCBodyColors, XtRBoolean, sizeof(Boolean),
	  XtOffset(HTMLWidget, html.body_colors), XtRString, "True"
	},
	{ WbNbodyBG, WbCBodyBG, XtRPixel, sizeof(Pixel),
	  XtOffset(HTMLWidget, html.background_SAVE), XtRString,
	  "#ffffffffffff"
	},
	{ WbNformButtonBackground, WbCFormButtonBackground, XtRPixel,
	  sizeof(Pixel), XtOffset(HTMLWidget, html.formbuttonbackground),
	  XtRString, "#bfbfbfbfbfbf"
	},
	{ WbNbodyImages, WbCBodyImages, XtRBoolean, sizeof(Boolean),
	  XtOffset(HTMLWidget, html.body_images), XtRString, "True"
	},
	{ WbNfontColors, WbCFontColors, XtRBoolean, sizeof(Boolean),
	  XtOffset(HTMLWidget, html.font_colors), XtRString, "True"
	},
	{ WbNfontSizes, WbCFontSizes, XtRBoolean, sizeof(Boolean),
	  XtOffset(HTMLWidget, html.font_sizes), XtRString, "True"
	},
	{ WbNfontBase, WbCFontBase, XtRInt, sizeof(int),
	  XtOffset(HTMLWidget, html.font_base), XtRString, "3"
	},
	{ WbNblinkingText, WbCBlinkingText, XtRBoolean, sizeof(Boolean),
	  XtOffset(HTMLWidget, html.blinking_text), XtRString, "True"
	},
	{ WbNblinkTime, WbCBlinkTime, XtRInt, sizeof(int),
	  XtOffset(HTMLWidget, html.blink_time), XtRString, "500"
	},
	{ WbNanchorUnderlines, WbCAnchorUnderlines, XtRInt, sizeof(int),
	  XtOffset(HTMLWidget, html.num_anchor_underlines), XtRString, "0"
	},
	{ WbNvisitedAnchorUnderlines, WbCVisitedAnchorUnderlines, 
	  XtRInt, sizeof(int),
	  XtOffset(HTMLWidget, html.num_visitedAnchor_underlines),
	  XtRString, "0"
	},
	{ WbNdashedAnchorUnderlines, WbCDashedAnchorUnderlines, 
	  XtRBoolean, sizeof(Boolean),
	  XtOffset(HTMLWidget, html.dashed_anchor_lines), XtRString, "False"
	},
	{ WbNdashVisitedAnchorUnderlines, WbCDashVisitedAnchorUnderlines, 
	  XtRBoolean, sizeof(Boolean),
	  XtOffset(HTMLWidget, html.dashed_visitedAnchor_lines),
	  XtRString, "False"
	},
	{ WbNanchorColor, XtCForeground, XtRPixel, sizeof(Pixel),
	  XtOffset(HTMLWidget, html.anchor_fg), XtRString, "blue2"
	},
	{ WbNvisitedAnchorColor, XtCForeground, XtRPixel, sizeof(Pixel),
	  XtOffset(HTMLWidget, html.visitedAnchor_fg), XtRString, "purple4"
	},
	{ WbNactiveAnchorFG, XtCBackground, XtRPixel, sizeof(Pixel),
	  XtOffset(HTMLWidget, html.activeAnchor_fg), XtRString, "Red"
	},
	{ WbNactiveAnchorBG, XtCForeground, XtRPixel, sizeof(Pixel),
	  XtOffset(HTMLWidget, html.activeAnchor_bg), XtRString, "White"
	},
	{ WbNisIndex, WbCIsIndex, XtRBoolean, sizeof(Boolean),
	  XtOffset(HTMLWidget, html.is_index), XtRString, "False"
	},
	{ WbNverticalScrollBar,	WbCVerticalScrollBar, XtRWidget,
	  sizeof(Widget), XtOffset(HTMLWidget, html.vbar), XtRImmediate, NULL
	},
	{ WbNhorizontalScrollBar, WbCHorizontalScrollBar, XtRWidget,
	  sizeof(Widget), XtOffset(HTMLWidget, html.hbar), XtRImmediate, NULL
	},
	{ WbNview, WbCView, XtRWidget, sizeof(Widget),
	  XtOffset(HTMLWidget, html.view), XtRImmediate, NULL
	},
	{ XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.font),
	  XtRString, "-adobe-times-medium-r-normal-*-14-*-*-*-*-*-*-*"
	},
	{ WbNitalicFont, WbCItalicFont, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.italic_font),
	  XtRString, "-adobe-times-medium-i-normal-*-14-*-*-*-*-*-*-*"
	},
	{ WbNboldFont, WbCBoldFont, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.bold_font),
	  XtRString, "-adobe-times-bold-r-normal-*-14-*-*-*-*-*-*-*"
	},
	{ WbNboldItalicFont, WbCBoldItalicFont, XtRFontStruct,
	  sizeof(XFontStruct *), XtOffset(HTMLWidget, html.bolditalic_font),
	  XtRString, "-adobe-times-bold-i-normal-*-14-*-*-*-*-*-*-*"
	},
	{ WbNmeterFont, WbCMeterFont, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.meter_font),
	  XtRString, "-adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*"
	},
	{ WbNtoolbarFont, WbCToolbarFont, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.toolbar_font),
	  XtRString, "-adobe-times-bold-r-normal-*-12-*-*-*-*-*-iso8859-1"
	},
	{ WbNfixedFont, WbCFixedFont, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.fixed_font),
	  XtRString, "-adobe-courier-medium-r-normal-*-14-*-*-*-*-*-*-*"
	},
	{ WbNfixedboldFont, WbCFixedboldFont, XtRFontStruct,
	  sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.fixedbold_font),
	  XtRString, "-adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*"
	},
	{ WbNfixeditalicFont, WbCFixeditalicFont, 
	  XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.fixeditalic_font),
	  XtRString, "-adobe-courier-medium-o-normal-*-14-*-*-*-*-*-*-*"
	},
	{ WbNheader1Font, WbCHeader1Font, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.header1_font),
	  XtRString, "-adobe-times-bold-r-normal-*-24-*-*-*-*-*-*-*"
	},
	{ WbNheader2Font, WbCHeader2Font, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.header2_font),
	  XtRString, "-adobe-times-bold-r-normal-*-18-*-*-*-*-*-*-*"
	},
	{ WbNheader3Font, WbCHeader3Font, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.header3_font),
	  XtRString, "-adobe-times-bold-r-normal-*-17-*-*-*-*-*-*-*"
	},
	{ WbNheader4Font, WbCHeader4Font, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.header4_font),
	  XtRString, "-adobe-times-bold-r-normal-*-14-*-*-*-*-*-*-*"
	},
	{ WbNheader5Font, WbCHeader5Font, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.header5_font),
	  XtRString, "-adobe-times-bold-r-normal-*-12-*-*-*-*-*-*-*"
	},
	{ WbNheader6Font, WbCHeader6Font, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.header6_font),
	  XtRString, "-adobe-times-bold-r-normal-*-10-*-*-*-*-*-*-*"
	},
	{ WbNaddressFont, WbCAddressFont, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.address_font),
	  XtRString, "-adobe-times-medium-i-normal-*-14-*-*-*-*-*-*-*"
	},
	{ WbNplainFont, WbCPlainFont, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.plain_font),
	  XtRString, "-adobe-courier-medium-r-normal-*-14-*-*-*-*-*-*-*"
	},
	{ WbNplainboldFont, WbCPlainboldFont, XtRFontStruct,
	  sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.plainbold_font),
	  XtRString, "-adobe-courier-bold-r-normal-*-14-*-*-*-*-*-*-*"
	},
	{ WbNplainitalicFont, WbCPlainitalicFont, 
	  XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.plainitalic_font),
	  XtRString, "-adobe-courier-medium-o-normal-*-14-*-*-*-*-*-*-*"
	},
	{ WbNlistingFont, WbCListingFont, XtRFontStruct, sizeof(XFontStruct *),
	  XtOffset(HTMLWidget, html.listing_font),
	  XtRString, "-adobe-courier-medium-r-normal-*-12-*-*-*-*-*-*-*"
	},
        { WbNpreviousVisitedTestFunction, WbCPreviousVisitedTestFunction, 
	  XtRPointer, sizeof(XtPointer),
          XtOffset(HTMLWidget, html.previously_visited_test),
          XtRImmediate, (XtPointer) NULL
        },
        { WbNpointerMotionCallback, WbCPointerMotionCallback, 
	  XtRCallback, sizeof(XtCallbackList),
          XtOffset(HTMLWidget, html.pointer_motion_callback),
          XtRImmediate, (XtPointer) NULL
        },
	{ WbNmaxColorsInImage, WbCMaxColorsInImage, XtRInt, sizeof(int),
	  XtOffset(HTMLWidget, html.max_colors_in_image),
	  XtRImmediate, (XtPointer) 50
	},
        { WbNimageCallback, WbCImageCallback, XtRCallback,
	  sizeof(XtCallbackList), XtOffset(HTMLWidget, html.image_callback),
	  XtRImmediate, (XtPointer) NULL
        },
        { WbNimageQuantizeCallback, WbCImageQuantizeCallback, XtRCallback,
	  sizeof(XtCallbackList), XtOffset(HTMLWidget, html.quantize_callback),
	  XtRImmediate, (XtPointer) NULL
        },
	{ WbNdelayImageLoads, WbCDelayImageLoads, XtRBoolean, sizeof(Boolean),
	  XtOffset(HTMLWidget, html.delay_image_loads), XtRString, "False"
	},
	{ WbNpercentVerticalSpace, WbCPercentVerticalSpace, XtRInt,
	  sizeof(int), XtOffset(HTMLWidget, html.percent_vert_space),
	  XtRString, "90"
	},
	{ WbNfancySelections, WbCFancySelections, XtRBoolean, sizeof(Boolean),
	  XtOffset(HTMLWidget, html.fancy_selections),	XtRString, "False"
	},
        { WbNgetUrlDataCB, WbCGetUrlDataCB, XtRCallback, sizeof(XtCallbackList),
          XtOffset(HTMLWidget, html.get_url_data_cb),
          XtRImmediate, (XtPointer) NULL
        },
        { WbNframeCallback, XtCCallback, XtRCallback, sizeof(XtCallbackList),
          XtOffset(HTMLWidget, html.frame_callback),
          XtRImmediate, (XtPointer) NULL
        },
	{ WbNframeSupport, WbCFrameSupport, XtRBoolean, sizeof(Boolean),
	  XtOffset(HTMLWidget, html.frame_support), XtRString, "True"
	},
	{ WbNisFrame, WbCIsFrame, XtRBoolean, sizeof(Boolean),
	  XtOffset(HTMLWidget, html.is_frame), XtRString, "False"
	},
	{ WbNscrollBars, WbCScrollBars, XtRInt, sizeof(int),
	  XtOffset(HTMLWidget, html.scroll_bars),
          XtRImmediate, (XtPointer) FRAME_SCROLL_AUTO
	},
	{ WbNmultiImage, WbCMultiImage, XtRBoolean, sizeof(Boolean),
	  XtOffset(HTMLWidget, html.multi_image_load), XtRString, "True"
	},
        { WbNmultiLoadCallback, WbCMultiLoadCallback, 
	  XtRCallback, sizeof(XtCallbackList),
          XtOffset(HTMLWidget, html.multi_load_callback),
          XtRImmediate, (XtPointer) NULL
        }
};


HTMLClassRec htmlClassRec = {
   {						/* core class fields  */
      (WidgetClass) &xmManagerClassRec,		/* superclass         */
      "HTML",					/* class_name         */
      sizeof(HTMLRec),				/* widget_size        */
      NULL,					/* class_initialize   */
      NULL,					/* class_part_init    */
      FALSE,					/* class_inited       */
      (XtInitProc) Initialize,			/* initialize         */
      NULL,					/* initialize_hook    */
      Realize,					/* realize            */
      actionsList,				/* actions	      */
      XtNumber(actionsList),			/* num_actions	      */
      resources,				/* resources          */
      XtNumber(resources),			/* num_resources      */
      NULLQUARK,				/* xrm_class          */
      TRUE,					/* compress_motion    */
      XtExposeCompressMultiple,			/* compress_exposure  */
      TRUE,					/* compress_enterlv   */
      FALSE,					/* visible_interest   */
      (XtWidgetProc) Destroy,	                /* destroy            */
      (XtWidgetProc) Resize,			/* resize             */
      (XtExposeProc) Redisplay,			/* expose             */
      (XtSetValuesFunc) SetValues,		/* set_values         */
      NULL,					/* set_values_hook    */
      XtInheritSetValuesAlmost,			/* set_values_almost  */
      NULL,					/* get_values_hook    */
      html_accept_focus,			/* accept_focus       */
      XtVersion,				/* version            */
      NULL,					/* callback_private   */
      defaultTranslations,			/* tm_table           */
      XtInheritQueryGeometry,			/* query_geometry     */
      XtInheritDisplayAccelerator,              /* display_accelerator*/
      NULL		                        /* extension          */
   },
   {		/* composite_class fields */
      (XtGeometryHandler) GeometryManager,   	/* geometry_manager   */
      NULL,					/* change_managed     */
      XtInheritInsertChild,			/* insert_child       */
      XtInheritDeleteChild,			/* delete_child       */
      NULL                                      /* extension          */
   },
   {		/* constraint_class fields */
      NULL,					/* resource list      */
      0,					/* num resources      */
      0,					/* constraint size    */
      NULL,					/* init proc          */
      NULL,					/* destroy proc       */
      NULL,					/* set values proc    */
      NULL                                      /* extension          */
   },
   {		/* manager_class fields */
      XtInheritTranslations,			/* translations           */
      NULL,					/* syn_resources      	  */
      0,					/* num_syn_resources 	  */
      NULL,					/* syn_cont_resources     */
      0,					/* num_syn_cont_resources */
      XmInheritParentProcess,                   /* parent_process         */
      NULL					/* extension 	          */
   },
   {		/* html_class fields */
      0						/* none			  */
   }
};

WidgetClass htmlWidgetClass = (WidgetClass)&htmlClassRec;

static XtAppContext app_con = NULL;
static Cursor in_anchor_cursor = (Cursor)NULL;
static char *mailToKludgeSubject = NULL; 
static char *mailToKludgeURL = NULL;

int installed_colormap = 0;
Colormap installed_cmap;

void hw_do_bg(HTMLWidget hw, char *bgname, PhotoComposeContext *pcc)
{
	ImageInfo lpicd;
	ImageInfo *picd;
	ElemInfo *eptr;
	static int init = 0;
	static int MaxPixmapWidth, MaxPixmapHeight;

	if (!bgname || !*bgname || !hw->html.drawGC || hw->html.bg_image)
		return;

	/* If image delayed don't load the background image... */
	if (hw->html.delay_image_loads || (currently_delaying_images == 1))
		return;

	if (!init) {
		MaxPixmapHeight = get_pref_int(eMAXPIXMAPHEIGHT);
		if (!MaxPixmapHeight)
			MaxPixmapHeight = 1000;
		MaxPixmapWidth  = get_pref_int(eMAXPIXMAPWIDTH);
		if (!MaxPixmapWidth)
			MaxPixmapWidth = 1200;
		init = 1;
	}
	memset(&lpicd, 0, sizeof(ImageInfo));
	lpicd.src = strdup(bgname);
	lpicd.align = ALIGN_NONE;
	lpicd.req_height = -1;  /* No req_height */
	lpicd.req_width = -1;   /* No req_width */
	lpicd.delayed = hw->html.delay_image_loads;
	lpicd.cw_only = pcc->cw_only;

	/** memset sets them
	lpicd.alt_text = NULL;
	lpicd.text = NULL;
	lpicd.height = 0;
	lpicd.width = 0;
	lpicd.percent_width = 0;
	lpicd.border = 0;
	lpicd.hspace = 0;
	lpicd.vspace = 0;
	lpicd.usemap = NULL;
	lpicd.map = NULL;
	lpicd.ismap = 0;
	lpicd.fptr = NULL;
	lpicd.internal = 0;
	lpicd.urldelayed = 0;
	lpicd.fetched = 0;
	lpicd.cached = 0;
	lpicd.num_colors = 0;
	lpicd.bg_index = 0;
	lpicd.image_data = NULL;
	lpicd.clip_data = NULL;
	lpicd.transparent = 0;
	lpicd.image = None;
	lpicd.clip = None;
	lpicd.alpha = NULL;
	lpicd.alpha_image_data = NULL;
	lpicd.ori_colrs = NULL;
	lpicd.ori_num_colors = 0;
	**/

	/* Get the image in lpicd */
	HtmlGetImage(hw, &lpicd, pcc, False);  /* Don't force load */

	/* Mark it as background image, so ImageRefresh ignores it */
	lpicd.is_bg_image = 1;

	if (lpicd.fetched) {
		int size;

		/* Reduce size if too big in one dimension */
		if (lpicd.height > MaxPixmapHeight) {
			if (lpicd.width > MaxPixmapWidth)
				return;
			lpicd.height = MaxPixmapHeight;
		} else if (lpicd.width > MaxPixmapWidth) {
			int i;
			unsigned char *tmpdata, *ptr, *ptr2;
			XColor colrs[256];

			if (lpicd.height > MaxPixmapHeight)
				return;
			ptr = tmpdata = (unsigned char *)malloc(MaxPixmapWidth *
								lpicd.height);
			ptr2 = lpicd.image_data;
			for (i = 0; i < lpicd.height; i++) {
				ptr = memcpy(ptr, ptr2, MaxPixmapWidth);
				ptr += MaxPixmapWidth;
				ptr2 += lpicd.width;
			}
			lpicd.image_data = tmpdata;
			for (i = 0; i < lpicd.num_colors; i++)
				colrs[i] = lpicd.colrs[i];			
			lpicd.width = MaxPixmapWidth;
			ProcessImageData((Widget)hw, &lpicd, colrs);
			lpicd.cached = 0;
		}
		/* Make bigger if small */
		size = lpicd.width * lpicd.height;
		if (size < 129) {
			int num, neww, newh, i, j, k;
			unsigned char *tmpdata, *ptr, *ptr2;
			XColor colrs[256];

			if (size < 9) {
			    num = 30;
			} else if (size < 33) {
			    num = 20;
			} else if (size < 65) {
			    num = 10;
			} else {
			    num = 5;
			}
			neww = num * lpicd.width;
			newh = num * lpicd.height;
			ptr = tmpdata = (unsigned char *)malloc(neww * newh);

			for (i = 0; i < num; i++) {
			    for (k = 0; k < lpicd.height; k++) {
				ptr2 = lpicd.image_data + (lpicd.width * k);
				for (j = 0; j < num; j++) {
				    memcpy(ptr, ptr2, lpicd.width);
				    ptr += lpicd.width;
				}
			    }
			}
			lpicd.image_data = tmpdata;
			for (i = 0; i < lpicd.num_colors; i++)
				colrs[i] = lpicd.colrs[i];			
			lpicd.width = neww;
			lpicd.height = newh;
			ProcessImageData((Widget)hw, &lpicd, colrs);
			lpicd.cached = 0;
		}
		lpicd.image = InfoToImage(hw, &lpicd, 0, NULL);
		if (!lpicd.fetched || !lpicd.image)
			return;
		if (!lpicd.transparent)
			lpicd.clip = None;
		hw->html.bg_image = 1;
		hw->html.bg_height = lpicd.height;
		hw->html.bg_width = lpicd.width;
		hw->html.bgmap_SAVE = lpicd.image;
		hw->html.bgclip_SAVE = lpicd.clip;
	} else {
		return;
	}
	/*
	** If we have a transparent background image then update the
	** transparent color in the image to match the background color.
	*/
	if (lpicd.transparent) {
	    unsigned long valuemask = GCForeground | GCBackground | GCFunction;
	    XGCValues values;
	    Display *dsp = hw->html.dsp;

	    if (lpicd.clip == None) {
		lpicd.clip = XCreatePixmapFromBitmapData(dsp,
						        XtWindow(hw->html.view),
						        (char *)lpicd.clip_data,
						        lpicd.width,
						        lpicd.height,
						        1, 0, 1);
                hw->html.bgclip_SAVE = lpicd.clip;
	    }
            /* Create a GC if not already done. */
            if (!maskGC) {
                maskGC = XCreateGC(dsp, XtWindow(hw->html.view), 0, 0);
                XCopyGC(dsp, hw->html.drawGC, 0xFFFFFFFF, maskGC);
            }
            /* Clear the background pixels to 0 */
            values.foreground = 0xFFFFFFFF;
            values.background = 0;
            values.function = GXand;
            XChangeGC(dsp, maskGC, valuemask, &values);
            XCopyPlane(dsp, hw->html.bgclip_SAVE, hw->html.bgmap_SAVE, maskGC,
                       0, 0, hw->html.bg_width, hw->html.bg_height, 0, 0, 1);
            /* Set the background pixels to background color */
            values.foreground = 0;
            values.background = hw->core.background_pixel;
            values.function = GXor;
            XChangeGC(dsp, maskGC, valuemask, &values);
            XCopyPlane(dsp, hw->html.bgclip_SAVE, hw->html.bgmap_SAVE, maskGC,
                       0, 0, hw->html.bg_width, hw->html.bg_height, 0, 0, 1);
	}

	/* Create element for it so the image data can be freed later */
	eptr = CreateElement(hw, E_IMAGE, pcc->cur_font, 0, 0, 0, 0, 0, pcc);
	picd = GetImageRec();
	*picd = lpicd;
	hw->html.bg_pic_data = eptr->pic_data = picd;

	return;
}

void hw_do_color(HTMLWidget hw, char *att, char *cname,
		 PhotoComposeContext *pcc)
{
	XColor ecol, col;
	Colormap cmap = hw->core.colormap;
	Display *dsp = hw->html.dsp;
	char *val = cname;
	int allocated = 0;

	if (!att || !*att || !cname || !*cname)
		return;

	/* Clean up leading and trailing crud */
	clean_white_space(val);

	if (*val != '#') {
		switch (TOLOWER(*val)) {
		    /* Adjust named colors per HTML 3.2 */
		    case 'a':
			if (!my_strcasecmp(val, "aqua"))
				val = "#00FFFF";
			break;
		    case 'f':
			if (!my_strcasecmp(val, "fuchsia"))
				val = "#FF00FF";
			break;
		    case 'g':
			if (!my_strcasecmp(val, "green")) {
				val = "#008000";
			} else if (!my_strcasecmp(val, "gray")) {
				val = "#808080";
			}
			break;
		    case 'l':
			if (!my_strcasecmp(val, "lime"))
				val = "#00FF00";
			break;
		    case 'm':
			if (!my_strcasecmp(val, "magenta")) {
				val = "#FF00FF";
			} else if (!my_strcasecmp(val, "maroon")) {
				val = "#800000";
			}
			break;
		    case 'n':
			if (!my_strcasecmp(val, "navy"))
				val = "#000080";
			break;
		    case 'o':
			if (!my_strcasecmp(val, "olive"))
				val = "#808000";
			break;
		    case 'p':
			if (!my_strcasecmp(val, "purple"))
				val = "#800080";
			break;
		    case 's':
			if (!my_strcasecmp(val, "silver"))
				val = "#C0C0C0";
			break;
		    case 't':
			if (!my_strcasecmp(val, "teal"))
				val = "#008080";
			break;
		    /* Add colors not included in Motif */
		    case 'c':
			if (!my_strcasecmp(val, "crimson") &&
			    !XLookupColor(dsp, cmap, val, &ecol, &col))
				val = "#DC1436";
			break;
		    case 'i':
			if (!my_strcasecmp(val, "indigo") &&
			    !XLookupColor(dsp, cmap, val, &ecol, &col))
				val = "#4B0082";
		}
	}
	if (*val != '#') {
		if (XAllocNamedColor(dsp, cmap, val, &col, &ecol)) {
			allocated = 1;
			if ((Vclass != TrueColor) && (Vclass != DirectColor))
				hw->html.allocation_index[col.pixel]++;
		} else if (!XLookupColor(dsp, cmap, val, &ecol, &col)) {
#ifndef DISABLE_TRACE
			if (htmlwTrace || reportBugs)
				fprintf(stderr,	"Bad named color %s\n", cname);
#endif
		} else if ((Vclass != TrueColor) && (Vclass != DirectColor)) {
			FindColor(hw, hw->core.colormap, &col);
			allocated = 1;
		} else {
			return;
		}
	} else if (XParseColor(dsp, cmap, val, &col)) {
		if ((Vclass != TrueColor) && (Vclass != DirectColor)) {
			FindColor(hw, hw->core.colormap, &col);
		} else if (!XAllocColor(dsp, cmap, &col)) {
			return;
		}
		allocated = 1;			
	}
	if (!allocated) {
		if (*val == '#')
			val++;
		/* Now strip out any spaces */
		if (strchr(val, ' ')) {
                	char *ptr;
			char *ptr2 = val;

			for (ptr = val; *ptr; ptr++, ptr2++) {
				while (*ptr && (*ptr == ' '))
					ptr++;
				*ptr2 = *ptr;
			}
			*ptr2 = '\0';
		}
		if (!*val)
			return;
		if (strlen(val) >= 6) {
			int i, r, g, b;
			char t[3];

	                /* Must be valid hex */
            		for (i = 0; i < 6; i++) {
                		if (!strchr("0123456789AaBbCcDdEeFf", val[i])) {
					if (val[i] == 'O') {
						val[i] = '0';
						continue;
					/* A bad hack which needs done right */
					} else if (((i == 1) || (i == 3)) && 
						   (val[i] == ',')) {
	                    			continue;
					}
					return;
                		}
            		}

			/* This hack handles colors specified like #0,0,ff */
			t[2] = 0;
			if (val[1] == ',') {
				t[0] = '0';
				t[1] = val[0];
			} else {
				t[0] = val[0];
				t[1] = val[1];
			}
			sscanf(t, "%x", &r);
			if (val[3] == ',') {
				t[0] = '0';
				t[1] = val[2];
			} else {
				t[0] = val[2];
				t[1] = val[3];
			}
			sscanf(t, "%x", &g);
			t[0] = val[4];
			t[1] = val[5];
			sscanf(t, "%x", &b);

			col.red = ((unsigned) r) << 8;
			col.green = ((unsigned) g) << 8;
			col.blue = ((unsigned) b) << 8;
			col.flags = DoRed | DoGreen | DoBlue;
            
			if ((Vclass != TrueColor) && (Vclass != DirectColor)) {
				FindColor(hw, hw->core.colormap, &col);
			} else if (!XAllocColor(dsp, cmap, &col)) {
				return;
			}
		} else {
			return;
		}
	}

	/* From the first parsing of the HTML */
	if (!my_strcasecmp(att, "preallo"))
		return;

	/* "text" and "bgcolor" are in body tag, "color" is in font tag
	 * "tdcolor" is from <TD> tag, "tcolor" is from <TABLE> tag
	 * "trcolor" is from <TR> tag, "base" is from <BASEFONT> tag
	 */
	if (!my_strcasecmp(att, "text")) {
		hw->manager.foreground = col.pixel;
		DefaultFontColor(hw, pcc, col.pixel);
	} else if (!my_strcasecmp(att, "color")) {
		pcc->fg = pcc->cur_font_color = col.pixel;
	} else if (!my_strcasecmp(att, "tdcolor") ||
		   !my_strcasecmp(att, "tcolor") ||
		   !my_strcasecmp(att, "trcolor")) {
		pcc->bg = col.pixel;
	} else if (!my_strcasecmp(att, "bgcolor")) {
		/* Calculate shadow colors */
		XColor fg, sel, ts, bs;
		XmColorProc calc = XmGetColorCalculation();

		calc(&col, &fg, &sel, &ts, &bs);
		if (XAllocColor(dsp, cmap, &ts)) 
			hw->manager.top_shadow_color = ts.pixel;
		if (XAllocColor(dsp, cmap, &bs)) 
			hw->manager.bottom_shadow_color = bs.pixel;
		hw->core.background_pixel = col.pixel;
		hw->html.view->core.background_pixel = col.pixel;
		hw->html.activeAnchor_bg = col.pixel;
		pcc->bg = col.pixel;
	} else if (!my_strcasecmp(att, "base")) {
		DefaultFontColor(hw, pcc, col.pixel);
	} else if (!my_strcasecmp(att, "link")) {  /* These are in body tag */
		hw->html.anchor_fg = col.pixel;
	} else if (!my_strcasecmp(att, "vlink")) {
		hw->html.visitedAnchor_fg = col.pixel;
	} else if (!my_strcasecmp(att, "alink")) {
		hw->html.activeAnchor_fg = col.pixel;
	}
	return;
}

/* Process an expose event in the View (or drawing area).  This 
 * Can be a regular expose event, or perhaps a GraphicsExpose Event.
 */
static void DrawExpose(Widget w, XtPointer data, XEvent *event)
{
	XEvent NewEvent;
	HTMLWidget hw = (HTMLWidget)data;
	int x, y, x1, y1, width, height;
	int nx, ny, nwidth, nheight;
	Display *dsp;
	Window win;

        if (!(event->xany.type == Expose ||
	      event->xany.type == GraphicsExpose))
                return;              

#ifndef DISABLE_TRACE
        if (htmlwTrace) {
		if (hw->html.is_frame)
	                fprintf(stderr, "Frame ");
		if (event->xany.type == GraphicsExpose)
	                fprintf(stderr, "GraphicsExpose in DrawExpose\n");
		if (event->xany.type == Expose)
	                fprintf(stderr, "Expose in DrawExpose\n");
        }
#endif
        if (event->xany.type == Expose) {
                x = event->xexpose.x;
                y = event->xexpose.y;
                width = event->xexpose.width;
                height = event->xexpose.height;
        } else {                       
                x = event->xgraphicsexpose.x;
                y = event->xgraphicsexpose.y;
                width = event->xgraphicsexpose.width;
                height = event->xgraphicsexpose.height;
        }  

        dsp = hw->html.dsp;
        win = XtWindow(w);

	/* Get rid of any extra expose events.
	 * Be sure to get the entire area of exposure.
	 */
        /* Force all exposure events into the queue */
        XSync(dsp, False);
	while (XCheckWindowEvent(dsp, win, ExposureMask, &NewEvent)) {
		if (NewEvent.xany.type == Expose ||
		    NewEvent.xany.type == GraphicsExpose) {
			if (NewEvent.xany.type == Expose) {
				nx = NewEvent.xexpose.x;
				ny = NewEvent.xexpose.y;
				nwidth = NewEvent.xexpose.width;
				nheight = NewEvent.xexpose.height;
			} else {      
				nx = NewEvent.xgraphicsexpose.x;
				ny = NewEvent.xgraphicsexpose.y;
				nwidth = NewEvent.xgraphicsexpose.width;
				nheight = NewEvent.xgraphicsexpose.height;
			}           
			x1 = x + width;
			y1 = y + height;
                                    
			if (x > nx)
				x = nx;
			if (y > ny)
				y = ny;
			if (x1 < (nx + nwidth))
				x1 = nx + nwidth;
			if (y1 < (ny + nheight))
				y1 = ny + nheight;
			width = x1 - x;
			height = y1 - y;
		}                   
	}    

#ifndef DISABLE_TRACE
        if (htmlwTrace)
                fprintf(stderr, "Doing ViewRedisplay in DrawExpose\n");
#endif
	/* Need to clear it because of progressive display */
	XClearArea(dsp, win, x, y, width, height, False);

	/* Reset cached GC settings */
	hw->html.cur_font = NULL;
	XSetForeground(hw->html.dsp, hw->html.drawGC, hw->html.cur_fg);
	XSetBackground(hw->html.dsp, hw->html.drawGC, hw->html.cur_bg);

	ViewRedisplay(hw, x, y, width, height);
}

static void ResetWidgetsOnResize(HTMLWidget hw)
{       
        WidgetInfo *wptr = hw->html.widget_list;
        
        while (wptr) {
                if (wptr->w)
                        wptr->seeable = 1;
                wptr = wptr->next;   
        }
        return;                      
}

void ScrollWidgets(HTMLWidget hw)
{
	WidgetInfo *wptr = hw->html.widget_list;
	FrameInfo *fptr;
	int xval = hw->html.scroll_x;
	int yval = hw->html.scroll_y;
	int view_height = hw->html.view_height;
	int view_width = hw->html.view_width;
	int x, y;
	int scrolled = 0;

#ifndef DISABLE_TRACE
        if (htmlwTrace)
		fprintf(stderr, "In Scrollwidgets\n");
#endif
	while (wptr) {
		if (wptr->w) {
                        x = wptr->x - xval;
                        y = wptr->y - yval;

                        /* lower_right on screen?
                         * lower_left on screen?
                         * upper_right on screen?
                         * upper_left on screen?
                         *           
                         * If any of the above, move the widget, otherwise
                         * it is not "seeable".  Incredible speed for many
                         * widget pages.  If no movement, then is probably
			 * an expose event which requires a repaint.
                         */     
                        if ((((y + wptr->eptr->height) >= 0) &&
			     (y <= view_height)) &&
                            (((x + wptr->eptr->width) >= 0) &&
			     (x <= view_width))) {
                                XtMoveWidget(wptr->w, x, y);
				scrolled = 1;
                        } else if (wptr->seeable) {
                                wptr->seeable = 0;
				if (wptr->mapped) {
					wptr->mapped = False;
					XtSetMappedWhenManaged(wptr->w, False);
					scrolled = 1;
				}
                        }  

		}
		wptr = wptr->next;
	}
	fptr = hw->html.iframe_list;
	while (fptr) {
		if (fptr->iframe) {
                        x = fptr->frame_x - xval;
                        y = fptr->frame_y - yval;

                        if ((((y + fptr->frame_height) >= 0) && 
			     (y <= view_height)) &&
                            (((x + fptr->frame_width) >= 0) &&
			     (x <= view_width))) {
                                XtMoveWidget(fptr->iframe, x + 2, y + 2);
				scrolled = 1;
                        } else if (fptr->seeable) {
                                fptr->seeable = 0;
				if (fptr->mapped) {
					fptr->mapped = False;
					XtSetMappedWhenManaged(fptr->iframe,
							       False);
					scrolled = 1;
				}
                        }  

		}
		fptr = fptr->frame_next;
	}

	/* Clean up previously covered areas */
	if (scrolled) {
		XEvent event;
		Display *dsp = hw->html.dsp;
		Window win = XtWindow(hw->html.view);
		int width = 0;
		int height = 0;
		int nx, ny, x1, y1, nwidth, nheight;

		XSync(dsp, False);
		scrolled = 0;
		while (XCheckWindowEvent(dsp, win, ExposureMask, &event)) {
			if (event.xany.type == Expose ||
			    event.xany.type == GraphicsExpose) {
				if (event.xany.type == Expose) {
					nx = event.xexpose.x;
					ny = event.xexpose.y;
					nwidth = event.xexpose.width;
					nheight = event.xexpose.height;
				} else {      
					nx = event.xgraphicsexpose.x;
					ny = event.xgraphicsexpose.y;
					nwidth = event.xgraphicsexpose.width;
					nheight = event.xgraphicsexpose.height;
				}           
				if (!scrolled) {
					scrolled = 1;
					x = nx;
					y = ny;
				}
				x1 = x + width;
				y1 = y + height;
 
				if (x > nx)
					x = nx;
				if (y > ny)
					y = ny;
				if (x1 < (nx + nwidth))
					x1 = nx + nwidth;
				if (y1 < (ny + nheight))
					y1 = ny + nheight;
				width = x1 - x;
				height = y1 - y;
			}
		}                   
		if (scrolled) {
#ifndef DISABLE_TRACE
		        if (htmlwTrace)
 	        		fprintf(stderr,
					"ScrollWidgets doing ViewRedisplay\n");
#endif
			ViewRedisplay(hw, x, y, width, height);
		}
	}
}

/*
 * Either the vertical or hortizontal scrollbar has been moved
 */
static void ScrollToPos(Widget w, HTMLWidget hw, int value)
{
	static unsigned long valuemask = GCGraphicsExposures;
	XGCValues values;
	Display *dsp = hw->html.dsp;
	Window win = XtWindow(hw->html.view);
	int vheight = hw->html.view_height;
	int vwidth = hw->html.view_width;

#ifndef DISABLE_TRACE
        if (htmlwTrace)
		fprintf(stderr, "ScrollToPos, value = %d\n", value);
#endif
	/*
	 * Special code incase the scrollbar is "moved" before we have a window
	 * (if we have a GC we have a window)
	 */
	if (!hw->html.drawGC) {
		if (w == hw->html.vbar) {
			hw->html.scroll_y = value;
		} else if (w == hw->html.hbar) {
			hw->html.scroll_x = value;
		}
		return;
	}
	/* Reset cached GC settings */
	hw->html.cur_font = NULL;
	XSetForeground(hw->html.dsp, hw->html.drawGC, hw->html.cur_fg);
	XSetBackground(hw->html.dsp, hw->html.drawGC, hw->html.cur_bg);

	/* Disable GraphicExpose events (from CopyArea) */
	values.graphics_exposures = False;
	XChangeGC(dsp, hw->html.drawGC, valuemask, &values);
	/*
	 * If we've moved the vertical scrollbar
	 */
	if (w == hw->html.vbar) {
		/*
		 * We've scrolled down.  Copy up the untouched part of the
		 * window.  Then Clear and redraw the new area exposed.
		 */
		if (value > hw->html.scroll_y) {
			int dy = value - hw->html.scroll_y;

			if (dy > vheight) {
				hw->html.scroll_y = value;
				XClearArea(dsp, win, 0, 0, vwidth,
					   vheight, False);
				ViewRedisplay(hw, 0, 0, vwidth, vheight);
			} else {
				XCopyArea(dsp, win, win, hw->html.drawGC, 0, dy,
					  vwidth, vheight - dy, 0, 0);
				hw->html.scroll_y = value;
				XClearArea(dsp, win, 0, vheight - dy,
					   vwidth, dy, False);
				ViewRedisplay(hw, 0, vheight - dy, vwidth, dy);
			}
		}
		/*
		 * We've scrolled up.  Copy down the untouched part of the
		 * window.  Then Clear and redraw the new area exposed.
		 */
		else if (value < hw->html.scroll_y) {
			int dy = hw->html.scroll_y - value;

			if (dy > vheight) {
				hw->html.scroll_y = value;
				XClearArea(dsp, win, 0, 0, vwidth,
					   vheight, False);
				ViewRedisplay(hw, 0, 0, vwidth, vheight);
			} else {
				XCopyArea(dsp, win, win, hw->html.drawGC, 0, 0,
					  vwidth, vheight - dy, 0, dy);
				hw->html.scroll_y = value;
				XClearArea(dsp, win, 0, 0, vwidth, dy, False);
				ViewRedisplay(hw, 0, 0, vwidth, dy);
			}
		}
	}
	/*
	 * Else we've moved the horizontal scrollbar
	 */
	else if (w == hw->html.hbar) {
		/*
		 * We've scrolled right. Copy left the untouched part of the
		 * window.  Then Clear and redraw the new area exposed.
		 */
		if (value > hw->html.scroll_x) {
			int dx = value - hw->html.scroll_x;

			if (dx > vwidth) {
				hw->html.scroll_x = value;
				XClearArea(dsp, win, 0, 0, vwidth,
					   vheight, False);
				ViewRedisplay(hw, 0, 0, vwidth, vheight);
			} else {
				XCopyArea(dsp, win, win, hw->html.drawGC, dx, 0,
					  vwidth - dx, vheight, 0, 0);
				hw->html.scroll_x = value;
				XClearArea(dsp, win, vwidth - dx, 0,
					   dx, vheight, False);
				ViewRedisplay(hw, vwidth - dx, 0, dx, vheight);
			}
		}
		/*
		 * We've scrolled left.  Copy right the untouched part of
		 * the window.  Then Clear and redraw the new area exposed.
		 */
		else if (value < hw->html.scroll_x) {
			int dx = hw->html.scroll_x - value;

			if (dx > vwidth) {
				hw->html.scroll_x = value;
				XClearArea(dsp, win, 0, 0, vwidth,
					   vheight, False);
				ViewRedisplay(hw, 0, 0, vwidth, vheight);
			} else {
				XCopyArea(dsp, win, win, hw->html.drawGC, 0, 0,
					  vwidth - dx, vheight, dx, 0);
				hw->html.scroll_x = value;
				XClearArea(dsp, win, 0, 0, dx, vheight, False);
				ViewRedisplay(hw, 0, 0, dx, vheight);
			}
		}
	}
	ScrollWidgets(hw);

	/* Enable exposure events */
	values.graphics_exposures = True;
	XChangeGC(dsp, hw->html.drawGC, valuemask, &values);
}

/*
 * Either the vertical or hortizontal scrollbar has been moved
 */
static void ScrollMove(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmScrollBarCallbackStruct *sc = (XmScrollBarCallbackStruct *)call_data;

	ScrollToPos(w, (HTMLWidget)client_data, sc->value);
}

static XtIntervalId Stimer = (XtIntervalId)0;
static struct scroll_info {
	int value;
	Widget w;
	HTMLWidget hw;
} Sinfo;

static void ScrollTimer(XtPointer cld, XtIntervalId *id)
{
	ScrollToPos(Sinfo.w, Sinfo.hw, Sinfo.value);
	Stimer = (XtIntervalId)0;
}

/* Drop excessive drag scroll callbacks */
static void DragScrollMove(Widget w, XtPointer client_data, XtPointer call_data)
{
	XmScrollBarCallbackStruct *sc = (XmScrollBarCallbackStruct *)call_data;

	Sinfo.value = sc->value;
	Sinfo.hw = (HTMLWidget)client_data;
	Sinfo.w = w;
	if (Stimer)
		return;
        Stimer = XtAppAddTimeOut(app_con, 10, (XtTimerCallbackProc)ScrollTimer,
				 NULL);
}

static void Realize(Widget w, XtValueMask *valueMask,
		    XSetWindowAttributes *attrs)     
{       
        HTMLWidget hw = (HTMLWidget)w;
        
        XtCreateWindow(w, InputOutput, (Visual *)CopyFromParent,
		       *valueMask, attrs);
        XtRealizeWidget(hw->html.view);
        XtRealizeWidget(hw->html.vbar);
        XtRealizeWidget(hw->html.hbar);
#ifndef DISABLE_TRACE
	if (htmlwTrace)
		fprintf(stderr, "Realized HTMLwidget\n");
#endif
}

/* Create the horizontal and vertical scroll bars.
 * Size them later.
 */
static void CreateScrollbars(HTMLWidget hw)
{
	Arg arg[5];
	Cardinal argcnt = 0;
	XtTranslations trans;

	if (!hw->html.is_frame) {
		XtSetArg(arg[argcnt], XmNwidth, 10);
		argcnt++;
		XtSetArg(arg[argcnt], XmNheight, 10);
		argcnt++;
	} else {
		XtSetArg(arg[argcnt], XmNwidth, hw->core.width);
		argcnt++;
		XtSetArg(arg[argcnt], XmNheight, hw->core.height);
		argcnt++;
	}
	XtSetArg(arg[argcnt], XmNmarginWidth, 0);
	argcnt++;
	XtSetArg(arg[argcnt], XmNmarginHeight, 0);
	argcnt++;
	hw->html.view = XtCreateWidget("View", xmDrawingAreaWidgetClass,
				       (Widget)hw, arg, argcnt);
	XtManageChild(hw->html.view);
	/*
	 * For the view widget catch all Expose and GraphicsExpose
	 * events.  Replace its translations with ours, and make
	 * sure all the actions are in order.
	 */
	XtAddEventHandler((Widget)hw->html.view,
			  ExposureMask | VisibilityChangeMask, True,
			  (XtEventHandler)DrawExpose, (XtPointer)hw);
	/*
	 * As described previously, for some reason with Motif 1.2/X11R5
	 * the list actionsList is corrupted when we get here,
	 * so we have to use the special copy SpareActionsList
	 */
	XtAppAddActions(app_con, SpareActionsList, XtNumber(SpareActionsList));
	trans = XtParseTranslationTable(defaultTranslations);
	XtSetArg(arg[0], XtNtranslations, trans);
	XtSetValues(hw->html.view, arg, 1);

	/* Vert scrollbar */
	XtSetArg(arg[0], XmNorientation, XmVERTICAL);
	XtSetArg(arg[1], XtNwidth, VERT_SCROLL_WIDTH);
	hw->html.vbar = XtCreateWidget("Vbar", xmScrollBarWidgetClass,
				       (Widget)hw, arg, 2);
	XtAddCallback(hw->html.vbar, XmNvalueChangedCallback,
		      (XtCallbackProc)ScrollMove, (XtPointer)hw);
	XtAddCallback(hw->html.vbar, XmNdragCallback,
		      (XtCallbackProc)DragScrollMove, (XtPointer)hw);

	/* Horiz scrollbar */
	XtSetArg(arg[0], XmNorientation, XmHORIZONTAL);
	XtSetArg(arg[1], XtNheight, HORIZ_SCROLL_HEIGHT);
	hw->html.hbar = XtCreateWidget("Hbar", xmScrollBarWidgetClass,
				       (Widget)hw, arg, 2);
	XtAddCallback(hw->html.hbar, XmNvalueChangedCallback,
		      (XtCallbackProc)ScrollMove, (XtPointer)hw);
	XtAddCallback(hw->html.hbar, XmNdragCallback,
		      (XtCallbackProc)DragScrollMove, (XtPointer)hw);
}

/* Return the width of the vertical scrollbar */
static Dimension VbarWidth(HTMLWidget hw)
{
	return(VERT_SCROLL_WIDTH);
}

/* Return the height of the horizontal scrollbar */
Dimension HbarHeight(HTMLWidget hw)
{
	return(HORIZ_SCROLL_HEIGHT);
}

/*
 * Resize and set the min and max values of the scrollbars.  Position viewing
 * area based on scrollbar locations.
 */
static void ConfigScrollBars(HTMLWidget hw)
{
	Arg arg[20];
	Cardinal argcnt;
	Dimension st = 0;
	int ss;
	int vheight = hw->html.view_height;
	int vwidth = hw->html.view_width;

	/* Don't move frames */
	if (!hw->html.is_frame) {
		/* Move and size the viewing area */
		st = hw->manager.shadow_thickness;
		XtMoveWidget(hw->html.view, st, st);
		XtResizeWidget(hw->html.view, vwidth, vheight,
			       hw->html.view->core.border_width);
	}
	/* Set up vertical scrollbar */
	if (hw->html.use_vbar) {
		int maxv;

		/* Size the vertical scrollbar to the height of
		 * the viewing area
		 */
		XtResizeWidget(hw->html.vbar, hw->html.vbar->core.width,
			       vheight + (2 * st),
			       hw->html.vbar->core.border_width);

		/* Set the slider size to be the percentage of the
		 * viewing area that the viewing area is of the
		 * document area.  Or set it to 1 if that isn't possible.
		 */
		if (hw->html.doc_height == 0) {
			ss = 1;
		} else {
#ifndef DISABLE_TRACE
			if (htmlwTrace)
			     fprintf(stderr, "view_height %d, doc_height %d\n",
				     vheight, hw->html.doc_height);
#endif
                        /* Added by marca: this produces results *very* close
			 * (~1 pixel) to the original scrolled window behavior.
			 */
                        ss = vheight;
		}
		if (ss < 1)
			ss = 1;
#ifndef DISABLE_TRACE
		if (htmlwTrace)
			fprintf(stderr, "Computed ss to be %d\n", ss);
#endif
		/* If resizing of the document has made scroll_y
		 * greater than the max, we want to hold it at the max.
		 */
		maxv = hw->html.doc_height - vheight;
		if (maxv < 0)
			maxv = 0;
		if (hw->html.scroll_y > maxv)
			hw->html.scroll_y = maxv;
		/* Prevent the Motif max value and slider size
		 * from going to zero, which is illegal
		 */
		maxv += ss;
		if (maxv < 1)
			maxv = 1;
		/* Motif will not allow the actual value to be equal to
		 * its max value.  Adjust accordingly.
		 * Since we might decrease scroll_y, cap it at zero.
		 */
		if (hw->html.scroll_y >= maxv)
			hw->html.scroll_y = maxv - 1;
		if (hw->html.scroll_y < 0)
			hw->html.scroll_y = 0;
		argcnt = 0;
		XtSetArg(arg[argcnt], XmNminimum, 0);
		argcnt++;
		XtSetArg(arg[argcnt], XmNmaximum, maxv);
		argcnt++;
		XtSetArg(arg[argcnt], XmNvalue, hw->html.scroll_y);
		argcnt++;
		XtSetArg(arg[argcnt], XmNsliderSize, ss);
		argcnt++;
                XtSetArg(arg[argcnt], XmNincrement, DEFAULT_INCREMENT);
		argcnt++;
                XtSetArg(arg[argcnt], XmNpageIncrement, 
                         vheight > DEFAULT_INCREMENT ? 
                         vheight - DEFAULT_INCREMENT : 1);
		argcnt++;
		XtSetValues(hw->html.vbar, arg, argcnt);
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			XtVaGetValues(hw->html.vbar, XmNsliderSize, &ss, NULL);
			fprintf(stderr, "real vertical slider size %d\n", ss);
		}
#endif
	}
	/* Set up horizontal scrollbar */
	if (hw->html.use_hbar) {
		int maxv;

		/* Size the horizontal scrollbar to the width of
		 * the viewing area
		 */
		XtResizeWidget(hw->html.hbar, vwidth + (2 * st),
			       hw->html.hbar->core.height,
			       hw->html.hbar->core.border_width);
		/* Set the slider size to be the percentage of the
		 * viewing area that the viewing area is of the
		 * document area.  Or set it to 1 if that isn't possible.
		 */
		if (hw->html.doc_width == 0) {
			ss = 1;
		} else {
                        /* marca: this produces results *very* close (~1 pixel)
                         * to the original scrolled window behavior. */
                        ss = vwidth;
		}
		if (ss < 1)
			ss = 1;
		/* If resizing of the document has made scroll_x
		 * greater than the max, we want to hold it at the max.
		 */
		maxv = hw->html.doc_width - vwidth;
		if (maxv < 0)
			maxv = 0;
		if (hw->html.scroll_x > maxv)
			hw->html.scroll_x = maxv;
		/* Prevent the Motif max value and slider size
		 * from going to zero, which is illegal
		 */
		maxv += ss;
		if (maxv < 1)
			maxv = 1;
		/*
		 * Motif will not allow the actual value to be equal to
		 * its max value.  Adjust accordingly.
		 * Since we might decrease scroll_x, cap it at zero.
		 */
		if (hw->html.scroll_x >= maxv)
			hw->html.scroll_x = maxv - 1;
		if (hw->html.scroll_x < 0)
			hw->html.scroll_x = 0;
		argcnt = 0;
		XtSetArg(arg[argcnt], XmNminimum, 0);
		argcnt++;
		XtSetArg(arg[argcnt], XmNmaximum, maxv);
		argcnt++;
		XtSetArg(arg[argcnt], XmNvalue, hw->html.scroll_x);
		argcnt++;
		XtSetArg(arg[argcnt], XmNsliderSize, ss);
		argcnt++;
                XtSetArg(arg[argcnt], XmNincrement, DEFAULT_INCREMENT);
		argcnt++;
                XtSetArg(arg[argcnt], XmNpageIncrement, 
                         vwidth > DEFAULT_INCREMENT ? 
                         vwidth - DEFAULT_INCREMENT : 1);
		argcnt++;
		XtSetValues(hw->html.hbar, arg, argcnt);
#ifndef DISABLE_TRACE
		if (htmlwTrace) {
			XtVaGetValues(hw->html.hbar, XmNsliderSize, &ss, NULL);
			fprintf(stderr, "real horizontal slider size %d\n", ss);
   	 	}
#endif
	}
}

/* Reformat the window and scrollbars.  May be called due to changed document */
static void ReformatWindow(HTMLWidget hw)
{
	int returned_y, new_width, width;
	Dimension swidth = VbarWidth(hw);
	Dimension sheight = HbarHeight(hw);
	Dimension st = 0;

	/* Reset background image, any previous one freed by now */
	hw->html.bg_image = 0;
	hw->html.bgmap_SAVE = None;
	hw->html.bgclip_SAVE = None;
	/*
	 * Find the current scrollbar sizes, and shadow thickness and
	 * format the document to the current window width
	 * (assume a vertical scrollbar)
	 */
	if (!hw->html.is_frame)
		st = hw->manager.shadow_thickness;
	if (hw->core.width <= swidth)
		hw->core.width = swidth + 10;
	new_width = hw->core.width - swidth;
	returned_y = FormatAll(hw, &new_width);

	/* Check if height of unneeded hbar is causing a vbar */
	if (hw->html.use_vbar) {
		width = new_width - (2 * hw->html.margin_width);
		if (hw->html.max_pre_width > width) {
			width = hw->html.max_pre_width;
		} else {
			width -= 20 * hw->html.margin_width / 100;
		}
		width += 2 * hw->html.margin_width;
		if ((width <= hw->html.view_width) &&
		    (returned_y <= hw->core.height))
			hw->html.use_vbar = False;
	}

	/* If we need the vertical scrollbar, place and manage it,
	 * and store the current viewing area width.
	 */
	if (hw->html.use_vbar || (hw->html.scroll_bars == FRAME_SCROLL_YES)) {
		XtMoveWidget(hw->html.vbar, hw->core.width - swidth, 0);
		XtManageChild(hw->html.vbar);
		hw->html.view_width = hw->core.width - swidth - (2 * st);
		hw->html.use_vbar = True;
	} else {
		/* Else we were wrong to assume a vertical scrollbar.
		 * Remove it.
		 */
		XtUnmanageChild(hw->html.vbar);
		hw->html.scroll_y = 0;
		/* Use the full width for frames */
		if (hw->html.is_frame) {
			new_width = hw->core.width;
			/* These get freed by Formatall */
			hw->html.bg_image = 0;
			hw->html.bgmap_SAVE = None;
			hw->html.bgclip_SAVE = None;
			returned_y = FormatAll(hw, &new_width);
			hw->html.view_width = hw->core.width;
		} else {
			hw->html.view_width = hw->core.width - swidth -(2 * st);
		}
	}
	/* Calculate the actual max width and height of the complete formatted
	 * document.  The max width may exceed the preformatted width due to
	 * special factors in the formatting of the widget.  Use the max of
	 * the 2 here, but leave max_pre_width unchanged for future formatting
	 * calls.  new_width includes the margins, and hw->html.max_pre_width
	 * does not, fix that here.
	 */
	new_width -= 2 * hw->html.margin_width;
	if (hw->html.max_pre_width > new_width) {
		new_width = hw->html.max_pre_width;
	/* If the maximum width derives from a formatted, as opposed to
	 * unformatted piece of text, allow a 20% of margin width slop
	 * over into the margin to cover up a minor glick with terminating
	 * punctuation after anchors at the end of the line.
	 */
	} else {
		new_width -= 20 * hw->html.margin_width / 100;
	}
	hw->html.doc_height = returned_y;
	hw->html.doc_width = new_width + (2 * hw->html.margin_width);
	if (hw->html.view_width > hw->html.doc_width)
		hw->html.doc_width = hw->html.view_width;

	/* If we need a horizontal scrollbar, place and manage it.
	 * Save the height of the current viewing area.
	 */
	if (((hw->html.scroll_bars == FRAME_SCROLL_YES) ||
	     (hw->html.doc_width > hw->html.view_width)) &&
	    (hw->html.scroll_bars != FRAME_SCROLL_NONE)) {
		hw->html.use_hbar = True;
		XtMoveWidget(hw->html.hbar, 0, hw->core.height - sheight);
		XtManageChild(hw->html.hbar);
		hw->html.view_height = hw->core.height - sheight - (2 * st);
	} else {
		/* Else we don't need a horizontal scrollbar.
		 * Remove it and save the current viewing area height.
		 */
		hw->html.use_hbar = False;
		XtUnmanageChild(hw->html.hbar);
		hw->html.scroll_x = 0;
		hw->html.view_height = hw->core.height - (2 * st);
	}

	/* Configure the scrollbar min, max, and slider sizes */
	ConfigScrollBars(hw);

#ifndef DISABLE_TRACE
        if (htmlwTrace)
                fprintf(stderr, "Completed ReformatWindow\n");
#endif
}

/* We're a happy widget.  We let any child move or resize themselves
 * however they want, we don't care.
 */
static XtGeometryResult GeometryManager(Widget w, XtWidgetGeometry *request,
					XtWidgetGeometry *reply)
{
	reply->x = request->x;
	reply->y = request->y;
	reply->width = request->width;
	reply->height = request->height;
	reply->border_width = request->border_width;
	reply->request_mode = request->request_mode;
	return (XtGeometryYes);
}

/* Initialize is called when the widget is first initialized.
 * Check to see that all the starting resources are valid.
 */
static void Initialize(HTMLWidget request, HTMLWidget nw)
{
	static GC HTMLdrawGC = NULL;

	if (!app_con)
		app_con = XtWidgetToApplicationContext((Widget) nw);

	/* Make sure height and width are not zero. */
	if (nw->core.width == 0)
		nw->core.width = nw->html.margin_width << 1;
	if (nw->core.width == 0)
		nw->core.width = 10;
	if (nw->core.height == 0)
		nw->core.height = nw->html.margin_height << 1;
	if (nw->core.height == 0)
		nw->core.height = 10;

	/* Make sure the underline numbers are within bounds. */
	if (nw->html.num_anchor_underlines < 0)
		nw->html.num_anchor_underlines = 0;
	if (nw->html.num_anchor_underlines > MAX_UNDERLINES)
		nw->html.num_anchor_underlines = MAX_UNDERLINES;
	if (nw->html.num_visitedAnchor_underlines < 0)
		nw->html.num_visitedAnchor_underlines = 0;
	if (nw->html.num_visitedAnchor_underlines > MAX_UNDERLINES)
		nw->html.num_visitedAnchor_underlines = MAX_UNDERLINES;

	nw->html.map_list = NULL;
	nw->html.formatted_elements = NULL;
	nw->html.widget_list = NULL;
	nw->html.widget_focus = NULL;
	nw->html.iframe_list = NULL;
	nw->html.form_list = NULL;
	nw->html.html_objects = NULL;
	nw->html.html_header_objects = NULL;
	nw->html.html_footer_objects = NULL;
	nw->html.html_refresh_objects = NULL;
	nw->html.header_text = NULL;
	nw->html.footer_text = NULL;
	nw->html.charset = NULL;
	nw->html.max_pre_width = 0;
	nw->html.blinking_elements = NULL;
	nw->html.image_src = NULL;

	/* Initialize the color allocation index */
	memset(nw->html.allocation_index, 0, 256 * sizeof(int));

	nw->html.nframe = 0;
	nw->html.frames = NULL;
	nw->html.frame_callback = NULL;

	nw->html.dsp = XtDisplay(nw);

	/* Create the scrollbars.  Find their dimensions and then decide which
	 * scrollbars you will need, and what the dimensions of the viewing
	 * area are.  Start assuming a vertical scrollbar and a horizontal one.
	 * Then remove vertical if short enough, and remove horizontal if
	 * narrow enough.
	 */
	nw->html.vbar = nw->html.hbar = nw->html.view = NULL;
	CreateScrollbars(nw);
	nw->html.scroll_x = nw->html.scroll_y = 0;

	/* Initialize private widget resources */
	nw->html.select_start = nw->html.select_end = NULL;
	nw->html.sel_start_pos = nw->html.sel_end_pos = 0;
	nw->html.new_start = nw->html.new_end = NULL;
	nw->html.new_start_pos = nw->html.new_end_pos = 0;
	nw->html.active_anchor = NULL;
	nw->html.press_x = nw->html.press_y = 0;

	nw->html.cached_tracked_ele = NULL;

        nw->html.top_color_SAVE = nw->manager.top_shadow_color;
        nw->html.bottom_color_SAVE = nw->manager.bottom_shadow_color;
	nw->html.foreground_SAVE = nw->manager.foreground;
	nw->html.anchor_fg_SAVE = nw->html.anchor_fg;
        nw->html.visitedAnchor_fg_SAVE = nw->html.visitedAnchor_fg;
        nw->html.activeAnchor_fg_SAVE = nw->html.activeAnchor_fg;
        nw->html.activeAnchor_bg_SAVE = nw->html.activeAnchor_bg;
	nw->html.bgmap_SAVE = None;
	nw->html.bgclip_SAVE = None;
	nw->html.bg_image = 0;
	nw->html.focus_follows_mouse = 0;
	nw->html.ignore_setvalues = 0;
	nw->html.changing_font = 0;
	nw->html.pushfont_count = 0;
	nw->html.font_save_count = 0;
	nw->html.fontstack = NULL;
	nw->html.underline_yoffset = -1;
	nw->html.table_cell_has_bg = 0;
	nw->html.draw_count = 0;
	nw->html.drawing = 0;
	nw->html.refresh_count = 0;
	nw->html.refresh_timer = 0;
	nw->html.blink_timer = 0;
	nw->html.workprocid = 0;

	/* Initialize cursor used when pointer is inside anchor. */
        if (in_anchor_cursor == (Cursor)NULL)
        	in_anchor_cursor = XCreateFontCursor(nw->html.dsp, XC_hand2);

	/* Make sure we have a valid GC to draw with */
	if (!HTMLdrawGC) {
		unsigned long valuemask;
		XGCValues values;

		values.function = GXcopy;
		values.plane_mask = AllPlanes;
		values.foreground = nw->manager.foreground;
		values.background = nw->core.background_pixel;
		values.fill_style = FillSolid;
		valuemask = GCFunction | GCPlaneMask | GCForeground |
			    GCBackground | GCFillStyle;
		HTMLdrawGC = XCreateGC(nw->html.dsp,
				       DefaultRootWindow(nw->html.dsp),
				       valuemask, &values);
	}
	nw->html.cur_fg = nw->manager.foreground;
	nw->html.cur_bg = nw->core.background_pixel;
	nw->html.cur_font = NULL;
	nw->html.drawGC = HTMLdrawGC;

#ifndef DISABLE_TRACE
	if (htmlwTrace)
		fprintf(stderr, "Initialized HTMLwidget\n");
#endif
        return;
}

/* This is called by various routines.  It is passed a rectangle
 * in the viewing area, and it redisplays that portion of the
 * underlying document area.
 */
static void ViewRedisplay(HTMLWidget hw, int x, int y, int width, int height)
{
	int doc_x = x;
	int doc_y = y;
	int doc_end_x, doc_end_y;
	ElemInfo *eptr;

	if (!XtIsRealized((Widget)hw))
		return;

	if (hw->html.bg_image)
		HTMLDrawBackgroundImage((Widget)hw, x, y, width, height);

	/* Reset underline offset */
	hw->html.underline_yoffset = -1;

	/* Use scrollbar values to map from view space to document space. */
	doc_x += hw->html.scroll_x;
	doc_y += hw->html.scroll_y;
	doc_end_x = doc_x + width;
	doc_end_y = doc_y + height;

	/* Store values so CellRefresh can get them */
	hw->html.redisplay_x = doc_x;
	hw->html.redisplay_y = doc_y;
	hw->html.redisplay_width = width;
	hw->html.redisplay_height = height;

	/* If redisplaying whole page, then get rid of any expose events */
	if (!x && !y && (width == hw->html.view_width) &&
	    (height == hw->html.view_height)) {
		Display *dsp = hw->html.dsp;
		Window win = XtWindow(hw->html.view);
		XEvent event;

		XSync(dsp, False);
		while (XCheckWindowEvent(dsp, win, ExposureMask, &event))
			;
	}

	DidAllImages = 1;
	/* Find Element to Refresh */
	eptr = hw->html.formatted_elements;
	while (eptr) {
		if (eptr->type == E_APPLET) {
			eptr = RefreshElement(hw, eptr);
			eptr = eptr->next;
			continue;
		}
		if (((eptr->y + eptr->height) < doc_y) ||
		    (eptr->y > doc_end_y) ||
		    ((eptr->x + eptr->width) < doc_x) ||
		    (eptr->x > doc_end_x)) {
			/* Need to reset underlining properly */
			if (eptr->type == E_CR)
				hw->html.underline_yoffset = -1;
			if (eptr->type == E_IMAGE)
				DidAllImages = 0;
			eptr = eptr->next;
			continue;
		}
		eptr = RefreshElement(hw, eptr);
		eptr = eptr->next;
	}
#ifndef DISABLE_TRACE
        if (htmlwTrace || refreshTrace)
                fprintf(stderr, "Completed ViewRedisplay\n");
#endif
}

/* This is called by ImagePlace to do progressive display.  It is
 * passed the element pointer to start displaying at.
 */
void ProgressiveDisplay(HTMLWidget hw, ElemInfo *eptr, PhotoComposeContext *pcc)
{
	int vheight = hw->html.view_height;
	int vwidth = hw->html.view_width;

	/* Only refresh if we have a window already */
	if (!XtIsRealized((Widget)hw))
		return;

	/* Will be equal first time called for this page */
	if (eptr == hw->html.formatted_elements) {
		int r = hw->manager.top_shadow_color;
		int b = hw->manager.bottom_shadow_color;

		hw->html.view->core.background_pixel = 
					      hw->core.background_pixel ? 0 : 1;
		hw->manager.top_shadow_color = r ? 0 : 1;
		hw->manager.bottom_shadow_color = b ? 0 : 1;
		XtVaSetValues(hw->html.view,
			      XmNbackground, hw->core.background_pixel,
			      XmNtopShadowColor, r,
			      XmNbottomShadowColor, b,
			      NULL);
		XtVaSetValues((Widget)hw,
			      XmNbackground, hw->core.background_pixel,
			      XmNtopShadowColor, r,
			      XmNbottomShadowColor, b,
			      NULL);
		XClearArea(hw->html.dsp, XtWindow(hw->html.view),
			   0, 0, 0, 0, False);

		if (hw->html.bg_image)
			HTMLDrawBackgroundImage((Widget)hw, 0, 0,
						vwidth, vheight);
		/* Reset underline offset */
		hw->html.underline_yoffset = -1;
	}

	hw->html.redisplay_x = 0;
	hw->html.redisplay_y = 0;
	hw->html.redisplay_width = vwidth;
	hw->html.redisplay_height = vheight;

	while (eptr) {
		/* Skip if not within view area or if transparent image in
		 * table with image background */
		if ((eptr->y > vheight) || (eptr->x > vwidth) ||
		    ((eptr->type == E_IMAGE) && pcc->in_table &&
		     hw->html.bg_image && eptr->pic_data->transparent)) {
			eptr = eptr->next;
			continue;
		}
		eptr = RefreshElement(hw, eptr);
		eptr = eptr->next;
	}
#ifndef DISABLE_TRACE
        if (htmlwTrace)
                fprintf(stderr, "Completed ProgressiveDisplay\n");
#endif
}

static void TextBlink(XtPointer cld, XtIntervalId *id)
{
	BlinkInfo *blink = (BlinkInfo *) cld;
	ElemInfo *eptr = blink->hw->html.blinking_elements;
	HTMLWidget hw = blink->hw;
	int width = hw->html.view_width;
	int height = hw->html.view_height;
	int doc_x = hw->html.scroll_x;
	int doc_y = hw->html.scroll_y;
	int blinking = hw->html.blinking_text;

	hw->html.blink_timer = 0;

	/* Quit if display changed */
	if (blink->drawing != hw->html.draw_count) {
		free(blink);
		return;
	}

	/* Reset cached GC settings */
	hw->html.cur_font = NULL;
	XSetForeground(hw->html.dsp, hw->html.drawGC, hw->html.cur_fg);
	XSetBackground(hw->html.dsp, hw->html.drawGC, hw->html.cur_bg);
	/* Tell ExtendAdjust on other windows that we messed with the GC */
	Select_GC_Reset = 1;

	while (eptr) {
		/* Skip if not in view area */
		if (((eptr->y + eptr->height) < doc_y) || 
		    (eptr->y > (doc_y + height)) ||
		    ((eptr->x + eptr->width) < doc_x) ||
		    (eptr->x > (doc_x + width))) {
			/* If blinking turned off, mark text visible. */
			if (!blinking)
				eptr->blink = 0;
			eptr = eptr->blink_next;
			continue;
		}
		/* If blinking turned off, make text visible. */
		if (eptr->blink || !blinking) {
			eptr->blink = 0;
		} else {
			eptr->blink = 1;
		}
		TextRefresh(hw, eptr, 0, eptr->edata_len - 2, eptr->blink);
		eptr = eptr->blink_next;
	}

	if (blinking) {
		hw->html.blink_timer = XtAppAddTimeOut(app_con,
						 hw->html.blink_time,
						 (XtTimerCallbackProc)TextBlink,
						 (XtPointer)blink);
	} else {
		free(blink);
	}
}

void ViewClearAndRefresh(HTMLWidget hw)
{
	int r = hw->manager.top_shadow_color;
	int b = hw->manager.bottom_shadow_color;

	/* Only refresh if we have a window already */
	if (!XtIsRealized((Widget)hw))
		return;

	/* Minor trickery to insure that the update happens... */
	hw->html.view->core.background_pixel = hw->core.background_pixel? 0 : 1;
	hw->manager.top_shadow_color = 	r ? 0 : 1;
	hw->manager.bottom_shadow_color = b ? 0 : 1;
	XtVaSetValues(hw->html.view,
		      XmNbackground, hw->core.background_pixel,
		      XmNtopShadowColor, r,
		      XmNbottomShadowColor, b,
		      NULL);
	XtVaSetValues((Widget)hw,
		      XmNbackground, hw->core.background_pixel,
		      XmNtopShadowColor, r,
		      XmNbottomShadowColor, b,
		      NULL);
	XClearArea(hw->html.dsp, XtWindow(hw->html.view), 0, 0, 0, 0, False);
	ViewRedisplay(hw, 0, 0, hw->html.view_width, hw->html.view_height);
	/* This is a fake deal to make an Expose event to call Redisplay
	 * to redraw the shadow around the view area.
	 */
	XClearArea(hw->html.dsp, XtWindow(hw->html.view), 0, 0, 1, 1, True);

	if (hw->html.blinking_elements) {
		BlinkInfo *cdata = (BlinkInfo *) malloc(sizeof(BlinkInfo));

		if (!cdata)
			return;
		cdata->drawing = hw->html.draw_count;
		cdata->hw = hw;
		hw->html.blinkdata = cdata;
		hw->html.blink_timer = XtAppAddTimeOut(app_con,
						 hw->html.blink_time,
						 (XtTimerCallbackProc)TextBlink,
						 (XtPointer)cdata);
#ifndef DISABLE_TRACE
		if (htmlwTrace)
			fprintf(stderr, "Started Blinking\n");
#endif
	}
}

/* The Redisplay function is what you do with an expose event.
 * Right now we call the CompositeWidget's Redisplay routine.
 */
static void Redisplay(HTMLWidget hw, XEvent *event, Region region)
{
	Dimension st = hw->manager.shadow_thickness;

#ifndef DISABLE_TRACE
        if (htmlwTrace)
		fprintf(stderr, "In Redisplay\n");
#endif
	/* Redraw the shadow around the scrolling area which may have been
	 * messed up.
	 */
#ifndef MOTIF1_2
       	_XmDrawShadow(hw->html.dsp, XtWindow(hw),
		      hw->manager.bottom_shadow_GC, hw->manager.top_shadow_GC,
		      st, 0, 0,
		      hw->html.view_width + (2 * st),
		      hw->html.view_height + (2 * st));
	_XmRedisplayGadgets((CompositeWidget)hw, (XExposeEvent *)event, region);
#else
	_XmDrawShadows(hw->html.dsp, XtWindow(hw),
		       hw->manager.top_shadow_GC, hw->manager.bottom_shadow_GC,
		       0, 0,
		       hw->html.view_width + (2 * st),
		       hw->html.view_height + (2 * st),
		       st, XmSHADOW_IN);
	_XmRedisplayGadgets((Widget)hw, (XEvent *)event, region);
#endif
	return;
}

static void Destroy(HTMLWidget hw)
{
#ifndef DISABLE_TRACE
        if (htmlwTrace)
		fprintf(stderr, "In Destroy\n");
#endif
}

/* Resize is called when the widget changes size.
 * Mostly any resize causes a reformat, except for the special case
 * where the width doesn't change, and the height doesn't change
 * enough to affect the vertical scrollbar.
 * It is too complex to guess exactly what needs to be redrawn, so refresh
 * the whole window on any resize.
 */
static void Resize(HTMLWidget hw)
{
#ifndef DISABLE_TRACE
        if (htmlwTrace)
		fprintf(stderr, "In Resize\n");
#endif
	ResetWidgetsOnResize(hw);

	/* Reset cached GC settings */
	hw->html.cur_font = NULL;
	XSetForeground(hw->html.dsp, hw->html.drawGC, hw->html.cur_fg);
	XSetBackground(hw->html.dsp, hw->html.drawGC, hw->html.cur_bg);

	ReformatWindow(hw);
	ScrollWidgets(hw);
	ViewClearAndRefresh(hw);
	/* If an image load took place, the cursor and logo need reset */
	mo_gui_done_with_icon();
}

/*
 * Find the complete text for the anchor that aptr is part of
 * and set it into the selection.
 */
static void FindSelectAnchor(HTMLWidget hw, ElemInfo *aptr)
{
	ElemInfo *eptr = aptr;

	while (eptr->prev && eptr->prev->anchor_tag_ptr->anc_href &&
	       !strcmp(eptr->prev->anchor_tag_ptr->anc_href,
		       eptr->anchor_tag_ptr->anc_href))
		eptr = eptr->prev;
	hw->html.select_start = eptr;
	hw->html.sel_start_pos = 0;

	eptr = aptr;
	while (eptr->next && eptr->next->anchor_tag_ptr->anc_href &&
	       !strcmp(eptr->next->anchor_tag_ptr->anc_href,
		       eptr->anchor_tag_ptr->anc_href))
		eptr = eptr->next;
	hw->html.select_end = eptr;
	hw->html.sel_end_pos = eptr->edata_len - 2;
}

/* Set as active all elements in the widget that are part of the anchor
 * in the widget's start ptr.
 */
static void SetAnchor(HTMLWidget hw)
{
	ElemInfo *eptr = hw->html.active_anchor;
	ElemInfo *start, *end;
	unsigned long fg, bg, old_fg, old_bg;

	if (!eptr || !eptr->anchor_tag_ptr->anc_href)
		return;
	fg = hw->html.activeAnchor_fg;
	bg = hw->html.activeAnchor_bg;

	FindSelectAnchor(hw, eptr);
	eptr = start = hw->html.select_start;
	end = hw->html.select_end;
	hw->html.underline_yoffset = -1;
	while (eptr && (eptr != end)) {
		if (eptr->type == E_TEXT) {
			old_fg = eptr->fg;
			old_bg = eptr->bg;
			eptr->fg = fg;
			eptr->bg = bg;
			TextRefresh(hw, eptr, 0, eptr->edata_len - 2, False);
			eptr->fg = old_fg;
			eptr->bg = old_bg;
		} else if (eptr->type == E_IMAGE) {
			old_fg = eptr->fg;
			old_bg = eptr->bg;
			eptr->fg = fg;
			eptr->bg = bg;
			ImageRefresh(hw, eptr, NULL);
			eptr->fg = old_fg;
			eptr->bg = old_bg;
		} else if (eptr->type == E_CR) {
			hw->html.underline_yoffset = -1;
		}
		eptr = eptr->next;
	}
	if (eptr) {
		if (eptr->type == E_TEXT) {
			old_fg = eptr->fg;
			old_bg = eptr->bg;
			eptr->fg = fg;
			eptr->bg = bg;
			TextRefresh(hw, eptr, 0, eptr->edata_len - 2, False);
			eptr->fg = old_fg;
			eptr->bg = old_bg;
		} else if (eptr->type == E_IMAGE) {
			old_fg = eptr->fg;
			old_bg = eptr->bg;
			eptr->fg = fg;
			eptr->bg = bg;
			ImageRefresh(hw, eptr, NULL);
			eptr->fg = old_fg;
			eptr->bg = old_bg;
		}
	}
}

/*
 * Draw selection for all elements in the widget
 * from start to end.
 */
static void DrawSelection(HTMLWidget hw, ElemInfo *start, ElemInfo *end,
			  int start_pos, int end_pos)
{
	ElemInfo *eptr;
	int epos, p1, p2;

	if (!start || !end)
		return;
	/* Keep positions within bounds (allows us to be sloppy elsewhere) */
	if (start_pos < 0)
		start_pos = 0;
	if (start_pos >= start->edata_len - 1)
		start_pos = start->edata_len - 2;
	if (end_pos < 0)
		end_pos = 0;
	if (end_pos >= end->edata_len - 1)
		end_pos = end->edata_len - 2;
	if (SwapElements(start, end, start_pos, end_pos)) {
		eptr = start;
		start = end;
		end = eptr;
		epos = start_pos;
		start_pos = end_pos;
		end_pos = epos;
	}
	/* Reset underline offset */
	hw->html.underline_yoffset = -1;
	eptr = start;
	while (eptr && (eptr != end)) {
		if (eptr == start) {
			p1 = start_pos;
		} else {
			p1 = 0;
		}
		p2 = eptr->edata_len - 2;
		if (eptr->type == E_TEXT) {
			eptr->selected = True;
			eptr->start_pos = p1;
			eptr->end_pos = p2;
			TextRefresh(hw, eptr, p1, p2, True);
		} else if (eptr->type == E_CR) {
			hw->html.underline_yoffset = -1;
		}
		eptr = eptr->next;
	}
	if (eptr) {
		if (eptr == start) {
			p1 = start_pos;
		} else {
			p1 = 0;
		}
		if (eptr == end) {
			p2 = end_pos;
		} else {
			p2 = eptr->edata_len - 2;
		}
		if (eptr->type == E_TEXT) {
			eptr->selected = True;
			eptr->start_pos = p1;
			eptr->end_pos = p2;
			TextRefresh(hw, eptr, p1, p2, True);
		}
	}
}

/*
 * Set selection for all elements in the widget's
 * start to end list.
 */
static void SetSelection(HTMLWidget hw)
{
	DrawSelection(hw, hw->html.select_start, hw->html.select_end,
		      hw->html.sel_start_pos, hw->html.sel_end_pos);
}

/*
 * Erase the selection from start to end
 */
static void EraseSelection(HTMLWidget hw, ElemInfo *start, ElemInfo *end,
			   int start_pos, int end_pos)
{
	ElemInfo *eptr;
	int epos, p1, p2;

	if (!start || !end)
		return;
	/*
	 * Keep positions within bounds (allows us to be sloppy elsewhere)
	 */
	if (start_pos < 0)
		start_pos = 0;
	if (start_pos >= start->edata_len - 1)
		start_pos = start->edata_len - 2;
	if (end_pos < 0)
		end_pos = 0;
	if (end_pos >= end->edata_len - 1)
		end_pos = end->edata_len - 2;

	if (SwapElements(start, end, start_pos, end_pos)) {
		eptr = start;
		start = end;
		end = eptr;
		epos = start_pos;
		start_pos = end_pos;
		end_pos = epos;
	}
	hw->html.underline_yoffset = -1;
	eptr = start;
	while (eptr && (eptr != end)) {
		if (eptr == start) {
			p1 = start_pos;
		} else {
			p1 = 0;
		}
		p2 = eptr->edata_len - 2;
		if (eptr->type == E_TEXT) {
			eptr->selected = False;
			TextRefresh(hw, eptr, p1, p2, True);
		} else if (eptr->type == E_CR) {
			hw->html.underline_yoffset = -1;
		}
		eptr = eptr->next;
	}
	if (eptr) {
		if (eptr == start) {
			p1 = start_pos;
		} else {
			p1 = 0;
		}
		if (eptr == end) {
			p2 = end_pos;
		} else {
			p2 = eptr->edata_len - 2;
		}
		if (eptr->type == E_TEXT) {
			eptr->selected = False;
			TextRefresh(hw, eptr, p1, p2, True);
		}
	}
}

/* Clear the current selection (if there is one)
 */
static void ClearSelection(HTMLWidget hw)
{
	EraseSelection(hw, hw->html.select_start, hw->html.select_end,
		       hw->html.sel_start_pos, hw->html.sel_end_pos);

	hw->html.select_start = hw->html.select_end = NULL;
	hw->html.sel_start_pos = hw->html.sel_end_pos = 0;
	hw->html.active_anchor = NULL;
}

/*
 * Clear from active all elements in the widget that are part of the anchor.
 * (These have already been previously set into the start and end of the
 * selection.)
 */
static void UnsetAnchor(HTMLWidget hw)
{
	ElemInfo *eptr = hw->html.select_start;

	/* Clear any activated images */
	while (eptr && (eptr != hw->html.select_end)) {
		if (eptr->type == E_IMAGE)
			ImageRefresh(hw, eptr, NULL);
		eptr = eptr->next;
	}
	if (eptr && (eptr->type == E_IMAGE))
		ImageRefresh(hw, eptr, NULL);
	ClearSelection(hw); 		/* Clear the activated anchor */
}

/*
 * Erase the old selection and draw the new one in such a way that
 * advantage is taken of overlap, and there is no obnoxious flashing.
 */
static void ChangeSelection(HTMLWidget hw, ElemInfo *start,
			    ElemInfo *end, int start_pos, int end_pos)
{
	ElemInfo *old_start = hw->html.new_start;
	ElemInfo *old_end = hw->html.new_end;
	ElemInfo *new_start = start;
	ElemInfo *new_end = end;
	ElemInfo *eptr;
	int epos;
	int new_start_pos = start_pos;
	int new_end_pos = end_pos;
	int old_start_pos = hw->html.new_start_pos;
	int old_end_pos = hw->html.new_end_pos;

	if (!new_start || !new_end)
		return;

	if (!old_start || !old_end) {
		DrawSelection(hw, new_start, new_end, new_start_pos,
			      new_end_pos);
		return;
	}
	if (SwapElements(old_start, old_end, old_start_pos, old_end_pos)) {
		eptr = old_start;
		old_start = old_end;
		old_end = eptr;
		epos = old_start_pos;
		old_start_pos = old_end_pos;
		old_end_pos = epos;
	}
	if (SwapElements(new_start, new_end, new_start_pos, new_end_pos)) {
		eptr = new_start;
		new_start = new_end;
		new_end = eptr;
		epos = new_start_pos;
		new_start_pos = new_end_pos;
		new_end_pos = epos;
	}
	/*
	 * Deal with all possible intersections of the 2 selection sets.
	 *
	 ********************************************************
	 *			*				*
	 *      |--		*	     |--		*
	 * old--|		*	new--|			*
	 *      |--		*	     |--		*
	 *			*				*
	 *      |--		*	     |--		*
	 * new--|		*	old--|			*
	 *      |--		*	     |--		*
	 *			*				*
	 ********************************************************
	 *			*				*
	 *      |----		*	       |--		*
	 * old--|		*	  new--|		*
	 *      | |--		*	       |		*
	 *      |-+--		*	     |-+--		*
	 *        |		*	     | |--		*
	 *   new--|		*	old--|			*
	 *        |--		*	     |----		*
	 *			*				*
	 ********************************************************
	 *			*				*
	 *      |---------	*	     |---------		*
	 *      |		*	     |			*
	 *      |      |--	*	     |      |--		*
	 * new--| old--|	*	old--| new--|		*
	 *      |      |--	*	     |      |--		*
	 *      |		*	     |			*
	 *      |---------	*	     |---------		*
	 *			*				*
	 ********************************************************
	 */
	if ((ElementLessThan(old_end, new_start, old_end_pos, new_start_pos)) ||
	    (ElementLessThan(new_end, old_start, new_end_pos, old_start_pos))) {
		EraseSelection(hw, old_start, old_end, old_start_pos,
			       old_end_pos);
		DrawSelection(hw, new_start, new_end, new_start_pos,
			      new_end_pos);
	} else if ((ElementLessThan(old_start, new_start,
			            old_start_pos, new_start_pos)) &&
		   (ElementLessThan(old_end, new_end,
				    old_end_pos, new_end_pos))) {
		if (new_start_pos != 0) {
			EraseSelection(hw, old_start, new_start,
				       old_start_pos, new_start_pos - 1);
		} else {
			EraseSelection(hw, old_start, new_start->prev,
				       old_start_pos,
				       new_start->prev->edata_len - 2);
		}
		if (old_end_pos < (old_end->edata_len - 2)) {
			DrawSelection(hw, old_end, new_end,
				      old_end_pos + 1, new_end_pos);
		} else {
			DrawSelection(hw, old_end->next, new_end, 0,
				      new_end_pos);
		}
	} else if ((ElementLessThan(new_start, old_start,
				    new_start_pos, old_start_pos)) &&
		   (ElementLessThan(new_end, old_end,
				    new_end_pos, old_end_pos))) {
		if (old_start_pos != 0) {
			DrawSelection(hw, new_start, old_start,
				      new_start_pos, old_start_pos - 1);
		} else {
			DrawSelection(hw, new_start, old_start->prev,
				      new_start_pos,
				      old_start->prev->edata_len - 2);
		}
		if (new_end_pos < (new_end->edata_len - 2)) {
			EraseSelection(hw, new_end, old_end,
				       new_end_pos + 1, old_end_pos);
		} else {
			EraseSelection(hw, new_end->next, old_end,
				       0, old_end_pos);
		}
	} else if ((ElementLessThan(new_start, old_start,
				    new_start_pos, old_start_pos)) ||
		   (ElementLessThan(old_end, new_end,
				    old_end_pos, new_end_pos))) {
		if ((new_start != old_start) ||
		    (new_start_pos != old_start_pos)) {
			if (old_start_pos != 0) {
				DrawSelection(hw, new_start, old_start,
					      new_start_pos, old_start_pos - 1);
			} else {
				DrawSelection(hw, new_start, old_start->prev,
					      new_start_pos,
					      old_start->prev->edata_len - 2);
			}
		}
		if ((old_end != new_end) || (old_end_pos != new_end_pos)) {
			if (old_end_pos < (old_end->edata_len - 2)) {
				DrawSelection(hw, old_end, new_end,
					      old_end_pos + 1, new_end_pos);
			} else {
				DrawSelection(hw, old_end->next, new_end,
					      0, new_end_pos);
			}
		}
	} else {
		if ((old_start != new_start) ||
		    (old_start_pos != new_start_pos)) {
			if (new_start_pos != 0) {
				EraseSelection(hw, old_start, new_start,
					      old_start_pos, new_start_pos - 1);
			} else {
				EraseSelection(hw, old_start, new_start->prev,
					       old_start_pos,
					       new_start->prev->edata_len - 2);
			}
		}
		if ((new_end != old_end) || (new_end_pos != old_end_pos)) {
			if (new_end_pos < (new_end->edata_len - 2)) {
				EraseSelection(hw, new_end, old_end,
					       new_end_pos + 1, old_end_pos);
			} else {
				EraseSelection(hw, new_end->next, old_end,
					       0, old_end_pos);
			}
		}
	}
}


static void SelectStart(Widget w, XEvent *event,
			String *params,         /* unused */
			Cardinal *num_params)   /* unused */
{
	HTMLWidget hw = (HTMLWidget)XtParent(w);
	XButtonPressedEvent *BuEvent = (XButtonPressedEvent *)event;
	ElemInfo *eptr;
	int epos;

	if (XtClass(XtParent(w)) != htmlWidgetClass)
		return;

        XUndefineCursor(hw->html.dsp, XtWindow(hw->html.view));
	hw->html.cursor_in_anchor = False;
	/*
	 * Because X sucks, we can get the button pressed in the window, but
	 * released out of the window.  This will highlight some text, but
	 * never complete the selection.  Now on the next button press we
	 * have to clean up this mess.
	 */
	/* Reset cached GC settings */
	hw->html.cur_font = NULL;
	XSetForeground(hw->html.dsp, hw->html.drawGC, hw->html.cur_fg);
	XSetBackground(hw->html.dsp, hw->html.drawGC, hw->html.cur_bg);

	EraseSelection(hw, hw->html.new_start, hw->html.new_end,
		       hw->html.new_start_pos, hw->html.new_end_pos);
	/*
	 * We want to erase the currently selected text, but still save the
	 * selection internally in case we don't create a new one.
	 */
	EraseSelection(hw, hw->html.select_start, hw->html.select_end,
		       hw->html.sel_start_pos, hw->html.sel_end_pos);
	hw->html.new_start = hw->html.select_start;
	hw->html.new_end = NULL;
	hw->html.new_start_pos = hw->html.sel_start_pos;
	hw->html.new_end_pos = 0;

	if (eptr = LocateElement(hw, BuEvent->x, BuEvent->y, &epos)) {
		/*
		 * If this is an anchor assume for now we are activating it
		 * and not selecting it.
		 */
		if (eptr->anchor_tag_ptr->anc_href) {
			hw->html.active_anchor = eptr;
			hw->html.press_x = BuEvent->x;
			hw->html.press_y = BuEvent->y;
			SetAnchor(hw);
		}
		/*
		 * Else if we are on an image we can't select text so
		 * pretend we got eptr == NULL, and exit after checking
		 * for USEMAPs, delayed images and form ISMAPs.
		 */
		else if (eptr->type == E_IMAGE) {
			hw->html.new_start = hw->html.new_end = NULL;
			hw->html.new_start_pos = hw->html.new_end_pos = 0;
			hw->html.press_x = BuEvent->x;
			hw->html.press_y = BuEvent->y;
			hw->html.but_press_time = BuEvent->time;
			if (eptr->pic_data &&
			   (eptr->pic_data->delayed || eptr->is_in_form ||
			    (eptr->pic_data->usemap && eptr->pic_data->area)))
				hw->html.active_anchor = eptr;
			return;
		}
		/* Else if used button2, we can't select text, so exit here. */
		else if (BuEvent->button == Button2) {
			hw->html.press_x = BuEvent->x;
			hw->html.press_y = BuEvent->y;
			hw->html.but_press_time = BuEvent->time;
			return;
		}
		/* Else a single click will not select a new object, but
		 * it will prime that selection on the next mouse move.
		 */
		else {
			hw->html.new_start = eptr;
			hw->html.new_start_pos = epos;
			hw->html.new_end = NULL;
			hw->html.new_end_pos = 0;
			hw->html.press_x = BuEvent->x;
			hw->html.press_y = BuEvent->y;
		}
	} else {
		hw->html.new_start = hw->html.new_end = NULL;
		hw->html.new_start_pos = hw->html.new_end_pos = 0;
		hw->html.press_x = BuEvent->x;
		hw->html.press_y = BuEvent->y;
	}
	hw->html.but_press_time = BuEvent->time;
}

static void ExtendStart(Widget w, XEvent *event,
			String *params,         /* Unused */
			Cardinal *num_params)   /* Unused */
{
	HTMLWidget hw = (HTMLWidget)XtParent(w);
	XButtonPressedEvent *BuEvent = (XButtonPressedEvent *)event;
	ElemInfo *eptr, *start, *end, *old_start, *old_end;
	int old_start_pos, old_end_pos, start_pos, end_pos, epos;

	if (XtClass(XtParent(w)) != htmlWidgetClass)
		return;

	eptr = LocateElement(hw, BuEvent->x, BuEvent->y, &epos);

	if (eptr && (eptr->type == E_IMAGE))  /* Ignore IMAGE elements. */
		eptr = NULL;
	/*
	 * Ignore NULL elements.
	 */
	if (eptr) {
		old_start = hw->html.new_start;
		old_start_pos = hw->html.new_start_pos;
		old_end = hw->html.new_end;
		old_end_pos = hw->html.new_end_pos;
		if (!hw->html.new_start) {
			hw->html.new_start = hw->html.select_start;
			hw->html.new_start_pos = hw->html.sel_start_pos;
			hw->html.new_end = hw->html.select_end;
			hw->html.new_end_pos = hw->html.sel_end_pos;
		} else {
			hw->html.new_end = eptr;
			hw->html.new_end_pos = epos;
		}
		if (SwapElements(hw->html.new_start, hw->html.new_end,
		    	        hw->html.new_start_pos, hw->html.new_end_pos)) {
			if (SwapElements(eptr, hw->html.new_end,
			    		 epos, hw->html.new_end_pos)) {
				start = hw->html.new_end;
				start_pos = hw->html.new_end_pos;
				end = eptr;
				end_pos = epos;
			} else {
				start = hw->html.new_start;
				start_pos = hw->html.new_start_pos;
				end = eptr;
				end_pos = epos;
			}
		} else {
			if (SwapElements(eptr, hw->html.new_start,
					 epos, hw->html.new_start_pos)) {
				start = hw->html.new_start;
				start_pos = hw->html.new_start_pos;
				end = eptr;
				end_pos = epos;
			} else {
				start = hw->html.new_end;
				start_pos = hw->html.new_end_pos;
				end = eptr;
				end_pos = epos;
			}
		}
		if (!start) {
			start = eptr;
			start_pos = epos;
		}
		if (!old_start) {
			hw->html.new_start = hw->html.select_start;
			hw->html.new_end = hw->html.select_end;
			hw->html.new_start_pos = hw->html.sel_start_pos;
			hw->html.new_end_pos = hw->html.sel_end_pos;
		} else {
			hw->html.new_start = old_start;
			hw->html.new_end = old_end;
			hw->html.new_start_pos = old_start_pos;
			hw->html.new_end_pos = old_end_pos;
		}
		ChangeSelection(hw, start, end, start_pos, end_pos);
		hw->html.new_start = start;
		hw->html.new_end = end;
		hw->html.new_start_pos = start_pos;
		hw->html.new_end_pos = end_pos;
	} else if (!hw->html.new_start) {
		hw->html.new_start = hw->html.select_start;
		hw->html.new_start_pos = hw->html.sel_start_pos;
		hw->html.new_end = hw->html.select_end;
		hw->html.new_end_pos = hw->html.sel_end_pos;
	}
	hw->html.press_x = BuEvent->x;
	hw->html.press_y = BuEvent->y;
}

static void ExtendAdjust(Widget w, XEvent *event,
			 String *params,         /* Unused */
			 Cardinal *num_params)   /* Unused */
{
	HTMLWidget hw = (HTMLWidget)XtParent(w);
	XPointerMovedEvent *MoEvent = (XPointerMovedEvent *)event;
	ElemInfo *eptr, *start, *end;
	int start_pos, end_pos, epos;

	if (XtClass(XtParent(w)) != htmlWidgetClass)
		return;
	/*
	 * Very small mouse motion immediately after button press is ignored.
	 */
	if ((ABS((hw->html.press_x - MoEvent->x)) <= SELECT_THRESHOLD) &&
	    (ABS((hw->html.press_y - MoEvent->y)) <= SELECT_THRESHOLD))
		return;
	/*
	 * Blinking text or animated image in another Mosaic window may
	 * mess with the GC.
	 */
	if (Select_GC_Reset) {
		/* Reset cached GC settings */
		hw->html.cur_font = NULL;
		XSetForeground(hw->html.dsp, hw->html.drawGC, hw->html.cur_fg);
		XSetBackground(hw->html.dsp, hw->html.drawGC, hw->html.cur_bg);
		Select_GC_Reset = 0;
	}
	/*
	 * If we have an active anchor and we got here, we have moved the
	 * mouse too far.  Deactivate anchor, and prime a selection.
	 * If the anchor is internal text, don't prime a selection.
	 */
	if (hw->html.active_anchor) {
		UnsetAnchor(hw);
		hw->html.new_start = hw->html.new_end = NULL;
		hw->html.new_start_pos = hw->html.new_end_pos = 0;
	}
	/*
	 * If we used button2, we can't select text, so
	 * clear selection and exit here.
	 */
	if (MoEvent->state & Button2Mask) {
		hw->html.select_start = hw->html.select_end = NULL;
		hw->html.sel_start_pos = hw->html.sel_end_pos = 0;
		hw->html.new_start = hw->html.new_end = NULL;
		hw->html.new_start_pos = hw->html.new_end_pos = 0;
		return;
	}
	eptr = LocateElement(hw, MoEvent->x, MoEvent->y, &epos);

	/*
	 * If we are on an image pretend we are nowhere
	 * and just return.
	 */
	if (eptr && (eptr->type == E_IMAGE))
		return;
	/*
	 * Ignore NULL items.
	 * Ignore if the same as last selected item and position.
	 * Ignore special internal text.
	 */
	if (eptr &&
	    ((eptr != hw->html.new_end) || (epos != hw->html.new_end_pos))) {
		start = hw->html.new_start;
		start_pos = hw->html.new_start_pos;
		end = eptr;
		end_pos = epos;
		if (!start) {
			start = eptr;
			start_pos = epos;
		}
		ChangeSelection(hw, start, end, start_pos, end_pos);
		hw->html.new_start = start;
		hw->html.new_end = end;
		hw->html.new_start_pos = start_pos;
		hw->html.new_end_pos = end_pos;
	}
}

static void ExtendEnd(Widget w, XEvent *event, String *params,
		      Cardinal *num_params)
{
	HTMLWidget hw = (HTMLWidget)XtParent(w);
	XButtonReleasedEvent *BuEvent = (XButtonReleasedEvent *)event;
	ElemInfo *eptr, *start, *end;
	Atom *atoms;
	int i, buffer, start_pos, end_pos, epos;
	char *text;

	if (XtClass(XtParent(w)) != htmlWidgetClass)
		return;
	eptr = LocateElement(hw, BuEvent->x, BuEvent->y, &epos);

	/*
	 * Blinking text or animated image in another Mosaic window may
	 * mess with the GC.
	 */
	if (Select_GC_Reset) {
		/* Reset cached GC settings */
		hw->html.cur_font = NULL;
		XSetForeground(hw->html.dsp, hw->html.drawGC, hw->html.cur_fg);
		XSetBackground(hw->html.dsp, hw->html.drawGC, hw->html.cur_bg);
		Select_GC_Reset = 0;
	}
	/*
	 * If we just released button one or two, and we are on an object,
	 * and we have an active anchor, and we are on the active anchor,
	 * and if we haven't waited too long.  Activate that anchor.
	 */
	if (((BuEvent->button == Button1) || (BuEvent->button == Button2)) &&
	    eptr && hw->html.active_anchor &&
	    (eptr == hw->html.active_anchor) &&
	    ((BuEvent->time - hw->html.but_press_time) < CLICK_TIME)) {
		_HTMLInput(w, event, eptr, False);
		return;
	}
	if (hw->html.active_anchor) {
		UnsetAnchor(hw);
		hw->html.new_start = eptr;
		hw->html.new_start_pos = epos;
		hw->html.new_end = NULL;
		hw->html.new_end_pos = 0;
	}
	/*
	 * If we clicked on form label
	 */
	if (eptr && eptr->label_id &&
	    ((BuEvent->button == Button1) || (BuEvent->button == Button2)) &&
	    ((BuEvent->time - hw->html.but_press_time) < CLICK_TIME)) {
		_HTMLInput(w, event, eptr, True);
		return;
	}
	/*
	 * If we used button2, we can't select text, so clear
	 * selection and exit here.
	 */
	if (BuEvent->button == Button2) {
		hw->html.new_start = hw->html.select_start;
		hw->html.new_end = NULL;
		hw->html.new_start_pos = hw->html.sel_start_pos;
		hw->html.new_end_pos = 0;
		return;
	}
	/*
	 * If we are on an image, pretend we are nowhere
	 * and NULL out the eptr
	 */
	if (eptr && (eptr->type == E_IMAGE))
		eptr = NULL;
	/*
	 * If button released on a NULL item, take the last non-NULL
	 * item that we highlighted.
	 */
	if (!eptr && hw->html.new_end) {
		eptr = hw->html.new_end;
		epos = hw->html.new_end_pos;
	}
	if (eptr && hw->html.new_end) {
		start = hw->html.new_start;
		start_pos = hw->html.new_start_pos;
		end = eptr;
		end_pos = epos;
		if (!start) {
			start = eptr;
			start_pos = epos;
		}
		ChangeSelection(hw, start, end, start_pos, end_pos);
		hw->html.select_start = start;
		hw->html.sel_start_pos = start_pos;
		hw->html.select_end = end;
		hw->html.sel_end_pos = end_pos;
		SetSelection(hw);
		hw->html.new_start = hw->html.new_end = NULL;
		hw->html.new_start_pos = hw->html.new_end_pos = 0;

		if (!(atoms = (Atom *)malloc(*num_params * sizeof(Atom)))) {
			fprintf(stderr, "Cannot allocate atom list\n");
			return;
		}
		XmuInternStrings(hw->html.dsp, params, *num_params, atoms);
		hw->html.selection_time = BuEvent->time;
		for (i = 0; i < *num_params; i++) {
			switch (atoms[i]) {
				case XA_CUT_BUFFER0: buffer = 0; break;
				case XA_CUT_BUFFER1: buffer = 1; break;
				case XA_CUT_BUFFER2: buffer = 2; break;
				case XA_CUT_BUFFER3: buffer = 3; break;
				case XA_CUT_BUFFER4: buffer = 4; break;
				case XA_CUT_BUFFER5: buffer = 5; break;
				case XA_CUT_BUFFER6: buffer = 6; break;
				case XA_CUT_BUFFER7: buffer = 7; break;
				default: buffer = -1; break;
			}
			if (buffer >= 0) {
				text = ParseTextToString(
						hw->html.formatted_elements,
						hw->html.select_start,
						hw->html.select_end,
						hw->html.sel_start_pos,
						hw->html.sel_end_pos,
						hw->html.font->max_bounds.width,
						hw->html.margin_width);
				if (text) {
					XStoreBuffer(hw->html.dsp, text,
						     strlen(text), buffer);
					free(text);
				}
			} else {
				XtOwnSelection((Widget)hw, atoms[i],
				       BuEvent->time,
				       (XtConvertSelectionProc)ConvertSelection,
				       (XtLoseSelectionProc)LoseSelection,
				       (XtSelectionDoneProc)SelectionDone);
			}
		}
		free((char *)atoms);
	} else if (!eptr) {
		hw->html.select_start = hw->html.select_end = NULL;
		hw->html.sel_start_pos = hw->html.sel_end_pos = 0;
		hw->html.new_start = hw->html.new_end = NULL;
		hw->html.new_start_pos = hw->html.new_end_pos = 0;
	}
}

#define LEAVING_ANCHOR(hw) \
  hw->html.cached_tracked_ele = NULL; \
  hw->html.title_elem = NULL; \
  XtCallCallbackList((Widget)hw, hw->html.pointer_motion_callback, ""); \
  XUndefineCursor(dsp, XtWindow(hw->html.view)); \
  hw->html.cursor_in_anchor = False;

/* KNOWN PROBLEM: We sometimes never get LeaveNotify or FocusOut events,
 * despite the fact we've requested them.  Bummer. */
static void TrackMotion(Widget w, XEvent *event,
			String *params,         /* unused */
			Cardinal *num_params)   /* unused */
{
	HTMLWidget hw = (HTMLWidget)XtParent(w);
	ElemInfo *eptr;
	AreaInfo *area;
	int epos, x, y;
	Display *dsp = hw->html.dsp;

	if (XtClass(hw) != htmlWidgetClass)
		return;

	if (event->type == MotionNotify) {
		x = ((XMotionEvent *)event)->x;
		y = ((XMotionEvent *)event)->y;
	} else {
		if (hw->html.cached_tracked_ele) {
			if (event->type == LeaveNotify) {
				x = ((XCrossingEvent *)event)->x;
				y = ((XCrossingEvent *)event)->y;
				eptr = LocateElement(hw, x, y, &epos);
				if (eptr == hw->html.cached_tracked_ele)
					return;
			} else if ((event->type != Expose) &&
				   (event->type != FocusOut)) {
				return;
			}
			LEAVING_ANCHOR(hw);
		} else {
			/* Widgets get LeaveNotify instead of MotionNotify */
			if (event->type == LeaveNotify) {
				x = ((XCrossingEvent *)event)->x;
				y = ((XCrossingEvent *)event)->y;
				eptr = LocateElement(hw, x, y, &epos);
				if (eptr && (eptr->type == E_WIDGET) &&
				    eptr->title) {
					hw->html.title_elem = eptr;
					hw->html.cached_tracked_ele = eptr;
					XtCallCallbackList((Widget)hw,
					  hw->html.pointer_motion_callback, "");
				}
			}
		}
		return;
	}
	if (!(eptr = LocateElement(hw, x, y, &epos))) {
		LEAVING_ANCHOR(hw);
		return;
	}
	if (eptr->title) {
		hw->html.title_elem = eptr;
	} else {
		hw->html.title_elem = NULL;
	}

	/* We're hitting a new anchor if eptr exists and
	 * eptr != cached tracked element and anchor_tag_ptr != NULL.
	 */
	if ((eptr != hw->html.cached_tracked_ele) &&
	    (eptr->anchor_tag_ptr->anc_href ||
	     (eptr->pic_data &&
	      (eptr->pic_data->usemap || eptr->pic_data->delayed ||
	       eptr->is_in_form)))) {
		hw->html.cached_tracked_ele = eptr;
		if (eptr->pic_data && eptr->pic_data->usemap) {
			MapInfo *map = eptr->pic_data->map;

			if (!map) {
				map = FindMap(hw, eptr->pic_data->usemap);
				eptr->pic_data->map = map;
			}
			if (map && (area = GetMapArea(map,
					    x + hw->html.scroll_x - eptr->x,
					    y + hw->html.scroll_y - eptr->y))) {
				eptr->pic_data->area = area;
				if (area->alt) {
				     XtCallCallbackList((Widget)hw,
					       hw->html.pointer_motion_callback,
					       area->alt);
				} else {
				     XtCallCallbackList((Widget)hw,
					       hw->html.pointer_motion_callback,
					       area->href);
				}
			} else {
			        eptr->pic_data->area = NULL;
			        XtCallCallbackList((Widget)hw,
				          hw->html.pointer_motion_callback, "");
			}
		} else if (eptr->anchor_tag_ptr->anc_href) {
		        XtCallCallbackList((Widget)hw,
					   hw->html.pointer_motion_callback,
					   eptr->anchor_tag_ptr->anc_href);
		} else {
		        XtCallCallbackList((Widget)hw,
					  hw->html.pointer_motion_callback, "");
		}
		XDefineCursor(dsp, XtWindow(hw->html.view), in_anchor_cursor);
		hw->html.cursor_in_anchor = True;
	} else {
		if (hw->html.cached_tracked_ele &&
		    (!eptr->anchor_tag_ptr->anc_href &&
		     (!eptr->pic_data ||
		      (!eptr->pic_data->usemap &&
		       !eptr->pic_data->delayed && !eptr->is_in_form)))) {
			/* If we're leaving an anchor (a cached ele exists) and
			 * the new element doesn't have an anchor. */
			if (hw->html.title_elem &&
			    (eptr != hw->html.cached_tracked_ele)) {
				hw->html.cached_tracked_ele = eptr;
			        XtCallCallbackList((Widget)hw,
					  hw->html.pointer_motion_callback, "");
			} else if (!hw->html.title_elem) {
				LEAVING_ANCHOR(hw);
			}
		} else if (hw->html.cached_tracked_ele && eptr->pic_data &&
			   eptr->pic_data->usemap && eptr->pic_data->map) {
			if (!hw->html.cursor_in_anchor) {
				XDefineCursor(dsp, XtWindow(hw->html.view),
					      in_anchor_cursor);
				hw->html.cursor_in_anchor = True;
			}
			area = GetMapArea(eptr->pic_data->map,
			    		  x + hw->html.scroll_x - eptr->x,
					  y + hw->html.scroll_y - eptr->y);
			if (!area && !eptr->pic_data->area) {
				/* No area now or before */
				if (hw->html.title_elem) {
				    XtCallCallbackList((Widget)hw,
					  hw->html.pointer_motion_callback, "");
				} else {
				    return;
				}
			} else if (!area && eptr->pic_data->area) {
				/* Left an area, but no new one */
				eptr->pic_data->area = NULL;
				XtCallCallbackList((Widget)hw,
					  hw->html.pointer_motion_callback, "");
			} else if ((area && !eptr->pic_data->area) ||
				   (area && eptr->pic_data->area &&
				    (area != eptr->pic_data->area))) {
				/* Entered a new or different area */
				eptr->pic_data->area = area;
				if (area->alt) {
				     XtCallCallbackList((Widget)hw,
					       hw->html.pointer_motion_callback,
					       area->alt);
				} else {
				     XtCallCallbackList((Widget)hw,
					       hw->html.pointer_motion_callback,
					       area->href);
				}
			}
		} else if (hw->html.title_elem &&
			   !hw->html.cached_tracked_ele) {
			hw->html.cached_tracked_ele = eptr;
			XtCallCallbackList((Widget)hw,
					  hw->html.pointer_motion_callback, "");
		}
	}
	return;
}


/* We're adding a subject attribute to the anchor tag.
 * Of course this subject attribute is dependent on the HREF attribute
 * being set to a mailto URL.  I think this is a kludge.  libwww is not set
 * up for this, so to minimize modifications, this routine exists for 
 * libwww:HTSendMailTo() to call to get the subject for the mailto URL.
 * The static globals mailToKludgeSubject, etc. are set in HTMLInput when
 * an anchor is clicked.  
 */
void GetMailtoKludgeInfo(char **url, char **subject)
{
	*url = mailToKludgeURL;
	*subject = mailToKludgeSubject;
}

/* Process mouse input to the HTML widget
 */
static void _HTMLInput(Widget w, XEvent *event, ElemInfo *eptr,
		       Boolean is_label)
{
	HTMLWidget hw = (HTMLWidget)XtParent(w);
	WbAnchorCallbackData cbdata;
	int epos;
	Boolean on_gadget;
	char *ptr;
	char *buf = NULL;
	char *tarbuf = NULL;
	char *tptr = NULL;

	if (XtClass(XtParent(w)) != htmlWidgetClass)
		return;
	/* If motif is defined, we don't want to process this button press
	 * if it is on a gadget */
#ifdef MOTIF1_2
	on_gadget = (_XmInputForGadget((Widget)hw,
#else
	on_gadget = (_XmInputForGadget((CompositeWidget)hw,
#endif
				   event->xbutton.x, event->xbutton.y) != NULL);

	if (on_gadget || (event->type != ButtonRelease) || !eptr)
		return; 

	if (eptr->anchor_tag_ptr->anc_href) {
		/* Save the anchor text, replace newlines with spaces. */
		ptr = tptr = ParseTextToString(hw->html.select_start,
				   hw->html.select_start, hw->html.select_end,
				   hw->html.sel_start_pos, hw->html.sel_end_pos,
				   hw->html.font->max_bounds.width,
				   hw->html.margin_width);
		if (ptr) {
			while (*ptr) {
				if (*ptr == '\n')
					*ptr = ' ';
				ptr++;
			}
		}
	}
	UnsetAnchor(hw);	/* Clear the activated anchor */
#ifdef EXTRA_FLUSH
	XFlush(hw->html.dsp);
#endif

        mailToKludgeSubject = eptr->anchor_tag_ptr->anc_title;
        mailToKludgeURL = eptr->anchor_tag_ptr->anc_href;

	if (eptr->pic_data && eptr->pic_data->delayed &&
	    !eptr->anchor_tag_ptr->anc_href) {
		force_image_load = 1;	/* img.c will reset */
		loading_inlined_images = 1;
		/* Get it in cache, then redo window */
		mo_gui_clear_icon();
		XtCallCallbackList((Widget)hw, hw->html.image_callback,
				   (XtPointer)eptr->pic_data);
		/* If fails, fetched will be 0, so eptr stuff is freed okay */
		loading_inlined_images = 0;
		ReformatWindow(hw);
		ScrollWidgets(hw);
		ViewClearAndRefresh(hw);
		mo_gui_done_with_icon();
		return;
	}
	if (eptr->pic_data && eptr->pic_data->delayed &&
	    eptr->anchor_tag_ptr->anc_href &&
	    ((event->xbutton.y + hw->html.scroll_y - eptr->y) > 10)) {
		force_image_load = 1;	/* img.c will reset */
		loading_inlined_images = 1;
		/* Get it in cache, then redo window */
		mo_gui_clear_icon();
		XtCallCallbackList((Widget)hw, hw->html.image_callback,
				   (XtPointer)eptr->pic_data);
		loading_inlined_images = 0;
		ReformatWindow(hw);
		ScrollWidgets(hw);
		ViewClearAndRefresh(hw);
		mo_gui_done_with_icon();
		return;
	} 
	/* A special ISMAP image in Form */
	if (eptr->pic_data && eptr->pic_data->ismap && eptr->is_in_form) {
		int form_x = event->xbutton.x + hw->html.scroll_x - eptr->x;
		int form_y = event->xbutton.y + hw->html.scroll_y - eptr->y;

		if (eptr->pic_data->ismap == 2) {
			/* Reset form */
			CBResetForm(w, (XtPointer) eptr->pic_data->fptr,
				    (XtPointer) NULL);
		} else {
			/* Submit form */
			ImageSubmitForm(eptr->pic_data->fptr, event,
					eptr->pic_data->text, form_x, form_y);
		}
		return;
	} 
	/* Label in a form */
	if (is_label) {
		ElemInfo *nptr = HTMLAnchorToEptr(hw, eptr->label_id);

#ifndef DISABLE_TRACE
		if (htmlwTrace)
			fprintf(stderr,	"Form label pressed: %s\n",
				eptr->label_id);
#endif
		/* Find form element */
		/* Should be next unless LF and CR elements were inserted */
		while (nptr && (nptr->type != E_WIDGET) &&
		       (nptr->type != E_IMAGE))
			nptr = nptr->next;

		if (nptr && nptr->widget_data && nptr->widget_data->w) {
			Widget w = nptr->widget_data->w;

			switch (nptr->widget_data->type) {
			    case W_CHECKBOX:
				if (XmToggleButtonGetState(w)) {
				    XmToggleButtonSetState(w, False, False);
				} else {
				    XmToggleButtonSetState(w, True, False);
				}
#ifndef DISABLE_TRACE
				if (htmlwTrace)
				    fprintf(stderr, "Toggled checkbox\n");
#endif
				break;
			    case W_RADIOBOX:
				/* Toggle if off and active changed callback */
				if (!XmToggleButtonGetState(w)) {
				    XmToggleButtonSetState(w, True, True);
#ifndef DISABLE_TRACE
				    if (htmlwTrace)
					fprintf(stderr, "Toggled radiobox\n");
#endif
				}
				break;
			    case W_PUSHBUTTON:
				/* Activate callback */
#ifdef VMS
				DXmActivateWidget(w);
#else
				XtCallCallbacks(w, XmNactivateCallback, NULL);
#endif
#ifndef DISABLE_TRACE
				if (htmlwTrace)
				    fprintf(stderr, "Pressed pushbutton\n");
#endif
				break;
			    case W_TEXTFIELD:
			    case W_TEXTAREA:
			    case W_PASSWORD: {
				Widget top = (Widget) hw;

				while (!XtIsTopLevelShell(top))
				    top = XtParent(top);
				/* Focus on it */
				XtSetKeyboardFocus(top, w);

				/* Tell traversal_current not to change focus */
				skip_traversal_current = 1;
#ifndef DISABLE_TRACE
				if (htmlwTrace)
				    fprintf(stderr, "Focused text\n");
#endif
				break;
			    }
			}
		} else if (nptr && (nptr->type == E_IMAGE) && nptr->pic_data &&
			   nptr->is_in_form) {
			int form_x, form_y;

			form_x = event->xbutton.x + hw->html.scroll_x - nptr->x;
			form_y = event->xbutton.y + hw->html.scroll_y - nptr->y;
			ImageSubmitForm(nptr->pic_data->fptr, event,
					nptr->pic_data->text, form_x, form_y);
#ifndef DISABLE_TRACE
			if (htmlwTrace)
			    fprintf(stderr, "Pressed submit image\n");
#endif
		}
		return;
	}
	/* Send the selection location along with the HRef
	 * for images.  Allows you to point at a location on a map
	 * and have the server send you the related document.
	 */
	if ((eptr->type == E_IMAGE) && eptr->pic_data->image_data) {
	   	if (eptr->pic_data->usemap && eptr->pic_data->map &&
		    eptr->pic_data->area) {
			if (eptr->pic_data->area->href)
				buf = strdup(eptr->pic_data->area->href);
			if (eptr->pic_data->area->target)
				tarbuf = strdup(eptr->pic_data->area->target);
		} else if (eptr->pic_data->ismap) {
 			buf = (char *) malloc(
				  strlen(eptr->anchor_tag_ptr->anc_href) + 256);
			sprintf(buf, "%s?%d,%d", eptr->anchor_tag_ptr->anc_href,
				event->xbutton.x + hw->html.scroll_x - eptr->x,
				event->xbutton.y + hw->html.scroll_y - eptr->y);
	   	} else {
			buf = strdup(eptr->anchor_tag_ptr->anc_href);
		}
	} else {
		buf = strdup(eptr->anchor_tag_ptr->anc_href);
	}
	/* Should call a media dependent function that decides how to munge
	 * the HRef.  For example mpeg data will want to know on what frame
	 * the event occured.
	 *
	 * cddata.href = *(eptr->eventf)(eptr, event);
	 */
	cbdata.event = event;
	cbdata.href = buf;
	cbdata.text = tptr;
	cbdata.title = eptr->anchor_tag_ptr->anc_title;
	if (!tarbuf) {
		cbdata.frame = eptr->anchor_tag_ptr->anc_target;
	} else {
		cbdata.frame = tarbuf;
	}
	cbdata.refresh = False;
	XtCallCallbackList((Widget)hw, hw->html.anchor_callback,
			   (XtPointer)&cbdata);
       	if (buf) 
		free(buf);
       	if (tarbuf) 
		free(tarbuf);
       	if (tptr)
		free(tptr);
	mailToKludgeSubject = mailToKludgeURL = NULL;
}

/* SetValues is called when XtSetValues is used to change resources in this
 * widget.
 */
static Boolean SetValues(HTMLWidget current, HTMLWidget request, HTMLWidget nw)
{
	/* Do nothing if in the change font routines */
	if (current->html.ignore_setvalues || current->html.changing_font)
		return (False);

	/* Make sure the underline numbers are within bounds. */
	if (request->html.num_anchor_underlines < 0)
		nw->html.num_anchor_underlines = 0;
	if (request->html.num_anchor_underlines > MAX_UNDERLINES)
		nw->html.num_anchor_underlines = MAX_UNDERLINES;
	if (request->html.num_visitedAnchor_underlines < 0)
		nw->html.num_visitedAnchor_underlines = 0;
	if (request->html.num_visitedAnchor_underlines > MAX_UNDERLINES)
		nw->html.num_visitedAnchor_underlines = MAX_UNDERLINES;

	if (request->html.raw_text != current->html.raw_text) {
		/* Free any old colors and pixmaps */
		FreeColors(current, current->core.colormap);
		/* Hide any old widgets */
		HideWidgets(current);
		/* Freeing these and forms is probably bad, but this section
		 * is never executed as far as I know */
		HTMLFreeWidgetInfo(current->html.widget_list);
		current->html.widget_list = NULL;
		nw->html.widget_list = NULL;
		nw->html.widget_focus = NULL;

		HTMLFreeFormInfo(current->html.form_list);
		current->html.form_list = NULL;
		nw->html.form_list = NULL;

		FreeMapList(current->html.map_list);
		current->html.map_list = NULL;
		nw->html.map_list = NULL;
		
		if (current->html.image_src) {
			HTBTreeAndObject_free(current->html.image_src);
			current->html.image_src = NULL;
		}
		nw->html.image_src = NULL;

		FreeMarkUpList(current->html.html_objects);
		FreeMarkUpList(current->html.html_header_objects);
		FreeMarkUpList(current->html.html_footer_objects);
		FreeMarkUpList(current->html.html_refresh_objects);

		/* Parse the raw text with the HTML parser */
		nw->html.html_objects = HTMLParse(nw, request->html.raw_text,
						  request->html.charset);
		if (request->html.header_text) {
			nw->html.html_header_objects =
				HTMLParse(nw, request->html.header_text, NULL);
		} else {
			nw->html.html_header_objects = NULL;
		}
		if (request->html.footer_text) {
			nw->html.html_footer_objects =
				HTMLParse(nw, request->html.footer_text, NULL);
		} else {
			nw->html.html_footer_objects = NULL;
		}
		/* Refreshes are done only once */
		nw->html.html_refresh_objects = NULL;

		/* Redisplay for the changed data. */
		nw->html.scroll_x = nw->html.scroll_y = 0;
		nw->html.max_pre_width = DocumentWidth(nw,
						       nw->html.html_objects);
		ReformatWindow(nw);
		ViewClearAndRefresh(nw);

		/* Clear any previous selection */
		nw->html.select_start = nw->html.select_end = NULL;
		nw->html.sel_start_pos = nw->html.sel_end_pos = 0;
		nw->html.new_start = nw->html.new_end = NULL;
		nw->html.new_start_pos = nw->html.new_end_pos = 0;
		nw->html.active_anchor = NULL;

		nw->html.cached_tracked_ele = NULL;
	} else if ((request->html.font != current->html.font) ||
	         (request->html.italic_font != current->html.italic_font) ||
	         (request->html.bold_font != current->html.bold_font) ||
	         (request->html.bolditalic_font != current->html.bolditalic_font) ||
	         (request->html.fixed_font != current->html.fixed_font) ||
	         (request->html.fixedbold_font != current->html.fixedbold_font) ||
	         (request->html.fixeditalic_font != current->html.fixeditalic_font) ||
	         (request->html.header1_font != current->html.header1_font) ||
	         (request->html.header2_font != current->html.header2_font) ||
	         (request->html.header3_font != current->html.header3_font) ||
	         (request->html.header4_font != current->html.header4_font) ||
	         (request->html.header5_font != current->html.header5_font) ||
	         (request->html.header6_font != current->html.header6_font) ||
	         (request->html.address_font != current->html.address_font) ||
	         (request->html.plain_font != current->html.plain_font) ||
	         (request->html.plainbold_font != current->html.plainbold_font) ||
	         (request->html.plainitalic_font != current->html.plainitalic_font) ||
	         (request->html.listing_font != current->html.listing_font) ||
	         (request->html.activeAnchor_fg != current->html.activeAnchor_fg) ||
	         (request->html.activeAnchor_bg != current->html.activeAnchor_bg) ||
	         (request->html.anchor_fg != current->html.anchor_fg) ||
	         (request->html.visitedAnchor_fg != current->html.visitedAnchor_fg) ||
	         (request->html.dashed_anchor_lines != current->html.dashed_anchor_lines) ||
	         (request->html.dashed_visitedAnchor_lines != current->html.dashed_visitedAnchor_lines) ||
	         (request->html.num_anchor_underlines != current->html.num_anchor_underlines) ||
	         (request->html.num_visitedAnchor_underlines != current->html.num_visitedAnchor_underlines))
	{
		if ((request->html.plain_font != current->html.plain_font) ||
		    (request->html.listing_font != current->html.listing_font))
			nw->html.max_pre_width = DocumentWidth(nw,
							 nw->html.html_objects);
		ReformatWindow(nw);
		ScrollWidgets(nw);
		ViewClearAndRefresh(nw);
	}
	/*
	 * Vertical space has been changed
	 */
	if (request->html.percent_vert_space !=
	    current->html.percent_vert_space) {
		ReformatWindow(nw);
		ScrollWidgets(nw);
		ViewClearAndRefresh(nw);
	}
	return(False);
}


static Boolean ConvertSelection(Widget w, Atom *selection, Atom *target,
				Atom *type, XtPointer *value,
				unsigned long *length, int *format)
{
	HTMLWidget hw = (HTMLWidget)w;
	Display *d = hw->html.dsp;

	if (!hw->html.select_start)
		return False;

	if (*target == XA_TARGETS(d)) {
		Atom *targetP, *std_targets;
		unsigned long std_length;

		XmuConvertStandardSelection(w, hw->html.selection_time,
#ifdef MOTIF1_3
			      selection, target, type, (XPointer *)&std_targets,
#else
			      selection, target, type,
			      (XtPointer *)&std_targets,
#endif
			      &std_length, format);

		*length = std_length + 5;
		*value = (XtPointer)XtMalloc(*length * sizeof(Atom));
		targetP = *(Atom **)value;
		*targetP++ = XA_STRING;
		*targetP++ = XA_TEXT(d);
		*targetP++ = XA_COMPOUND_TEXT(d);
		*targetP++ = XA_LENGTH(d);
		*targetP++ = XA_LIST_LENGTH(d);

		memcpy((char *)targetP, (char *)std_targets,
		       sizeof(Atom) * std_length);
		XtFree((char *)std_targets);
		*type = XA_ATOM;
		*format = 32;
		return True;
	}

	if (*target == XA_STRING || *target == XA_TEXT(d) ||
	    *target == XA_COMPOUND_TEXT(d)) {
		char *text;

		if (*target == XA_COMPOUND_TEXT(d)) {
			*type = *target;
		} else {
			*type = XA_STRING;
		}
		text = ParseTextToString(hw->html.formatted_elements,
				   hw->html.select_start, hw->html.select_end,
				   hw->html.sel_start_pos, hw->html.sel_end_pos,
				   hw->html.font->max_bounds.width,
				   hw->html.margin_width);
		*value = text;
		*length = strlen(*value);
		*format = 8;
		return True;
	}

	if (*target == XA_LIST_LENGTH(d)) {
		*value = XtMalloc(4);
		if (sizeof(long) == 4) {
			*(long *)*value = 1;
		} else {
			long temp = 1;

			memcpy((char *)*value,
			       ((char *)&temp) + sizeof(long) - 4, 4);
		}
		*type = XA_INTEGER;
		*length = 1;
		*format = 32;
		return True;
	}

	if (*target == XA_LENGTH(d)) {
		char *text;

		text = ParseTextToString(hw->html.formatted_elements,
				   hw->html.select_start, hw->html.select_end,
				   hw->html.sel_start_pos, hw->html.sel_end_pos,
				   hw->html.font->max_bounds.width,
				   hw->html.margin_width);
		*value = XtMalloc(4);
		if (sizeof(long) == 4) {
			*(long *)*value = strlen(text);
		} else {
			long temp = strlen(text);

			memcpy((char *)*value,
			       ((char *)&temp) + sizeof(long) - 4, 4);
		}
		free(text);
		*type = XA_INTEGER;
		*length = 1;
		*format = 32;
		return True;
	}

	if (XmuConvertStandardSelection(w, hw->html.selection_time, selection,
#ifdef MOTIF1_3
				        target, type, (XPointer *)value, length,
				        format))
#else
				        target, type, value, length, format))
#endif
		return True;
	return False;
}

static void LoseSelection(Widget w, Atom *selection)
{
	HTMLWidget hw = (HTMLWidget)w;

	/* Reset cached GC settings */
	hw->html.cur_font = NULL;
	XSetForeground(hw->html.dsp, hw->html.drawGC, hw->html.cur_fg);
	XSetBackground(hw->html.dsp, hw->html.drawGC, hw->html.cur_bg);

	ClearSelection(hw);
}

static void SelectionDone(Widget w, Atom *selection, Atom *target)
{
	/* Empty proc so Intrinsics know we want to keep storage */
}


/******************************* PUBLIC FUNCTIONS *************************/
/*
 * Convenience function to return the text of the HTML document as a plain
 * ascii text string.  This function allocates memory for the returned string,
 * that it is up to the user to free.
 * Extra option flags "pretty" text to be returned.
 * When pretty is two or larger, Postscript is returned.  The font used is
 * encoded in the pretty parameter:
 *   pretty = 2: Times
 *   pretty = 3: Helvetica
 *   pretty = 4: New century schoolbook
 *   pretty = 5: Lucida Bright
 */
char *HTMLGetText(Widget w, int pretty, char *url, char *time_str)
{
	HTMLWidget hw = (HTMLWidget)w;
	char *text = NULL;
	char *tptr;
	ElemInfo *start, *end;

	if (currentURL)
		free(currentURL);
	if (url && *url) {
		currentURL = strdup(url);
	} else {
		currentURL = strdup("UNKNOWN");
	}
	end = start = hw->html.formatted_elements;
	while (end)
		end = end->next;

	if (pretty >= 2) {
		tptr = ParseTextToPSString(hw, start, start, end, 0, 0,
					   hw->html.font->max_bounds.width,
					   hw->html.margin_width, pretty - 2,
					   url, time_str);
	} else if (pretty) {
		tptr = ParseTextToPrettyString(hw, start, start, end, 0, 0,
					       hw->html.font->max_bounds.width,
					       hw->html.margin_width);
	} else {
		tptr = ParseTextToString(start, start, end, 0, 0,
					 hw->html.font->max_bounds.width,
					 hw->html.margin_width);
	}
	if (tptr) {
		if (!text) {
			text = tptr;
		} else {
			char *buf = (char *)malloc(strlen(text) +
						   strlen(tptr) + 1);

			strcpy(buf, text);
			strcat(buf, tptr);
			free(text);
			free(tptr);
			text = buf;
		}
	}
	return(text);
}

/* Convenience function to return the element id of the first element
 * on or after the x,y coordinates passed in.
 * If no element is found, return the beginning of the document.
 */
int HTMLPositionToId(Widget w, int x, int y)
{
	HTMLWidget hw = (HTMLWidget)w;
	ElemInfo *eptr = NULL;

	/* We special case for when the scrollbars are at the
	 * absolute top and left. */
	if (!hw->html.scroll_y && !hw->html.scroll_x) 
		return(0);

	x += hw->html.scroll_x;
	y += hw->html.scroll_y;
	if (hw->html.scroll_x) {
		/* Find first element visible after or at x and y */
		eptr = hw->html.formatted_elements;
		while (eptr) {
			if (((eptr->y + eptr->height) <= y) ||
			    ((eptr->x + eptr->width) <= x)) {
				eptr = eptr->next;
				continue;
			} 
			break;
		}
	}

	/* None have both x and y in view area, so try with just y */
	if (!eptr && hw->html.scroll_y) {
		/* Find first element visible after or at y */
		eptr = hw->html.formatted_elements;
		while (eptr) {
			if ((eptr->y + eptr->height) <= y) {
				eptr = eptr->next;
				continue;
			} 
			break;
		}
	}
	/* 0 means the very top of the document.  We put you there
	 * for unfound elements.
	 */
	if (!eptr) 
		return(0);
	return(eptr->ele_id);
}

/* Convenience function to return the position of the element
 * based on the element id passed in.
 * Function returns 1 on success and fills in x,y pixel values.
 * If there is no such element, x=0, y=0 and -1 is returned.
 */
int HTMLIdToPosition(Widget w, int element_id, int *x, int *y)
{
	HTMLWidget hw = (HTMLWidget)w;
	ElemInfo *start;
	ElemInfo *eptr = NULL;

	start = hw->html.formatted_elements;
	while (start) {
		if (start->ele_id == element_id) {
			eptr = start;
			break;
		}
		start = start->next;
	}
	if (!eptr) {
		*x = 0;
		*y = 0;
		return(-1);
	}
	*x = eptr->x;
	*y = eptr->y;
	return(1);
}

/*
 * Convenience function to position the element
 * based on the element id passed at the top of the viewing area.
 * A passed in id of 0 means goto the top.
 */
/* 
 * "correction" is either -1, 0, or 1. These values determine if we are
 *   to set the pointer a 1/2 page in the negative or positive direction...or
 *   simply leave it alone.
 */ 
void HTMLGotoId(Widget w, int element_id, int correction)
{
	HTMLWidget hw = (HTMLWidget)w;
	ElemInfo *start;
	ElemInfo *eptr = NULL;
	int newy, val, size, inc, pageinc;

	/* If we have no scrollbar, just return. */
	if (!hw->html.use_vbar)
		return;
	/* Find the element corrsponding to the id passed in. */
	start = hw->html.formatted_elements;
	while (start) {
		if (start->ele_id == element_id) {
			eptr = start;
			break;
		}
		start = start->next;
	}

	/* No such element, do nothing. */
	if (element_id && !eptr)
		return;
	if (!element_id) {
		newy = 0;
	} else {
                if (!correction) {    
                        newy = eptr->y - 2; 
                } else if (correction < 0) {  /* "up" a 1/2 page */
                        newy = eptr->y - 2 - ((int)(hw->html.view_height) / 2);
                } else {  /* "down" a 1/2 page */
                        newy = eptr->y - 2 + ((int)(hw->html.view_height) / 2);
                }                     
        }   
	if (newy < 0) {
		newy = 0;
	} else if (newy > (hw->html.doc_height - (int)hw->html.view_height)) {
		newy = hw->html.doc_height - (int)hw->html.view_height;
	}
	if (newy < 0)
		newy = 0;
	XmScrollBarGetValues(hw->html.vbar, &val, &size, &inc, &pageinc);
	XmScrollBarSetValues(hw->html.vbar, newy, size, inc, pageinc, True);
	XmScrollBarGetValues(hw->html.hbar, &val, &size, &inc, &pageinc);
	XmScrollBarSetValues(hw->html.hbar, 0, size, inc, pageinc, True);
}

/*
 * Convenience function to return the position of the anchor
 * based on the anchor NAME passed.
 * Function returns 1 on success and fills in x,y pixel values.
 * If there is no such element, x=0, y=0 and -1 is returned.
 */
int HTMLAnchorToPosition(Widget w, char *name, int *x, int *y)
{
	HTMLWidget hw = (HTMLWidget)w;
	ElemInfo *start;
	ElemInfo *eptr = NULL;

	start = hw->html.formatted_elements;
	while (start) {
		if (start->anchor_tag_ptr->anc_name &&
		    !strcmp(start->anchor_tag_ptr->anc_name, name)) {
			eptr = start;
			break;
		}
		start = start->next;
	}
	if (!eptr) {
		*x = 0;
		*y = 0;
		return(-1);
	}
	*x = eptr->x;
	*y = eptr->y;
	return(1);
}

/*
 * Convenience function to return the element id of the anchor
 * based on the anchor NAME passed.
 * Function returns id on success.
 * If there is no such element, 0 is returned.
 */
int HTMLAnchorToId(Widget w, char *name)
{
	HTMLWidget hw = (HTMLWidget)w;
	ElemInfo *start;
	ElemInfo *eptr = NULL;

	/* Find the passed anchor name */
	start = hw->html.formatted_elements;
	while (start) {
		if (start->anchor_tag_ptr->anc_name &&
		    !strcmp(start->anchor_tag_ptr->anc_name, name)) {
			eptr = start;
			break;
		}
		start = start->next;
	}
	if (!eptr)
		return(0);
	return(eptr->ele_id);
}

/*
 * Convenience function to return the element rec of the anchor
 * based on the anchor NAME passed.
 * If there is no such element rec, NULL is returned.
 */
static ElemInfo *HTMLAnchorToEptr(HTMLWidget hw, char *name)
{
	ElemInfo *start = hw->html.formatted_elements;
	ElemInfo *eptr = NULL;

	/* Find the passed anchor name */
	while (start) {
		if (start->anchor_tag_ptr->anc_name &&
		    !strcmp(start->anchor_tag_ptr->anc_name, name)) {
			eptr = start;
			break;
		}
		start = start->next;
	}
	return(eptr);
}

/*
 * Convenience function to return the HREFs of all active anchors in the
 * document.
 * Function returns an array of strings and fills num_hrefs passed.
 * If there are no HREFs, NULL returned.
 */
char **HTMLGetHRefs(Widget w, int *num_hrefs)
{
	HTMLWidget hw = (HTMLWidget)w;
	int cnt = 0;
	ElemInfo *start, *eptr;
	ElemInfo *list = NULL;
	char **harray;

	/*
	 * Construct a linked list of all the different hrefs, counting
	 * them as we go.
	 */
	start = hw->html.formatted_elements;
	while (start) {
		/*
		 * This one has an HREF
		 */
		if (start->anchor_tag_ptr->anc_href) {
			/*
			 * Check to see if we already have
			 * this HREF in our list.
			 */
			eptr = list;
			while (eptr) {
				if (!strcmp(eptr->anchor_tag_ptr->anc_href,
				            start->anchor_tag_ptr->anc_href))
					break;
				eptr = eptr->next;
			}
			/*
			 * This HREF is not, in our list.  Add it.
			 * That is, if it's not an internal reference.
			 */
			if (!eptr) {
				eptr = GetElemRec();
				eptr->anchor_tag_ptr = start->anchor_tag_ptr;
				eptr->next = list;
				list = eptr;
				cnt++;
			}
		}
		start = start->next;
	}

	if (!cnt) {
		*num_hrefs = 0;
		return(NULL);
	} 
	*num_hrefs = cnt;
	harray = (char **)malloc(sizeof(char *) * cnt);
	eptr = list;
	cnt--;
	while (eptr) {
		harray[cnt--] = strdup(eptr->anchor_tag_ptr->anc_href);
		eptr = eptr->next;
	}
	FreeLineList(list, hw);
	return(harray);
}

/*
 * Convenience function to return the SRCs of all images in the document.
 * Function returns an array of strings and fills num_srcs passed.
 * If there are no SRCs, NULL returned.
 */
char **HTMLGetImageSrcs(Widget w, int *num_srcs)
{
	HTMLWidget hw = (HTMLWidget)w;
	MarkInfo *mptr;
	int cnt = 0;
	char *tptr;
	char **harray;

	mptr = hw->html.html_objects;
	while (mptr) {
		if (mptr->type == M_IMAGE) {
			tptr = ParseMarkTag(mptr->start, MT_IMAGE, "SRC");
			if (tptr) {
				if (*tptr)
					cnt++;
				free(tptr);
			}
		} else if (mptr->type == M_FIGURE) {
			tptr = ParseMarkTag(mptr->start, MT_FIGURE, "SRC");
			if (tptr) {
				if (*tptr)
					cnt++;
				free(tptr);
			}
		}
		mptr = mptr->next;
	}
	if (!cnt) {
		*num_srcs = 0;
		return(NULL);
	} 
	*num_srcs = cnt;
	harray = (char **)malloc(sizeof(char *) * cnt);
	mptr = hw->html.html_objects;
	cnt = 0;
	while (mptr) {
		if (mptr->type == M_IMAGE) {
			tptr = ParseMarkTag(mptr->start, MT_IMAGE, "SRC");
			if (tptr && *tptr) {
				harray[cnt++] = tptr;
			} else if (tptr) {
				free(tptr);
			}
		} else if (mptr->type == M_FIGURE) {
			tptr = ParseMarkTag(mptr->start, MT_FIGURE, "SRC");
			if (tptr && *tptr) {
				harray[cnt++] = tptr;
			} else if (tptr) {
				free(tptr);
			}
		}
		mptr = mptr->next;
	}
	return(harray);
}

/*
 * Convenience function to return the link information
 * for all the <LINK> tags in the document.
 * Function returns an array of LinkInfo structures and fills
 * num_links passed.
 * If there are no LINKs NULL returned.
 */
LinkInfo *HTMLGetLinks(Widget w, int *num_links)
{
	HTMLWidget hw = (HTMLWidget)w;
	MarkInfo *mptr;
	int cnt = 0;
	LinkInfo *larray;

	mptr = hw->html.html_objects;
	while (mptr) {
		if (mptr->type == M_LINK)
			cnt++;
		mptr = mptr->next;
	}
	if (!cnt) {
		*num_links = 0;
		return(NULL);
	}
	*num_links = cnt;
	larray = (LinkInfo *)malloc(sizeof(LinkInfo) * cnt);
	mptr = hw->html.html_objects;
	cnt = 0;
	while (mptr) {
		if (mptr->type == M_LINK)
			larray[cnt++].href = ParseMarkTag(mptr->start, MT_LINK,
							  "HREF");
		mptr = mptr->next;
	}
	return(larray);
}

/* Convenience function to redraw all active anchors in the document.
 * Can also pass a new predicate function to check visited
 * anchors.  If NULL passed for function, uses default predicate function.
 */
void HTMLRetestAnchors(Widget w, visitTestProc testFunc)
{
	HTMLWidget hw = (HTMLWidget)w;
	ElemInfo *start;
	int doc_x = hw->html.scroll_x;
	int doc_y = hw->html.scroll_y;
	int doc_end_x = hw->html.scroll_x + hw->html.view_width;
	int doc_end_y = hw->html.scroll_y + hw->html.view_height;

	/* Reset cached GC settings */
	hw->html.cur_font = NULL;
	XSetForeground(hw->html.dsp, hw->html.drawGC, hw->html.cur_fg);
	XSetBackground(hw->html.dsp, hw->html.drawGC, hw->html.cur_bg);

	if (!testFunc)
		testFunc = (visitTestProc)hw->html.previously_visited_test;
	/*
	 * Search all elements
	 */
	start = hw->html.formatted_elements;
	hw->html.underline_yoffset = -1;
	while (start) {
		if (!start->anchor_tag_ptr->anc_href ||
		    start->fixed_anchor_color) {
			if (start->type == E_CR)
				hw->html.underline_yoffset = -1;
			start = start->next;
			continue;
		}
		if (testFunc) {
			if ((*testFunc)((Widget)hw,
				        start->anchor_tag_ptr->anc_href)) {
				start->fg = hw->html.visitedAnchor_fg;
				start->underline_number =
					hw->html.num_visitedAnchor_underlines;
				start->dashed_underline =
					hw->html.dashed_visitedAnchor_lines;
			} else {
				start->fg = hw->html.anchor_fg;
				start->underline_number =
					hw->html.num_anchor_underlines;
			 	start->dashed_underline =
					hw->html.dashed_anchor_lines;
			}
		} else {
			start->fg = hw->html.anchor_fg;
			start->underline_number =
				hw->html.num_anchor_underlines;
			start->dashed_underline = hw->html.dashed_anchor_lines;
		}
		/*
		 * Since the element may have changed, redraw it if visible
		 */
		if (((start->y + start->height) < doc_y) || 
		    (start->y > doc_end_y) ||
		    ((start->x + start->width) < doc_x) ||
		    (start->x > doc_end_x)) {
			/* Not visible */
			start = start->next;
			continue;
		}
		switch(start->type) {
		    case E_TEXT:
			TextRefresh(hw, start, 0, start->edata_len - 2, False);
			break;
		    case E_IMAGE:
			ImageRefresh(hw, start, NULL);
			break;
		    case E_BULLET:
			BulletRefresh(hw, start);
			break;
		}
		start = start->next;
	}
}

void HTMLClearSelection(Widget w)
{
	ClearSelection((HTMLWidget)w);
}

/*
 * Set the current selection based on the ElementRefs passed in.
 * Both refs must be valid.
 */
void HTMLSetSelection(Widget w, ElementRef *start, ElementRef *end)
{
	HTMLWidget hw = (HTMLWidget)w;
	ElemInfo *eptr, *e_start, *e_end;
	int start_pos, end_pos, i, buffer, found;
	Atom *atoms;
	char *text;
	char *params[2];

	/*
	 * If the starting position is not valid, fail the selection
	 */
	if ((start->id > 0) && (start->pos >= 0)) {
		found = 0;
		eptr = hw->html.formatted_elements;

		while (eptr) {
			if (eptr->ele_id == start->id) {
				e_start = eptr;
				start_pos = start->pos;
				found = 1;
				break;
			}
			eptr = eptr->next;
		}
		if (!found)
			return;
	}
	/*
	 * If the ending position is not valid, fail the selection
	 */
	if ((end->id > 0) && (end->pos >= 0)) {
		found = 0;
		eptr = hw->html.formatted_elements;

		while (eptr) {
			if (eptr->ele_id == end->id) {
				e_end = eptr;
				end_pos = end->pos;
				found = 1;
				break;
			}
			eptr = eptr->next;
		}
		if (!found)
			return;
	}

	ClearSelection(hw);

	/*
	 * We expect the ElementRefs came from HTMLSearchText, so we know
	 * that the end_pos is one past what we want to select.
	 */
	end_pos -= 1;

	/*
	 * Sanify the position data
	 */
	if ((start_pos > 0) && (start_pos >= e_start->edata_len - 1))
		start_pos = e_start->edata_len - 2;
	if ((end_pos > 0) && (end_pos >= e_end->edata_len - 1))
		end_pos = e_end->edata_len - 2;

	hw->html.select_start = e_start;
	hw->html.sel_start_pos = start_pos;
	hw->html.select_end = e_end;
	hw->html.sel_end_pos = end_pos;
	SetSelection(hw);
	hw->html.new_start = hw->html.new_end = NULL;
	hw->html.new_start_pos = hw->html.new_end_pos = 0;

	/*
	 * Do all the gunk from the end of the ExtendEnd function
	 */
	params[0] = "PRIMARY";
	params[1] = "CUT_BUFFER0";
	atoms = (Atom *)malloc(2 * sizeof(Atom));
	if (!atoms) {
		fprintf(stderr, "Cannot allocate atom list\n");
		return;
	}
	XmuInternStrings(hw->html.dsp, params, 2, atoms);
	hw->html.selection_time = CurrentTime;
	for (i = 0; i < 2; i++) {
		switch (atoms[i]) {
			case XA_CUT_BUFFER0: buffer = 0; break;
			case XA_CUT_BUFFER1: buffer = 1; break;
			case XA_CUT_BUFFER2: buffer = 2; break;
			case XA_CUT_BUFFER3: buffer = 3; break;
			case XA_CUT_BUFFER4: buffer = 4; break;
			case XA_CUT_BUFFER5: buffer = 5; break;
			case XA_CUT_BUFFER6: buffer = 6; break;
			case XA_CUT_BUFFER7: buffer = 7; break;
			default: buffer = -1; break;
		}
		if (buffer >= 0) {
			text = ParseTextToString(hw->html.formatted_elements,
						hw->html.select_start,
						hw->html.select_end,
						hw->html.sel_start_pos,
						hw->html.sel_end_pos,
						hw->html.font->max_bounds.width,
						hw->html.margin_width);
			XStoreBuffer(hw->html.dsp, text, strlen(text), buffer);
			free(text);
		} else {
			XtOwnSelection((Widget)hw, atoms[i], CurrentTime,
				       (XtConvertSelectionProc)ConvertSelection,
				       (XtLoseSelectionProc)LoseSelection,
				       (XtSelectionDoneProc)SelectionDone);
		}
	}
	free((char *)atoms);
}

/*
 * Convenience function to return the text of the HTML document as a single
 * white space separated string, with pointers to the various start and
 * end points of selections.
 * This function allocates memory for the returned string, that it is up
 * to the user to free.
 */
char *HTMLGetTextAndSelection(Widget w, char **startp, char **endp,
			      char **insertp)
{
	HTMLWidget hw = (HTMLWidget)w;
	int length = 0;
	char *text, *tptr;
	ElemInfo *eptr, *sel_start, *sel_end, *insert_start;
	int start_pos, end_pos, insert_pos;

	if (SwapElements(hw->html.select_start, hw->html.select_end,
		         hw->html.sel_start_pos, hw->html.sel_end_pos)) {
		sel_end = hw->html.select_start;
		end_pos = hw->html.sel_start_pos;
		sel_start = hw->html.select_end;
		start_pos = hw->html.sel_end_pos;
	} else {
		sel_start = hw->html.select_start;
		start_pos = hw->html.sel_start_pos;
		sel_end = hw->html.select_end;
		end_pos = hw->html.sel_end_pos;
	}
	insert_start = hw->html.new_start;
	insert_pos = hw->html.new_start_pos;
	*startp = NULL;
	*endp = NULL;
	*insertp = NULL;

	eptr = hw->html.formatted_elements;
	while (eptr) {
		if (eptr->type == E_TEXT) {
			length += eptr->edata_len - 1;
		} else if (eptr->type == E_LINEFEED) {
			length++;
		}
		eptr = eptr->next;
	}
	tptr = text = (char *)calloc(length + 1, 1);
	if (!text) {
		fprintf(stderr, "No space for return string\n");
		return(NULL);
	}

	eptr = hw->html.formatted_elements;
	while (eptr) {
		if (eptr->type == E_TEXT) {
			if (eptr == sel_start)
				*startp = (char *)(tptr + start_pos);
			if (eptr == sel_end)
				*endp = (char *)(tptr + end_pos);
			if (eptr == insert_start)
				*insertp = (char *)(tptr + insert_pos);
			strcat(text, (char *)eptr->edata);
			tptr += eptr->edata_len - 1;
		} else if (eptr->type == E_LINEFEED) {
			if (eptr == sel_start)
				*startp = tptr;
			if (eptr == sel_end)
				*endp = tptr;
			if (eptr == insert_start)
				*insertp = tptr;
			strcat(text, " ");
			tptr++;
		}
		eptr = eptr->next;
	}
	return(text);
}

/*
 * Idle work process which processes images which were not processed
 * during the initial display (i.e., they were not within view).
 * Processes only one image each time to avoid extended blockages.
 */
static int FinishImages(XtPointer cld)
{
	WorkInfo *work = (WorkInfo *)cld;
	ImageInfo *pic_data;
	ElemInfo *eptr = work->eptr;
	HTMLWidget hw = work->hw;

	/* Quit if display changed */
	if (work->drawing != hw->html.draw_count)
		eptr = NULL;

	while (eptr) {
		if ((eptr->type == E_IMAGE) && !eptr->pic_data->image) {
		    pic_data = eptr->pic_data;
		    if (pic_data->image = InfoToImage(hw, pic_data, 0, eptr)) {
		    	if (pic_data->transparent && !pic_data->clip) {
			    pic_data->clip = XCreatePixmapFromBitmapData(
						   hw->html.dsp,
						   XtWindow(hw->html.view),
						   (char *) pic_data->clip_data,
						   pic_data->width,
						   pic_data->height,
						   1, 0, 1);
			} else if (!pic_data->transparent) {
			    pic_data->clip = None;
			}
		    }
		    /* Force background overwrite the first time. */
		    pic_data->background_pixel = (Pixmap) 0xFFFFFFFF;

		    work->eptr = eptr->next;
#ifndef DISABLE_TRACE
		    if (htmlwTrace)
			fprintf(stderr, "Did image in FinishImages\n");
#endif
		    return 0;
		}
		eptr = eptr->next;
	}
	free(work);
	hw->html.workprocid = 0;
	return(1);
}

/* Function to set the raw text into the widget.
 * Forces a reparse and a reformat.
 * If any pointer is passed in as NULL that text is unchanged.
 * If a pointer points to an empty string, that text is set to NULL.
 * Also pass an element ID to set the view area to that section of the new
 * text.  Finally pass an anchor NAME to set position of the new text
 * to that anchor.
 */
void HTMLSetText(Widget w, char *text, char *header_text, char *footer_text,
		 int element_id, char *target_anchor, void *ptr,
		 char *refresh_text, char *charset)
{
	HTMLWidget hw = (HTMLWidget)w;
	WidgetInfo *wptr = (WidgetInfo *)ptr;
	ElemInfo *start;
	ElemInfo *eptr = NULL;
	int newx, newy;

#ifndef DISABLE_TRACE
        if (htmlwTrace)
                fprintf(stderr, "Entering HTMLSetText\n");
#endif
	if (!text && !header_text && !footer_text && !refresh_text)
		return;

	hw->html.drawing = 1;		/* Disable animation stop button */

	/* Restore default colors as required */
	hw->html.cur_fg = hw->manager.foreground = hw->html.foreground_SAVE;
	XSetForeground(hw->html.dsp, hw->html.drawGC, hw->html.cur_fg);
        hw->html.anchor_fg = hw->html.anchor_fg_SAVE;
        hw->html.visitedAnchor_fg = hw->html.visitedAnchor_fg_SAVE;
        hw->html.activeAnchor_fg = hw->html.activeAnchor_fg_SAVE;
        hw->html.activeAnchor_bg = hw->html.activeAnchor_bg_SAVE;
        hw->html.cur_bg = hw->core.background_pixel = hw->html.background_SAVE;
        hw->html.view->core.background_pixel = hw->html.background_SAVE;
	XSetBackground(hw->html.dsp, hw->html.drawGC, hw->html.cur_bg);
        if (hw->html.top_color_SAVE != hw->manager.top_shadow_color) {
		/* Freed here since hw_do_color does not track the allocation */
        	XFreeColors(hw->html.dsp, hw->core.colormap,
                            &hw->manager.top_shadow_color, 1, 0);
        	hw->manager.top_shadow_color = hw->html.top_color_SAVE;
        }            
        if (hw->html.bottom_color_SAVE != hw->manager.bottom_shadow_color) {
        	XFreeColors(hw->html.dsp, hw->core.colormap,
                            &hw->manager.bottom_shadow_color, 1, 0);
        	hw->manager.bottom_shadow_color = hw->html.bottom_color_SAVE;
        }            
	hw->html.cur_font = NULL;

	HideWidgets(hw); 		/* Hide any old widgets */
	/* Widgets and forms freed in History.c */
	hw->html.widget_list = wptr;
	hw->html.widget_focus = NULL;
	hw->html.form_list = NULL;
	if (hw->html.map_list) {
		FreeMapList(hw->html.map_list);
		hw->html.map_list = NULL;
	}
	if (hw->html.image_src) {
		HTBTreeAndObject_free(hw->html.image_src);
		hw->html.image_src = NULL;
	}
	/* Keep charset, if any */
	if (charset) {
		hw->html.charset = strdup(charset);
	} else {
		hw->html.charset = NULL;
	}

	if (text) {
		if (!*text)
			text = NULL;
		hw->html.raw_text = text;

		/* Free any old colors, pixmaps and markup */
		FreeColors(hw, hw->core.colormap);
		FreeMarkUpList(hw->html.html_objects);  /* Clear previous */

		/* Parse the raw text with the HTML parser */
		hw->html.html_objects = HTMLParse(hw, text, charset);
	}
	if (header_text) {
		/* GUI expects us to free any old text */
		if (hw->html.header_text)
			free(hw->html.header_text);
		if (!*header_text)
			header_text = NULL;
		hw->html.header_text = header_text;
		FreeMarkUpList(hw->html.html_header_objects);
		/*
		 * Parse the header text with the HTML parser
		 */
		if (header_text) {
			hw->html.html_header_objects =
					       HTMLParse(hw, header_text, NULL);
		} else {
			hw->html.html_header_objects = NULL;
		}
	}
	if (footer_text) {
		/* GUI expects us to free any old text */
		if (hw->html.footer_text)
			free(hw->html.footer_text);
		if (!*footer_text)
			footer_text = NULL;
		hw->html.footer_text = footer_text;
		FreeMarkUpList(hw->html.html_footer_objects);
		/*
		 * Parse the footer text with the HTML parser
		 */
		if (footer_text) {
			hw->html.html_footer_objects =
					       HTMLParse(hw, footer_text, NULL);
		} else {
			hw->html.html_footer_objects = NULL;
		}
	}
	if (refresh_text) {
		FreeMarkUpList(hw->html.html_refresh_objects);
		/*
		 * Parse the refresh text with the HTML parser
		 */
		if (*refresh_text) {
			hw->html.html_refresh_objects =
					      HTMLParse(hw, refresh_text, NULL);
		} else {
			hw->html.html_refresh_objects = NULL;
		}
	}

	/* Start image loads */
	if (hw->html.image_src)
		XtCallCallbackList((Widget)hw, hw->html.multi_load_callback,"");

	/* Reformat the new text */
	hw->html.max_pre_width = DocumentWidth(hw, hw->html.html_objects);

	/* Get scroll stuff out of way of progressive display */
	XtUnmanageChild(hw->html.vbar);
	hw->html.scroll_y = 0;
	XtUnmanageChild(hw->html.hbar);
	hw->html.scroll_x = 0;
	ReformatWindow(hw);	/* Rescan all tag and make all */

	/* Position text at target anchor or specified id, or at top
	 * if no position specified.
	 *
	 * If a target anchor is passed, get the element rec
	 */
	if (target_anchor)
		eptr = HTMLAnchorToEptr(hw, target_anchor);

	/* or find the element corresponding to the id passed in. */
	if (!eptr && element_id) {
		start = hw->html.formatted_elements;
		while (start) {
			if (start->ele_id == element_id) {
				eptr = start;
				break;
			}
			start = start->next;
		}
	}
	if (!eptr) {
		newx = newy = 0;
	} else {
		/* Don't move x if going to target anchor */
		if (!target_anchor) {
			newx = eptr->x - 2;
			if (newx <= hw->html.margin_width)
				newx = 0;
		} else {
			newx = 0;
		}
		newy = eptr->y - 2;
		if (newy <= hw->html.margin_height)
			newy = 0;
	}
	if (newx > (hw->html.doc_width - (int)hw->html.view_width))
		newx = hw->html.doc_width - (int)hw->html.view_width;
	if (newy > (hw->html.doc_height - (int)hw->html.view_height))
		newy = hw->html.doc_height - (int)hw->html.view_height;
	if (newx < 0)
		newx = 0;
	if (newy < 0)
		newy = 0;
	hw->html.scroll_x = newx;
	hw->html.scroll_y = newy;
#ifndef DISABLE_TRACE
	if (htmlwTrace)
		fprintf(stderr, "Display update in HTMLSetText\n");
#endif
	/* Hide any progressively displayed widgets, if not starting at top */
	if ((hw->html.scroll_y > 0) || (hw->html.scroll_x > 0))
		HideWidgets(hw);

	ConfigScrollBars(hw);
	ScrollWidgets(hw);
	ViewClearAndRefresh(hw); 	/* Display the new text */
	/* Finish processing any unprocessed images */
	if (!DidAllImages) {
		WorkInfo *cdata = (WorkInfo *) malloc(sizeof(WorkInfo));

		cdata->drawing = hw->html.draw_count;
		cdata->eptr = hw->html.formatted_elements;
		cdata->hw = hw;
		hw->html.workprocdata = cdata;
		hw->html.workprocid = XtAppAddWorkProc(app_con,
						       (XtWorkProc)FinishImages,
						       (XtPointer)cdata);
#ifndef DISABLE_TRACE
		if (htmlwTrace)
			fprintf(stderr, "Started FinishImages\n");
#endif
	}
	/* Clear any previous selection */
	hw->html.select_start = hw->html.select_end = NULL;
	hw->html.sel_start_pos = hw->html.sel_end_pos = 0;
	hw->html.new_start = hw->html.new_end = NULL;
	hw->html.new_start_pos = hw->html.new_end_pos = 0;
	hw->html.active_anchor = NULL;
	hw->html.drawing = 0;		/* Enable animation stop button */
}

/* Allows us to jump to the bottom of a document (or very close).
 */
int HTMLLastId(Widget w)
{
	HTMLWidget hw = (HTMLWidget)w;
	ElemInfo *eptr;
	int Id;

	if (!w)
		return(0);
	eptr = hw->html.formatted_elements;
	Id = eptr->ele_id;
	while (eptr->next) {
		Id = eptr->ele_id;
		eptr = eptr->next;
	}
	return(Id);
}

/* News hack of searching function for HTML widget... only looks for an edata
 * of ">>>" as it will be by itself because the one we are looking for
 * will be enclosed in a <b></b>.
 */                   
int HTMLSearchNews(Widget w, ElementRef *m_start, ElementRef *m_end)
{                     
      HTMLWidget hw = (HTMLWidget)w;
      ElemInfo *eptr;
                      
      /*                             
       * If bad parameters are passed, just fail the search
       */
      if (!m_start || !m_end)
              return(-1);

      eptr = hw->html.formatted_elements;

      while (eptr) {
              if (eptr->type == E_TEXT) {
                      if (eptr->edata && !strcmp(eptr->edata, ">>>")) {
                              m_start->id = eptr->ele_id;
                              m_start->pos = 0;
                              m_end->id = eptr->ele_id;
                              m_end->pos = 3;
                              return(1);
                      }
              }
              eptr = eptr->next;
      }
      return(-1);
}

/* Convenience function to search the text of the HTML document as a single
 * white space separated string.  Linefeeds are converted into spaces.
 *
 * Takes a pattern, pointers to the start and end blocks to store the
 * start and end of the match into.  Start is also used as the location to
 * start the search from for incremental searching.  If start is an invalid
 * position (id = 0).  Default start is the beginning of the document for
 * forward searching, and the end of the document for backwards searching.
 * The backward and caseless parameters I hope are self-explanatory.
 *
 * returns 1 on success
 *      (and the start and end positions of the match).
 * returns -1 otherwise (and start and end are unchanged).
 */
int HTMLSearchText(Widget w, char *pattern,
		   ElementRef *m_start, ElementRef *m_end,
		   int backward, int caseless)
{
	HTMLWidget hw = (HTMLWidget)w;
	int found, equal, s_pos;
	char *match, *tptr, *mptr;
	char cval;
	ElemInfo *eptr, *s_eptr;
	ElementRef s_ref, e_ref;
	ElementRef *start, *end;

	/* If bad parameters are passed, just fail the search */
	if (!pattern || !*pattern || !m_start || !m_end)
		return(-1);

	/* If we are caseless, make a lower case copy of the pattern to
	 * use in compares.  Remember to free this before returning.
	 */
	if (caseless) {
		mptr = match = (char *)malloc(strlen(pattern) + 1);
		tptr = pattern;
		while (*tptr) {
			*mptr++ = (char)TOLOWER((int)*tptr);
			tptr++;
		}
		*mptr = '\0';
	} else {
		match = pattern;
	}

	/* Slimy coding.  I later decided I didn't want to change start and
	 * end if the search failed.  Rather than changing all the code, I just
	 * copy it into locals here, and copy it out again if a match is found.
	 */
	start = &s_ref;
	end = &e_ref;
	start->id = m_start->id;
	start->pos = m_start->pos;
	end->id = m_end->id;
	end->pos = m_end->pos;

	/*
	 * Find the user specified start position.
	 */
	if (start->id > 0) {
		found = 0;
		eptr = hw->html.formatted_elements;

		while (eptr) {
			if (eptr->ele_id == start->id) {
				s_eptr = eptr;
				found = 1;
				break;
			}
			eptr = eptr->next;
		}
		/*
		 * Bad start position, fail them out.
		 */
		if (!found) {
			if (caseless)
				free(match);
			return(-1);
		}
		/*
		 * Sanify the start position
		 */
		s_pos = start->pos;
		if (s_pos >= s_eptr->edata_len - 1)
			s_pos = s_eptr->edata_len - 2;
		if (s_pos < 0)
			s_pos = 0;
	} else {
		/*
		 * Default search starts at end for backward, and
		 * beginning for forwards.
		 */
		if (backward) {
			s_eptr = hw->html.formatted_elements;
			while (s_eptr->next)
				s_eptr = s_eptr->next;
			s_pos = s_eptr->edata_len - 2;
		} else {
			s_eptr = hw->html.formatted_elements;
			s_pos = 0;
		}
	}
	if (backward) {
		char *mend = match;

		/*
		 * Save the end of match here for easy end to start searching
		 */
		while (*mend)
			mend++;
		if (mend > match)
			mend--;
		found = equal = 0;
		mptr = mend;

		if (s_eptr) {
			eptr = s_eptr;
		} else {
			eptr = hw->html.formatted_elements;
			while (eptr->next)
				eptr = eptr->next;
		}
		while (eptr) {
			if (eptr->type == E_TEXT) {
			    tptr = (char *)(eptr->edata + eptr->edata_len - 2);
			    if (eptr == s_eptr)
				tptr = (char *)(eptr->edata + s_pos);
			    while (tptr >= eptr->edata) {
				if (equal) {
				    if (caseless) {
					cval = (char)TOLOWER((int)*tptr);
				    } else {
					cval = *tptr;
				    }
				    while ((mptr >= match) &&
					   (tptr >= eptr->edata) &&
					   (cval == *mptr)) {
					mptr--;
					if (--tptr >= eptr->edata) {
					    if (caseless) {
						cval = (char)
							    TOLOWER((int)*tptr);
					    } else {
						cval = *tptr;
					    }
					}
				    }
				    if (mptr < match) {
					found = 1;
					start->id = eptr->ele_id;
					start->pos = (int)
						       (tptr - eptr->edata + 1);
					break;
				    } else if (tptr < eptr->edata) {
					break;
				    } else {
					equal = 0;
				    }
				} else {
				    mptr = mend;
				    if (caseless) {
					cval = (char)TOLOWER((int)*tptr);
				    } else {
					cval = *tptr;
				    }
				    while ((tptr >= eptr->edata) &&
					   (cval != *mptr)) {
					if (--tptr >= eptr->edata) {
					    if (caseless) {
						cval = (char)
							    TOLOWER((int)*tptr);
					    } else {
						cval = *tptr;
					    }
					}
				    }
				    if ((tptr >= eptr->edata) &&
					(cval == *mptr)) {
					equal = 1;
					end->id = eptr->ele_id;
					end->pos = (int)
						       (tptr - eptr->edata + 1);
				    }
				}
			    }
			}
			/*
			 * Linefeeds match to single space characters.
			 */
			else if (eptr->type == E_LINEFEED) {
			    if (equal) {
				if (*mptr == ' ') {
				    if (--mptr < match) {
					found = 1;
					start->id = eptr->ele_id;
					start->pos = 0;
				    }
				} else {
				    equal = 0;
				}
			    } else {
				mptr = mend;
				if (*mptr == ' ') {
				    equal = 1;
				    end->id = eptr->ele_id;
				    end->pos = 0;
				    if (--mptr < match) {
					found = 1;
					start->id =eptr->ele_id;
					start->pos = 0;
				    }
				}
			    }
			}
			if (found)
			    break;
			eptr = eptr->prev;
		}
	} else {  /* Forward */
		found = equal = 0;
		mptr = match;

		if (s_eptr) {
			eptr = s_eptr;
		} else {
			eptr = hw->html.formatted_elements;
		}
		while (eptr) {
			if (eptr->type == E_TEXT) {
			    tptr = eptr->edata;
			    if (eptr == s_eptr)
				tptr = (char *)(tptr + s_pos);
			    while (*tptr ) {
				if (equal) {
				    if (caseless) {
					cval = (char)TOLOWER((int)*tptr);
				    } else {
					cval = *tptr;
				    }
				    while (*mptr && (cval == *mptr)) {
					tptr++;
					mptr++;
					if (caseless) {
					    cval = (char)TOLOWER((int)*tptr);
					} else {
					    cval = *tptr;
					}
				    }
				    if (!*mptr) {
					found = 1;
					end->id = eptr->ele_id;
					end->pos = (int)(tptr - eptr->edata);
					break;
				    } else if (!*tptr) {
					break;
				    } else {
					equal = 0;
				    }
				} else {
				    mptr = match;
				    if (caseless) {
					cval = (char)TOLOWER((int)*tptr);
				    } else {
					cval = *tptr;
				    }
				    while (*tptr && (cval != *mptr)) {
					tptr++;
					if (caseless) {
					    cval = (char)TOLOWER((int)*tptr);
					} else {
					    cval = *tptr;
					}
				    }
				    if (cval == *mptr) {
					equal = 1;
					start->id = eptr->ele_id;
					start->pos = (int)(tptr - eptr->edata);
				    }
				}
			    }
			} else if (eptr->type == E_LINEFEED) {
			    if (equal) {
				if (*mptr == ' ') {
				    if (!*++mptr) {
					found = 1;
					end->id = eptr->ele_id;
					end->pos = 0;
				    }
				} else {
				    equal = 0;
				}
			    } else {
				mptr = match;
				if (*mptr == ' ') {
				    equal = 1;
				    start->id = eptr->ele_id;
				    start->pos = 0;
				    if (!*++mptr) {
					found = 1;
					end->id = eptr->ele_id;
					end->pos = 0;
				    }
				}
			    }
			}
			if (found)
			    break;
			eptr = eptr->next;
		}
	}
	if (found) {
		m_start->id = start->id;
		m_start->pos = start->pos;
		m_end->id = end->id;
		m_end->pos = end->pos;
	}
	if (caseless)
		free(match);
	if (found)
		return(1);
	return(-1);
}

void HTMLDrawBackgroundImage(Widget wid, int x, int y, int width, int height) 
{
	HTMLWidget hw = (HTMLWidget) wid;
	int w_whole = 0;
	int h_whole = 0;
	int start_width = 0;
	int start_height = 0;
	int w_whole_tiles = 0;
	int h_whole_tiles = 0;
	int destx = 0;
	int w_start_offset, h_start_offset;
	int end_width, end_height, w, h, desty;
	int bg_height, bg_width;
	Display *dsp = hw->html.dsp;
	Window win = XtWindow(hw->html.view);

	bg_height = hw->html.bg_height;
	bg_width = hw->html.bg_width;

	if (!hw || (x < 0) || (y < 0) || (width <= 0) || (height <= 0) ||
	    !bg_width || !bg_height || !hw->html.bgmap_SAVE ||
	    !hw->html.bg_image)
                return;

        if (width > hw->html.view_width)
                width = hw->html.view_width - x;
        if (height > hw->html.view_height)
                height = hw->html.view_height - y;

	/*
	 * Figure out the width offset into the bg image.
	 * Figure out the width of the area to draw.
	 * If there is a width offset, index the number of width tiles.
	 * Figure out the height offset into the bg image.
	 * Figure out the height of the area to draw.
	 * If there is a height offset, index the number of height tiles.
	 */
	w_start_offset = (x + hw->html.scroll_x) % bg_width;  
	if (w_start_offset || (!w_start_offset && width < bg_width)) {
		w_whole++;
		start_width = bg_width - w_start_offset;
		if (start_width > width)
			start_width = width;
	}
	h_start_offset = (y + hw->html.scroll_y) % bg_height;
	if (h_start_offset || (!h_start_offset && height < bg_height)) {
		h_whole++;
		start_height = bg_height - h_start_offset;
		if (start_height > height)
			start_height = height;
	}

	/*
	 * Now that we know the width and height of the first area to draw,
	 *   we can compute how many "whole" tiles to draw for entire width
	 *   and the entire height.
	 * We do not bother setting w_whole_tiles or h_whole_tiles to zero
	 *   if the if fails because they are inited to zero.
	 */
	if (width - start_width) {
		w_whole_tiles = (width - start_width) / bg_width;
		w_whole += w_whole_tiles;
	}
	if (height - start_height) {
		h_whole_tiles = (height - start_height) / bg_height;
		h_whole += h_whole_tiles;
	}

	/*
	 * Now we have the numbers to compute the amount of the last tile to
	 * draw.  If there is something to draw, index the "whole" variable.
	 */
	end_width = width - (start_width + (w_whole_tiles * bg_width));
	if (end_width)
		w_whole++;
	end_height = height - (start_height + (h_whole_tiles * bg_height));
	if (end_height)
		h_whole++;
 /*
 printf("x:%d  y:%d  width:%d  height:%d\n",x,y,width,height);
 printf("w_start_offset:%d  h_start_offset:%d\n",w_start_offset,h_start_offset);
 printf("start_width:%d  start_height:%d\n",start_width,start_height);
 printf("end_width:%d  end_height:%d\n",end_width,end_height);
 printf("w_whole_tiles:%d  h_whole_tiles:%d\n",w_whole_tiles,h_whole_tiles);
 printf("w_whole:%d  h_whole:%d\n\n",w_whole,h_whole);
 */
	/*
	 * Now it's time to draw...yippeeeee.
	 *
	 * This could probably stand to be optimized, but I wanted something
	 * that worked first.
	 */
	desty = y;
	for (h = 0; h < h_whole; h++) {
		destx = x;
		for (w = 0; w < w_whole; w++) {
		    XCopyArea(dsp, hw->html.bgmap_SAVE, win, hw->html.drawGC,
			      w_start_offset * !w, h_start_offset * !h,
			      !w ? (start_width ? start_width : bg_width) :
			       ((w + 1) == w_whole ? (end_width ? end_width :
						      bg_width) :
				bg_width),
			      !h ? (start_height ? start_height : bg_height) :
			       ((h + 1) == h_whole ? (end_height ? end_height :
						      bg_height) :
				bg_height),
			      destx, desty);
		    destx += !w ? (start_width ? start_width : bg_width) :
			     bg_width;
		}
		desty += !h ? (start_height ? start_height : bg_height) :
			 bg_height;
	}
	return;
}
 
static Boolean html_accept_focus(Widget w, Time *t)
{
    return True;
}

/* This allows the client to set the focus policy for all the widgets
 * created as children of the html widget
 */
void HTMLSetFocusPolicy(Widget w, int to)
{
    HTMLWidget hw = (HTMLWidget) w;

    if (hw->html.focus_follows_mouse == to) {
        return;
    } else {
        Widget shell = w;

        while (!XtIsTopLevelShell(shell))
	    shell = XtParent(shell);

        hw->html.focus_follows_mouse = to;
        if (to) {
	    XtVaSetValues(shell, XmNkeyboardFocusPolicy, XmPOINTER, NULL);
        } else {
	    XtVaSetValues(shell, XmNkeyboardFocusPolicy, XmEXPLICIT, NULL);
	    /* When we have preference dialog this will have to 
	     * undo all the translations that are currently installed
	     * in the widgets and set the keyboardFocus policy of the
	     * toplevel shell to pointer */
        }
    }
}

/* Timer routine to handle URL refresh reguests in META tags */
void RefreshURL(XtPointer cld, XtIntervalId *id)
{
	WbAnchorCallbackData cbdata;
	XButtonReleasedEvent event;
	RefreshInfo *refresh_info = (RefreshInfo *) cld;
	HTMLWidget hw = refresh_info->hw;

	if (!XtIsRealized((Widget) hw)) {
		if (refresh_info->url)
			free(refresh_info->url);
		free(refresh_info);
		return;
	}

	hw->html.refresh_timer = 0;

	if (refresh_info->refresh != hw->html.refresh_count) {
		if (refresh_info->url)
			free(refresh_info->url);
		free(refresh_info);
		return;
	}
	/* If LIBXMX processing events, requeue the timer */
	if (XmxBlockTimers) {
		hw->html.refresh_timer = XtAppAddTimeOut(app_con, 1000,
					        (XtTimerCallbackProc)RefreshURL,
						(XtPointer)refresh_info);
		return;
	}
	/* Only do first timer to go off if page had multiple refresh URLs */
	hw->html.refresh_count++;

	event.button = Button1;
	event.state = 0;
	cbdata.event = (XEvent *)&event;
	cbdata.href = refresh_info->url;
	cbdata.text = NULL;
	cbdata.title = NULL;
	cbdata.frame = NULL;
	cbdata.refresh = True;
	XtCallCallbackList((Widget) hw, hw->html.anchor_callback,
			   (XtPointer)&cbdata);
	if (refresh_info->url)
		free(refresh_info->url);
	free(refresh_info);
}

void HTMLFreeWidget(HTMLWidget hw)
{
#ifndef DISABLE_TRACE
    if (htmlwTrace) 
        fprintf(stderr, "Doing HTMLFreeWidget\n");
#endif

    if (hw->html.nframe)
        HTMLDestroyFrames(hw);

    if (hw->html.refresh_timer) {
        XtRemoveTimeOut(hw->html.refresh_timer);
        hw->html.refresh_timer = 0;
        if (hw->html.refreshdata->url)
	    free(hw->html.refreshdata->url);
        free(hw->html.refreshdata);
        hw->html.refreshdata = NULL;
    }
    if (hw->html.blink_timer) {
        XtRemoveTimeOut(hw->html.blink_timer);
        hw->html.blink_timer = 0;
        free(hw->html.blinkdata);
        hw->html.blinkdata = NULL;
    }
    if (hw->html.workprocid) {
        XtRemoveWorkProc(hw->html.workprocid);
        hw->html.workprocid = 0;
        free(hw->html.workprocdata);
        hw->html.workprocdata = NULL;
    }
    if (hw->html.map_list) {
	FreeMapList(hw->html.map_list);
	hw->html.map_list = NULL;
    }
    if (hw->html.image_src) {
	HTBTreeAndObject_free(hw->html.image_src);
	hw->html.image_src = NULL;
    }
    FreeMarkUpList(hw->html.html_objects);
    if (hw->html.html_header_objects)
	FreeMarkUpList(hw->html.html_header_objects);
    if (hw->html.html_footer_objects)
	FreeMarkUpList(hw->html.html_footer_objects);
    if (hw->html.html_refresh_objects)
	FreeMarkUpList(hw->html.html_refresh_objects);
    if (hw->html.header_text) {
        free(hw->html.header_text);
        hw->html.header_text = NULL;
    }
    if (hw->html.footer_text) {
        free(hw->html.footer_text);
        hw->html.footer_text = NULL;
    }
    if (hw->html.charset) {
        free(hw->html.charset);
        hw->html.charset = NULL;
    }
    hw->html.html_objects = NULL;

    FreeLineList(hw->html.formatted_elements, hw);
    hw->html.formatted_elements = NULL;

    hw->html.bg_image = 0;
    hw->html.bgmap_SAVE = None;
    hw->html.bgclip_SAVE = None;

    while (hw->html.fontstack) {
        FontRec *fptr = hw->html.fontstack;

        hw->html.fontstack = hw->html.fontstack->next;
        free(fptr);
    }
    hw->html.pushfont_count = 0;
    hw->html.font_save_count = 0;

    FreeColors(hw, hw->core.colormap);
}
