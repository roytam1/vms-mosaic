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
#include "XmxP.h"

/* ------------------------------------------------------------------------ */
/* --------------------------- PRIVATE ROUTINES --------------------------- */
/* ------------------------------------------------------------------------ */

/* ----------------------- _XmxMenuAddEntryToRecord ----------------------- */

/* Create a new MenuEntry and add it to the head of a MenuRecord list. */
static void _XmxMenuAddEntryToRecord(XmxMenuRecord *rec, Widget w, int token)
{
  XmxMenuEntry *ent = (XmxMenuEntry *)malloc(sizeof(XmxMenuEntry));

  ent->w = w;
  ent->token = token;

  /* Add rest of list to tail of this entry. */
  ent->next = rec->first_entry;

  /* Make this entry head of list. */
  rec->first_entry = ent;

  return;
}

/* ---------------------- _XmxMenuGetEntryFromRecord ---------------------- */

/* Given token, fetch the corresponding entry. */
static XmxMenuEntry *_XmxMenuGetEntryFromRecord(XmxMenuRecord *rec, int token)
{
  XmxMenuEntry *ent = rec->first_entry;

  /* Search the linked list. */
  while (ent && (ent->token != token))
      ent = ent->next;
 
  return ent;
}

/* ------------------------- _XmxMenuCreateRecord ------------------------- */

/* Create a new MenuRecord and clear out its list. */
static XmxMenuRecord *_XmxMenuCreateRecord(Widget base)
{
  XmxMenuRecord *rec = (XmxMenuRecord *)malloc(sizeof(XmxMenuRecord));

  rec->base = base;
  rec->first_entry = NULL;
  return rec;
}

/* ------------------------------------------------------------------------ */
/* --------------------------- ROUTINES ---------------------------- */
/* ------------------------------------------------------------------------ */

/* --------------------------- XmxRSetSensitive --------------------------- */

/* args NOT used on Widget */
void XmxRSetSensitive(XmxMenuRecord *rec, int token, int state)
{
  XmxMenuEntry *entry = _XmxMenuGetEntryFromRecord(rec, XmxExtractToken(token));

  if ((state != XmxSensitive) && (state != XmxUnsensitive)) {
      fprintf(stderr, "Bug in XmxRSetSensitive.  state = %d\n", state);
      return;
  }
  /* XtSetSensitive propagates down Widget hierarchy. */
  if (entry)
      XtSetSensitive(entry->w, (state == XmxSensitive) ? True : False);
  
  return;
}

/* -------------------------- XmxRSetToggleState -------------------------- */

/* args not used */
void XmxRSetToggleState(XmxMenuRecord *rec, int token, int state)
{
  XmxMenuEntry *entry = _XmxMenuGetEntryFromRecord(rec, XmxExtractToken(token));

  if ((state != XmxSet) && (state != XmxUnset)) {
      fprintf(stderr, "Bug in XmxRSetToggleState.  state = %d\n", state);
      return;
  }
  if (entry)
      XmToggleButtonSetState(entry->w, (state == XmxSet) ? True : False, False);

  return;
}

/* ------------------------- XmxRUnsetAllToggles -------------------------- */

/* args not used */
void XmxRUnsetAllToggles(XmxMenuRecord *rec)
{
  XmxMenuEntry *ent;

  for (ent = rec->first_entry; ent; ent = ent->next)
      XmToggleButtonSetState(ent->w, False, False);

  return;
}

/* ----------------------- XmxRSetOptionMenuHistory ----------------------- */

/* args used on Widget */
void XmxRSetOptionMenuHistory(XmxMenuRecord *rec, int token)
{
  XmxMenuEntry *entry = _XmxMenuGetEntryFromRecord(rec, XmxExtractToken(token));

  if (entry) {
      XmxSetArg(XmNmenuHistory, (XtArgVal)(entry->w));
      XtSetValues(rec->base, Xmx_wargs, Xmx_n);
  }
  Xmx_n = 0;
  return;
}

/* ---------------------------- XmxRSetValues ----------------------------- */

/* args used on Widget */
void XmxRSetValues(XmxMenuRecord *rec, int token)
{
  XmxMenuEntry *entry = _XmxMenuGetEntryFromRecord(rec, XmxExtractToken(token));

  if (entry)
      XtSetValues(entry->w, Xmx_wargs, Xmx_n);

  Xmx_n = 0;
  return;
}

/* ---------------------------- XmxRGetWidget ----------------------------- */

/* args irrelevant */
Widget XmxRGetWidget(XmxMenuRecord *rec, int token)
{
  XmxMenuEntry *entry = _XmxMenuGetEntryFromRecord(rec, XmxExtractToken(token));

  if (entry)
      return entry->w;

  return NULL;
}

/* -------------------------- XmxRMakeOptionMenu -------------------------- */

/* args apply to pulldown menu */
XmxMenuRecord *XmxRMakeOptionMenu(Widget parent, String name, XtCallbackProc cb,
				  XmxOptionMenuStruct *opts)
{
  XmxMenuRecord *rec;
  Widget pulldown, button;
  Widget menuhist = NULL;
  int i = 0;
  XmString xmstr;

  /* Create a pulldown menupane to attach to the option menu;
   * preloaded wargs affect this. */
  pulldown = XmCreatePulldownMenu(parent, "pulldownmenu", Xmx_wargs, Xmx_n);

  /* menuHistory will not be applied to pulldown, so we'll modify
   * rec directly after creating the option menu. */
  rec = _XmxMenuCreateRecord(pulldown);

  /* Create pushbutton gadgets as childen of the pulldown menu. */
  while (opts[i].namestr) {
      Xmx_n = 0;
      xmstr = XmStringCreateLtoR(opts[i].namestr, XmSTRING_DEFAULT_CHARSET);
      XmxSetArg(XmNlabelString, (XtArgVal)xmstr);
      button = XmCreatePushButtonGadget(pulldown, "pushbutton",
                                        Xmx_wargs, Xmx_n);
      XmStringFree(xmstr);
      XtManageChild(button);
      XtAddCallback(button, XmNactivateCallback, cb,
                    (XtPointer)_XmxMakeClientData(opts[i].data));
      if (opts[i].set_state == XmxSet)
          menuhist = button;
      _XmxMenuAddEntryToRecord(rec, button, opts[i].data);
      i++;
  }

  /* Create the option menu itself; tie in the pulldown menu. */
  Xmx_n = 0;
  XmxSetArg(XmNsubMenuId, (XtArgVal)pulldown);
  if (menuhist)
      XmxSetArg(XmNmenuHistory, (XtArgVal)menuhist);
  Xmx_w = XmCreateOptionMenu(parent, "optionmenu", Xmx_wargs, Xmx_n);
  XtManageChild(Xmx_w);

  XmxSetArg(XmNalignment, (XtArgVal)XmALIGNMENT_BEGINNING);
  XmxSetValues(XmOptionButtonGadget(Xmx_w));

  if (name) {
      xmstr = XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);
      XmxSetArg(XmNlabelString, (XtArgVal)xmstr);
      XmxSetValues(XmOptionLabelGadget(Xmx_w));
      XmStringFree(xmstr);
  } else {
      XmxSetArg(XmNspacing, (XtArgVal)0);
      XmxSetArg(XmNmarginWidth, (XtArgVal)0);
      XmxSetValues(Xmx_w);
      XmxSetArg(XmNlabelString, (XtArgVal)NULL);
      XmxSetValues(XmOptionLabelGadget(Xmx_w));
  }

  /* Explicitly set base Widget of record. */
  rec->base = Xmx_w;

  Xmx_n = 0;
  return rec;
}

/* -------------------------- XmxRMakeToggleMenu -------------------------- */

/* args apply to radiobox or optionbox */
XmxMenuRecord *XmxRMakeToggleMenu(Widget parent, int behavior,
				  XtCallbackProc cb, XmxToggleMenuStruct *opts)
{
  XmxMenuRecord *rec;
  Widget box;
  int i = 0;

  switch (behavior) {
    case XmxOneOfMany:
      box = XmxMakeRadioBox(parent);
      break;
    case XmxNOfMany:
      box = XmxMakeOptionBox(parent);
      break;
    default:
      fprintf(stderr, "Bug in XmxRMakeToggleMenu.  behavior = %d\n", behavior);
      return NULL;
  }
  rec = _XmxMenuCreateRecord(box);

  while (opts[i].namestr) {
      XmxMakeToggleButton(box, opts[i].namestr, cb, opts[i].data);
      XmxSetToggleButton(Xmx_w, opts[i].set_state);
      _XmxMenuAddEntryToRecord(rec, Xmx_w, opts[i].data);
      i++;
  }
  Xmx_w = box;
  Xmx_n = 0;
  return rec;
}

/* -------------------------- _XmxRCreateMenubar -------------------------- */

/* Possible deficiency: will not be able to grey out a submenu
 * (cascade button). */
static void _XmxRCreateMenubar(Widget menu, XmxMenubarStruct *menulist, 
                               XmxMenuRecord *rec, Boolean tearoff)
{
  Widget *buttons;
  int i, bnum;
  int separators = 0;
  int nitems = 0;

  while (menulist[nitems].namestr)
      nitems++;

  buttons = (Widget *)XtMalloc(nitems * sizeof(Widget));

  for (i = 0; i < nitems; i++) {
      bnum = i - separators;

      /* Name of "----" means make a separator. */
      if (!strncmp(menulist[i].namestr, "----", 4)) {
          XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
                                menu, NULL, 0);
	  /* Double separator */
	  if (menulist[i].namestr[4] == '2')
              XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
                                    menu, NULL, 0);
          separators++;
      /* A function means it's an ordinary entry with callback. */
      } else if (menulist[i].func) {
          XmString xmstr;

          Xmx_n = 0;
          if (menulist[i].mnemonic)
              XmxSetArg(XmNmnemonic, (XtArgVal)menulist[i].mnemonic);
          if ((menulist[i].namestr[0] == '#') ||
              (menulist[i].namestr[0] == '<')) {  /* option/toggle button */

              /* A toggle button is diamond-shaped. */
              if (menulist[i].namestr[0] == '<')
                  XmxSetArg(XmNindicatorType, (XtArgVal)XmONE_OF_MANY);

	      /* Make sure the button shows up even when toggled off. */
              if (menulist[i].namestr[0] == '#')
                  XmxSetArg(XmNvisibleWhenOff, (XtArgVal)True);

              /* Ignore first character of label. */
              xmstr = XmStringCreateLtoR(&menulist[i].namestr[1],
					 XmSTRING_DEFAULT_CHARSET);
              XmxSetArg(XmNlabelString, (XtArgVal)xmstr);
              buttons[bnum] = XmCreateToggleButton(menu, "togglebutton",
						   Xmx_wargs, Xmx_n);
              XmStringFree(xmstr);
              XtAddCallback(buttons[bnum], XmNvalueChangedCallback,
              		    menulist[i].func,
              		    (XtPointer)_XmxMakeClientData(menulist[i].data));
              /* Add the button to the menu record. */
              _XmxMenuAddEntryToRecord(rec, buttons[bnum], menulist[i].data);
          } else {    /* Regular button */
	      xmstr = XmStringCreateLtoR(menulist[i].namestr,
					 XmSTRING_DEFAULT_CHARSET);
              XmxSetArg(XmNlabelString, (XtArgVal)xmstr);
              buttons[bnum] = XtCreateManagedWidget("pushbutton",
						    xmPushButtonWidgetClass,
					            menu, Xmx_wargs, Xmx_n);
              XmStringFree(xmstr);
              XtAddCallback(buttons[bnum], XmNactivateCallback,
			    menulist[i].func, 
              		    (XtPointer)_XmxMakeClientData(menulist[i].data));
              /* Add button to the menu record. */
              _XmxMenuAddEntryToRecord(rec, buttons[bnum], menulist[i].data);
          }
      /* No function and no submenu entry means it's just a label. */
      } else if (!menulist[i].sub_menu) {
          XmString xmstr;

          Xmx_n = 0;
	  xmstr = XmStringCreateLtoR(menulist[i].namestr,
				     XmSTRING_DEFAULT_CHARSET);
          XmxSetArg(XmNlabelString, (XtArgVal)xmstr);
          buttons[bnum] = XtCreateManagedWidget("label", xmLabelWidgetClass,
						menu, Xmx_wargs, Xmx_n);
          XmStringFree(xmstr);

      /* If all else fails, it's a submenu. */
      } else {
          XmString xmstr;
          Widget sub_menu;

          Xmx_n = 0;
#ifdef MOTIF1_2
	  if (tearoff)
              XmxSetArg(XmNtearOffModel, (XtArgVal)XmTEAR_OFF_ENABLED);
#endif
          sub_menu = XmCreatePulldownMenu(menu, "pulldownmenu",
					  Xmx_wargs, Xmx_n);
          Xmx_n = 0;
          XmxSetArg(XmNsubMenuId, (XtArgVal)sub_menu);
          if (menulist[i].mnemonic)
              XmxSetArg(XmNmnemonic, (XtArgVal)menulist[i].mnemonic);
          xmstr = XmStringCreateLtoR(menulist[i].namestr,
				     XmSTRING_DEFAULT_CHARSET);
          XmxSetArg(XmNlabelString, (XtArgVal)xmstr);
          buttons[bnum] = XtCreateWidget("cascadebutton",
					 xmCascadeButtonWidgetClass,
          				 menu, Xmx_wargs, Xmx_n);
          XmStringFree(xmstr);

          /* If name is "Help", put on far right. */
          if (!strcmp(menulist[i].namestr, "Help")) {
              Xmx_n = 0;
              XmxSetArg(XmNmenuHelpWidget, (XtArgVal)buttons[bnum]);
              XtSetValues(menu, Xmx_wargs, Xmx_n);
          }
          /* Recursively create new submenu. */
          _XmxRCreateMenubar(sub_menu, menulist[i].sub_menu, rec, tearoff);
      }
  }
  XtManageChildren(buttons, nitems - separators);

  XtFree((char *)buttons);
  return;
}

/* --------------------------- XmxRMakeMenubar ---------------------------- */

/* args apply to menubar */
XmxMenuRecord *XmxRMakeMenubar(Widget parent, XmxMenubarStruct *mainmenu,
			       Boolean tearoff)
{
  Widget menubar;
  XmxMenuRecord *rec;

  /* Preset resources applied to main menubar only. */
  menubar = XmCreateMenuBar(parent, "menubar", Xmx_wargs, Xmx_n);
  XtManageChild(menubar);

  /* Create the new XmxMenuRecord. */
  rec = _XmxMenuCreateRecord(menubar);

  Xmx_n = 0;
  _XmxRCreateMenubar(menubar, mainmenu, rec, tearoff);

  Xmx_n = 0;
  Xmx_w = menubar;
  return rec;
}
