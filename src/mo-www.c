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

/* Moved libwww2 above mosaic.h to avoid VAXC compiler errors, PGE */
#include "../libwww2/htutils.h"
#include "../libwww2/htstring.h"
#ifdef MULTINET
#if defined(__TIME_T) && !defined(__TYPES_LOADED) && !defined(__TYPES)
#define __TYPES_LOADED
#endif /* Different defs in OpenVMS and MultiNet include files, BSN */
#endif /* MULTINET, BSN */
#include "../libwww2/tcp.h"
#include "../libwww2/httcp.h"
#include "../libwww2/htparse.h"
#include "../libwww2/htaccess.h"
#include "../libwww2/html.h"
#include "../libwww2/htext.h"
#include "../libwww2/htinit.h"
#include "../libwww2/htmime.h"
#include "../libnut/system.h"
#ifdef VAXC
#include "mosaic.h"
#endif /* VAXC, GEC */
#include "../libhtmlw/html.h"

#ifndef VAXC
#include "mosaic.h"
#endif /* VAXC, Move above HTML.H to work around caddr_t problem, GEC */
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

#ifdef CCI
extern int cci_docommand;
extern int cci_get;
#endif

#define __MAX_HOME_LEN__ 256
#define __SRC__
#include "../libwww2/htaautil.h"
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

/* Mosaic does NOT use either the anchor or style sheet systems of libwww2. */

HText *HTMainText = NULL;               /* Equivalent of main window */

/* These are used in libwww2 */
char *HTAppName = "VMS_Mosaic";
char *HTAppVersion;  /* Now set this in gui.c -- mo_do_gui() */
extern char *HTLibraryVersion;

int force_dump_to_file = 0;             /* Hook to force dumping binary data
                                         * straight to file named by... */
char *force_dump_filename = NULL;       /* this filename. */

/* From gui-documents.c */
extern int interrupted;
extern int frames_interrupted;

/* From HTAccess.c. */
extern char *use_this_url_instead;

/* From HTTP.c */
extern int do_post;
extern char *post_content_type;
extern char *post_data;
extern int do_put;
extern int put_file_size;
extern FILE *put_fp;

#ifdef HAVE_SSL
extern char *encrypt_cipher;
#endif

/* From HTMIME.c */
extern MIMEInfo MIME_http;

/* Agent Spoofing Public Variables */
#define MAX_AGENTS 51
char **agent;
int selectedAgent = 0;

extern int binary_transfer;

#ifdef VMS
static char *mail_fnam = NULL;
#endif /* VMS, BSN, GEC */

/****************************************************************************
 * name:    hack_htmlsrc (PRIVATE)
 * purpose: Do really nasty things to a stream of HTML that just got
 *          pulled over from a server.
 * inputs:  
 *   - none (global HTMainText is assumed to contain current HText object)
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
 *   appropriate mark to make adjustments, and keep the original
 *   head of the allocated block of text in HTMainText->htmlSrcHead.
 ****************************************************************************/
static char *hack_htmlsrc(void)
{
  if (!HTMainText)
      return NULL;

  if (!HTMainText->htmlSrc) {
      HTMainText->htmlSrcHead = NULL;
      return NULL;
  }

  /* Keep pointer to real head of htmlSrc memory block. */
  HTMainText->htmlSrcHead = HTMainText->htmlSrc;
  
  if (HTMainText->srclen > 30) {
      char *loc;
      int len;
      int count = 0;

      /* Remove any nulls, but give up if is garbage */
      while ((count++ < 100) &&
	     (len = strlen(HTMainText->htmlSrc)) < (HTMainText->srclen - 1)) {
	  loc = HTMainText->htmlSrc + len;
	  *loc = '\n';
      }
      if (!my_strncasecmp(HTMainText->htmlSrc, "<PLAINTEXT>", 11)) {
	  char *ptr = HTMainText->htmlSrc + 11;

          if (!my_strncasecmp(ptr, "<PLAINTEXT>", 11)) {
              HTMainText->htmlSrc += 11;
          } else if (!my_strncasecmp(ptr, "\n<PLAINTEXT>", 12) ||
		     !my_strncasecmp(ptr, "\n<TITLE>", 8) ||
		     !my_strncasecmp(ptr, "\n<HEAD>", 7) ||
                     !my_strncasecmp(ptr, "\n<HTML>", 7) ||
                     !my_strncasecmp(ptr, "\n\n<HTML>", 8) ||
                     !my_strncasecmp(ptr, "\n<BASE",  6) ||
                     !my_strncasecmp(ptr, "\n<!--",  5) ||
	  	     !strncmp(ptr, "\n<!DOCTYPE HTML", 15)) {
              HTMainText->htmlSrc += 12;
          }
      }
      if (!strncmp(HTMainText->htmlSrc, 
                   "<TITLE>Document</TITLE>\n<PLAINTEXT>", 35) &&
          !my_strncasecmp(HTMainText->htmlSrc + 35, "\n<TITLE>", 8))
          HTMainText->htmlSrc += 36;
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
static char *doit(char *url, char **texthead)
{
  char *msg;
  int rv;
  mo_window *win = current_win;

  /* Hmmmmmmm... */
  if (HTMainText) {
      free(HTMainText);
      HTMainText = NULL;
  }
  if (mo_gui_check_icon(1) || frames_interrupted) {
      interrupted = 1;
      *texthead = NULL;
      return NULL;
  }

  is_uncompressed = 0;

  rv = HTLoadAbsolute(url);

  if (rv == 1) {
      char *txt = hack_htmlsrc();

      if (HTMainText) {
          *texthead = HTMainText->htmlSrcHead;
      } else {
          *texthead = NULL;
      }
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
	  if (txt) {
	      return txt;
	  } else {
	      /* Take care of failed local access */
	      txt = strdup("<H1>ERROR</H1>"); 
	  }
      }
#endif
      return txt;
  }

  /* No markup returned at this point */

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
char *mo_pull_er_over(char *url, char **texthead)
{
  char *rtext;
 
  /* Always reset the icon interrupt */
  mo_gui_clear_icon();

  if (binary_transfer) {
      force_dump_to_file = 1;
      force_dump_filename = mo_tmpnam(url);
  }
  if (saveFileName)
      free(saveFileName);
  saveFileName = strdup(url);

#ifdef HAVE_SSL
  if (encrypt_cipher) {
      free(encrypt_cipher);
      encrypt_cipher = NULL;
  }
#endif

  rtext = doit(url, texthead);

  if (binary_transfer) {
      force_dump_to_file = 0;
      free(force_dump_filename);
      force_dump_filename = NULL;
  }
  return rtext;
}


char *mo_post_pull_er_over(char *url, char *content_type, char *data, 
                           char **texthead)
{
  char *rtext;

  /* Always reset the icon interrupt */
  mo_gui_clear_icon();

  do_post = 1;
  post_content_type = content_type;
  post_data = data;

  if (binary_transfer) {
      force_dump_to_file = 1;
      force_dump_filename = mo_tmpnam(url);
  }

  rtext = doit(url, texthead);

  if (binary_transfer) {
      force_dump_to_file = 0;
      free(force_dump_filename);
      force_dump_filename = NULL;
  }
  do_post = 0;

  return rtext;
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
mo_status mo_pull_er_over_virgin(char *url, char *fnam)
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
  if (saveFileName)
      free(saveFileName);

  saveFileName = strdup(url);

  is_uncompressed = 0;

  rv = HTLoadAbsolute(url);

  force_dump_to_file = 0;
  if (rv == 1) {
      return mo_succeed;
  } else if (rv == -1) {
      interrupted = 1;
  }
  return mo_fail;
}


mo_status mo_re_init_formats(void)
{
  HTReInit();
  return mo_succeed;
}

/* ------------------------------------------------------------------------ */

HText *HText_new(void)
{
  HText *htObj = (HText *)calloc(1, sizeof(HText));

  /** calloc zeros
  htObj->expandedAddress = NULL;
  htObj->simpleAddress = NULL;
  htObj->htmlSrc = NULL;
  htObj->htmlSrcHead = NULL;
  htObj->srcalloc = 0;
  htObj->srclen = 0;
  **/

  /* Free the struct but not the text, as it will be handled
   * by Mosaic proper -- apparently. */
  if (HTMainText)
      free(HTMainText);

  HTMainText = htObj;

  return htObj;
}

void HText_beginAppend(HText *text)
{
  HTMainText = text;
  return;
}

void HText_endAppend(HText *text)
{
  if (text)
      HText_appendCharacter(text, '\0');
  HTMainText = text;
  return;
}

void HText_doAbort(HText *text)
{
  /* Clean up -- we want to free htmlSrc here because htmlSrcHead
   * doesn't get assigned until hack_htmlsrc, and by the time we
   * reach that, this should never be called. */
  if (text) {
      if (text->htmlSrc)
          free(text->htmlSrc);
      text->htmlSrc = NULL;
      text->htmlSrcHead = NULL;
      text->srcalloc = 0;
      text->srclen = 0;
  }
  return;
}

void HText_clearOutForNewContents(HText *text)
{
  HText_doAbort(text);
}

static void new_chunk(HText *text)
{
  if (!text->srcalloc) {
      text->htmlSrc = (char *)malloc(MO_BUFFER_SIZE);
      *text->htmlSrc = '\0';
  } else {
      text->htmlSrc = (char *)realloc(text->htmlSrc,
				      text->srcalloc + MO_BUFFER_SIZE);
  }
  text->srcalloc += MO_BUFFER_SIZE;

  return;
}

void HText_appendCharacter(HText *text, char ch)
{
  if (!text)
      return;

  if (text->srcalloc < text->srclen + 1)
      new_chunk(text);

  text->htmlSrc[text->srclen++] = ch;

  return;
}

void HText_appendText(HText *text, char *str)
{
  int len;

  if (!str || !text)
      return;

  len = strlen(str);

  while (text->srcalloc < text->srclen + len + 1)
      new_chunk(text);

  memcpy(text->htmlSrc + text->srclen, str, len);

  text->srclen += len;
  text->htmlSrc[text->srclen] = '\0';

  return;
}

void HText_appendBlock(HText *text, char *data, int len)
{
  if (!data || !text)
      return;
  
  while (text->srcalloc < text->srclen + len + 1)
      new_chunk(text);

  memcpy(text->htmlSrc + text->srclen, data, len);

  text->srclen += len;
  text->htmlSrc[text->srclen] = '\0';

  return;
}

void HText_beginAnchor(HText *text, char *anc)
{
  HText_appendText(text, "<A HREF=\"");
  HText_appendText(text, anc);
  HText_appendText(text, "\">");
  return;
}

void HText_endAnchor(HText *text)
{
  HText_appendText(text, "</A>");
  return;
}

char *HText_getText(HText *me)
{
  if (me) {
      return me->htmlSrc;
  } else {
      return NULL;
  }
}

int HText_getTextLength(HText *me)
{
  if (me) {
      return me->srclen;
  } else {
      return 0;
  }
}


/****************************************************************************
 * name:    fileOrServer
 * purpose: Given a string, checks to see if it can stat it.  If so, it is
 *     assumed the user expects to open the file, not a web site.  If not, we
 *     assume it is supposed to be a server and prepend the default protocol.
 * inputs:  
 *   - char    *url: URL to canonicalize.
 * returns: 
 *   The canonical representation of the URL.
 * remarks: 
 *   Written by spowers@ncsa.uiuc.edu
 ****************************************************************************/
static char *fileOrServer(char *url)
{
#if (stat != decc$stat) || !defined(MULTINET)
    struct stat buf;
#else
#undef stat
    struct stat buf;
#define stat decc$stat
#endif /* VMS MultiNet work around, GEC */
    char *xurl;
    static int init = 0;
    static char *defproto;

    if (!init) {
	init = 1;
	defproto = get_pref_string(eDEFAULT_PROTOCOL);
    }

    /* At this point we know the URL specified is of the form:
     *   shire.ncsa.uiuc.edu[:PORT]/path/to/something
     * or is a VMS file spec 
     */
    if (!stat(url, &buf)) {  /* It's a file, and we have access */
#ifdef VMS
	/* Get file spec in UNIX syntax */
	int fd = open(url, O_RDONLY, 0);
	char fname[256];

	if (getname(fd, fname, 0)) {
	    /* Strip off version, unless explicity given */
	    char *ptr;

	    if (!strchr(url, ';') && (ptr = strchr(fname, '.'))) {
		if (ptr = strchr(++ptr, '.'))
		    *ptr = '\0';
	    }
	    xurl = mo_url_canonicalize_local(fname);
	} else {
	    xurl = mo_url_canonicalize_local(url);
	}
	close(fd);
#else
	xurl = mo_url_canonicalize_local(url);
#endif
    } else if (!defproto || !*defproto) {
	xurl = malloc(strlen(url) + 15);
	sprintf(xurl, "http://%s", url);
    } else {
	xurl = malloc(strlen(url) + strlen(defproto) + 10);
	sprintf(xurl, "%s://%s", defproto, url);
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
    static int expand = -1;
    static int nexpand;

    if (!url || !*url)
	return(NULL);

    if (expand == -1) {     /* Avoid repeating routine call */
	expand = get_pref_boolean(eEXPAND_URLS);
	nexpand = get_pref_boolean(eEXPAND_URLS_WITH_NAME);
    }
    if (!expand) {
	if (!strchr(url, ':')) {
	    /* No colon found, treat as file */
	    xurl = mo_url_canonicalize_local(url);
	} else {
	    /* It's prolly a real url */
	    xurl = strdup(url);
	}
    } else if (my_strncasecmp(url, "mailto:", 7) &&
	       my_strncasecmp(url, "news:", 5) &&
	       my_strncasecmp(url, "about:", 6) &&
	       my_strncasecmp(url, "cookiejar:", 10) &&
	       !strstr(url, "://")) {	/* No protocol specified, default */
	if (nexpand) {
	    if (!my_strncasecmp(url, "www.", 4)) {
		xurl = (char *)malloc(strlen(url) + (8 * sizeof(char)));
		sprintf(xurl, "http://%s", url);

	    } else if (!my_strncasecmp(url, "ftp.", 4)) {
		xurl = (char *)malloc(strlen(url) + (7 * sizeof(char)));
		sprintf(xurl, "ftp://%s", url);

	    } else if (!my_strncasecmp(url, "gopher.", 7)) {
		xurl = (char *)malloc(strlen(url) + (10 * sizeof(char)));
		sprintf(xurl, "gopher://%s", url);

	    } else if (!my_strncasecmp(url, "wais.", 5)) {
		xurl = (char *)malloc(strlen(url) + (8 * sizeof(char)));
		sprintf(xurl, "wais://%s", url);

	    } else {
		xurl = fileOrServer(url);
	    }
	} else {
	    xurl = fileOrServer(url);
	}
    } else {	/* Protocol was specified */
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
char *mo_url_canonicalize(char *url, char *oldurl)
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
char *mo_url_canonicalize_keep_anchor(char *url, char *oldurl)
{
  /* We KEEP anchor information already present in url,
   * but any anchor in oldurl is ignored. */
  return HTParse(url, oldurl, PARSE_ACCESS | PARSE_HOST | PARSE_PATH |
                 PARSE_PUNCTUATION | PARSE_ANCHOR);
}


/****************************************************************************
 * name:    mo_url_extract_anchor
 * purpose: Given a URL (presumably in canonical form), extract
 *          the internal anchor, if any.
 * inputs:  
 *   - char *url: URL to use.
 * returns: 
 *   Internal anchor, if one exists in the URL; else NULL string.
 * remarks: 
 *   
 ****************************************************************************/
char *mo_url_extract_anchor(char *url)
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
char *mo_url_extract_access(char *url, char *oldurl)
{
  return HTParse(url, oldurl, PARSE_ACCESS);
}


char *mo_url_canonicalize_local(char *url)
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

  if (!url) {
#ifndef CONVEX
      free(cwd);
#endif
      return NULL;
  }

  tmp = (char *)malloc(strlen(url) + strlen(cwd) + 32);
  if (*url == '/') {
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
 *   The new temporary filename in malloced memory with a litte extra room.
 * remarks: 
 *   We make up an unique filename and use the preference
 *   TMP_DIRECTORY, if it has a value, for the directory.
 ****************************************************************************/
#ifndef L_tmpnam
#define L_tmpnam 32
#endif
char *mo_tmpnam(char *url)
{
#ifdef CCI
  extern void MoCCIAddFileURLToList(char *, char *);
#endif
  char *tmp;
  static int len, tdlen;
  static char *tmp_dir, *unique;
  static int init = 0;
  static int count = 0;

  /* Get unique string for this process */
  if (!init) {
      unique = strdup(tmpnam(NULL));
      unique += 3;
      len = L_tmpnam + 32;
      tmp_dir = get_pref_string(eTMP_DIRECTORY);
      if (tmp_dir)
          tdlen = strlen(tmp_dir);
      init = 1;
  }
  count++;

  tmp = (char *)malloc(sizeof(char) * len);
  if (!tmp) {
      fprintf(stderr, "Unable to get storage for tmp name\n");
      return NULL;
  }

  sprintf(tmp, "MOSAIC-TMP%d_%s", count, unique);

  if (!tmp_dir) {
      /* Fast path. */
#ifdef CCI
      if (url)
	  MoCCIAddFileURLToList(tmp, url);
#endif
      return tmp;
  } else {
      /* OK, reconstruct to go in the directory of our choice. */
      char *oldtmp = tmp;
      int i;

#ifndef VMS
      /* Start at the back and work our way forward. */
      for (i = strlen(oldtmp) - 1; i >= 0; i--) {
          if (oldtmp[i] == '/')
              goto found_it;
      }
      
      /* No luck, just punt. */
#ifdef CCI
      if (url)
	  MoCCIAddFileURLToList(tmp, url);
#endif
      return tmp;
#else
      i = 0;
#endif

    found_it:
      tmp = (char *)malloc(sizeof(char) * (tdlen + strlen(&oldtmp[i]) + 20));
      if (!tmp) {
  	  fprintf(stderr, "Unable to get second storage for tmp name\n");
	  return oldtmp;
      }
#ifndef VMS
      if (tmp_dir[tdlen - 1] == '/') {
          /* Trailing slash in tmp_directory spec. */
          sprintf(tmp, "%s%s", tmp_dir, &oldtmp[i] + 1);
      } else {
          /* No trailing slash. */
          sprintf(tmp, "%s%s", tmp_dir, &oldtmp[i]);
      }
#else
      if ((tmp_dir[tdlen - 1] == ']') ||
          (tmp_dir[tdlen - 1] == ':')) {
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
char *strdup(char *str)
{
  char *dup;

  if (!str)
      return NULL;

  if (!(dup = (char *)malloc(strlen(str) + 1)))
      return NULL;

  strcpy(dup, str);

  return dup;
}
#endif


/* Error from the library */
void application_error(char *str, char *title)
{
  XmxMakeErrorDialogWait(current_win->base, app_context, str, title, "OK");
}

/* Warning from the library */
void application_warning(char *str, char *title)
{
  XmxMakeWarningDialogWait(current_win->base, app_context, str, title, "OK");
}

/* Feedback from the library. */
void application_user_feedback(char *str)
{
  extern Widget toplevel;

  XmxMakeInfoDialog(toplevel, str, "VMS Mosaic: Application Feedback");
}

void application_user_info_wait(char *str)
{
  XmxMakeInfoDialogWait(current_win->base, app_context, str,
			"VMS Mosaic: Application Feedback", "OK");
}

/* Returned string must be freed with XtFree */
char *prompt_for_string(char *questionstr)
{
  return XmxModalPromptForString(current_win->base, app_context,
                                 questionstr, "OK", "Cancel");
}

/* Returned string must be freed */
char *prompt_for_password(char *questionstr)
{
  return XmxModalPromptForPassword(current_win->base, app_context,
                                   questionstr, "OK", "Cancel");
}

int prompt_for_yes_or_no(char *questionstr)
{
  return XmxModalYesOrNo(current_win->base, app_context,
                         questionstr, "Yes", "No");
}

char *mo_get_html_return(char **texthead)
{
  char *txt = hack_htmlsrc();

  *texthead = HTMainText->htmlSrcHead;
  return txt;
}


/* Convert all newlines and CRs to spaces, and remove leading whitespace.
 * Returns True if converted any newlines. */
Boolean mo_convert_newlines_to_spaces(char *str)
{
  int i;
  char *tptr = str;
  Boolean converted = False;

  if (!str)
      return False;

  for (i = 0; i < strlen(str); i++) {
      if ((str[i] == '\n') || (str[i] == '\r')) {
          str[i] = ' ';
          converted = True;
      }
  }

  while (*tptr && isspace((int)*tptr))
      tptr++;

  if (tptr != str)
      memcpy(str, tptr, strlen(tptr) + 1);

  return converted;
}

/* ---------------------------- escaping code ----------------------------- */

#define MO_HEX(i) (i < 10 ? '0' + i : 'A' + i - 10)

/* Create new string with no LFs, CRs, tabs, leading and tailing spaces,
 * and with other spaces and controls escaped. */
char *mo_clean_and_escape_url(char *url, int free_it)
{
  char *q, *p, *escaped;
  int len;

  if (!url)
      return NULL;

  len = strlen(url);
  escaped = (char *)malloc((len * 3) + 1);

  /* Remove tailing spaces */
  q = url + len - 1;
  while ((q >= url) && (*q == ' '))
      *q-- = '\0';

  p = url;
  /* Skip over leading spaces */
  while (*p == ' ')
      p++;

  for (q = escaped; *p; p++) {
      /*
       * This makes sure that values 128 and over don't get
       * converted to negative values.
       */
      int c = (int)((unsigned char)*p);

      if (c >= 33 && c <= 127) {
          *q++ = *p;
      } else if ((*p != '\n') && (*p != '\r') && (*p != '\t')) {
          *q++ = '%';
          *q++ = MO_HEX(c / 16);
          *q++ = MO_HEX(c % 16);
      }
  }
  
  *q = '\0';

  if (free_it == 1) {
      free(url);
  } else if (free_it == 2) {
      XtFree(url);
  }
  return escaped;
}

/* Convert escaped spaces to real spaces */
char *mo_unescape_spaces(char *txt)
{
    char *new, *p;

    if (txt) {
	p = new = malloc(strlen(txt) + 1);
    } else {
	return NULL;
    }
    while (*txt) {
        if ((*txt == '%') && (*(txt + 1) == '2') && (*(txt + 2) == '0')) {
             *p++ = ' ';
             txt += 3;
        } else {
             *p++ = *txt++;
        }
    }
    *p = '\0';
    return new;
}

static char mo_from_hex(char c)
{
  return ((c >= '0' && c <= '9') ? (c - '0') : 
          ((c >= 'A' && c <= 'F') ? (c - 'A' + 10) : 
           (c - 'a' + 10)));
}

char *mo_unescape_part(char *str)
{
  char *p = str;
  char *q = str;

  while (*p) {
      /* Plus's turn back into spaces. */
      if (*p == '+') {
          *q++ = ' ';
          p++;
      } else if (*p == '%') {
          if (*++p) 
              *q = mo_from_hex(*p++) * 16;
          if (*p) 
              *q += mo_from_hex(*p++);
          q++;
      } else {
          *q++ = *p++; 
      }
  }
  *q = '\0';
  return str;
}


/* ---------------------------- Agent Spoofing ---------------------------- */

/*
 * Agent Spoofing is simple.  Mosaic's real agent is always a member of the
 * menu.  Any more than that, you can add to the file in your home directory
 * called ".mosaic-spoof-agents".
 */
static int readAgents(int numAgents)
{
    FILE *fp;
    char fname[BUFSIZ], buf[512];
    char *homedir, *ptr;

    if (get_home(&homedir) || !homedir) {
	fprintf(stderr, "Agents: Could not get your home directory.\n");
	return numAgents;
    }
#ifndef VMS  /* PGE */
    sprintf(fname, "%s/.mosaic-spoof-agents", homedir);
#else
    sprintf(fname, "%smosaic-spoof-agents", homedir);
#endif
    free(homedir);

    if (!(fp = fopen(fname, "r")))
	return numAgents;

    while (!feof(fp)) {
	fgets(buf, 511, fp);
	if (feof(fp))
	    break;
	if (*buf && *buf != '#') {
	    buf[strlen(buf) - 1] = '\0';
	    for (ptr = buf; *ptr && isspace(*ptr); ptr++)
		;
	    if (*ptr == '+') {	/* This is to be the default */
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
	    if (++numAgents == MAX_AGENTS) {	/* Limit reached */
		fprintf(stderr,
		        "WARNING: Hard limit reached for agent spoof file.\n");
		break;
	    }
	}
    }
    fclose(fp);

    return numAgents;
}


int loadAgents(void)
{
  char buf[512];
  int numAgents = 6;

  agent = (char **)calloc(MAX_AGENTS + 1, sizeof(char *));
  sprintf(buf, "%s/%s libwww/%s",
	  HTAppName ? HTAppName : "unknown",
	  HTAppVersion ? HTAppVersion : "0.0",
	  HTLibraryVersion);
  agent[0] = strdup(buf);
  agent[1] = strdup(
	"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.2; VMS Mosaic)");
  agent[2] = strdup(
	"Mozilla/5.0 (X11; U; VMS Mosaic; en-US; rv:1.7.8) Gecko/20050511 Firefox/1.0.4");
  agent[3] = strdup("Lynx/2.7 libwww-FM/2.14 (VMS Mosaic)");
  agent[4] = strdup(
	"Mozilla/5.0 (X11; U; OpenVMS Alpha; en-US; rv:1.7) Gecko/20040621)");
  agent[5] = strdup("Mozilla/4.0 (VMS Mosaic)");

  numAgents = readAgents(numAgents);

  return numAgents;
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

void StartClock(void) 
{
  gtime = times(&tbuf);
  
  return;
}

void StopClock()
{
#ifndef VMS
  int donetime;
#else
  time_t donetime;
#endif /* VMS, BSN*/

  donetime = times(&tbuf);

#ifndef DISABLE_TRACE
  if (srcTrace)
      fprintf(stderr, "Elapsed time %d\n", donetime - gtime);
#endif

  return;
}
#endif


/* ------------------------- upload stuff -------------------------- */
static char *mo_put_er_over(char *url, char **texthead)
{
    char *rtext;

    do_put = do_post = 1;

    if (saveFileName)
	free(saveFileName);
    saveFileName = strdup(url);

    rtext = doit(url, texthead);

    do_put = do_post = 0;

    return(rtext);
}  


static mo_status upload(mo_window *win, FILE *fp, char *fname)
{
    char *put_url, *xurl, *newtext;
    char *newtexthead = NULL;
    int res = mo_fail;

    if (!win)
	return(res);

    put_url = prompt_for_string("Enter the URL to upload the file as:");
    if (!put_url)
	return(res);

    xurl = mo_url_prepend_protocol(put_url);
    XtFree(put_url);
    put_url = xurl;

    fseek(fp, 0, SEEK_END);
    put_file_size = ftell(fp);
    rewind(fp);
    put_fp = fp;

    if (win->target_anchor)
	free(win->target_anchor);
    win->target_anchor = NULL;

    mo_set_current_cached_win(win);

    newtext = mo_put_er_over(put_url, &newtexthead);

    if (newtext && strncmp(newtext, "<H1>ERROR<H1>", 10) &&
	strncmp(newtext, "<HEAD><TITLE>404 Not Found</TITLE></HEAD>", 28))
	res = mo_succeed;

    /* Check for redirect URL */
    if (use_this_url_instead) {
	mo_here_we_are_son(put_url);
	free(put_url);
	put_url = use_this_url_instead;

	/* Go get another target_anchor. */
	if (win->target_anchor)
	    free(win->target_anchor);
	win->target_anchor = NULL;
    }
    if (newtext)
	mo_do_window_text(win, put_url, newtext, newtexthead,
			  1, NULL, MIME_http.last_modified,
			  MIME_http.expires, MIME_http.charset);
    if (win->current_node)
	mo_gui_check_win_security_icon(win->current_node->authType,
				       current_win);
    free(put_url);

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
	if (srcTrace)
	    fprintf(stderr, "Error in evaluating the path.\n");
#endif
    }
    if (!(fp = fopen(efname, "r"))) {
	char *buf, *final;
	char tmpbuf[80];
	int final_len;

#ifdef CCI
	/* Don't display dialog if command issued by cci application */
	if (cci_docommand)
	    return(mo_fail);
#endif
	buf = my_strerror(errno);
	if (!buf || !*buf || !strcmp(buf, "Error 0")) {
	    sprintf(tmpbuf, "Unknown Error");
	    buf = tmpbuf;
	}
	final_len = 30 + ((!efname || !*efname ? 3 : strlen(efname)) + 13) +
		    15 + (strlen(buf) + 3);
	final = (char *)malloc(final_len);

	sprintf(final,
	        "\nUnable to upload document:\n   %s\n\nUpload Error:\n   %s\n",
		(!efname || !*efname ? " " : efname), buf);

	application_error(final, "Upload Error");

	if (final)
	    free(final);
	free(efname);

	return(mo_fail);
    }

    res = upload(win, fp, efname);
    fclose(fp);
    free(efname);

    return(res);
}


static XmxCallback(upload_win_cb)
{
    char *fname;
    mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

    XtUnmanageChild(win->upload_win);
  
    XmStringGetLtoR(((XmFileSelectionBoxCallbackStruct *)call_data)->value,
		    XmSTRING_DEFAULT_CHARSET, &fname);

    mo_upload_window(win, fname);

    XtFree(fname);
    return;
}


mo_status mo_post_upload_window(mo_window *win)
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
