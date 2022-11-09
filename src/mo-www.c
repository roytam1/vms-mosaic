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

/* Copyright (C) 1998, 1999, 2000 - The VMS Mosaic Project */

#include "../config.h"

/* Moved libwww above mosaic.h to avoid VAXC compiler errors, PGE */
#include "../libwww2/HTUtils.h"
#include "../libwww2/HTString.h"
#ifdef MULTINET
#if defined(__TIME_T) && !defined(__TYPES_LOADED) && !defined(__TYPES)
#define __TYPES_LOADED
#endif /* Different defs in OpenVMS and MultiNet include files, BSN */
#endif /* MULTINET, BSN */
#include "../libwww2/tcp.h"
#include "../libwww2/HTTCP.h"
#include "../libwww2/HTParse.h"
#include "../libwww2/HTAccess.h"
#include "../libwww2/HTML.h"
#include "../libwww2/HText.h"
#include "../libwww2/HTList.h"
#include "../libwww2/HTInit.h"
#include "../libnut/system.h"
#ifdef VAXC
#include "mosaic.h"
#endif /* VAXC, GEC */
#include "../libhtmlw/HTML.h"

#ifndef VAXC
#include "mosaic.h"
#endif /* VAXC, Move above HTML.H to work around caddr_t problem, GEC */
#include "comment.h"
#include "mo-www.h"
#include "globalhist.h"
#include "gui-dialogs.h"
#include "gui-documents.h"
#include "gui.h"
#include "../libnut/str-tools.h"
#include <ctype.h>
#include <stdio.h>
#include <unixio.h>
#include <Xm/FilesB.h>

#ifdef __GNUC__
#include <stat.h>
#endif

#ifndef VMS
#include <memory.h>	/* For memcpy */
#else
#include <string.h>
#include <unixlib.h>
#if defined(MULTINET) && defined(__DECC)
#define getname decc$getname
#define open    decc$open
#endif
#endif

extern mo_window *current_win;

#ifdef CCI
extern int cci_docommand;
extern int cci_get;
#endif
#define __MAX_HOME_LEN__ 256
#define __SRC__
#include "../libwww2/HTAAUtil.h"
extern int securityType;

extern XtAppContext app_context;
extern mo_window *current_win;

extern int is_uncompressed;

extern Pixmap *IconPix;

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

/* Binary save filename */
char *saveFileName = NULL;

/* Grumble grumble... */
#if defined(__sgi) && !defined(__STDC__)
#define __STDC__
#endif

#define MO_BUFFER_SIZE 8192

/* Bare minimum. */
struct _HText {
  char *expandedAddress;
  char *simpleAddress;

  /* This is what we should parse and display; it is *not* safe to free. */
  char *htmlSrc;
  /* This is what we should free. */
  char *htmlSrcHead;
  int srcalloc;    /* Amount of space allocated */
  int srclen;      /* Amount of space used */
};

/* Mosaic does NOT use either the anchor system or the style sheet
   system of libwww. */

HText *HTMainText = 0;                  /* Equivalent of main window */

/* These are used in libwww */
char *HTAppName = "VMS_Mosaic";
char *HTAppVersion;  /* Now set this in gui.c -- mo_do_gui() */
extern char *HTLibraryVersion;

int force_dump_to_file = 0;             /* Hook to force dumping binary data
                                           straight to file named by... */
char *force_dump_filename = NULL;       /* this filename. */

/* From gui-documents.c */
extern int interrupted;

/* From HTAccess.c. */
extern char *use_this_url_instead;

/* From HTTP.c */
extern int do_post;
extern char *post_content_type;
extern char *post_data;
extern int do_put;
extern int put_file_size;
extern FILE *put_fp;
extern char *encrypt_cipher;

/* From HTMIME.c */
extern char *HTTP_last_modified;

extern char *HTTP_expires;

/* Agent Spoofing Public Variables */
#define MAX_AGENTS 51
int numAgents;
char **agent;
int selectedAgent = 0;

#ifdef VMS
static char *mail_fnam = NULL;
#endif /* VMS, BSN, GEC */

/****************************************************************************
 * name:    hack_htmlsrc (PRIVATE)
 * purpose: Do really nasty things to a stream of HTML that just got
 *          pulled over from a server.
 * inputs:  
 *   - none (global HTMainText is assumed to contain current
 *           HText object)
 * returns: 
 *   - HTMainText->htmlSrc (char *)
 * remarks: 
 *   This is ugly but it gets the job done.
 *   The job is this:
 *     - Filter out repeated instances of <PLAINTEXT>.
 *     - Discover if <PLAINTEXT> has been improperly applied
 *       to the head of an HTML document (we discover HTML documents
 *       by checking to see if a <TITLE> element is on the next line).
 *     - Same as above but for <HEAD> and <HTML>.
 *     - Filter out leading <! ... >
 *   We advance the character pointer HTMainText->htmlSrc by the
 *   appropriate remark to make adjustments, and keep the original
 *   head of the allocated block of text in HTMainText->htmlSrcHead.
 ****************************************************************************/
static char *hack_htmlsrc (void)
{
  char *loc;

  if (!HTMainText)
      return NULL;

  if (!HTMainText->htmlSrc) {
      HTMainText->htmlSrcHead = NULL;
      return NULL;
  }

  /* Keep pointer to real head of htmlSrc memory block. */
  HTMainText->htmlSrcHead = HTMainText->htmlSrc;
  
  if (HTMainText->srclen > 30) {
      /* Remove any nulls */
      while (strlen(HTMainText->htmlSrc) < (HTMainText->srclen - 1)) {
	      loc = HTMainText->htmlSrc + strlen(HTMainText->htmlSrc);
	      *loc = '\n';
      }
      if (!my_strncasecmp(HTMainText->htmlSrc, "<PLAINTEXT>", 11)) {
          if (!my_strncasecmp(HTMainText->htmlSrc + 11, "<PLAINTEXT>", 11)) {
              HTMainText->htmlSrc += 11;
          } else if (!my_strncasecmp(HTMainText->htmlSrc + 11, "\n<PLAINTEXT>", 12) ||
		   !my_strncasecmp(HTMainText->htmlSrc + 11, "\n<TITLE>", 8) ||
		   !my_strncasecmp(HTMainText->htmlSrc + 11, "\n<HEAD>", 7) ||
                   !my_strncasecmp(HTMainText->htmlSrc + 11, "\n<HTML>", 7) ||
                   !my_strncasecmp(HTMainText->htmlSrc + 11, "\n\n<HTML>", 8) ||
                   !my_strncasecmp(HTMainText->htmlSrc + 11, "\n<BASE",  6) ||
                   !my_strncasecmp(HTMainText->htmlSrc + 11, "\n<!--",  5) ||
	  	   !strncmp(HTMainText->htmlSrc + 11, "\n<!DOCTYPE HTML", 15)) {
              HTMainText->htmlSrc += 12;
          }
      }
      if (!strncmp(HTMainText->htmlSrc, 
                   "<TITLE>Document</TITLE>\n<PLAINTEXT>", 35)) {
          if (!my_strncasecmp(HTMainText->htmlSrc + 35, "\n<TITLE>", 8)) {
              HTMainText->htmlSrc += 36;
          }
      }
  }

  return HTMainText->htmlSrc;
}


/****************************************************************************
 * name:    doit (PRIVATE)
 * purpose: Given a URL, go fetch information.
 * inputs:  
 *   - char       *url: The URL to fetch.
 *   - char **texthead: Return pointer for the head of the allocated
 *                      text block (which may be different than the
 *                      return text intended for display).
 * returns: 
 *   The text intended for display (char *).
 ****************************************************************************/
static char *doit (char *url, char **texthead)
{
  char *msg;
  int rv;
  mo_window *win = current_win;

  /* Hmmmmmmm... */
  if (HTMainText) {
      free (HTMainText);
      HTMainText = NULL;
  }

  XmxApplyPixmapToLabelWidget(win->logo, IconPix[0]);

  is_uncompressed = 0;

  rv = HTLoadAbsolute(url);

  if (rv == 1) {
      char *txt = hack_htmlsrc();
      if (HTMainText)
        *texthead = HTMainText->htmlSrcHead;
      else
        *texthead = NULL;
      return txt;
  } else if (rv == -1) {
      interrupted = 1;
      *texthead = NULL;
      return NULL;
  }

   /*
   ** Just because we errored out, doesn't mean there isn't markup to 
   ** look at.  For example, an FTP site that doesn't let a user in because
   ** the maximum number of users has been reached often has a message
   ** telling about other mirror sites.  The failed FTP connection returns
   ** a message that is taken care of below.  
   */
   if (HTMainText) {
	char *txt = hack_htmlsrc();

	*texthead = HTMainText->htmlSrcHead;

#ifdef CCI
	if (cci_get) {
		if (txt)
			return txt;
		else
			/* Take care of failed local access */
			txt = strdup("<H1>ERROR</H1>"); 
	}
#endif
	return txt;
		
   }

  /* Return proper error message if we experienced redirection. */
  if (use_this_url_instead)
    url = use_this_url_instead;
  msg = (char *)malloc((strlen(url) + 200) * sizeof(char));
  sprintf(msg,
	"<H1>ERROR</H1> Requested document (URL %s) could not be accessed.<p>The information server either is not accessible or is refusing to serve the document to you.<p>",
	url);
   
  *texthead = msg;
  securityType = HTAA_UNKNOWN;
  return msg;
}


/****************************************************************************
 * name:    mo_pull_er_over
 * purpose: Given a URL, pull 'er over.
 * inputs:  
 *   - char       *url: The URL to pull over.
 *   - char **texthead: Return pointer to head of allocated block.
 * returns: 
 *   Text to display (char *).
 * remarks: 
 *   
 ****************************************************************************/
char *mo_pull_er_over (char *url, char **texthead)
{
  char *rv;
  extern int binary_transfer;
 
  /* Always reset the icon interrupt */
  mo_gui_clear_icon();

  if (binary_transfer) {
      force_dump_to_file = 1;
      force_dump_filename = mo_tmpnam(url);
  }

  if (saveFileName) {
      free(saveFileName);
  }
  saveFileName = strdup(url);

  if (HTTP_last_modified) {
      free(HTTP_last_modified);
      HTTP_last_modified = 0;
  }

  if (encrypt_cipher) {
      free(encrypt_cipher);
      encrypt_cipher = NULL;
  }

  rv = doit(url, texthead);

  if (binary_transfer) {
      force_dump_to_file = 0;
      force_dump_filename = NULL;
  }

  return rv;
}


char *mo_post_pull_er_over (char *url, char *content_type, char *data, 
                            char **texthead)
{
  char *rv;
  extern int binary_transfer;

  /* Always reset the icon interrupt */
  mo_gui_clear_icon();

  do_post = 1;
  post_content_type = content_type;
  post_data = data;

  if (binary_transfer) {
      force_dump_to_file = 1;
      force_dump_filename = mo_tmpnam(url);
  }
  if (HTTP_last_modified) {
      free(HTTP_last_modified);
      HTTP_last_modified = 0;
  }
  rv = doit(url, texthead);

  if (binary_transfer) {
      force_dump_to_file = 0;
      force_dump_filename = NULL;
  }

  do_post = 0;

  return rv;
}  


/****************************************************************************
 * name:    mo_pull_er_over_virgin
 * purpose: Given a URL, pull 'er over in such a way that no format
 *          handling takes place and the data gets dumped in the filename
 *          of the calling routine's choice.
 * inputs:  
 *   - char  *url: The URL to pull over.
 *   - char *fnam: Filename in which to dump the received data.
 * returns: 
 *   mo_succeed on success; mo_fail otherwise.
 * remarks: 
 *   This routine is called when we know there's data out there we
 *   want to get and we know we just want it dumped in a file, no
 *   questions asked, by the WWW library.  Appropriate global flags
 *   are set to make this happen.
 *   This must be made cleaner.
 ****************************************************************************/
mo_status mo_pull_er_over_virgin (char *url, char *fnam)
{
  int rv;

  if (mo_gui_check_icon(0)) {
      force_dump_to_file = 0;
      interrupted = 1;
      return mo_fail;
  }
  /* Force dump to file. */
  force_dump_to_file = 1;
  force_dump_filename = fnam;
  if (saveFileName) {
      free(saveFileName);
  }
  saveFileName = strdup(url);

  is_uncompressed = 0;

  rv = HTLoadAbsolute(url);

  if (rv == 1) {
      force_dump_to_file = 0;
      return mo_succeed;
  } else if (rv == -1) {
      force_dump_to_file = 0;
      interrupted = 1;
      return mo_fail;
  } else {
      force_dump_to_file = 0;
      return mo_fail;
  }
}


mo_status mo_re_init_formats (void)
{
  HTReInit();
  return mo_succeed;
}

/* ------------------------------------------------------------------------ */

HText *HText_new (void)
{
  HText *htObj = (HText *)malloc(sizeof(HText));

  htObj->expandedAddress = NULL;
  htObj->simpleAddress = NULL;
  htObj->htmlSrc = NULL;
  htObj->htmlSrcHead = NULL;
  htObj->srcalloc = 0;
  htObj->srclen = 0;

  /* Free the struct but not the text, as it will be handled
     by Mosaic proper -- apparently. */
  if (HTMainText)
    free(HTMainText);

  HTMainText = htObj;

  return htObj;
}

void HText_free (HText *self)
{
  if (self) {
      if (self->htmlSrcHead)
          free(self->htmlSrcHead);
      free(self);
  }
  return;
}

void HText_beginAppend (HText *text)
{
  HTMainText = text;
  return;
}

void HText_endAppend (HText *text)
{
  if (text)
      HText_appendCharacter(text, '\0');

  HTMainText = text;
  return;
}

void HText_doAbort (HText *self)
{
  /* Clean up -- we want to free htmlSrc here because htmlSrcHead
     doesn't get assigned until hack_htmlsrc, and by the time we
     reach that, this should never be called. */
  if (self) {
      if (self->htmlSrc)
          free(self->htmlSrc);
      self->htmlSrc = NULL;
      self->htmlSrcHead = NULL;
      self->srcalloc = 0;
      self->srclen = 0;
  }
  return;
}

void HText_clearOutForNewContents (HText *self)
{
  if (self) {
      if (self->htmlSrc)
          free (self->htmlSrc);
      self->htmlSrc = NULL;
      self->htmlSrcHead = NULL;
      self->srcalloc = 0;
      self->srclen = 0;
  }
  return;
}

static void new_chunk (HText *text)
{
  if (text->srcalloc == 0) {
      text->htmlSrc = (char *)malloc(MO_BUFFER_SIZE);
      text->htmlSrc[0] = '\0';
  } else {
      text->htmlSrc = (char *)realloc(text->htmlSrc,
		text->srcalloc + MO_BUFFER_SIZE);
  }

  text->srcalloc += MO_BUFFER_SIZE;

  return;
}

#if (defined(__alpha) && !defined(VMS)) || defined(_IBMR2) || (defined(VAXC) && defined(__DECC))
void HText_appendCharacter (text, ch)
HText *text;
char ch;
#else
void HText_appendCharacter (HText *text, char ch)
#endif
{
  if (!text)
    return;

  if (text->srcalloc < text->srclen + 1)
    new_chunk(text);
  
  text->htmlSrc[text->srclen++] = ch;

  return;
}

void HText_appendText (HText *text, char *str)
{
  int len;

  if (!str || !text)
    return;

  len = strlen(str);

  while (text->srcalloc < text->srclen + len + 1)
    new_chunk(text);

  memcpy((text->htmlSrc + text->srclen), str, len);

  text->srclen += len;
  text->htmlSrc[text->srclen] = '\0';

  return;
}

void HText_appendBlock (HText *text, char *data, int len)
{
  if (!data || !text)
    return;
  
  while (text->srcalloc < text->srclen + len + 1)
    new_chunk(text);

  memcpy((text->htmlSrc + text->srclen), data, len);

  text->srclen += len;
  text->htmlSrc[text->srclen] = '\0';

  return;
}

void HText_appendParagraph (HText *text)
{
  /* Boy, talk about a misnamed function. */
  char *str = " <p> \n";

  HText_appendText(text, str);

  return;
}

void HText_beginAnchor (HText *text, char *anc)
{
  HText_appendText(text, "<A HREF=\"");
  HText_appendText(text, anc);
  HText_appendText(text, "\">");
  return;
}

void HText_endAnchor (HText * text)
{
  HText_appendText(text, "</A>");
  return;
}

void HText_dump (HText *me)
{
  return;
}

char *HText_getText (HText *me)
{
  if (me)
    return me->htmlSrc;
  else
    return NULL;
}

char **HText_getPtrToText (HText *me)
{
  if (me)
    return &(me->htmlSrc);
  else
    return NULL;
}

int HText_getTextLength (HText *me)
{
  if (me)
    return me->srclen;
  else
    return 0;
}


/****************************************************************************
 * name:    fileOrServer
 * purpose: Given a string, checks to see if it can stat it. If so, it is
 *   assumed the user expects to open the file, not a web site. If not, we
 *   assume it is supposed to be a server and prepend the default protocol.
 * inputs:  
 *   - char    *url: URL to canonicalize.
 * returns: 
 *   The canonical representation of the URL.
 * remarks: 
 *   Written by spowers@ncsa.uiuc.edu
 ****************************************************************************/
char *fileOrServer(char *url)
{
#if (stat != decc$stat) || !defined(MULTINET)
	struct stat buf;
#else
#undef stat
	struct stat buf;
#define stat decc$stat
#endif /* VMS MultiNet work around, GEC */
	char *xurl;
#ifdef VMS
	int fd;
	char fname[256];
	char *ptr;
#endif

	/*
	 * At this point we know the URL specified is of the form:
	 *   shire.ncsa.uiuc.edu[:PORT]/path/to/something
	 * or is a VMS file spec
	 */
	if (!stat(url, &buf)) { /* It's a file and we have access */
#ifdef VMS
		/* Get file spec in UNIX syntax */
		fd = open(url, O_RDONLY, 0);
		if (getname(fd, fname, 0)) {
			/* Strip off version, unless explicity given */
			if (!strchr(url, ';') && (ptr = strchr(fname, '.')))
				if (ptr = strchr(++ptr, '.'))
					*ptr = '\0';
			xurl = mo_url_canonicalize_local(fname);
		} else {
			xurl = mo_url_canonicalize_local(url);
		}
		close(fd);
#else
		xurl = mo_url_canonicalize_local(url);
#endif
	} else if (!(get_pref_string(eDEFAULT_PROTOCOL)) || 
		   !*(get_pref_string(eDEFAULT_PROTOCOL))) {
		xurl = (char *)calloc(strlen(url) + 15, sizeof(char));
		sprintf(xurl, "http://%s", url);
	} else {
		xurl = (char *)calloc(strlen(url) +
			strlen(get_pref_string(eDEFAULT_PROTOCOL)) + 10,
			sizeof(char));
		sprintf(xurl, "%s://%s", get_pref_string(eDEFAULT_PROTOCOL),
			url);
	}

	return(xurl);
}


/****************************************************************************
 * name:    mo_url_prepend_protocol
 * purpose: To prepend the proper protocol to the url if it is not present.
 * inputs:  
 *   - char    *url: URL to canonicalize.
 * returns: 
 *   The canonical representation of the URL.
 * remarks: 
 *   Contributed by martin@gizmo.lut.ac.uk, modified by spowers@ncsa.uiuc.edu
 ****************************************************************************/
char *mo_url_prepend_protocol(char *url)
{
	char *xurl;
	static int check1 = -1;
	static int check2 = -1;

	if (!url || !*url) {
		return(NULL);
	}

	if (check1 == -1)     /* Avoid repeating routine call */
		check1 = get_pref_boolean(eEXPAND_URLS);

	if (!check1) {
		if (!strchr(url, ':')) { /* No colon found, treat as file */
			xurl = mo_url_canonicalize_local(url);
		} else { /* It's prolly a real url */
			xurl = strdup(url);
		}
	} else if (my_strncasecmp(url, "mailto:", 7) &&
		   my_strncasecmp(url, "news:", 5) &&
		   my_strncasecmp(url, "about:", 6) &&
		   my_strncasecmp(url, "cookiejar:", 10) &&
		   !strstr(url, "://")) { /* No protocol specified, default */

		if (check2 == -1)     /* Avoid repeating routine call */
			check2 = get_pref_boolean(eEXPAND_URLS_WITH_NAME);

		if (check2) {
			if (!my_strncasecmp(url, "www.", 4)) {
				xurl = (char *)malloc(strlen(url) +
					(8 * sizeof(char)));
				sprintf(xurl, "http://%s", url);
			}
			else if (!my_strncasecmp(url, "ftp.", 4)) {
				xurl = (char *)malloc(strlen(url) +
					(7 * sizeof(char)));
				sprintf(xurl, "ftp://%s", url);
			}
			else if (!my_strncasecmp(url, "gopher.", 7)) {
				xurl = (char *)malloc(strlen(url) + 
					(10 * sizeof(char)));
				sprintf(xurl, "gopher://%s", url);
			}
			else if (!my_strncasecmp(url, "wais.", 5)) {
				xurl = (char *)malloc(strlen(url) +
					(8 * sizeof(char)));
				sprintf(xurl, "wais://%s", url);
			}
			else {
				xurl = fileOrServer(url);
			}
		} else {
			xurl = fileOrServer(url);
		}
	} else { /* Protocol was specified */
		xurl = strdup(url);
	}

	return(xurl);
}


/****************************************************************************
 * name:    mo_url_canonicalize
 * purpose: Turn a URL into its canonical form, based on the previous
 *          URL in this context (if appropriate).  
 *          INTERNAL ANCHORS ARE STRIPPED OFF.
 * inputs:  
 *   - char    *url: URL to canonicalize.
 *   - char *oldurl: The previous URL in this context.
 * returns: 
 *   The canonical representation of the URL.
 * remarks: 
 *   All we do is call HTParse.
 ****************************************************************************/
char *mo_url_canonicalize (char *url, char *oldurl)
{
  return HTParse(url, oldurl,
                 PARSE_ACCESS | PARSE_HOST | PARSE_PATH | PARSE_PUNCTUATION);
}


/****************************************************************************
 * name:    mo_url_canonicalize_keep_anchor
 * purpose: Turn a URL into its canonical form, based on the previous
 *          URL in this context (if appropriate).  
 *          INTERNAL ANCHORS ARE *NOT* STRIPPED OFF.
 * inputs:  
 *   - char    *url: URL to canonicalize.
 *   - char *oldurl: The previous URL in this context.
 * returns: 
 *   The canonical representation of the URL.
 * remarks: 
 *   All we do is call HTParse.
 ****************************************************************************/
char *mo_url_canonicalize_keep_anchor (char *url, char *oldurl)
{
  char *rv;

  /* We KEEP anchor information already present in url,
   * but NOT in oldurl. */
  oldurl = HTParse(oldurl, "", PARSE_ACCESS | PARSE_HOST | PARSE_PATH |
                   PARSE_PUNCTUATION);
  rv = HTParse(url, oldurl,
               PARSE_ACCESS | PARSE_HOST | PARSE_PATH |
               PARSE_PUNCTUATION | PARSE_ANCHOR);
  /* We made a new copy of oldurl, so free it. */
  free(oldurl);
  return rv;
}


/****************************************************************************
 * name:    mo_url_extract_anchor
 * purpose: Given a URL (presumably in canonical form), extract
 *          the internal anchor, if any.
 * inputs:  
 *   - char *url: URL to use.
 * returns: 
 *   Internal anchor, if one exists in the URL; else NULL.
 * remarks: 
 *   
 ****************************************************************************/
char *mo_url_extract_anchor (char *url)
{
  return HTParse(url, "", PARSE_ANCHOR);
}


/****************************************************************************
 * name:    mo_url_extract_access
 * purpose: Given a URL (presumably in canonical form), extract
 *          the access method, if any.
 * inputs:  
 *   - char *url: URL to use.
 * returns: 
 *   Access method, if one exists in the URL; else NULL.
 * remarks: 
 *   
 ****************************************************************************/
char *mo_url_extract_access (char *url, char *oldurl)
{
  return HTParse(url, oldurl, PARSE_ACCESS);
}


char *mo_url_canonicalize_local (char *url)
{
  /* Convex OS apparently sucks. */
#ifdef CONVEX
  char blah[129];
  char *cwd = getcwd(blah, 128);
#else
#ifndef VMS
  char *cwd = getcwd(NULL, 128);
#else
  /* Force it to return UNIX syntax */
#ifndef __GNUC__
  char *cwd = getcwd(NULL, 128, 0);
#else
  char *cwd = (char *)getcwd(NULL, 128, 0);
#endif /* GNU C is strange, GEC */
#endif
#endif
  char *tmp;

  if (!url)
    return NULL;

  tmp = (char *)malloc((strlen(url) + strlen(cwd) + 32));
  if (url[0] == '/') {
    sprintf(tmp, "file://localhost%s", url);
  } else {
#ifndef VMS
      sprintf(tmp, "file://localhost%s/%s", cwd, url);
#else
      if (strchr(url, ':')) {
        /* Have a device spec */
        sprintf(tmp, "file://localhost/%s\0", url);
      } else {
        sprintf(tmp, "file://localhost%s/%s\0", cwd, url);
      }
#endif
  }

  /* Sigh... */
#ifndef CONVEX
  free(cwd);
#endif
  
  return tmp;
}


/****************************************************************************
 * name:    mo_tmpnam
 * purpose: Make a temporary, unique filename.
 * inputs:  
 *   none
 * returns: 
 *   The new temporary filename.
 * remarks: 
 *   We make up an unique filename and use the preference
 *   TMP_DIRECTORY, if it has a value, for the directory.
 ****************************************************************************/
#ifndef L_tmpnam
#define L_tmpnam 32
#endif
char *mo_tmpnam (char *url)
{
#ifdef CCI
  extern void MoCCIAddFileURLToList(char *, char *);
#endif
  char *tmp = (char *)malloc(sizeof(char) * (L_tmpnam + 32));
  static char *tmp_dir;
  static char *unique;
  static int init = 0;
  static int count = 0;

  if (!tmp) {
      fprintf(stderr, "Unable to get storage for tmp name\n");
      return NULL;
  }

  /* Get unique string for this process */
  if (!init) {
      unique = strdup(tmpnam(NULL));
      unique += 3;
      tmp_dir = get_pref_string(eTMP_DIRECTORY);
      init = 1;
  }

  count++;

  sprintf(tmp, "MOSAIC-TMP%d_%s", count, unique);

  if (!tmp_dir) {
      /* Fast path. */
#ifdef CCI
      if (url) MoCCIAddFileURLToList(tmp, url);
#endif
      return tmp;
  } else {
      /* OK, reconstruct to go in the directory of our choice. */
      char *oldtmp = tmp;
      int i;

#ifndef VMS
      /* Start at the back and work our way forward. */
      for (i = strlen(oldtmp)-1; i >= 0; i--) {
          if (oldtmp[i] == '/')
            goto found_it;
      }
      
      /* No luck, just punt. */
#ifdef CCI
      if (url) MoCCIAddFileURLToList(tmp, url);
#endif
      return tmp;
#else
      i = 0;
#endif

    found_it:
      tmp = (char *)malloc(sizeof(char) * (strlen(tmp_dir) + 
                                             strlen(&(oldtmp[i])) + 8));
      if (!tmp) {
  	  fprintf(stderr, "Unable to get second storage for tmp name\n");
	  return oldtmp;
      }
#ifndef VMS
      if (tmp_dir[strlen(tmp_dir)-1] == '/') {
          /* Trailing slash in tmp_directory spec. */
          sprintf(tmp, "%s%s", tmp_dir, &(oldtmp[i])+1);
      } else {
          /* No trailing slash. */
          sprintf(tmp, "%s%s", tmp_dir, &(oldtmp[i]));
      }
#else
      if ((tmp_dir[strlen(tmp_dir)-1] == ']') ||
          (tmp_dir[strlen(tmp_dir)-1] == ':')) {
            sprintf(tmp, "%s%s", tmp_dir, oldtmp);
      } else {
            sprintf(tmp, "%s:%s", tmp_dir, oldtmp);
      }
#endif

#ifdef CCI
      MoCCIAddFileURLToList(tmp, url);
#endif
      free(oldtmp);
      return tmp;
    }
}


/* ------------------------------ dumb stuff ------------------------------ */

/* Grumble grumble... */
#if defined(ultrix) || (defined(VMS) && (!defined(__GNUC__) || defined(vax)) && (!defined(__VMS_VER) || (__VMS_VER < 70000000))) || defined(NeXT) || defined(M4310)
char *strdup (char *str)
{
  char *dup;

  dup = (char *)malloc(strlen(str) + 1);
  dup = strcpy(dup, str);

  return dup;
}
#endif


/* Error from the library */
void application_error(char *str, char *title) {

  XmxMakeErrorDialogWait(current_win->base, app_context, str, title, "OK");

  return;
}


/* Feedback from the library. */
void application_user_feedback (char *str)
{
  extern Widget toplevel;
  XmxMakeInfoDialog(toplevel, str, "VMS Mosaic: Application Feedback");
  XmxManageRemanage(Xmx_w);
}

void application_user_info_wait (char *str)
{

  XmxMakeInfoDialogWait(current_win->base, app_context, str,
			"VMS Mosaic: Application Feedback", "OK");

}

char *prompt_for_string (char *questionstr)
{

  return XmxModalPromptForString(current_win->base, app_context,
                                 questionstr, "OK", "Cancel");
}

char *prompt_for_password (char *questionstr)
{

  return XmxModalPromptForPassword(current_win->base, app_context,
                                   questionstr, "OK", "Cancel");
}

int prompt_for_yes_or_no (char *questionstr)
{

  return XmxModalYesOrNo(current_win->base, app_context,
                         questionstr, "Yes", "No");
}

char *mo_get_html_return (char **texthead)
{
  char *txt = hack_htmlsrc();

  *texthead = HTMainText->htmlSrcHead;
  return txt;
}


/* Simply loop through a string and convert all newlines to spaces. */
/* We now remove leading whitespace as well */
char *mo_convert_newlines_to_spaces (char *str)
{
  int i;
  char *tptr;

  if (!str)
    return NULL;

  for (i = 0; i < strlen(str); i++)
    if (str[i] == '\n')
      str[i] = ' ';

  tptr = str;
  while (*tptr && (isspace((int)(*tptr))))
    tptr++;

  if (tptr != str) {
    memcpy(str, tptr, (strlen(tptr) + 1));
  }

  return str;
}

/* ---------------------------- escaping code ----------------------------- */

static unsigned char isAcceptable[96] =
/*   0 1 2 3 4 5 6 7 8 9 A B C D E F */
{    0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,	/* 2x   !"#$%&'()*+,-./	 */
     1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,	/* 3x  0123456789:;<=>?	 */
     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	/* 4x  @ABCDEFGHIJKLMNO  */
     1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,	/* 5x  PQRSTUVWXYZ[\]^_	 */
     0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	/* 6x  `abcdefghijklmno	 */
     1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 };	/* 7x  pqrstuvwxyz{\}~	DEL */

#define MO_HEX(i) (i < 10 ? '0'+i : 'A'+ i - 10)

/* The string returned from here, if any, can be free'd by caller. */
char *mo_escape_part (char *part)
{
  char *q;
  char *p;		/* Pointers into keywords */
  char *escaped;

  if (!part)
    return NULL;

  escaped = (char *)malloc(strlen(part) * 3 + 1);
  
  for (q = escaped, p = part; *p != '\0'; p++) {
      /*
       * This makes sure that values 128 and over don't get
       * converted to negative values.
       */
      int c = (int)((unsigned char)(*p));

      if (*p == ' ') {
          *q++ = '+';
      } else if (c >= 32 && c <= 127 && isAcceptable[c-32]) {
          *q++ = *p;
      } else {
          *q++ = '%';
          *q++ = MO_HEX(c / 16);
          *q++ = MO_HEX(c % 16);
      }
  }
  
  *q = 0;
  
  return escaped;
}


static char mo_from_hex (char c)
{
  return ((c >= '0' && c <= '9') ? (c - '0') : 
          ((c >= 'A' && c <= 'F') ? (c - 'A' + 10) : 
           (c - 'a' + 10)));
}

char *mo_unescape_part (char *str)
{
  char *p = str, *q = str;

  while (*p) {
      /* Plus's turn back into spaces. */
      if (*p == '+') {
          *q++ = ' ';
          p++;
      } else if (*p == '%') {
          p++;
          if (*p) 
            *q = mo_from_hex(*p++) * 16;
          if (*p) 
            *q += mo_from_hex(*p++);
          q++;
      } else {
          *q++ = *p++; 
      }
  }
  
  *q++ = 0;
  return str;
}


/* ---------------------------- Agent Spoofing ---------------------------- */

/*
 * Agent Spoofing is simple.  Mosaic's real agent is always a member of the
 * menu.  Any more than that, you can add to the file in your home directory
 * called ".mosaic-spoof-agents".
 */
void readAgents(void)
{
	FILE *fp;
	char fname[BUFSIZ], buf[512];
	char *homedir, *ptr;

	if ((get_home(&homedir) != 0) || !homedir) {
		fprintf(stderr, "Agents: Could not get your home directory.\n");
		return;
	}
#ifndef VMS  /* PGE */
	sprintf(fname, "%s/.mosaic-spoof-agents", homedir);
#else
	sprintf(fname, "%smosaic-spoof-agents", homedir);
#endif
	free(homedir);

	if (!(fp = fopen(fname, "r"))) {
		return;
	}

	while (!feof(fp)) {
		fgets(buf, 511, fp);
		if (feof(fp)) {
			break;
		}
		if (*buf && *buf != '#') {
			buf[strlen(buf)-1] = '\0';
			for (ptr = buf; *ptr && isspace(*ptr); ptr++);
			if (*ptr == '+') { /* This is to be the default */
				if (*(ptr + 1)) {
					agent[numAgents] = strdup(ptr + 1);
					selectedAgent = numAgents;
				} else {
					continue;
				}
			} else if (*ptr) {
				agent[numAgents] = strdup(ptr);
			} else {
				continue;
			}
			numAgents++;
			if (numAgents == MAX_AGENTS) { /* Limit reached */
			    fprintf(stderr,
				"WARNING: Hard limit reached for agent spoof file.\n");
			    break;
			}
		}
	}

	fclose(fp);

	return;
}


void loadAgents(void)
{
	char buf[512];

	agent = (char **)calloc(MAX_AGENTS + 1, sizeof(char *));
	sprintf(buf, "%s/%s  libwww/%s",
		HTAppName ? HTAppName : "unknown",
		HTAppVersion ? HTAppVersion : "0.0",
		HTLibraryVersion);
	agent[0] = strdup(buf);
	agent[1] = strdup("Mozilla/4.0 (VMS_Mosaic)");
	agent[2] = strdup("Lynx/2.7 (VMS_Mosaic)");
	agent[3] = strdup("MSIE 4.01 (VMS_Mosaic)");
	numAgents = 4;

	readAgents();

	return;
}


#ifdef TIMING
/* ----------------------------- Timing stuff ----------------------------- */

#ifndef VMS
#include <sys/types.h>
#include <sys/times.h>
#include <sys/param.h>
#else
#include <time.h>
#endif /* VMS, BSN*/

/* Time globals. */
#ifndef VMS
static struct tms tbuf;
static int gtime;
#else
static time_t tbuf;
static time_t gtime;
#define times time
#endif /* VMS, BSN*/

void StartClock (void) 
{
  gtime = times(&tbuf);
  
  return;
}

void StopClock ()
{
#ifndef VMS
  int donetime;
#else
  time_t donetime;
#endif /* VMS, BSN*/

  donetime = times(&tbuf);

#ifndef DISABLE_TRACE
  if (srcTrace) {
	fprintf(stderr, "Elapsed time %d\n", donetime - gtime);
  }
#endif

  return;
}
#endif


/* Originally in whine.c and then in techsupport.c...now it's here. - SWP */
/* ------------------------------------------------------------------------ */

static FILE *_fp = NULL;

FILE *mo_start_sending_mail_message (char *to, char *subj, 
                                     char *content_type, char *url)
{
#ifdef VMS
  char *cp;
#else
  char *tmp;
  char cmd[2048];
#endif

  if (!to)
    return NULL;

  if (!strcmp(content_type, "url_only")) {
    content_type = "text/plain";
  }

#ifndef VMS
#ifdef OLD
  if (get_pref_string(eMAIL_FILTER_COMMAND))
    {
      sprintf(cmd, "%s | %s -t", get_pref_string(eMAIL_FILTER_COMMAND), 
               get_pref_string(eSENDMAIL_COMMAND));
    }
  else
    {
      sprintf(cmd, "%s -t", get_pref_string(eSENDMAIL_COMMAND));
    }

#else
  /* Try listing address on command line. */
  for (tmp = to; *tmp; tmp++)
    if (*tmp == ',')
      *tmp = ' ';

  if (get_pref_string(eMAIL_FILTER_COMMAND) && content_type &&
      strcmp(content_type, "application/postscript"))
    {
      sprintf(cmd, "%s | %s %s", get_pref_string(eMAIL_FILTER_COMMAND), 
               get_pref_string(eSENDMAIL_COMMAND), to);
    }
  else
    {
      sprintf(cmd, "%s %s", get_pref_string(eSENDMAIL_COMMAND), to);
    }
#endif

  if ((_fp = popen(cmd, "w")) == NULL)
    return NULL;

  fprintf(_fp, "To: %s\n", to);

  fprintf(_fp, "Subject: %s\n", subj);
  fprintf(_fp, "Reply-To: %s <%s>\n", get_pref_string(eDEFAULT_AUTHOR_NAME),
	   get_pref_string(eDEFAULT_AUTHOR_EMAIL));
  fprintf(_fp, "Content-Type: %s\n", content_type);
  fprintf(_fp, "Mime-Version: 1.0\n");
  fprintf(_fp, "X-Mailer: VMS Mosaic %s on %s\n", 
           MO_VERSION_STRING, MO_MACHINE_TYPE);
  if (url)
    fprintf(_fp, "X-URL: %s\n", url);

  fprintf(_fp, "\n"); 
  
  /* Stick in BASE tag as appropriate. */
  if (url && content_type && (strcmp(content_type, "text/x-html") == 0))
    fprintf(_fp, "<base href=\"%s\">\n", url);
#else /* VMS */
  mail_fnam = mo_tmpnam(NULL);
  _fp = fopen(mail_fnam, "w");
  if (!_fp) {
    printf("\nVMS scratch file open error: %s\n", strerror(errno, vaxc$errno));
    goto oops;
  }

  for (cp = subj; *cp; cp++)	/* Convert quotes to spaces, GEC */
    if (*cp == '\"')
      *cp = ' '; 
  fprintf(_fp, "$ Set NoVerify\n");
  fprintf(_fp, "$ On Error Then Goto End\n");
  /* Next for PMDF only */
  if ((strcmp(get_pref_string(eVMS_MAIL_PREFIX), "IN%") == 0) ||
      (strcmp(get_pref_string(eVMS_MAIL_PREFIX), "in%") == 0)) {
    fprintf(_fp, "$ Define PMDF_HEADER \"X-Courtesy-Of: VMS Mosaic %s on %s\"\n",
            MO_VERSION_STRING, MO_MACHINE_TYPE);
    if (url)
      fprintf(_fp, "$ Define PMDF_HEADER_1 \"X-URL: %s\"\n", url);
    fprintf(_fp, "$ Define /User SYS$Output nl:\n");
    fprintf(_fp, "$ Mail$$/Subject=\"%s\" SYS$Input: \"%s\"\"%s\"\"\"\n", subj,
            get_pref_string(eVMS_MAIL_PREFIX), to);
  } else {
    fprintf(_fp, "$ Define /User SYS$Output nl:\n");
    if ((strchr(to, '@') == NULL) ||
        (strcmp(get_pref_string(eVMS_MAIL_PREFIX), " ") == 0)) {
      fprintf(_fp, "$ Mail$$/Subject=\"%s\" SYS$Input: \"%s\"\n", subj, to);
    } else {
      fprintf(_fp, "$ Mail$$/Subject=\"%s\" SYS$Input: \"%s\"\"%s\"\"\"\n",
              subj, get_pref_string(eVMS_MAIL_PREFIX), to);
    }
  }
  fprintf(_fp, "$ Deck/Dollars=\"$ EOD_MOSAIC\"\n");
  return _fp;

  oops:
    free(mail_fnam);
#endif /* VMS, BSN, GEC */

  return _fp;
}

mo_status mo_finish_sending_mail_message (void)
{
#ifndef VMS
  if (_fp)
    pclose(_fp);
#else
  if (_fp) {
    char *cmd = (char *)malloc(strlen(mail_fnam) + 8);
    fprintf(_fp, "\n$ EOD_MOSAIC\n");
    fprintf(_fp, "$End:\n");
    fclose(_fp);
    sprintf(cmd, "@%s.", mail_fnam);
    if ((system(cmd)) != 1)
      fprintf(stderr, "Unable to mail document.\n");
    free(cmd);
    remove(mail_fnam);
    free(mail_fnam);
  }
#endif /* VMS, BSN, GEC */

  _fp = NULL;

  return mo_succeed;
}

/* ------------------------------------------------------------------------ */

mo_status mo_send_mail_message (char *text, char *to, char *subj, 
                                char *content_type, char *url)
{
  FILE *fp;

  fp = mo_start_sending_mail_message(to, subj, content_type, url);
  if (!fp)
    return mo_fail;

#ifndef VMS
  if (!strcmp(content_type, "url_only")) {
      fputs(url, fp);
      fputs("\n\n", fp);
  } else {
      fputs(text, fp);
  }
#else
  if (!strcmp(content_type, "url_only")) {
      fputs(url, fp);
      fputs("\n\n", fp);
  } else {
      if (write(fileno(fp), text, strlen(text)) < 0)
        return mo_fail;
  }
#endif /* VMS, BSN, GEC */
  
  mo_finish_sending_mail_message();

  return mo_succeed;
}


/* ------------------------- upload stuff -------------------------- */
char *mo_put_er_over(char *url, char **texthead)
{
	char *rv;

	do_put = do_post = 1;

	if (saveFileName) {
		free(saveFileName);
	}
	saveFileName = strdup(url);
	if (HTTP_last_modified) {
		free(HTTP_last_modified);
		HTTP_last_modified = 0;
	}

	rv = doit(url, texthead);

	do_put = do_post = 0;

	return(rv);
}  


int upload(mo_window *win, FILE *fp, char *fname)
{
	char *put_url, *xurl;
	int res = mo_fail;
	char *newtext = NULL, *newtexthead = NULL;
	char *last_modified = NULL, *expires = NULL;
	char *ref;

	if (!win) {
		return(0);
	}

	put_url = prompt_for_string(
		"Enter the URL you wish to upload the file as:");
	if (!put_url) {
		return(0);
	}

/*
	if (win->current_node &&
	    win->current_node->url &&
	    *(win->current_node->url)) {
		ref = strdup(win->current_node->url);
	} else {
*/
		ref = NULL;
/*
	}
*/

	xurl = mo_url_prepend_protocol(put_url);
	free(put_url);
	put_url = xurl;

	fseek(fp, 0, SEEK_END);
	put_file_size = ftell(fp);
	rewind(fp);
	put_fp = fp;

	if (win->target_anchor) {
		free(win->target_anchor);
	}
	win->target_anchor = NULL;

	mo_set_current_cached_win(win);

	newtext = mo_put_er_over(put_url, &newtexthead);

	if (newtext) {
		if ((!strncmp(newtext, "<H1>ERROR<H1>", 10)) ||
		    (!strncmp(newtext,
		     "<HEAD><TITLE>404 Not Found</TITLE></HEAD>", 28))) {
			res = mo_fail;
		}
	}

	if (HTTP_last_modified) {
		last_modified = strdup(HTTP_last_modified);
	}
	if (HTTP_expires) {
		expires = strdup(HTTP_expires);
	}

	/* Check for redirect URL */
	if (use_this_url_instead) {
		mo_here_we_are_son(put_url);
		free(put_url);
		put_url = use_this_url_instead;

		/* Go get another target_anchor. */
		if (win->target_anchor) {
			free(win->target_anchor);
		}
		win->target_anchor = NULL;
	}

	if (newtext) {
		res = mo_do_window_text(win,
				      put_url,
				      newtext,
				      newtexthead,
				      1,
				      ref,
				      last_modified,
				      expires);
		HTMLTraverseTabGroups(win->view,
				      XmTRAVERSE_HOME);
	}

	if (win->current_node) {
		mo_gui_check_security_icon(win->current_node->authType);
	}

	if (last_modified) {
		free(last_modified);
		last_modified = NULL;
	}
	if (expires) {
		free(expires);
		expires = NULL;
	}

	free(put_url);

	if (ref) {
		free(ref);
		ref = NULL;
	}

	mo_gui_done_with_icon();

	return(res);
}


mo_status mo_upload_window(mo_window *win, char *fname)
{
	char *efname = (char *)malloc(sizeof(char) * (__MAX_HOME_LEN__ * 2));
	FILE *fp;
	int res;

	if (pathEval(efname, fname) < 0) {
#ifndef DISABLE_TRACE
		if (srcTrace) {
			fprintf(stderr,
			    "Error in evaluating the path. (gui-dialogs.c)\n");
		}
#endif
	}

	if (!(fp = fopen(efname, "r"))) {
		char *buf, *final, tmpbuf[80];
		int final_len;

#ifdef CCI
		/* Don't display dialog if command issued by cci application */
		if (cci_docommand) {
			return mo_fail;
		}
#endif
		buf = my_strerror(errno);
		if (!buf || !*buf || !strcmp(buf, "Error 0")) {
			sprintf(tmpbuf, "Unknown Error" );
			buf = tmpbuf;
		}

		final_len = 30 + ((!efname || !*efname?3:strlen(efname)) + 13) +
			15 + (strlen(buf) + 3);
		final = (char *)calloc(final_len, sizeof(char));

		sprintf(final,
		    "\nUnable to upload document:\n   %s\n\nUpload Error:\n   %s\n",
		    (!efname || !*efname ? " " : efname), buf);

		application_error(final, "Upload Error");

		if (final) {
			free(final);
			final = NULL;
		}

		free(efname);

		return(mo_fail);
	}

	res = upload(win, fp, efname);
	fclose(fp);
	free(efname);

	return(mo_succeed);
}


static XmxCallback (upload_win_cb)
{
    char *fname = (char *)malloc(sizeof(char) * 128);
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

    XtUnmanageChild(win->upload_win);
  
    XmStringGetLtoR(((XmFileSelectionBoxCallbackStruct *)call_data)->value,
		    XmSTRING_DEFAULT_CHARSET,
		    &fname);

    mo_upload_window(win, fname);

    free(fname);
  
    return;
}


mo_status mo_post_upload_window (mo_window *win)
{
	XmxSetUniqid(win->id);

	if (!win->upload_win) {
		Widget frame, workarea;

		win->upload_win = XmxMakeFileSBDialog(win->base,
					    "VMS Mosaic: Upload Document",
					    "Name of document to upload:",
					    upload_win_cb, 0);
		/* This makes a frame as a work area for the dialog box. */
		XmxSetArg(XmNmarginWidth, 5);
		XmxSetArg(XmNmarginHeight, 5);
		frame = XmxMakeFrame(win->upload_win, XmxShadowEtchedIn);
		workarea = XmxMakeForm(frame);

	} else {
		XmFileSelectionDoSearch(win->upload_win, NULL);
	}
  
	XmxManageRemanage(win->upload_win);

	return mo_succeed;
}
