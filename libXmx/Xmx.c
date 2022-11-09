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

/* Copyright (C) 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"

#if defined(MULTINET) && defined(__DECC) && (__VMS_VER >= 70000000)
#define strdup  decc$strdup
#endif /* VMS V7.0 has.  Define it before XmxP.h includes string.h.  GEC */

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

#include "XmxP.h"

#include "../libliteclue/liteclue.h"

#if defined(ultrix) || (defined(VMS) && (!defined(__DECC) || (__VMS_VER < 70000000) || (__DECC_VER <= 50230003))) || defined(NeXT)
extern char *strdup();
#endif /* DEC C V5.2 string.h has for VMS V7.0, GEC */

extern Pixmap dialogError, dialogInformation, dialogQuestion, dialogWarning;

static XmString blank = NULL;
static Display *dsp = NULL;

/* Modal dialog stuff */
static Widget Xmx_shell = NULL;
static Widget InfoDialog = NULL;
static Widget PassDialog = NULL;
static Widget PromptDialog = NULL;
static Widget QuestDialog = NULL;
static Widget InfoParent = NULL;
static Widget PassParent = NULL;
static Widget PromptParent = NULL;
static Widget QuestParent = NULL;

/* ---------------------------- FILE VARIABLES ---------------------------- */

/* Variables accessed through Xmx.h as extern. */
int    Xmx_n = 0;
Arg    Xmx_wargs[XmxMaxNumArgs];
Widget Xmx_w;
int    Xmx_uniqid = 0;

/* Clue stuff */
static Widget liteClue = NULL;
static Widget Xmx_button1 = NULL;
static Widget Xmx_button2 = NULL;
static Widget Xmx_button3 = NULL;
static Widget Xmx_button4 = NULL;
static Widget Xmx_button5 = NULL;
static String Xmx_button1_help = NULL;
static String Xmx_button2_help = NULL;
static String Xmx_button3_help = NULL;
static String Xmx_button4_help = NULL;
static String Xmx_button5_help = NULL;
int XmxBlockTimers = 0;

/* Counter for values returned from XmxMakeNewUniqid. */
static int Xmx_uniqid_counter = 0;

/* Flag for whether or not XmxSetUniqid has ever been called. */
static int Xmx_uniqid_has_been_set = 0;

/* --------------------------- UNIQID FUNCTIONS --------------------------- */

int XmxMakeNewUniqid(void)
{
  return (++Xmx_uniqid_counter);
}

void XmxSetUniqid(int uniqid)
{
  Xmx_uniqid = uniqid;
  Xmx_uniqid_has_been_set = 1;

  return;
}

void XmxZeroUniqid(void)
{
  Xmx_uniqid = 0;
  /* Do NOT reset Xmx_uniqid_has_been_set. */

  return;
}

int XmxExtractUniqid(int cd)
{
  /* Pull the high 16 bits, if uniqid has been set. */
  if (Xmx_uniqid_has_been_set)
      return (cd >> 16);
  return 0;
}

int XmxExtractToken(int cd)
{
  /* Pull the low 16 bits, if uniqid has been set. */
  if (Xmx_uniqid_has_been_set)
      return ((cd << 16) >> 16);
  return cd;
}

/* This function should be called by every Xmx routine
 * when registering a callback or event handler. */
/* This is PRIVATE but accessible to Xmx2.c also. */
int _XmxMakeClientData(int token)
{
  if (Xmx_uniqid_has_been_set)
      return ((Xmx_uniqid << 16) | token);
  return token;
}

/* -------------------------- INTERNAL CALLBACKS -------------------------- */

/* Internal routine to unmanage file selection box on Cancel. */
static XmxCallback(XmxCancelCallback)
{
  XtUnmanageChild(w);
  return;
}


/* --------------------------- CALLBACK SUPPORT --------------------------- */

/* args do nothing */
void XmxAddCallback(Widget w, String name, XtCallbackProc cb, int cb_data)
{
  XtAddCallback(w, name, cb, (XtPointer)_XmxMakeClientData(cb_data));
  return;
}


/* ------------------------ EVENT HANDLER SUPPORT ------------------------- */

void XmxAddEventHandler(Widget w, EventMask event_mask, XtEventHandler proc,
                        int client_data)
{
  XtAddEventHandler(w, event_mask, False, proc,
		    (XtPointer)_XmxMakeClientData(client_data));
  return;
}

void XmxRemoveEventHandler(Widget w, EventMask event_mask, XtEventHandler proc,
                           int client_data)
{
  XtRemoveEventHandler(w, event_mask, False, proc,
		       (XtPointer)_XmxMakeClientData(client_data));
  return;
}


/* ------------------- (nonworking) CENTERING ROUTINES -------------------- */

#ifdef NONWORKING_CENTERING

/* Adapted from Dan Heller's post in comp.windows.x.motif;
 * assumes BulletinBoard with one (centered) child. */
static void XmxCenteringResize(CompositeWidget w, XConfigureEvent *event,
			       String args[], int *num_args)
{
  WidgetList children;
  int width = event->width;
  int height = event->height;
  Dimension w_width, w_height; 
  
  /* Get handle to BulletinBoard's children and marginal spacing */
  XtVaGetValues(w,
                XmNchildren, &children,
                NULL);
  XtVaGetValues(children[0],
                XmNheight, &w_height,
                XmNwidth, &w_width,
                NULL);
  XtVaSetValues(children[0],
                XmNx, width / 2 - w_width / 2,
                XmNy, height / 2 - w_height / 2,
                NULL);
  return;
}

#endif /* NONWORKING_CENTERING */

/* -------------------------- UTILITY FUNCTIONS --------------------------- */

/* Resets args */
void XmxStartup(void)
{
  Xmx_n = 0;
#ifdef MOTIF1_2
  XmRepTypeInstallTearOffModelConverter();
#endif
  return;
}

/* Set Display to use */
void XmxSetDisplay(Display *disp)
{
  dsp = disp;
  return;
}

/* Set an arg */
void XmxSetArg(String arg, XtArgVal val)
{
  XtSetArg(Xmx_wargs[Xmx_n], arg, val);
  Xmx_n++;
  return;
}

void XmxSetValues(Widget w)
{
  if (Xmx_n) {
      XtSetValues(w, Xmx_wargs, Xmx_n);
      Xmx_n = 0;
  }
  return;
}

void XmxGetValues(Widget w)
{
  if (Xmx_n) {
      XtGetValues(w, Xmx_wargs, Xmx_n);
      Xmx_n = 0;
  }
  return;
}

/* args do nothing */
void XmxManageRemanage(Widget w)
{
  if (XtIsManaged(w)) {
      if (XtIsShell(w)) {
          XRaiseWindow(dsp, XtWindow(w));
      } else {
          XMapRaised(dsp, XtWindow(XtParent(w)));
      }
  } else {
      XtManageChild(w);
  }
  return;
}

/* args do nothing */
void XmxSetSensitive(Widget w, int state)
{
  if ((state != XmxSensitive) && (state != XmxUnsensitive)) {
      fprintf(stderr, "Bug in XmxSetSensitive.  State = %d\n", state);
  } else {
      XtSetSensitive(w, (state == XmxSensitive) ? True : False);
  }
  return;
}

/* ------------------------------------------------------------------------ */
/* ---------------- WIDGET CREATION AND HANDLING ROUTINES ----------------- */
/* ------------------------------------------------------------------------ */

/* ----------------------------- PUSHBUTTONS ------------------------------ */

/* args work */
Widget XmxMakePushButton(Widget parent, String name, XtCallbackProc cb,
                         int cb_data)
{
  XmString xmstr = NULL;

  if (name) {
      xmstr = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);
      XmxSetArg(XmNlabelString, (XtArgVal)xmstr);
  }
  Xmx_w = XtCreateManagedWidget("pushbutton", xmPushButtonWidgetClass,
                                parent, Xmx_wargs, Xmx_n);
  XtAddCallback(Xmx_w, XmNactivateCallback, cb, 
                (XtPointer)_XmxMakeClientData(cb_data));
  if (xmstr)
      XmStringFree(xmstr);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeNamedPushButton(Widget parent, String name, String wname, 
                              XtCallbackProc cb, int cb_data)
{
  XmString label;
  
  if (name) {
      label = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);
      XmxSetArg(XmNlabelString, (XtArgVal)label);
  }
  Xmx_w = XtCreateManagedWidget(wname, xmPushButtonWidgetClass,
                                parent, Xmx_wargs, Xmx_n);
  XtAddCallback(Xmx_w, XmNactivateCallback, cb, 
                (XtPointer)_XmxMakeClientData(cb_data));
  if (name)
      XmStringFree(label);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeBlankButton(Widget parent, XtCallbackProc cb, int cb_data)
{
  Xmx_w = XtCreateManagedWidget("blankbutton", xmPushButtonWidgetClass,
                                parent, Xmx_wargs, Xmx_n);
  XtAddCallback(Xmx_w, XmNactivateCallback, cb, 
                (XtPointer)_XmxMakeClientData(cb_data));
  Xmx_n = 0;
  return Xmx_w;
}

/* ------------------------------- COMMAND -------------------------------- */

/* args work */
Widget XmxMakeCommand(Widget parent, String prompt, XtCallbackProc cb,
		      int cb_data)
{
  XmString xmstr = XmxMakeXmstrFromString(prompt);

  XmxSetArg(XmNpromptString, (XtArgVal)xmstr);
  Xmx_w = XtCreateManagedWidget("command", xmCommandWidgetClass, parent,
			        Xmx_wargs, Xmx_n);
  XtAddCallback(Xmx_w, XmNcommandEnteredCallback, cb, 
		(XtPointer)_XmxMakeClientData(cb_data));
  XmStringFree(xmstr);

  Xmx_n = 0;
  return Xmx_w;
}

/* ---------------------------- SCROLLED LIST ----------------------------- */

/* args work */
Widget XmxMakeScrolledList(Widget parent, XtCallbackProc cb, int cb_data)
{
  Xmx_w = XmCreateScrolledList(parent, "scrolled_list", Xmx_wargs, Xmx_n);
  XtManageChild(Xmx_w);
  /* defaultAction gets triggered on double click and sends item
   * along with it... */
  XtAddCallback(Xmx_w, XmNdefaultActionCallback, cb,
                (XtPointer)_XmxMakeClientData(cb_data));
  Xmx_n = 0;
  return Xmx_w;
}

/* ----------------------------- DRAWING AREA ----------------------------- */

/* args work */
Widget XmxMakeDrawingArea(Widget parent, int width, int height)
{
  XmxSetArg(XmNwidth, (XtArgVal)width);
  XmxSetArg(XmNheight, (XtArgVal)height);
  Xmx_w = XtCreateManagedWidget("drawingarea", xmDrawingAreaWidgetClass,
				parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* ------------------------ TOGGLE BUTTONS & BOXES ------------------------ */

/* args work */
Widget XmxMakeRadioBox(Widget parent)
{
  /* Set XmNspacing here to avoid having to play with
   * margins for each togglebutton. */
  XmxSetArg(XmNspacing, (XtArgVal)0);
  XmxSetArg(XmNentryClass, (XtArgVal)xmToggleButtonGadgetClass);
  Xmx_w = XmCreateRadioBox(parent, "radiobox", Xmx_wargs, Xmx_n);
  XtManageChild(Xmx_w);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeOptionBox(Widget parent)
{
  XmxSetArg(XmNentryClass, (XtArgVal)xmToggleButtonGadgetClass);
  XmxSetArg(XmNisHomogeneous, (XtArgVal)True);
  Xmx_w = XtCreateManagedWidget("optionbox", xmRowColumnWidgetClass,
                                parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeToggleButton(Widget parent, String name, XtCallbackProc cb,
                           int cb_data)
{
  XmString label = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNlabelString, (XtArgVal)label);
  XmxSetArg(XmNmarginHeight, (XtArgVal)0);
  Xmx_w = XtCreateManagedWidget("togglebutton", xmToggleButtonWidgetClass,
				parent, Xmx_wargs, Xmx_n);
  /* Used to be XmNarmCallback --- probably not right. */
  if (cb)
      XtAddCallback(Xmx_w, XmNvalueChangedCallback, cb, 
                    (XtPointer)_XmxMakeClientData(cb_data));
  XmStringFree(label);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
void XmxSetToggleButton(Widget button, int set_state)
{
  if ((set_state != XmxSet) && (set_state != XmxUnset)) {
      fprintf(stderr, "Bug in XmxSetToggleButton. set_state = %d\n", set_state);
  } else {
      XmToggleButtonSetState(button,
			     (set_state == XmxSet) ? True : False, False);
  }
  Xmx_n = 0;
  return;
}

/* -------------------------------- SCALES -------------------------------- */

/* args ignored if label is non-NULL, otherwise args work */
Widget XmxMakeScale(Widget parent, XtCallbackProc cb, int cb_data,
                    String label, int min, int max, int start, int dec_adj)
{
  if (label) {
      Xmx_n = 0;
      XmxMakeLabel(parent, label);
  }
  XmxSetArg(XmNminimum, (XtArgVal)min);
  XmxSetArg(XmNmaximum, (XtArgVal)max);
  XmxSetArg(XmNvalue, (XtArgVal)start);
  XmxSetArg(XmNorientation, (XtArgVal)XmHORIZONTAL);
  XmxSetArg(XmNprocessingDirection, (XtArgVal)XmMAX_ON_RIGHT);
  if (dec_adj != XmxNotDisplayed) {
      XmxSetArg(XmNshowValue, (XtArgVal)True);
      XmxSetArg(XmNdecimalPoints, (XtArgVal)dec_adj);
  }
  Xmx_w = XtCreateManagedWidget("scale", xmScaleWidgetClass, parent,
                                Xmx_wargs, Xmx_n);
  XtAddCallback(Xmx_w, XmNvalueChangedCallback, cb, 
		(XtPointer)_XmxMakeClientData(cb_data));
  XtAddCallback(Xmx_w, XmNdragCallback, cb, 
                (XtPointer)_XmxMakeClientData(cb_data));
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
void XmxAdjustScale(Widget scale, int val)
{
  XmxSetArg(XmNvalue, (XtArgVal)val);
  XtSetValues(scale, Xmx_wargs, Xmx_n);

  Xmx_n = 0;
  return;
}


/* args work */
Widget XmxMakeFrame(Widget parent, int shadow)
{
  switch(shadow) {
    case XmxShadowIn:
      XmxSetArg(XmNshadowType, (XtArgVal)XmSHADOW_IN);
      break;
    case XmxShadowOut:
      XmxSetArg(XmNshadowType, (XtArgVal)XmSHADOW_OUT);
      break;
    case XmxShadowEtchedIn:
      XmxSetArg(XmNshadowType, (XtArgVal)XmSHADOW_ETCHED_IN);
      break;
    case XmxShadowEtchedOut:
      XmxSetArg(XmNshadowType, (XtArgVal)XmSHADOW_ETCHED_OUT);
      break;
    default:
      fprintf(stderr, "Bug in XmxMakeFrame.  Shadow = %d\n", shadow);
  }
  Xmx_w = XtCreateManagedWidget("frame", xmFrameWidgetClass,
                                parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* -------------------------------- FORMS --------------------------------- */

/* args work */
Widget XmxMakeForm(Widget parent)
{
  Xmx_w = XtCreateManagedWidget("form", xmFormWidgetClass, parent,
			        Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* args sent to w */
void XmxSetPositions(Widget w, int top, int bottom, int left, int right)
{
  if (top != XmxNoPosition) {
      XmxSetArg(XmNtopAttachment, (XtArgVal)XmATTACH_POSITION);
      XmxSetArg(XmNtopPosition, (XtArgVal)top);
  }
  if (bottom != XmxNoPosition) {
      XmxSetArg(XmNbottomAttachment, (XtArgVal)XmATTACH_POSITION);
      XmxSetArg(XmNbottomPosition, (XtArgVal)bottom);
  }
  if (left != XmxNoPosition) {
      XmxSetArg(XmNleftAttachment, (XtArgVal)XmATTACH_POSITION);
      XmxSetArg(XmNleftPosition, (XtArgVal)left);
  }
  if (right != XmxNoPosition) {
      XmxSetArg(XmNrightAttachment, (XtArgVal)XmATTACH_POSITION);
      XmxSetArg(XmNrightPosition, (XtArgVal)right);
  }

  XmxSetValues(w);

  Xmx_n = 0;
  return;
}

/* args sent to w */
void XmxSetOffsets(Widget w, int top, int bottom, int left, int right)
{
  if (top != XmxNoOffset)
      XmxSetArg(XmNtopOffset, (XtArgVal)top);
  if (bottom != XmxNoOffset)
      XmxSetArg(XmNbottomOffset, (XtArgVal)bottom);
  if (left != XmxNoOffset)
      XmxSetArg(XmNleftOffset, (XtArgVal)left);
  if (right != XmxNoOffset)
      XmxSetArg(XmNrightOffset, (XtArgVal)right);
  
  XmxSetValues(w);

  Xmx_n = 0;
  return;
}

/* args sent to w */
void XmxSetConstraints(Widget w, int top, int bottom, int left, int right,
		       Widget topw, Widget botw, Widget lefw, Widget rigw)
{
  if (top != XmATTACH_NONE) {
      XmxSetArg(XmNtopAttachment, (XtArgVal)top);
      if (topw)
          XmxSetArg(XmNtopWidget, (XtArgVal)topw);
  }
  if (bottom != XmATTACH_NONE) {
      XmxSetArg(XmNbottomAttachment, (XtArgVal)bottom);
      if (botw)
          XmxSetArg(XmNbottomWidget, (XtArgVal)botw);
  }
  if (left != XmATTACH_NONE) {
      XmxSetArg(XmNleftAttachment, (XtArgVal)left);
      if (lefw)
          XmxSetArg(XmNleftWidget, (XtArgVal)lefw);
  }
  if (right != XmATTACH_NONE) {
      XmxSetArg(XmNrightAttachment, (XtArgVal)right);
      if (rigw)
          XmxSetArg(XmNrightWidget, (XtArgVal)rigw);
  }
  
  XmxSetValues(w);

  Xmx_n = 0;
  return;
}

/* ------------------------------ ROWCOLUMNS ------------------------------ */

/* args work */
Widget XmxMakeVerticalRowColumn(Widget parent)
{
  Xmx_w = XtCreateManagedWidget("rowcolumn", xmRowColumnWidgetClass,
                                parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeHorizontalRowColumn(Widget parent)
{
  XmxSetArg(XmNorientation, (XtArgVal)XmHORIZONTAL);
  Xmx_w = XtCreateManagedWidget("rowcolumn", xmRowColumnWidgetClass,
                                parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeNColumnRowColumn(Widget parent, int ncolumns)
{
  XmxSetArg(XmNorientation, (XtArgVal)XmVERTICAL);
  XmxSetArg(XmNpacking, (XtArgVal)XmPACK_COLUMN);
  XmxSetArg(XmNnumColumns, (XtArgVal)ncolumns);
  Xmx_w = XtCreateManagedWidget("rowcolumn", xmRowColumnWidgetClass,
                                parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* --------------------------- BULLETIN BOARDS ---------------------------- */

/* args work */
Widget XmxMakeVerticalBboard(Widget parent)
{
  Xmx_w = XtCreateManagedWidget("bboard", xmBulletinBoardWidgetClass,
                                parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeVerticalBboardWithFont(Widget parent, String fontname)
{
  XmFontList fontlist;
  XFontStruct *font = XLoadQueryFont(dsp, fontname);

  if (font) {
      fontlist = XmFontListCreate(font, XmSTRING_DEFAULT_CHARSET);
      XmxSetArg(XmNbuttonFontList, (XtArgVal)fontlist);
      XmxSetArg(XmNlabelFontList, (XtArgVal)fontlist);
      XmxSetArg(XmNtextFontList, (XtArgVal)fontlist);
      /* fontlist can be freed here, but may be better to leave it cached? */
  }
  XmxSetArg(XmNmarginWidth, (XtArgVal)0);
  XmxSetArg(XmNmarginHeight, (XtArgVal)0);
  Xmx_w = XtCreateManagedWidget("bboard", xmBulletinBoardWidgetClass,
                                parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeHorizontalBboard(Widget parent)
{
  XmxSetArg(XmNorientation, (XtArgVal)XmHORIZONTAL);
  Xmx_w = XtCreateManagedWidget("bboard", xmBulletinBoardWidgetClass,
                                parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* -------------------- (nonworking) CENTERING BBOARD --------------------- */

#ifdef NONWORKING_CENTERING

/* args work */
Widget XmxMakeCenteringBboard(Widget parent, XtAppContext app)
{
  XtActionsRec rec;

  Xmx_w = XtCreateManagedWidget("bboard", xmBulletinBoardWidgetClass,
				parent, Xmx_wargs, Xmx_n);
  /* Does this have to happen more than once? */
  rec.string = "resize";
  rec.proc = XmxCenteringResize;
  XtAppAddActions(app, &rec, 1);

  /* This does, for sure... */
  XtOverrideTranslations(Xmx_w,
			 XtParseTranslationTable("<Configure>: resize()"));
  Xmx_n = 0;
  return Xmx_w;
}

#endif /* NONWORKING_CENTERING */

/* -------------------------------- LABELS -------------------------------- */

/* args work */
void XmxAdjustLabelText(Widget label, String text)
{
  XmString xmstr = XmStringCreateLtoR(text, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNlabelString, (XtArgVal)xmstr);
  XtSetValues(label, Xmx_wargs, Xmx_n);
  XmStringFree(xmstr);

  Xmx_n = 0;
  return;
}

/* args work */
Widget XmxMakeLabel(Widget parent, String name)
{
  XmString xmstr = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNlabelString, (XtArgVal)xmstr);
  Xmx_w = XtCreateManagedWidget("label", xmLabelWidgetClass,
                                parent, Xmx_wargs, Xmx_n);
  XmStringFree(xmstr);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeNamedLabel(Widget parent, String name, String wname)
{
  XmString xmstr = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNlabelString, (XtArgVal)xmstr);
  Xmx_w = XtCreateManagedWidget(wname, xmLabelWidgetClass,
                                parent, Xmx_wargs, Xmx_n);
  XmStringFree(xmstr);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeBlankLabel(Widget parent)
{
  Xmx_w = XtCreateManagedWidget("label", xmLabelWidgetClass,
                                parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;
  return Xmx_w;
}

/* ------------------------------- DIALOGS -------------------------------- */

static XmxCallback(XmxDeleteDialogCallback)
{
  XtDestroyWidget(w);
}

/* args work */
/* Manages and deletes itself */
void XmxMakeErrorDialog(Widget parent, String name, String title)
{
  XmString message = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);
  XmString dialog = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNmessageString, (XtArgVal)message);
  XmxSetArg(XmNdialogTitle, (XtArgVal)dialog);
  XmxSetArg(XmNsymbolPixmap, (XtArgVal)dialogError);

  Xmx_w = XmCreateErrorDialog(parent, "error", Xmx_wargs, Xmx_n);
  XtUnmanageChild(XmMessageBoxGetChild(Xmx_w, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild(XmMessageBoxGetChild(Xmx_w, XmDIALOG_HELP_BUTTON));

  XtAddCallback(Xmx_w, XmNokCallback, XmxDeleteDialogCallback, NULL);

  XtManageChild(Xmx_w);

  XmStringFree(message);
  XmStringFree(dialog);

  Xmx_n = 0;
  return;
}

/* args work */
/* Manages and deletes itself */
void XmxMakeWarningDialog(Widget parent, String name, String title)
{
  XmString message = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);
  XmString dialog = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNmessageString, (XtArgVal)message);
  XmxSetArg(XmNdialogTitle, (XtArgVal)dialog);
  XmxSetArg(XmNsymbolPixmap, (XtArgVal)dialogWarning);

  Xmx_w = XmCreateWarningDialog(parent, "warning", Xmx_wargs, Xmx_n);
  XtUnmanageChild(XmMessageBoxGetChild(Xmx_w, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild(XmMessageBoxGetChild(Xmx_w, XmDIALOG_HELP_BUTTON));

  XtAddCallback(Xmx_w, XmNokCallback, XmxDeleteDialogCallback, NULL);

  XtManageChild(Xmx_w);

  XmStringFree(message);
  XmStringFree(dialog);

  Xmx_n = 0;
  return;
}

/* args work */
/* Manages and deletes itself */
void XmxMakeInfoDialog(Widget parent, String name, String title)
{
  XmString message = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);
  XmString dialog = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNmessageString, (XtArgVal)message);
  XmxSetArg(XmNdialogTitle, (XtArgVal)dialog);
  XmxSetArg(XmNsymbolPixmap, (XtArgVal)dialogInformation);

  Xmx_w = XmCreateInformationDialog(parent, "infozoid", Xmx_wargs, Xmx_n);
  XtUnmanageChild(XmMessageBoxGetChild(Xmx_w, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild(XmMessageBoxGetChild(Xmx_w, XmDIALOG_HELP_BUTTON));

  XtAddCallback(Xmx_w, XmNokCallback, XmxDeleteDialogCallback, NULL);

  XtManageChild(Xmx_w);

  XmStringFree(message);
  XmStringFree(dialog);

  Xmx_n = 0;
  return;
}

/* args work */
/* Does not manage or delete itself */
Widget XmxMakeQuestionDialog(Widget parent, String question, String title,
			     XtCallbackProc cb, int yes_token, int no_token)
{
  XmString message = XmStringCreateLtoR(question, XmSTRING_DEFAULT_CHARSET);
  XmString dialog = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);
  XmString ok = XmStringCreateLtoR("Yes", XmSTRING_DEFAULT_CHARSET);
  XmString cancel = XmStringCreateLtoR("No", XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNmessageString, (XtArgVal)message);
  XmxSetArg(XmNdialogTitle, (XtArgVal)dialog);
  XmxSetArg(XmNokLabelString, (XtArgVal)ok);
  XmxSetArg(XmNcancelLabelString, (XtArgVal)cancel);
  XmxSetArg(XmNsymbolPixmap, (XtArgVal)dialogQuestion);

  Xmx_w = XmCreateQuestionDialog(parent, "question", Xmx_wargs, Xmx_n);
  XtUnmanageChild(XmMessageBoxGetChild(Xmx_w, XmDIALOG_HELP_BUTTON));

  XtAddCallback(Xmx_w, XmNcancelCallback, cb, 
                (XtPointer)_XmxMakeClientData(no_token));
  XtAddCallback(Xmx_w, XmNokCallback, cb, 
                (XtPointer)_XmxMakeClientData(yes_token));

  XmStringFree(message);
  XmStringFree(dialog);
  XmStringFree(ok);
  XmStringFree(cancel);

  Xmx_n = 0;
  return Xmx_w;
}

/* ----------------------------- STRING UTILS ----------------------------- */

/* args do nothing */
XmString XmxMakeXmstrFromFile(String filename)
{
  FILE *f;
  char mstr[81];
  XmString xmstr = NULL;
  XmString str1, str2, sep;

  f = fopen(filename, "r");
  if (!f) {
      fprintf(stderr, "Bug in XmxMakeXmstrFromFile.  Failed to open file.\n");
      return(NULL);
  }
  sep = XmStringSeparatorCreate();

  while (!feof(f)) {
      if (!fgets(mstr, 80, f))
          break;
      mstr[strlen(mstr) - 1] = '\0';
      if (xmstr) {
	  str1 = xmstr;
          xmstr = XmStringConcat(str1, sep);
	  XmStringFree(str1);
      }
      str1 = xmstr;
      str2 = XmStringCreateLtoR(mstr, XmSTRING_DEFAULT_CHARSET);
      xmstr = XmStringConcat(str1, str2);
      XmStringFree(str1);
      XmStringFree(str2);
  }

  XmStringFree(sep);
  fclose(f);
  return xmstr;
}

/* args do nothing */
XmString XmxMakeXmstrFromString(String mstr)
{
  return XmStringCreateLtoR(mstr, XmSTRING_DEFAULT_CHARSET);
}

/* args work */
Widget XmxMakeBboardDialog(Widget parent, String title)
{
  XmString xmstr = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNdialogTitle, (XtArgVal)xmstr);
  XmxSetArg(XmNautoUnmanage, (XtArgVal)False);
  XmxSetArg(XmNmarginWidth, (XtArgVal)0);
  XmxSetArg(XmNmarginHeight, (XtArgVal)0);

  Xmx_w = XmCreateBulletinBoardDialog(parent, "bbdialog", Xmx_wargs, Xmx_n);
  XmStringFree(xmstr);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeFormDialog(Widget parent, String title)
{
  XmString xmstr = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNdialogTitle, (XtArgVal)xmstr);
  XmxSetArg(XmNautoUnmanage, (XtArgVal)False);

  Xmx_w = XmCreateFormDialog(parent, "formdialog", Xmx_wargs, Xmx_n);
  XmStringFree(xmstr);
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeFileSBDialog(Widget parent, String title, String selection_txt,
                           XtCallbackProc cb, int cb_data)
{
  Widget selection_label;
  XmString dialog_title = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);
  XmString label = XmStringCreateLtoR(selection_txt, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNdialogTitle, (XtArgVal)dialog_title);
  /* Can't set width of box with XmNwidth here... why not? */

  /* This will cause the dialog to only resize if needed.  That 
   * way it won't be growing and shrinking all the time... very annoying. */
  XmxSetArg(XmNresizePolicy, (XtArgVal)XmRESIZE_GROW);

  /* Create the FileSelectionBox with OK and Cancel buttons. */
  Xmx_w = XmCreateFileSelectionDialog(parent, "fsb", Xmx_wargs, Xmx_n);
  XtUnmanageChild(XmFileSelectionBoxGetChild(Xmx_w, XmDIALOG_HELP_BUTTON));
  XtAddCallback(Xmx_w, XmNokCallback, cb,
                (XtPointer)_XmxMakeClientData(cb_data));
  XtAddCallback(Xmx_w, XmNcancelCallback,
		(XtCallbackProc)XmxCancelCallback, NULL);

  /* Set selection label to specified selection_txt. */
  Xmx_n = 0;
  selection_label = XmFileSelectionBoxGetChild(Xmx_w, XmDIALOG_SELECTION_LABEL);
  XmxSetArg(XmNlabelString, (XtArgVal)label);
  XtSetValues(selection_label, Xmx_wargs, Xmx_n);

  XmStringFree(dialog_title);
  XmStringFree(label);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeHelpDialog(Widget parent, XmString xmstr, String title)
{
  XmString dialog_title = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNmessageString, (XtArgVal)xmstr);
  XmxSetArg(XmNdialogTitle, (XtArgVal)dialog_title);

  Xmx_w = XmCreateMessageDialog(parent, "helpdialog", Xmx_wargs, Xmx_n);
  XtUnmanageChild(XmMessageBoxGetChild(Xmx_w, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild(XmMessageBoxGetChild(Xmx_w, XmDIALOG_HELP_BUTTON));

  XmStringFree(dialog_title);

  Xmx_n = 0;
  return Xmx_w;
}

/* Boy, this is a hack. */
static XmxCallback(XmxHelpTextCancelCallback)
{
  /* This is highly dependent on the button being four layers
   * below the dialog shell... what a ridiculous hack. */
  XtUnmanageChild(XtParent(XtParent(XtParent(XtParent(w)))));

  return;
}

/* args work */
Widget XmxMakeHelpTextDialog(Widget parent, String str, String title,
			     Widget *text_w)
{
  Widget box, outer_frame, form;
  Widget scr_text, sep, buttons_form;

  /* Create the dialog box. */
  box = XmxMakeFormDialog(parent, title);

  /* Make it 3D. */
  outer_frame = XmxMakeFrame(box, XmxShadowOut);
  XmxSetConstraints
    (outer_frame, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM,
     NULL, NULL, NULL, NULL);

  /* Put form inside that, then frame for text window. */
  form = XmxMakeForm(outer_frame);

  /* Make multiline non-editable text window, with scrollbars. */
  XmxSetArg(XmNscrolledWindowMarginWidth, (XtArgVal)10);
  XmxSetArg(XmNscrolledWindowMarginHeight, (XtArgVal)10);
  XmxSetArg(XmNcursorPositionVisible, (XtArgVal)False);
  XmxSetArg(XmNeditable, (XtArgVal)False);
  XmxSetArg(XmNeditMode, (XtArgVal)XmMULTI_LINE_EDIT);
  XmxSetArg(XmNrows, (XtArgVal)20);
  XmxSetArg(XmNcolumns, (XtArgVal)60);
  XmxSetArg(XmNwordWrap, (XtArgVal)True);
  XmxSetArg(XmNscrollHorizontal, (XtArgVal)False);
  scr_text = XmxMakeScrolledText(form);
  XmTextSetString(scr_text, str);

  /* Separate the text window/frame and the OK button. */
  XmxSetArg(XmNtopOffset, (XtArgVal)10);
  sep = XmxMakeHorizontalSeparator(form);

  /* Make an OK button. */
  buttons_form = XmxMakeFormAndOneButton(form, XmxHelpTextCancelCallback,
					 "OK", 0);
  /* Constraints for form. */
  XmxSetConstraints 
    (XtParent(scr_text), XmATTACH_FORM, XmATTACH_WIDGET, XmATTACH_FORM, 
     XmATTACH_FORM, NULL, sep, NULL, NULL);
  XmxSetConstraints 
    (sep, XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_FORM, XmATTACH_FORM,
     NULL, buttons_form, NULL, NULL);
  XmxSetConstraints 
    (buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM,
     NULL, NULL, NULL, NULL);

  /* Return scr_text in text_w argument. */
  *text_w = scr_text;
  Xmx_w = box;
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
void XmxAdjustHelpDialogText(Widget dialog, XmString message, String title)
{
  XmString dialog_title = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNdialogTitle, (XtArgVal)dialog_title);
  XmxSetArg(XmNmessageString, (XtArgVal)message);
  XtSetValues(dialog, Xmx_wargs, Xmx_n);

  XmStringFree(dialog_title);

  Xmx_n = 0;
  return;
}

/* args work */
void XmxAdjustDialogTitle(Widget dialog, String title)
{
  XmString dialog_title = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNdialogTitle, (XtArgVal)dialog_title);
  XmxSetValues(dialog);

  XmStringFree(dialog_title);

  Xmx_n = 0;
  return;
}

/* ------------------------------ SEPARATORS ------------------------------ */

/* args work */
Widget XmxMakeHorizontalSeparator(Widget parent)
{
  Xmx_w = XmCreateSeparatorGadget(parent, "separator", Xmx_wargs, Xmx_n);
  XtManageChild(Xmx_w);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeHorizontalSpacer(Widget parent)
{
  XmString label = XmStringCreateLtoR(" ", XmSTRING_DEFAULT_CHARSET);

  XmxSetArg(XmNlabelString, (XtArgVal)label);
  Xmx_w = XtCreateManagedWidget("label", xmLabelGadgetClass, parent, 
                                Xmx_wargs, Xmx_n);
  XmStringFree(label);

  Xmx_n = 0;
  return Xmx_w;
}

/* args do nothing */
Widget XmxMakeHorizontalBoundary(Widget parent)
{
  /* To avoid confusion, nullify preloaded resources first. */
  Xmx_n = 0;
  XmxMakeHorizontalSpacer(parent);
  XmxMakeHorizontalSeparator(parent);
  XmxMakeHorizontalSpacer(parent);

  return Xmx_w;
}

/* ------------------------- TEXT & SCROLLED TEXT ------------------------- */

/* args work */
Widget XmxMakeScrolledText(Widget parent)
{
  Xmx_w = XmCreateScrolledText(parent, "scrolledtext", Xmx_wargs, Xmx_n);
  XtManageChild(Xmx_w);

  /* Remember this returns the Text Widget, NOT the ScrolledWindow Widget, 
   * which is what needs to be tied into a form.  Use XtParent to get the
   * actual ScrolledWindow. */
  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeText(Widget parent)
{
  Xmx_w = XmCreateText(parent, "text", Xmx_wargs, Xmx_n);
  XtManageChild(Xmx_w);

  Xmx_n = 0;
  return Xmx_w;
}

/* args work */
Widget XmxMakeTextField(Widget parent)
{
  Xmx_w = XmCreateTextField(parent, "textfield", Xmx_wargs, Xmx_n);
  XtManageChild(Xmx_w);

  Xmx_n = 0;
  return Xmx_w;
}

/* args do nothing */
void XmxTextSetString(Widget text, String str)
{
  XmTextSetString(text, str);
  XmTextShowPosition(text, 0);
  return;
}

/* Insert a sting into a text widget */
void XmxTextInsertString(Widget text, String str)
{
  XmTextInsert(text, XmTextGetInsertionPosition(text), str);
  XmTextShowPosition(text, 0);
  return;
}

/* args do nothing */
String XmxTextGetString(Widget text)
{
  return XmTextGetString(text);
}

/* args do nothing */
void XmxAddCallbackToText(Widget text, XtCallbackProc cb, int cb_data)
{
  XtAddCallback(text, XmNactivateCallback, cb, 
                (XtPointer)_XmxMakeClientData(cb_data));
  return;
}


#if 0

/* --------------------------- DRAWING VOLUMES ---------------------------- */

#ifdef __sgi
/* args work */
Widget XmxMakeDrawingVolume(Widget parent, int width, int height,
			    GLXconfig *glxConfig,
			    XtCallbackProc redraw_cb, XtCallbackProc resize_cb,
			    XtCallbackProc ginit_cb)
{
  XmxSetArg(XmNwidth, (XtArgVal)width);
  XmxSetArg(XmNheight, (XtArgVal)height);
  XmxSetArg(GlxNglxConfig, (XtArgVal)glxConfig);

  Xmx_w = GlxCreateMDraw(parent, "drawingvolume", Xmx_wargs, Xmx_n);
  XtManageChild(Xmx_w);

  XtAddCallback(Xmx_w, GlxNexposeCallback, redraw_cb, 
                (XtPointer)_XmxMakeClientData(0));
  XtAddCallback(Xmx_w, GlxNresizeCallback, resize_cb, 
                (XtPointer)_XmxMakeClientData(0));
  XtAddCallback(Xmx_w, GlxNginitCallback, ginit_cb, 
                (XtPointer)_XmxMakeClientData(0));
  Xmx_n = 0;
  return Xmx_w;
}

void XmxInstallColormaps(Widget toplevel, Widget glw)
{
  Window windows[2];

  windows[0] = XtWindow(glw);
  windows[1] = XtWindow(toplevel);
  XSetWMColormapWindows(dsp, XtWindow(toplevel), windows, 2);
  return;
}

void XmxInstallColormapsWithOverlay(Widget toplevel, Widget glw)
{
  Window windows[5];
  Window overlay, popup, underlay;
  Arg args[5];
  register int i = 0;

  XtSetArg(args[0], GlxNoverlayWindow, &overlay);
  XtSetArg(args[1], GlxNpopupWindow, &popup);
  XtSetArg(args[2], GlxNunderlayWindow, &underlay);
  XtGetValues(glw, args, 3);
  if (overlay)
      windows[i++] = overlay;
  if (popup)
      windows[i++] = popup;
  if (underlay)
      windows[i++] = underlay;
  windows[i++] = XtWindow(glw);
  windows[i++] = XtWindow(toplevel);
  XSetWMColormapWindows(dsp, XtWindow(toplevel), windows, i);
  
  return;
}

void XmxWinset(Widget w)
{
  GLXwinset(dsp, XtWindow(w));
  return;
}
#endif /* __sgi */


#ifdef _IBMR2
/* args work */
Widget XmxMakeDrawingVolume(Widget parent, int width, int height,
			    XtCallbackProc redraw_cb, XtCallbackProc resize_cb,
			    XtCallbackProc ginit_cb)
{
  XmxSetArg(XmNwidth, (XtArgVal)width);
  XmxSetArg(XmNheight, (XtArgVal)height);

  Xmx_w = XtCreateWidget("drawingvolume", glibWidgetClass, parent,
			 Xmx_wargs, Xmx_n);
  XtManageChild(Xmx_w);

  XtAddCallback(Xmx_w, XglNexposeCallback, redraw_cb, 
                (XtPointer)_XmxMakeClientData(0));
  XtAddCallback(Xmx_w, XglNresizeCallback, resize_cb, 
                (XtPointer)_XmxMakeClientData(0));
  XtAddCallback(Xmx_w, XglNgconfigCallback, ginit_cb, 
                (XtPointer)_XmxMakeClientData(0));
  Xmx_n = 0;
  return Xmx_w;
}

void XmxInstallColormaps(Widget toplevel, Widget glw)
{
  return;
}

void XmxWinset(Widget w)
{
  GlWinsetWidget(w);
  return;
}
#endif /* _IBMR2 */

#endif /* if 0 */

/* ----------------------------- BITMAP UTILS ----------------------------- */

/* args ignored and reset */
void XmxApplyBitmapToLabelWidget(Widget label, String data,
				 unsigned int width, unsigned int height)
{
  Pixel fg, bg;
  Pixmap pix;

  Xmx_n = 0;
  XmxSetArg(XmNforeground, (XtArgVal)&fg);
  XmxSetArg(XmNbackground, (XtArgVal)&bg);
  XtGetValues(label, Xmx_wargs, Xmx_n);
  Xmx_n = 0;

  pix = XCreatePixmapFromBitmapData(dsp, DefaultRootWindow(dsp), data, width,
			     height, fg, bg,
			     DefaultDepthOfScreen(DefaultScreenOfDisplay(dsp)));
  XmxSetArg(XmNlabelPixmap, (XtArgVal)pix);
  XmxSetArg(XmNlabelType, (XtArgVal)XmPIXMAP);
  XmxSetValues(label);

  Xmx_n = 0;
  return;
}

/* args ignored and reset */
Pixmap XmxCreatePixmapFromBitmap(Widget label, String data,
				 unsigned int width, unsigned int height)
{
  Pixel fg, bg;
  Pixmap pix;

  Xmx_n = 0;
  XmxSetArg(XmNforeground, (XtArgVal)&fg);
  XmxSetArg(XmNbackground, (XtArgVal)&bg);
  XtGetValues(label, Xmx_wargs, Xmx_n);
  Xmx_n = 0;

  pix = XCreatePixmapFromBitmapData(dsp, DefaultRootWindow(dsp), data, width,
			     height, fg, bg,
			     DefaultDepthOfScreen(DefaultScreenOfDisplay(dsp)));
  return pix;
}

/* args used */
void XmxApplyPixmapToLabelWidget(Widget label, Pixmap pix)
{
  XmxSetArg(XmNlabelPixmap, (XtArgVal)pix);
  XmxSetArg(XmNlabelType, (XtArgVal)XmPIXMAP);
  XmxSetValues(label);

  Xmx_n = 0;
  return;
}

/* ------------------------ DIALOG CONTROL BUTTONS ------------------------ */

/* args apply to form */
Widget XmxMakeFormAndOneButton(Widget parent, XtCallbackProc cb, 
                               String name1, int cb_data1)
{
  Widget form, button1;

  XmxSetArg(XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg(XmNfractionBase, (XtArgVal)3);
  form = XmxMakeForm(parent);

  Xmx_button1 = button1 = XmxMakePushButton(form, name1, cb, cb_data1);

  XmxSetConstraints
    (button1, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetPositions(button1, XmxNoPosition, XmxNoPosition, 1, 2);
  XmxSetOffsets(button1, XmxNoOffset, XmxNoOffset, 8, 8);
  
  Xmx_n = 0;
  Xmx_w = form;
  return Xmx_w;
}

/* args apply to form */
Widget XmxMakeFormAndTwoButtons(Widget parent, XtCallbackProc cb,
				String name1, String name2,
				int cb_data1, int cb_data2)
{
  Widget form, button1, button2;

  XmxSetArg(XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg(XmNfractionBase, (XtArgVal)2);
  form = XmxMakeForm(parent);

  Xmx_button1 = button1 = XmxMakePushButton(form, name1, cb, cb_data1);
  Xmx_button2 = button2 = XmxMakePushButton(form, name2, cb, cb_data2);

  XmxSetConstraints
    (button1, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (button2, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetPositions(button1, XmxNoPosition, XmxNoPosition, 0, 1);
  XmxSetPositions(button2, XmxNoPosition, XmxNoPosition, 1, 2);
  XmxSetOffsets(button1, XmxNoOffset, XmxNoOffset, 8, 4);
  XmxSetOffsets(button2, XmxNoOffset, XmxNoOffset, 4, 8);
  
  Xmx_n = 0;
  Xmx_w = form;
  return Xmx_w;
}

/* args apply to form */
Widget XmxMakeFormAndTwoButtonsTight(Widget parent, XtCallbackProc cb,
				     String name1, String name2,
				     int cb_data1, int cb_data2)
{
  Widget form, button1, button2;

  XmxSetArg(XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg(XmNfractionBase, (XtArgVal)5);
  form = XmxMakeForm(parent);

  Xmx_button1 = button1 = XmxMakePushButton(form, name1, cb, cb_data1);
  Xmx_button2 = button2 = XmxMakePushButton(form, name2, cb, cb_data2);

  XmxSetConstraints
    (button1, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (button2, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetPositions(button1, XmxNoPosition, XmxNoPosition, 1, 2);
  XmxSetPositions(button2, XmxNoPosition, XmxNoPosition, 3, 4);
  XmxSetOffsets(button1, XmxNoOffset, XmxNoOffset, 8, 4);
  XmxSetOffsets(button2, XmxNoOffset, XmxNoOffset, 4, 8);
  
  Xmx_n = 0;
  Xmx_w = form;
  return Xmx_w;
}

/* args apply to form */
Widget XmxMakeFormAndThreeButtonsTight(Widget parent, XtCallbackProc cb,
   				       String name1, String name2, String name3,
				       int cb_data1, int cb_data2, int cb_data3)
{
  Widget form, button1, button2, button3;

  XmxSetArg(XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg(XmNfractionBase, (XtArgVal)7);
  form = XmxMakeForm(parent);

  Xmx_button1 = button1 = XmxMakePushButton(form, name1, cb, cb_data1);
  Xmx_button2 = button2 = XmxMakePushButton(form, name2, cb, cb_data2);
  Xmx_button3 = button3 = XmxMakePushButton(form, name3, cb, cb_data3);

  XmxSetConstraints
    (button1, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (button2, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (button3, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetPositions(button1, XmxNoPosition, XmxNoPosition, 1, 2);
  XmxSetPositions(button2, XmxNoPosition, XmxNoPosition, 3, 4);
  XmxSetPositions(button3, XmxNoPosition, XmxNoPosition, 5, 6);
  XmxSetOffsets(button1, XmxNoOffset, XmxNoOffset, 8, 4);
  XmxSetOffsets(button2, XmxNoOffset, XmxNoOffset, 4, 4);
  XmxSetOffsets(button3, XmxNoOffset, XmxNoOffset, 4, 8);
  
  Xmx_n = 0;
  Xmx_w = form;
  return Xmx_w;
}

/* args apply to form */
Widget XmxMakeFormAndThreeButtons(Widget parent, XtCallbackProc cb,
				  String name1, String name2, String name3,
				  int cb_data1, int cb_data2, int cb_data3)
{
  Widget form, button1, button2, button3;

  XmxSetArg(XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg(XmNfractionBase, (XtArgVal)3);
  form = XmxMakeForm(parent);

  Xmx_button1 = button1 = XmxMakePushButton(form, name1, cb, cb_data1);
  Xmx_button2 = button2 = XmxMakePushButton(form, name2, cb, cb_data2);
  Xmx_button3 = button3 = XmxMakePushButton(form, name3, cb, cb_data3);

  XmxSetConstraints
    (button1, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (button2, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (button3, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetPositions(button1, XmxNoPosition, XmxNoPosition, 0, 1);
  XmxSetPositions(button2, XmxNoPosition, XmxNoPosition, 1, 2);
  XmxSetPositions(button3, XmxNoPosition, XmxNoPosition, 2, 3);
  XmxSetOffsets(button1, XmxNoOffset, XmxNoOffset, 8, 4);
  XmxSetOffsets(button2, XmxNoOffset, XmxNoOffset, 4, 4);
  XmxSetOffsets(button3, XmxNoOffset, XmxNoOffset, 4, 8);
  
  Xmx_n = 0;
  Xmx_w = form;
  return Xmx_w;
}

/* args apply to form */
Widget XmxMakeFormAndFourButtons(Widget parent, XtCallbackProc cb,
			 String name1, String name2, String name3, String name4,
			 int cb_data1, int cb_data2, int cb_data3, int cb_data4)
{
  Widget form, button1, button2, button3, button4;

  XmxSetArg(XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg(XmNfractionBase, (XtArgVal)4);
  form = XmxMakeForm(parent);

  Xmx_button1 = button1 = XmxMakePushButton(form, name1, cb, cb_data1);
  Xmx_button2 = button2 = XmxMakePushButton(form, name2, cb, cb_data2);
  Xmx_button3 = button3 = XmxMakePushButton(form, name3, cb, cb_data3);
  Xmx_button4 = button4 = XmxMakePushButton(form, name4, cb, cb_data4);

  XmxSetConstraints
    (button1, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (button2, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (button3, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (button4, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetPositions(button1, XmxNoPosition, XmxNoPosition, 0, 1);
  XmxSetPositions(button2, XmxNoPosition, XmxNoPosition, 1, 2);
  XmxSetPositions(button3, XmxNoPosition, XmxNoPosition, 2, 3);
  XmxSetPositions(button4, XmxNoPosition, XmxNoPosition, 3, 4);
  XmxSetOffsets(button1, XmxNoOffset, XmxNoOffset, 8, 4);
  XmxSetOffsets(button2, XmxNoOffset, XmxNoOffset, 4, 4);
  XmxSetOffsets(button3, XmxNoOffset, XmxNoOffset, 4, 4);
  XmxSetOffsets(button4, XmxNoOffset, XmxNoOffset, 4, 8);
  
  Xmx_n = 0;
  Xmx_w = form;
  return Xmx_w;
}

/* args apply to form */
Widget XmxMakeFormAndFiveButtons(Widget parent, XtCallbackProc cb,
	   String name1, String name2, String name3, String name4, String name5,
	   int cb_data1, int cb_data2, int cb_data3, int cb_data4, int cb_data5)
{
  Widget form, button1, button2, button3, button4, button5;

  XmxSetArg(XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg(XmNverticalSpacing, (XtArgVal)8);
  XmxSetArg(XmNfractionBase, (XtArgVal)5);
  form = XmxMakeForm(parent);

  Xmx_button1 = button1 = XmxMakePushButton(form, name1, cb, cb_data1);
  Xmx_button2 = button2 = XmxMakePushButton(form, name2, cb, cb_data2);
  Xmx_button3 = button3 = XmxMakePushButton(form, name3, cb, cb_data3);
  Xmx_button4 = button4 = XmxMakePushButton(form, name4, cb, cb_data4);
  Xmx_button5 = button5 = XmxMakePushButton(form, name5, cb, cb_data5);

  XmxSetConstraints
    (button1, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (button2, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (button3, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (button4, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetConstraints
    (button5, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_NONE,
     NULL, NULL, NULL, NULL);
  XmxSetPositions(button1, XmxNoPosition, XmxNoPosition, 0, 1);
  XmxSetPositions(button2, XmxNoPosition, XmxNoPosition, 1, 2);
  XmxSetPositions(button3, XmxNoPosition, XmxNoPosition, 2, 3);
  XmxSetPositions(button4, XmxNoPosition, XmxNoPosition, 3, 4);
  XmxSetPositions(button5, XmxNoPosition, XmxNoPosition, 4, 5);
  XmxSetOffsets(button1, XmxNoOffset, XmxNoOffset, 8, 4);
  XmxSetOffsets(button2, XmxNoOffset, XmxNoOffset, 4, 4);
  XmxSetOffsets(button3, XmxNoOffset, XmxNoOffset, 4, 4);
  XmxSetOffsets(button4, XmxNoOffset, XmxNoOffset, 4, 4);
  XmxSetOffsets(button5, XmxNoOffset, XmxNoOffset, 4, 8);

  Xmx_n = 0;
  Xmx_w = form;
  return Xmx_w;
}

/* ---------------------------MODAL DIALOGS -------------------------------- */

Widget XmxInitModalDialogs(Widget toplevel)
{
  if (Xmx_shell)
      return Xmx_shell;
  if (!toplevel)
      return NULL;

  Xmx_shell = XtCreatePopupShell("xmx_shell", topLevelShellWidgetClass,
				 toplevel, NULL, 0);
  return Xmx_shell;
}

/* Delete cached modal dialogs */
void XmxResetModalDialogs()
{
  if (InfoDialog) {
      XtDestroyWidget(InfoDialog);
      InfoDialog = NULL;
      InfoParent = NULL;
  }
  if (QuestDialog) {
      XtDestroyWidget(QuestDialog);
      QuestDialog = NULL;
      QuestParent = NULL;
  }
  if (PassDialog) {
      XtDestroyWidget(PassDialog);
      PassDialog = NULL;
      PassParent = NULL;
  }
  if (PromptDialog) {
      XtDestroyWidget(PromptDialog);
      PromptDialog = NULL;
      PromptParent = NULL;
  }
}

static void XmxYesOrNoResponse(Widget w, int *answer, XmAnyCallbackStruct *cbs)
{
  if (cbs->reason == XmCR_OK) {
      *answer = 1;
  } else if (cbs->reason == XmCR_CANCEL) {
      *answer = 0;
  }
}

int XmxModalYesOrNo(Widget parent, XtAppContext app,
                    char *questionstr, char *yesstr, char *nostr)
{
  Widget shell;
  XmString question, yes, no;
  static int answer;

  answer = -1;

  if (!Xmx_shell) {
      if (QuestDialog && (parent != QuestParent)) {
	  XtDestroyWidget(QuestDialog);
	  QuestDialog = NULL;
	  QuestParent = NULL;
      }
      shell = parent;
  } else {
      shell = Xmx_shell;
  }

  if (!QuestDialog) {
      XmString title = XmStringCreateLtoR("Prompt", XmSTRING_DEFAULT_CHARSET);

      XmxSetArg(XmNdialogTitle, (XtArgVal)title);
      XmxSetArg(XmNsymbolPixmap, (XtArgVal)dialogQuestion);
      XmxSetArg(XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
      XmxSetArg(XmNmwmDecorations, MWM_DECOR_BORDER | MWM_DECOR_TITLE);

      QuestDialog = XmCreateQuestionDialog(shell, "modal_dialog",
					   Xmx_wargs, Xmx_n);
      Xmx_n = 0;
  
      XtUnmanageChild(XmMessageBoxGetChild(QuestDialog, XmDIALOG_HELP_BUTTON));
      XtAddCallback(QuestDialog, XmNokCallback, 
                    (XtCallbackProc)XmxYesOrNoResponse, &answer);
      XtAddCallback(QuestDialog, XmNcancelCallback, 
                    (XtCallbackProc)XmxYesOrNoResponse, &answer);
      XmStringFree(title);
  }
  question = XmStringCreateLtoR(questionstr, XmSTRING_DEFAULT_CHARSET);
  yes = XmStringCreateLtoR(yesstr, XmSTRING_DEFAULT_CHARSET);
  no = XmStringCreateLtoR(nostr, XmSTRING_DEFAULT_CHARSET);
  
  XmxSetArg(XmNmessageString, (XtArgVal)question);
  XmxSetArg(XmNokLabelString, (XtArgVal)yes);
  XmxSetArg(XmNcancelLabelString, (XtArgVal)no);
  XmxSetValues(QuestDialog);

  XtManageChild(QuestDialog);

  XmxBlockTimers = 1;
  while (answer == -1) {
      XtAppProcessEvent(app, XtIMAll);
      XSync(dsp, 0);
  }
  XmxBlockTimers = 0;

  XtUnmanageChild(QuestDialog);
  XSync(dsp, 0);
  XmUpdateDisplay(QuestDialog);

  XmStringFree(question);
  XmStringFree(yes);
  XmStringFree(no);

  return answer;
}

#define XMX_NO_ANSWER "-*-no answer, dammit, but Xmx rules, yo yo yo-*-"

static void XmxActivate(Widget w, char **answer,
			XmSelectionBoxCallbackStruct *cbs)
{
  *answer = XMX_NO_ANSWER;
}

static void XmxPromptForStringResponse(Widget w, char **answer, 
                                       XmSelectionBoxCallbackStruct *cbs)
{
  if (!XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, answer))
      *answer = XMX_NO_ANSWER;
}

static void XmxPromptForStringCancel(Widget w, char **answer, 
                                     XmSelectionBoxCallbackStruct *cbs)
{
  *answer = XMX_NO_ANSWER;
}

static void XmxMakeDialogWait(Widget parent, XtAppContext app, char *infostr,
			      char *titlestr, char *yesstr, Pixmap pix)
{
  Widget shell;
  XmString title, info, yes;
  static char *answer;

  answer = NULL;

  if (!Xmx_shell) {
      if (InfoDialog && (parent != InfoParent)) {
	  XtDestroyWidget(InfoDialog);
	  InfoDialog = NULL;
	  InfoParent = parent; 
      }
      shell = parent;
  } else {
      shell = Xmx_shell;
  }

  if (!InfoDialog) {
      XmxSetArg(XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
      XmxSetArg(XmNmwmDecorations, MWM_DECOR_BORDER | MWM_DECOR_TITLE);

      InfoDialog = XmCreateInformationDialog(shell, "information_dialog",
				             Xmx_wargs, Xmx_n);
      Xmx_n = 0;

      XtUnmanageChild(XmMessageBoxGetChild(InfoDialog, XmDIALOG_HELP_BUTTON));
      XtUnmanageChild(XmMessageBoxGetChild(InfoDialog, XmDIALOG_CANCEL_BUTTON));
      XtAddCallback(InfoDialog, XmNokCallback,
		    (XtCallbackProc)XmxActivate, &answer);
  }
  title = XmStringCreateLtoR(titlestr, XmSTRING_DEFAULT_CHARSET);
  info = XmStringCreateLtoR(infostr, XmSTRING_DEFAULT_CHARSET);
  yes = XmStringCreateLtoR(yesstr, XmSTRING_DEFAULT_CHARSET);
  XmxSetArg(XmNdialogTitle, (XtArgVal)title);
  XmxSetArg(XmNmessageString, (XtArgVal)info);
  XmxSetArg(XmNokLabelString, (XtArgVal)yes);
  XmxSetArg(XmNsymbolPixmap, (XtArgVal)pix);
  XmxSetValues(InfoDialog);

  XtManageChild(InfoDialog);

  XmxBlockTimers = 1;
  while (!answer) {
      XtAppProcessEvent(app, XtIMAll);
      XSync(dsp, 0);
  }
  XmxBlockTimers = 0;

  XtUnmanageChild(InfoDialog);
  XSync(dsp, 0);
  XmUpdateDisplay(InfoDialog);

  XmStringFree(title);
  XmStringFree(info);
  XmStringFree(yes);

  return;
}

void XmxMakeInfoDialogWait(Widget parent, XtAppContext app, 
                           char *infostr, char *titlestr, char *yesstr)
{
  XmxMakeDialogWait(parent, app, infostr, titlestr, yesstr, dialogInformation);
}


void XmxMakeWarningDialogWait(Widget parent, XtAppContext app,
                              char *infostr, char *titlestr, char *yesstr)
{
  XmxMakeDialogWait(parent, app, infostr, titlestr, yesstr, dialogWarning);
}


void XmxMakeErrorDialogWait(Widget parent, XtAppContext app,
                            char *infostr, char *titlestr, char *yesstr)
{
  XmxMakeDialogWait(parent, app, infostr, titlestr, yesstr, dialogError);
}

char *XmxModalPromptForString(Widget parent, XtAppContext app,
                              char *questionstr, char *yesstr, char *nostr)
{
  Widget shell;
  XmString question, yes, no;
  static char *answer;

  answer = NULL;

  if (!Xmx_shell) {
      if (PromptDialog && (parent != PromptParent)) {
	  XtDestroyWidget(PromptDialog);
	  PromptDialog = NULL;
	  PromptParent = NULL;
      }
      shell = parent;
  } else {
      shell = Xmx_shell;
  }

  if (!PromptDialog) {
      XmString title = XmStringCreateLtoR("Prompt", XmSTRING_DEFAULT_CHARSET);

      if (!blank)
          blank = XmStringCreateSimple("");
      XmxSetArg(XmNdialogTitle, (XtArgVal)title);
      XmxSetArg(XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
      XmxSetArg(XmNmwmDecorations, MWM_DECOR_BORDER | MWM_DECOR_TITLE);

      PromptDialog = XmCreatePromptDialog(shell, "modal_dialog",
					  Xmx_wargs, Xmx_n);
      Xmx_n = 0;
      
      XtUnmanageChild(XmSelectionBoxGetChild(PromptDialog,
					     XmDIALOG_HELP_BUTTON));
      XtAddCallback(PromptDialog, XmNokCallback, 
                    (XtCallbackProc)XmxPromptForStringResponse, &answer);
      XtAddCallback(PromptDialog, XmNcancelCallback, 
                    (XtCallbackProc)XmxPromptForStringCancel, &answer);
      XmStringFree(title);
  }

  question = XmStringCreateLtoR(questionstr, XmSTRING_DEFAULT_CHARSET);
  yes = XmStringCreateLtoR(yesstr, XmSTRING_DEFAULT_CHARSET);
  no = XmStringCreateLtoR(nostr, XmSTRING_DEFAULT_CHARSET);
  
  /* Must blank it out when reused */
  XmxSetArg(XmNtextString, (XtArgVal)blank);
  XmxSetArg(XmNselectionLabelString, (XtArgVal)question);
  XmxSetArg(XmNokLabelString, (XtArgVal)yes);
  XmxSetArg(XmNcancelLabelString, (XtArgVal)no);
  XmxSetValues(PromptDialog);
  
  XtManageChild(PromptDialog);

  XmxBlockTimers = 1;
  while (!answer) {
      XtAppProcessEvent(app, XtIMAll);
      XSync(dsp, 0);
  }
  XmxBlockTimers = 0;

  XtUnmanageChild(PromptDialog);
  XSync(dsp, 0);
  XmUpdateDisplay(PromptDialog);

  XmStringFree(question);
  XmStringFree(yes);
  XmStringFree(no);

  if (!strcmp(answer, XMX_NO_ANSWER)) {
      return NULL;
  } else {
      return answer;
  }
}

static char *xmx_passwd = NULL;

static void XmxPromptForPasswordResponse(Widget w, char **answer, 
                                         XmSelectionBoxCallbackStruct *cbs)
{
  if (!XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, answer))
      *answer = XMX_NO_ANSWER;
}

static void XmxPromptForPasswordCancel(Widget w, char **answer, 
                                       XmSelectionBoxCallbackStruct *cbs)
{
  *answer = XMX_NO_ANSWER;
}

static void XmxPromptForPasswordVerify(Widget text_w, XtPointer unused, 
                                       XmTextVerifyCallbackStruct *cbs)
{
  if (cbs->reason != XmCR_MODIFYING_TEXT_VALUE)
      return;

  if (!cbs->text->ptr) {  /* Backspace */
      cbs->doit = True;
      if (xmx_passwd && *xmx_passwd) {
	  char *tptr;
	  int start;
	  int len = strlen(xmx_passwd);

	  /* Find the start of the delete */
	  if (cbs->startPos < len) {
	      start = cbs->startPos;
	  } else {
	      start = len - 1;
	  }
	  /* Move up stuff after the delete */
	  if (cbs->endPos > len) {
	      tptr = &xmx_passwd[len];
	  } else {
	      tptr = &xmx_passwd[cbs->endPos];
	  }
	  xmx_passwd[start] = '\0';
	  strcat(xmx_passwd, tptr);
      }
  } else if (cbs->text->length >= 1) {
      int i;

      if (!xmx_passwd) {
	  xmx_passwd = XtMalloc(cbs->text->length + 1);
	  strncpy(xmx_passwd, cbs->text->ptr, cbs->text->length);
	  xmx_passwd[cbs->text->length] = '\0';
      } else {
	  char *tptr, *new;
	  char tchar;
	  int start;
	  int len = strlen(xmx_passwd);

	  /* Find the start of the delete */
	  if (cbs->startPos < len) {
	      start = cbs->startPos;
	  } else {
	      start = len;
	  }
	  tptr = &xmx_passwd[start];
	  tchar = *tptr;
	  *tptr = '\0';
	  new = XtMalloc(len + cbs->text->length + 1);
	  strcpy(new, xmx_passwd);
	  strncat(new, cbs->text->ptr, cbs->text->length);
	  new[start + cbs->text->length] = '\0';
	  *tptr = tchar;
	  strcat(new, tptr);
	  XtFree(xmx_passwd);
	  xmx_passwd = new;
      }
      cbs->doit = True;
      /* Make a '*' show up instead of what they typed */
      for (i = 0; i < cbs->text->length; i++)
	  cbs->text->ptr[i] = '*';
  }
}

char *XmxModalPromptForPassword(Widget parent, XtAppContext app, 
                                char *questionstr, char *yesstr, char *nostr)
{
  Widget shell;
  XmString question, yes, no;
  static char *answer;

  answer = NULL;
  xmx_passwd = NULL;

  if (!Xmx_shell) {
      if (PassDialog && (parent != PassParent)) {
	  XtDestroyWidget(PassDialog);
	  PassDialog = NULL;
	  PassParent = NULL;
      }
      shell = parent;
  } else {
      shell = Xmx_shell;
  }

  if (!PassDialog) {
      XmString title = XmStringCreateLtoR("Prompt", XmSTRING_DEFAULT_CHARSET);
  
      if (!blank)
          blank = XmStringCreateSimple("");
      XmxSetArg(XmNdialogTitle, (XtArgVal)title);
      XmxSetArg(XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
      XmxSetArg(XmNmwmDecorations, MWM_DECOR_BORDER | MWM_DECOR_TITLE);

      PassDialog = XmCreatePromptDialog(shell, "modal_dialog",
					Xmx_wargs, Xmx_n);
      Xmx_n = 0;
  
      XtUnmanageChild(XmSelectionBoxGetChild(PassDialog, XmDIALOG_HELP_BUTTON));
      XtAddCallback(XmSelectionBoxGetChild(PassDialog, XmDIALOG_TEXT),
     		    XmNmodifyVerifyCallback,
		    (XtCallbackProc)XmxPromptForPasswordVerify, NULL);
      XtAddCallback(PassDialog, XmNokCallback, 
                    (XtCallbackProc)XmxPromptForPasswordResponse, &answer);
      XtAddCallback(PassDialog, XmNcancelCallback, 
                    (XtCallbackProc)XmxPromptForPasswordCancel, &answer);
      XmStringFree(title);
  }

  question = XmStringCreateLtoR(questionstr, XmSTRING_DEFAULT_CHARSET);
  yes = XmStringCreateLtoR(yesstr, XmSTRING_DEFAULT_CHARSET);
  no = XmStringCreateLtoR(nostr, XmSTRING_DEFAULT_CHARSET);

  /* Must blank it out when reused */
  XmxSetArg(XmNtextString, (XtArgVal)blank);
  XmxSetArg(XmNselectionLabelString, (XtArgVal)question);
  XmxSetArg(XmNokLabelString, (XtArgVal)yes);
  XmxSetArg(XmNcancelLabelString, (XtArgVal)no);
  XmxSetValues(PassDialog);

  XtManageChild(PassDialog);

  XmxBlockTimers = 1;
  while (!answer) {
      XtAppProcessEvent(app, XtIMAll);
      XSync(dsp, 0);
  }
  XmxBlockTimers = 0;

  XtUnmanageChild(PassDialog);
  XSync(dsp, 0);
  XmUpdateDisplay(PassDialog);

  XmStringFree(question);
  XmStringFree(yes);
  XmStringFree(no);

  if (!strcmp(answer, XMX_NO_ANSWER) || !xmx_passwd || !*xmx_passwd) {
      return NULL;
  } else {
      return strdup(xmx_passwd);
  }
}

static int DoFourButtonsAnswer;

static void XmxDoFourButtons_cb(Widget w, int answer, XtPointer call)
{
    DoFourButtonsAnswer = answer;
}

int XmxDoFourButtons(Widget parent, XtAppContext app, String title,
		     String question, String name1, String name2, String name3,
		     String name4, int width)
{
  Widget dialog, dialog_form, dialog_frame, dialog_sep, label, buttons_form;
  int save_id = Xmx_uniqid;

  DoFourButtonsAnswer = -1;

  if (!width)
      width = 520;

  XmxSetArg(XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
  dialog = XmxMakeFormDialog(parent, title);
  dialog_frame = XmxMakeFrame(dialog, XmxShadowOut);

  XmxSetConstraints 
    (dialog_frame, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM,
     NULL, NULL, NULL, NULL);
      
  dialog_form = XmxMakeForm(dialog_frame);
      
  dialog_sep = XmxMakeHorizontalSeparator(dialog_form);
      
  buttons_form = XmxMakeFormAndFourButtons(dialog_form,
					   (XtCallbackProc)XmxDoFourButtons_cb,
					   name1, name2, name3, name4,
					   1, 2, 3, 4);
  Xmx_uniqid = 0;
  XmxSetButtonClue(Xmx_button1_help, Xmx_button2_help, Xmx_button3_help,
		   Xmx_button4_help, NULL);
  Xmx_uniqid = save_id;

  label = XtVaCreateManagedWidget(question, xmLabelWidgetClass, dialog_form,
                                  XmNwidth, width,
                                  XmNleftAttachment, XmATTACH_FORM,
                                  XmNrightAttachment, XmATTACH_FORM,
                                  XmNtopAttachment, XmATTACH_FORM,
                                  XmNtopOffset, 2,
                                  NULL);
  XmxSetArg(XmNtopOffset, 8);
  XmxSetConstraints
    (dialog_sep, XmATTACH_WIDGET, XmATTACH_WIDGET, XmATTACH_FORM, XmATTACH_FORM,
     label, buttons_form, NULL, NULL);

  XmxSetConstraints
    (buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM,
     NULL, NULL, NULL, NULL);

  XtManageChild(dialog);

  XmxBlockTimers = 1;
  while(DoFourButtonsAnswer == -1) {
      XtAppProcessEvent(app, XtIMAll);
      XSync(dsp, 0);
  }
  XmxBlockTimers = 0;

  XmxClearButtonClue(Xmx_button1_help, Xmx_button2_help, Xmx_button3_help,
		     Xmx_button4_help, NULL);
  Xmx_button1 = Xmx_button2 = Xmx_button3 = Xmx_button4 = NULL;
  XcgLiteClueResetWait(liteClue);

  XtUnmanageChild(dialog);
  XSync(dsp, 0);
  XmUpdateDisplay(dialog);

  XtDestroyWidget(dialog);

  return DoFourButtonsAnswer;
}

static int DoFiveButtonsAnswer;

static void XmxDoFiveButtons_cb(Widget w, int answer, XtPointer call)
{
    DoFiveButtonsAnswer = answer;
}

int XmxDoFiveButtons(Widget parent, XtAppContext app, String title,
		     String question, String name1, String name2, String name3,
		     String name4, String name5, int width)
{
  Widget dialog, dialog_form, dialog_frame, dialog_sep, label, buttons_form;
  int save_id = Xmx_uniqid;

  DoFiveButtonsAnswer = -1;

  if (!width)
      width = 520;

  XmxSetArg(XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
  dialog = XmxMakeFormDialog(parent, title);
  dialog_frame = XmxMakeFrame(dialog, XmxShadowOut);

  XmxSetConstraints
    (dialog_frame, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM,
     NULL, NULL, NULL, NULL);
      
  dialog_form = XmxMakeForm(dialog_frame);
      
  dialog_sep = XmxMakeHorizontalSeparator(dialog_form);
      
  buttons_form = XmxMakeFormAndFiveButtons(dialog_form,
					   (XtCallbackProc)XmxDoFiveButtons_cb,
					   name1, name2, name3, name4, name5,
					   1, 2, 3, 4, 5);
  Xmx_uniqid = 0;
  XmxSetButtonClue(Xmx_button1_help, Xmx_button2_help, Xmx_button3_help,
		   Xmx_button4_help, Xmx_button5_help);
  Xmx_uniqid = save_id;

  label = XtVaCreateManagedWidget(question, xmLabelWidgetClass, dialog_form,
                                  XmNwidth, width,
                                  XmNleftAttachment, XmATTACH_FORM,
                                  XmNrightAttachment, XmATTACH_FORM,
                                  XmNtopAttachment, XmATTACH_FORM,
                                  XmNtopOffset, 2,
                                  NULL);
  XmxSetArg(XmNtopOffset, 8);
  XmxSetConstraints
    (dialog_sep, XmATTACH_WIDGET, XmATTACH_WIDGET, XmATTACH_FORM, XmATTACH_FORM,
     label, buttons_form, NULL, NULL);

  XmxSetConstraints
    (buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM,
     NULL, NULL, NULL, NULL);

  XtManageChild(dialog);

  XmxBlockTimers = 1;
  while(DoFiveButtonsAnswer == -1) {
      XtAppProcessEvent(app, XtIMAll);
      XSync(dsp, 0);
  }
  XmxBlockTimers = 0;

  XmxClearButtonClue(Xmx_button1_help, Xmx_button2_help, Xmx_button3_help,
		     Xmx_button4_help, Xmx_button5_help);
  Xmx_button1 = Xmx_button2 = Xmx_button3 = Xmx_button4 = Xmx_button5 = NULL;
  XcgLiteClueResetWait(liteClue);

  XtUnmanageChild(dialog);
  XSync(dsp, 0);
  XmUpdateDisplay(dialog);

  XtDestroyWidget(dialog);

  return DoFiveButtonsAnswer;
}


/*************************** LiteClue interface ***************************/

Widget XmxInitClue(Widget top, Boolean active)
{
  liteClue = XtVaCreatePopupShell("LiteClue_shell", xcgLiteClueWidgetClass,
				  top, NULL);
  XcgLiteClueSetActive(liteClue, active);

  return liteClue;
}

void XmxSetButtonClueText(String H1, String H2, String H3, String H4, String H5)
{
  Xmx_button1_help = H1;
  Xmx_button2_help = H2;
  Xmx_button3_help = H3;
  Xmx_button4_help = H4;
  Xmx_button5_help = H5;
}

void XmxSetButtonClue(String c1, String c2, String c3, String c4, String c5)
{
  if (c1 && Xmx_button1)
      XcgLiteClueAddWidget(liteClue, Xmx_button1, c1, 0, Xmx_uniqid);
  if (c2 && Xmx_button2)
      XcgLiteClueAddWidget(liteClue, Xmx_button2, c2, 0, Xmx_uniqid);
  if (c3 && Xmx_button3)
      XcgLiteClueAddWidget(liteClue, Xmx_button3, c3, 0, Xmx_uniqid);
  if (c4 && Xmx_button4)
      XcgLiteClueAddWidget(liteClue, Xmx_button4, c4, 0, Xmx_uniqid);
  if (c5 && Xmx_button5)
      XcgLiteClueAddWidget(liteClue, Xmx_button5, c5, 0, Xmx_uniqid);
}

void XmxClearButtonClue(String c1, String c2, String c3, String c4, String c5)
{
  if (c1 && Xmx_button1)
      XcgLiteClueDeleteWidget(liteClue, Xmx_button1);
  if (c2 && Xmx_button2)
      XcgLiteClueDeleteWidget(liteClue, Xmx_button2);
  if (c3 && Xmx_button3)
      XcgLiteClueDeleteWidget(liteClue, Xmx_button3);
  if (c4 && Xmx_button4)
      XcgLiteClueDeleteWidget(liteClue, Xmx_button4);
  if (c5 && Xmx_button5)
      XcgLiteClueDeleteWidget(liteClue, Xmx_button5);
}

void XmxAddClue(Widget wid, String clue)
{
  XcgLiteClueAddWidget(liteClue, wid, clue, 0, Xmx_uniqid);
}

void XmxDeleteClue(Widget wid)
{
  XcgLiteClueDeleteWidget(liteClue, wid);
}

void XmxDeleteClueGroup(void)
{
  XcgLiteClueDeleteGroup(liteClue, Xmx_uniqid);
}

void XmxClueForeground(unsigned long pixel)
{
  XtVaSetValues(liteClue, XtNforeground, pixel, NULL);
}

void XmxClueBackground(unsigned long pixel)
{
  XtVaSetValues(liteClue, XtNbackground, pixel, NULL);
}

void XmxClueTimers(int wait, int cancelwait, int popdowndelay)
{
  XtVaSetValues(liteClue,
		XcgNwaitPeriod, wait,
		XcgNcancelWaitPeriod, cancelwait,
		XcgNpopdownDelay, popdowndelay,
		NULL);
}

void XmxClueFont(String fontname)
{
  XFontStruct *font = XLoadQueryFont(dsp, fontname);

  if (font)
      XtVaSetValues(liteClue, XtNfont, font, NULL);
}

void XmxClueOval(Boolean oval)
{
  XtVaSetValues(liteClue, XcgNshaped, oval, NULL);
}

void XmxClueRounded(Boolean rounded)
{
  XtVaSetValues(liteClue, XcgNrounded, rounded, NULL);
}

void XmxClueActive(Boolean active)
{
  XcgLiteClueSetActive(liteClue, active);
}

Boolean XmxClueIsActive(void)
{
  return XcgLiteClueGetActive(liteClue);
}


/*************** Popup one time routines ***************/

void XmxClueOncePopup(Widget w, String clue, int x, int y, Boolean at_mouse)
{
#ifndef DISABLE_TRACE
  if (srcTrace)
      fprintf(stderr, "Clue popup '%s' at [%d,%d]\n", clue, x, y);
#endif
  XcgLiteCluePopup(liteClue, w, clue, x, y, at_mouse);
}

void XmxClueOncePopdown(void)
{
  XcgLiteCluePopdown(liteClue);
}

Boolean XmxClueOnceIsUp(void)
{
  return XcgLiteCluePopupIsUp(liteClue);
}
