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

/* Copyright (C) 2004, 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"
#include "../libwww2/HTNews.h"   /* Includes tcp.h so must be done early, GEC */
#include "../libwww2/HTAlert.h"
#include "mosaic.h"
#include "gui.h"
#include "gui-news.h"
#include "gui-dialogs.h"
#include "../libnut/system.h"
#include "../libnut/str-tools.h"
#include "gui-documents.h"

#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>

#include <Xm/FilesB.h>
#include <Xm/Protocols.h>

#include "../libhtmlw/HTML.h"

#ifndef VMS
#include <pwd.h>
#else
#include "vms_pwd.h"
#endif  /* PGE */

#include "newsrc.h"

/* Very non-reentrant */
/* NNTP command lines are limited to 512, so no groupname or id can be > 512 */
/* Made it 1024 just in case */
static char url[1024];

void gui_news_post_subgroupwin(mo_window *win) 
{

}

void gui_news_updateprefs(mo_window *win)
{
  if (newsShowAllGroups) {
      XmxRSetToggleState(win->menubar, mo_news_grp0, XmxSet);
      XmxRSetToggleState(win->menubar, mo_news_grp1, XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_news_grp2, XmxNotSet);
  } else if (newsShowReadGroups) {
      XmxRSetToggleState(win->menubar, mo_news_grp0, XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_news_grp1, XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_news_grp2, XmxSet);
  } else {
      XmxRSetToggleState(win->menubar, mo_news_grp0, XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_news_grp1, XmxSet);
      XmxRSetToggleState(win->menubar, mo_news_grp2, XmxNotSet);
  }

  if (newsShowAllArticles) {
      XmxRSetToggleState(win->menubar, mo_news_art0, XmxSet);
      XmxRSetToggleState(win->menubar, mo_news_art1, XmxNotSet);
  } else {
      XmxRSetToggleState(win->menubar, mo_news_art0, XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_news_art1, XmxSet);
  }

  if (ConfigView) {
      XmxRSetToggleState(win->menubar, mo_news_fmt0, XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_news_fmt1, XmxSet);
  } else {
      XmxRSetToggleState(win->menubar, mo_news_fmt0, XmxSet);
      XmxRSetToggleState(win->menubar, mo_news_fmt1, XmxNotSet);
  }
}

void gui_news_subgroup(mo_window *win)
{
  if (NewsGroup) {
      char *buf;

      subscribegroup(NewsGroup);
      newsrc_flush();
      buf = (char *) malloc(strlen(" successfully subscribed") +
	  		    strlen(NewsGroup) + 1);
      sprintf(buf, "%s successfully subscribed", NewsGroup);
      mo_gui_notify_progress(buf);
      free(buf);
  }
}

void gui_news_unsubgroup(mo_window *win)
{
  if (NewsGroup) {
      char *buf;

      unsubscribegroup(NewsGroup);
      newsrc_flush();
      buf = (char *) malloc(strlen(" successfully unsubscribed") +
	  		    strlen(NewsGroup) + 1);
      sprintf(buf, "%s successfully unsubscribed", NewsGroup);
      mo_gui_notify_progress(buf);
      free(buf);
  }
}

void gui_news_flush(mo_window *win)
{
  mo_gui_notify_progress("Flushing newsrc data");
  newsrc_flush();
  mo_gui_notify_progress("");
}

void gui_news_flushgroup(mo_window *win)
{
  if (NewsGroupS) {
      char *buf = (char *) malloc(strlen("Flushing newsrc data for ") +
				  strlen(NewsGroupS->name) + 1);

      sprintf(buf, "Flushing newsrc data for %s", NewsGroupS->name);
      mo_gui_notify_progress(buf);
      free(buf);
      newsrc_flush();
      mo_gui_notify_progress("");
  }
}

void gui_news_list(mo_window *win)
{
  mo_load_window_text(win, "news:*", NULL);
}

void gui_news_showAllGroups(mo_window *win)
{
  gui_news_flush(win);
  newsGotList = 0;
  HTSetNewsConfig(-1, -1, 1, 1, -1, -1, -1, -1);
  gui_news_updateprefs(win);
  mo_load_window_text(win, "news:*", NULL);
}

void gui_news_showGroups(mo_window *win)
{
  /* Show only subbed groups */
  HTSetNewsConfig(-1, -1, 0, 0, -1, -1, -1, -1);
  gui_news_updateprefs(win);
  mo_load_window_text(win, "news:*", NULL);
}

void gui_news_showReadGroups(mo_window *win)
{
  HTSetNewsConfig(-1, -1, 0, 1, -1, -1, -1, -1); 
  gui_news_updateprefs(win);
  mo_load_window_text(win, "news:*", NULL);
}

void gui_news_showAllArticles(mo_window *win)
{
  char *buf;

  HTSetNewsConfig(-1, 1, -1, -1, -1, -1, -1, -1); 
  gui_news_updateprefs(win);

  if (NewsGroupS) {
      buf = (char *) malloc(strlen(NewsGroupS->name) + 6);
      sprintf(buf, "news:%s", NewsGroupS->name);
  } else if (NewsGroup) {
      buf = (char *) malloc(strlen(NewsGroup) + 6);
      sprintf(buf, "news:%s", NewsGroup);
  } else {
      return;
  }
  mo_load_window_text(win, buf, NULL);
  free(buf);
}

void gui_news_showArticles(mo_window *win)
{
  char *buf;

  HTSetNewsConfig(-1, 0, -1, -1, -1, -1, -1, -1); 
  gui_news_updateprefs(win);

  if (NewsGroup) {
      buf = (char *) malloc(strlen(NewsGroup) + 6);
      sprintf(buf, "news:%s", NewsGroup);
  } else if (NewsGroupS) { 
      buf = (char *) malloc(strlen(NewsGroupS->name) + 6);
      sprintf(buf, "news:%s", NewsGroupS->name);
  } else {
      return;
  }
  mo_load_window_text(win, buf, NULL);
  free(buf);
}


void gui_news_markGroupRead(mo_window *win)
{
  char *buf;

  if (!NewsGroupS)
      return;
  markrangeread(NewsGroupS, NewsGroupS->minart, NewsGroupS->maxart);
  buf = (char *) malloc(strlen("All articles in  marked read") +
			strlen(NewsGroupS->name) + 1);
  sprintf(buf, "All articles in %s marked read", NewsGroupS->name);
  mo_gui_notify_progress(buf);

  /* Return to newsgroup list */
  mo_load_window_text(win, "news:*", NULL);
  free(buf);
}

void gui_news_markGroupUnread(mo_window *win)
{
  char *buf;

  if (!NewsGroupS)
      return;
  markrangeunread(NewsGroupS, NewsGroupS->minart, NewsGroupS->maxart);
  buf = (char *) malloc(strlen("All articles in  marked unread") +
			strlen(NewsGroupS->name) + 1);
  sprintf(buf, "All articles in %s marked unread", NewsGroupS->name);
  mo_gui_notify_progress(buf);

  /* Return to newsgroup list */
  mo_load_window_text(win, "news:*", NULL);
  free(buf);
}

void gui_news_markArticleUnread(mo_window *win)
{
  char *buf;

  if (!NewsGroupS || !CurrentArt)
      return;
  markunread(NewsGroupS, CurrentArt->num);
  buf = (char *) malloc(strlen("Article  marked unread") +
			strlen(CurrentArt->ID) + 1);
  sprintf(buf, "Article %s marked unread", CurrentArt->ID);
  mo_gui_notify_progress(buf);

  buf = (char *) realloc(buf, strlen(NewsGroup) + 6);
  sprintf(buf, "news:%s", NewsGroup);
  mo_load_window_text(win, buf, NULL);
  free(buf);
}

void gui_news_initflush(mo_window *win)
{
  newsrc_initflush();
}

void gui_news_index(mo_window *win)
{
  newsrc_flush();
  strcpy(url, win->current_node->url);
  news_index(url);
  if (*url) 
      mo_load_window_text(win, url, NULL);
}

void gui_news_prev(mo_window *win)
{
  strcpy(url, win->current_node->url);
  news_prev(url);
  if (*url)
      mo_load_window_text(win, url, NULL);
}

void gui_news_next(mo_window *win)
{
  strcpy(url, win->current_node->url);
  news_next(url);
  if (*url)
      mo_load_window_text(win, url, NULL);
}

void gui_news_prevt(mo_window *win)
{
  strcpy(url, win->current_node->url);
  news_prevt(url);
  if (*url)
      mo_load_window_text(win, url, NULL);
}

void gui_news_nextt(mo_window *win)
{
  strcpy(url, win->current_node->url);
  news_nextt(url);
  if (*url)
      mo_load_window_text(win, url, NULL);
}

static XmxCallback(include_fsb_cb)
{
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
  char efname[MO_LINE_LENGTH], line[MO_LINE_LENGTH];
  char *fname, *status;
  FILE *fp;
  long pos;

  if (!win)
      return;

  XtUnmanageChild(win->news_fsb_win);
  
  XmStringGetLtoR(((XmFileSelectionBoxCallbackStruct *)call_data)->value,
                  XmSTRING_DEFAULT_CHARSET, &fname);
  pathEval(efname, fname);
  XtFree(fname);

  if (!(fp = fopen(efname, "r"))) {
#ifndef VMS
      char *buf = my_strerror(errno);
      char *final;
      char tmpbuf[80];
      int final_len;

      if (!buf || !*buf || !strcmp(buf, "Error 0")) {
          sprintf(tmpbuf, "Unknown Error");
          buf = tmpbuf;
      }
      final_len = 30 + ((!efname || !*efname ? 3 : strlen(efname)) + 13) + 15 +
		  (strlen(buf) + 13);
      final = (char *)malloc(final_len);
      sprintf(final,
	      "\nUnable to Open Include File:\n   %s\n\nOpen Error:\n   %s\n",
	      (!efname || !*efname) ? " " : efname, buf);
      XmxMakeErrorDialog(win->news_win, final, "News Include Error");
      if (final)
	  free(final);
#else
      char str[1024];

      sprintf(str, "Unable to Open File: %s\nOpen Error: %s",
              fname, strerror(errno, vaxc$errno));
      XmxMakeErrorDialog(win->news_win, str, "Open Error");
#endif /* VMS, GEC */
      return;
  }
  
  while (1) {
      status = fgets(line, MO_LINE_LENGTH, fp);
      if (!status || !*line)
          goto done;
      
      XmTextInsert(win->news_text,
                   pos = XmTextGetInsertionPosition(win->news_text), line);
      /* Move insertion position to past this line to avoid inserting the
       * lines in reverse order */
      XmTextSetInsertionPosition(win->news_text, pos + strlen(line));
  }

 done:
  fclose(fp);
  return;
}

/* ----------------------- mo_post_news_window -------------------------- */

static XmxCallback(news_win_cb)
{
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

  switch (XmxExtractToken((int)client_data)) {
    case 0: {  /* POST */
	char *msg, *subj, *group, *from;

        XtUnmanageChild(win->news_win);

        msg = XmxTextGetString(win->news_text);
        if (!msg)
            return;
	if (!*msg) {
	    XtFree(msg);
	    return;
	}
        from = XmxTextGetString(win->news_text_from);
        subj = XmxTextGetString(win->news_text_subj);
        group = XmxTextGetString(win->news_text_group);

        NNTPpost(from, subj, NULL, group, msg);

        XtFree(msg);
        XtFree(from);
        XtFree(group);
        XtFree(subj);
    }
    case 1:  /* DISMISS */
        XtUnmanageChild(win->news_win);
	/* Since we're going to re-use this in different configs
	 * we'll destroy it */
        XtDestroyWidget(win->news_win);
        win->news_win = NULL;
        win->news_fsb_win = NULL;
        /* Do nothing. */
        break;
    case 2:  /* HELP */
        mo_open_another_window(win, mo_assemble_help_url("help-on-news.html"),
			       NULL, NULL);
        break;
    case 3:  /* INSERT FILE */
	if (!win->news_fsb_win) {
	    win->news_fsb_win = XmxMakeFileSBDialog(win->news_win,
					    "VMS Mosaic: Include File for News",
					    "Name of file to include:",
					    include_fsb_cb, 0);
	} else {
	    XmFileSelectionDoSearch(win->news_fsb_win, NULL);
	}
	XmxManageRemanage(win->news_fsb_win);
	break;
    case 4:  /* QUOTE */
	break;
  }
  return;
}


static XmxCallback(follow_win_cb)
{
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
  char *msg, *subj, *group, *from, *line;
  int pos;

  switch (XmxExtractToken((int)client_data)) {
    case 0:  /* POST */
        XtUnmanageChild(win->news_win);

        msg = XmxTextGetString(win->news_text);
        if (!msg)
            return;
	if (!*msg) {
	    XtFree(msg);
	    return;
	}
        from = XmxTextGetString(win->news_text_from);
        subj = XmxTextGetString(win->news_text_subj);
        group = XmxTextGetString(win->news_text_group);

        NNTPpost(from, subj, win->newsfollow_ref, group, msg); 

        XtFree(msg);
        XtFree(from);
        XtFree(group);
        XtFree(subj);

    case 1:  /* DISMISS */
        if (win->newsfollow_ref)
	    free(win->newsfollow_ref);
        if (win->newsfollow_grp)
	    free(win->newsfollow_grp);
        if (win->newsfollow_subj)
	    free(win->newsfollow_subj);
        if (win->newsfollow_from)
	    free(win->newsfollow_from);
        if (win->newsfollow_artid)
	    free(win->newsfollow_artid);

        XtUnmanageChild(win->news_win);
	/* Since we're going to re-use this in different configs
	 * we'll destroy it */
        XtDestroyWidget(win->news_win);
        win->news_win = NULL;
        win->news_fsb_win = NULL;
        /* Do nothing. */
        break;
    case 2:  /* HELP */
        mo_open_another_window(win, mo_assemble_help_url("help-on-news.html"),
			       NULL, NULL);
        break;
    case 3:  /* INSERT FILE */
	if (!win->news_fsb_win) {
	    win->news_fsb_win = XmxMakeFileSBDialog(win->news_win,
		 			    "VMS Mosaic: Include File for News",
		 			    "Name of file to include:",
		 			    include_fsb_cb, 0);
	} else {
	    XmFileSelectionDoSearch(win->news_fsb_win, NULL);
	}
	XmxManageRemanage(win->news_fsb_win);
	break;
    case 4:  /* QUOTE */
	line = malloc(strlen(win->newsfollow_from) + 30);
	sprintf(line, "%s writes:\n\n", win->newsfollow_from);

	XmTextInsert(win->news_text,
		     pos = XmTextGetInsertionPosition(win->news_text), line);
	/* Move insertion position to past this line to avoid 
	 * inserting the lines in reverse order */
	XmTextSetInsertionPosition(win->news_text, pos + strlen(line));
	free(line);

	if (line = NNTPgetquoteline(win->newsfollow_artid)) {
	    do {
		XmTextInsert(win->news_text,
			     pos = XmTextGetInsertionPosition(win->news_text),
			     line);
		/* Move insertion position to past this line to avoid 
		 * inserting the lines in reverse order */
		XmTextSetInsertionPosition(win->news_text, pos + strlen(line));
	    } while (line = NNTPgetquoteline(NULL));
	}
        break;
  }
  return;
}

mo_status mo_post_news_win(mo_window *win)
{
    return mo_post_generic_news_win(win, 0); 
}

mo_status mo_post_follow_win(mo_window *win)
{
    char *s;

    if (strncmp("news:", win->current_node->url, 5))
        return mo_fail;  /* Fix me */
    if (!strncmp("news:*", win->current_node->url, 6))
        return mo_fail;
    if (NNTPgetarthdrs(&(win->current_node->url)[5], 
		       &win->newsfollow_ref, 
		       &win->newsfollow_grp, 
		       &win->newsfollow_subj, 
		       &win->newsfollow_from) != 1)
       return mo_fail;
    
    /* Add an Re: if needed */
    if (my_strncasecmp("Re: ", win->newsfollow_subj, 4)) {
	s = malloc(strlen(win->newsfollow_subj) + 5);    /* this sucks -bjs */
	sprintf(s, "Re: %s", win->newsfollow_subj);
	free(win->newsfollow_subj);
	win->newsfollow_subj = s;
    }

    /* Add this article to ref */
    win->newsfollow_artid = malloc(strlen(win->current_node->url));
    strcpy(win->newsfollow_artid, &(win->current_node->url)[5]);

    if (!win->newsfollow_ref) {
	win->newsfollow_ref = malloc(strlen(win->current_node->url));
	sprintf(win->newsfollow_ref, "<%s>", &(win->current_node->url)[5]);
    } else {
	s = malloc(strlen(win->newsfollow_ref) +
		   strlen(win->current_node->url));
	sprintf(s, "%s <%s>", win->newsfollow_ref,
	        &(win->current_node->url)[5]);
	free(win->newsfollow_ref);
	win->newsfollow_ref = s;
    }
    return mo_post_generic_news_win(win, 1); 
}

mo_status mo_post_generic_news_win(mo_window *win, int follow)
{
  if (!win->news_win) {
      char namestr[1024], tmp[1024];
      Widget dialog_frame, dialog_sep, buttons_form;
      Widget news_form, yap_label, f_label, s_label, g_label;
      FILE *fp;
      long pos;

      sprintf(namestr, "%s <%s>",
              get_pref_string(eDEFAULT_AUTHOR_NAME),
              get_pref_string(eDEFAULT_AUTHOR_EMAIL));

      /* Create it for the first time. */
      XmxSetUniqid(win->id);

      Xmx_n = 0;
      win->news_win = XmxMakeFormDialog(win->base, "VMS Mosaic: News");
      dialog_frame = XmxMakeFrame(win->news_win, XmxShadowOut);

      /* Constraints for base. */
      XmxSetConstraints
        (dialog_frame, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM,
	 XmATTACH_FORM, NULL, NULL, NULL, NULL);

      /* Main form. */
      news_form = XmxMakeForm(dialog_frame);

      XmxSetArg(XmNalignment, XmALIGNMENT_END);
      f_label = XmxMakeLabel(news_form, "From:");

      XmxSetArg(XmNalignment, XmALIGNMENT_END);
      s_label = XmxMakeLabel(news_form, "Subject:");

      XmxSetArg(XmNalignment, XmALIGNMENT_END);
      g_label = XmxMakeLabel(news_form, "Groups:");

      if (follow) {
	  yap_label = XmxMakeLabel(news_form,
				   "Follow-up to UseNet News Article");
      } else {
	  yap_label = XmxMakeLabel(news_form, "Post a UseNet News Article");
      }
      XmxSetArg(XmNcolumns, 65);
      win->news_text_subj = XmxMakeText(news_form);

      XmxSetArg(XmNcolumns, 65);
      win->news_text_group = XmxMakeText(news_form);

      XmxSetArg(XmNcolumns, 65);
      XmxSetArg(XmNeditable, False);
      win->news_text_from = XmxMakeText(news_form);

      XmxSetArg(XmNscrolledWindowMarginWidth, 10);
      XmxSetArg(XmNscrolledWindowMarginHeight, 10);
      XmxSetArg(XmNcursorPositionVisible, True);
      XmxSetArg(XmNeditable, True);
      XmxSetArg(XmNeditMode, XmMULTI_LINE_EDIT);
      XmxSetArg(XmNrows, 30);
      XmxSetArg(XmNcolumns, 80);
#ifndef VMS
      XmxSetArg(XmNwordWrap, True);
      XmxSetArg(XmNscrollHorizontal, False);
#else
      XmxSetArg(XmNwordWrap, False);
      XmxSetArg(XmNscrollHorizontal, True);
#endif /* VMS, GEC */
      win->news_text = XmxMakeScrolledText(news_form);

      dialog_sep = XmxMakeHorizontalSeparator(news_form);

      if (follow) {
	  buttons_form = XmxMakeFormAndFiveButtons(news_form, follow_win_cb,
		       "Post", "Quote", "Include File...", "Dismiss", "Help...",
		       0, 4, 3, 1, 2);
      } else {
	  buttons_form = XmxMakeFormAndFourButtons(news_form, news_win_cb,
				"Post", "Include File...", "Dismiss", "Help...",
				0, 3, 1, 2);
      }

      /* Constraints for news_form. */
      XmxSetOffsets(yap_label, 10, 20, 0, 0);
      XmxSetConstraints
        (yap_label,
	 XmATTACH_FORM, XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM,
         NULL, NULL, NULL, NULL);

      XmxSetOffsets(win->news_text_from, 10, 10, 10, 10);
      XmxSetConstraints
	(win->news_text_from,
	 XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_FORM,
	 yap_label, NULL, f_label, NULL);

      XmxSetOffsets(f_label, 14, 10, 10, 10);
      XmxSetConstraints
	(f_label,
	 XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_NONE, XmATTACH_NONE,
	 yap_label, NULL, NULL, NULL);

      XmxSetOffsets(win->news_text_subj, 10, 10, 10, 10);
      XmxSetConstraints
	(win->news_text_subj,
	 XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_FORM,
	 win->news_text_from, NULL, s_label, NULL);

      XmxSetOffsets(s_label, 14, 10, 10, 10);
      XmxSetConstraints
	(s_label,
	 XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_NONE, XmATTACH_NONE,
	 win->news_text_from, NULL, NULL, NULL);

      XmxSetOffsets(win->news_text_group, 10, 10, 10, 10);
      XmxSetConstraints
	(win->news_text_group,
	 XmATTACH_WIDGET, XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_FORM,
	 win->news_text_subj, NULL, g_label, NULL);

      XmxSetOffsets(g_label, 14, 10, 10, 10);
      XmxSetConstraints
	(g_label,
	 XmATTACH_WIDGET ,XmATTACH_NONE, XmATTACH_NONE, XmATTACH_NONE,
	 win->news_text_subj, NULL, NULL, NULL);

      XmxSetOffsets(XtParent(win->news_text), 10, 0, 3, 3);
      XmxSetConstraints
        (XtParent(win->news_text),
	 XmATTACH_WIDGET, XmATTACH_WIDGET, XmATTACH_FORM, XmATTACH_FORM,
         win->news_text_group, dialog_sep, NULL, NULL);

      XmxSetArg(XmNtopOffset, 10);
      XmxSetConstraints
        (dialog_sep,
	 XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_FORM, XmATTACH_FORM,
         NULL, buttons_form, NULL, NULL);

      XmxSetConstraints
        (buttons_form,
	 XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM,
         NULL, NULL, NULL, NULL);

      XmxTextSetString(win->news_text, "");

      /* Tack signature on the end if it exists - code from Martin Hamilton */
      if (get_pref_string(eSIGNATURE)) {
          XmxTextSetString(win->news_text, "\n\n");
          /* Leave a gap... */
          XmTextSetInsertionPosition(win->news_text, 2);
          if (fp = fopen(get_pref_string(eSIGNATURE), "r")) {
              while(fgets(tmp, sizeof(tmp) - 1, fp)) {
                  XmTextInsert(win->news_text,
                               pos = XmTextGetInsertionPosition(win->news_text),
                               tmp);
                  XmTextSetInsertionPosition(win->news_text, pos + strlen(tmp));
              }
              fclose(fp);
          } else {
              XmxTextSetString(win->news_text, "");
          }
      }
      XmTextSetInsertionPosition(win->news_text, 0);

      if (follow) {
	  XmxTextSetString(win->news_text_group, win->newsfollow_grp);
	  XmxTextSetString(win->news_text_subj, win->newsfollow_subj);
      } else {
	  XmxTextSetString(win->news_text_group, "");
	  XmxTextSetString(win->news_text_subj, "");
      }
      XmxTextSetString(win->news_text_from, namestr);
  }
  XmxManageRemanage(win->news_win);

  return mo_succeed;
}
