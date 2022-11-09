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
#include "mosaic.h"
#include "hotlist.h"
#include "hotfile.h"
#include "mo-www.h"
#include "../libhtmlw/HTMLp.h"
#include "../libhtmlw/HTMLputil.h"

/* The new file format provides support for nested hotlists of interesting
   documents within the browser.  It uses a subset of HTML.
   Here is a simplified BNF for this format:

   <hotlist-file> ::= <start-html-tag><hotlist-document><end-html-tag> |
		      <hotlist-document>

   <hotlist-document> ::= <head><body> | <body>
   <head> ::= <start-head-tag><title element><end-head-tag> | <title element>
   <body> ::= <start-body-tag><list><end-body-tag> | <list>

   <list> ::= <structured-list> | <flat-list>

   <structured-list> ::= <title-part><ul-list> | <ul-list>

   <title-part> ::= <title-of-list> | <header-element>
   <header-element> ::= <start-header-tag><title-of-list><end-header-tag>

   <ul-list> ::=  <start-ul-tag><list-of-items><end-ul-tag> |
		  <start-ul-tag><end-ul-tag>
   <list-of-items> ::= <list-item> | <list-item><list-of-items>
   <list-item> ::= <li-tag><item>
   <item> ::= <structured-list> | <url-item>

   <url-item> ::= <start-anchor-tag><title><end-anchor-tag>
   <start-anchor-tag> ::= '<'<anchor-name><space><anchor-attrs>'>'
   <anchor-attrs> ::= <href-attr> | <title-attr><space><href-attr> |
		      <href-attr><space><title-attr>

   <href-attr> ::= <href-keyword>'='<url-val>
   <url-val> ::= <double-quote><url><double-quote> |
		 <single-quote><url><single-quote>

   <title-attr> ::= <title-keyword>'='<title-val>
   <title-val> ::= <double-quote><title><double-quote> |
		   <single-quote><title><single-quote>

   <flat-list> ::= <url-item> | <url-item><flat-list>
*/


static int notSpacesOrNewLine(char *s)
{
  for (; *s; s++) {
      if (!isspace(*s))
          return 1;
  }
  return 0;
}

/*
 * Extract a hotlist from any HTML document
 */
static char *mo_extract_anchors(mo_hotlist *list, MarkInfo *mptr)
{
  mo_hotnode *node;
  char *name = NULL;
  char *last_text = NULL;
  char *url = NULL;
  char *title = NULL;
  char *rbm = NULL;

  for (; mptr; mptr = mptr->next) {
    switch (mptr->type) {
      case M_TITLE:		/* Title tag */
	if (mptr->is_end && last_text) {
	    /* If this is the end tag, take the last text as name */
	    name = strdup(last_text);
	    mo_convert_newlines_to_spaces(name);
	}
	break;
      case M_NONE:		/* Text, not tag */
	if (notSpacesOrNewLine(mptr->text))
	    last_text = mptr->text;
	break;
      case M_ANCHOR:
	if (!mptr->is_end) {		/* Start anchor tag */
	    last_text = NULL;
	    if (url)
		free(url);
	    if (title)
		free(title);
	    if (rbm)
		free(rbm);
	    url = ParseMarkTag(mptr->start, MT_ANCHOR, AT_HREF);
	    title = ParseMarkTag(mptr->start, MT_ANCHOR, "title");
	    rbm = ParseMarkTag(mptr->start, MT_ANCHOR, "RBM");
	} else {			/* End anchor tag */
	    node = (mo_hotnode *)malloc(sizeof(mo_hotnode));
	    node->type = mo_t_url;
	    node->url = url;
	    /* If there is a title attribute in the anchor, take it,
	     * otherwise take the last text. */
	    node->title = title ? title :
	      		  (last_text ? strdup(last_text) : strdup("Unnamed"));
	    if (rbm) {
		node->rbm = 1;
		free(rbm);
		rbm = NULL;
	    } else {
		node->rbm = 0;
	    }
	    mo_convert_newlines_to_spaces(node->title);
	    mo_append_item_to_hotlist(list, (mo_hot_item *)node);
	    url = title = last_text = NULL;
	}
      default:
	break;
    }
  }
  return name;
}

/*
 * Parse a structured hotlist file recursively
 */
static void mo_parse_hotlist_list(mo_hotlist *list, MarkInfo **current)
{
  mo_hotlist *hotlist;
  mo_hotnode *node;
  char *last_text = NULL;
  char *url = NULL;
  char *title = NULL;
  char *rbm = NULL;
  MarkInfo *mptr;
  int done = 0;
  
  for (mptr = *current; mptr && !done; mptr && (mptr = mptr->next)) {
    switch (mptr->type) {
      case M_NONE:		/* Text, not tag */
	if (notSpacesOrNewLine(mptr->text))
	    last_text = mptr->text;
	break;
      case M_ANCHOR:
	if (!mptr->is_end) {		/* Start anchor tag */
	    last_text = NULL;
	    url = ParseMarkTag(mptr->start, MT_ANCHOR, AT_HREF);
	    title = ParseMarkTag(mptr->start, MT_ANCHOR, "title");
	    rbm = ParseMarkTag(mptr->start, MT_ANCHOR, "RBM");
	} else {			/* End anchor tag */
	    node = (mo_hotnode *)malloc(sizeof(mo_hotnode));
	    node->type = mo_t_url;
	    node->url = url;
	    /* If there is a title attribute in the anchor, take it,
	     * otherwise take the last text. */
	    node->title = title ? title :
	    		  (last_text ? strdup(last_text) : strdup("Unnamed"));
	    if (node->title && node->title[strlen(node->title) - 1] == '\n')
	        node->title[strlen(node->title) - 1] = '\0';
	    if (rbm) {
		node->rbm = 1;
		free(rbm);
		rbm = NULL;
	    } else {
		node->rbm = 0;
	    }
	    mo_convert_newlines_to_spaces(node->title);
	    mo_append_item_to_hotlist(list, (mo_hot_item *)node);
	    url = title = last_text = NULL;
	  }
	break;
      case M_UNUM_LIST:
	if (!mptr->is_end) {		/* Start Unum List tag */
	    hotlist = (mo_hotlist *)malloc(sizeof(mo_hotlist));
	    hotlist->type = mo_t_list;
	    hotlist->nodelist = hotlist->nodelist_last = NULL;
	    hotlist->parent = list;
	    hotlist->name = last_text ? strdup(last_text) : strdup("Unnamed");
	    if (hotlist->name &&
		hotlist->name[strlen(hotlist->name) - 1] == '\n')
	        hotlist->name[strlen(hotlist->name) - 1] = '\0';
	    mo_convert_newlines_to_spaces(hotlist->name);
	    rbm = ParseMarkTag(mptr->start, MT_UNUM_LIST, "RBM");
	    if (rbm) {
		hotlist->rbm = 1;
		free(rbm);
		rbm = NULL;
	    } else {
		hotlist->rbm = 0;
	    }
	    mo_append_item_to_hotlist(list, (mo_hot_item *)hotlist);
	    mptr = mptr->next;
	    last_text = NULL;
	    mo_parse_hotlist_list(hotlist, &mptr);
	    /* After this call, mptr is positioned on the end Unum List tag */
	} else {			/* End Unum List tag */
	    *current = mptr;
	    done = 1;
	}
      default:
	break;
    }
  }
  if (!done)
      *current = NULL;
}

/* Read a hotlist from a file.
 * Fill the hotlist list given as parameter
 */
char *mo_read_new_hotlist(mo_hotlist *list, FILE *fp)
{
  char *name, *text;
  int done = 0;
  int normal = 0;
  int has_list = 0;
  int depth = 0;
  long size;
  MarkInfo *hot_mark_up, *mptr;

  setbuf(fp, NULL);
  fseek(fp, 0L, SEEK_END);
  size = ftell(fp);

  if (!(text = malloc(size + 1)))
      return NULL;

  fseek(fp, 0L, SEEK_SET);
  fread(text, (size_t)1, (size_t)size, fp);
  text[size] = '\0';

  /* Parse the HTML document */
  hot_mark_up = HTMLParse(NULL, text, NULL);
  free(text);

  /* Some pre-processing to see if this is in hotlist format or if
   * this is a normal document.
   * The algo is as follow:  if an anchor is outside a list or if there
   * are more than one top level list, then it is not in hotlist format.
   * The 'normal' flag at the end of the pre-processing tells if it
   * is a normal document or a hotlist.
   */
  for (mptr = hot_mark_up; mptr && !done; mptr = mptr->next) {
      switch (mptr->type) {
	case M_ANCHOR:
	  if (!depth) {
	      done = 1;
	      normal = 1;
	  }
	  break;
	case M_UNUM_LIST:
	  if (!mptr->is_end) {	/* Start unum list tag */
	      if (!depth && has_list) {
	          done = 1;
		  normal = 1;
	      } else {
	          depth++;
		  has_list = 1;
	      }
	  } else {		/* End unum list tag */
	      depth--;
	  }
	default:
	  break;
      }
      if (done)
	  break;
  }
  /* Now we know what kind of file we are dealing with */
  if (normal) {
      name = mo_extract_anchors(list, hot_mark_up);
  } else {
      char *last_text = NULL;

      done = 0;
      for (mptr = hot_mark_up; mptr && !done; mptr = mptr->next) {
	  switch (mptr->type) {
	    case M_NONE:		/* Text, not tag */
	      if (notSpacesOrNewLine(mptr->text))
	          last_text = mptr->text;
	      break;
	    case M_UNUM_LIST:		/* Unum List tag */
	      done = 1;
	    default:
	      break;
	  }
      }
      /* After this loop, mptr is positioned just after the
       * start anchor tag */
      if (last_text) {
          name = strdup(last_text);
          mo_convert_newlines_to_spaces(name);
      } else {
	  name = NULL;
      }
      mo_parse_hotlist_list(list, &mptr);
  }

  FreeMarkUpList(hot_mark_up);

  /*
   * Problem with hotlist name growing by 1 space with each write.  So...
   *   we chop off all the spaces on the end here.
   * We do it this way to get rid of all the people out there who already
   *   have hotlist names that are space infested.
   */
  if (name) {
      char *ptr;

      for (ptr = name + strlen(name) - 1; ptr && *ptr == ' '; ptr--)
          *ptr = '\0';
  }
  return name;
}

/*
 * This function replaces '>', '<' and '&' by their entity references
 * and outputs them.
 */
static void fputExpanded(char *s, FILE *fp)
{
  for (; *s; s++) {
      if (*s == '<') {
          fputs("&lt;", fp);
      } else if (*s == '>') {
          fputs("&gt;", fp);
      } else if (*s == '&') {
          fputs("&amp;", fp);
      } else {
          putc(*s, fp);
      }
  }
}

/*
 * Recursive function called to write a hotlist out to a file
 */
static void mo_write_list_r(mo_hotlist *list, FILE *fp)
{
  mo_hot_item *item;

  fputExpanded(list->name, fp);
  if (list->rbm) {
      fputs("\n<UL RBM>\n", fp);
  } else {
      fputs("\n<UL>\n", fp);
  }
  for (item = list->nodelist; item; item = item->any.next) {
      if (item->type == mo_t_url) {  /* URL item */
	  if (!item->hot.url)
	      continue;
	  if (item->hot.rbm) {
	      fputs("<LI> <A RBM HREF=\"", fp);
	  } else {
	      fputs("<LI> <A HREF=\"", fp);
	  }
	  fputExpanded(item->hot.url, fp);
	  fputs("\"> ", fp);
	  if (!item->hot.title) {
	      fputs("No Title\n", fp);
	  } else {
	      fputExpanded(item->hot.title, fp);
	  }
	  fputs("</A>\n", fp);
      } else {			/* List item */
	  fputs("<LI> ", fp);
	  mo_write_list_r(&item->list, fp);
      }
  }
  fputs("</UL>\n", fp);
}

/*
 * Write a hotlist out to a file.
 * Return mo_succeed if everything goes OK, else mo_fail.
 */
mo_status mo_write_hotlist(mo_hotlist *list, FILE *fp)
{
  static Boolean init = 0;
  static char *DAN;

  if (!init) {
      DAN = get_pref_string(eDEFAULT_AUTHOR_NAME);
      init = 1;
  }

  fprintf(fp, "<HTML>\n%s\n", NCSA_HOTLIST_FORMAT_COOKIE_THREE);

  fputs("<TITLE>Hotlist from ", fp);
  if (!DAN) {
      fputs("Unknown", fp);      
  } else {
      fputExpanded(DAN, fp);      
  }
  fputs("</TITLE>\n", fp);
  
  mo_write_list_r(list, fp);

  fputs("</HTML>\n", fp);

  fflush(fp);

  return mo_succeed;
}
