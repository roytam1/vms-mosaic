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
#include "mosaic.h"
#include "gui.h"
#include "gui-extras.h"
#include "gui-documents.h"
#include "gui-dialogs.h"
#include "mo-www.h"
#include "../libhtmlw/HTML.h"
#include <Xm/Xm.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include <Xm/Label.h>

#include "../libnut/system.h"
#include "../libnut/str-tools.h"


static XmxCallback(links_win_cb)
{
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
  int *posns, pcount;
  char *text, *fnam, *url;
  
  switch (XmxExtractToken((int)client_data)) {
    case 0:  /* GOTO */
      if (XmListGetSelectedPos(win->links_list, &posns, &pcount)) {
          if (pcount && XmStringGetLtoR(win->links_items[posns[0] - 1],
                                        XmSTRING_DEFAULT_CHARSET, &text)) {
              if (strncmp(text, "===", 3))
                  mo_access_document(win, text);
              XtFree(text);
          }
	  XtFree((char *)posns);
      }
      mo_gui_done_with_icon();

      return;
    case 3:  /* SAVE */
      if (XmListGetSelectedPos(win->links_list, &posns, &pcount)) {
          if (pcount && XmStringGetLtoR(win->links_items[posns[0] - 1],
                                        XmSTRING_DEFAULT_CHARSET, &text)) {
              if (strncmp(text, "===", 3)) {
                  /* SAVE TO FILE */
                  url = mo_url_canonicalize(text, win->current_node->url);
		  mo_gui_clear_icon();
                  if (mo_pull_er_over_virgin(url, fnam = mo_tmpnam(text)))
                      rename_binary_file(fnam);
                  free(url);
                  free(fnam);
              }
              XtFree(text);
          }
          XtFree((char *)posns);
      }
      mo_gui_done_with_icon();

      return;        
    case 1:  /* DISMISS */
      XtUnmanageChild(win->links_win);
      break;
    case 2:  /* HELP */
      mo_open_another_window(win,
			     mo_assemble_help_url("help-on-links-win.html"),
                             NULL, NULL);
      break;
  }
  return;
}

static void links_list_cb(Widget w, XtPointer client, XtPointer call)
{
  mo_window *win = (mo_window *) client;
  char *text;
  XmListCallbackStruct *cs = (XmListCallbackStruct *) call;
  
  if (XmStringGetLtoR(win->links_items[cs->item_position - 1],
                      XmSTRING_DEFAULT_CHARSET, &text)) {
      if (strncmp(text, "===", 3))
          mo_access_document(win, text);
      XtFree(text);
  }
  /* Don't unmanage the list. */

  return;
}

mo_status mo_post_links_window(mo_window *win)
{
  Widget dialog_frame, dialog_sep, buttons_form;
  Widget links_form, list, scroller, label;
    
  if (!win->links_win) {
      /* Create it for the first time. */
      XmxSetUniqid(win->id);

      Xmx_n = 0;
      win->links_win = XmxMakeFormDialog(win->base,
					 "VMS Mosaic: Document Links");
      dialog_frame = XmxMakeFrame(win->links_win, XmxShadowOut);

      /* Constraints for base. */
      XmxSetConstraints 
        (dialog_frame, XmATTACH_FORM, XmATTACH_FORM, 
         XmATTACH_FORM, XmATTACH_FORM, NULL, NULL, NULL, NULL);
      
      /* Main form. */
      links_form = XmxMakeForm(dialog_frame);
      
      dialog_sep = XmxMakeHorizontalSeparator(links_form);
      
      buttons_form = XmxMakeFormAndFourButtons(links_form, links_win_cb, 
				       "Goto URL", "Save", "Dismiss", "Help...",
				       0, 3, 1, 2);
      XmxSetButtonClue("Open highlighted URL", "Save highlighted URL to a file",
  		       "Close this menu", "Open help in new Mosaic window",
		       NULL);
      label = XtVaCreateManagedWidget("Document Links & Images ...",
                                      xmLabelWidgetClass, links_form,
                                      XmNwidth, 500,
                                      XmNleftAttachment, XmATTACH_FORM,
                                      XmNrightAttachment, XmATTACH_FORM,
                                      XmNtopAttachment, XmATTACH_FORM,
                                      XmNtopOffset, 2,
                                      NULL);
      scroller = XtVaCreateWidget("scroller", xmScrolledWindowWidgetClass,
                                  links_form,
                                  XmNheight, 200,
                                      /* Form attachments */
                                  XmNleftAttachment, XmATTACH_FORM,
                                  XmNrightAttachment, XmATTACH_FORM,
                                  XmNtopAttachment, XmATTACH_WIDGET,
                                  XmNtopWidget, label,
                                  XmNbottomAttachment, XmATTACH_WIDGET,
                                  XmNbottomWidget, dialog_sep,
                                      /* Offsets */
                                  XmNtopOffset, 10,
                                  XmNbottomOffset, 10,
                                  XmNleftOffset, 8,
                                  XmNrightOffset, 8,
                                  NULL);
      list = XtVaCreateManagedWidget("list", xmListWidgetClass, scroller,
                                     XmNvisibleItemCount, 10,
                                     XmNresizable, False,
                                     XmNscrollBarDisplayPolicy, XmSTATIC,
                                     XmNlistSizePolicy, XmCONSTANT,
                                     NULL);
      XtAddCallback(list, XmNdefaultActionCallback, links_list_cb,
		    (XtPointer) win);
      XmxAddClue(list, "Double click on entry to visit it");

      win->links_list = list;
      win->links_items = NULL;
      win->links_count = 0;

      XtManageChild(scroller);

      XmxSetArg(XmNtopOffset, 10);
      XmxSetConstraints
        (dialog_sep,
	 XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_FORM, XmATTACH_FORM,
         NULL, buttons_form, NULL, NULL);
      XmxSetConstraints
        (buttons_form,
	 XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM,
         NULL, NULL, NULL, NULL);
  }

  XmxManageRemanage(win->links_win);
  mo_update_links_window(win);

  return mo_succeed;
}

mo_status mo_update_links_window(mo_window *win)
{
    char **hrefs, **imgs;
    int i, p, count, hcount, icount;
    XmStringTable xmstr;

    hrefs = HTMLGetHRefs(win->scrolled_win, &hcount);
    imgs = HTMLGetImageSrcs(win->scrolled_win, &icount);

    count = icount + hcount;
    if (!count) {
        XtVaSetValues(win->links_list,
                      XmNitemCount, 0,
                      NULL);
    } else {
        if (hrefs)
	    count++;
        if (imgs)
	    count++;
        xmstr = (XmStringTable) XtMalloc(sizeof(XmString) * count);
        p = 0;

        if (hrefs) {
            xmstr[p++] = XmStringCreateLtoR("=== Links ===",
					    XmSTRING_DEFAULT_CHARSET);
            for (i = 0; i < hcount; i++, p++) {
                xmstr[p] = XmStringCreateLtoR(hrefs[i],
					      XmSTRING_DEFAULT_CHARSET);
                free(hrefs[i]);
            }
            free(hrefs);
        }
        if (imgs) {
            xmstr[p++] = XmStringCreateLtoR("=== Images ===",
					    XmSTRING_DEFAULT_CHARSET);
            for (i = 0; i < icount; i++, p++) {
                xmstr[p] = XmStringCreateLtoR(imgs[i],
					      XmSTRING_DEFAULT_CHARSET);
                free(imgs[i]);
            }
            free(imgs);
        }
        XtVaSetValues(win->links_list,
                      XmNitems, xmstr,
                      XmNitemCount, count,
                      NULL);
    }
    if (win->links_count) {
	for (i = 0; i < win->links_count; i++)
	    XmStringFree(win->links_items[i]);
        XtFree((char *)win->links_items);
    }
    win->links_count = count;
    win->links_items = xmstr;

    return mo_succeed;
}

static struct {
    char *name;
    char *expand;
} abouts[] = {
    {"about",
	"<TITLE>Some magic words...</TITLE>\n<p><ul>\
	 <li><a href=\"about:mosaic\">mosaic</a>\n\
	 <li><a href=\"about:xmosaic\">xmosaic</a>\n\
	 <li><a href=\"about:cgi\">cgi</a>\n\
	 <li><a href=\"about:help\">help</a>\n\
	 <li><a href=\"about:html\">html</a>\n\
	 <li><a href=\"about:motif\">motif</a>\n\
	 <li><a href=\"about:cats\">cats</a>\n\
         <li><a href=\"about:wvnet\">wvnet</a>\n</ul></p>"},
    {"mosaic", "0http://vaxa.wvnet.edu/vmswww/vms_mosaic.html"},
    {"xmosaic",
#if !defined(VMS) || defined(__DECC)  /* VAX C is different (and has a line length limit) */
	"<TITLE>NCSA X Mosaic</TITLE><P>Please read our disclaimer below (can you say, 'Joke?') before proceeding to:"
        "</p><blockquote><a href=\"http://www.ncsa.uiuc.edu/SDG/Software/XMosaic/\">http://www.ncsa.uiuc.edu/SDG/Software/XMosaic/</a>."
        "</blockquote><hr><h1>X-Mosaic Disclaimer (humor)</h1><p>Consult your physician before using this program.  "
        "Batteries not included.  May cause drowsiness.  Must be over 17.  Not available in all states.  "
        "Not responsible for acts of God.  Prices subject to change without notice. Proof of purchase required.  "
        "Read label before using.  Some assembly required.  Not responsible for typographical errors.  Some restrictions apply.  "
        "Subject to local regulation.  Warrantee period limited.  Close cover before striking.  "
        "No resemblance to any person, living or dead, is intended.  Subject to availability.  No COD's.  "
        "Sales tax not included.  Shipping and handling extra.  For external use only. May cause excitability.  "
        "Avoid alcoholic beverages while using this software.  If symptoms persist, consult your physician.  "
        "Keep this and all software out of the reach of children.  Parental guidance suggested.  "
        "The buyer assumes all risks associated with using this product.  In case of irritation, flush eyes with cold water "
        "and consult your physician.  Not insured by the Federal Deposit Insurance Corporation.  Use with adequate ventilation. "
        "Avoid repeated or prolonged contact with skin.  Contents under pressure; Do not puncture or incinerate.  "
        "Store in original containers.  Harmful if swallowed.  Do not fold, bend, staple or mutilate. "
        "PLEASE NOTE: Some quantum physics theories suggest that when the consumer is not directly observing this product, "
        "it may cease to exist or will exist only in a vague and undetermined state.</p>"},
#else
 "<TITLE>NCSA X Mosaic</TITLE><P>Please read our disclaimer below (can you say, 'Joke?') before proceeding to:\
  </p><blockquote><a href=\"http://www.ncsa.uiuc.edu/SDG/Software/XMosaic/\">http://www.ncsa.uiuc.edu/SDG/Software/XMosaic/</a>.\
  </blockquote><hr><h1>X-Mosaic Disclaimer (humor)</h1><p>Consult your physician before using this program.\
  Batteries not included.  May cause drowsiness.  Must be over 17.  Not available in all states.\
  Not responsible for acts of God.  Prices subject to change without notice. Proof of purchase required.\
  Read label before using.  Some assembly required.  Not responsible for typographical errors.  Some restrictions apply.\
  Subject to local regulation.  Warrantee period limited.  Close cover before striking.\
  No resemblance to any person, living or dead, is intended.  Subject to availability.  No COD's.\
  Sales tax not included.  Shipping and handling extra.  For external use only. May cause excitability.\
  Avoid alcoholic beverages while using this software.  If symptoms persist, consult your physician.\
  Keep this and all software out of the reach of children.  Parental guidance suggested.\
  The buyer assumes all risks associated with using this product.  In case of irritation, flush eyes with cold water\
  and consult your physician.  Not insured by the Federal Deposit Insurance Corporation.  Use with adequate ventilation.\
  Avoid repeated or prolonged contact with skin.  Contents under pressure; Do not puncture or incinerate.\
  Store in original containers.  Harmful if swallowed.  Do not fold, bend, staple or mutilate.\
  PLEASE NOTE: Some quantum physics theories suggest that when the consumer is not directly observing this product,\
  it may cease to exist or will exist only in a vague and undetermined state.</p>"},
#endif
    {"blank", "<TITLE>Blank Page</TITLE>"},
    /** Gone
    {"cci", "0http://www.ncsa.uiuc.edu/SDG/Software/XMosaic/CCI/cci-spec.html"},
    **/
    {"cgi", "0http://hoohoo.ncsa.uiuc.edu/cgi/overview.html"},
    {"help", "0http://wvnvms.wvnet.edu/vmswww/mosaic/d2-userguide.html"},
    {"html", "0http://wvnvms.wvnet.edu/vmswww/mosaic/htmlprimerall.html"},
    {"logo", "0http://wvnvms.wvnet.edu/vmswww/mosaic.gif"},
    {"motif", "0http://www.faqs.org/faqs/motif-faq/"},
    {"cats",
     "<TITLE>Cats</TITLE>There is no such thing as a bad cat, only bad people."},
    {"wvnet", "0http://www.wvnet.edu/"},
    {NULL, NULL}
};
    
/* Assorted FUN things */
char *mo_special_urls(char *url)
{
    int i;

    if (!url || my_strncasecmp(url, "about:", 6))
	return NULL;

    if (!my_strcasecmp(url, "about:"))
	return strdup(abouts[0].expand);
    
    for (i = 0; abouts[i].name; i++) {
        if (!my_strncasecmp(&url[6], abouts[i].name, strlen(abouts[i].name))) {
            if (abouts[i].expand[0] == '0') {
                return abouts[i].expand;
            } else {
                return strdup(abouts[i].expand);
	    }
        }
    }
    return NULL;
}

void GUI_System(char *cmd, char *title)
{
    char buf[BUFSIZ];
    int retValue;

    if ((retValue = my_system(cmd, buf, BUFSIZ)) != SYS_SUCCESS) {
        char *msg;

	/* Give them the error code message */
	switch(retValue) {
	    case SYS_NO_COMMAND:
		msg = "There was no command to execute.";
		break;
	    case SYS_FORK_FAIL:
		msg = "The fork call failed.";
		break;
	    case SYS_PROGRAM_FAILED:
		msg = "The program specified could not execute.";
		break;
	    case SYS_NO_RETBUF:
		msg = "There was no return buffer.";
		break;
	    case SYS_FCNTL_FAILED:
		msg = "Fcntl failed to set non-block on the pipe.";
		break;
	    default:
		msg = "Unknown failure.";
	}
	/* Give them the output */
	if (*buf) {
            char final[BUFSIZ * 2];

	    sprintf(final, "%s\n%s", msg, buf);
	    application_error(final, title);
	} else {
	    application_error(msg, title);
	}
    } else if (*buf) {
	/* Give them the output */
	application_error(buf, title);
    }
    return;
}
