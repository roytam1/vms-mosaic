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

/* Copyright (C) 1998, 1999, 2000, 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"
#include "../libwww2/htaautil.h"
#include "../libwww2/htnews.h"
#include "../libwww2/htmime.h"
#include "../libwww2/http.h"
#include "mosaic.h"
#include "gui.h"
#include "gui-documents.h"
#include "gui-extras.h"
#include "gui-dialogs.h"
#include "gui-popup.h"
#include "img.h"
#include "mo-www.h"
#include "globalhist.h"
#include "annotate.h"
#include "history.h"
#include "../libhtmlw/htmlp.h"
#include "../libnut/str-tools.h"

#ifdef CCI
#include "cci.h"
#include "cciBindings.h"
#include "cciBindings2.h"
/* From cciBindings.c */
extern int cci_get;
int CCIprotocol_handler_found;
#endif

#include <Xm/List.h>

extern char pre_title[80];
extern char *cached_url;
extern char *startup_document, *home_document;
extern char reloading;
extern int binary_transfer;
extern int do_head;
extern int securityType;

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

int loading_inlined_images = 0;
char *url_base_override = NULL;
int keep_url_base_override = 0;
int interrupted = 0;
extern int frames_interrupted;

/* Kludge to pass last modified, expires, refresh and charset from htmime.c */
extern MIMEInfo MIME_http;

#ifdef HAVE_SSL
/* From http.c */
extern char *encrypt_cipher;
extern char *encrypt_issuer;
extern int encrypt_bits;
extern HTSSL_Status encrypt_status;
#endif

/* From HTAccess.c. */
extern char *use_this_url_instead;

/* For selective image loading */
extern char **imagedelay_sites;
extern Boolean currently_delaying_images;


/****************************************************************************
 * name:    check_imagedelay
 * purpose: checks the win->current_node->url against the imagedelay_sites
 * inputs:  
 *   - char *url - the current url
 * returns: 
 *   1 if there is a match in imagedelay_sites
 *   0 if there is not a match
 * 
 ****************************************************************************/
static Boolean check_imagedelay (char *url)
{
    if (imagedelay_sites) {
	int i;

        for (i = 0; imagedelay_sites[i]; i++) {
            if (strstr(url, imagedelay_sites[i]))
                return 1;
        }
    }
    return 0;
}


/****************************************************************************
 * name:    mo_snarf_scrollbar_values
 * purpose: Store current viewing state in the current node, in case
 *          we want to return to the same location later.
 * inputs:  
 *   - mo_window *win: Current window.
 * returns: 
 *   mo_succeed
 *   (mo_fail if no current node exists)
 * remarks: 
 *   Snarfs current docid position in the HTML widget.
 ****************************************************************************/
static mo_status mo_snarf_scrollbar_values (mo_window *win)
{
  /* Make sure we have a node. */
  if (!win->current_node)
      return mo_fail;

  win->current_node->docid = HTMLPositionToId(win->scrolled_win, 0, 3);

  /* Do the cached stuff thing. */
  win->current_node->cached_widgets = HTMLGetWidgetInfo(win->scrolled_win);
  win->current_node->cached_forms = HTMLGetFormInfo(win->scrolled_win);

  if (win->frames) {
      mo_window *wframe = win->frames;
      mo_window *next = NULL;
      mo_frame *nframe = win->current_node->frames;
      int already_had;

      while (nframe && wframe) {
	  /* Have we already put it in this node? */
	  if (nframe->cached_widgets) {
	      already_had = 1;
	  } else {
	      nframe->cached_widgets = HTMLGetWidgetInfo(wframe->scrolled_win);
	      already_had = 0;
	  }
	  if (nframe->cached_widgets && !already_had) {
	      WidgetInfo *wid = (WidgetInfo *) nframe->cached_widgets;
	      FormInfo *form;

	      wid->cache_count++;
	      form = HTMLGetFormInfo(wframe->scrolled_win);
	      if (wid->cache_count == 1) {
	          /* Save original form list; form callbacks need it */
		  wid->cached_forms = form;
		  if (form)
		      form->cached = 1;
	      } else if (form && !form->cached) {
		  /* Free others; they are not needed for callbacks */
		  HTMLFreeFormInfo(form);
	      }
	  }
	  nframe->docid = HTMLPositionToId(wframe->scrolled_win, 0, 0);
	  nframe = nframe->next;
          if (wframe->frames) {
	      next = wframe->next_frame;
	      wframe = wframe->frames;
	  } else {
	      wframe = wframe->next_frame;
	  }
	  if (!wframe && next) {
	      wframe = next;
	      next = NULL;
	  }
      }
  }
  return mo_succeed;
}


/* ---------------------- mo_reset_document_headers ----------------------- */

static mo_status mo_reset_document_headers (mo_window *win)
{
    static int init = 0;
    static int iswindowtitle;

    if (!init) {
        iswindowtitle = get_pref_boolean(eTITLEISWINDOWTITLE);
        init = 1;
    }

    if (win->current_node) {
	if (win->title_text) {
	    /* Title in title slab.  Not the window title. */
	    if (strstr(win->current_node->title, "%20")) {
	        char *tmp = mo_unescape_spaces(win->current_node->title);

	        XmxTextSetString(win->title_text, tmp);
	        free(tmp);
	    } else {
                XmxTextSetString(win->title_text, win->current_node->title);
	    }
	}
	if (strstr(win->current_node->url, "%20")) {
	    char *tmp = mo_unescape_spaces(win->current_node->url);

	    /* Update this early on */
	    XmxTextSetString(win->url_text, tmp);
	    free(tmp);
	} else {
            XmxTextSetString(win->url_text, win->current_node->url);
	}
    }
    if (iswindowtitle) {
	char *buf;

	if (win && win->base && win->current_node &&
	    win->current_node->title && *win->current_node->title) {
	    buf = (char *)malloc(strlen(pre_title) +
				 strlen(win->current_node->title) + 15);
	    if (!buf) {
		perror("Title Buffer");
		return mo_fail;
	    }
	    sprintf(buf, "%s [%s", pre_title, win->current_node->title);

	    /* Title callback may already have put "]" on the end */
	    if (buf[strlen(buf) - 1] != ']')
		strcat(buf, "]");

	    XtVaSetValues(win->base,
			  XmNtitle, buf,
			  NULL);
	    free(buf);
	} else if (win && win->base) {
	    /* Display "No Title" if none */
	    buf = (char *)malloc(strlen(pre_title) + 15);
	    if (!buf) {
		perror("Title Buffer");
		return mo_fail;
	    }
	    sprintf(buf, "%s [%s]", pre_title, "No Title");
	    XtVaSetValues(win->base,
			  XmNtitle, buf,
			  NULL);
	    free(buf);
	}
    }
    return mo_succeed;
}

/* --------------------------- Back and forward  --------------------------- */

static void mo_back_possible (mo_window *win)
{
  win->mo_back = True;

  if (win->has_toolbar) {
      mo_tool_state(&win->tools[BTN_PREV], XmxSensitive, BTN_PREV);
      XmxRSetSensitive(win->menubar, mo_back, XmxSensitive);
  }
  mo_popup_set_something("Back", XmxSensitive, NULL);

  return;
}


/****************************************************************************
 * name:    mo_back_impossible
 * purpose: Can't go back (nothing in the history list).
 ****************************************************************************/
void mo_back_impossible (mo_window *win)
{
  win->mo_back = False;

  if (win->has_toolbar) {
      XmxRSetSensitive(win->menubar, mo_back, XmxNotSensitive);
      mo_tool_state(&win->tools[BTN_PREV], XmxNotSensitive, BTN_PREV);
  }
  mo_popup_set_something("Back", XmxNotSensitive, NULL);

  return;
}

static void mo_forward_possible (mo_window *win)
{
  win->mo_forward = True;

  if (win->has_toolbar) {
      mo_tool_state(&win->tools[BTN_NEXT], XmxSensitive, BTN_NEXT);
      XmxRSetSensitive(win->menubar, mo_forward, XmxSensitive);
  }
  mo_popup_set_something("Forward", XmxSensitive, NULL);

  return;
}


/****************************************************************************
 * name:    mo_forward_impossible
 * purpose: Can't go forward (nothing in the history list).
 ****************************************************************************/
void mo_forward_impossible (mo_window *win)
{
  win->mo_forward = False;

  if (win->has_toolbar) {
      mo_tool_state(&win->tools[BTN_NEXT], XmxNotSensitive, BTN_NEXT);
      XmxRSetSensitive(win->menubar, mo_forward, XmxNotSensitive);
  }
  mo_popup_set_something("Forward", XmxNotSensitive, NULL);

  return;
}

/* ---------------------- mo_annotate_edit_possible ----------------------- */

static void mo_annotate_edit_possible (mo_window *win)
{
  XmxRSetSensitive(win->menubar, mo_annotate_edit, XmxSensitive);
  XmxRSetSensitive(win->menubar, mo_annotate_delete, XmxSensitive);
  return;
}

static void mo_annotate_edit_impossible (mo_window *win)
{
  XmxRSetSensitive(win->menubar, mo_annotate_edit, XmxNotSensitive);
  XmxRSetSensitive(win->menubar, mo_annotate_delete, XmxNotSensitive);
  return;
}


/* ------------------------------------------------------------------------ */

static void mo_set_text (Widget w, char *txt, char *ans, int id, 
                         char *targetanchor, void *cached_widgets,
			 char *charset, Boolean is_frame)
{
  char *rtxt = NULL;
  int save_interrupted = frames_interrupted;
  static int init = 0;
  static int top;

  if (!init) {
      top = get_pref_boolean(eANNOTATIONS_ON_TOP);
      init = 1;
  }

  /* Any data transfer that takes place in here must be inline image loading */
  loading_inlined_images = 1;  /* Use correct transfer progress message */

  interrupted = frames_interrupted;

  /* Create text for http header refresh */
  if (MIME_http.refresh) {
      rtxt = malloc((60 + strlen(MIME_http.refresh)) * sizeof(char));
      strcpy(rtxt, "<meta http-equiv=\"Refresh\" content=\"");
      strcat(rtxt, MIME_http.refresh);
      strcat(rtxt, "\">");
#ifndef DISABLE_TRACE
      if (srcTrace)
	  fprintf(stderr, "Created meta refresh tag = %s\n", rtxt);
#endif
      /* Only do header refresh one time per download of page */
      free(MIME_http.refresh);
      MIME_http.refresh = NULL;
  }

#ifndef DISABLE_TRACE
  if (srcTrace && charset)
      fprintf(stderr, "charset = %s\n", charset);
#endif

  /* ans must be freed by HTML widget */
  /* charset must neither be freed nor kept (it may keep a copy) by widget */
  if (top) {
      HTMLSetText(w, txt, ans ? ans : "", "", id, targetanchor,
		  cached_widgets, rtxt ? rtxt : "", charset);
  } else {
      HTMLSetText(w, txt, "", ans ? ans : "", id, targetanchor,
		  cached_widgets, rtxt ? rtxt : "", charset);
  }
  loading_inlined_images = 0;
  if (is_frame) {
      if (!interrupted) {
	  interrupted = frames_interrupted = save_interrupted;
      } else {
	  frames_interrupted = interrupted;
      }
  } else {
      if (interrupted)
	  /* Stop Refresh URLs */
	  ((HTMLWidget)w)->html.refresh_count++;
      interrupted = 0;
      mo_gui_done_with_icon();
  }

  /* HTML widget doesn't keep it */
  if (rtxt)
      free(rtxt);
}


/****************************************************************************
 * name:    mo_do_window_text
 * purpose: Set a window's text and do lots of other housekeeping
 *          and GUI-maintenance things.
 * inputs:  
 *   - mo_window *win: The current window.
 *   - char      *url: The URL for the text; may not be canonicalized
 *                     and otherwise ready for inclusion in history lists,
 *                     the window's overhead URL display, etc.
 *   - char      *txt: The new text for the window.
 *   - char  *txthead: The start of the malloc'd block of text corresponding
 *                     to txt.
 *   - int register_visit: If TRUE, then this text should be registered
 *                         as a new node in the history list.  If FALSE,
 *                         then we're just moving around in the history list.
 *   - char      *ref: Reference (possible title) for this text.
 * returns: 
 *   mo_succeed
 * remarks: 
 *   This is the mother of all functions in Mosaic.  Probably should be
 *   rethought and broken down.
 ****************************************************************************/
mo_status mo_do_window_text (mo_window *win, char *url, char *txt,
                             char *txthead, int register_visit, char *ref,
                             char *last_modified, char *expires, char *charset)
{
    char *ans;
    char *cipher = NULL;
    char *cipher_issuer = NULL;
    HTSSL_Status estatus = HTSSL_OFF;
    int ebits = 0;
    int did_we_image_delay = 0;
    mo_window *load_win;
    Boolean is_frame = 0;
    Boolean new_node = 0;
    static int init = 0;
    static int focus, track, track_anchors, on_top;
    static XmString xmblankstr;

    if (!init) {
	/* Do these calls only once */
        track = get_pref_boolean(eTRACK_POINTER_MOTION);
	if (track)
            xmblankstr = XmStringCreateLtoR(" ", XmSTRING_DEFAULT_CHARSET);
        on_top = get_pref_boolean(eANNOTATIONS_ON_TOP);
	track_anchors = get_pref_boolean(eTRACK_TARGET_ANCHORS);
        focus = get_pref_boolean(eFOCUS_FOLLOWS_MOUSE);
        init = 1;
    }

    if (win->do_frame) {
	load_win = win->do_frame;
	is_frame = 1;
	if (load_win->new_node)
	    new_node = 1;
    } else {
	load_win = win;
	ResetMultiLoad();
	if (strstr(url, "%20")) {
	    char *tmp = mo_unescape_spaces(url);

	    /* Update this early on */
	    XmxTextSetString(win->url_text, tmp);
	    free(tmp);
	} else {
	    XmxTextSetString(win->url_text, url);
	}
    }
    /* Need to be sure icon interrupt is cleared, otherwise images
     * may not get loaded when revisiting a currently loaded page */
    mo_gui_clear_icon();

    /* Mark as Refresh URL refreshable */
    win->refreshable = True;

    if (!win->delay_image_loads && check_imagedelay(url)) {
        win->delay_image_loads = 1;
        currently_delaying_images = 1;
        did_we_image_delay = 1;
    } else {
       /* Reset the global for imagekill */
        currently_delaying_images = 0;
    }
#ifdef CCI
    /* Send document over cci if needed */
    if (txt)
 	MoCCISendBrowserViewOutput(url, "text/html", txt, strlen(txt));
#endif
    /* Track application mode */
    {
        int newmode = moMODE_PLAIN;

        if (!my_strncasecmp(url, "ftp:", 4)) {
	    newmode = moMODE_FTP;
        } else if (!my_strncasecmp(url, "news:", 5)) {
            int p, n, pt, nt, f;

            news_status(url, &pt, &nt, &p, &n, &f);

            mo_tool_state(&win->tools[BTN_PTHR],
                          pt ? XmxSensitive : XmxNotSensitive, BTN_PTHR);
	    XmxRSetSensitive(win->menubar, mo_news_prevt,
		             pt ? XmxSensitive : XmxNotSensitive);

            mo_tool_state(&win->tools[BTN_NTHR],
                          nt ? XmxSensitive : XmxNotSensitive, BTN_NTHR);
	    XmxRSetSensitive(win->menubar, mo_news_nextt, 
			     nt ? XmxSensitive : XmxNotSensitive);

            mo_tool_state(&win->tools[BTN_PART],
                          p ? XmxSensitive : XmxNotSensitive, BTN_PART);
	    XmxRSetSensitive(win->menubar, mo_news_prev, 
			  p ? XmxSensitive : XmxNotSensitive);

	    mo_tool_state(&win->tools[BTN_NART],
                          n ? XmxSensitive : XmxNotSensitive, BTN_NART);
	    XmxRSetSensitive(win->menubar, mo_news_next, 
			     n ? XmxSensitive : XmxNotSensitive);

            mo_tool_state(&win->tools[BTN_POST], XmxSensitive, BTN_POST);

            mo_tool_state(&win->tools[BTN_FOLLOW],
                          f ? XmxSensitive : XmxNotSensitive, BTN_FOLLOW);
	    XmxRSetSensitive(win->menubar, mo_news_follow, 
			     f ? XmxSensitive : XmxNotSensitive);
	    /* Set the popup too */
	    mo_popup_set_something("Previous Thread", 
				   pt ? XmxSensitive : XmxNotSensitive, NULL);
	    mo_popup_set_something("Next Thread",
				   nt ? XmxSensitive : XmxNotSensitive, NULL);
	    mo_popup_set_something("Previous Article", 
				   p ? XmxSensitive : XmxNotSensitive, NULL);
	    mo_popup_set_something("Next Article", 
				   n ? XmxSensitive : XmxNotSensitive, NULL);
	    mo_popup_set_something("Followup",
				   f ? XmxSensitive : XmxNotSensitive, NULL);
            newmode = moMODE_NEWS;
        }
        if (newmode != win->mode) {
            win->mode = newmode;
            mo_switch_mode(win);
        }
    }
    mo_set_current_cached_win(win);

    if (track)
	/* Blank it out */
        XtVaSetValues(win->tracker_label,
		      XmNlabelString, (XtArgVal)xmblankstr,
		      NULL);

    /* If !register_visit, we're just screwing around with current_node
     * already, so don't bother snarfing scrollbar values. */
    if (register_visit && (!is_frame || new_node))
        mo_snarf_scrollbar_values(win);

    /* Don't track Meta (HEAD) requests */
    if (register_visit < 2)
        mo_here_we_are_son(url);
  
    {
        /* Since mo_fetch_annotation_links uses the communications code,
         * we need to play games with binary_transfer. */
        int tmp = binary_transfer;

        binary_transfer = 0;
        ans = mo_fetch_annotation_links(url, on_top);

        binary_transfer = tmp;
    }

    /* If there is a BASE tag in the document that contains a "real"
     * URL, this will be non-NULL by the time we exit and base_callback
     * will have been called.  If flag is set then, we are displaying
     * an edited document and forcing a BASE url for it */
    /* cached_url HAS to be set here, since Resolve counts on it. */
    if (cached_url)
        free(cached_url);
    if (!keep_url_base_override) {
	if (url_base_override) {
            free(url_base_override);
            url_base_override = NULL;
	}
        cached_url = mo_url_canonicalize(url, "");
    } else {
        cached_url = mo_url_canonicalize(url_base_override, "");
        keep_url_base_override = 0;
    }
    if (load_win->cached_url)
        free(load_win->cached_url);
    load_win->cached_url = strdup(cached_url);

    {
        int id = 0;
	int freeta = 0;
        void *cached_widgets = NULL;
        char *target_anchor = load_win->target_anchor;

        if (!register_visit && win->current_node && !is_frame) {
            id = win->current_node->docid;
            cached_widgets = win->current_node->cached_widgets;
        } else if (is_frame && !register_visit && win->current_node) {
	    mo_frame *nframe = win->current_node->frames;

	    while (nframe) {
	        if (!strcmp(nframe->url, url)) {
		    WidgetInfo *wid = (WidgetInfo *) nframe->cached_widgets;

		    id = nframe->docid;
		    if (wid) {
		        if (wid->cache_invalid) {
			    /* The frame's text has been reloaded */
			    id = 0;
			    if (--wid->cache_count < 1) {
			        if (wid->cached_forms)
				    HTMLFreeFormInfo(wid->cached_forms);
			        HTMLFreeWidgetInfo(nframe->cached_widgets);
			    }
			    ((HTMLWidget)nframe->scrolled_win)->
							html.widget_list = NULL;
			    nframe->cached_widgets = NULL;
		        } else {
			    cached_widgets = nframe->cached_widgets;
		        }
		    }
		    break;
	        }
	        nframe = nframe->next;
	    }
        }
        /* If the window doesn't have a target anchor already,
         * see if there's one in this node's URL. */
        if (!target_anchor && win->current_node && !is_frame) {
            target_anchor = mo_url_extract_anchor(win->current_node->url);
            freeta = 1;
        }
        if (!txt || !txthead)
            /* Just make it look OK...  band-aid city. */
            txthead = txt = strdup("\0");

        /* Track target anchor's visited */
        if (target_anchor && *target_anchor && track_anchors)
            mo_track_url_anchors_visited(url);	

#ifdef HAVE_SSL
        /* Save the encryption info (if any) */ 
        if (register_visit && encrypt_cipher && !is_frame) {
            cipher = encrypt_cipher;
	    encrypt_cipher = NULL;
            cipher_issuer = encrypt_issuer;
	    encrypt_issuer = NULL;
	    ebits = encrypt_bits;
	    estatus = encrypt_status;
	}
#endif
        mo_set_text(load_win->scrolled_win, txt, ans, id, target_anchor,
		    cached_widgets, charset, is_frame);

        /* HREF ListBox Stuff */
        if (win->links_win && !is_frame && XtIsManaged(win->links_win))
            mo_update_links_window(win);
    
        if (load_win->target_anchor)
            free(load_win->target_anchor);
        load_win->target_anchor = NULL;

        if (freeta)
            free(target_anchor);
    }

    /* Every time we view the document, we reset the search_start
     * struct so searches will start at the beginning of the document. */
    ((ElementRef *)win->search_start)->id = 0;
    win->src_search_pos = 0;

    /* CURRENT_NODE ISN'T SET UNTIL HERE (assuming register_visit is 1).
     * Now that WbNtext has been set, we can pull out WbNtitle.
     * First, check and see if we have a URL.  If not, we probably
     * are only jumping around inside a document.
     */
    if (url && *url && !is_frame) {
        if (register_visit) {
            mo_record_visit(win, url, txt, txthead, ref, last_modified,
		            expires, cipher, ebits, cipher_issuer, (int)estatus,
			    charset);
	    if (cipher)
	        free(cipher);
	    if (cipher_issuer)
	        free(cipher_issuer);
        } else {
            /* At the very least we want to pull out the new title,
             * if one exists. */
            if (win->current_node) {
                if (win->current_node->title)
                    free(win->current_node->title);
                win->current_node->title = mo_grok_title(win, url, ref,
							 win->current_node);
            } else {
	        mo_node *node = (mo_node *)calloc(1, sizeof(mo_node));

	        node->url = strdup(url);
	        node->text = strdup(txt);
	        if (ref)
		    node->ref = strdup(ref);
	        node->title = mo_grok_title(win, url, ref, node);
	        node->authType = HTAA_NONE;
	        node->docid = 1;
	        win->current_node = node;
	    }
        }
    } else if (url && *url && is_frame) {
        load_win->frametext = txt;
        load_win->frametexthead = txthead;

	/* Register node again if new node in history requested */
        if (register_visit && new_node && win->current_node) {
	    char *text = strdup(win->current_node->text);

            mo_record_visit(win, win->current_node->url, text, text,
			    win->current_node->ref,
			    win->current_node->last_modified,
		            win->current_node->expires,
			    win->current_node->cipher,
			    win->current_node->cipher_bits,
			    win->current_node->cipher_issuer,
			    win->current_node->cipher_status,
			    win->current_node->charset);
        }
    }
    if (!is_frame || new_node) {
        mo_reset_document_headers(win);

        if (win->history_list && win->current_node) {
            XmListSelectPos(win->history_list, win->current_node->position,
			    False);
            XmListSetBottomPos(win->history_list, win->current_node->position);
        }
        /* Update source text if necessary. */
        if (win->source_text && XtIsManaged(win->source_text) &&
	    win->current_node) {
            XmxTextSetString(win->source_text, win->current_node->text);
            XmxTextSetString(win->source_url_text, win->current_node->url);
            XmxTextSetString(win->source_date_text,
		             win->current_node->last_modified ?
			     win->current_node->last_modified : "Unknown");
        }
        if (win->current_node && win->current_node->previous) {
	    if (!win->mo_back)
        	mo_back_possible(win);
        } else if (win->mo_back) {
            mo_back_impossible(win);
        }  
        if (win->current_node && win->current_node->next) {
	    if (!win->mo_forward)
        	mo_forward_possible(win);
        } else if (win->mo_forward) {
            mo_forward_impossible(win);
        }
        if (win->current_node && 
#ifndef VAXC
            mo_is_editable_annotation(win, win->current_node->text)) {
#else
            (mo_status *)mo_is_editable_annotation(win,
						   win->current_node->text)) {
#endif /* VAXC, BSN */
            mo_annotate_edit_possible(win);
        } else {
            mo_annotate_edit_impossible(win);
        }
        mo_gui_check_win_security_icon(win->current_node->authType, win);
    }
    /* Every time we load a new page set the focus to hotkeys.  We do
     * this because we may have gotten here via forms and since we
     * don't kill any widgets, some unmanaged widget out there could
     * have the focus. */
    if (!focus && win->have_focus)
        /* Make traversal start at top of document should there be forms */
        XtSetKeyboardFocus(win->base, win->view);

    HTMLTraverseTabGroups(load_win->view, XmTRAVERSE_HOME);

    if (did_we_image_delay)
        win->delay_image_loads = did_we_image_delay = 0;

    return mo_succeed;
}


/****************************************************************************
 * name:    mo_set_win_current_node
 * purpose: Given a window and a node, set the window's current node.
 *          This assumes node is already all put together, in the history
 *          list for the window, etc.
 * inputs:  
 *   - mo_window *win: The current window.
 *   - mo_node  *node: The node to use.
 * returns: 
 *   Result of calling mo_do_window_text.
 * remarks: 
 *   This routine is meant to be used to move forward, backward,
 *   and to arbitrary locations in the history list.
 ****************************************************************************/
mo_status mo_set_win_current_node (mo_window *win, mo_node *node)
{
  typedef struct tmp {
      void *wid;
      struct tmp *next;
  } tmp;
  tmp *start = NULL;
  tmp *freeit;
  void *wto_free = NULL;
  void *fto_free = NULL;
  mo_status r;
      
  /* Must be NULL so we always check node URL for one */
  if (win->target_anchor) {
      free(win->target_anchor);
      win->target_anchor = NULL;
  }

  mo_snarf_scrollbar_values(win);

  if (win->current_node && win->reloading) {
      wto_free = win->current_node->cached_widgets;
      fto_free = win->current_node->cached_forms;

      win->current_node->cached_widgets = NULL;
      win->current_node->cached_forms = NULL;

      if (win->current_node->frames) {
	  mo_frame *frame = win->current_node->frames;

	  start = freeit = (tmp *)calloc(1, sizeof(tmp));
	  while (frame) {
	      WidgetInfo *wid = (WidgetInfo *) frame->cached_widgets;

	      if (wid) {
		  if (--wid->cache_count < 1) {
		      freeit->wid = frame->cached_widgets;
		  } else {
		      wid->cache_invalid = 1;
		  }
	      }
	      ((HTMLWidget)frame->scrolled_win)->html.widget_list = NULL;
	      frame->cached_widgets = NULL;
	      frame = frame->next;
	      if (frame) {
		  freeit->next = (tmp *)calloc(1, sizeof(tmp));
		  freeit = freeit->next;
	      }
	  }
      }
  }
  win->current_node = node;

  mo_set_current_cached_win(win);

#ifdef CCI
  /* Send Anchor history to CCI if CCI wants it */
  MoCCISendAnchorToCCI(win->current_node->url, 0);
#endif

  win->new_node = 1;
  r = mo_do_window_text(win, win->current_node->url, 
                        win->current_node->text, 
                        win->current_node->texthead,
                        FALSE, win->current_node->ref,
			win->current_node->last_modified,
			win->current_node->expires,
			win->current_node->charset);
  win->new_node = 0;

  if (win->reloading) {
      if (wto_free)
          HTMLFreeWidgetInfo(wto_free);
      if (fto_free)
          HTMLFreeFormInfo(fto_free);

      while (start) {
	  WidgetInfo *wid = (WidgetInfo *)start->wid;

	  if (wid) {
	      if (wid->cached_forms)
		  HTMLFreeFormInfo(wid->cached_forms);
	      HTMLFreeWidgetInfo(start->wid);
	  }
	  freeit = start;
	  start = start->next;
	  free(freeit);
      }
      win->reloading = 0;
  }
  return r;
}


/****************************************************************************
 * name:    mo_reload_window_text
 * purpose: Reload the current window's text by pulling it over the
 *          network again.
 * inputs:  
 *   - mo_window *win: The current window.
 * returns: 
 *   mo_succeed
 * remarks: 
 *   This frees the current window's texthead.  This calls mo_pull_er_over
 *   directly.
 ****************************************************************************/
mo_status mo_reload_window_text (mo_window *win, int reload_images_also)
{
  static int init = 0;
  static int reload_images, nocache;

  if (!init) {
      reload_images = get_pref_boolean(eRELOAD_RELOADS_IMAGES);
      nocache = get_pref_boolean(eRELOAD_PRAGMA_NO_CACHE);
      init = 1;
  }

  mo_set_current_cached_win(win);

  if (!win->current_node)
      /* Uh oh, this is trouble... */
      return mo_load_window_text(win,
		     startup_document ? startup_document : home_document, NULL);

  /* Free all images in the current document. */
  if (reload_images || reload_images_also) {
      mo_zap_cached_images_here(win);
      if (win->frames) {
	  mo_window *frame = win->frames;
          mo_window *next = NULL;

          while (frame) {
	      mo_zap_cached_images_here(frame);
	      if (frame->frames) {
		  next = frame->next_frame;
		  frame = frame->frames;
	      } else {
		  frame = frame->next_frame;
	      }
	      if (!frame && next) {
		  frame = next;
		  next = NULL;
	      }
	  }
      }
  }

  /* Free the current document's text. */
  /* REALLY we shouldn't do this until we've verified we have new text that's
   * actually good here -- for instance, if we have a document on display,
   * then go to binary transfer mode, then do reload, we should pick up the
   * access override here and keep the old copy up on screen. */
  if (win->current_node->texthead) {
      free(win->current_node->texthead);
      win->current_node->texthead = NULL;
  }

  /* Free any current internal image viewer file */
  if (win->current_node->image_file) {
      remove(win->current_node->image_file);
      free(win->current_node->image_file);
      win->current_node->image_file = NULL;
  }

  /* Set binary_transfer as per current window. */
  binary_transfer = win->binary_transfer;

  interrupted = 0;
  if (nocache)
      reloading = 1;

  /* Stop animations because they will run if have to prompt for a cookie */
  mo_stop_animations(win, 0);

  win->current_node->text = mo_pull_er_over(win->current_node->url, 
                                            &win->current_node->texthead);
  /* Security type may change on a reload */
  win->current_node->authType = securityType;

#ifdef HAVE_SSL
  if (win->current_node->cipher)
      free(win->current_node->cipher);
  win->current_node->cipher = encrypt_cipher;
  encrypt_cipher = NULL;

  if (win->current_node->cipher_issuer)
      free(win->current_node->cipher_issuer);
  win->current_node->cipher_issuer = encrypt_issuer;
  encrypt_issuer = NULL;

  win->current_node->cipher_bits = encrypt_bits;
  win->current_node->cipher_status = encrypt_status;
#endif

  if (MIME_http.last_modified) {
      if (win->current_node->last_modified)
          free(win->current_node->last_modified);
      win->current_node->last_modified = strdup(MIME_http.last_modified);
  }
  if (MIME_http.expires) {
      if (win->current_node->expires)
          free(win->current_node->expires);
      win->current_node->expires = strdup(MIME_http.expires);
  }
  if (MIME_http.charset) {
      if (win->current_node->charset)
          free(win->current_node->charset);
      win->current_node->charset = strdup(MIME_http.charset);
  }

  reloading = 0;

  /* Check for redirect or replacement cookiejar: URL */
  if (use_this_url_instead) {
      free(win->current_node->url);
      win->current_node->url = use_this_url_instead;
  }

  /* Check if frames have been disabled prior to reloading window with frames */
  if (win->frames && !win->frame_support) {
      /* If so, delete the frames */
      mo_delete_frames(win->frames);
      win->frames = NULL;
  }  
  /* Clear out the cached stuff, if any exists. */
  win->reloading = 1;

  mo_set_win_current_node(win, win->current_node);

  win->reloading = 0;

  /* If news: URL, then we need to auto-scroll to the >>> marker if it
   * is here.  We use a hacked version of the searching function here
   * which will need to be updated when we rewrite.
   */
  if (win->current_node && win->current_node->url &&
      !my_strncasecmp(win->current_node->url, "news:", 5))
      mo_search_window(win, ">>>", 0, 1, 1);

  return mo_succeed;
}


/****************************************************************************
 * name:    mo_reload_frame_text
 * purpose: Reload the current frame's text by pulling it over the
 *          network again.
 * inputs:  
 *   - mo_window *win: The frame.
 * returns: 
 *   mo_succeed
 * remarks: 
 *   This frees the frame's texthead and calls mo_pull_er_over directly.
 ****************************************************************************/
mo_status mo_reload_frame_text (mo_window *win, mo_window *parent)
{
  mo_frame *nframe;
  int top_reloading;
  static int init = 0;
  static int reload_images, nocache;
    
  if (!init) {
      reload_images = get_pref_boolean(eRELOAD_RELOADS_IMAGES);
      nocache = get_pref_boolean(eRELOAD_PRAGMA_NO_CACHE);
      init = 1;
  }

  /* Free all images in the current document. */
  if (reload_images)
      mo_zap_cached_images_here(win);

  /* Set binary_transfer as per current window. */
  binary_transfer = parent->binary_transfer;

  interrupted = 0;
  if (nocache)
      reloading = 1;

  /* Stop animations because they will run if have to prompt for a cookie */
  mo_stop_animations(win, 0);

  win->frametext = mo_pull_er_over(win->frameurl, &win->frametexthead);

  /* Reset HTTP no_cache stuff */
  reloading = 0;

  if (MIME_http.last_modified) {
      win->framelast_modified = strdup(MIME_http.last_modified);
  } else {
      win->framelast_modified = NULL;
  }
  if (MIME_http.expires) {
      win->frameexpires = strdup(MIME_http.expires);
  } else {
      win->frameexpires = NULL;
  }
  if (MIME_http.charset) {
      win->framecharset = strdup(MIME_http.charset);
  } else {
      win->framecharset = NULL;
  }

  /* Replace data in cache */
  mo_replace_cached_frame_data(win);

  /* Clear out the cached stuff, if any exists. */
  nframe = parent->current_node->frames;
  while (nframe) {
      if (!strcmp(nframe->url, win->frameurl)) {
	  nframe->docid = 1;
	  if (nframe->cached_widgets) {
	      WidgetInfo *wid = (WidgetInfo *) nframe->cached_widgets;

	      if (--wid->cache_count < 1) {
		  if (wid->cached_forms)
		      HTMLFreeFormInfo(wid->cached_forms);
		  HTMLFreeWidgetInfo(nframe->cached_widgets);
	      } else {
		  wid->cache_invalid = 1;
	      }
	      ((HTMLWidget)nframe->scrolled_win)->html.widget_list = NULL;
	      nframe->cached_widgets = NULL;
	  }
	  break;
      }
      nframe = nframe->next;
  }

  /* Check for redirect or replacement cookiejar: URL */
  if (use_this_url_instead) {
      free(win->frameurl);
      win->frameurl = use_this_url_instead;
  }
  top_reloading = parent->reloading;
  parent->reloading = 1;

  parent->do_frame = win;
  mo_do_window_text(parent, win->frameurl, win->frametext, win->frametext, 0,
		    NULL, win->framelast_modified, win->frameexpires,
		    win->framecharset);
  parent->do_frame = NULL;

  parent->reloading = top_reloading;

  return mo_succeed;
}


/****************************************************************************
 * name:    mo_refresh_window_text
 * purpose: Reload the current window's text without pulling it over the net.
 * inputs:  
 *   - mo_window *win: The current window.
 * returns: 
 *   mo_succeed
 * remarks: 
 ****************************************************************************/
mo_status mo_refresh_window_text (mo_window *win)
{
  mo_set_current_cached_win(win);

  if (!win->current_node)
      return mo_fail;

  /* Clear out the cached stuff, if any exists. */
  win->reloading = 1;

  mo_set_win_current_node(win, win->current_node);
  mo_gui_check_win_security_icon(win->current_node->authType, win);

  return mo_succeed;
}


/****************************************************************************
 * name:    mo_load_window_text
 * purpose: Given a window and a raw URL, load the window.  The window
 *          is assumed to already exist with a document inside, etc.
 * inputs:  
 *   - mo_window *top: The current top level window.
 *   - char      *url: The URL to load.
 *   - char      *ref: The reference ("parent") URL.
 *         NOTE: actually, the ref field is the citation hypertext
 * returns: 
 *   mo_succeed
 * remarks: 
 *   This is getting ugly.
 ****************************************************************************/
mo_status mo_load_window_text (mo_window *top, char *url, char *ref)
{
    char *newtext;
    char *newtexthead = NULL;
    char *last_modified = NULL;
    char *expires = NULL;
    char *charset = NULL;
    char *ori_url = url;
    mo_status return_stat = mo_succeed;
    int free_url = 0;
    int did_head = do_head;
    mo_window *win;

    if (top->do_frame) {
	win = top->do_frame;
    } else {
	win = top;
    }
    win->target_anchor = mo_url_extract_anchor(url);

    if (newtext = mo_special_urls(url)) {
        if (*newtext == '0') {
            url = newtext + 1;
            newtext = NULL;
        } else {
            newtexthead = newtext;
            goto special_urls;
        }   
    }
    
    /* If we're just referencing an anchor inside a document,
     * do the right thing. */
    if (url && *url == '#') {
        /* Now we make a copy of the current text and make sure we ask
         * for a new mo_node and entry in the history list. */
	if (win->is_frame) {
	    newtext = strdup(win->frametext);
	    if (win->framelast_modified)
                last_modified = strdup(win->framelast_modified);
	    if (win->frameexpires)
                expires = strdup(win->frameexpires);
	    if (win->framecharset)
                charset = strdup(win->framecharset);
        } else if (win->current_node) {
            newtext = strdup(win->current_node->text);
	    if (win->current_node->last_modified)
                last_modified = strdup(win->current_node->last_modified);
	    if (win->current_node->expires)
                expires = strdup(win->current_node->expires);
	    if (win->current_node->charset)
                charset = strdup(win->current_node->charset);
        } else {
            newtext = strdup("lose");
        }
        newtexthead = newtext;
	if (!win->is_frame) {
            url = mo_url_canonicalize_keep_anchor(url,
			       win->current_node ? win->current_node->url : "");
	} else {
            url = mo_url_canonicalize_keep_anchor(url, win->frameurl);
	}
        free_url = 1;
#ifdef CCI
        /* Send Anchor history to CCI if CCI wants it */
        MoCCISendAnchorToCCI(url, 1);
#endif
    } else {
        /* Get a full address for this URL. */
        /* Under some circumstances we may not have a current node yet
         * and may wish to just run with it... so check for that. */
	if (win->is_frame) {
	    /* This is probably redundant */
            url = mo_url_canonicalize_keep_anchor(url, win->frameurl);
	    free_url = 1;
        } else if (win->current_node && win->current_node->url) {
            url = mo_url_canonicalize_keep_anchor(url, win->current_node->url);
	    free_url = 1;
        }
        /* Set binary_transfer as per current window. */
        binary_transfer = top->binary_transfer;
        mo_set_current_cached_win(top);

        {
            char *canon = mo_url_canonicalize(url, "");

            interrupted = 0;
#ifdef CCI
            CCIprotocol_handler_found = 0;
 
            /* Send Anchor history to CCI if CCI wants to handle it */
            MoCCISendAnchorToCCI(url, 3);
 
            if (CCIprotocol_handler_found) {
		free(canon);
	        if (free_url)
		    free(url);
                return return_stat;         /* Success */
	    }
            MoCCISendAnchorToCCI(url, 1);
#endif
	    /* Stop animations; they will run if have to prompt for a cookie */
	    if (!top->do_frame) {
	        /* Also stop frames unless loading a frame */
		mo_stop_animations(win, 0);
	    } else {
		/* Otherwise just stop in this window */
		((HTMLWidget)win->scrolled_win)->html.draw_count++;
	    }

	    if (my_strncasecmp(ori_url, "cookiejar:", 10)) {
                newtext = mo_pull_er_over(canon, &newtexthead);
	    } else {
                newtext = mo_pull_er_over(ori_url, &newtexthead);
		if (free_url && url[10])  /* Original is not modifiable */
		    url[10] = '\0';
	    }
            /* 
	     * Added so MCCIRequestGetURL could return failed when url fails.
	     */
            if (newtext &&
                (!strncmp(newtext, "<H1>ERROR<H1>", 10) ||
                 !strncmp(newtext, 
                          "<HEAD><TITLE>404 Not Found</TITLE></HEAD>", 28)))
                return_stat = mo_fail;

	    /* Yes, this is a really big hack (ETG) */
#ifdef CCI
            if (win->target_anchor && *win->target_anchor)
                MoCCIAddAnchorToURL(canon, url);
#endif
            free(canon);
        }
        if (MIME_http.last_modified)
            last_modified = strdup(MIME_http.last_modified);
        if (MIME_http.expires)
            expires = strdup(MIME_http.expires);
        if (MIME_http.charset)
            charset = strdup(MIME_http.charset);

	/* Check for redirect URL */
        if (use_this_url_instead) {
            mo_here_we_are_son(url);
	    if (free_url)
		free(url);
            url = use_this_url_instead;
	    free_url = 1;
            
            /* Go get another target_anchor. */
            if (win->target_anchor)
                free(win->target_anchor);
            win->target_anchor = mo_url_extract_anchor(url);
        }
    }

    /* Now, if it's a telnet session, there should be no need
     * to do anything else.  Also check for override in text itself.
     */
    if (!my_strncasecmp(url, "telnet:", 7) ||
        !my_strncasecmp(url, "tn3270:", 7) ||
        !my_strncasecmp(url, "rlogin:", 7) ||
        (newtext && !strncmp(newtext, "<mosaic-access-override>", 24))) {

        /* We don't need this anymore. */
        free(newtext);

        /* We still want a global history entry but NOT a 
         * window history entry. */
        mo_here_we_are_son(url);
        /* ... and we want to redisplay the current window to
         * get the effect of the history entry today, not tomorrow. */
        mo_redisplay_window(win);
        /* We're not busy anymore... */
        mo_gui_done_with_icon();
    } else if (newtext) {
        /* Not a telnet session and not an override, but text present
         * (the "usual" case): */
#ifdef CCI
        /* First check if we are using cci Get, if so, don't display
         * the error message */
        if (cci_get && (return_stat == mo_fail)) {
#ifndef DISABLE_TRACE
            if (srcTrace)
                fprintf(stderr, "MCCI GET has passed in a wrong url\n");
#endif
        } else
#endif
        {
          special_urls:
            /* Set the window text. */
            mo_do_window_text(top, url, newtext, newtexthead, did_head ? 2 : 1, 
                              ref, last_modified, expires, charset);
        }
    } else {
        /* No text at all. */
        mo_gui_done_with_icon();
    }

#ifdef CCI
    /* Send Anchor history to CCI if CCI wants it */
    MoCCISendAnchorToCCI(url, 2);
    /* First check if we are using CCI Get, if so, don't display
     * the error message. */
    if (cci_get && (return_stat == mo_fail)) {
#ifndef DISABLE_TRACE
	if (srcTrace)
	    fprintf(stderr, "MCCI GET has passed in a wrong url\n");
#endif
    } else if (!win->is_frame && win->current_node) {
        mo_gui_check_win_security_icon(win->current_node->authType, win);
    }
#else
    if (!win->is_frame && win->current_node)
        mo_gui_check_win_security_icon(win->current_node->authType, win);
#endif
    if (last_modified)
	free(last_modified);
    if (expires)
	free(expires);
    if (charset)
	free(charset);
    /*
    if (cci_event)
	MoCCISendEventOutput(LINK_LOADED);
    */

    /* If news: URL, then we need to auto-scroll to the >>> marker if it
     * is here.  We use a hacked version of the searching function here
     * which will need to be updated when we rewrite.
     */
    if (!win->is_frame && win->current_node && win->current_node->url &&
        !my_strncasecmp(win->current_node->url, "news:", 5))
        mo_search_window(win, ">>>", 0, 1, 1);

    if (free_url)
        free(url);

    return return_stat;
}


mo_status mo_post_load_window_text (mo_window *top, char *url,
                                    char *content_type, char *post_data)
{
  char *newtext = NULL;
  char *newtexthead = NULL;
  mo_window *win;
#ifdef CCI
  char *actionID;
#endif
  char *last_modified = NULL;
  char *expires = NULL;
  char *charset = NULL;
  int free_url = 0;

  if (top->do_frame) {
      win = top->do_frame;
      /* Will be done later if not frame */
      ResetMultiLoad();
  } else {
      win = top;
  }
  win->target_anchor = mo_url_extract_anchor(url);

#ifdef CCI
  actionID = strdup(url);       /* Make a copy of url for cci's register id */
#endif

  /* If we're just referencing an anchor inside a document,
   * do the right thing. */
  if (url && *url == '#') {
      /* I believe this section is never executed */
      /* Now we make a copy of the current text and make sure we ask
       * for a new mo_node and entry in the history list. */
      if (win->is_frame) {
	  newtext = strdup(win->frametext);
      } else if (win->current_node) {
          newtext = strdup(win->current_node->text);
      } else {
          newtext = strdup("lose");
      }
      newtexthead = newtext;
      if (!win->is_frame) {
          url = mo_url_canonicalize_keep_anchor(url,
			       win->current_node ? win->current_node->url : "");
      } else {
          url = mo_url_canonicalize_keep_anchor(url, win->frameurl);
      }
      free_url = 1;
  } else {
      /* Get a full address for this URL. */
      /* Under some circumstances we may not have a current node yet
       * and may wish to just run with it... so check for that. */
      if (win->is_frame) {
          url = mo_url_canonicalize_keep_anchor(url, win->frameurl);
	  free_url = 1;
      } else if (win->current_node && win->current_node->url) {
          url = mo_url_canonicalize_keep_anchor(url, win->current_node->url);
	  free_url = 1;
      }

      /* Set binary_transfer as per current window. */
      binary_transfer = top->binary_transfer;
      mo_set_current_cached_win(top);

      {
        char *canon = mo_url_canonicalize(url, "");

        interrupted = 0;

	/* Stop animations; they will run if have to prompt for a cookie */
        if (!top->do_frame) {
	    /* Also stop frames unless loading a frame */
	    mo_stop_animations(win, 0);
	} else {
	    /* Otherwise just stop in this window */
	    ((HTMLWidget)win->scrolled_win)->html.draw_count++;
	}
#ifdef CCI
	if (!MoCCIFormToClient(actionID, NULL, content_type, post_data, 0))
#endif
	    /* Always call if no CCI stuff */
       	    newtext = mo_post_pull_er_over(canon, content_type, post_data,
					   &newtexthead);
        free(canon);
      }
      if (MIME_http.last_modified)
          last_modified = strdup(MIME_http.last_modified);
      if (MIME_http.expires)
          expires = strdup(MIME_http.expires);
      if (MIME_http.charset)
          charset = strdup(MIME_http.charset);

      /* Check for redirect URL */
      if (use_this_url_instead) {
          mo_here_we_are_son(url);
	  if (free_url)
	      free(url);
          url = use_this_url_instead;
	  free_url = 1;

          /* Go get another target_anchor. */
          if (win->target_anchor)
              free(win->target_anchor);
          win->target_anchor = mo_url_extract_anchor(url);
      }
  }

  /* Now, if it's a telnet session, there should be no need
   * to do anything else.  Also check for override in text itself. */
  if (!my_strncasecmp(url, "telnet:", 7) ||
      !my_strncasecmp(url, "tn3270:", 7) ||
      !my_strncasecmp(url, "rlogin:", 7) ||
      (newtext && !strncmp(newtext, "<mosaic-access-override>", 24))) {
      /* We don't need this anymore. */
      free(newtext);

      /* We still want a global history entry but not a 
       * window history entry. */
      mo_here_we_are_son(url);
      /* ... and we want to redisplay the current window to
       * get the effect of the history entry today, not tomorrow. */
      mo_redisplay_window(win);
  } else if (newtext) {
      /* Not telnet session and not override, but text present("usual" case) */
      /* Set the window text. */
      mo_do_window_text(top, url, newtext, newtexthead, 1, NULL,
			last_modified, expires, charset);
      /* Could be an interrupted frame */
      frames_interrupted = 0;
  }

  mo_gui_done_with_icon();

  if (!win->is_frame && win->current_node)
      mo_gui_check_win_security_icon(win->current_node->authType, win);

  if (last_modified)
      free(last_modified);
  if (expires)
      free(expires);
  if (charset)
      free(charset);
  /*
  if (cci_event)
      MoCCISendEventOutput(LINK_LOADED);
  */
  if (free_url)
      free(url);

  return mo_succeed;
}


/****************************************************************************
 * name:    mo_duplicate_window_text
 * purpose: Given an old window and a new window, make a copy of the text
 *          in the old window and install it in the new window.
 * inputs:  
 *   - mo_window *oldw: The old window.
 *   - mo_window *neww: The new window.
 * returns: 
 *   mo_succeed
 * remarks: 
 *   This is how windows are cloned: a new window is created and this
 *   call sets up its contents.
 ****************************************************************************/
mo_status mo_duplicate_window_text (mo_window *oldw, mo_window *neww)
{
  /* We can get away with just cloning text here and forgetting
   * about texthead, obviously, since we're making a new copy. */
  char *newtext;

  if (!oldw->current_node)
      return mo_fail;

  if (oldw->current_node->text) {
      newtext = strdup(oldw->current_node->text);
  } else {
      newtext = strdup(" ");
  }
  mo_do_window_text(neww, oldw->current_node->url, newtext, newtext, TRUE,
		    oldw->current_node->ref,
		    oldw->current_node->last_modified,
		    oldw->current_node->expires,
		    oldw->current_node->charset);
  return mo_succeed;
}


/****************************************************************************
 * name:    mo_access_document
 * purpose: Given a URL, access the document by loading the current 
 *          window's text.
 * inputs:  
 *   - mo_window *win: The current window.
 *   - char      *url: URL to access.
 * returns: 
 *   mo_succeed
 * remarks: 
 *   This should be the standard call for accessing a document.
 ****************************************************************************/
mo_status mo_access_document (mo_window *win, char *url)
{
  mo_set_current_cached_win(win);

  mo_load_window_text(win, url, NULL);

  return mo_succeed;
}
