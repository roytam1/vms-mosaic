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

/* Copyright (C) 1998, 1999, 2000, 2004, 2005, 2006, 2007
 * The VMS Mosaic Project
 */

#include "../config.h"
#include "../libwww2/htparse.h"
#include "mosaic.h"
#include "gui-ftp.h"
#include "mo-www.h"
#include "../libnut/system.h"
#include "gui.h"
#include "../libnut/ellipsis.h"
#include "../libnut/str-tools.h"
#include "hotlist.h"

#ifndef GUI_POPUP_H
/* See gui-popup.h for effects of this define */
#define GUI_POPUP_H
#include "gui-popup.h"
#endif

#include "gui-documents.h"
#include "gui-dialogs.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

extern mo_window *current_win;
extern int imageViewInternal;
extern int do_head;
extern char *HTReferer;
extern Widget toplevel;
extern int LimDimY;

static Boolean have_popup;
static Widget popup = NULL;
Widget popup_shell = NULL;


static char **user_defs_get_entries(FILE *fp, int *num)
{
  char **entries = malloc(sizeof(char *) * 100);
  char str[512];
  int i = 0;

  while (fgets(str, 512, fp)) {
      int index = 0;

      while (isspace(str[index]))
	  index++;

      if (str[index] != '#' && str[index] != '\n' && str[index]) {
	  if (i % 2) {	/* URL spec line */
	      switch(str[index]) {
		case 'G':  /* GET: */
		case 'P':  /* POST: */
		case 'F':  /* FETCH: */
		  entries[i] = strdup(&str[index]);
		  entries[i] = my_chop(entries[i]);
		  i++;
		  break;
		default:  /* Error */
		  fprintf(stderr, "User defined field wrong:%s.  Ignoring it\n",
			  &str[index]);
	      }
	  } else {	/* Button name */
	      if (strlen(str) > 50) {
		  fprintf(stderr,
			  "User defined button name too long.  Ignoring it\n");
	      } else {
		  entries[i] = strdup(&str[index]);
		  entries[i] = my_chop(entries[i]); 
		  i++;
	      }
	  }
      }
  }
  if (i % 2 == 1) {
      fprintf(stderr, "Problem in gui_popup.c:%d \n", __LINE__);
      return NULL;
  }

  *num = i / 2;
  return entries;  /* They better free this */
}


static void select_cb(Widget w, XtPointer client_data, Atom *sel, Atom *type,
		      XtPointer value, unsigned long *length, int *format)
{
  char *end = NULL;
  char *select = (char *) value;
  char *str = (char *) client_data;
  char *nselect, *begin, *pt, *bptr;
  char mode;

  /* This filters out empty strings and most possible errors */
  if ((*type != XA_STRING) || !select || !*select || !str || !*str || !*length)
      return;

  bptr = begin = strdup(str);

  /* Do this cause select is not null terminated sometimes */
  pt = my_strndup(select, *length);
  
  if (pt) {
      select = pt;
  } else {
      XtFree(value);
      free(begin);
      return;
  }

  switch (*bptr) {
    case 'G':  /* GET: */
      bptr += 4;
      mode = 'G';
      break;
    case 'P':  /* POST: */
      bptr += 5;
      mode = 'P';
      break;
    case 'F':  /* FETCH: */
      bptr += 6;
      mode = 'F';
      break;
    default:  /* Error */
      fprintf(stderr, "User defined field wrong:%s. Ignoring it\n", begin);
      XtFree(value);
      free(begin);
      return;
  }
  bptr = my_chop(bptr);

  if (mode == 'F') {
      /* Expand url */
      nselect = mo_url_prepend_protocol(select);
  } else {
      /* Make search string sendable */
      nselect = HTEscape(select);
  }
  XtFree(value);  /* This frees select */

  pt = strchr(bptr, '_');
  while (!end && pt && *pt) {
      if (!strncmp(pt, "__string__", 10)) {
	  end = pt + 10;
      } else if (++pt && *pt) {
	  pt = strchr(pt, '_');
      }
  } 

  if (pt && *pt && end && nselect) {
#ifndef DISABLE_TRACE
      if (srcTrace)
	  fprintf(stderr, "Popup getting %s from user menu.\n", pt);
#endif
      if (mode == 'P') {
	  char *ptr;
	  
	  ptr = strrchr(bptr, ' ');  /* Shouldn't fail because bptr chopped */
	  *ptr++ = '\0';	     /* Make bptr not have name value pair */
	  bptr = my_chop(bptr);
	  ptr = my_chop(ptr);

	  *pt = '\0';		     /* Make __string__ go away from ptr */
	  pt = malloc(sizeof(char) * (strlen(end) + strlen(nselect) +
		                      strlen(ptr) + 1));
	  sprintf(pt, "%s%s%s", ptr, nselect, end);

	  mo_post_load_window_text(current_win, bptr, 
				   "application/x-www-form-urlencoded", pt);
	  free(pt);
      } else if (mode == 'G') {
	  *pt = '\0';
	  pt = malloc(sizeof(char) * (strlen(end) + strlen(bptr) +
		                      strlen(nselect) + 1));
	  sprintf(pt, "%s%s%s", bptr, nselect, end);
	  mo_load_window_text(current_win, pt, NULL);      
	  free(pt);
      } else if (mode == 'F') {
	  mo_load_window_text(current_win, nselect, NULL);
      }
  }
  free(begin);
  free(nselect);
}


static void user_defs_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
  act_struct *acst = (act_struct *) client_data;  /* acst->str is the url */
  XmPushButtonCallbackStruct *cbs = (XmPushButtonCallbackStruct *) call_data;
  char *str = acst->str;
  
  if (!str)
      return;
  XtGetSelectionValue(current_win->scrolled_win, XA_PRIMARY, XA_STRING, 
		      select_cb, str, cbs->event->xbutton.time);
}


static PopupItem *build_user_defs_items(char **entries, int num)
{
  PopupItem *items;
  int i;

  if (!entries || (num <= 0))
      return NULL;

  if (!(items = calloc(sizeof(PopupItem), num + 1)))
      return NULL;

  for (i = 0; i < num; i++) {
      items[i].class = PushButton;
      items[i].label = strdup(entries[i * 2]);
      items[i].cbfp = user_defs_cb;
      items[i].acst.str = strdup(entries[i * 2 + 1]);
      items[i].startup = 1;
      /** Calloc zeros them
      items[i].types = 0;
      items[i].types_method = 0;
      items[i].modes = 0;
      items[i].modes_method = 0;
      items[i].acst.act_code = 0;
      items[i].acst.eptr = NULL;
      items[i].mnemonic = '\0';
      items[i].accel_text = NULL;
      items[i].accel = NULL;
      items[i].w = NULL;
      items[i].sub_items = NULL;
      **/
  }
  items[num].class = LastItem;
  /* items[num].sub_items = NULL; */

  return items;
}


static PopupItem *popup_build_user_defs()
{
  PopupItem *items;
  char *str, *file;
  char **entries;
  FILE *fp;
  int num, i;

  if (num = get_home(&str)) 
      return NULL;
	
  file = malloc(sizeof(char) * (strlen(str) + strlen("/.mosaic-user-defs") +1));
#ifndef VMS   /* PGE */
  sprintf(file, "%s/.mosaic-user-defs", str);
#else
  sprintf(file, "%smosaic-user-defs.", str);
#endif
  
  free(str);

  if (!file_exists(file)) {
      free(file);
      return NULL;
  }
  fp = fopen(file, "r");
  free(file);

  if (fp) {
      fseek(fp, 0, SEEK_SET);
      entries = user_defs_get_entries(fp, &num);
      fclose(fp);
  } else {
      return NULL;
  }
  items = build_user_defs_items(entries, num);

  for (i = 0; i < (num + 2); i++)
      free(entries[i]);
  free(entries);

  return items;
}


static void set_eptr_field(PopupItem *items, ElemInfo *eptr)
{
  int i;

  for (i = 0; items[i].class != LastItem; i++) {
      if (items[i].sub_items)
	  set_eptr_field(items[i].sub_items, eptr);
      items[i].acst.eptr = eptr;
  }
}


static void rbm_ballonify(Widget w, XtPointer client_data, XtPointer call_data)
{
  mo_gui_notify_progress((char *) client_data);
}


static Widget PopupMenuBuilder(Widget parent, int type, char *title, 
			       char mnem, PopupItem *items)
{
  Widget menu, cascade;
  XmString str;
  int i, mapping_del;
  int scol_max = 38;
  int mcol_max = 42;

  if (type == XmMENU_POPUP) {
      menu = XmCreatePopupMenu(parent, title, NULL, 0);
  } else if (type == XmMENU_PULLDOWN) { 
      if (LimDimY < 1024) {
	  scol_max = 36;
	  mcol_max = 36;
      }
      menu = XmCreatePulldownMenu(parent, title, NULL, 0);
      str = XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET);
      mapping_del = get_pref_int(ePOPUPCASCADEMAPPINGDELAY);
      cascade = XtVaCreateManagedWidget(title, xmCascadeButtonGadgetClass,
					parent,
					XmNsubMenuId, menu,
					XmNlabelString, str,
					XmNmnemonic, mnem,
					XmNmappingDelay, mapping_del,
					NULL);
      XmStringFree(str);
  } else {
      return (Widget)NULL;  /* This shouldn't happen */
  }

  for (i = 0; items[i].class != LastItem; i++) {
      switch(items[i].class) {
	case PushButton:
	    items[i].w = XtVaCreateManagedWidget(items[i].label,
						 xmPushButtonGadgetClass,
						 menu, NULL);
	    if (items[i].mnemonic)
	        XtVaSetValues(items[i].w,
			      XmNmnemonic, items[i].mnemonic,
			      NULL);
	    if (items[i].accel)
		XtVaSetValues(items[i].w,
			      XmNaccelerator, items[i].accel,
			      NULL);
	    if (items[i].accel_text) {
		str = XmStringCreateLtoR(items[i].accel_text,
					 XmSTRING_DEFAULT_CHARSET);
		XtVaSetValues(items[i].w,
			      XmNacceleratorText, str,
			      NULL);
		XmStringFree(str);
	    }
	    if (items[i].cbfp)
	        XtAddCallback(items[i].w, XmNactivateCallback,
			      items[i].cbfp, &items[i].acst);

	    XtSetSensitive(items[i].w, items[i].startup);

	    if (items[i].acst.str && items[i].acst.act_code == 69) {
		XtAddCallback(items[i].w, XmNarmCallback, rbm_ballonify,
			      items[i].acst.str);
		XtAddCallback(items[i].w, XmNdisarmCallback, rbm_ballonify,
			      NULL);
	    }
	    break;
	case Separator:
	    items[i].w = XtVaCreateManagedWidget(items[i].label,
						 xmSeparatorGadgetClass,
						 menu, NULL);
	    break;
	case ToggleButton:
	    if ((items[i].cbfp == presentation_cb) &&
		!get_pref_boolean(ePRESENTATION_MODE_ON_RBM))
		break;
	    items[i].w = XtVaCreateManagedWidget(items[i].label,
				  xmToggleButtonGadgetClass, menu,
				  XmNset, items[i].acst.act_code ? True : False,
				  NULL);
	    if (items[i].cbfp)
	        XtAddCallback(items[i].w, XmNvalueChangedCallback,
			      items[i].cbfp, &items[i].acst);
	    break;
	case CascadeButton:
	    /* Check if no hotlist on RBM */
	    if ((items[i].acst.act_code == -1) && !items[i].sub_items) {
		break;
	    } else if (items[i].sub_items && (items[i].acst.act_code != -2)) {
	        items[i].w = PopupMenuBuilder(menu, XmMENU_PULLDOWN,
					      items[i].label, items[i].mnemonic,
					      items[i].sub_items);
	    } else if (get_pref_boolean(eSESSION_HISTORY_ON_RBM)) {
		    int mapping_del = get_pref_int(ePOPUPCASCADEMAPPINGDELAY);

		    items[i].w = XtVaCreateManagedWidget("Session History",
					xmCascadeButtonGadgetClass, menu,
					XmNsubMenuId, current_win->session_menu,
					XmNmappingDelay, mapping_del, NULL);
	    }
      }
  }
  /* Make it multiple columns, if needed */
  if ((type == XmMENU_PULLDOWN) && (i > scol_max)) {
      int columns;

      if (i < (4 * scol_max)) {
          columns = (i / scol_max) + 1;
      } else {
          columns = (i / mcol_max) + 1;
      }
      XmxSetArg(XmNpacking, XmPACK_COLUMN);
      XmxSetArg(XmNnumColumns, columns);
      XmxSetValues(menu);
  }
  return type == XmMENU_POPUP ? menu : cascade;
}


static void ThirdButtonMenu(Widget w, XtPointer *client_data, XEvent *event,
			    Boolean *ctd)
{
  XButtonPressedEvent *BuEvent = (XButtonPressedEvent *) event;
  static ElemInfo *eptr;

  if (have_popup && (BuEvent->button == Button3)) {
      int epos, mode, type, i;
      int del = 12;
      int sens = False;

      while (w && (XtClass(w) != htmlWidgetClass))
	  w = XtParent(w);
      if (!w)
	  return;

      eptr = LocateElement((HTMLWidget) w, BuEvent->x, BuEvent->y, &epos);

      if (!popup) { 
	  /* First time through, we need to do the hotlist init.
	   * If we don't do it here, it gets the wrong info in it. */
	  if (get_pref_boolean(eHOTLIST_ON_RBM))
	      mo_init_hotmenu();
 
	  /* Before we build the popup see if the user has a .mosaic/user-defs
	   * file and, if so, use it to build a user menu. */
	  popup_items[3].sub_items = popup_build_user_defs();

	  /* If we didn't have any turn it off */
	  if (!popup_items[3].sub_items)
	      popup_items[3].types = 0;

	  /* Need shell widget because cannot be directly under "toplevel" */
	  if (!popup_shell)
	      popup_shell = XtCreatePopupShell("popup_shell",
					       topLevelShellWidgetClass,
					       toplevel, NULL, 0);
	  popup = PopupMenuBuilder(popup_shell, XmMENU_POPUP, 
				   "popup", 0, popup_items);
      }
      mode = current_win->mode;

      if (eptr) {
	  type = eptr->type;
	  if ((type == E_IMAGE) && eptr->anchor_tag_ptr->anc_href) {
	      /* Turn on anchor and off text */
	      type |= E_ANCHOR;
	      if (type & E_TEXT)
		  type -= E_TEXT;
	  }
	  if ((type == E_TEXT) && eptr->anchor_tag_ptr->anc_href &&
	      my_strncasecmp(eptr->anchor_tag_ptr->anc_href, "mailto:", 7) &&
	      my_strncasecmp(eptr->anchor_tag_ptr->anc_href, "cookiejar:",10) &&
	      my_strncasecmp(eptr->anchor_tag_ptr->anc_href, "telnet:", 7))
	      type = E_ANCHOR;
      } else {
	  type = E_HRULE;  /* Pick a good normal little element */
      }
  
      for (i = 0; popup_items[i].class != LastItem; i++) {
	  if (popup_items[i].w) {
	      /* Anything is possible in Mosaic */
	      int good = True;
	      
	      /* Take care of session menu */
	      if (popup_items[i].acst.act_code == -2) {
		  XtVaSetValues(popup_items[i].w,
				XmNsubMenuId, current_win->session_menu,
				NULL);
	      } else if (popup_items[i].cbfp == presentation_cb) {
		  XtVaSetValues(popup_items[i].w,
				XmNindicatorOn, current_win->presentation_mode,
				XmNset, current_win->presentation_mode,
				NULL);
	      }
	      /* Determine if we want this guy */
	      if (popup_items[i].types_method == LOOSE) {
		  good = good && (popup_items[i].types & type);
	      } else {
		  good = good && (popup_items[i].types == type);
	      }
	      if (popup_items[i].modes_method == LOOSE) {
		  good = good && (popup_items[i].modes & mode);
	      } else {
		  good = good && (popup_items[i].modes == mode);
	      }
	      if (good) {
		  if (popup_items[i].class == Separator) {
		      del += 4;
		  } else {
		      switch(mode) {
			case moMODE_PLAIN:
			  if (!(type & E_IMAGE)) {
			      if (!XtIsSensitive(popup_items[i].w) && !sens) {
				  del += 24;
			      } else {
				  sens = True;
			      }
			      break;
			  }
			case moMODE_NEWS:
			case moMODE_FTP:
			  if (i > 3) {
			      /* Skip forward, backward, hotlist, and sep */
			      if (!XtIsSensitive(popup_items[i].w) && !sens) {
				  del += 24;
			      } else {
				  sens = True;
			      }
			  } else {
			      del += 24;
			  }
			  break;
		      }
		  }
		  XtManageChild(popup_items[i].w); 
	      } else {
		  XtUnmanageChild(popup_items[i].w);
	      }
	  }
      }

      /* Set all the widgets eptr data */
      set_eptr_field(popup_items, eptr);

      /* Motif puts the menu in a boring place.  Lets fix it. */      
      /*   BuEvent->x_root -= 40; */   /* Middle of buttons */
      BuEvent->y_root -= del;  /* First active button or first specialty
			        * item if we're over element that has em */
      XmMenuPosition(popup, BuEvent);
      XtManageChild(popup);
  }
}


void mo_make_popup(Widget view)
{
  if (!get_pref_boolean(eKIOSK) && !get_pref_boolean(eKIOSKNOEXIT)) {
      have_popup = True;  /* This will cause it to be created later */
      XtInsertEventHandler(view, ButtonPressMask, False,
			   (XtEventHandler)ThirdButtonMenu, NULL, XtListHead);
  } else {
      have_popup = False;
  }
}


void mo_popup_set_something(char *what, int to, PopupItem *items)
{
  Widget w = NULL;
  int i;

  if (!items)
      items = popup_items;

  for (i = 0; (items[i].class != LastItem) && !w; i++) {
      if (items[i].label && (items[i].label[0] == *what)) {
	  if (!strcmp(items[i].label, what)) {
	      w = items[i].w;
	      if (!w) {
		  items[i].startup = to;
		  break;
	      }
	  }
      }
      if (items[i].sub_items)
	  mo_popup_set_something(what, to, items[i].sub_items);
  }
  if (w)
      XtSetSensitive(w, to);
}


static XmxCallback(metadata_cb)
{
  act_struct *acst = (act_struct *) client_data;
  char *xurl;
  ElemInfo *eptr;
  int which;

  if (!acst)
      return;

  which = acst->act_code;
  if (which != M_FileData) {
      if (!acst->eptr)
          return;
      eptr = acst->eptr;
  }

  switch (which) {
      case M_ImageData:
	  if (!eptr->pic_data || !eptr->pic_data->src)
	      /* Do what? */
	      return;
	  xurl = mo_url_prepend_protocol(eptr->pic_data->src);
	  do_head = 1;
	  mo_load_window_text(current_win, xurl, NULL);
	  free(xurl);
	  break;

      case M_FileData:
	  do_head = 1;
	  mo_load_window_text(current_win, current_win->current_node->url,NULL);
	  break;

      case M_LinkData:
	  if (!eptr->anchor_tag_ptr->anc_href ||
	      !*eptr->anchor_tag_ptr->anc_href)
	      return;
	  xurl = mo_url_canonicalize(eptr->anchor_tag_ptr->anc_href,
				     current_win->current_node->url);
	  do_head = 2;
	  mo_load_window_text(current_win, xurl, NULL);
	  free(xurl);
	  break;

      default:
	  fprintf(stderr, "Smells like an error...\n");
  }
  do_head = 0;
  return;
}

static XmxCallback(fsb_OKCallback)
{
  XmFileSelectionBoxCallbackStruct *cbs = 
				 (XmFileSelectionBoxCallbackStruct *) call_data;
  char *filename;
  char *url = (char *) client_data;
  char efilename[MO_LINE_LENGTH];
  
  /* Remove the widget from the screen, and kill it. */
  XtUnmanageChild(w);
  
  /* Retrieve the character string from the compound string format. */
  XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &filename);

  /* Expand any ~ */
  pathEval(efilename, filename);
  XtFree(filename);

  mo_gui_clear_icon();
  mo_pull_er_over_virgin(url, efilename);

  /* We need to reset the icons and let the user know */
  mo_gui_done_with_icon();
  mo_gui_notify_progress("Image has been downloaded and saved.");

}

static XmxCallback(fsb_CancelCallback)
{
  XtUnmanageChild(w);
}

static void fsb(char *src)
{
  static Widget dialog;
  XmString str;
  char *fname;
  char fBuf[1024];
  static char *last_src = NULL;
  
  if (!dialog) {
      last_src = strdup(src);
      if (!popup_shell)
	  popup_shell = XtCreatePopupShell("popup_shell",
					   topLevelShellWidgetClass,
					   toplevel, NULL, 0);
      dialog = XmCreateFileSelectionDialog(popup_shell, "Save Image File",
					   NULL, 0);
      XtAddCallback(dialog, XmNcancelCallback, fsb_CancelCallback, NULL);
      XtAddCallback(dialog, XmNokCallback, fsb_OKCallback, last_src);
      XtSetSensitive(XmFileSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON),
		     False);
      XtVaSetValues(dialog,
		    XmNfileTypeMask, XmFILE_REGULAR,
		    NULL);
  } else {
      /* Dance with the callbacks so we get the correct URL later */
      XtRemoveCallback(dialog, XmNokCallback, fsb_OKCallback, last_src);
      if (last_src)
	  free(last_src);
      last_src = strdup(src);
      XtAddCallback(dialog, XmNokCallback, fsb_OKCallback, last_src);
      /* Re-Init the Stupid Box */
      XmFileSelectionDoSearch(dialog, NULL);
  }

  /* Set the save file string */
  XtVaGetValues(dialog,
		XmNdirSpec, &str,
		NULL);
  XmStringGetLtoR(str, XmSTRING_DEFAULT_CHARSET, &fname);
  XmStringFree(str);

  if (fname) {
      if (src && *src) {
	  sprintf(fBuf, "%s%s", fname, getFileName(src));
      } else {
	  sprintf(fBuf, "%s", fname);
      }
      str = XmStringCreateLtoR(fBuf, XmSTRING_DEFAULT_CHARSET);
      XtVaSetValues(dialog,
		    XmNdirSpec, str,
		    NULL);
      XmStringFree(str);
      XtFree(fname);
  }
  XtManageChild(dialog);
}

static XmxCallback(image_cb)
{
  act_struct *acst = (act_struct *) client_data;
  char *xurl;
  ElemInfo *eptr;
  int tmp; 
  static char *referer = NULL;

  eptr = acst->eptr;

  /* Somewhere we lose source on rare occasions... */
  if (!eptr || !eptr->pic_data || !eptr->pic_data->src) {
      fprintf(stderr, "Lost source.\n");
      return;  /* Oh, well */
  }

  /* Free previous one, if any */
  if (referer) {
      free(referer);
      referer = NULL;
  }

  switch(acst->act_code) {
    case I_Save:
        fsb(eptr->pic_data->src);
        break;

    case I_ViewExternal:
    case I_ViewInternal:
	xurl = mo_url_prepend_protocol(eptr->pic_data->src);
	referer = strdup(current_win->current_node->url);
	HTReferer = referer;
	tmp = imageViewInternal;
	if (acst->act_code == I_ViewExternal) {
	    imageViewInternal = 0;
	} else {
	    imageViewInternal = 1;
	}
	mo_load_window_text(current_win, xurl, NULL);
	imageViewInternal = tmp;
	free(xurl);
        break;

    case I_Reload:
	mo_reload_window_text(current_win, 1);
        break;
  }
}

static XmxCallback(presentation_cb)
{
    mo_presentation_mode(current_win);
}

static void hot_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    act_struct *acst = (act_struct *) client_data;
    char *xurl;

    switch(acst->act_code) {
      case 1:  /* Add item */
        mo_add_item_to_hotlist(acst->str, mo_t_url,
                               current_win->current_node->title,
                               current_win->current_node->url, 0,
			       get_pref_boolean(eADD_RBM_ADDS_RBM));
	mo_write_default_hotlist();
        break;

      case 2:  /* Add list */
        if (xurl = prompt_for_string("Enter new Hotlist List name:")) {
            mo_add_item_to_hotlist(acst->str, mo_t_list, xurl, NULL, 0,
				   get_pref_boolean(eADD_RBM_ADDS_RBM));
	    XtFree(xurl);
	    mo_write_default_hotlist();
        }
        break;

      default:  /* Goto link */
        if (acst->str) {
            xurl = mo_url_prepend_protocol(acst->str);
            mo_load_window_text(current_win, xurl, NULL); 
	    free(xurl);
        }
        break;
    }
}

static void mo_destroy_hot_menu(PopupItem *pmenu)
{
    int i;

    for (i = 0; pmenu[i].class != LastItem; i++) {
        if ((pmenu[i].class != Separator) &&
            (pmenu[i].acst.act_code != 1) &&
	    (pmenu[i].acst.act_code != 2)) 
            free(pmenu[i].label);
        if (pmenu[i].class == CascadeButton)
            mo_destroy_hot_menu(pmenu[i].sub_items);
    }
    free(pmenu);
}
        
static PopupItem *mo_assemble_hot_menu(mo_hotlist *list)
{
    mo_hot_item *item;
    char str[31];
    PopupItem *pmenu;
    int i = 0;

    /* Have to count it first.  Sigh */
    for (item = list->nodelist; item; item = item->any.next) {
	if ((item->type == mo_t_url && item->hot.rbm) ||
	    (item->type == mo_t_list && item->list.rbm))
	    i++;
    }

    pmenu = (PopupItem *) calloc(sizeof(PopupItem), i + 5);
    for (i = 0, item = list->nodelist; item; item = item->any.next) {
	if ((item->type == mo_t_url && !item->hot.rbm) ||
	    (item->type == mo_t_list && !item->list.rbm))
	    continue;

        if (compact_string(item->hot.title, str, 30, 3, 3) == 2) {
            pmenu[i].label = strdup(item->hot.title);
	} else {
            pmenu[i].label = strdup(str);
	}
        pmenu[i].cbfp = hot_cb;
	pmenu[i].startup = 1;
	/* calloc zeros them
        pmenu[i].types = 0;
        pmenu[i].modes = 0;
        pmenu[i].mnemonic = '\0';
        pmenu[i].accel_text = NULL;
        pmenu[i].accel = NULL;
        pmenu[i].w = NULL;
	**/
        if (item->type == mo_t_url) {	  /* URL item */
	    pmenu[i].acst.str = item->hot.url;
	    pmenu[i].acst.act_code = 69;  /* Identifies this as a hotlist 
					   * button so we can ballon it */
            pmenu[i].class = PushButton;
            /* pmenu[i].sub_items = NULL; */
        } else {
            pmenu[i].class = CascadeButton;
            pmenu[i].sub_items = mo_assemble_hot_menu(&item->list);
            /* pmenu[i].acst.act_code = 0; */
        }
        i++;
    }
    pmenu[i].class = Separator;
    /* pmenu[i].sub_items = NULL; */
    pmenu[i].label = strdup("Sep");
    i++;
    
    pmenu[i].class = PushButton;
    pmenu[i].label = "Add current URL...";
    pmenu[i].cbfp = hot_cb;
    pmenu[i].acst.str = list;
    pmenu[i].acst.act_code = 1;
    pmenu[i].startup = 1;
    /** calloced
    pmenu[i].types = 0;
    pmenu[i].modes = 0;
    pmenu[i].mnemonic = '\0';
    pmenu[i].accel_text = NULL;
    pmenu[i].accel = NULL;
    pmenu[i].w = NULL;
    pmenu[i].sub_items = NULL;
    **/
    i++;

    pmenu[i].class = PushButton;
    pmenu[i].label = "Add New List...";
    pmenu[i].cbfp = hot_cb;
    pmenu[i].acst.str = list;
    pmenu[i].acst.act_code = 2;
    pmenu[i].startup = 1;
    /**
    pmenu[i].types = 0;
    pmenu[i].modes = 0;
    pmenu[i].mnemonic = '\0';
    pmenu[i].accel_text = NULL;
    pmenu[i].accel = NULL;
    pmenu[i].w = NULL;
    pmenu[i].sub_items = NULL;
    **/
    i++;
    
    pmenu[i].class = LastItem;
    return pmenu;
}

static int hot_button = 0;

void mo_init_hotlist_menu(mo_hotlist *list)
{
    /* This doesn't check the first button, but that is okay because
     * the first two buttons are always back and forward */
    while (popup_items[hot_button].acst.act_code != -1) 
        hot_button++;
  
    popup_items[hot_button].sub_items = mo_assemble_hot_menu(list);
}

void mo_reinit_hotlist_menu(mo_hotlist *list)
{
    if (!popup)
	return;
    
    /* May never have been initialized (HOTLIST_ON_RBM == False) */
    if (!hot_button) {
	mo_init_hotlist_menu(list);
    } else {
	mo_destroy_hot_menu(popup_items[hot_button].sub_items);
	popup_items[hot_button].sub_items = mo_assemble_hot_menu(list);
    }
    XtDestroyWidget(popup);
    
    popup = PopupMenuBuilder(popup_shell, XmMENU_POPUP,
			     "popup", 0, popup_items);
}


static XmxCallback(load_link_to_disk_cb)
{
  act_struct *acst = (act_struct *) client_data;
  char *url;

  if (!acst || !acst->eptr || !acst->eptr->anchor_tag_ptr->anc_href ||
      !*acst->eptr->anchor_tag_ptr->anc_href)
      return;

  url = mo_url_canonicalize(acst->eptr->anchor_tag_ptr->anc_href,
	                    current_win->current_node->url);
  pub_anchor_ltd(url);

  free(url);
}


static Boolean convert_selection(Widget w, Atom *sel, Atom *tar, Atom *typ_ret,
				 XtPointer *val_ret, unsigned long *val_len, 
				 int *format)
{
  if (*tar == XA_STRING) {
      char *url;
      int i;

#ifndef DISABLE_TRACE
      if (srcTrace) 
	  fprintf(stderr, "Pasting text selection.\n");
#endif
      for (i = 0; popup_items[i].class != LastItem; i++) {
	  if (!strcmp(popup_items[i].label, COPY_URL_LABEL)) {
	      if (popup_items[i].acst.str) {
		  url = (char *) popup_items[i].acst.str;
		  break;
	      }
	      return False;
	  }
      }
      *val_ret = strdup(url);
      *val_len = strlen(url);
      *typ_ret = XA_STRING;
      *format = 8;
      return(True);
  }
  return(False);
}


static XmxCallback(copy_link_cb)
{
  act_struct *acst = (act_struct *) client_data;
  XmPushButtonCallbackStruct *cbs = (XmPushButtonCallbackStruct *) call_data;
  char *url;

  if (!acst || !acst->eptr || !acst->eptr->anchor_tag_ptr->anc_href ||
      !*acst->eptr->anchor_tag_ptr->anc_href)
      return;

  url = mo_url_canonicalize(acst->eptr->anchor_tag_ptr->anc_href,
	                    current_win->current_node->url);

  if (XtOwnSelection((Widget) current_win->scrolled_win, XA_PRIMARY, 
		     cbs->event->xbutton.time, convert_selection,
		     NULL, NULL) == False) {
      fprintf(stderr, "Mosaic Error: Could not copy selection, try again.\n");
      if (url)
	  free(url);
  } else {
      int i;
      
      for (i = 0; popup_items[i].class != LastItem; i++) {
	  if (!strcmp(popup_items[i].label, COPY_URL_LABEL) && url) {
	      char *copy_str = malloc((strlen(url) + 24) * sizeof(char));

	      if (popup_items[i].acst.str)
		  free(popup_items[i].acst.str);
	      popup_items[i].acst.str = url;

	      sprintf(copy_str, "URL: %s  has been copied", url);
	      mo_gui_notify_progress(copy_str);
	      free(copy_str);
	      break;
	  } else if (!strcmp(popup_items[i].label, COPY_URL_LABEL) && !url) {
	      if (popup_items[i].acst.str)
		  free(popup_items[i].acst.str);
	      popup_items[i].acst.str = NULL;
	      break;
	  }
      }
  }
}


static XmxCallback(session_cb)
{
  mo_load_window_text(current_win, (char *)client_data, NULL); 
}


void mo_add_to_rbm_history(mo_window *win, char *url, char *title)
{
  char label[32];
  static Boolean rbm_on;
  static int max;
  static int init = 0;

  if (!win || !url || !*url || !title || !*title)
      return;

  if (!init) {
      max = get_pref_int(eNUMBER_OF_ITEMS_IN_RBM_HISTORY);
      rbm_on = get_pref_boolean(eSESSION_HISTORY_ON_RBM);
      init = 1;
  }

  if (!rbm_on) {
      return;
  } else if (!win->session_menu) {
      win->session_menu = XmCreatePulldownMenu(win->view, "session_menu",
					       NULL, 0);
  }
  if (compact_string(title, label, 31, 3, 3) == 2)
      strcpy(label, title);

  if (win->num_session_items < max) {
      win->session_items[win->num_session_items] = XtVaCreateManagedWidget(
						 label, xmPushButtonGadgetClass,
						 win->session_menu, NULL);
      XtAddCallback(win->session_items[win->num_session_items],
		    XmNactivateCallback, session_cb, url);
      XtAddCallback(win->session_items[win->num_session_items],
		    XmNarmCallback, rbm_ballonify, url);
      XtAddCallback(win->session_items[win->num_session_items],
		    XmNdisarmCallback, rbm_ballonify, " ");
      win->num_session_items++;
  } else if (win && win->session_items) {
      int i;
      int smax = max - 1;

      XtDestroyWidget(win->session_items[0]);

      /* Scoot the widget pointers */
      for (i = 0; i < smax; i++)
	  win->session_items[i] = win->session_items[i + 1];

      win->session_items[smax] = XtVaCreateManagedWidget(label,
						       xmPushButtonGadgetClass,
						       win->session_menu, NULL);
      XtAddCallback(win->session_items[smax],
		    XmNactivateCallback, session_cb, url);
      XtAddCallback(win->session_items[smax],
		    XmNarmCallback, rbm_ballonify, url);
      XtAddCallback(win->session_items[smax],
		    XmNdisarmCallback, rbm_ballonify, " ");
  }
}
