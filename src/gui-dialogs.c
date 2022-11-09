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
#include "../libwww2/HTalert.h"
#include "mosaic.h"
#include "gui-dialogs.h"
#include "gui-documents.h"
#include "gui.h"
#include "mo-www.h"
#include "mailto.h"
#include "gui-extras.h"

#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/DrawingA.h>
#include <Xm/ScrollBar.h>
#include <Xm/FilesB.h>
#include <Xm/Protocols.h>

#ifdef VMS
#include <errno.h>
#include <unixio.h>
#include <lib$routines.h>
#endif /* VMS, BSN */

#include "../libhtmlw/HTML.h"
#include "../libnut/system.h"
#include "../libnut/str-tools.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int reportBugs;
#endif

extern int HTML_Print_Headers;
extern int HTML_Print_Footers;
extern int HTML_Print_Paper_Size_A4;
extern int HTML_Print_Duplex;

extern int is_uncompressed;

#ifndef VMS
/* For ~ expansion */
#include <pwd.h>
extern int sys_nerr;
extern char *sys_errlist[];
#endif /* VMS, GEC */

extern int errno;
#define __MAX_HOME_LEN__ 256

extern char *saveFileName;
extern mo_window *current_win;

#ifdef CCI
extern int cci_docommand;
#endif

XmxOptionMenuStruct *format_opts;


#ifdef VMS
static void VMS_ErrorDialog(char *fname, mo_window *win)
{
  char str[1024];

  sprintf(str, "Unable to save document to file %s\nError: %s",
          fname, strerror(errno, vaxc$errno));
  XmxMakeErrorDialog(win->base, str, "Save Error");
}

static void Saved_it(char *text, int len)
{
  char line[128];

  sprintf(line, "Saved %d bytes of data.", len);
  mo_gui_notify_progress(line);
}

/*
 * VMS fputs has a limited buffer size (65534 chars).  Redefine to
 * use write instead, which seems to write unlimited amounts.
 */
#define FPUTS(text, fp, fname, win) \
 { \
  int len = strlen(text); \
 \
  if (write(fileno(fp), text, len) < 0) { \
      VMS_ErrorDialog(fname, win); \
  } else { \
      Saved_it(text, len); \
  } \
 }
#endif /* VMS, BSN, GEC */


static XmxCallback(save_print_header_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

    set_pref_boolean(ePRINT_BANNERS,
		     XmToggleButtonGetState(win->print_header_toggle_save));
    return;
}

static XmxCallback(mail_print_header_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

    set_pref_boolean(ePRINT_BANNERS,
		     XmToggleButtonGetState(win->print_header_toggle_mail));
    return;
}

static XmxCallback(print_print_header_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

    set_pref_boolean(ePRINT_BANNERS,
		     XmToggleButtonGetState(win->print_header_toggle_print));
    return;
}

static XmxCallback(save_print_footer_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

    set_pref_boolean(ePRINT_FOOTNOTES,
		     XmToggleButtonGetState(win->print_footer_toggle_save));
    return;
}

static XmxCallback(mail_print_footer_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

    set_pref_boolean(ePRINT_FOOTNOTES,
		     XmToggleButtonGetState(win->print_footer_toggle_mail));
    return;
}

static XmxCallback(print_print_footer_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

    set_pref_boolean(ePRINT_FOOTNOTES,
		     XmToggleButtonGetState(win->print_footer_toggle_print));
    return;
}

static XmxCallback(save_print_size_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
    int us_size = !XmToggleButtonGetState(win->print_us_toggle_save);

    XmxSetToggleButton(win->print_a4_toggle_save,
		       !XmToggleButtonGetState(win->print_a4_toggle_save));
    XmxSetToggleButton(win->print_us_toggle_save, us_size);

    set_pref_boolean(ePRINT_PAPER_SIZE_US, us_size);

    return;
}

static XmxCallback(print_url_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

    XmxSetToggleButton(win->print_doc_only,
                       !XmToggleButtonGetState(win->print_doc_only));
    XmxSetToggleButton(win->print_url_only,
                       !XmToggleButtonGetState(win->print_url_only));
    return;
}

static XmxCallback(mail_print_size_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
    int us_size = !XmToggleButtonGetState(win->print_us_toggle_mail);

    XmxSetToggleButton(win->print_a4_toggle_mail,
		       !XmToggleButtonGetState(win->print_a4_toggle_mail));
    XmxSetToggleButton(win->print_us_toggle_mail, us_size);

    set_pref_boolean(ePRINT_PAPER_SIZE_US, us_size);

    return;
}

static XmxCallback(print_print_size_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
    int us_size = !XmToggleButtonGetState(win->print_us_toggle_print);

    XmxSetToggleButton(win->print_a4_toggle_print,
		       !XmToggleButtonGetState(win->print_a4_toggle_print));
    XmxSetToggleButton(win->print_us_toggle_print, us_size);

    set_pref_boolean(ePRINT_PAPER_SIZE_US, us_size);

    return;
}

static XmxCallback(save_print_duplex_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

    set_pref_boolean(ePRINT_DUPLEX,
		     XmToggleButtonGetState(win->print_duplex_toggle_save));
    return;
}

static XmxCallback(mail_print_duplex_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

    set_pref_boolean(ePRINT_DUPLEX,
		     XmToggleButtonGetState(win->print_duplex_toggle_mail));
    return;
}

static XmxCallback(print_print_duplex_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

    set_pref_boolean(ePRINT_DUPLEX,
		     XmToggleButtonGetState(win->print_duplex_toggle_print));
    return;
}


/* ------------------------------------------------------------------------ */
/* ----------------------------- SAVE WINDOW ------------------------------ */
/* ------------------------------------------------------------------------ */

mo_status mo_save_window(mo_window *win, char *fname, 
			 mo_format_token save_format)
{
  char *efname = (char *)malloc(sizeof(char) * (__MAX_HOME_LEN__ * 2));
  char *text;
  FILE *fp;
  mo_window *next = NULL;
  mo_window *top = win;

  if (pathEval(efname, fname) < 0) {
#ifndef DISABLE_TRACE
      if (srcTrace)
	  fprintf(stderr, "Error in evaluating the path. (mo_save_window)\n");
#endif
  }

#ifndef VMS
  fp = fopen(efname, "w");
#else
  /* Open for efficient writes, VaxC RMS defaults are pitiful. PGE */
  fp = fopen(efname, "w", "shr = nil", "rop = WBH", "mbf = 4",
             "mbc = 32", "deq = 8", "fop = tef");
#endif /* VMS, GEC */

  if (!fp) {
#ifndef VMS   /* PGE, Unused variables */
      char *buf, *final;
      char tmpbuf[80];
      int final_len;
#endif

#ifdef CCI
      /* Don't display dialog if command issued by cci application */
      if (cci_docommand)
	  return mo_fail;
#endif
#ifndef VMS   /* PGE, Moved around because now supporting CCI */
      buf = my_strerror(errno);
      if (!buf || !*buf || !strcmp(buf, "Error 0")) {
	  sprintf(tmpbuf, "Unknown Error");
	  buf = tmpbuf;
      }
      final_len = 30 + ((!efname || !*efname ? 3 : strlen(efname)) + 13) + 15 +
		 (strlen(buf) + 3);
      final = (char *)malloc(final_len);
      sprintf(final,
	      "\nUnable to save document:\n   %s\n\nSave Error:\n   %s\n",
	      !efname || !*efname ? " " : efname, buf);
      application_error(final, "Save Error");

      if (final)
	  free(final);
#else
      VMS_ErrorDialog(efname, win);
#endif /* VMS, BSN */
  } else {
      int freeit;

#ifdef CCI
      if (!cci_docommand) {
#endif
          HTML_Print_Headers =
			  XmToggleButtonGetState(win->print_header_toggle_save);
          HTML_Print_Footers =
			  XmToggleButtonGetState(win->print_footer_toggle_save);
          HTML_Print_Paper_Size_A4 = 
			  XmToggleButtonGetState(win->print_a4_toggle_save);
          HTML_Print_Duplex = 
			  XmToggleButtonGetState(win->print_duplex_toggle_save);
#ifdef CCI
      } else {  /* CCI app telling mosaic to save a file */
	  if (save_format == mo_postscript) {
	      HTML_Print_Headers = HTML_Print_Footers = HTML_Print_Duplex = 1;
	  } else {
	      HTML_Print_Headers = HTML_Print_Footers = HTML_Print_Duplex = 0;
	  }
	  HTML_Print_Paper_Size_A4 = 0;
      }
#endif
      while (win) {
          /* Do if no frames except inline frames or html requested */
          if (!win->frames || (win->frames->is_frame == 2) ||
	      (save_format == mo_html)) {
	      text = NULL;
	      freeit = 1;
              if (save_format == mo_plaintext) {
	          if (win->is_frame) {
	              text = HTMLGetText(win->scrolled_win, 0,
					 win->frameurl, NULL);
	          } else {
	              text = HTMLGetText(win->scrolled_win, 0,
				         win->current_node->url, NULL);
	          }
	      } else if (save_format == mo_formatted_text) {
	          if (win->is_frame) {
	              text = HTMLGetText(win->scrolled_win, 1,
					 win->frameurl, NULL);
	          } else {
	              text = HTMLGetText(win->scrolled_win, 1,
				         win->current_node->url, NULL);
	          }
              } else if (save_format == mo_postscript) {
	          if (win->is_frame) {
	              text = HTMLGetText(win->scrolled_win,
					 2 + win->font_family, 
				         win->frameurl, " ");
	          } else {
	              text = HTMLGetText(win->scrolled_win,
					 2 + win->font_family, 
				         win->current_node->url,
				         win->current_node->last_modified);
	          }
              } else if (!win->is_frame && win->current_node &&
		         win->current_node->text) {
                  /* HTML source */
                  text = win->current_node->text;
		  freeit = 0;
              } else if (win->is_frame && win->frametext) {
                  text = win->frametext;
		  freeit = 0;
              }
              if (text) {
#ifndef VMS
                  fputs(text, fp);
#else
                  FPUTS(text, fp, efname, top);
#endif
		  if (freeit)
                      free(text);
              }
          }
          if (win->frames) {
              next = win->next_frame;
              win = win->frames;
          } else {
              win = win->next_frame;
          }
          if (!win && next) {
              win = next;
	      next = NULL;
	  }
      }
      fclose(fp);
  }
  free(efname);
  return(mo_succeed);
}

/* ------------------------- mo_post_save_window -------------------------- */

static XmxCallback(save_win_cb)
{
  char *fname;
  char efname[128 + 1];
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

  XtUnmanageChild(win->save_win);
  
  XmStringGetLtoR(((XmFileSelectionBoxCallbackStruct *)call_data)->value,
                   XmSTRING_DEFAULT_CHARSET, &fname);
  pathEval(efname, fname);
  XtFree(fname);

  mo_save_window(win, efname, win->save_format);

  return;
}

static void format_sensitive(mo_window *win, int format)
{
	Arg args[2];
  
	if (format == mo_postscript) {
		/* Postscript */
		int us_size = get_pref_boolean(ePRINT_PAPER_SIZE_US);

                XmxSetToggleButton(win->print_header_toggle_save,
				   get_pref_boolean(ePRINT_BANNERS));
                XmxSetToggleButton(win->print_footer_toggle_save,
				   get_pref_boolean(ePRINT_FOOTNOTES));
                XmxSetToggleButton(win->print_a4_toggle_save, !us_size);
                XmxSetToggleButton(win->print_us_toggle_save, us_size);
                XmxSetToggleButton(win->print_duplex_toggle_save,
				   get_pref_boolean(ePRINT_DUPLEX));

		XtSetArg(args[0], XmNsensitive, TRUE);
	} else {
		/* Plain, formatted or HTML */
		XmxSetToggleButton(win->print_header_toggle_save, XmxNotSet);
		XmxSetToggleButton(win->print_footer_toggle_save, XmxNotSet);
		XmxSetToggleButton(win->print_a4_toggle_save, XmxNotSet);
		XmxSetToggleButton(win->print_us_toggle_save, XmxNotSet);
		XmxSetToggleButton(win->print_duplex_toggle_save, XmxNotSet);

		XtSetArg(args[0], XmNsensitive, FALSE);
	}
	XtSetValues(win->print_header_toggle_save, args, 1);
	XtSetValues(win->print_footer_toggle_save, args, 1);
	XtSetValues(win->print_a4_toggle_save, args, 1);
	XtSetValues(win->print_us_toggle_save, args, 1);
	XtSetValues(win->print_duplex_toggle_save, args, 1);

	return;
}


static XmxCallback(format_optmenu_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
    char *mode;
  
    win->save_format = XmxExtractToken((int)client_data);
    format_sensitive(win, win->save_format);

    switch (win->save_format) {
	case mo_plaintext:
	    mode = strdup(MODE_PLAIN);
	    break;
	case mo_postscript:
	    mode = strdup(MODE_POSTSCRIPT);
	    break;
	case mo_formatted_text:
	    mode = strdup(MODE_FORMATTED);
	    break;
	case mo_html:
	    mode = strdup(MODE_HTML);
    }
    set_pref(eSAVE_MODE, (void *)mode);

    return;
}

mo_status mo_post_save_window(mo_window *win)
{
  XmString sfn, fbfn;
  char fileBuf[2048];
  char *fileBoxFileName;

  XmxSetUniqid(win->id);
  if (!win->save_win) {
      Widget frame, workarea, format_label, paper_size_toggle_box;
      char *mode;
      int i;

      win->save_win = XmxMakeFileSBDialog(win->base,
					  "VMS Mosaic: Save Document",
					  "Name for saved document:",
					  save_win_cb, 0);
      /* This makes a frame as a work area for the dialog box. */
      XmxSetArg(XmNmarginWidth, 5);
      XmxSetArg(XmNmarginHeight, 5);
      frame = XmxMakeFrame(win->save_win, XmxShadowEtchedIn);
      workarea = XmxMakeForm(frame);

      win->print_header_toggle_save = XmxMakeToggleButton(workarea,
			 	    "Include Banners", save_print_header_cb, 0);
      XmxSetToggleButton(win->print_header_toggle_save,
			 get_pref_boolean(ePRINT_BANNERS));
      win->print_footer_toggle_save = XmxMakeToggleButton(workarea,
			 	  "Include Footnotes", save_print_footer_cb, 0);
      XmxSetToggleButton(win->print_footer_toggle_save,
			 get_pref_boolean(ePRINT_FOOTNOTES));
      
      paper_size_toggle_box = XmxMakeRadioBox(workarea);
      win->print_a4_toggle_save = XmxMakeToggleButton(paper_size_toggle_box,
						      "A4 Paper Size",
						      save_print_size_cb, 0);
      win->print_us_toggle_save = XmxMakeToggleButton(paper_size_toggle_box,
						      "US Letter Paper Size",
						      save_print_size_cb, 0);
      XmxSetToggleButton(win->print_a4_toggle_save,
			 !get_pref_boolean(ePRINT_PAPER_SIZE_US));
      XmxSetToggleButton(win->print_us_toggle_save,
			 get_pref_boolean(ePRINT_PAPER_SIZE_US));      

      win->print_duplex_toggle_save = XmxMakeToggleButton(workarea,
				    "Duplex printing", save_print_duplex_cb, 0);
      XmxSetToggleButton(win->print_duplex_toggle_save,
			 get_pref_boolean(ePRINT_DUPLEX));

      format_label = XmxMakeLabel(workarea, "Format for document:");
      /* XmxSetArg(XmNwidth, 210); */

      mode = get_pref_string(eSAVE_MODE);
      if (!mode || !*mode) {
	  char tbuf[BUFSIZ];

	  sprintf(tbuf,
		  "You have set the default %s mode to:\n     [NULL], which is not valid. Defaulting to %s mode.\n\nPlease use one of the following:\n     plain, formatted, postscript, or html.",
		  "save", "plain text save");
	  application_user_info_wait(tbuf);
	  set_pref(eSAVE_MODE, (void *)strdup(MODE_PLAIN));
	  mode = get_pref_string(eSAVE_MODE);
      }

      for (i = 0; i < 4; i++)
	  format_opts[i].set_state = XmxNotSet;

      if (!my_strcasecmp(mode, MODE_HTML)) {
          format_opts[3].set_state = XmxSet;
	  win->save_format = mo_html;
      } else if (!my_strcasecmp(mode, MODE_POSTSCRIPT)) {
          format_opts[2].set_state = XmxSet;
	  win->save_format = mo_postscript;
      } else if (!my_strcasecmp(mode, MODE_FORMATTED)) {
          format_opts[1].set_state = XmxSet;
	  win->save_format = mo_formatted_text;
      } else if (!my_strcasecmp(mode, MODE_PLAIN)) {
          format_opts[0].set_state = XmxSet;
	  win->save_format = mo_plaintext;
      } else {
	  char tbuf[BUFSIZ];

	  sprintf(tbuf,
		  "You have set the default %s mode to:\n     [%d], which is not valid. Defaulting to %s mode.\n\nPlease use one of the following:\n     plain, formatted, postscript, or html.",
		  "save", mode, "plain text save");
	  application_user_info_wait(tbuf);
          format_opts[0].set_state = XmxSet;
	  win->save_format = mo_plaintext;
      }

      win->format_optmenu = XmxRMakeOptionMenu(workarea, "", format_optmenu_cb, 
                                               format_opts);
      XmxSetArg(XmNtopOffset, 7);
      XmxSetConstraints
        (format_label, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_FORM,
         XmATTACH_NONE, NULL, NULL, NULL, NULL);
      XmxSetConstraints
        (win->format_optmenu->base, XmATTACH_FORM, XmATTACH_NONE, 
         XmATTACH_WIDGET,
         XmATTACH_FORM, NULL, NULL, format_label, NULL);
      XmxSetArg(XmNtopOffset, 15);
      XmxSetConstraints
         (win->print_header_toggle_save, XmATTACH_WIDGET, XmATTACH_NONE,
         XmATTACH_FORM, XmATTACH_NONE,
         format_label, NULL, NULL, NULL);
      XmxSetConstraints
         (win->print_footer_toggle_save, XmATTACH_WIDGET, XmATTACH_NONE,
         XmATTACH_FORM, XmATTACH_NONE,
         win->print_header_toggle_save, NULL, NULL, NULL);
      XmxSetConstraints
	(paper_size_toggle_box, XmATTACH_WIDGET, XmATTACH_NONE,
	 XmATTACH_FORM, XmATTACH_NONE,
	 win->print_footer_toggle_save, NULL, NULL, NULL);
      XmxSetConstraints
	(win->print_duplex_toggle_save, XmATTACH_WIDGET, XmATTACH_FORM,
	 XmATTACH_FORM, XmATTACH_NONE,
	 paper_size_toggle_box, NULL, NULL, NULL);

      format_sensitive(win, win->save_format);
  } else {
      XmFileSelectionDoSearch(win->save_win, NULL);
  }
  
  XtVaGetValues(win->save_win,
                XmNdirSpec, &fbfn,
                NULL);
  if (!XmStringGetLtoR(fbfn, XmSTRING_DEFAULT_CHARSET, &fileBoxFileName)) {
#ifndef DISABLE_TRACE
      if (srcTrace || reportBugs)
	  fprintf(stderr,
		  "Internal Error In Save As... PLEASE REPORT THIS!\n");
#endif
      return mo_fail;
  }
  XmStringFree(fbfn);
  if (*fileBoxFileName && win && win->current_node && win->current_node->url &&
      *win->current_node->url) {

      /* No need to check on NULL from getFileName as we know url exists */
      sprintf(fileBuf, "%s%s", fileBoxFileName,
	      getFileName(win->current_node->url));
      sfn = XmStringCreateLtoR(fileBuf, XmSTRING_DEFAULT_CHARSET);
      XtVaSetValues(win->save_win, XmNdirSpec, sfn, NULL);
      XmStringFree(sfn);
  }
  XtFree(fileBoxFileName);

  XmxManageRemanage(win->save_win);

  return mo_succeed;
}

/* ------------------------------------------------------------------------ */
/* -------------------------- SAVEBINARY WINDOW --------------------------- */
/* ------------------------------------------------------------------------ */

/* This is used by libwww/HTFormat.c to present a user interface
 * for retrieving files in binary transfer mode.  Obviously a redesign
 * of the interface between the GUI and the commom library really needs
 * to happen -- hopefully libwww2 will make this easy.  HA!!! */

/* Thanks to Martha Weinberg (lyonsm@hpwcsvp.mayfield.hp.com) for
 * idea and code starting point. */
static char *temp_binary_fnam;

static XmxCallback(savebinary_cancel_cb)
{
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

#ifndef VMS
  if (unlink(temp_binary_fnam) < 0) {
      char *buf= my_strerror(errno);
      char *final;
      char tmpbuf[80];
      int final_len;

      if (!buf || !*buf || !strcmp(buf, "Error 0")) {
	  sprintf(tmpbuf, "Unknown Error");
	  buf = tmpbuf;
      }
      final_len = 30 +
		((!temp_binary_fnam || !*temp_binary_fnam ? 3 :
		 strlen(temp_binary_fnam)) + 13) + 15 + (strlen(buf) + 13);
      final = (char *)malloc(final_len);

      sprintf(final,
	     "\nUnable to Remove Local File:\n   %s\n\nRemove Error:\n   %s\n",
	     (!temp_binary_fnam || !*temp_binary_fnam ? " " : temp_binary_fnam),
	     buf);
      XmxMakeErrorDialog(win->base, final, "Remove Error");
      if (final)
	  free(final);
  }
#else
  if (remove(temp_binary_fnam))
      XmxMakeErrorDialog(win->base,
                         "Unable to remove local copy of binary file.", 
                         "Remove Error");
#endif /* VMS, BSN */

  /* This was dup'd down below... */
  free(temp_binary_fnam);
  
  return;
}

#if defined(VMS)
#include <fab.h>
#include <rmsdef.h>		/* RMS status codes */
#include <iodef.h>		/* I/O function codes */
#include <fibdef.h>		/* File information block defs */
#include <atrdef.h>		/* Attribute request codes */
#define FCH$V_CONTIGB	0x005			/* Pos of contig best try bit */
#define FCH$M_CONTIGB	(1 << FCH$V_CONTIGB)	/* Contig best try bit mask */
/* VMS I/O User's Reference Manual: Part I (V5.x doc set) */
struct fatdef {
    unsigned char	fat$b_rtype,	fat$b_rattrib;
    unsigned short	fat$w_rsize;
    unsigned long	fat$l_hiblk,	fat$l_efblk;
    unsigned short	fat$w_ffbyte;
    unsigned char	fat$b_bktsize,	fat$b_vfcsize;
    unsigned short	fat$w_maxrec,	fat$w_defext,	fat$w_gbc;
    unsigned	: 16, : 32, : 16;   /* 6 bytes reserved, 2 bytes not used */
    unsigned short	fat$w_versions;
};

/* Arbitrary descriptor without type and class info */
typedef struct dsc {
    unsigned short len;
    unsigned short mbz;
    void *adr;
} Desc;

extern unsigned long sys$open(), sys$qiow(), sys$dassgn();

#define syswork(sts) ((sts) & 1)
#define sysfail(sts) (!syswork(sts))

/*
 *  Originally by:
 *  25-Jul-1995 - Pat Rankin (rankin@eql.caltech.edu)
 *
 *  Force a file to be marked as having fixed-length, 512 byte records
 *  without implied carriage control, and with best_try_contiguous set.
 */
static unsigned long VMS_FixedLengthRecords(char *filename)
{
    struct FAB	    fab;		/* RMS file access block */
    struct fibdef   fib;		/* XQP file information block */
    struct fatdef   recattr;		/* XQP file "record" attributes */
    struct atrdef   attr_rqst_list[3];	/* XQP attribute request itemlist */
    Desc	    fib_dsc;
    unsigned short  channel;
    unsigned short  iosb[4];
    unsigned long   fchars, sts, tmp;

    /* Initialize file access block */
    fab = cc$rms_fab;
    fab.fab$l_fna = filename;
    fab.fab$b_fns = (unsigned char) strlen(filename);
    fab.fab$l_fop = FAB$M_UFO;	/* User file open; no further RMS processing */
    fab.fab$b_fac = FAB$M_PUT;	/* Need write access */
    fab.fab$b_shr = FAB$M_NIL;	/* Exclusive access */

    sts = sys$open(&fab);	/* Channel in stv; $dassgn to close */
    if (sts == RMS$_FLK) {
	/* For MultiNet, at least, if the file was just written by a remote
	 * NFS client, the local NFS server might still have it open, and the
	 * failed access attempt will provoke it to be closed, so try again. */
	sts = sys$open(&fab);
    }
    if (sysfail(sts))
	return sts;

    /* RMS supplies a user-mode channel (see FAB$L_FOP FAB$V_UFO doc) */
    channel = (unsigned short) fab.fab$l_stv;

    /* Set up ACP interface structures */
    /* File information block, passed by descriptor; it's okay to start with
     * an empty FIB after RMS has accessed the file for us */
    fib_dsc.len = sizeof fib;
    fib_dsc.mbz = 0;
    fib_dsc.adr = &fib;
    memset((void *)&fib, 0, sizeof fib);

    /* Attribute request list */
    attr_rqst_list[0].atr$w_size = sizeof recattr;
    attr_rqst_list[0].atr$w_type = ATR$C_RECATTR;
    *(void **)&attr_rqst_list[0].atr$l_addr = (void *)&recattr;
    attr_rqst_list[1].atr$w_size = sizeof fchars;
    attr_rqst_list[1].atr$w_type = ATR$C_UCHAR;
    *(void **)&attr_rqst_list[1].atr$l_addr = (void *)&fchars;
    attr_rqst_list[2].atr$w_size = attr_rqst_list[2].atr$w_type = 0;
    attr_rqst_list[2].atr$l_addr = 0;

    /* File "record" attributes */
    memset((void *)&recattr, 0, sizeof recattr);
    fchars = 0; 	/* File characteristics */

    /* Get current attributes */
    sts = sys$qiow(0, channel, IO$_ACCESS, iosb, (void(*)())0, 0,
		   &fib_dsc, 0, 0, 0, attr_rqst_list, 0);
    if (syswork(sts))
	sts = iosb[0];

    /* Set desired attributes */
    if (syswork(sts)) {
	recattr.fat$b_rtype = FAB$C_SEQ | FAB$C_FIX;   /* org=seq, rfm=fix */
	recattr.fat$w_rsize = recattr.fat$w_maxrec = 512;   /* lrl=mrs=512 */
	recattr.fat$b_rattrib = 0;		       /* rat=none */
	fchars |= FCH$M_CONTIGB;		       /* contiguous-best-try */
	sts = sys$qiow(0, channel, IO$_DEACCESS, iosb, (void(*)())0, 0,
		       &fib_dsc, 0, 0, 0, attr_rqst_list, 0);
	if (syswork(sts))
	    sts = iosb[0];
    }

    /* All done */
    tmp = sys$dassgn(channel);
    if (syswork(sts))
	sts = tmp;
    return sts;
}
#endif /* VMS */

static XmxCallback(savebinary_win_cb)
{
  char *fname;
  char efname[128 + 1];
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
  char retBuf[BUFSIZ];
  int status;

#ifdef VMS
  if (!temp_binary_fnam)
      return;
#endif/* VMS, GEC for ML */

  XtUnmanageChild(win->savebinary_win);

  XmStringGetLtoR(((XmFileSelectionBoxCallbackStruct *)call_data)->value,
                   XmSTRING_DEFAULT_CHARSET, &fname);
  pathEval(efname, fname);
  XtFree(fname);

  /* New "mv" function to take care of these /bin/mv things */
  if ((status = my_move(temp_binary_fnam, efname, retBuf, BUFSIZ, 1)) !=
      SYS_SUCCESS)
      application_user_info_wait(retBuf);

#ifdef VMS
  /* Make it fixed 512 */
  VMS_FixedLengthRecords(efname);
#endif

  free(temp_binary_fnam);
  temp_binary_fnam = NULL;

  return;
}


static mo_status mo_post_savebinary_window(mo_window *win)
{
	XmString sfn, fbfn;
	char fileBuf[2048];
	char *fileBoxFileName;

	XmxSetUniqid(win->id);
	if (!win->savebinary_win) {
		XmxSetArg(XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
		win->savebinary_win = XmxMakeFileSBDialog(win->base,
				   "VMS Mosaic: Save Binary File To Local Disk",
				   "Name for binary file on local disk:",
				   savebinary_win_cb, 0);
		XmxAddCallback(win->savebinary_win,
			       XmNcancelCallback, savebinary_cancel_cb, 0);
	} else {
		XmFileSelectionDoSearch(win->savebinary_win, NULL);
	}

	/* Save File now goes to a specific filename */
	XtVaGetValues(win->savebinary_win,
		      XmNdirSpec, &fbfn,
		      NULL);
	if (!XmStringGetLtoR(fbfn, XmSTRING_DEFAULT_CHARSET,&fileBoxFileName)) {
#ifndef DISABLE_TRACE
		if (srcTrace || reportBugs)
			fprintf(stderr, "Internal Error In Save Binary\n");
#endif
		return mo_fail;
	}
	XmStringFree(fbfn);
	if (*fileBoxFileName && saveFileName && *saveFileName) {
		/* No need to check for NULL as we know url exists */
		char *sptr = getFileName(saveFileName);
		char *ptr;

		/* There is a "." in it */
		if (is_uncompressed && (ptr = strrchr(sptr, '.'))) {
			char *ptr2;

			if (!strncmp(ptr, ".Z", 2) || !strncmp(ptr, ".gz", 3)) {
				/* Get rid of it! */
				*ptr = '\0';
			} else if ((ptr2 = strrchr(ptr, '-')) &&
				   !my_strcasecmp(ptr2, "-gz")) {
				*ptr2 = '\0';
			} else if (!strncmp(ptr, ".tgz", 4)) {
				*(ptr + 2) = 'a';
				*(ptr + 3) = 'r';
			}
		}
		sprintf(fileBuf, "%s%s", fileBoxFileName, sptr);
		sfn = XmStringCreateLtoR(fileBuf, XmSTRING_DEFAULT_CHARSET);
		XtVaSetValues(win->savebinary_win,
			      XmNdirSpec, sfn,
			      NULL);
		XmStringFree(sfn);
	}
	XtFree(fileBoxFileName);
	XmxManageRemanage(win->savebinary_win);

	return mo_succeed;
}

void rename_binary_file(char *fnam)
{
  temp_binary_fnam = strdup(fnam);
  mo_post_savebinary_window(current_win);
}

/* ---------------------- mo_post_open_local_window ----------------------- */

static XmxCallback(open_local_win_cb)
{
  char *fname, *url;
  char efname[128 + 1];
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

  XtUnmanageChild(win->open_local_win);
  
  XmStringGetLtoR(((XmFileSelectionBoxCallbackStruct *)call_data)->value,
                  XmSTRING_DEFAULT_CHARSET, &fname);
  pathEval(efname, fname);
  XtFree(fname);

  url = mo_url_canonicalize_local(efname);
  if (url[strlen(url) - 1] == '/')
      url[strlen(url) - 1] = '\0';
  mo_load_window_text(win, url, NULL);

  free(url);

  return;
}

mo_status mo_post_open_local_window(mo_window *win)
{
  XmxSetUniqid(win->id);
  if (!win->open_local_win) {
      win->open_local_win = XmxMakeFileSBDialog(win->base,
					      "VMS Mosaic: Open Local Document",
 	   			       	      "Name of local document to open:",
  					      open_local_win_cb, 0);
  } else {
      XmFileSelectionDoSearch(win->open_local_win, NULL);
  }
  XmxManageRemanage(win->open_local_win);
  return mo_succeed;
}

/* ----------------------- mo_post_open_window ------------------------ */

static XmxCallback(open_win_cb)
{
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
  char *url, *xurl;

  switch (XmxExtractToken((int)client_data)) {
    case 0:
    case 4:
      XtUnmanageChild(win->open_win);
      url = mo_clean_and_escape_url(XmxTextGetString(win->open_text), 2);
      if (!url || !*url)
	  /* Nothing here so do nothing */
	  return;

      /* If URL is enclosed inside <brackets> then extract it */
      if (strstr(url, "<"))
	  url = strtok(url, "<>");

      xurl = mo_url_prepend_protocol(url);

      if (XmxExtractToken((int)client_data) == 0) {
	  mo_load_window_text(win, xurl, NULL);
      } else {
	  mo_open_another_window(win, xurl, NULL, NULL);
      }
      free(url);
      free(xurl);
      break;
    case 1:
      /* Dismiss it */
      XtUnmanageChild(win->open_win);
      break;
    case 2:
      /* Open help */
      mo_open_another_window(win,
			     mo_assemble_help_url("docview-menubar-file.html"),
			     NULL, NULL);
      break;
    case 3:
      XmxTextSetString(win->open_text, "");
      break;
  }
  return;
}

mo_status mo_post_open_window(mo_window *win)
{
  if (!win->open_win) {
      Widget dialog_frame, dialog_sep, buttons_form;
      Widget open_form, label;
      
      /* Create it for the first time. */
      XmxSetUniqid(win->id);
      win->open_win = XmxMakeFormDialog(win->base, "VMS Mosaic: Open Document");
      dialog_frame = XmxMakeFrame(win->open_win, XmxShadowOut);

      /* Constraints for base. */
      XmxSetConstraints 
        (dialog_frame, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM, XmATTACH_FORM, NULL, NULL, NULL, NULL);
      
      /* Main form. */
      open_form = XmxMakeForm(dialog_frame);
      
      label = XmxMakeLabel(open_form, "URL To Open: ");
      XmxSetArg(XmNwidth, 320);
      win->open_text = XmxMakeTextField(open_form);
      XmxAddCallbackToText(win->open_text, open_win_cb, 0);
      
      dialog_sep = XmxMakeHorizontalSeparator(open_form);
      
      buttons_form = XmxMakeFormAndFiveButtons(open_form, open_win_cb,
			      "Open", "Open New", "Clear", "Dismiss", "Help...",
			      0, 4, 3, 1, 2);
      XmxSetButtonClue("Open in current window", "Open in new window",
  		       "Clear URL", "Close this menu",
		       "Open help in new Mosaic window");

      /* Constraints for open_form. */
      XmxSetOffsets(label, 14, 0, 10, 0);
      XmxSetConstraints
        (label, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_NONE,
         NULL, NULL, NULL, NULL);
      XmxSetOffsets(win->open_text, 10, 0, 5, 10);
      XmxSetConstraints
        (win->open_text, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_WIDGET,
         XmATTACH_FORM, NULL, NULL, label, NULL);
      XmxSetArg(XmNtopOffset, 10);
      XmxSetConstraints 
        (dialog_sep, XmATTACH_WIDGET, XmATTACH_WIDGET, XmATTACH_FORM, 
         XmATTACH_FORM,
         win->open_text, buttons_form, NULL, NULL);
      XmxSetConstraints 
        (buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM,
	 XmATTACH_FORM,
         NULL, NULL, NULL, NULL);
  }
  XmxManageRemanage(win->open_win);
  
  return mo_succeed;
}


/* ------------------------- mo_post_mail_window -------------------------- */

static XmxCallback(mail_win_cb)
{
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
  char *to, *subj, *content_type;
  char *text = NULL;
  int free_text;
  int sXtFree = 1;
  int tXtFree = 1;

  switch (XmxExtractToken((int)client_data)) {
    case 0:
      XtUnmanageChild(win->mail_win);

      HTML_Print_Headers = XmToggleButtonGetState(
				win->print_header_toggle_mail);
      HTML_Print_Footers = XmToggleButtonGetState(
				win->print_footer_toggle_mail);
      HTML_Print_Paper_Size_A4 = XmToggleButtonGetState(
				win->print_a4_toggle_mail);
      HTML_Print_Duplex = XmToggleButtonGetState(
				win->print_duplex_toggle_mail);

      /* Ignore missing info for now.  Will cause error display later. */
      to = XmxTextGetString(win->mail_to_text);
      if (!to) {
	  to = strdup("");
	  tXtFree = 0;
      }
      subj = XmxTextGetString(win->mail_subj_text);
      if (!subj) {
	  subj = strdup("");
	  sXtFree = 0;
      }
      if (win->mail_format == mo_plaintext) {
          text = HTMLGetText(win->scrolled_win, 0,
			     win->current_node->url, NULL);
          content_type = "text/plain";
          free_text = 1;

      } else if (win->mail_format == mo_formatted_text) {
          text = HTMLGetText(win->scrolled_win, 1,
			     win->current_node->url, NULL);
          content_type = "text/plain";
          free_text = 1;

      } else if (win->mail_format == mo_postscript) {
          text = HTMLGetText(win->scrolled_win, 2 + win->font_family,
			     win->current_node->url,
			     win->current_node->last_modified);
          content_type = "application/postscript";
          free_text = 1;

      } else if (win->current_node && win->current_node->text) {
          /* HTML source. */
          text = win->current_node->text;
          content_type = "text/html";
          free_text = 0;
      }

      /* Ignore blank text for now.  Will cause error display later. */
      mo_send_mailto_message(text, to, subj,
                             XmToggleButtonGetState(win->print_url_only) ?
                             "url_only" : content_type,
			     win->current_node ? win->current_node->url : NULL);
      if (free_text && text)
          free(text);
      if (tXtFree) {
          XtFree(to);
      } else {
          free(to);
      }
      if (sXtFree) {
          XtFree(subj);
      } else {
          free(subj);
      }
      break;
    case 1:
      /* Dismiss and return */
      XtUnmanageChild(win->mail_win);
      break;
    case 2:
      mo_open_another_window(win, 
			 mo_assemble_help_url("docview-menubar-file.html#mail"),
			 NULL, NULL);
      break;
  }
  return;
}


static void mail_sensitive(mo_window *win, int format)
{
	Arg args[2];

	if (format == mo_postscript) {
		/* Postscript */
		int us_size = get_pref_boolean(ePRINT_PAPER_SIZE_US);

                XmxSetToggleButton(win->print_header_toggle_mail,
				   get_pref_boolean(ePRINT_BANNERS));
                XmxSetToggleButton(win->print_footer_toggle_mail,
				   get_pref_boolean(ePRINT_FOOTNOTES));
                XmxSetToggleButton(win->print_a4_toggle_mail, !us_size);
                XmxSetToggleButton(win->print_us_toggle_mail, us_size);
                XmxSetToggleButton(win->print_duplex_toggle_mail,
				   get_pref_boolean(ePRINT_DUPLEX));

		XtSetArg(args[0], XmNsensitive, TRUE);
	} else {
		XmxSetToggleButton(win->print_header_toggle_mail, XmxNotSet);
		XmxSetToggleButton(win->print_footer_toggle_mail, XmxNotSet);
                XmxSetToggleButton(win->print_a4_toggle_mail, XmxNotSet);
                XmxSetToggleButton(win->print_us_toggle_mail, XmxNotSet);
                XmxSetToggleButton(win->print_duplex_toggle_mail, XmxNotSet);

		XtSetArg(args[0], XmNsensitive, FALSE);
	}
	XtSetValues(win->print_header_toggle_mail, args, 1);
	XtSetValues(win->print_footer_toggle_mail, args, 1);
        XtSetValues(win->print_a4_toggle_mail, args, 1);
        XtSetValues(win->print_us_toggle_mail, args, 1);
        XtSetValues(win->print_duplex_toggle_mail, args, 1);

	return;
}

static XmxCallback(mail_fmtmenu_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
    char *mode;
  
    win->mail_format = XmxExtractToken((int)client_data);
    mail_sensitive(win, win->mail_format);

    switch (win->mail_format) {
	case mo_plaintext:
	    mode = strdup(MODE_PLAIN);
	    break;
	case mo_postscript:
	    mode = strdup(MODE_POSTSCRIPT);
	    break;
	case mo_formatted_text:
	    mode = strdup(MODE_FORMATTED);
	    break;
	case mo_html:
	    mode = strdup(MODE_HTML);
    }
    set_pref(eMAIL_MODE, (void *)mode);

    return;
}

mo_status mo_post_mail_window(mo_window *win)
{
  if (!win->mail_win) {
      Widget dialog_frame, dialog_sep, buttons_form;
      Widget mail_form, to_label, subj_label;
      Widget frame, frame2, workarea, format_label;
      Widget paper_size_toggle_box, url_toggle_box;
      int i;
          
      /* Create it for the first time. */
      XmxSetUniqid(win->id);
      win->mail_win = XmxMakeFormDialog(win->base, "VMS Mosaic: Mail Document");
      dialog_frame = XmxMakeFrame(win->mail_win, XmxShadowOut);

      /* Constraints for base. */
      XmxSetConstraints 
        (dialog_frame, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM, XmATTACH_FORM, NULL, NULL, NULL, NULL);
      
      /* Main form. */
      mail_form = XmxMakeForm(dialog_frame);
      
      to_label = XmxMakeLabel(mail_form, "Mail To: ");
      XmxSetArg(XmNwidth, 335);
      win->mail_to_text = XmxMakeTextField(mail_form);
      
      subj_label = XmxMakeLabel(mail_form, "Subject: ");
      win->mail_subj_text = XmxMakeTextField(mail_form);

      {
	int us_size = get_pref_boolean(ePRINT_PAPER_SIZE_US);
	char *mode;

        XmxSetArg(XmNmarginWidth, 5);
        XmxSetArg(XmNmarginHeight, 5);
        frame = XmxMakeFrame(mail_form, XmxShadowEtchedIn);
        workarea = XmxMakeForm(frame);
        
        win->print_header_toggle_mail = XmxMakeToggleButton(workarea,
        			    "Include Banners", mail_print_header_cb, 0);
        XmxSetToggleButton(win->print_header_toggle_mail,
			   get_pref_boolean(ePRINT_BANNERS));

        win->print_footer_toggle_mail = XmxMakeToggleButton(workarea,
        			  "Include Footnotes", mail_print_footer_cb, 0);
        XmxSetToggleButton(win->print_footer_toggle_mail,
			   get_pref_boolean(ePRINT_FOOTNOTES));

	paper_size_toggle_box = XmxMakeRadioBox(workarea);
	win->print_a4_toggle_mail = XmxMakeToggleButton(paper_size_toggle_box,
							"A4 Paper Size",
							mail_print_size_cb, 0);
	win->print_us_toggle_mail = XmxMakeToggleButton(paper_size_toggle_box,
							"US Letter Paper Size",
							mail_print_size_cb, 0);
	XmxSetToggleButton(win->print_a4_toggle_mail, !us_size);
	XmxSetToggleButton(win->print_us_toggle_mail, us_size);

        win->print_duplex_toggle_mail = XmxMakeToggleButton(workarea,
				    "Duplex Printing", mail_print_duplex_cb, 0);
        XmxSetToggleButton(win->print_duplex_toggle_mail,
			   get_pref_boolean(ePRINT_DUPLEX));

        format_label = XmxMakeLabel(workarea, "Format for document:");

        /* Set the default mode */
	mode = get_pref_string(eMAIL_MODE);
	if (!mode || !*mode) {
	    char tbuf[BUFSIZ];

	    sprintf(tbuf,
		    "You have set the default %s mode to:\n     [NULL], which is not valid. Defaulting to %s mode.\n\nPlease use one of the following:\n     plain, formatted, postscript, or html.",
		    "mail", "plain text mail");
	    application_user_info_wait(tbuf);
	    set_pref(eMAIL_MODE, (void *)strdup(MODE_PLAIN));
	    mode = get_pref_string(eMAIL_MODE);
	}
        for (i = 0; i < 4; i++)
            format_opts[i].set_state = XmxNotSet;

	if (!my_strcasecmp(mode, MODE_HTML)) {
            format_opts[3].set_state = XmxSet;
	    win->mail_format = mo_html;
	} else if (!my_strcasecmp(mode, MODE_POSTSCRIPT)) {
            format_opts[2].set_state = XmxSet;
	    win->mail_format = mo_postscript;
	} else if (!my_strcasecmp(mode, MODE_FORMATTED)) {
            format_opts[1].set_state = XmxSet;
	    win->mail_format = mo_formatted_text;
	} else if (!my_strcasecmp(mode, MODE_PLAIN)) {
            format_opts[0].set_state = XmxSet;
	    win->mail_format = mo_plaintext;
	} else {
	    char tbuf[BUFSIZ];

	    sprintf(tbuf,
		    "You have set the default %s mode to:\n     [%s], which is not valid. Defaulting to %s mode.\n\nPlease use one of the following:\n     plain, formatted, postscript, or html.",
		    "mail", mode, "plain text mail");
	    application_user_info_wait(tbuf);
            format_opts[0].set_state = XmxSet;
	    win->mail_format = mo_plaintext;
	}

	win->mail_fmtmenu = XmxRMakeOptionMenu(workarea, "", mail_fmtmenu_cb,
                                               format_opts);
        XmxSetArg(XmNtopOffset, 7);
        XmxSetConstraints
          (format_label, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_FORM,
           XmATTACH_NONE, NULL, NULL, NULL, NULL);
        XmxSetConstraints
          (win->mail_fmtmenu->base, XmATTACH_FORM, XmATTACH_NONE, 
           XmATTACH_WIDGET,
           XmATTACH_FORM, NULL, NULL, format_label, NULL);
        XmxSetArg(XmNtopOffset, 15);
        XmxSetConstraints
           (win->print_header_toggle_mail, XmATTACH_WIDGET, XmATTACH_NONE,
            XmATTACH_FORM, XmATTACH_NONE,
            format_label, NULL, NULL, NULL);
        XmxSetConstraints
           (win->print_footer_toggle_mail, XmATTACH_WIDGET, XmATTACH_NONE,
            XmATTACH_FORM, XmATTACH_NONE,
            win->print_header_toggle_mail, NULL, NULL, NULL);
	XmxSetConstraints
	   (paper_size_toggle_box, XmATTACH_WIDGET, XmATTACH_NONE,
	    XmATTACH_FORM, XmATTACH_NONE,
	    win->print_footer_toggle_mail, NULL, NULL, NULL);
	XmxSetConstraints
	   (win->print_duplex_toggle_mail, XmATTACH_WIDGET, XmATTACH_FORM,
	    XmATTACH_FORM, XmATTACH_NONE,
	    paper_size_toggle_box, NULL, NULL, NULL);

        mail_sensitive(win, win->mail_format);
      }

      frame2 = XmxMakeFrame(mail_form, XmxShadowEtchedIn);
      url_toggle_box = XmxMakeRadioBox(frame2);
      win->print_doc_only = XmxMakeToggleButton(url_toggle_box,
                                                "Mail Entire Document",
                                                print_url_cb, 0);
      win->print_url_only = XmxMakeToggleButton(url_toggle_box,
                                                "Mail URL Only",
                                                print_url_cb, 0);
      XmxSetToggleButton(win->print_doc_only, 1);
      XmxSetToggleButton(win->print_url_only, 0);
      dialog_sep = XmxMakeHorizontalSeparator(mail_form);

      buttons_form = XmxMakeFormAndThreeButtons(mail_form, mail_win_cb,
						"Mail", "Dismiss", "Help...",
						0, 1, 2);
      /* Constraints for mail_form. */
      XmxSetOffsets(to_label, 14, 0, 10, 0);
      XmxSetConstraints
        (to_label, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_NONE,
         NULL, NULL, NULL, NULL);
      XmxSetOffsets(win->mail_to_text, 10, 0, 5, 10);
      XmxSetConstraints
        (win->mail_to_text, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_WIDGET,
         XmATTACH_FORM, NULL, NULL, to_label, NULL);

      XmxSetOffsets(subj_label, 14, 0, 10, 0);
      XmxSetConstraints
        (subj_label, XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_FORM, 
         XmATTACH_NONE,
         win->mail_to_text, NULL, NULL, NULL);
      XmxSetOffsets(win->mail_subj_text, 10, 0, 5, 10);
      XmxSetConstraints
        (win->mail_subj_text, XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_WIDGET,
         XmATTACH_FORM, win->mail_to_text, NULL, subj_label, NULL);

      XmxSetOffsets(frame, 10, 0, 10, 10);
      XmxSetConstraints
        (frame, XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM,
         win->mail_subj_text, NULL, NULL, NULL);

      XmxSetOffsets(frame2, 10, 0, 10, 10);
      XmxSetConstraints
        (frame2, XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM,
         frame, NULL, NULL, NULL);

      XmxSetArg(XmNtopOffset, 10);
      XmxSetConstraints 
        (dialog_sep, XmATTACH_WIDGET, XmATTACH_WIDGET, XmATTACH_FORM, 
         XmATTACH_FORM,
         frame2, buttons_form, NULL, NULL);
      XmxSetConstraints 
        (buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM,
         NULL, NULL, NULL, NULL);
  }
  XmxManageRemanage(win->mail_win);

  return mo_succeed;
}

mo_status mo_print_window(mo_window *win, mo_format_token print_format,
			  char *lpr)
{
  char *fnam, *cmd, *text;
  FILE *fp;
  mo_window *next = NULL;
  mo_window *top = win;
  int freeit;

  fnam = mo_tmpnam(win->current_node->url);
#ifdef VMS
  strcat(fnam, ".");
  if (print_format == mo_postscript)
      strcat(fnam, "ps");
#endif /* VMS, BSN add a dot or more to filename */

#ifdef CCI
  if (cci_docommand) {
      if (print_format == mo_postscript) {
          HTML_Print_Headers = HTML_Print_Footers = HTML_Print_Duplex = 1;
      } else {
          HTML_Print_Headers = HTML_Print_Footers = HTML_Print_Duplex = 0;
      }
      HTML_Print_Paper_Size_A4 = 0;
  } else
#endif
  {
      HTML_Print_Headers = XmToggleButtonGetState(
						win->print_header_toggle_print);
      HTML_Print_Footers = XmToggleButtonGetState(
						win->print_footer_toggle_print);
      HTML_Print_Paper_Size_A4 = XmToggleButtonGetState(
						win->print_a4_toggle_print);
      HTML_Print_Duplex = XmToggleButtonGetState(
						win->print_duplex_toggle_print);
  }

#ifndef VMS
  fp = fopen(fnam, "w");
  if (!fp)
      goto oops;
#else
  /* Open for efficient writes, VaxC RMS defaults are pitiful. PGE */
  fp = fopen(fnam, "w", "shr = nil", "rop = WBH", "mbf = 4",
             "mbc = 32", "deq = 8", "fop = tef");
  if (!fp) {
      VMS_ErrorDialog(fnam, win);
      goto oops;
  }
#endif /* VMS, BSN, PGE */

  while (win) {
      /* Do if no frames except inline frames or html requested */
      if (!win->frames || (win->frames->is_frame == 2) ||
	  (print_format == mo_html)) {
	  freeit = 1;
	  text = NULL;
          if (print_format == mo_plaintext) {
	      if (win->is_frame) {
	          text = HTMLGetText(win->scrolled_win, 0, win->frameurl, NULL);
	      } else {
	          text = HTMLGetText(win->scrolled_win, 0,
				     win->current_node->url, NULL);
	      }
          } else if (print_format == mo_formatted_text) {
	      if (win->is_frame) {
	          text = HTMLGetText(win->scrolled_win, 1, win->frameurl, NULL);
	      } else {
	          text = HTMLGetText(win->scrolled_win, 1,
				     win->current_node->url, NULL);
	      }
          } else if (print_format == mo_postscript) {
	      if (win->is_frame) {
	          text = HTMLGetText(win->scrolled_win, 2 + win->font_family,
				     win->frameurl, " ");
	      } else {
	          text = HTMLGetText(win->scrolled_win, 2 + win->font_family,
				     win->current_node->url,
				     win->current_node->last_modified);
	      }
          } else if (!win->is_frame && win->current_node &&
		     win->current_node->text) {
              text = win->current_node->text;
	      freeit = 0;
          } else if (win->is_frame && win->frametext) {
              text = win->frametext;
	      freeit = 0;
          }
          if (text) {
#ifndef VMS
              fputs(text, fp);
#else
              FPUTS(text, fp, fnam, top);
#endif
	      if (freeit)
                  free(text);
          }
      }

      if (win->frames) {
	  next = win->next_frame;
	  win = win->frames;
      } else {
          win = win->next_frame;
      }
      if (!win && next) {
	  win = next;
	  next = NULL;
      }
  }
  fclose(fp);

  cmd = (char *)malloc(strlen(lpr) + strlen(fnam) + 24);
  sprintf(cmd, "%s %s", lpr, fnam);
  GUI_System(cmd, "Print Information");
  free(cmd);

 oops:
#ifndef VMS
  unlink(fnam); 
#endif
  free(fnam);

  return mo_succeed;
}

/* ----------------------- mo_post_print_window ------------------------ */

static XmxCallback(print_win_cb)
{
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
  char *lpr;

  switch (XmxExtractToken((int)client_data)) {
    case 0:
      XtUnmanageChild(win->print_win);

      lpr = XmxTextGetString(win->print_text);
      if (!lpr || !*lpr) {
	  if (lpr)
	      XtFree(lpr);
          return;
      }
      mo_print_window(win, win->print_format, lpr);
      XtFree(lpr);
      break;
    case 1:
      /* Dismiss it and return */
      XtUnmanageChild(win->print_win);
      break;
    case 2:
      mo_open_another_window(win,
			mo_assemble_help_url("docview-menubar-file.html#print"),
			NULL, NULL);
      break;
  }
  return;
}


static void print_sensitive(mo_window *win, int format)
{
	Arg args[2];
  
	if (format == mo_postscript) {
		/* Postscript */
		int us_size = get_pref_boolean(ePRINT_PAPER_SIZE_US);

		XmxSetToggleButton(win->print_header_toggle_print,
				   get_pref_boolean(ePRINT_BANNERS));
		XmxSetToggleButton(win->print_footer_toggle_print,
				   get_pref_boolean(ePRINT_FOOTNOTES));
                XmxSetToggleButton(win->print_a4_toggle_print, !us_size);
                XmxSetToggleButton(win->print_us_toggle_print, us_size);
                XmxSetToggleButton(win->print_duplex_toggle_print,
				   get_pref_boolean(ePRINT_DUPLEX));

		XtSetArg(args[0], XmNsensitive, TRUE);
	} else {
		XmxSetToggleButton(win->print_header_toggle_print, XmxNotSet);
		XmxSetToggleButton(win->print_footer_toggle_print, XmxNotSet);
                XmxSetToggleButton(win->print_a4_toggle_print, XmxNotSet);
                XmxSetToggleButton(win->print_us_toggle_print, XmxNotSet);
                XmxSetToggleButton(win->print_duplex_toggle_print, XmxNotSet);

		XtSetArg(args[0], XmNsensitive, FALSE);
	}
	XtSetValues(win->print_header_toggle_print, args, 1);
	XtSetValues(win->print_footer_toggle_print, args, 1);
        XtSetValues(win->print_a4_toggle_print, args, 1);
        XtSetValues(win->print_us_toggle_print, args, 1);
        XtSetValues(win->print_duplex_toggle_print, args, 1);

	return;
}


static XmxCallback(print_fmtmenu_cb)
{
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
    char *mode;

    win->print_format = XmxExtractToken((int)client_data);
    print_sensitive(win, win->print_format);

    switch (win->print_format) {
	case mo_plaintext:
	    mode = strdup(MODE_PLAIN);
	    break;
	case mo_postscript:
	    mode = strdup(MODE_POSTSCRIPT);
	    break;
	case mo_formatted_text:
	    mode = strdup(MODE_FORMATTED);
	    break;
	case mo_html:
	    mode = strdup(MODE_HTML);
    }
    set_pref(ePRINT_MODE, (void *)mode);

    return;
}

mo_status mo_post_print_window(mo_window *win)
{
  if (!win->print_win) {
      Widget dialog_frame, dialog_sep, buttons_form;
      Widget print_form, print_label;
      Widget frame, workarea, format_label;
      Widget paper_size_toggle_box;
      int i;

      /* Create it for the first time. */
      XmxSetUniqid(win->id);
      win->print_win = XmxMakeFormDialog(win->base,
					 "VMS Mosaic: Print Document");
      dialog_frame = XmxMakeFrame(win->print_win, XmxShadowOut);

      /* Constraints for base. */
      XmxSetConstraints 
        (dialog_frame, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM, XmATTACH_FORM, NULL, NULL, NULL, NULL);

      /* Main form */
      print_form = XmxMakeForm(dialog_frame);

      print_label = XmxMakeLabel(print_form, "Print Command: ");
      XmxSetArg(XmNwidth, 400);
      if (get_pref_boolean(eKIOSK) && get_pref_boolean(eKIOSKPRINT))
	  XmxSetArg (XmNsensitive, False);

      win->print_text = XmxMakeTextField(print_form);
      XmxTextSetString(win->print_text, get_pref(ePRINT_COMMAND));

      {
	int us_size = get_pref_boolean(ePRINT_PAPER_SIZE_US);
	char *mode;

        XmxSetArg(XmNmarginWidth, 5);
        XmxSetArg(XmNmarginHeight, 5);
        frame = XmxMakeFrame(print_form, XmxShadowEtchedIn);
        workarea = XmxMakeForm(frame);

	win->print_header_toggle_print = XmxMakeToggleButton(workarea,
				   "Include Banners", print_print_header_cb, 0);
	XmxSetToggleButton(win->print_header_toggle_print,
			   get_pref_boolean(ePRINT_BANNERS));

	win->print_footer_toggle_print = XmxMakeToggleButton(workarea,
				 "Include Footnotes", print_print_footer_cb, 0);
	XmxSetToggleButton(win->print_footer_toggle_print,
			   get_pref_boolean(ePRINT_FOOTNOTES));

	paper_size_toggle_box = XmxMakeRadioBox(workarea);
	win->print_a4_toggle_print = XmxMakeToggleButton(paper_size_toggle_box,
				       "A4 Paper Size", print_print_size_cb, 0);
	win->print_us_toggle_print = XmxMakeToggleButton(paper_size_toggle_box,
				"US Letter Paper Size", print_print_size_cb, 0);
	XmxSetToggleButton(win->print_a4_toggle_print, !us_size);
	XmxSetToggleButton(win->print_us_toggle_print, us_size);

	win->print_duplex_toggle_print = XmxMakeToggleButton(workarea,
				   "Duplex printing", print_print_duplex_cb, 0);
	XmxSetToggleButton(win->print_duplex_toggle_print,
			   get_pref_boolean(ePRINT_DUPLEX));

        format_label = XmxMakeLabel(workarea, "Format for document:");

        /* Set the default mode */
	mode = get_pref_string(ePRINT_MODE);
	if (!mode || !*mode) {
	    char tbuf[BUFSIZ];

	    sprintf(tbuf,
		"You have set the default print mode to:\n     [NULL], which is not valid. Defaulting to %s mode.\n\nPlease use one of the following:\n     plain, formatted, postscript, or html.",
		"plain text print");
	    application_user_info_wait(tbuf);
	    set_pref(ePRINT_MODE, (void *)strdup(MODE_PLAIN));
	    mode = get_pref_string(ePRINT_MODE);
	}
        for (i = 0; i < 4; i++)
            format_opts[i].set_state = XmxNotSet;

	if (!my_strcasecmp(mode, MODE_HTML)) {
            format_opts[3].set_state = XmxSet;
	    win->print_format = mo_html;
	} else if (!my_strcasecmp(mode, MODE_POSTSCRIPT)) {
            format_opts[2].set_state = XmxSet;
	    win->print_format = mo_postscript;
	} else if (!my_strcasecmp(mode, MODE_FORMATTED)) {
            format_opts[1].set_state = XmxSet;
	    win->print_format = mo_formatted_text;
	} else if (!my_strcasecmp(mode, MODE_PLAIN)) {
            format_opts[0].set_state = XmxSet;
	    win->print_format = mo_plaintext;
	} else {
	    char tbuf[BUFSIZ];

	    sprintf(tbuf,
		"You have set the default print mode to:\n     [%s], which is not valid. Defaulting to %s mode.\n\nPlease use one of the following:\n     plain, formatted, postscript, or html.",
		mode, "plain text print");
	    application_user_info_wait(tbuf);
            format_opts[0].set_state = XmxSet;
	    win->print_format = mo_plaintext;
	}

	win->print_fmtmenu = XmxRMakeOptionMenu(workarea, "", print_fmtmenu_cb, 
                                                format_opts);
        XmxSetArg(XmNtopOffset, 7);
        XmxSetConstraints
          (format_label, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_FORM,
           XmATTACH_NONE, NULL, NULL, NULL, NULL);
        XmxSetArg(XmNleftOffset, 0);
        XmxSetConstraints
          (win->print_fmtmenu->base, XmATTACH_FORM, XmATTACH_NONE, 
           XmATTACH_WIDGET, XmATTACH_NONE, NULL, NULL, format_label, NULL);
        XmxSetArg(XmNtopOffset, 15);
	XmxSetConstraints
	  (win->print_header_toggle_print, XmATTACH_WIDGET, XmATTACH_NONE,
	   XmATTACH_FORM, XmATTACH_NONE, format_label, NULL, NULL, NULL);
	XmxSetConstraints
	  (win->print_footer_toggle_print, XmATTACH_WIDGET, XmATTACH_NONE,
	   XmATTACH_FORM, XmATTACH_NONE,
	   win->print_header_toggle_print, NULL, NULL, NULL);
	XmxSetConstraints
	  (paper_size_toggle_box, XmATTACH_WIDGET, XmATTACH_NONE,
	   XmATTACH_FORM, XmATTACH_NONE,
	   win->print_footer_toggle_print, NULL, NULL, NULL);
	XmxSetConstraints
	  (win->print_duplex_toggle_print, XmATTACH_WIDGET, XmATTACH_FORM,
	   XmATTACH_FORM, XmATTACH_NONE,
	   paper_size_toggle_box, NULL, NULL, NULL);

        print_sensitive(win, win->print_format);
      }

      dialog_sep = XmxMakeHorizontalSeparator(print_form);
      
      buttons_form = XmxMakeFormAndThreeButtonsTight(print_form, print_win_cb,
						  "Print", "Dismiss", "Help...",
						  0, 1, 2);
      /* Constraints for print_form. */
      XmxSetOffsets(print_label, 14, 0, 10, 0);
      XmxSetConstraints
        (print_label, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_FORM,
	 XmATTACH_NONE, NULL, NULL, NULL, NULL);
      XmxSetOffsets(win->print_text, 10, 0, 5, 10);
      XmxSetConstraints
        (win->print_text, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_WIDGET,
         XmATTACH_FORM, NULL, NULL, print_label, NULL);

      XmxSetOffsets(frame, 10, 0, 10, 10);
      XmxSetConstraints
        (frame, XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM,
         win->print_text, NULL, NULL, NULL);

      XmxSetArg(XmNtopOffset, 10);
      XmxSetConstraints 
        (dialog_sep, XmATTACH_WIDGET, XmATTACH_WIDGET, XmATTACH_FORM, 
         XmATTACH_FORM,
         frame, buttons_form, NULL, NULL);

      XmxSetConstraints 
        (buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM,
         NULL, NULL, NULL, NULL);
  }
  XmxManageRemanage(win->print_win);

  return mo_succeed;
}


/* ----------------------- mo_post_source_window ------------------------ */

/*
 * Okay...forward caseless search works...I think forward caseful search works.
 *
 * Reverse searching is screwed up as far as where the point that gets
 * highlighted and the actual cursor position.  Should be easy to fix
 * with a little work.
 */

static void source_position(Widget source_view, int pos, int end)
{
  XmTextSetSelection(source_view, pos, end == (-1) ? pos : end, CurrentTime);
  XmTextShowPosition(source_view, pos);
  return;
}


mo_status mo_source_search_window(mo_window *win, char *str, int backward,
				  int caseless)
{
	int searchlen, start;
	int str_len = strlen(str);
	char *ptr = NULL;
	char *tptr = NULL;
	char *my_str = NULL;

	if (!win || !win->current_node || !win->current_node->text ||
	    !*win->current_node->text) {
		application_user_info_wait(
		    "This is a bug! Please report what you were\ndoing and the URL you are current at to:\n\ncook@wvnet.edu\n\nThank You!!");
		return(mo_fail);
	}
	searchlen = strlen(win->current_node->text);

#ifdef CCI
	/* Search the first hit every time if by cci application */ 
	if (cci_docommand)
		win->src_search_pos = 0;
#endif
	/*
	 * If we are going forwards, the start position is the current
	 *   search position.
	 * If we are going backwards, the start position is the current
	 *   search position - the current search string length.
	 * If the current position is non-zero, this is a "find again" type
	 *   search which is why the "backwards" way works.
	 */
	if (!backward) {  /* Forward Search */
		start = win->src_search_pos;
		if (start >= searchlen) {
			if (win->src_search_pos) {
				application_user_info_wait(
				    "Sorry, no more matches in this document.");
			} else {
				application_user_info_wait(
				    "Sorry, no matches in this document.");
			}
			return(mo_fail);
		}
		ptr = my_str = strdup(win->current_node->text + start);
	} else {  /* Backwards Search */
		if (!win->src_search_pos) {  /* First time...go to end */
			start = searchlen;
		} else {  /* "Find Again" */
			start = win->src_search_pos - str_len;
			if (start < 0) {
				if (win->src_search_pos) {
					application_user_info_wait(
					    "Sorry, no more matches in this document.");
				} else {
					application_user_info_wait(
					     "Sorry, no matches in this document.");
				}
				return(mo_fail);
			}
		}
		my_str = strdup(win->current_node->text);
		*(my_str + start) = '\0';
	}

	while (1) {
		if (!backward) {
			if (tptr) {
				ptr++;
				if (!ptr || !*ptr) {
					ptr = NULL;
					break;
				}
			}
			if (caseless) {
				/* Find occurrence */
				tptr = ptr = strcasechr(ptr, *str);
			} else {
				/* Find occurrence */
				tptr = ptr = strchr(ptr, *str);
			}
		} else {
			if (tptr)
				*tptr = '\0';
			if (caseless) {
				tptr = ptr = strrcasechr(my_str, *str);
			} else {
				tptr = ptr = strrchr(my_str, *str);
			}
		}
		if (!ptr)
			break;
		if (caseless) {
			if (!my_strncasecmp(ptr, str, str_len))
				break;
			continue;
		} else {
			if (!strncmp(ptr, str, str_len))
				break;
			continue;
		}
	}
	if (!ptr) {
		free(my_str);

		if (win->src_search_pos) {
			application_user_info_wait(
				"Sorry, no more matches in this document.");
		} else {
			application_user_info_wait(
				"Sorry, no matches in this document.");
		}
		return(mo_fail);
	}
	if (!backward) {
		win->src_search_pos = (ptr - my_str) + start + str_len;
		source_position(win->source_text,
				win->src_search_pos - str_len,
				win->src_search_pos);
	} else {
		win->src_search_pos = ptr - my_str;
		source_position(win->source_text,
				win->src_search_pos,
				win->src_search_pos + str_len);
	}
	free(my_str);

	return(mo_succeed);
}


static XmxCallback(source_search_win_cb)
{
	mo_window *win =
		      mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

	if (!win)
		win = current_win;

	switch (XmxExtractToken((int)client_data)) {
	    case 0: {  /* Search */
		char *str = XmxTextGetString(win->src_search_win_text);

		if (str && *str) {
			mo_source_search_window(win, str,
					  XmToggleButtonGetState(
					      win->src_search_backwards_toggle),
					  XmToggleButtonGetState(
					      win->src_search_caseless_toggle));
		}
		if (str)
			XtFree(str);
		break;
	    }

	    case 1:  /* Reset */
		/* Clear out the search text. */
		XmxTextSetString(win->src_search_win_text, "");

		/* Subsequent searches start at the beginning. */
		win->src_search_pos = 0;

		/* Reposition document at top of screen. */
		source_position(win->source_text, 0, -1);
		break;

	    case 2:  /* Dismiss */
		XtUnmanageChild(win->src_search_win);
		break;

	    case 3:  /* Help */
		mo_open_another_window(win, mo_assemble_help_url(
			      	            "docview-menubar-file.html#search"),
			      	       NULL, NULL);
		break;
	}
	return;
}

static mo_status mo_post_source_search_window(mo_window *win)
{
	if (!win->src_search_win) {
		Widget dialog_frame, dialog_sep;
		Widget buttons_form, search_form, label;
      
		/* Create it for the first time. */
		XmxSetUniqid(win->id);
		win->src_search_win = XmxMakeFormDialog(win->base,
				           "VMS Mosaic: Search in Source View");
		dialog_frame = XmxMakeFrame(win->src_search_win, XmxShadowOut);

		/* Constraints for base. */
		XmxSetConstraints(dialog_frame,
				  XmATTACH_FORM, XmATTACH_FORM, 
				  XmATTACH_FORM, XmATTACH_FORM,
				  NULL, NULL, NULL, NULL);
		/* Main form. */
		search_form = XmxMakeForm(dialog_frame);
      
		label = XmxMakeLabel(search_form,
				     "Find string in Source View: ");
		XmxSetArg(XmNcolumns, 25);
		win->src_search_win_text = XmxMakeText(search_form);
		XmxAddCallbackToText(win->src_search_win_text,
				     source_search_win_cb, 0);
#ifdef MOTIF1_2 /* Not in Motif 1.1 */
		/* Make keyboardFocusPolicy:explicit work properly */
		XmxSetArg(XmNinitialFocus, (XtArgVal)win->src_search_win_text);
		XmxSetValues(search_form);
#endif
		win->src_search_caseless_toggle =
			XmxMakeToggleButton(search_form, "Caseless Search",
					    NULL, 0);
		XmxSetToggleButton(win->src_search_caseless_toggle, XmxSet);
		XmxAddClue(win->src_search_caseless_toggle,
			   "Ignores upper/lower case when selected");
		win->src_search_backwards_toggle =
			XmxMakeToggleButton(search_form, "Backwards Search",
					    NULL, 0);
		XmxAddClue(win->src_search_backwards_toggle,
			   "Search in reverse direction when selected");
		dialog_sep = XmxMakeHorizontalSeparator(search_form);
      
		buttons_form = XmxMakeFormAndFourButtons(search_form,
					  source_search_win_cb, 
					  "Find", "Reset", "Dismiss", "Help...",
					  0, 1, 2, 3);
		XmxSetButtonClue("Find string in source window",
				 "Reset this menu", "Close this menu",
				 "Open help in new Mosaic window", NULL);
		/* Constraints for search_form. */
		XmxSetOffsets(label, 13, 0, 10, 0);
		/* Label attaches top to form, bottom to nothing,
		 * left to form, right to nothing. */
		XmxSetConstraints(label,
				  XmATTACH_FORM, XmATTACH_NONE,
				  XmATTACH_FORM, XmATTACH_NONE,
				  NULL, NULL, NULL, NULL);
		XmxSetOffsets(win->src_search_win_text, 10, 0, 5, 8);
		/* search_win_text attaches top to form, bottom to nothing,
		 * left to label, right to form. */
		XmxSetConstraints(win->src_search_win_text,
				  XmATTACH_FORM, XmATTACH_NONE,
				  XmATTACH_WIDGET, XmATTACH_FORM,
				  NULL, NULL, label, NULL);

		/* search_caseless_toggle attaches top to search_win_text,
		 * bottom to nothing, left to position, right to position. */
		XmxSetConstraints(win->src_search_caseless_toggle,
				  XmATTACH_WIDGET, XmATTACH_NONE,
				  XmATTACH_WIDGET, XmATTACH_NONE, 
				  win->src_search_win_text, NULL, label, NULL);
		XmxSetOffsets(win->src_search_caseless_toggle, 8, 0, 2, 0);

		/* search_backwards_toggle attaches top to
		 * search_caseless_toggle, bottom to nothing,
		 * left to position, right to position. */
		XmxSetConstraints(win->src_search_backwards_toggle,
				  XmATTACH_WIDGET, XmATTACH_NONE,
				  XmATTACH_WIDGET, XmATTACH_NONE,
				  win->src_search_caseless_toggle, 
				  NULL, label, NULL);
		XmxSetOffsets(win->src_search_backwards_toggle, 8, 0, 2, 0);

		XmxSetOffsets(dialog_sep, 8, 0, 0, 0);
		/* dialog_sep attaches top to search_backwards_toggle,
		 * bottom to buttons_form, left to form, right to form */
		XmxSetConstraints(dialog_sep,
				  XmATTACH_WIDGET, XmATTACH_WIDGET,
				  XmATTACH_FORM, XmATTACH_FORM,
				  win->src_search_backwards_toggle,
				  buttons_form, NULL, NULL);
		XmxSetConstraints(buttons_form,
				  XmATTACH_NONE, XmATTACH_FORM,
				  XmATTACH_FORM, XmATTACH_FORM,
				  NULL, NULL, NULL, NULL);
	}
	XmxManageRemanage(win->src_search_win);
  
	return(mo_succeed);
}


static XmxCallback(source_win_cb)
{
	mo_window *win =
		      mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

        switch (XmxExtractToken((int)client_data)) {
	    case 0:  /* Dismiss */
	        XtUnmanageChild(win->source_win);
		if (win->src_search_win && XtIsManaged(win->src_search_win))
			XtUnmanageChild(win->src_search_win);
		break;

	    case 1:  /* Help */
		mo_open_another_window(win, mo_assemble_help_url(
					    "docview-menubar-file.html#source"),
			      	       NULL, NULL);
		break;

	    case 2:  /* Search */
		mo_post_source_search_window(win);
		break;
	}
	return;
}


mo_status mo_post_source_window(mo_window *win)
{
	if (!win->source_win) {
		Widget dialog_frame, dialog_sep, buttons_form;
		Widget source_form, label, dlabel;
      
		/* Create it for the first time. */
		XmxSetUniqid(win->id);
		win->source_win = XmxMakeFormDialog(win->base,
					    "VMS Mosaic: Document Source View");
		dialog_frame = XmxMakeFrame(win->source_win, XmxShadowOut);

		/* Constraints for base. */
		XmxSetConstraints(dialog_frame,
				  XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM,
				  XmATTACH_FORM, NULL, NULL, NULL, NULL);
		/* Main form. */
		source_form = XmxMakeForm(dialog_frame);
      
		label =	XmxMakeLabel(source_form, "URL: ");
		dlabel=	XmxMakeLabel(source_form, "Last Modified: ");
		XmxSetArg(XmNcursorPositionVisible, False);
		XmxSetArg(XmNeditable, False);
		win->source_url_text = XmxMakeText(source_form);
		XmxSetArg(XmNcursorPositionVisible, False);
		XmxSetArg(XmNeditable, False);
		win->source_date_text = XmxMakeText(source_form);
      
		/* Info window: text widget, not editable. */
		XmxSetArg(XmNscrolledWindowMarginWidth, 10);
		XmxSetArg(XmNscrolledWindowMarginHeight, 10);
		XmxSetArg(XmNcursorPositionVisible, True);
		XmxSetArg(XmNeditable, False);
		XmxSetArg(XmNeditMode, XmMULTI_LINE_EDIT);
		XmxSetArg(XmNrows, 15);
		XmxSetArg(XmNcolumns, 80);
		win->source_text = XmxMakeScrolledText(source_form);
      
		dialog_sep = XmxMakeHorizontalSeparator(source_form);

		buttons_form = XmxMakeFormAndThreeButtonsTight(source_form,
							       source_win_cb,
							       "Search...",
							       "Dismiss",
							       "Help...",
							       2, 0, 1);
		XmxSetButtonClue("Open source search menu", "Close this menu",
				 "Open help in new Mosaic window", NULL, NULL);

		/* Constraints for source_form. */
		XmxSetOffsets(label, 13, 0, 10, 0);
		XmxSetConstraints(label,
				  XmATTACH_FORM, XmATTACH_NONE,
				  XmATTACH_FORM, XmATTACH_NONE,
				  NULL, NULL, NULL, NULL);
		XmxSetOffsets(dlabel, 18, 0, 10, 0);
		XmxSetConstraints(dlabel,
				  XmATTACH_WIDGET, XmATTACH_NONE, 
				  XmATTACH_FORM, XmATTACH_NONE,
				  label, NULL, NULL, NULL);
		XmxSetOffsets(win->source_url_text, 10, 0, 5, 10);
		XmxSetConstraints(win->source_url_text,
				  XmATTACH_FORM, XmATTACH_NONE,
				  XmATTACH_WIDGET, XmATTACH_FORM,
				  NULL, NULL, label, NULL);
		XmxSetOffsets(win->source_date_text, 10, 0, 5, 10);
		XmxSetConstraints(win->source_date_text,
				  XmATTACH_WIDGET, XmATTACH_NONE,
				  XmATTACH_WIDGET, XmATTACH_FORM,
				  win->source_url_text, NULL, dlabel, NULL);
		XmxSetConstraints(XtParent(win->source_text),
				  XmATTACH_WIDGET, XmATTACH_WIDGET, 
				  XmATTACH_FORM, XmATTACH_FORM,
				  win->source_date_text,
				  dialog_sep, NULL, NULL);
		XmxSetArg(XmNtopOffset, 10);
		XmxSetConstraints(dialog_sep,
				  XmATTACH_NONE, XmATTACH_WIDGET,
				  XmATTACH_FORM, XmATTACH_FORM,
				  NULL, buttons_form, NULL, NULL);
		XmxSetConstraints(buttons_form,
				  XmATTACH_NONE, XmATTACH_FORM,
				  XmATTACH_FORM, XmATTACH_FORM,
				  NULL, NULL, NULL, NULL);
	}
	XmxManageRemanage(win->source_win);

	if (win->current_node) {
		XmxTextSetString(win->source_text,
				 win->current_node->text);
		XmxTextSetString(win->source_url_text,
				 win->current_node->url);
		XmxTextSetString(win->source_date_text,
				 win->current_node->last_modified ?
				 win->current_node->last_modified : "Unknown");
	}
	return(mo_succeed);
}


/* ----------------------- mo_search_window ------------------------ */
mo_status mo_search_window(mo_window *win, char *str, int backward,
			   int caseless, int news)
{
  int rc;

#ifdef CCI
  /* Search the first hit every time if by cci application */ 
  if (cci_docommand || news)
#else
  if (news)
#endif
      ((ElementRef *)win->search_start)->id = 0;

  if (!backward) {
      /* Either win->search_start->id is 0, in which case the search
       * should start from the beginning, or it's non-0, in which case
       * at least one search step has already been taken.
       * If the latter, it should be incremented so as to start
       * the search after the last hit.  Right? */
      if (((ElementRef *)win->search_start)->id) {
          ((ElementRef *)win->search_start)->id =
			((ElementRef *)win->search_end)->id;
          ((ElementRef *)win->search_start)->pos =
		        ((ElementRef *)win->search_end)->pos;
      }
  }

  if (news) {
      rc = HTMLSearchNews(win->scrolled_win,
			  (ElementRef *)win->search_start,
			  (ElementRef *)win->search_end);
  } else {
      rc = HTMLSearchText(win->scrolled_win, str, 
			  (ElementRef *)win->search_start,
			  (ElementRef *)win->search_end, 
			  backward, caseless);
  }
  if (rc == -1) {
#ifdef CCI
      if (cci_docommand) {
	  if (news)
	      ((ElementRef *)win->search_start)->id = 0;
	  return mo_fail;
      } else
#endif
      {
          /* No match was found. */
          if (!news) {
	      if (((ElementRef *)win->search_start)->id) {
	          application_user_info_wait(
				    "Sorry, no more matches in this document.");
	      } else {
	          application_user_info_wait(
				    "Sorry, no matches in this document.");
	      }
          }
          if (news)
	      ((ElementRef *)win->search_start)->id = 0;
          return mo_fail;
      }
  } else {
      /* Now search_start and search_end are starting and ending
       * points of the match. */
      HTMLGotoId(win->scrolled_win, ((ElementRef *)win->search_start)->id,
		 news ? (-1) : 0);

      /* Set the selection. */
      if (!news) {
	  HTMLSetSelection(win->scrolled_win, (ElementRef *)win->search_start,
			   (ElementRef *)win->search_end);
      }
  }  /* Found a target */

  if (news)
      ((ElementRef *)win->search_start)->id = 0;

  return mo_succeed;
}

static XmxCallback(search_win_cb)
{
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

  if (!win)
      win = current_win;

  switch (XmxExtractToken((int)client_data)) {
    case 0: {  /* Search */
        char *str = XmxTextGetString(win->search_win_text);

        if (str && *str) {
            int backward = XmToggleButtonGetState(win->search_backwards_toggle);
            int caseless = XmToggleButtonGetState(win->search_caseless_toggle);

	    mo_search_window(win, str, backward, caseless, 0);
	}
	if (str)
	    XtFree(str);
      }
      break;

    case 1:  /* Reset */
      /* Clear out the search text. */
      XmxTextSetString(win->search_win_text, "");

      /* Subsequent searches start at the beginning. */
      ((ElementRef *)win->search_start)->id = 0;

      /* Reposition document at top of screen. */
      HTMLGotoId(win->scrolled_win, 0, 0);
      break;

    case 2:  /* Dismiss */
      XtUnmanageChild(win->search_win);
      break;

    case 3:  /* Help */
      mo_open_another_window(win, 
         	       mo_assemble_help_url("docview-menubar-file.html#search"),
         	       NULL, NULL);
      break;
  }
  return;
}

mo_status mo_post_search_window(mo_window *win)
{
  if (!win->search_win) {
      Widget dialog_frame, dialog_sep, buttons_form;
      Widget search_form, label;
      
      /* Create it for the first time. */
      XmxSetUniqid(win->id);
      win->search_win = XmxMakeFormDialog(win->base,
					  "VMS Mosaic: Find In Document");
      dialog_frame = XmxMakeFrame(win->search_win, XmxShadowOut);

      /* Constraints for base. */
      XmxSetConstraints(dialog_frame, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM, XmATTACH_FORM, NULL, NULL, NULL, NULL);
      
      /* Main form. */
      search_form = XmxMakeForm(dialog_frame);
      
      label = XmxMakeLabel(search_form, "Find string in document: ");
      XmxSetArg(XmNcolumns, 25);
      win->search_win_text = XmxMakeText(search_form);
      XmxAddCallbackToText(win->search_win_text, search_win_cb, 0);
#ifdef MOTIF1_2 /* Not in Motif 1.1 */
      /* Make keyboardFocusPolicy:explicit work properly */
      XmxSetArg(XmNinitialFocus, (XtArgVal)win->search_win_text);
      XmxSetValues(search_form);
#endif
      win->search_caseless_toggle = XmxMakeToggleButton(search_form,
						    "Caseless Search", NULL, 0);
      XmxSetToggleButton(win->search_caseless_toggle, XmxSet);
      XmxAddClue(win->search_caseless_toggle,
		 "Ignores upper/lower case when selected");
      win->search_backwards_toggle = XmxMakeToggleButton(search_form,
						   "Backwards Search", NULL, 0);
      XmxAddClue(win->search_backwards_toggle,
		 "Search in reverse direction when selected");

      dialog_sep = XmxMakeHorizontalSeparator(search_form);
      
      buttons_form = XmxMakeFormAndFourButtons(search_form, search_win_cb,
					  "Find", "Reset", "Dismiss", "Help...",
					  0, 1, 2, 3);
      XmxSetButtonClue("Find string in current window", "Reset this menu",
  		       "Close this menu", "Open help in new Mosaic window",
		       NULL);

      /* Constraints for search_form. */
      XmxSetOffsets (label, 13, 0, 10, 0);
      /* Label attaches top to form, bottom to nothing,
       * left to form, right to nothing. */
      XmxSetConstraints
        (label, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_NONE,
         NULL, NULL, NULL, NULL);
      XmxSetOffsets(win->search_win_text, 10, 0, 5, 8);
      /* Search_win_text attaches top to form, bottom to nothing,
       * left to label, right to form. */
      XmxSetConstraints
        (win->search_win_text, XmATTACH_FORM, XmATTACH_NONE, XmATTACH_WIDGET,
         XmATTACH_FORM, NULL, NULL, label, NULL);

      /* Search_caseless_toggle attaches top to search_win_text, bottom to
       * nothing, left to position, right to position. */
      XmxSetConstraints
        (win->search_caseless_toggle, XmATTACH_WIDGET, XmATTACH_NONE,
         XmATTACH_WIDGET, XmATTACH_NONE, 
         win->search_win_text, NULL, label, NULL);
      XmxSetOffsets(win->search_caseless_toggle, 8, 0, 2, 0);

      /* Search_backwards_toggle attaches top to search_caseless_toggle,
       * bottom to nothing, left to position, right to position. */
      XmxSetConstraints
        (win->search_backwards_toggle, XmATTACH_WIDGET, XmATTACH_NONE,
         XmATTACH_WIDGET, XmATTACH_NONE, win->search_caseless_toggle, 
         NULL, label, NULL);
      XmxSetOffsets(win->search_backwards_toggle, 8, 0, 2, 0);

      XmxSetOffsets(dialog_sep, 8, 0, 0, 0);
      /* dialog_sep attaches top to search_backwards_toggle,
       * bottom to buttons_form, left to form, right to form */
      XmxSetConstraints 
        (dialog_sep, XmATTACH_WIDGET, XmATTACH_WIDGET, XmATTACH_FORM, 
         XmATTACH_FORM,
         win->search_backwards_toggle, buttons_form, NULL, NULL);
      XmxSetConstraints 
        (buttons_form, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM,
         NULL, NULL, NULL, NULL);
  }
  XmxManageRemanage(win->search_win);
  
  return mo_succeed;
}

/*------------------------------------------------------------*/

typedef struct {
	mo_window *win;
	char *fileName;
	char *url;
} EditFile;

static void mo_done_editing(EditFile *e, int pid)
{
	char *url;
	extern char *url_base_override;
	extern int keep_url_base_override;
	mo_window *win = NULL;
	int foundit = 0;

#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "Done Editing: pid = %d, file %s, url=%s\n",
			pid, e->fileName, e->url);
#endif
	/* Does window still exist? */
	while (win = mo_next_window(win)) {
		if (win == e->win)
			foundit = 1;
	}
	if (foundit) {
	  	url = mo_url_canonicalize_local(e->fileName);
		if (url[strlen(url) - 1] == '/')
			url[strlen(url) - 1] = '\0';

		/* Force a BASE url for the document */
		keep_url_base_override = 1;
		if (url_base_override)
			free(url_base_override);
		url_base_override = strdup(e->url);

		mo_load_window_text(e->win, url, e->url);
		free(url);
	}
#ifdef VMS
	/* Remove all versions */
	strcat(e->fileName, ";*");
#endif
	remove(e->fileName);

	free(e->fileName);
	free(e->url);
	free(e);
}

/* Not currently used anywhere */
static mo_status mo_source_date(mo_window *win)
{
	char msg[200];

	if (win->current_node->last_modified) {
		sprintf(msg, "Source Last Modified Date:\n  %s\n",
			win->current_node->last_modified);
	} else {
		sprintf(msg, "Source Last Modified Date is not available.\n");
	}
	application_user_info_wait(msg);

	return(mo_succeed);
}


mo_status mo_edit_source(mo_window *win)
{
	FILE *fp;
	char *sourceFileName;
	char execString[1024], editorTitle[1024], editorCommand[1024];
	char *execArg[20];
	int argCount, pid, length;
	EditFile *e;
	extern void AddChildProcessHandler(int, void (*)(), void *);
	static char *editorName, *xterm_command;
	static int use_xterm;
	static int init = 0;

	if (!init) {
		char *edit_command;

		/* Get editor */
		edit_command = get_pref_string(eEDIT_COMMAND);
		if (edit_command && *edit_command) {
			editorName = edit_command;
		} else {
			/* May not need to strdup any of these? */
			editorName = strdup(getenv("EDITOR"));
			if (!editorName || !*editorName)
#ifndef VMS
				editorName = strdup("vi");    /* Default */
#else
				editorName = strdup("EDIT");
#endif /* VMS, GEC */
		}
		use_xterm = get_pref_boolean(eEDIT_COMMAND_USE_XTERM);
		xterm_command = get_pref_string(eXTERM_COMMAND);
		init =	1;
	}

	if (!win->current_node || !win->current_node->text)
		return mo_fail;

	/* Write out source to tmp file with .html extension */
	sourceFileName = mo_tmpnam(NULL);
	/* mo_tmpnam returns room for adding stuff */
        strcat(sourceFileName, ".html");

#ifndef VMS
	if (!(fp = fopen(sourceFileName, "w"))) {
		char *buf = my_strerror(errno);
		char *final;
		char tmpbuf[80];
		int final_len;

		if (!buf || !*buf || !strcmp(buf, "Error 0")) {
			sprintf(tmpbuf, "Unknown Error");
			buf = tmpbuf;
		}
		final_len = 30 + ((!sourceFileName || !*sourceFileName ?
		      		   3 : strlen(sourceFileName)) + 13) +
			    15 + (strlen(buf) + 3);
		final = (char *)malloc(final_len);

		sprintf(final,
			"\nUnable to Open Editor Temp File:\n   %s\n\nOpen Error:\n   %s\n",
			!sourceFileName || !*sourceFileName ?
			" " : sourceFileName, buf);
		XmxMakeErrorDialog(win->base, final, "Edit Source Error");
		if (final)
			free(final);
#else
        /* Open for efficient writes, VaxC RMS defaults are pitiful. PGE */
        if (!(fp = fopen(sourceFileName, "w", "shr = nil", "rop = WBH",
                         "mbf = 4", "mbc = 32", "deq = 8", "fop = tef"))) {
                char str[1024];

                sprintf(str,
			"Unable to Open Editor Temp File: %s\nOpen Error: %s",
			sourceFileName, strerror(errno, vaxc$errno));
                XmxMakeErrorDialog(win->base, str, "Edit Source Error");
#endif /* VMS, GEC, PGE */
		return mo_fail;
	}

	length = strlen(win->current_node->text);
	if (length != fwrite(win->current_node->text,
			     sizeof(char), length, fp)) {
		fclose(fp);
		{
#ifndef VMS
			char *buf = my_strerror(errno);
			char *final;
			char tmpbuf[80];
			int final_len;

			if (!buf || !*buf || !strcmp(buf, "Error 0")) {
				sprintf(tmpbuf, "Unknown Error");
				buf = tmpbuf;
			}
			final_len = 30 +
			    ((!sourceFileName ||
			      !*sourceFileName ? 3 : strlen(sourceFileName)) +
			     13) + 15 + (strlen(buf) + 3);
			final = (char *)malloc(final_len);

			sprintf(final,
          "\nUnable to Write Editor Temp File:\n   %s\n\nWrite Error:\n   %s\n",
          (!sourceFileName || !*sourceFileName ? " " : sourceFileName), buf);

			XmxMakeErrorDialog(win->base, final,
					   "Edit Write Error");
			if (final)
				free(final);
#else
                	char str[1024];

                	sprintf(str,
			      "Unable to Write Editor Temp File: %s\nError: %s",
			      sourceFileName, strerror(errno, vaxc$errno));
                	XmxMakeErrorDialog(win->base, str, "Edit Source Error");
#endif /* VMS, GEC */
		}
		return mo_fail;
	}
	fclose(fp);

	sprintf(editorCommand, "%s %s", editorName, sourceFileName);
#ifndef VMS
	sprintf(editorTitle, "(VMS Mosaic) Editing Copy of: %.56s",
		win->current_node->url);
#else
	sprintf(editorTitle,
          "/Window=(Init=Window,Title=\"(VMS Mosaic) Editing Copy of: %.56s\")",
	   win->current_node->url);
#endif /* VMS, GEC */

	argCount = 0;
	if (use_xterm) {
#ifndef VMS
		sprintf(execString, "%s -T %s -e %s",
#else
        	sprintf(execString, "%s%s %s",
#endif /* VMS, GEC */
		        xterm_command, editorTitle, editorCommand);

		execArg[argCount++] = xterm_command;
		execArg[argCount++] = "-T";
		execArg[argCount++] = editorTitle;
		execArg[argCount++] = "-e";
	} else {
		sprintf(execString, "%s %s", editorName, sourceFileName);
	}

	execArg[argCount++] = editorName; /* Problem if there are spaces 
					   * in this edit command....will have 
					   * to parse and break up */
	execArg[argCount++] = sourceFileName;
	execArg[argCount++] = NULL;

#ifndef VMS
#ifdef __sgi
	pid = fork();
#else
	pid = vfork();
#endif
	if (!pid) {
		/* I'm the child */
	        execvp(execArg[0], execArg); 
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "Couldn't execute:\n%s\n", execString);
#endif
  		/* Don't use regular exit() or mom's I/O channels will close */
 		_exit(-1);
	}

#else
    {
#define NOWAIT 1	       /* No wait flag for LIB$SPAWN */
	void ChildTerminated();
	extern int child_count;
	int flags = NOWAIT;
	int status;
#include <descrip.h>
#define $NEW_DESCRIPTOR(name) \
        struct dsc$descriptor_s name = { \
            0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 }
        $NEW_DESCRIPTOR(cmd_desc);

        cmd_desc.dsc$w_length = strlen(execString);
        cmd_desc.dsc$a_pointer = execString;

	child_count++;
        status = lib$spawn(&cmd_desc, 0, 0, &flags, 0, &pid, 0, 0,
			   ChildTerminated, child_count);
        if (status != 1) {
                char str[1024];

                sprintf(str,
		      "Unable to Execute Editor Command: %s\nCommand Error: %s",
                      execString, strerror(errno, vaxc$errno));
                XmxMakeErrorDialog(win->base, str, "Edit Command Error");
	}
    }
#endif /* VMS, GEC */       

	/* Need to save file name and pid for later reading of source */
	if (!(e = (EditFile *) malloc(sizeof(EditFile)))) {
#ifndef DISABLE_TRACE
		if (reportBugs)
			fprintf(stderr, "Out of Memory!\n");
#endif
		return mo_fail;
	}
	e->fileName = sourceFileName;
	e->url = strdup(win->current_node->url);
	e->win = win;

	AddChildProcessHandler(pid, mo_done_editing, e);

	return mo_succeed;
} 


/*---------------------------Utility Functions-----------------------------*/

/*
 * DA FORMAT:
 *	"src" is the pathname to check for ~ expansion.  If a tilda is the
 *	first character, I expand it, store it in "dest", and return a 1.
 *	If the first character is not a tilda, I return a 0.  If "dest" does
 *	not exist, I return a -1.
 *
 * DA RULES:
 *	1) If the tilda is alone, expand it.
 *	   Ex: '~'
 *	2) If the tilda is first, followed by an alphanumeric,
 *	   stick the "path" to the home directory in front of it.
 *	   Ex: '~spowers'
 *	3) Otherwise, leave it alone.
 *
 * DA FORMULA:
 *	1) If there is a HOME variable, use it.
 *	2) If there is a password entry, use the dir from it.
 *	3) Else...use /tmp.
 *
 */
int pathEval(char *dest, char *src)
{
	int i;
	char *sptr, *hptr;
	char home[__MAX_HOME_LEN__];
#ifndef VMS
	struct passwd *pwdent;
#endif

	/*
	 * There is no place to store the result...punt.
	 */
	if (!dest) {
#ifndef DISABLE_TRACE
		if (reportBugs)
		       fprintf(stderr, "No place to put the Evaluated Path!\n");
#endif
		return(-1);
	}

	/*
	 * There's nothing to expand
	 */
	if (!src || !*src) {
		*dest = '\0';
		return(0);
	}
	if (*src != '~') {
		strcpy(dest, src);
		return(0);
	}

	/*
	 * Once here, we are gonna need to know what the expansion is...
	 *
	 * Try the HOME environment variable, then the password file, and
	 * finally give up and use /tmp.
	 */
#ifndef VMS
	if (!(hptr = getenv("HOME"))) {
		if (!(pwdent = getpwuid(getuid()))) {
			strcpy(home, "/tmp");
		} else {
			strcpy(home, pwdent->pw_dir);
		}
	} else {
		strcpy(home, hptr);
	}
#else
	hptr = getenv("HOME");
	strcpy(home, hptr);
#endif
	sptr = src;
	sptr++;
	/*
	 * Nothing after the tilda, just give dest a value and return...
	 */
	if (!sptr || !*sptr) {
		strcpy(dest, home);
		return(1);
	}

	/*
	 * The next character is a slash...so prepend home to the rest of
	 * src and return.
	 */
	if (*sptr == '/') {
		strcpy(dest, home);
		strcat(dest, sptr);
		return(1);
	}

	/*
	 * Make the assumption that they want whatever comes after to be
	 *   appended to the "HOME" path, sans the last directory (e.g.
	 *   HOME=/opt/home/spowers, we would use /opt/home<REST OF "src">)
	 */
	/*
	 * Search backwards through home for a "/" on the conditions that
	 *   this is not the slash that could possibly be at the _very_ end
	 *   of home, home[i] is not a slash, and i is >= 0.
	 *
	 * If a slash is not found (i < 0), then we assume that HOME is a
	 *   directory off of the root directory, or something strange like
	 *   that... so we simply ignore "home" and return the src without
	 *   the ~.
	 *
	 * If we do find a slash, we set the position of the slash + 1 to
	 *   NULL and store that in dest, then cat the rest of src onto
	 *   dest and return.
	 */
	for (i = strlen(home);
	     ((i >= 0) && (home[i] != '/')) || (i == strlen(home)); i--)
		;

	if (i < 0) {
		strcpy(dest, sptr);
	} else {
		home[i + 1] = '\0';
		strcpy(dest, home);
		strcat(dest, sptr);
	}
	return(1);
}
