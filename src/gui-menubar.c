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

/* Copyright (C) 1998, 1999, 2000, 2002, 2003, 2004, 2005, 2006, 2007
 * The VMS Mosaic Project
 */

#include "../config.h"
#include "../libwww2/HTNews.h"
#include "../libwww2/HTAlert.h"
#include "mosaic.h"
#include "main.h"
#include "../libhtmlw/HTMLp.h"
#include "../libhtmlw/HTMLfont.h"
#include "gui.h"
#include "grpan.h"
#include "gui-ftp.h"
#include "gui-popup.h"  /* For callback struct definition */
#include "gui-dialogs.h"
#include "gui-news.h"
#ifdef CCI
#include "cci.h"
#include "cciBindings.h"
#include "cciBindings2.h"
extern int cci_event;
#endif
#include "history.h"
#include "pan.h"
#include "annotate.h"
#include "mo-www.h"
#include "globalhist.h"
#include "proxy.h"
#include "proxy-dialogs.h"
#include "gui-documents.h"
#include "gui-extras.h"
#include "comment.h"

#include <sys/types.h>
#ifndef WIN_TCP
#if defined(__DECC) && (__VMS_VER >= 70000000)
#define _VMS_V6_SOURCE
#endif /* Avoid __UTC_STAT in VMS V7.0, GEC */
#include <sys/stat.h>
#if defined(__DECC) && (__VMS_VER >= 70000000)
#undef _VMS_V6_SOURCE
#endif
#else
#include "sys$library:stat.h"
#endif /* WIN_TCP */

#define __SRC__
#include "../libwww2/HTAAUtil.h"

extern int tableSupportEnabled;
extern int imageViewInternal;

extern int progressiveDisplayEnabled;

#ifdef HAVE_SSL
/* From libwww2 http.c */
extern int verifyCertificates;
#endif

/* Spoof Agents Stuff */
extern int selectedAgent;
extern char **agent;

extern mo_root_hotlist *default_hotlist;

#ifndef DISABLE_TRACE
extern int cacheTrace;
#ifdef CCI
extern int cciTrace;
#endif
extern int cookieTrace;
extern int htmlwTrace;
extern int httpTrace;
extern int nutTrace;
extern int srcTrace;
extern int tableTrace;
extern int www2Trace;
extern int refreshTrace;
extern int reportBugs;
#endif

extern Widget toplevel;
extern mo_window *current_win;
extern Widget popup_shell;

extern char *home_document;

#ifndef PRERELEASE
extern char do_comment;
#endif
static Widget exitbox = NULL;

#define MAX_DOCUMENTS_MENU_ITEMS 120
#define DOCUMENTS_MENU_COUNT_OFFSET 5000
/* List of URL's matching items in documents menu. */
static char *urllist[MAX_DOCUMENTS_MENU_ITEMS];


/* --------------------------- mo_post_exitbox ---------------------------- */

static XmxCallback(exit_confirm_cb)
{
  if (XmxExtractToken((int)client_data)) {
      mo_exit();
  } else {
      XtUnmanageChild(w);
  }
  return;
}

static void mo_post_exitbox(void)
{
  if (get_pref_boolean(eCONFIRM_EXIT)) {
      if (!exitbox) {
	  if (!popup_shell)
	      popup_shell = XtCreatePopupShell("popup_shell",
					       topLevelShellWidgetClass,
					       toplevel, NULL, 0);
          exitbox = XmxMakeQuestionDialog(popup_shell,
				        "Are you sure you want to exit Mosaic?",
				        "VMS Mosaic: Exit Confirmation",
				        exit_confirm_cb, 1, 0);
          XtManageChild(exitbox);
      } else {
          XmxManageRemanage(exitbox);
      }
  } else {
      /* Don't confirm exit; just zap it. */
      mo_exit();
  }
  return;
}

/* -------------------- mo_set_fancy_selections_toggle -------------------- */

mo_status mo_set_fancy_selections_toggle(mo_window *win)
{
  XmxRSetToggleState(win->menubar, mo_fancy_selections,
                     win->pretty ? XmxSet : XmxNotSet);
  return mo_succeed;
}

/* ---------------------------- mo_set_fonts ---------------------------- */

mo_status mo_set_fonts(mo_window *win, int size)
{
  /* A fake pcc for SetFontSize */
  PhotoComposeContext pcc;
  HTMLWidget hw = (HTMLWidget)win->scrolled_win;

  pcc.cur_font_type = FONT;

  switch (size) {
    case mo_large_fonts:
      pcc.cur_font_size = pcc.cur_font_base = 5;
      pcc.cur_font_family = TIMES;
      win->font_family = 0;
      break;
    case mo_regular_fonts:
      pcc.cur_font_size =  pcc.cur_font_base = 3;
      pcc.cur_font_family = TIMES;
      win->font_family = 0;
      break;
    case mo_small_fonts:
      pcc.cur_font_size = pcc.cur_font_base = 2;
      pcc.cur_font_family = TIMES;
      win->font_family = 0;
      break;
    case mo_large_helvetica:
      pcc.cur_font_size = pcc.cur_font_base = 5;
      pcc.cur_font_family = HELVETICA;
      win->font_family = 1;
      break;
    case mo_regular_helvetica:
      pcc.cur_font_size = pcc.cur_font_base = 3;
      pcc.cur_font_family = HELVETICA;
      win->font_family = 1;
      break;
    case mo_small_helvetica:
      pcc.cur_font_size = pcc.cur_font_base = 2;
      pcc.cur_font_family = HELVETICA;
      win->font_family = 1;
      break;
    case mo_large_newcentury:
      pcc.cur_font_size = pcc.cur_font_base = 5;
      pcc.cur_font_family = CENTURY;
      win->font_family = 2;
      break;
    case mo_small_newcentury:
      pcc.cur_font_size = pcc.cur_font_base = 2;
      pcc.cur_font_family = CENTURY;
      win->font_family = 2;
      break;
    case mo_regular_newcentury:
      pcc.cur_font_size = pcc.cur_font_base = 3;
      pcc.cur_font_family = CENTURY;
      win->font_family = 2;
      break;
    case mo_large_lucidabright:
      pcc.cur_font_size = pcc.cur_font_base = 5;
      pcc.cur_font_family = LUCIDA;
      win->font_family = 3;
      break;
    case mo_regular_lucidabright:
      pcc.cur_font_size = pcc.cur_font_base = 3;
      pcc.cur_font_family = LUCIDA;
      win->font_family = 3;
      break;
    case mo_small_lucidabright:
      pcc.cur_font_size = pcc.cur_font_base = 2;
      pcc.cur_font_family = LUCIDA;
      win->font_family = 3;
      break;
    default:
      pcc.cur_font_size = pcc.cur_font_base = 3;
      pcc.cur_font_family = TIMES;
      win->font_family = 0;
  }
  SetFontSize(hw, &pcc, 1);

  if (!win->is_frame) {
      XmxRSetToggleState(win->menubar, win->font_size, XmxNotSet);
      XmxRSetToggleState(win->menubar, size, XmxSet);
  }
  win->font_size = size;

  return mo_succeed;
}

/* -------------------------- mo_set_underlines --------------------------- */

mo_status mo_set_underlines(mo_window *win, int choice)
{
  if (!win->underlines_snarfed) {
      XtVaGetValues(win->scrolled_win,
                    WbNanchorUnderlines, &win->underlines,
                    WbNvisitedAnchorUnderlines, &win->visited_underlines,
                    WbNdashedAnchorUnderlines, &win->dashed_underlines,
                    WbNdashVisitedAnchorUnderlines, 
                    &win->dashed_visited_underlines,
                    NULL);
      win->underlines_snarfed = 1;
  }

  switch (choice) {
    case mo_default_underlines:
      XmxSetArg(WbNanchorUnderlines, win->underlines);
      XmxSetArg(WbNvisitedAnchorUnderlines, win->visited_underlines);
      XmxSetArg(WbNdashedAnchorUnderlines, win->dashed_underlines);
      XmxSetArg(WbNdashVisitedAnchorUnderlines, 
                win->dashed_visited_underlines);
      break;
    case mo_l1_underlines:
      XmxSetArg(WbNanchorUnderlines, 1);
      XmxSetArg(WbNvisitedAnchorUnderlines, 1);
      XmxSetArg(WbNdashedAnchorUnderlines, False);
      XmxSetArg(WbNdashVisitedAnchorUnderlines, True);
      break;
    case mo_l2_underlines:
      XmxSetArg(WbNanchorUnderlines, 1);
      XmxSetArg(WbNvisitedAnchorUnderlines, 1);
      XmxSetArg(WbNdashedAnchorUnderlines, False);
      XmxSetArg(WbNdashVisitedAnchorUnderlines, False);
      break;
    case mo_l3_underlines:
      XmxSetArg(WbNanchorUnderlines, 2);
      XmxSetArg(WbNvisitedAnchorUnderlines, 1);
      XmxSetArg(WbNdashedAnchorUnderlines, False);
      XmxSetArg(WbNdashVisitedAnchorUnderlines, False);
      break;
    case mo_no_underlines:
      XmxSetArg(WbNanchorUnderlines, 0);
      XmxSetArg(WbNvisitedAnchorUnderlines, 0);
      XmxSetArg(WbNdashedAnchorUnderlines, False);
      XmxSetArg(WbNdashVisitedAnchorUnderlines, False);
      break;
    default:
      return mo_fail;
  }
  XmxSetValues(win->scrolled_win);

  if (!win->is_frame) {
      XmxRSetToggleState(win->menubar, win->underlines_state, XmxNotSet);
      XmxRSetToggleState(win->menubar, choice, XmxSet);
      win->underlines_state = choice;
  }
  return mo_succeed;
}

/* --------------------------- exit_confirm_cb ---------------------------- */

static XmxCallback(clear_history_confirm_cb)
{
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
  
  if (XmxExtractToken((int)client_data)) {
      mo_window *next = NULL;

      mo_wipe_global_history(win);

      while (next = mo_next_window(next))
          mo_redisplay_window(next);
  }
  XtDestroyWidget(w);
}

/* ----------------------- mo_do_delete_annotation ------------------------ */

/* Presumably we're on an annotation. */
static mo_status mo_do_delete_annotation(mo_window *win)
{
  if (!win->current_node)
      return mo_fail;

  if (win->current_node->annotation_type == mo_annotation_private) {
      char *author, *title, *text, *fname;
      int id;

      mo_grok_pan_pieces(win->current_node->url, win->current_node->text,
                         &title, &author, &text, &id, &fname);
      mo_delete_annotation(win, id);
  } else if (win->current_node->annotation_type == mo_annotation_workgroup) {
      mo_delete_group_annotation(win, win->current_node->url);
  }
  return mo_succeed;
}

static XmxCallback(delete_annotation_confirm_cb)
{
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

#if defined(VAXC) && !defined(__DECC)
  /* Avoid VAX C compiler bug */
  if (win->current_node) {
      if (mo_is_editable_annotation(win, win->current_node->text)) {
          if (XmxExtractToken((int)client_data))
              mo_do_delete_annotation(win);
      }
  }
#else
  if (win->current_node &&
      mo_is_editable_annotation(win, win->current_node->text) &&
      XmxExtractToken((int)client_data))
      mo_do_delete_annotation(win);
#endif
  XtDestroyWidget(w);
}


/* --------------------------agent menubar_cb ------------------------------ */

void mo_set_agents(mo_window *win, int which)
{
  XmxRSetToggleState(win->menubar, win->agent_state, XmxNotSet);
  XmxRSetToggleState(win->menubar, which, XmxSet);
  win->agent_state = which;
  selectedAgent = which-mo_last_entry;
}


static XmxCallback(agent_menubar_cb)
{
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

  mo_set_agents(win, XmxExtractToken((int)client_data));
  return;
}
  

/* ------------------------------ menubar_cb ------------------------------ */

XmxCallback(menubar_cb)
{
  ElemInfo *eptr = NULL;
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
  int i = XmxExtractToken((int)client_data);
  char *grp;
  char buf[512];
  
  if (!win) {		/* This may be from the popup menu */
      act_struct *acst = (act_struct *) client_data;

      if (!acst)
          return;
      i = acst->act_code;
      win = current_win;
      eptr = acst->eptr;
  }

  switch (i) {
    case mo_stop:
      mo_stop_it(win);
      break;

    case mo_reload_document:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(MOSAIC_RELOAD_CURRENT);
#endif
      mo_reload_window_text(win, 0);
      break;

    case mo_reload_document_and_images:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(FILE_RELOAD_IMAGES);
#endif
      mo_reload_window_text(win, 1);
      break;

    case mo_refresh_document:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(FILE_REFRESH_CURRENT);
#endif
      mo_refresh_window_text(win);
      break;

    case mo_re_init:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_RELOAD_CONFIG_FILES);
#endif
      read_preferences_file(NULL);
      mo_re_init_formats();
      break;

    case mo_clear_image_cache:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_FLUSH_IMAGE_CACHE);
#endif
      XmUpdateDisplay(win->base);
      mo_flush_image_cache();
      /* Force a complete reload...nothing else we can do */
      mo_reload_window_text(win, 1);
      break;

    case mo_clear_passwd_cache:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_FLUSH_PASSWD_CACHE);
#endif
      mo_flush_passwd_cache(win);
      break;

#ifdef CCI
    case mo_cci:
      if (cci_event) MoCCISendEventOutput(FILE_CCI);
      MoDisplayCCIWindow(win);
      break;
#endif

    case mo_document_source:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(FILE_VIEW_SOURCE);
#endif
      mo_post_source_window(win);
      break;

    case mo_document_edit:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(FILE_EDIT_SOURCE);
#endif
      mo_edit_source(win);
      break;

    case mo_search:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(FILE_FIND_IN_CURRENT);
#endif
      mo_post_search_window(win);
      break;

    case mo_open_document:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(MOSAIC_OPEN_URL);
#endif
      mo_post_open_window(win);
      break;

    case mo_open_local_document:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(FILE_OPEN_LOCAL);
#endif
      mo_post_open_local_window(win);
      break;

    case mo_save_document:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(MOSAIC_SAVE_AS);
#endif
      mo_post_save_window(win);
      break;

    case mo_mail_document:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(FILE_MAIL_TO);
#endif
      mo_post_mail_window(win);
      break;

    case mo_print_document:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(FILE_PRINT);
#endif
      mo_post_print_window(win);
      break;

    case mo_new_window:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(MOSAIC_NEW);
#endif
      mo_open_another_window(win, home_document, NULL, NULL);
      break;

    case mo_clone_window:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(MOSAIC_CLONE);
#endif
      mo_duplicate_window(win);
      break;

    case mo_close_window:
      mo_gui_notify_progress("Destroying Window - Please Wait");
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(MOSAIC_CLOSE);
#endif
      mo_delete_window(win);
      break;

    case mo_exit_program:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(FILE_EXIT_PROGRAM);
#endif
      mo_post_exitbox();
      break;

#ifdef KRB4
    case mo_kerberosv4_login:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(FILE_KERBEROS_V4_LOGIN);
#endif
      scheme_login(HTAA_KERBEROS_V4);
      break;
#endif

#ifdef KRB5
    case mo_kerberosv5_login:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(FILE_KERBEROS_V5_LOGIN);
#endif
      scheme_login(HTAA_KERBEROS_V5);
      break;
#endif

    case mo_proxy:
      ShowProxyDialog(win);
      break;

    case mo_no_proxy:
      ShowNoProxyDialog(win);
      break;

    case mo_home_document:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(MOSAIC_HOME_DOCUMENT);
#endif
      mo_access_document(win, home_document);
      break;

    case mo_network_search:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NAVIGATE_SEARCH);
#endif
      mo_access_document(win, NETWORK_SEARCH_DEFAULT);
      break;

    case mo_usenet_search:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NAVIGATE_USENET_SEARCH);
#endif
      mo_access_document(win, USENET_SEARCH_DEFAULT);
      break;

    case mo_people_search:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NAVIGATE_PEOPLE_SEARCH);
#endif
      mo_access_document(win, PEOPLE_SEARCH_DEFAULT);
      break;

    case mo_meta_search:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NAVIGATE_META_SEARCH);
#endif
      mo_access_document(win, META_SEARCH_DEFAULT);
      break;

    case mo_internet_metaindex:
#ifdef CCI
      if (cci_event) 
	  MoCCISendEventOutput(NAVIGATE_INTERNET_RESOURCES_META_INDEX);
#endif
      mo_access_document(win, INTERNET_METAINDEX_DEFAULT);
      break;

    case mo_list_search:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NAVIGATE_LIST_SEARCH);
#endif
      mo_access_document(win, LIST_SEARCH_DEFAULT);
      break;

    case mo_map_search:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NAVIGATE_MAP_SEARCH);
#endif
      mo_access_document(win, MAP_SEARCH_DEFAULT);
      break;

    case mo_auction_search:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NAVIGATE_AUCTION_SEARCH);
#endif
      mo_access_document(win, AUCTION_SEARCH_DEFAULT);
      break;

    case mo_encyclopedia_search:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NAVIGATE_ENCYCLOPEDIA_SEARCH);
#endif
      mo_access_document(win, ENCYCLOPEDIA_SEARCH_DEFAULT);
      break;

    case mo_mosaic_manual:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(HELP_MANUAL);
#endif
      mo_open_another_window(win,
	 "http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/Docs/UserGuide/XMosaic.0.html",
         NULL, NULL);
      break;

    case mo_cookie_manager:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPEN_COOKIEJAR);
#endif
      mo_open_another_window(win, "cookiejar:", NULL, NULL);
      break;

    case mo_back:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(MOSAIC_BACK);
#endif
      mo_back_node(win);
      break;

    case mo_forward:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(MOSAIC_FORWARD);
#endif
      mo_forward_node(win);
      break;

    case mo_history_list:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NAVIGATE_WINDOW_HISTORY);
#endif
      mo_post_history_win(win);
      break;

    case mo_clear_global_history:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_CLEAR_GLOBAL_HISTORY);
#endif
      XmxSetUniqid(win->id);
      XmxMakeQuestionDialog(win->base,
			   "Are you sure you want to clear the global history?",
			   "VMS Mosaic: Clear Global History",
			   clear_history_confirm_cb, 1, 0);
      XtManageChild(Xmx_w);
      break;

    case mo_hotlist_postit:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NAVIGATE_HOTLIST);
#endif
      mo_post_hotlist_win(win);
      break;

    case mo_register_node_in_default_hotlist:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NAVIGATE_ADD_CURRENT_TO_HOTLIST);
#endif
      if (win->current_node) {
          mo_add_node_to_current_hotlist(win);
          mo_write_default_hotlist();
      }
      break;

    case mo_all_hotlist_to_rbm:
      if (!win->hotlist_win)
	  win->current_hotlist = (mo_hotlist *)default_hotlist;
      mo_rbm_myself_to_death(win, 1);
      break;

    case mo_all_hotlist_from_rbm:
      if (!win->hotlist_win)
	  win->current_hotlist = (mo_hotlist *)default_hotlist;
      mo_rbm_myself_to_death(win, 0);
      break;

    /* Removed 5/17/96 - bjs 
    case mo_fancy_selections:
      win->pretty = 1 - win->pretty;
      mo_set_fancy_selections_toggle(win);
      HTMLClearSelection(win->scrolled_win);
      XmxSetArg(WbNfancySelections, win->pretty ? True : False);
      XmxSetValues(win->scrolled_win);
      if (cci_event) {
	  if (win->pretty) {
	      MoCCISendEventOutput(OPTIONS_FANCY_SELECTIONS_ON);
	  } else {
	      MoCCISendEventOutput(OPTIONS_FANCY_SELECTIONS_OFF);
	  }
      }
      break;
    */
    case mo_preferences:
      win->preferences = win->preferences ? 0 : 1;
      set_pref_boolean(eUSE_PREFERENCES,
		       get_pref_boolean(eUSE_PREFERENCES) ? 0 : 1);
      write_preferences_file(NULL);
      break;

    case mo_save_preferences:
      write_preferences_file(NULL);
      break;

    case mo_table_support:
      tableSupportEnabled = win->table_support = win->table_support ? 0 : 1;
      set_pref_boolean(eENABLE_TABLES, win->table_support);
      break;

    case mo_frame_support:
      win->frame_support = win->frame_support ? 0 : 1;
      XtVaSetValues(win->scrolled_win,
		    WbNframeSupport, win->frame_support,
		    NULL);
      set_pref_boolean(eFRAME_SUPPORT, win->frame_support);
      break;

    case mo_hotkeys:
      win->hotkeys = win->hotkeys ? 0 : 1;
      set_pref_boolean(eHOTKEYS, win->hotkeys);
      break;

    case mo_multi_load:
      win->multi_image_load = win->multi_image_load ? 0 : 1;
      set_pref_boolean(eMULTIPLE_IMAGE_LOAD, win->multi_image_load);
      break;

    case mo_tooltips:
      {
	  Boolean active = !XmxClueIsActive();

	  XmxClueActive(active);
	  XmxRSetToggleState(win->menubar, mo_tooltips,
			     active ? XmxSet : XmxNotSet);
	  set_pref_boolean(eCLUE_HELP, active);
      }
      break;

    case mo_progressive_loads:
      progressiveDisplayEnabled = win->progressive_loads =
				  	         win->progressive_loads ? 0 : 1;
      set_pref_boolean(ePROGRESSIVE_DISPLAY, win->progressive_loads);
      break;

    case mo_animate_images:
      win->image_animation = win->image_animation ? 0 : 1;
      set_pref_boolean(eIMAGE_ANIMATION, win->image_animation);
      break;

    case mo_blink_text:
      win->blink_text = win->blink_text ? 0 : 1;
      XtVaSetValues(win->scrolled_win,
		    WbNblinkingText, win->blink_text,
		    NULL);
      set_pref_boolean(eBLINKING_TEXT, win->blink_text);
      break;

    case mo_safe_colors:
      win->safe_colors = win->safe_colors ? 0 : 1;
      set_pref_boolean(eBROWSER_SAFE_COLORS, win->safe_colors);
      break;

    case mo_body_color:
      win->body_color = win->body_color ? 0 : 1;
      XtVaSetValues(win->scrolled_win,
		    WbNbodyColors, win->body_color,
		    NULL);
      set_pref_boolean(eBODYCOLORS, win->body_color);
      break;

    case mo_body_images:
      win->body_images = win->body_images ? 0 : 1;
      XtVaSetValues(win->scrolled_win,
		    WbNbodyImages, win->body_images,
	            NULL);
      set_pref_boolean(eBODYIMAGES, win->body_images);
      break;

    case mo_font_color:
      win->font_color = win->font_color ? 0 : 1;
      XtVaSetValues(win->scrolled_win,
		    WbNfontColors, win->font_color,
		    NULL);
      set_pref_boolean(eFONTCOLORS, win->font_color);
      break;

    case mo_font_sizes:
      win->font_sizes = win->font_sizes ? 0 : 1;
      XtVaSetValues(win->scrolled_win,
		    WbNfontSizes, win->font_sizes,
		    NULL);
      set_pref_boolean(eFONTSIZES, win->font_sizes);
      break;

    case mo_binary_transfer:
      win->binary_transfer = win->binary_transfer ? 0 : 1;
#ifdef CCI
      if (cci_event) {
	  if (win->binary_transfer) {
	      MoCCISendEventOutput(OPTIONS_LOAD_TO_LOCAL_DISK_ON);  
	  } else {
	      MoCCISendEventOutput(OPTIONS_LOAD_TO_LOCAL_DISK_OFF);
	  }
      }
#endif
      break;

    case mo_binary_ftp_mode:
      win->binary_ftp_mode = win->binary_ftp_mode ? 0 : 1;
      set_pref_boolean(eFTP_BINARY_MODE, win->binary_ftp_mode);
#ifdef CCI
      if (cci_event) {
	  if (win->binary_ftp_mode) {
	      MoCCISendEventOutput(OPTIONS_BINARY_FTP_MODE_ON);  
	  } else {
	      MoCCISendEventOutput(OPTIONS_BINARY_FTP_MODE_OFF);
	  }
      }
#endif
      break;

    case mo_delay_image_loads:
      win->delay_image_loads = win->delay_image_loads ? 0 : 1;
      XmxSetArg(WbNdelayImageLoads, win->delay_image_loads ? True : False);
      XmxSetValues(win->scrolled_win);
      XmxRSetSensitive(win->menubar, mo_expand_images_current,
                       win->delay_image_loads ? XmxSensitive : XmxNotSensitive);
      set_pref_boolean(eDELAY_IMAGE_LOADS, win->delay_image_loads);
#ifdef CCI
      if (cci_event) {
	  if (win->delay_image_loads) {
	      MoCCISendEventOutput(OPTIONS_DELAY_IMAGE_LOADING_ON);
	  } else {
	      MoCCISendEventOutput(OPTIONS_DELAY_IMAGE_LOADING_OFF);
	  }
      }
#endif
      break;

    case mo_expand_images_current:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_LOAD_IMAGES_IN_CURRENT);
#endif
      XmxSetArg(WbNdelayImageLoads, False);
      XmxSetValues(win->scrolled_win);
      {
	int tmp = win->delay_image_loads;

      	win->delay_image_loads = 0;
      	mo_refresh_window_text(win);
      	win->delay_image_loads = tmp;
      }
      XmxSetArg(WbNdelayImageLoads, win->delay_image_loads ? True : False);
      XmxSetValues(win->scrolled_win);
      break;

    case mo_image_view_internal:
      imageViewInternal = win->image_view_internal =
			  		       win->image_view_internal ? 0 : 1;
      set_pref_boolean(eIMAGEVIEWINTERNAL, win->image_view_internal);
      break;

    case mo_refresh_url:
      win->refresh_url = win->refresh_url ? 0 : 1;
      set_pref_boolean(eREFRESH_URL, win->refresh_url);
      break;

#ifdef HAVE_SSL
    case mo_verify_certs:
      verifyCertificates = verifyCertificates ? 0 : 1;
      set_pref_boolean(eVERIFY_SSL_CERTIFICATES, verifyCertificates);
#ifdef CCI
      if (cci_event) {
	  if (win->verify_certs) {
	      MoCCISendEventOutput(OPTIONS_VERIFY_CERTS_ON);  
	  } else {
	      MoCCISendEventOutput(OPTIONS_VERIFY_CERTS_OFF);
	  }
      }
#endif
      break;
#endif

    case mo_large_fonts:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_TL);
#endif
      mo_set_fonts(win, i);
      set_pref(eDEFAULT_FONT_CHOICE, (void *) "TimesLarge");
      break;

    case mo_regular_fonts:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_TR);
#endif
      mo_set_fonts(win, i);
      set_pref(eDEFAULT_FONT_CHOICE, (void *) "TimesRegular");
      break;

    case mo_small_fonts:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_TS);
#endif
      mo_set_fonts(win, i);
      set_pref(eDEFAULT_FONT_CHOICE, (void *) "TimesSmall");
      break;

    case mo_large_helvetica:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_HL);
#endif
      mo_set_fonts(win, i);
      set_pref(eDEFAULT_FONT_CHOICE, (void *) "HelveticaLarge");
      break;

    case mo_regular_helvetica:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_HR);
#endif
      mo_set_fonts(win, i);
      set_pref(eDEFAULT_FONT_CHOICE, (void *) "HelveticaRegular");
      break;

    case mo_small_helvetica:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_HS);
#endif
      mo_set_fonts(win, i);
      set_pref(eDEFAULT_FONT_CHOICE, (void *) "HelveticaSmall");
      break;

    case mo_large_newcentury:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_NCL);
#endif
      mo_set_fonts(win, i);
      set_pref(eDEFAULT_FONT_CHOICE, (void *) "NewCenturyLarge");
      break;

    case mo_regular_newcentury:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_NCR);
#endif
      mo_set_fonts(win, i);
      set_pref(eDEFAULT_FONT_CHOICE, (void *) "NewCenturyRegular");
      break;

    case mo_small_newcentury:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_NCS);
#endif
      mo_set_fonts(win, i);
      set_pref(eDEFAULT_FONT_CHOICE, (void *) "NewCenturySmall");
      break;

    case mo_large_lucidabright:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_LBL);
#endif
      mo_set_fonts(win, i);
      set_pref(eDEFAULT_FONT_CHOICE, (void *) "LucidaBrightLarge");
      break;

    case mo_regular_lucidabright:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_LBR);
#endif
      mo_set_fonts(win, i);
      set_pref(eDEFAULT_FONT_CHOICE, (void *) "LucidaBrightRegular");
      break;

    case mo_small_lucidabright:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_FONTS_LBS);
#endif
      mo_set_fonts(win, i);
      set_pref(eDEFAULT_FONT_CHOICE, (void *) "LucidaBrightSmall");
      break;

    case mo_default_underlines:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_ANCHOR_UNDERLINES_DU);
#endif
      mo_set_underlines(win, i);
      set_pref(eDEFAULTUNDERLINES, (void *) "Default");
      break;

    case mo_l1_underlines:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_ANCHOR_UNDERLINES_LU);
#endif
      mo_set_underlines(win, i);
      set_pref(eDEFAULTUNDERLINES, (void *) "Light");
      break;

    case mo_l2_underlines:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_ANCHOR_UNDERLINES_MU);
#endif
      mo_set_underlines(win, i);
      set_pref(eDEFAULTUNDERLINES, (void *) "Medium");
      break;

    case mo_l3_underlines:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_ANCHOR_UNDERLINES_HU);
#endif
      mo_set_underlines(win, i);
      set_pref(eDEFAULTUNDERLINES, (void *) "Heavy");
      break;

    case mo_no_underlines:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(OPTIONS_ANCHOR_UNDERLINES_NU);
#endif
      mo_set_underlines(win, i);
      set_pref(eDEFAULTUNDERLINES, (void *) "No");
      break;

    case mo_help_about:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(HELP_ABOUT);
#endif
      mo_open_another_window(win, mo_assemble_help_url("d2-userguide.html"),
			     NULL, NULL);
      break;

    case mo_help_onwindow:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(HELP_ON_WINDOW);
#endif
      mo_open_another_window(win,
			    mo_assemble_help_url("help-on-docview-window.html"),
			    NULL, NULL);
      break;

    case mo_help_onversion:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(HELP_ON_VERSION);
#endif
      mo_open_another_window(win, MO_HELP_ON_VERSION_DOCUMENT, NULL, NULL);
      break;

    case mo_help_vmsmosaic:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(HELP_ON_VMS_VERSION);
#endif
      mo_open_another_window(win,
		             "http://vaxa.wvnet.edu/vmswww/vms_mosaic.html",
			     NULL, NULL);
      break;

    case mo_help_faq:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(HELP_ON_FAQ);
#endif
      mo_open_another_window(win, mo_assemble_help_url("mosaic-faq.html"), 
                             NULL, NULL);
      break;

    case mo_help_html:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(HELP_ON_HTML);
#endif
      mo_open_another_window(win, HTMLPRIMER_PAGE_DEFAULT, NULL, NULL);
      break;

    case mo_help_url:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(HELP_ON_URLS);
#endif
      mo_open_another_window(win, URLPRIMER_PAGE_DEFAULT, NULL, NULL);
      break;

#ifndef PRERELEASE
    case mo_cc:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(HELP_COMMENT_CARD);
#endif
      do_comment = 1;
      CommentCard(win);
      break;
#endif

    case mo_techsupport:        
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(HELP_MAIL_TECH_SUPPORT);
#endif
      {
          char subj[128];

          sprintf(subj, "User Feedback -- VMS Mosaic %s on %s.",
                  MO_VERSION_STRING, MO_MACHINE_TYPE);
          mo_post_mailto_win(MO_DEVELOPER_ADDRESS, subj);
      }
      break;

    case mo_annotate:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(ANNOTATE_ANNOTATE);
#endif
      mo_post_annotate_win(win, 0, 0, NULL, NULL, NULL, NULL);
      break;

    case mo_news_prev:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NEWS_PREV);
#endif
      gui_news_prev(win);
      break;

    case mo_news_next:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NEWS_NEXT);
#endif
      gui_news_next(win);
      break;

    case mo_news_prevt:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NEWS_PREV_THREAD);
#endif
      gui_news_prevt(win);
      break;

    case mo_news_nextt:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NEWS_NEXT_THREAD);
#endif
      gui_news_nextt(win);
      break;

    case mo_news_index:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NEWS_INDEX);
#endif
      gui_news_index(win);
      break;

    case mo_news_flush:
      gui_news_flush(win);
      break;

    case mo_news_flushgroup:
      gui_news_flushgroup(win);
      break;

    case mo_news_sub_anchor:
      if (NewsGroupS) {
	  gui_news_subgroup(win);
	  break;
      }
      if (!eptr)
	  break;
      grp = &eptr->anchor_tag_ptr->anc_href[5];
      subscribegroup(grp);
      sprintf(buf, "%s successfully subscribed", grp);
      mo_gui_notify_progress(buf);
      break;

    case mo_news_sub:
      gui_news_subgroup(win);
      break;

    case mo_news_unsub_anchor:
      if (NewsGroupS) {
	  gui_news_unsubgroup(win);
	  break;
      }
      if (!eptr)
	  break;
      grp = &eptr->anchor_tag_ptr->anc_href[5];
      unsubscribegroup(grp);
      sprintf(buf, "%s successfully unsubscribed", grp);
      mo_gui_notify_progress(buf);
      break;

    case mo_news_unsub:
      gui_news_unsubgroup(win);
      break;

    case mo_news_grp0:
      gui_news_showAllGroups(win);
      break;

    case mo_news_grp1:
      gui_news_showGroups(win);
      break;

    case mo_news_grp2:
      gui_news_showReadGroups(win);
      break;

    case mo_news_art0:
      gui_news_showAllArticles(win);
      break;

    case mo_news_art1:
      gui_news_showArticles(win);
      break;

    case mo_news_mread_anchor:
      if (NewsGroupS)
	  gui_news_markGroupRead(win);
      if (!eptr)
	  break;
      grp = &eptr->anchor_tag_ptr->anc_href[5];
      NewsGroupS = findgroup(grp);
      if (!NewsGroupS)
	  break;
      markrangeread(NewsGroupS, NewsGroupS->minart, NewsGroupS->maxart);
      sprintf(buf, "All articles in %s marked read", NewsGroupS->name);
      mo_gui_notify_progress(buf);
      NewsGroupS = NULL;
      /* Return to newsgroup list */
      mo_load_window_text(win, "news:*", NULL);
      break;
      
    case mo_news_mread:
      gui_news_markGroupRead(win);
      break;

    case mo_news_munread:
      gui_news_markGroupUnread(win);
      break;

    case mo_news_maunread:
      gui_news_markArticleUnread(win);
      break;

    case mo_news_groups:
    case mo_news_list:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NEWS_LIST_GROUPS);
#endif
      gui_news_list(win);
      break;

    case mo_news_fmt0:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NEWS_FORMAT_TV);
#endif
      HTSetNewsConfig(1, -1, -1, -1, -1, -1, -1, -1);
      XmxRSetToggleState(win->menubar, mo_news_fmt1, XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_news_fmt0, XmxSet);
      mo_reload_window_text(win, 0);
      break;

    case mo_news_fmt1:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NEWS_FORMAT_GV);
#endif
      HTSetNewsConfig(0, -1, -1, -1, -1, -1, -1, -1);
      XmxRSetToggleState(win->menubar, mo_news_fmt0, XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_news_fmt1, XmxSet);
      mo_reload_window_text(win, 0);
      break;

    case mo_news_post:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NEWS_POST);
#endif
      mo_post_news_win(win);
      break;

    case mo_news_follow:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(NEWS_FOLLOW_UP);
#endif
      mo_post_follow_win(win);
      break;

      /* Handle FTP stuff here */
    case mo_ftp_put:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(FTP_PUT);
#endif
      mo_handle_ftpput(win);
      break;

    case mo_ftp_mkdir:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(FTP_MKDIR);
#endif
      mo_handle_ftpmkdir(win);
      break;

    /* Tag and Bag */
    /*
    case mo_tag_current:
      mo_tagnbag_current(win);
      break;

    case mo_tag_url:
      mo_tagnbag_url(win);
      break;
    */

    case mo_links_window:
      mo_post_links_window(win);
      break;

#ifdef HAVE_AUDIO_ANNOTATIONS
    case mo_audio_annotate:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(ANNOTATE_AUDIO_ANNOTATE);
#endif
      mo_post_audio_annotate_win(win);
      break;
#endif

    case mo_annotate_edit:
      /* OK, let's be smart.
       * If we get here, we know we're viewing an editable annotation.
       * We also know the filename (just strip the leading
       * file: off the URL).
       * We also know the ID, by virtue of the filename
       * (just look for PAN-#.html. */
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(ANNOTATE_EDIT_THIS_ANNOTATION);
#endif
      if (win->current_node) {
          char *author, *title, *text, *fname;
          int id;
          
          if (win->current_node->annotation_type == mo_annotation_private) {
              mo_grok_pan_pieces(win->current_node->url,
                                 win->current_node->text,
                                 &title, &author, &text, &id, &fname);
              mo_post_annotate_win(win, 1, id, title, author, text, fname);
          } else if (win->current_node->annotation_type ==
		     mo_annotation_workgroup) {
              mo_grok_grpan_pieces(win->current_node->url,
                                   win->current_node->text,
                                   &title, &author, &text, &id, &fname);
              mo_post_annotate_win(win, 1, id, title, author, text, fname);
          }
      }
      break;

    case mo_annotate_delete:
#ifdef CCI
      if (cci_event) MoCCISendEventOutput(ANNOTATE_DELETE_THIS_ANNOTATION);
#endif
      if (get_pref_boolean(eCONFIRM_DELETE_ANNOTATION)) {
          XmxSetUniqid(win->id);
          XmxMakeQuestionDialog(win->base,
			     "Are you sure you want to delete this annotation?",
			     "VMS Mosaic: Delete Annotation",
			     delete_annotation_confirm_cb, 1, 0);
          XtManageChild(Xmx_w);
      } else {
          mo_do_delete_annotation(win);
      }
      break;

#ifndef DISABLE_TRACE
    /* These are global across all windows, so need to set their GUI
     * states in case changed by another window.
     */

    case mo_trace_cache:
      cacheTrace = cacheTrace ? 0 : 1;
      set_pref_boolean(eCACHETRACE, cacheTrace);
      XmxRSetToggleState(win->menubar, mo_trace_cache,
			 cacheTrace ? XmxSet : XmxNotSet);
      break;

#ifdef CCI
    case mo_trace_cci:
      cciTrace = cciTrace ? 0 : 1;
      set_pref_boolean(eCCITRACE, cciTrace);
      XmxRSetToggleState(win->menubar, mo_trace_cci,
			 cciTrace ? XmxSet : XmxNotSet);
      break;
#endif

    case mo_trace_cookie:
      cookieTrace = cookieTrace ? 0 : 1;
      set_pref_boolean(eCOOKIETRACE, cookieTrace);
      XmxRSetToggleState(win->menubar, mo_trace_cookie,
			 cookieTrace ? XmxSet : XmxNotSet);
      break;

    case mo_trace_html:
      htmlwTrace = htmlwTrace ? 0 : 1;
      set_pref_boolean(eHTMLWTRACE, htmlwTrace);
      XmxRSetToggleState(win->menubar, mo_trace_html,
			 htmlwTrace ? XmxSet : XmxNotSet);
      break;

    case mo_trace_http:
      httpTrace = httpTrace ? 0 : 1;
      set_pref_boolean(eHTTPTRACE, httpTrace);
      XmxRSetToggleState(win->menubar, mo_trace_http,
			 httpTrace ? XmxSet : XmxNotSet);
      break;

    case mo_trace_nut:
      nutTrace = nutTrace ? 0 : 1;
      set_pref_boolean(eNUTTRACE, nutTrace);
      XmxRSetToggleState(win->menubar, mo_trace_nut,
			 nutTrace ? XmxSet : XmxNotSet);
      break;

    case mo_trace_src:
      srcTrace = srcTrace ? 0 : 1;
      set_pref_boolean(eSRCTRACE, srcTrace);
      XmxRSetToggleState(win->menubar, mo_trace_src,
			 srcTrace ? XmxSet : XmxNotSet);
      break;

    case mo_trace_table:
      tableTrace = tableTrace ? 0 : 1;
      set_pref_boolean(eTABLETRACE, tableTrace);
      XmxRSetToggleState(win->menubar, mo_trace_table,
			 tableTrace ? XmxSet : XmxNotSet);
      break;

    case mo_trace_www2:
      www2Trace = www2Trace ? 0 : 1;
      set_pref_boolean(eWWW2TRACE, www2Trace);
      XmxRSetToggleState(win->menubar, mo_trace_www2,
			 www2Trace ? XmxSet : XmxNotSet);
      break;

    case mo_trace_refresh:
      refreshTrace = refreshTrace ? 0 : 1;
      set_pref_boolean(eREFRESHTRACE, refreshTrace);
      XmxRSetToggleState(win->menubar, mo_trace_refresh,
			 refreshTrace ? XmxSet : XmxNotSet);
      break;

    case mo_report_bugs:
      reportBugs = reportBugs ? 0 : 1;
      set_pref_boolean(eREPORTBUGS, reportBugs);
      XmxRSetToggleState(win->menubar, mo_report_bugs,
			 reportBugs ? XmxSet : XmxNotSet);
      break;
#endif

    default:
      if (i >= DOCUMENTS_MENU_COUNT_OFFSET)
          mo_access_document(win, urllist[i - DOCUMENTS_MENU_COUNT_OFFSET]);
      break;
  }
  return;
}

/* ------------------------------------------------------------------------ */
/* --------------------------- Colleen menubar ---------------------------- */
/* ------------------------------------------------------------------------ */
static XmxMenubarStruct *file_menuspec;
static XmxMenubarStruct *fnts_menuspec;
static XmxMenubarStruct *undr_menuspec;
static XmxMenubarStruct *agent_menuspec;
static XmxMenubarStruct *opts_menuspec;
static XmxMenubarStruct *navi_menuspec;
static XmxMenubarStruct *help_menuspec;
static XmxMenubarStruct *anno_menuspec;
static XmxMenubarStruct *newsfmt_menuspec;
static XmxMenubarStruct *newsgrpfmt_menuspec;
static XmxMenubarStruct *newsartfmt_menuspec;
static XmxMenubarStruct *news_menuspec;
static XmxMenubarStruct *menuspec;

/* ----------------------- simple menubar interface ----------------------- */
static XmxMenubarStruct *file_simple_menuspec;
static XmxMenubarStruct *navi_simple_menuspec;
static XmxMenubarStruct *opts_simple_menuspec;
static XmxMenubarStruct *help_simple_menuspec;
static XmxMenubarStruct *anno_simple_menuspec;
static XmxMenubarStruct *simple_menuspec;

/* --------------------------- format options ----------------------------- */
extern XmxOptionMenuStruct *format_opts;

/* -------------------------- annotation options -------------------------- */
extern XmxOptionMenuStruct *pubpri_opts;


/* ----------------------- macros for menubar stuff ----------------------- */

#define ALLOC_MENUBAR(menuPtr, numEntries) \
{ \
  (menuPtr) = (XmxMenubarStruct *)calloc((numEntries), \
					 sizeof(XmxMenubarStruct)); \
  maxMenuCnt = (numEntries); \
  menuCnt = 0; \
  current = (menuPtr); \
}

#define ALLOC_OPTIONS(optPtr, numOpts) \
{ \
  (optPtr) = (XmxOptionMenuStruct *)calloc((numOpts), \
					   sizeof(XmxOptionMenuStruct)); \
  maxMenuCnt = (numOpts); \
  menuCnt = 0; \
  ocurrent = (optPtr); \
}

#ifndef DISABLE_TRACE

#define DEF_MENUBAR(nameStr, mnemonicStr, cb, cbData, subMenu) \
{ \
  if (menuCnt >= maxMenuCnt) { \
      if (srcTrace) \
	 fprintf(stderr, "Tried to allocate too many option menu items!\n\n"); \
      exit(1); \
  } \
  if ((nameStr) && *(nameStr)) \
      current[menuCnt].namestr = strdup((nameStr)); \
  if (mnemonicStr) \
      current[menuCnt].mnemonic = *(mnemonicStr); \
  current[menuCnt].func = (cb); \
  current[menuCnt].data = (cbData); \
  current[menuCnt++].sub_menu = (subMenu); \
}

#define DEFINE_OPTIONS(nameStr, optData, optState) \
{ \
  if (menuCnt >= maxMenuCnt) { \
      if (srcTrace) \
	  fprintf(stderr,"Tried to allocate too many menu items!\n\n"); \
      exit(1); \
  } \
  if ((nameStr) && *(nameStr)) \
      ocurrent[menuCnt].namestr = (nameStr); \
  ocurrent[menuCnt].data = (optData); \
  ocurrent[menuCnt++].set_state = (optState); \
}

#else /* Take out the srcTrace stuff */

#define DEF_MENUBAR(nameStr, mnemonicStr, cb, cbData, subMenu) \
{ \
  if (menuCnt >= maxMenuCnt) \
      exit(1); \
  if ((nameStr) && *(nameStr)) \
      current[menuCnt].namestr = strdup((nameStr)); \
  if (mnemonicStr) \
      current[menuCnt].mnemonic = *(mnemonicStr); \
  current[menuCnt].func = (cb); \
  current[menuCnt].data = (cbData); \
  current[menuCnt++].sub_menu = (subMenu); \
}

#define DEFINE_OPTIONS(nameStr, optData, optState) \
{ \
  if (menuCnt >= maxMenuCnt) \
      exit(1); \
  if ((nameStr) && *(nameStr)) \
      ocurrent[menuCnt].namestr = (nameStr); \
  ocurrent[menuCnt].data = (optData); \
  ocurrent[menuCnt++].set_state = (optState); \
}
#endif

#define NULL_MENUBAR() \
{ \
  menuCnt++;\
}

#define NULL_OPTIONS() \
{ \
  ocurrent[menuCnt++].set_state = XmxNotSet; \
}

#define SPACER() \
{ \
  current[menuCnt++].namestr = "----"; \
}

#define SPACER2() \
{ \
  current[menuCnt++].namestr = "----2"; \
}

#define LABEL(nameStr) \
{ \
  if ((nameStr) && *(nameStr)) { \
      current[menuCnt++].namestr = (nameStr); \
  } else { \
      current[menuCnt++].namestr = " "; \
  } \
}

/* -------------------------- mo_init_menubar ----------------------------- */
/*
   This function allocates the menubar variables and properly defines them
   according to the international resources set.

   ALLOC_MENUBAR(menuPtr, numEntries) allows you to give it an address and
     it will autocate the specified numbber of pointers for the menubar.
     menuPtr -- XmxMenubarStruct *
     numEntries -- int

   ALLOC_OPTIONS(optPtr, numOpts) allows you to autocate the number of options
     in the option menu.
     optPtr -- XmxOptionMenuStruct *
     numOpts -- int

   DEF_MENUBAR(nameStr, mnemonic, cb, cbData, subMenu) allows you to
     actually fill in the menubar struct.
     nameStr -- char *
     mnemonic -- char *   (only first character is used)
     cb -- void (*func)()
     cbData -- int
     subMenu -- XmxMenubarStruct *

   DEFINE_OPTIONS(nameStr, optData, optState) allows you to
     actually fill in the option menu struct.
     nameStr -- char *
     optData -- int
     optState -- int

   NULL_MENUBAR() defines the current menu entry to be NULL, thus ending
     the current definition.

   NULL_OPTIONS() defines the current menu entry to be NULL, thus ending
     the current definition.

   SPACER() defines a separator for a menu.
   SPACER2() defines a double separator for a menu.

   LABEL(nameStr) defines a label in the menu.
     nameStr -- char *

   Note: To create submenus, you use ALLOC_MENUBAR on the "sub_menu" attribute
     of the XmxMenubarStruct (on an already allocated menubar).  Also, the
     XmxMenubarStruct for the sub_menu must already be allocated...
*/
void mo_init_menubar(void)
{
  int maxMenuCnt, menuCnt, numAgents, i;
  XmxMenubarStruct *current;
  XmxOptionMenuStruct *ocurrent;
  char buf[BUFSIZ];

 /* --------------------------- format options ------------------------------ */
  ALLOC_OPTIONS(format_opts, 5)
  DEFINE_OPTIONS("Plain Text", mo_plaintext, XmxNotSet)
  DEFINE_OPTIONS("Formatted Text", mo_formatted_text, XmxNotSet)
  DEFINE_OPTIONS("PostScript", mo_postscript, XmxNotSet)
  DEFINE_OPTIONS("HTML", mo_html, XmxNotSet)
  NULL_OPTIONS()

 /* -------------------------- annotation options --------------------------- */
  ALLOC_OPTIONS(pubpri_opts, 4)
  DEFINE_OPTIONS("Personal Annotation", mo_annotation_private, XmxSet)
  DEFINE_OPTIONS("Workgroup Annotation", mo_annotation_workgroup, XmxNotSet)
  DEFINE_OPTIONS("Public Annotation", mo_annotation_public, XmxNotSet)
  NULL_OPTIONS()
  
 if (!get_pref_boolean(eSIMPLE_INTERFACE)) {

 /* ----------------------- full menubar interface -------------------------- */
  /* File Menu */
  ALLOC_MENUBAR(file_menuspec, 32)
  DEF_MENUBAR("New", "N", menubar_cb, mo_new_window, NULL)
  DEF_MENUBAR("Clone", "e", menubar_cb, mo_clone_window, NULL)
  SPACER()
  DEF_MENUBAR("Open URL...", "O", menubar_cb, mo_open_document, NULL)
  DEF_MENUBAR("Open Local...", "L", menubar_cb, mo_open_local_document, NULL)
  SPACER()
  DEF_MENUBAR("Reload Current", "R", menubar_cb, mo_reload_document, NULL)
  DEF_MENUBAR("Reload Images", "g",menubar_cb,mo_reload_document_and_images,NULL)
  DEF_MENUBAR("Refresh Current", "u", menubar_cb, mo_refresh_document, NULL)
  SPACER()
  DEF_MENUBAR("Find In Current...", "F", menubar_cb, mo_search, NULL)
  DEF_MENUBAR("View Source", "V", menubar_cb, mo_document_source, NULL)
  DEF_MENUBAR("Edit Source", "d", menubar_cb, mo_document_edit, NULL)
  SPACER()
  DEF_MENUBAR("Save As...", "A", menubar_cb, mo_save_document, NULL)
  DEF_MENUBAR("Print...", "P", menubar_cb, mo_print_document, NULL)
  DEF_MENUBAR("Mail To...", "M", menubar_cb, mo_mail_document, NULL)
#ifdef CCI
  SPACER()
  DEF_MENUBAR("CCI...", "I", menubar_cb, mo_cci, NULL)
#endif
#if defined(KRB4) || defined(KRB5)
  SPACER()
#ifdef KRB4
  DEF_MENUBAR("Kerberos v4 Login...", "4", menubar_cb, mo_kerberosv4_login,NULL)
#endif
#ifdef KRB5
  DEF_MENUBAR("Kerberos v5 Login...", "5", menubar_cb, mo_kerberosv5_login,NULL)
#endif
#endif
  SPACER()
  DEF_MENUBAR("Proxy List...", "y", menubar_cb, mo_proxy, NULL)
  DEF_MENUBAR("No Proxy List...", "t", menubar_cb, mo_no_proxy, NULL)
  SPACER()
  DEF_MENUBAR("Save Preferences", "S", menubar_cb, mo_save_preferences, NULL)
  SPACER()
  DEF_MENUBAR("Close", "C", menubar_cb, mo_close_window, NULL)
  DEF_MENUBAR("Exit Program...", "x", menubar_cb, mo_exit_program, NULL)
  NULL_MENUBAR()

  /* Fonts Sub-Menu */
  ALLOC_MENUBAR(fnts_menuspec, 16);
  DEF_MENUBAR("<Times Regular", "T", menubar_cb, mo_regular_fonts, NULL)
  DEF_MENUBAR("<Times Small", "S", menubar_cb, mo_small_fonts, NULL)
  DEF_MENUBAR("<Times Large", "L", menubar_cb, mo_large_fonts, NULL)
  SPACER()
  DEF_MENUBAR("<Helvetica Regular", "H", menubar_cb, mo_regular_helvetica, NULL)
  DEF_MENUBAR("<Helvetica Small", "e", menubar_cb, mo_small_helvetica, NULL)
  DEF_MENUBAR("<Helvetica Large", "v", menubar_cb, mo_large_helvetica, NULL)
  SPACER()
  DEF_MENUBAR("<New Century Regular", "N",menubar_cb,mo_regular_newcentury,NULL)
  DEF_MENUBAR("<New Century Small", "w", menubar_cb, mo_small_newcentury, NULL)
  DEF_MENUBAR("<New Century Large", "C", menubar_cb, mo_large_newcentury, NULL)
  SPACER()
  DEF_MENUBAR("<Lucida Bright Regular", "L",menubar_cb,mo_regular_lucidabright,NULL)
  DEF_MENUBAR("<Lucida Bright Small", "u",menubar_cb,mo_small_lucidabright,NULL)
  DEF_MENUBAR("<Lucida Bright Large", "i",menubar_cb,mo_large_lucidabright,NULL)
  NULL_MENUBAR()

  /* Underline Sub-Menu */
  ALLOC_MENUBAR(undr_menuspec, 6)
  DEF_MENUBAR("<Default Underlines", "D", menubar_cb,mo_default_underlines,NULL)
  DEF_MENUBAR("<Light Underlines", "L", menubar_cb, mo_l1_underlines, NULL)
  DEF_MENUBAR("<Medium Underlines", "M", menubar_cb, mo_l2_underlines, NULL)
  DEF_MENUBAR("<Heavy Underlines", "H", menubar_cb, mo_l3_underlines, NULL)
  DEF_MENUBAR("<No Underlines", "N", menubar_cb, mo_no_underlines, NULL)
  NULL_MENUBAR()

  /* Agent Spoofing Sub-Menu */
  numAgents = loadAgents();
  ALLOC_MENUBAR(agent_menuspec, numAgents + 1);
  for (i = 0; i < numAgents; i++) {
      if (agent[i][0] == '-') {
	  SPACER()
      } else {
	  sprintf(buf, "<%s", agent[i]);
	  DEF_MENUBAR(buf, " ", agent_menubar_cb, i + mo_last_entry, NULL)
      }
  }
  NULL_MENUBAR()

  /* Options Menu */
  ALLOC_MENUBAR(opts_menuspec, 39)
  /*
  DEF_MENUBAR("#Fancy Selections", "S", menubar_cb, mo_fancy_selections, NULL)
  SPACER()
   */
  DEF_MENUBAR("#Preferences", "P", menubar_cb, mo_preferences, NULL)
  SPACER()
  DEF_MENUBAR("#Binary ftp Mode", "f", menubar_cb, mo_binary_ftp_mode, NULL)
  DEF_MENUBAR("#Load to Local Disk", "t", menubar_cb, mo_binary_transfer, NULL)
  SPACER()
  DEF_MENUBAR("#Table Support", "T", menubar_cb, mo_table_support, NULL)
  DEF_MENUBAR("#Frame Support", "r", menubar_cb, mo_frame_support, NULL)
  SPACER()
  DEF_MENUBAR("#Hotkeys", "H", menubar_cb, mo_hotkeys, NULL)
  DEF_MENUBAR("#Tooltip Help", "p", menubar_cb, mo_tooltips, NULL)
  SPACER()
  DEF_MENUBAR("#Refresh URL Support", "e", menubar_cb, mo_refresh_url, NULL)
  SPACER()
  DEF_MENUBAR("#Browser Safe Colors", "B", menubar_cb, mo_safe_colors, NULL)
  DEF_MENUBAR("#Body Colors", "y", menubar_cb, mo_body_color, NULL)
  DEF_MENUBAR("#Body (Background) Images", "k", menubar_cb, mo_body_images,NULL)
  DEF_MENUBAR("#Font Colors", "o", menubar_cb, mo_font_color, NULL)
  DEF_MENUBAR("#Font Sizes", "s", menubar_cb, mo_font_sizes, NULL)
  DEF_MENUBAR("#Blinking Text", "l", menubar_cb, mo_blink_text, NULL)
  SPACER()
#ifdef HAVE_SSL
  DEF_MENUBAR("#Verify SSL Certificates", "i", menubar_cb, mo_verify_certs,NULL)
  SPACER()
#endif
  DEF_MENUBAR("#View Images Internally", "V",menubar_cb,mo_image_view_internal,NULL)
  DEF_MENUBAR("#Delay Image Loading", "D", menubar_cb,mo_delay_image_loads,NULL)
  DEF_MENUBAR("#Progressive Image Loading", "g",menubar_cb,mo_progressive_loads,NULL)
  DEF_MENUBAR("#Image Animation", "A", menubar_cb, mo_animate_images, NULL)
  DEF_MENUBAR("#Multiple Image Loading", "M", menubar_cb, mo_multi_load, NULL)
  DEF_MENUBAR("Load Images In Current", "L",menubar_cb,mo_expand_images_current,NULL)
  SPACER()
  DEF_MENUBAR("Reload Config Files", "R", menubar_cb, mo_re_init, NULL)
  SPACER()
  DEF_MENUBAR("Flush Image Cache", "I", menubar_cb, mo_clear_image_cache, NULL)
  DEF_MENUBAR("Flush Password Cache", "F",menubar_cb,mo_clear_passwd_cache,NULL)
  DEF_MENUBAR("Clear Global History...", "C",menubar_cb,mo_clear_global_history,NULL)
  SPACER()
  DEF_MENUBAR("Fonts", "n", NULL, 0, fnts_menuspec)
  DEF_MENUBAR("Anchor Underlines", "U", NULL, 0, undr_menuspec)
  DEF_MENUBAR("Agent Spoofs", "S", NULL, 0, agent_menuspec)
  NULL_MENUBAR()

  /* Navigation Menu */
  ALLOC_MENUBAR(navi_menuspec, 23)
  DEF_MENUBAR("Back", "B", menubar_cb, mo_back, NULL)
  DEF_MENUBAR("Forward", "F", menubar_cb, mo_forward, NULL)
  SPACER()
  DEF_MENUBAR("Home Document", "D", menubar_cb, mo_home_document, NULL)
  DEF_MENUBAR("Window History...", "W", menubar_cb, mo_history_list, NULL)
  DEF_MENUBAR("Document Links...", "L", menubar_cb, mo_links_window, NULL)
  SPACER()
  DEF_MENUBAR("Hotlist...", "H", menubar_cb, mo_hotlist_postit, NULL)
  DEF_MENUBAR("Add Current To Hotlist", "C",menubar_cb,mo_register_node_in_default_hotlist,NULL)
  DEF_MENUBAR("Add Hotlist to RBM", "A", menubar_cb,mo_all_hotlist_to_rbm,NULL);
  DEF_MENUBAR("Remove Hotlist from RBM", "R",menubar_cb,mo_all_hotlist_from_rbm,NULL);
  SPACER2()
  LABEL("Search")
  DEF_MENUBAR(NETWORK_SEARCH_NAME, "G", menubar_cb, mo_network_search, NULL)
  DEF_MENUBAR(USENET_SEARCH_NAME, "U", menubar_cb, mo_usenet_search, NULL)
  DEF_MENUBAR(PEOPLE_SEARCH_NAME, "P", menubar_cb, mo_people_search, NULL)
  DEF_MENUBAR(META_SEARCH_NAME, "M", menubar_cb, mo_meta_search, NULL)
  DEF_MENUBAR(INTERNET_METAINDEX_NAME, "Y",menubar_cb,mo_internet_metaindex,NULL)
  DEF_MENUBAR(LIST_SEARCH_NAME, "t", menubar_cb, mo_list_search, NULL)
  DEF_MENUBAR(AUCTION_SEARCH_NAME, "e", menubar_cb, mo_auction_search, NULL)
  DEF_MENUBAR(MAP_SEARCH_NAME, "Q", menubar_cb, mo_map_search, NULL)
  DEF_MENUBAR(ENCYCLOPEDIA_SEARCH_NAME, "k",menubar_cb,mo_encyclopedia_search,NULL)
  NULL_MENUBAR()

  /* Help Menu */
  ALLOC_MENUBAR(help_menuspec, 12)
  DEF_MENUBAR("Using", "U", menubar_cb, mo_help_about, NULL)
  /*
  DEF_MENUBAR("Manual", "M", menubar_cb, mo_mosaic_manual, NULL)
   */
  SPACER()
  /*
  DEF_MENUBAR("Help on Version 2.7b6", "V", menubar_cb, mo_help_onversion, NULL)
   */
  DEF_MENUBAR("Help on VMS Mosaic", "V", menubar_cb, mo_help_vmsmosaic, NULL)
  DEF_MENUBAR("On Window", "W", menubar_cb, mo_help_onwindow, NULL)
  /*
  DEF_MENUBAR("On FAQ", "F",menubar_cb,mo_help_faq, NULL)
   */
  SPACER()
  DEF_MENUBAR("On HTML", "H", menubar_cb, mo_help_html, NULL)
  DEF_MENUBAR("On URLs", "O", menubar_cb, mo_help_url, NULL)
  SPACER()
  DEF_MENUBAR("Mail Tech Support...", "M", menubar_cb, mo_techsupport, NULL)
#ifndef PRERELEASE
  SPACER()
  DEF_MENUBAR("Comment Card...", "C", menubar_cb, mo_cc, NULL)
#endif
  NULL_MENUBAR()

  /* Annotation Menu */
  ALLOC_MENUBAR(anno_menuspec, 6)
  DEF_MENUBAR("Annotate...", "A", menubar_cb, mo_annotate, NULL)
#ifdef HAVE_AUDIO_ANNOTATIONS
  DEF_MENUBAR("Audio Annotate...", "u", menubar_cb, mo_audio_annotate, NULL)
#endif
  SPACER()
  DEF_MENUBAR("Edit This Annotation...", "E", menubar_cb, mo_annotate_edit,NULL)
  DEF_MENUBAR("Delete This Annotation...", "D",menubar_cb,mo_annotate_delete,NULL)
  NULL_MENUBAR()

  /* News Format Sub-Menu */
  ALLOC_MENUBAR(newsfmt_menuspec, 3)
  DEF_MENUBAR("<Thread View", "T", menubar_cb, mo_news_fmt0, NULL)
  DEF_MENUBAR("<Article View", "G", menubar_cb, mo_news_fmt1, NULL)
  NULL_MENUBAR()

  /* News Menu */
  ALLOC_MENUBAR(news_menuspec, 27)
  DEF_MENUBAR("Next", "N", menubar_cb, mo_news_next, NULL)
  DEF_MENUBAR("Prev", "P", menubar_cb, mo_news_prev, NULL)
  DEF_MENUBAR("Next Thread", "t", menubar_cb, mo_news_nextt, NULL)
  DEF_MENUBAR("Prev Thread", "v", menubar_cb, mo_news_prevt, NULL)
  DEF_MENUBAR("Article Index", "I", menubar_cb, mo_news_index, NULL)
  DEF_MENUBAR("Group Index", "G", menubar_cb, mo_news_groups, NULL)
  SPACER()
  DEF_MENUBAR("Post...", "o", menubar_cb, mo_news_post, NULL)
  DEF_MENUBAR("Followup...", "F", menubar_cb, mo_news_follow, NULL)
  SPACER()
  DEF_MENUBAR("Subscribe to Group", "s", menubar_cb, mo_news_sub, NULL)
  DEF_MENUBAR("Unsubscribe Group", "u", menubar_cb, mo_news_unsub, NULL)
  SPACER()
  DEF_MENUBAR("<Show All Groups", "A", menubar_cb, mo_news_grp0, NULL)
  DEF_MENUBAR("<Show Unread Subscribed Groups", "S",menubar_cb,mo_news_grp1,NULL)
  DEF_MENUBAR("<Show All Subscribed Groups", "R", menubar_cb, mo_news_grp2,NULL)
  SPACER()
  DEF_MENUBAR("<Show All Articles", "l", menubar_cb, mo_news_art0, NULL)
  DEF_MENUBAR("<Show Only Unread Articles", "n", menubar_cb, mo_news_art1, NULL)
  SPACER()
  DEF_MENUBAR("Mark Group Read", "e", menubar_cb, mo_news_mread, NULL)
  DEF_MENUBAR("Mark Group Unread", "d", menubar_cb, mo_news_munread, NULL)
  DEF_MENUBAR("Mark Article Unread", "M", menubar_cb, mo_news_maunread, NULL)
  SPACER()
  DEF_MENUBAR("Flush News Data", "F", menubar_cb, mo_news_flush, NULL)
  DEF_MENUBAR("Thread Style", "T", NULL, 0, newsfmt_menuspec)
  NULL_MENUBAR()

  /* The Menubar */
  ALLOC_MENUBAR(menuspec, 9)
  DEF_MENUBAR("File", "F", NULL, 0, file_menuspec)
  DEF_MENUBAR("Options", "O", NULL, 0, opts_menuspec)
  DEF_MENUBAR("Navigate", "N", NULL, 0, navi_menuspec)
  DEF_MENUBAR("Annotate", "A", NULL ,0, anno_menuspec)
  DEF_MENUBAR("News", "w", NULL, 0, news_menuspec)
  DEF_MENUBAR("Help", "H", NULL, 0, help_menuspec)
  /* Dummy submenus for Documents and Debug */
  NULL_MENUBAR()
  NULL_MENUBAR()
  NULL_MENUBAR()

 } else {

 /* ----------------------- simple menubar interface ----------------------- */
  /* File Menu */
  ALLOC_MENUBAR(file_simple_menuspec, 7)
  DEF_MENUBAR("Clone", "e", menubar_cb, mo_clone_window, NULL)
  SPACER()
  DEF_MENUBAR("Find In Current", "F", menubar_cb, mo_search, NULL)
  SPACER()
  DEF_MENUBAR("Close", "C", menubar_cb, mo_close_window, NULL)
  DEF_MENUBAR("Exit Program..." , "x", menubar_cb, mo_exit_program, NULL)
  NULL_MENUBAR()

  /* Options Menu */
  ALLOC_MENUBAR(opts_simple_menuspec, 3)
  DEF_MENUBAR("#Load to Local Disk", "T", menubar_cb, mo_binary_transfer, NULL)
  DEF_MENUBAR("#Binary ftp Mode", "B", menubar_cb, mo_binary_ftp_mode, NULL)
  NULL_MENUBAR()

  /* Navigation Menu */
  ALLOC_MENUBAR(navi_simple_menuspec, 12)
  DEF_MENUBAR("Back", "B", menubar_cb, mo_back, NULL)
  DEF_MENUBAR("Forward", "F", menubar_cb, mo_forward, NULL)
  SPACER()
  DEF_MENUBAR("Home Document", "D", menubar_cb, mo_home_document, NULL)
  SPACER()
  DEF_MENUBAR("Window History...", "W", menubar_cb, mo_history_list, NULL)
  SPACER()
  DEF_MENUBAR("Hotlist...", "H", menubar_cb, mo_hotlist_postit, NULL)
  DEF_MENUBAR("Add Current To Hotlist", "A",menubar_cb,mo_register_node_in_default_hotlist,NULL)
  DEF_MENUBAR("Add All Hotlist Entries to RBM", "E",menubar_cb,mo_all_hotlist_to_rbm,NULL);
  DEF_MENUBAR("Remove All Hotlist Entries from RBM", "R",menubar_cb,mo_all_hotlist_from_rbm,NULL);
  NULL_MENUBAR()

  /* Help Menu */
  ALLOC_MENUBAR(help_simple_menuspec, 7)
  DEF_MENUBAR("Using", "U", menubar_cb, mo_help_about, NULL)
  SPACER()
  DEF_MENUBAR("Help on VMS Mosaic", "V", menubar_cb, mo_help_vmsmosaic, NULL)
  DEF_MENUBAR("On Window", "O", menubar_cb, mo_help_onwindow, NULL)
#ifndef PRERELEASE
  SPACER()
  DEF_MENUBAR("Comment Card...", "C", menubar_cb, mo_cc, NULL)
#endif
  NULL_MENUBAR()

  /* Annotation Menu */
  ALLOC_MENUBAR(anno_simple_menuspec, 6)
  DEF_MENUBAR("Annotate...", "A", menubar_cb, mo_annotate, NULL)
#ifdef HAVE_AUDIO_ANNOTATIONS
  DEF_MENUBAR("Audio Annotate...", "u", menubar_cb, mo_audio_annotate, NULL)
#endif 
  SPACER()
  DEF_MENUBAR("Edit This Annotation...", "E", menubar_cb, mo_annotate_edit,NULL)
  DEF_MENUBAR("Delete This Annotation...", "D",menubar_cb,mo_annotate_delete,NULL)
  NULL_MENUBAR()

  /* The Simple Menubar */
  ALLOC_MENUBAR(simple_menuspec, 8)
  DEF_MENUBAR("File", "F", NULL, 0, file_simple_menuspec)
  DEF_MENUBAR("Options", "O", NULL, 0, opts_simple_menuspec)
  DEF_MENUBAR("Navigate", "N", NULL, 0, navi_simple_menuspec)
  DEF_MENUBAR("Annotate", "A", NULL, 0, anno_simple_menuspec)
  DEF_MENUBAR("Help", "H", NULL, 0, help_simple_menuspec)
  /* Dummy submenu */
  NULL_MENUBAR()
  NULL_MENUBAR()
  NULL_MENUBAR()
 }
}


/* -------------------- mo_make_document_view_menubar --------------------- */

/* We now allow a single customizable menu.  
 *
 * First choice for the spec file is the value of the resource
 * documentsMenuSpecfile.
 * If that doesn't exist, second choice is the value of the
 * environment variable MOSAIC_DOCUMENTS_MENU_SPECFILE.
 * If *that* doesn't exist, third choice is specified in 
 * #define DOCUMENTS_MENU_SPECFILE.
 */
static mo_status mo_file_exists(char *filename)
{
#if (stat != decc$stat) || !defined(MULTINET)
  struct stat buf;
#else
#undef stat
  struct stat buf;
#define stat decc$stat
#endif /* VMS MultiNet work around, GEC */
 
  if (stat(filename, &buf) != -1) {
      return mo_succeed;
  } else {
      return mo_fail;
  }
}

static void mo_grok_menubar(char *filename)
{
  FILE *fp;
  char line[MO_LINE_LENGTH];
  char *status;
  XmxMenubarStruct *menu;
  int count = 0;

  if (!(fp = fopen(filename, "r")))
      return;

  /* Make the menu. */
  menu = (XmxMenubarStruct *)calloc(1, MAX_DOCUMENTS_MENU_ITEMS *
				       sizeof(XmxMenubarStruct));
  /* File consists of alternating titles and URLs.
   * A title consisting of at least two leading dashes
   * is a separator. */
  while (1) {
      status = fgets(line, MO_LINE_LENGTH, fp);
      if (!status || !*line)
	  break;

      /* calloc has zeroed everything */
      if ((line[0] == '-') && (line[1] == '-')) {
          /* It's a separator. */
          menu[count].namestr = "----";
      } else {
          /* That's the title. */
          menu[count].namestr = strdup(line);
          /* Wipe out trailing newline. */
          menu[count].namestr[strlen(line) - 1] = '\0';
          menu[count].func = (void (*)())menubar_cb;
          menu[count].data = count + DOCUMENTS_MENU_COUNT_OFFSET;
          
          status = fgets(line, MO_LINE_LENGTH, fp);
          if (!status || !*line) {
              /* Oops, something went wrong. */
	      free(menu[count].namestr);
	      menu[count].namestr = NULL;
	      break;
          }
          /* There's a URL. */
          urllist[count] = strdup(line);
          urllist[count][strlen(line) - 1] = '\0';
      }
      /* Count increases. */
      if (++count == MAX_DOCUMENTS_MENU_ITEMS)
	  break;
  }
  fclose(fp);

  if (count > 0) {
      if (get_pref_boolean(eSIMPLE_INTERFACE)) {
          simple_menuspec[5].namestr = strdup("Documents");
          simple_menuspec[5].mnemonic = 'D';
          simple_menuspec[5].func = NULL;
          simple_menuspec[5].data = 0;
          simple_menuspec[5].sub_menu = menu;
      } else {
          menuspec[6].namestr = strdup("Documents");
          menuspec[6].mnemonic = 'D';
          menuspec[6].func = NULL;
          menuspec[6].data = 0;
          menuspec[6].sub_menu = menu;
      }
  }
  return;
}

static void mo_try_to_grok_menubar(void)
{
  char *filename = get_pref_string(eDOCUMENTS_MENU_SPECFILE);
  
  if (filename && (mo_status *) mo_file_exists(filename)) {
      mo_grok_menubar(filename);
  } else {
      filename = getenv("MOSAIC_DOCUMENTS_MENU_SPECFILE");
      if (filename && (mo_status *) mo_file_exists(filename)) {
          mo_grok_menubar(filename);
      } else {
          filename = DOCUMENTS_MENU_SPECFILE;
          if (filename && (mo_status *) mo_file_exists(filename))
              mo_grok_menubar(filename);
      }
  }
  return;
}

#ifndef DISABLE_TRACE
static void mo_try_to_setup_debug(void)
{
  int i = 6;
  XmxMenubarStruct *menu;

  /* Make the menu. */
  menu = (XmxMenubarStruct *) calloc(1, 12 * sizeof(XmxMenubarStruct));

  /* Skip over Document menu if there */
  if (menuspec[i].namestr)
     i++;

  menuspec[i].namestr = strdup("Debug");
  menuspec[i].mnemonic = 'b';
  menuspec[i].func = NULL;
  menuspec[i].data = 0;
  menuspec[i].sub_menu = menu;

  for (i = 0; i < 11; i++)
      menu[i].func = (void (*)())menubar_cb;

  /* calloc has zeroed everything */
#ifndef CCI
  menu[0].namestr = "#Report Bugs";
  menu[0].data = mo_report_bugs;

  menu[1].namestr = "#Trace Cache";
  menu[1].data = mo_trace_cache;
#else
  menu[0].namestr = "#Trace Cache";
  menu[0].data = mo_trace_cache;

  menu[1].namestr = "#Trace CCI";
  menu[1].data = mo_trace_cci;
#endif

  menu[2].namestr = "#Trace Cookies";
  menu[2].data = mo_trace_cookie;

  menu[3].namestr = "#Trace HTML";
  menu[3].data = mo_trace_html;

  menu[4].namestr = "#Trace HTTP";
  menu[4].data = mo_trace_http;

  menu[5].namestr = "#Trace NUT";
  menu[5].data = mo_trace_nut;

  menu[6].namestr = "#Trace Refresh";
  menu[6].data = mo_trace_refresh;

  menu[7].namestr = "#Trace SRC";
  menu[7].data = mo_trace_src;

  menu[8].namestr = "#Trace Table";
  menu[8].data = mo_trace_table;

  menu[9].namestr = "#Trace WWW2";
  menu[9].data = mo_trace_www2;

#ifdef CCI
  menu[10].namestr = "#Report Bugs";
  menu[10].data = mo_report_bugs;
#endif

  return;
}

static int setup_debug_menu = 0;
#endif


XmxMenuRecord *mo_make_document_view_menubar(Widget form)
{
  XmxMenuRecord *toBeReturned;
  static int init = 0;
  static Boolean kiosk, simple, tearoff;

  /* If we've never tried to set up the user's configurable menubar by
   * loading menuspec[5], give it a shot now. */
  if (!init) {
      init = 1;
      mo_try_to_grok_menubar();
      kiosk = get_pref_boolean(eKIOSK) || get_pref_boolean(eKIOSKNOEXIT);
      simple = get_pref_boolean(eSIMPLE_INTERFACE);
      tearoff = get_pref_boolean(eMENUBAR_TEAROFF);
  }

#ifndef DISABLE_TRACE
  if (!setup_debug_menu && get_pref_boolean(eDEBUG_MENU)) {
      setup_debug_menu = 1;
      mo_try_to_setup_debug();
  }
#endif

  Xmx_n = 0;
  toBeReturned = XmxRMakeMenubar(form, simple ? simple_menuspec : menuspec,
				 tearoff);
  if (kiosk)
      /* Won't appear */
      XtUnmanageChild(toBeReturned->base);

  return toBeReturned;
}
