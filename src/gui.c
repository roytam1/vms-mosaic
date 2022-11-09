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

/* Copyright (C) 1998, 1999, 2000, 2003, 2004, 2005, 2006
 * The VMS Mosaic Project
 */

#include "../config.h"
#include "../libwww2/HTAABrow.h"
#include "../libwww2/htalert.h"
#include "../libwww2/HTfile.h"
#include "../libwww2/HTftp.h"
#include "../libwww2/HTcookie.h"
#include "../libwww2/http.h"
#include "mosaic.h"
#include "gui.h"
#include "gui-documents.h"
#include "main.h"
#include "mo-www.h"
#include "proxy.h"
#include "gui-menubar.h"
#include "gui-popup.h"
#include "pan.h"
#include "pixmaps.h"
#include "colors.h"
#include "../libhtmlw/HTMLp.h"
#include "../libnut/system.h"
#include "../libnut/str-tools.h"
#include "history.h"
#include "gui-dialogs.h"
#include "globalhist.h"
#include "hotlist.h"
#ifdef CCI
#include "ccibindings2.h"
#endif
#include "gui-news.h"
#include "annotate.h"
#include "comment.h"
#include "quantize.h"
#include <X11/Xatom.h>

#include <signal.h>

#ifndef MULTINET
#ifndef VMS
#include <sys/socket.h>
#include <netinet/in.h>
#endif /* Cause logical name problems plus already included in tcp.h, GEC */
#if !defined(ultrix)
#include <netdb.h>
#endif
#endif

#include <ctype.h>
#ifndef VMS
#include <sys/utsname.h>
#include <pwd.h>
#else
#include "vms_pwd.h"
#include <lib$routines.h>
#endif  /* PGE, GEC */

struct Proxy *noproxy_list = NULL, *proxy_list = NULL;
struct Proxy *ReadProxies();

static void mo_fill_toolbar(mo_window *win);

#define __SRC__
#include "../libwww2/HTAAUtil.h"

extern Pixmap *tmp_pix;

char pre_title[80];

extern int installed_colormap;
extern Colormap installed_cmap;

/* Spoof Agent stuff */
extern int selectedAgent;

extern int newsShowAllGroups;
extern int newsShowAllArticles;
extern int newsShowReadGroups;
extern int newsNoThreadJumping;
extern int ConfigView;

extern int interrupted;

/* 1 if we need to free colors, 0 if already popped down */
static int splash_cc = 1;
static Widget splash = NULL;
static int do_splash;

/* Flag to track if we want to start up as an icon */
static int iconic = 0;

static char *slab_words[] =
    {"MENU", "TITLE", "URL", "TOOLS", "STATUS", "VIEW", "GLOBE", "SMALLGLOBE",
     "TEXTTOOLS", NULL};

static int sarg[7];
static int scount = -1, smalllogo = 0, stexttools = 0, pres = 0;

extern int tableSupportEnabled;
extern int securityType;
extern int progressiveDisplayEnabled;
int browserSafeColors;

/* From libwww2 http.c */
#ifdef HAVE_SSL
extern int verifyCertificates;
#endif

/* Doesn't seem to be on all X11R4 systems
#if (XtSpecificationRelease == 4)
extern void _XEditResCheckMessages();
#define EDITRES_SUPPORT
#endif
*/

#if (XtSpecificationRelease > 4)
#define EDITRES_SUPPORT
#endif

/*
 * EDITRES_SUPPORT seems to fail with the HP libraries
 */
#if defined(__hpux) && defined(EDITRES_SUPPORT)
#undef EDITRES_SUPPORT
#endif

#ifdef VMS
#undef EDITRES_SUPPORT
#endif /* VMS Xmu has no Editres, BSN */

#ifdef EDITRES_SUPPORT
#include <X11/Xmu/Editres.h>
#endif

#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/ScrolledW.h>
#include <Xm/ScrollBar.h>
#include <Xm/List.h>
#include <Xm/ToggleB.h>
#include <Xm/DrawnB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/DrawingA.h>
#include <Xm/MainW.h>
#include <Xm/MenuShell.h>
#include <Xm/Protocols.h>
#include <Xm/Separator.h>
#include <Xm/BulletinB.h>
#include <Xm/RowColumn.h>
#include <Xm/DialogS.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#if defined(VMS) && !defined(__DECC)
#include <X11/Intrinsicp.h>
#endif /* VAX C needs it to define _WidgetRec, GEC */

#include "img.h"
#include "xresources.h"
#ifdef CCI
#include "cci.h"
#endif

#include "bitmaps/iconify.xbm"
#include "bitmaps/iconify_mask.xbm"
#include "bitmaps/xmosaic_icon.xbm"
#include "bitmaps/xmosaic_32_icon.xbm"
#include "bitmaps/xmosaic_75_icon.xbm"
#include "bitmaps/xmosaic.xbm"
#include "bitmaps/xmosaic_left.xbm"
#include "bitmaps/xmosaic_right.xbm"
#include "bitmaps/xmosaic_down.xbm"
#include "bitmaps/security.xbm"
/* Busy Cursor */
#include "bitmaps/busy_1.xbm"
#include "bitmaps/busy_2.xbm"
#include "bitmaps/busy_3.xbm"
#include "bitmaps/busy_4.xbm"
#include "bitmaps/busy_5.xbm"
#include "bitmaps/busy_6.xbm"
#include "bitmaps/busy_7.xbm"
#include "bitmaps/busy_8.xbm"
#include "bitmaps/busy_9.xbm"
#include "bitmaps/busy_1_mask.xbm"
#include "bitmaps/busy_2_mask.xbm"
#include "bitmaps/busy_3_mask.xbm"
#include "bitmaps/busy_4_mask.xbm"
#include "bitmaps/busy_5_mask.xbm"
#include "bitmaps/busy_6_mask.xbm"
#include "bitmaps/busy_7_mask.xbm"
#include "bitmaps/busy_8_mask.xbm"
#include "bitmaps/busy_9_mask.xbm"

/* For selective image loading */
char **imagedelay_sites = NULL;
char **imagekill_sites = NULL;
Boolean currently_delaying_images = 0;

/*******************************/

/* Because Sun cripples the keysym.h file. */
#ifndef XK_KP_Up
#define XK_KP_Home              0xFF95  /* Keypad Home */
#define XK_KP_Left              0xFF96  /* Keypad Left Arrow */
#define XK_KP_Up                0xFF97  /* Keypad Up Arrow */
#define XK_KP_Right             0xFF98  /* Keypad Right Arrow */
#define XK_KP_Down              0xFF99  /* Keypad Down Arrow */
#define XK_KP_Prior             0xFF9A  /* Keypad Page Up */
#define XK_KP_Next              0xFF9B  /* Keypad Page Down */
#define XK_KP_End               0xFF9C  /* Keypad End */
#endif

#define SLAB_MENU 0
#define SLAB_TITLE 1
#define SLAB_URL 2
#define SLAB_TOOLS 3
#define SLAB_STATUS 4
#define SLAB_VIEW 5
#define SLAB_GLOBE 6
#define SLAB_SMALLGLOBE 7
#define SLAB_TEXTTOOLS 8

/*
 * Globals used by the pixmaps for the animated icon.
 * Marc, I imagine you might want to do something cleaner
 * with these?
 *
 * Marc's not here, man.
 */
extern int IconWidth, IconHeight, WindowWidth, WindowHeight;

extern Pixmap *IconPix, *IconPixSmall, *IconPixBig;

#ifdef CCI
extern int cci_event;
#endif

#ifdef VMS
extern unsigned long vms_mosaic_cmdline(int *, char ***);
#endif /* VMS DCL command line interface */

static void paste_init(XtAppContext app);

extern char *HTAppVersion;

/* From [.libwww2]htcookie.c */
extern int HTSetCookies;
extern int HTEatAllCookies;
extern int HTCookieFile;

/* For the -geometry fix (kludge) - DXP 8/30/95 */
static int userSpecifiedGeometry = 0;
static Dimension userWidth, userHeight;
static Position userX, userY;


/* ------------------------------ variables ------------------------------- */

Display *dsp;
XtAppContext app_context;
Widget toplevel;
Widget view = NULL;  /* HORRIBLE HACK @@@@ */

int Vclass;  		/* Visual class for 24bit support hack */
Visual *theVisual;	/* The visual we are using for display */

int LimDim;  /* Are Pixmaps limited to display dimensions */
int LimDimX;
int LimDimY;

char *global_xterm_str;  /* Required for HTAccess.c now */

char *uncompress_program;
char *gunzip_program;

int use_default_extension_map;
char *global_extension_map;
char *personal_extension_map;
int use_default_type_map;
char *global_type_map;
char *personal_type_map;

int tweak_gopher_types;
int max_wais_responses;
int useAFS;
int ftp_timeout_val;
int ftpRedial;
int ftpRedialSleep;
int ftpFilenameLength;
int ftpEllipsisLength;
int ftpEllipsisMode;
int twirl_increment;

char kioskProtocols[50][40];  /* 50 protocols 40 chars long should be plenty */
int max_kiosk_protocols = 0;

extern int sendAgent;
extern int sendReferer;
extern int imageViewInternal;

#ifdef PRERELEASE
extern int do_comment;
#endif

static int connect_interrupt = 0;
extern int sleep_interrupt;

static int use_tool[BTN_COUNT];


#ifdef VMS  /* PGE, allows resource file have font lists */ 
/*
** Takes a string "fontname/fontname/.../fontname" and tries loading the fonts
** in order until one succeeds.  If none succeeds then returns fail.  Must be
** loaded with a call to XtSetTypeConverter before resource list is read.
*/
static Boolean convertMultiFontStruct(Display   *display,
                                      XrmValue  *args,
                                      Cardinal  *num_args,
                                      XrmValue  *src,
                                      XrmValue  *dest,
                                      XtPointer *converter_data)
{
   XFontStruct *font = NULL;
   char        *name = (char *)src->addr;
   char        *slash;

   slash = strchr(name, '/');
   if (slash == NULL) {
      font = XLoadQueryFont(display, name);
   } else {
      char  temp[200];
      char *slash2;
      int   count;

      while (name && (font == NULL)) {
         if (slash == NULL) {
            count = 200;
         } else {
            count = slash - name + 5;
	 }
         strncpy(temp, name, (count > 200 ? 200 : count));
         temp[199] = '\0';
         slash2 = strchr(name, '/');
         if (slash2)
            *slash2 = '\0';

         font = XLoadQueryFont(display, temp);
         if (font == NULL) {
            if (slash) {
               name = slash + 1;
               slash = strchr(name, '/');
            } else {
               name = NULL;
            }
         }
      }
   }

   if (!font) {
      int count = 1;

      name = (char *)src->addr;
      XtAppWarningMsg(app_context, "", "", "",
                      "Cannot convert string\n\"%s\" to type FontStruct\n",
                      &name, (unsigned int *)&count);
      font = XLoadQueryFont(display, "fixed");
   }

   if (font) {
      /* If memory for result not allocated then do it ourselves */
      if (dest->addr == NULL) {
         dest->size = sizeof(XFontStruct *);
         dest->addr = (XtPointer) malloc(sizeof(XFontStruct *));
      }

      /* If memory for result not large enough then return required amount */
      if (dest->size < sizeof(XFontStruct *)) {
         dest->size = sizeof(XFontStruct *);
         return False;
      }

      /* Finally, return the result. */
      *((XFontStruct **) dest->addr) = font;
      return True;
   } else {
      return False;
   }
}

/*
** Same as convertMultiFontStruct except it returns an XmFontList instead of an
** XFontStruct.
*/
static Boolean convertMultiFontList(Display   *display,
                                    XrmValue  *args,
                                    Cardinal  *num_args,
                                    XrmValue  *src,
                                    XrmValue  *dest,
                                    XtPointer *converter_data)
{
   XFontStruct *font;
   XmFontList   fontList;
   Boolean      result;
   XrmValue     tempDest;

   tempDest.size = sizeof(XFontStruct *);
   tempDest.addr = (XtPointer)&font;
   result = convertMultiFontStruct(display, args, num_args,
                                   src, &tempDest, converter_data);
   if (result) {
      fontList = XmFontListCreate(font, XmSTRING_DEFAULT_CHARSET);
      if (fontList == NULL) {
         int count = 1;
         char *name = (char *)src->addr;

         XtAppWarningMsg(app_context, "", "", "",
                         "Cannot convert string \"%s\" to type FontList\n",
                         &name, (unsigned int *)&count);
         result = False;
      } else {
         /* If memory for result not allocated then do it ourselves */
         if (dest->addr == NULL) {
            dest->size = sizeof(XmFontList);
            dest->addr = (XtPointer) malloc(sizeof(XmFontList));
         }

         /* If memory for result not large enough then return required amount */
         if (dest->size < sizeof(XmFontList)) {
            dest->size = sizeof(XmFontList);
            return False;
         }

         /* Finally, return the result. */
         *((XmFontList *) dest->addr) = fontList;
      }
   }
   return result;
}
#endif

/* --------------BalloonHelpStuff---------------------------------------- */

static void BalloonHelpMe(Widget w, XEvent *event)
{
    char *info;

    XtVaGetValues(w, XmNuserData, (XtPointer) &info, NULL);    
    mo_gui_notify_progress(info);
}

static void UnBalloonHelpMe(Widget w, XEvent *event)
{
    mo_gui_notify_progress(" ");
}

/* Icon balloon help.  Also disable button 2 Drag and Drop */
static char xlattab[] = "\
	<Enter>:	BalloonHelpMe()		\n\
	<Leave>:	UnBalloonHelpMe()	\n\
        <Btn2Down>:     take_focus()";

static XtActionsRec balloon_action[] = {
    {"BalloonHelpMe", (XtActionProc)BalloonHelpMe},
    {"UnBalloonHelpMe", (XtActionProc)UnBalloonHelpMe}
};

/* To use balloon help, add these bits to your widget ...
 *    XmNtranslations, XtParseTranslationTable(xlattab),
 *    XmNuserData, (xtpointer) "Balloon Help String!",
 */

/* ------------------------------------------------------ */

/* emacs bindings to be used in text fields */

static char text_translations[] = "\
           ~Meta ~Alt Ctrl<Key>u:	beginning-of-line()		\
					delete-to-end-of-line()		\n\
           ~Meta ~Alt Ctrl<Key>x:	beginning-of-line()		\
					delete-to-end-of-line()		\n\
           ~Meta ~Alt Ctrl<Key>k:	delete-to-end-of-line()		\n\
           ~Meta ~Alt Ctrl<Key>a:	beginning-of-line()		\n\
           ~Meta ~Alt Ctrl<Key>e:	end-of-line()   		\n\
           ~Meta ~Alt Ctrl<Key>w:	key-select()			\
					delete-selection()		\n\
           ~Meta ~Alt Ctrl<Key>y:	paste-clipboard()		\n\
	 Meta ~Ctrl       <Key>d:	delete-next-word()		\n\
	  Alt ~Ctrl       <Key>d:	delete-next-word()		\n\
           ~Meta ~Alt Ctrl<Key>d:       delete-next-character()         \n\
     Meta ~Ctrl<Key>osfBackSpace:	delete-previous-word()		\n\
      Alt ~Ctrl<Key>osfBackSpace:	delete-previous-word()		\n\
	Meta ~Ctrl<Key>osfDelete:	delete-next-word()		\n\
	 Alt ~Ctrl<Key>osfDelete:	delete-next-word()              \n\
                      <Btn1Down>:       take_focus() grab-focus()";

/* This will have to be handled dynamically when we go to preferences */

static char url_translations[] = "Ctrl<Key>z:         set_focus_to_view()";

static char logo_translations[] = "<Btn2Down>:\n <Btn2Up>: URLPaste()";

static void set_focus_to_view(Widget w, XEvent *event,
	       String *params, Cardinal *num_params);
static void take_focus(Widget w, XEvent *event,
	       String *params, Cardinal *num_params);
static XtActionsRec url_actions[] = {
    {"set_focus_to_view", (XtActionProc)set_focus_to_view},
    {"take_focus", (XtActionProc)take_focus}
};

/* This stuff is so we can properly update the current_win variable
   eliminating alot of problems with cloned windows (We love globals!)

   Globals? Where? There are no globals here!
*/

static char toplevel_translations[] = "\
                      <Enter>:            set_current_win() \n\
                      <Leave>:            set_current_win()";

static void set_current_win(Widget w, XEvent *event, String *params,
			    Cardinal *num_params);

static XtActionsRec toplevel_actions[] = {
    {"set_current_win", (XtActionProc)set_current_win}
};

/* ------------------------------------------------------ */

#ifndef DISABLE_TRACE
extern int cookieTrace;
extern int httpTrace;
extern int www2Trace;
extern int htmlwTrace;
extern int tableTrace;
extern int nutTrace;
extern int refreshTrace;

#ifdef CCI
int cciTrace;
#endif
int srcTrace;
int cacheTrace;
int reportBugs;
#endif

#ifdef CCI
/* From cciBindings.c */
extern int cci_get;
#endif

char *HTReferer = NULL;

/* This is exported to libwww, like altogether too many other variables here. */
int binary_transfer;

/* Now we cache the current window right before doing a binary
 * transfer, too.  Sheesh, this is not pretty. */
mo_window *current_win;

/* If startup_document is set to anything but NULL, it will be the
 * initial document viewed (this is separate from home_document below).
 */
char *startup_document = NULL;

/* If startup_document is NULL, home_document will be the initial document. */
char *home_document = NULL;
char *machine;
static char *shortmachine;
char *machine_with_domain;

#ifdef VMS
/*
** Need to distinguish between commandline -home argument and Xresource file
** homeDocument setting so that commandline takes precedence over WWW_HOME
** environment variable.
*/
static char *cmdline_homeDocument = NULL;
#endif /* VMS, LLL */

static int cursorAnimCnt;
static int makeBusy = 0;
static Cursor busy_cursor;
static int busy = 0;
char *cached_url = NULL;

/* Forward declaration of test predicate. */
int anchor_visited_predicate(Widget, char *);

#define MAX_BUSY_CURSORS 9
static int numCursors = MAX_BUSY_CURSORS;
/* Pixmaps for the busy cursor animation */
static Cursor busyCursor[MAX_BUSY_CURSORS];

extern Pixmap toolbarBack, toolbarForward, toolbarHome, toolbarReload,
    toolbarOpen, toolbarSave, toolbarClone, toolbarNew, toolbarClose,
    toolbarBackGRAY, toolbarForwardGRAY,
    toolbarSearch, toolbarPrint, toolbarPost, toolbarFollow,
    tearv, tearh, toolbarPostGRAY, toolbarFollowGRAY,
    toolbarNewsFwd, toolbarNewsFFwd, toolbarNewsRev, toolbarNewsFRev,
    toolbarNewsIndex, toolbarAddHotlist, toolbarNewsGroups,
    toolbarNewsFwdGRAY, toolbarNewsFFwdGRAY, toolbarNewsRevGRAY,
    toolbarNewsFRevGRAY,
    toolbarFTPput, toolbarFTPmkdir, toolbarCookie, toolbarStop;

extern Pixmap securityKerberos4, securityBasic, securityMd5, securityNone,
    securityUnknown, securityKerberos5, securityDomain, securityLogin,
    enc_not_secure;

#ifndef VMS
struct utsname mo_uname;
#endif

#ifdef VMS
extern unsigned long mbx_event_flag;
extern unsigned short mbx_iosb[4];
static char mbx_name[64];
static XtInputId mo_InputId;
static int use_mbx = 0;
static int grp_mbx = 0;
#endif /* VMS, BSN, TJA */


static int kioskSafe(char *url)
{
    int i, psize;
    char *protocol, *ptr;

    if (!max_kiosk_protocols)
	return(1); /* Not supposed to check anything if 0 */

    if (!(ptr = strchr(url, ':')))
	return(1); /* Return safe...not a valid URL */

    psize = ptr - url;
    protocol = (char *)malloc((psize + 1) * sizeof(char));
    strncpy(protocol, url, psize);
    protocol[psize] = '\0';

    for (i = 0; i < max_kiosk_protocols; i++) {
	if (!my_strcasecmp(protocol, kioskProtocols[i])) {
	    free(protocol);
	    return(1);
	}
    }

    free(protocol);

    return(0);
}

/* ----------------------------- WINDOW LIST ------------------------------ */

static mo_window *winlist = NULL;
static int wincount = 0;

/* Return either the first window in the window list or the next
 * window after the current window.
 */
mo_window *mo_next_window(mo_window *win)
{
  if (!win) {
      return winlist;
  } else {
      return win->next;
  }
}

/* Return a window matching a specified uniqid. */
mo_window *mo_fetch_window_by_id(int id)
{
  mo_window *win;

  win = winlist;
  while (win) {
      if (win->id == id)
	  return(win);
      win = win->next;
  }

  return NULL;
}

/* Register a window in the window list. */
static mo_status mo_add_window_to_list(mo_window *win)
{
  wincount++;

  if (winlist == NULL) {
      win->next = NULL;
      winlist = win;
  } else {
      win->next = winlist;
      winlist = win;
  }

  return mo_succeed;
}

/* Remove a window from the window list. */
static mo_status mo_remove_window_from_list(mo_window *win)
{
  mo_window *w = NULL, *prev = NULL;

  while (w = mo_next_window(w)) {
      if (w == win) {
          /* Delete w. */
          if (!prev) {
              /* No previous window. */
              winlist = w->next;
              free(w);
              w = NULL;
              wincount--;

              /* Maybe exit. */
              if (!winlist)
                  mo_exit();
          } else {
              /* Previous window. */
              prev->next = w->next;
              free(w);
              w = NULL;
              wincount--;

              return mo_succeed;
          }
      }
      prev = w;
  }
   
  /* Couldn't find it. */
  return mo_fail;
}


/****************************************************************************
 * name:    mo_assemble_help_url
 * purpose: Make a temporary, unique filename.
 * inputs:  
 *   - char *file: Filename to be appended to Rdata.docs_directory.
 * returns: 
 *   The desired help url (a malloc'd string).
 * remarks: 
 ****************************************************************************/
char *mo_assemble_help_url(char *file)
{
  char *tmp;
  char *docs_directory = get_pref_string(eDOCS_DIRECTORY);

  if (!file || !docs_directory)
      return strdup("http://lose.lose/lose");

  tmp = (char *)malloc((strlen(file) + strlen(docs_directory) + 4) *
                        sizeof(char));

  if (docs_directory[strlen(docs_directory) - 1] == '/') {
      /* Trailing slash in docs_directory spec. */
      sprintf(tmp, "%s%s", docs_directory, file);
  } else {
      /* No trailing slash. */
      sprintf(tmp, "%s/%s", docs_directory, file);
  }
  
  return tmp;
}


/* ----------------------------- busy cursor ------------------------------ */

static void stopBusyAnimation()
{
    mo_window *win = NULL;
  
    if (busy) {
	XUndefineCursor(dsp, XtWindow(toplevel));
	while (win = mo_next_window(win)) {
	    XUndefineCursor(dsp, XtWindow(win->base));
	    if (win->history_win)
		XUndefineCursor(dsp, XtWindow(win->history_win));
	    if (win->hotlist_win)
		XUndefineCursor(dsp, XtWindow(win->hotlist_win));
	    if (win->searchindex_win)
		XUndefineCursor(dsp, XtWindow(win->searchindex_win));
	}
	XFlush(dsp);
	busy = 0;
    }

    return;
}


/* For lack of a better place, we do the iconify icon stuff here as well... */

static void createBusyCursors(Widget bob)
{
	Pixmap pmap, mmap;
	XColor ccell1, ccell_fg, ccell_bg;

	if (!get_pref_boolean(eANIMATEBUSYICON)) {
		numCursors = 1;
		busyCursor[0] = busy_cursor;
		return;
	}

	XAllocNamedColor(dsp, (installed_colormap ?
			      installed_cmap :
			      DefaultColormapOfScreen(XtScreen(bob))),
			 "black", &ccell1, &ccell_fg);
	XAllocNamedColor(dsp,(installed_colormap ?
			      installed_cmap :
			      DefaultColormapOfScreen(XtScreen(bob))),
			 "white", &ccell1, &ccell_bg);

	mmap = XCreatePixmapFromBitmapData(XtDisplay(bob),
		DefaultRootWindow(XtDisplay(bob)), (char *)busy_1_mask_bits,
		busy_1_mask_width, busy_1_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
		(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
		busy_1_bits, busy_1_width, busy_1_height, 1, 0, 1);

	busyCursor[0] = XCreatePixmapCursor(dsp, pmap, mmap, &ccell_fg, 
				&ccell_bg, busy_1_x_hot, busy_1_y_hot);
	XFreePixmap(dsp, mmap);
	XFreePixmap(dsp, pmap);

	mmap = XCreatePixmapFromBitmapData(XtDisplay(bob),
		DefaultRootWindow(XtDisplay(bob)), (char *)busy_2_mask_bits,
		busy_2_mask_width, busy_2_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
		(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
	 	busy_2_bits, busy_2_width, busy_2_height, 1, 0, 1);
	busyCursor[1] = XCreatePixmapCursor(dsp, pmap, mmap, &ccell_fg,
		&ccell_bg, busy_2_x_hot, busy_2_y_hot);
	XFreePixmap(dsp, mmap);
	XFreePixmap(dsp, pmap);

	mmap = XCreatePixmapFromBitmapData(XtDisplay(bob),
		DefaultRootWindow(XtDisplay(bob)), (char *)busy_3_mask_bits,
		busy_3_mask_width, busy_3_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
		(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
		busy_3_bits, busy_3_width, busy_3_height, 1, 0, 1);
	busyCursor[2] = XCreatePixmapCursor(dsp, pmap, mmap, &ccell_fg,
		&ccell_bg, busy_3_x_hot, busy_3_y_hot);
	XFreePixmap(dsp, mmap);
	XFreePixmap(dsp, pmap);

	mmap = XCreatePixmapFromBitmapData(XtDisplay(bob),
		DefaultRootWindow(XtDisplay(bob)), (char *)busy_4_mask_bits,
		busy_4_mask_width, busy_4_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
		(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
		busy_4_bits, busy_4_width, busy_4_height, 1, 0, 1);
	busyCursor[3] = XCreatePixmapCursor(dsp, pmap, mmap, &ccell_fg,
		&ccell_bg, busy_4_x_hot, busy_4_y_hot);
	XFreePixmap(dsp, mmap);
	XFreePixmap(dsp, pmap);

	mmap = XCreatePixmapFromBitmapData(XtDisplay(bob),
		DefaultRootWindow(XtDisplay(bob)), (char *)busy_5_mask_bits,
		busy_5_mask_width, busy_5_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
		(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
		busy_5_bits, busy_5_width, busy_5_height, 1, 0, 1);
	busyCursor[4] = XCreatePixmapCursor(dsp, pmap, mmap, &ccell_fg,
		&ccell_bg, busy_5_x_hot, busy_5_y_hot);
	XFreePixmap(dsp, mmap);
	XFreePixmap(dsp, pmap);

	mmap = XCreatePixmapFromBitmapData(XtDisplay(bob),
		DefaultRootWindow(XtDisplay(bob)), (char *)busy_6_mask_bits,
		busy_6_mask_width, busy_6_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
		(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
		busy_6_bits, busy_6_width, busy_6_height, 1, 0, 1);
	busyCursor[5] = XCreatePixmapCursor(dsp, pmap, mmap, &ccell_fg,
		&ccell_bg, busy_6_x_hot, busy_6_y_hot);
	XFreePixmap(dsp, mmap);
	XFreePixmap(dsp, pmap);

	mmap = XCreatePixmapFromBitmapData(XtDisplay(bob),
		DefaultRootWindow(XtDisplay(bob)), (char *)busy_7_mask_bits,
		busy_7_mask_width, busy_7_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
		(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
		busy_7_bits, busy_7_width, busy_7_height, 1, 0, 1);
	busyCursor[6] = XCreatePixmapCursor(dsp, pmap, mmap, &ccell_fg,
		&ccell_bg, busy_7_x_hot, busy_7_y_hot);
	XFreePixmap(dsp, mmap);
	XFreePixmap(dsp, pmap);

	mmap = XCreatePixmapFromBitmapData(XtDisplay(bob),
		DefaultRootWindow(XtDisplay(bob)), (char *)busy_8_mask_bits,
		busy_8_mask_width, busy_8_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
		(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
		busy_8_bits, busy_8_width, busy_8_height, 1, 0, 1);
	busyCursor[7] = XCreatePixmapCursor(dsp, pmap, mmap, &ccell_fg,
		&ccell_bg, busy_8_x_hot, busy_8_y_hot);
	XFreePixmap(dsp, mmap);
	XFreePixmap(dsp, pmap);

	mmap = XCreatePixmapFromBitmapData(XtDisplay(bob),
		DefaultRootWindow(XtDisplay(bob)), (char *)busy_9_mask_bits,
		busy_9_mask_width, busy_9_mask_height, 1, 0, 1);
	pmap = XCreatePixmapFromBitmapData
		(XtDisplay(bob), DefaultRootWindow(XtDisplay(bob)),
		busy_9_bits, busy_9_width, busy_9_height, 1, 0, 1);
	busyCursor[8] = XCreatePixmapCursor(dsp, pmap, mmap, &ccell_fg,
		&ccell_bg, busy_9_x_hot, busy_9_y_hot);
	XFreePixmap(dsp, mmap);
	XFreePixmap(dsp, pmap);
}


static void animateCursor()
{
    mo_window *win = NULL;

    cursorAnimCnt++;
    if (cursorAnimCnt >= numCursors)
	cursorAnimCnt = 0;

    XDefineCursor(dsp, XtWindow(toplevel), busyCursor[cursorAnimCnt]);
    while (win = mo_next_window(win)) {
	XDefineCursor(dsp, XtWindow(win->base),
		      busyCursor[cursorAnimCnt]);
	if (win->history_win)
	    XDefineCursor(dsp, XtWindow(win->history_win),
			  busyCursor[cursorAnimCnt]);
	if (win->hotlist_win)
	    XDefineCursor(dsp, XtWindow(win->hotlist_win),
			  busyCursor[cursorAnimCnt]);
	if (win->searchindex_win)
	    XDefineCursor(dsp, XtWindow(win->searchindex_win),
			  busyCursor[cursorAnimCnt]);
    }

    XFlush(dsp);
    busy = 1;
}


/****************************************************************************
 * name:    mo_redisplay_window
 * purpose: Cause the current window's HTML widget to be refreshed.
 *          This causes the anchors to be reexamined for visited status.
 * inputs:  
 *   - mo_window *win: Current window.
 * returns: 
 *   mo_succeed
 * remarks: 
 *   
 ****************************************************************************/
mo_status mo_redisplay_window(mo_window *win)
{
  char *curl = cached_url;

  /* We don't free cached_url because change is only temporary */
  cached_url = win->cached_url;

  HTMLRetestAnchors(win->scrolled_win, anchor_visited_predicate);

  cached_url = curl;

  return mo_succeed;
}


/* ---------------------- mo_set_current_cached_win ----------------------- */

mo_status mo_set_current_cached_win (mo_window *win)
{
  current_win = win;
  view = win->view;

  return mo_succeed;
}


/****************************************************************************
 * name:    mo_view_keypress_handler (PRIVATE)
 * purpose: This is the event handler for the HTML widget and associated
 *          scrolled window; it handles keypress events and enables the
 *          hotkey support.
 * inputs:  
 *   - as per XmxEventHandler
 * returns: 
 *   nothing
 * remarks: 
 *   Hotkeys and their actions are currently hardcoded.  This is probably
 *   a bad idea, and Eric hates it.
 ****************************************************************************/
static void mo_view_keypress_handler(Widget w, XtPointer clid,
				     XEvent *event, Boolean *cont)
{
  mo_window *win = (mo_window *) clid;
  int _bufsize = 3;
  int _count;
  char _buffer[3];
  KeySym _key;
  XComposeStatus _cs;
  Widget sb;
  String params[1];
  static int focus;
  static int catch_pn;
  static int kiosk = 0;
  static int init = 0;

  if (!win)
      return;

  /* Avoid expensive calls */
  if (!init) {
      focus = get_pref_boolean(eFOCUS_FOLLOWS_MOUSE);
      catch_pn = get_pref_boolean(eCATCH_PRIOR_AND_NEXT);
      if (get_pref_boolean(eKIOSK) || get_pref_boolean(eKIOSKNOEXIT))
	  kiosk = 1;
      init = 1;
  }
  /* Go get ascii translation. */
  _count = XLookupString(&(event->xkey), _buffer, _bufsize, 
                         &_key, &_cs);

  /* I don't know why this is necessary but for some reason the rbm was making
   * the above function return 0 as the _key, this fixes it */
  if (!_key)
      _key = XKeycodeToKeysym(XtDisplay(win->view), event->xkey.keycode, 0);
  
  /* Insert trailing Nil. */
  _buffer[_count] = '\0';

  params[0] = "0";
  
  switch (_key) {
  case XK_Prior:  /* Page up. */
  case XK_KP_Prior:
      if (!catch_pn)
	  break;
  case XK_BackSpace:
  case XK_Delete:
      XtVaGetValues(win->scrolled_win, XmNverticalScrollBar, (long)(&sb), NULL);
      if (sb && XtIsManaged(sb))
          XtCallActionProc(sb, "PageUpOrLeft", event, params, 1);
      break;

  case XK_Next:  /* Page down. */
  case XK_KP_Next:
      if (!catch_pn)
	  break;
  case XK_Return:
  case XK_space:
      XtVaGetValues(win->scrolled_win, XmNverticalScrollBar, (long)(&sb), NULL);
      if (sb && XtIsManaged(sb))
          XtCallActionProc(sb, "PageDownOrRight", event, params, 1);
      break;

  case XK_Tab:
      if (!focus) {
	  if (event->xkey.state & ControlMask) {
	      XtSetKeyboardFocus(win->base, win->view);
	      HTMLTraverseTabGroups(win->view, XmTRAVERSE_HOME);
	  } else {
	      HTMLTraverseTabGroups(win->scrolled_win,
				    XmTRAVERSE_NEXT_TAB_GROUP);
	  }
      }
      break;
      
  case XK_Home:  /* Home -- Top */
      HTMLGotoId(win->scrolled_win, 0, 0);
      break;

  case XK_End:  /* End -- Bottom */
      HTMLGotoId(win->scrolled_win, HTMLLastId(win->scrolled_win), 0);
      break;

  case XK_Down:
  case XK_KP_Down:
      XtVaGetValues(win->scrolled_win, XmNverticalScrollBar, (long)(&sb), NULL);
      if (sb && XtIsManaged(sb))
          XtCallActionProc(sb, "IncrementDownOrRight", event, params, 1);
      break;

  case XK_Right:
  case XK_KP_Right:
      params[0] = "1";
      XtVaGetValues(win->scrolled_win, XmNhorizontalScrollBar, 
                    (long)(&sb), NULL);
      if (sb && XtIsManaged(sb))
          XtCallActionProc(sb, "IncrementDownOrRight", event, params, 1);
      break;

  case XK_Up:
  case XK_KP_Up:
      XtVaGetValues(win->scrolled_win, XmNverticalScrollBar, (long)(&sb), NULL);
      if (sb && XtIsManaged(sb))
          XtCallActionProc(sb, "IncrementUpOrLeft", event, params, 1);
      break;

  case XK_Left:
  case XK_KP_Left:
      params[0] = "1";
      XtVaGetValues(win->scrolled_win, XmNhorizontalScrollBar,
		    (long)(&sb), NULL);
      if (sb && XtIsManaged(sb))
          XtCallActionProc(sb, "IncrementUpOrLeft", event, params, 1);
      break;
  }

  if (!kiosk && win->hotkeys) {
      switch (_key) {
              /* News Hotkeys ...
                 < > = prev/next thread  , . = prev/next message */
      case XK_less:
          gui_news_prevt(win);
          break;

      case XK_greater:
          gui_news_nextt(win);
          break;

      case XK_comma:
          gui_news_prev(win);
	  break;

      case XK_period:
          gui_news_next(win);
          break;

      case XK_A:  /* Annotate */
      case XK_a: 
          mo_post_annotate_win(win, 0, 0, NULL, NULL, NULL, NULL);
          break;
          
      case XK_B:  /* Back */
      case XK_b:
          mo_back_node(win);
          break;

      case XK_C:  /* Clone */
      case XK_c:
          mo_duplicate_window(win);
          break;

      case XK_D:  /* Document source. */
      case XK_d:
          mo_post_source_window(win);
          break;

      case XK_E:  /* Edit */
      case XK_e:
          mo_edit_source(win);
          break;

      case XK_F:
      case XK_f:
	if (event->xkey.state & ControlMask) {
	    if (XtIsManaged(win->slab[SLAB_URL]) && !focus) {
		XmTextFieldSetString(win->url_text, "ftp://");
		XtSetKeyboardFocus(win->base, win->url_text);
		XmTextSetInsertionPosition(win->url_text, 7);
	    }
	} else {
            mo_forward_node(win);
	}
        break;
  
      case XK_H:  /* Hotlist */
          mo_post_hotlist_win(win);
          break;

      case XK_h:  /* History */
	if (event->xkey.state & ControlMask) {
	    if (XtIsManaged(win->slab[SLAB_URL]) && !focus) {
		XmTextFieldSetString(win->url_text, "http://");
		XtSetKeyboardFocus(win->base, win->url_text);
		XmTextSetInsertionPosition(win->url_text, 8);
	    }
	} else {
            mo_post_history_win(win);
	}
	break;

      case XK_L:  /* Open Local */
      case XK_l:
          mo_post_open_local_window(win);
          break;
  
      case XK_M:  /* Mailto */
      case XK_m:
          mo_post_mail_window(win);
          break;

      case XK_N:  /* New */
      case XK_n:
	if (event->xkey.state & ControlMask) {
	    if (XtIsManaged(win->slab[SLAB_URL]) && !focus) {
		XtSetKeyboardFocus(win->base, win->url_text);
		XmTextFieldSetString(win->url_text, "news:");
		XmTextSetInsertionPosition(win->url_text, 8);
	    }
	} else {
	    mo_open_another_window(win, home_document, NULL, NULL);
	}
	break;
  
      case XK_O:  /* Open */
      case XK_o:
          mo_post_open_window(win);
          break;

      case XK_P:  /* Print */
      case XK_p:
/*
	  if (event->xkey.state & ControlMask) {
		mo_presentation_mode(win);
	  } else
*/
	  mo_post_print_window(win);
          break;

      case XK_r: /* Reload */
          mo_reload_window_text(win, 0);
          break;
   
      case XK_R:  /* Refresh */
          mo_refresh_window_text(win);
          break;
          
      case XK_S:  /* Search. */
      case XK_s:
          mo_post_search_window(win);
          break;
          
	/* Tag 'n Bag */
#ifdef NOT_USED
      case XK_T:
      case XK_t:
	  mo_tagnbag_url(win);
          break;

      /* Not active */
      case XK_U:  /* Upload a file (method of put) */
      case XK_u:
	  mo_post_upload_window(win);
	  break;
#endif
      case XK_Z:
      case XK_z:
	if (XtIsManaged(win->slab[SLAB_URL]) && !focus) {
	    char *str;
	    
	    str = XmTextFieldGetString(win->url_text);
	    XmTextFieldSetSelection(win->url_text, 0, strlen(str), 
				    event->xkey.time);
	    XtSetKeyboardFocus(win->base, win->url_text);
	    XtSetKeyboardFocus(win->base, win->url_text);
	    XtFree(str);
	}
	break;

      case XK_Escape:
          mo_delete_window(win);
          break;

      default:
          break;
      }
  } else if (win->hotkeys) {
      /* Kiosk mode */
      switch (_key) {
	case XK_B:  /* Back */
	case XK_b:
          mo_back_node(win);
          break;

	case XK_F:  /* Forward */
	case XK_f:
          mo_forward_node(win);
          break;

        default:
          break;
      }
  }
  
  return;
}


/* Stop animations.  If check_drawing is 1, then we do not stop
 * animations in a window being drawn */
void mo_stop_animations(mo_window *win, int check_drawing)
{
  HTMLWidget hw = (HTMLWidget) win->scrolled_win;

  /* Stop animations unless we are still drawing the window */
  if (!check_drawing || !hw->html.drawing) {
      hw->html.draw_count++;
  } else {
      return;
  }

  /* Stop frame animations */
  win = win->frames;
  while (win) {
      FrameInfo *fptr = hw->html.iframe_list;

      hw = (HTMLWidget) win->scrolled_win;
      hw->html.draw_count++;
      /* IFRAME */
      while (fptr) {
	  if (fptr->iframe) {
	      HTMLWidget frame = (HTMLWidget) fptr->iframe;

	      frame->html.draw_count++;
	  }
	  fptr = fptr->frame_next;
      }
      win = win->next_frame;
  }
}


/* Stop loading and/or animations */
void mo_stop_it(mo_window *win)
{
  sleep_interrupt = connect_interrupt = 1;

#ifdef CCI
  if (cci_event)
      MoCCISendEventOutput(MOSAIC_GLOBE);
#endif

  mo_stop_animations(win, 1);
}


XmxCallback(icon_pressed_cb)
{
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));

  mo_stop_it(win);

  return;
}


static XmxCallback(security_pressed_cb)
{
    mo_window *win = current_win;
    char buf[BUFSIZ];

    if (!win || !win->current_node)
	return;
#ifdef CCI
    if (cci_event)
	MoCCISendEventOutput(AUTHENTICATION_BUTTON);
#endif
    mo_gui_check_security_icon(win->current_node->authType);

    switch(win->current_node->authType) {
	case HTAA_NONE:
	    strcpy(buf, "There is no authentication for this URL.");
	    break;
	case HTAA_BASIC:
	    strcpy(buf,
		   "The authentication method for this URL is\nBasic (uuencode/uudecode).");
	    break;
	case HTAA_KERBEROS_V4:
	    strcpy(buf,
		   "The authentication method for this URL is\nKerberos v4.");
	    break;
	case HTAA_KERBEROS_V5:
	    strcpy(buf,
		   "The authentication method for this URL is\nKerberos v5.");
	    break;
	case HTAA_MD5:
	    strcpy(buf, "The authentication method for this URL is\nMD5.");
	    break;
	case HTAA_DOMAIN:
	    strcpy(buf, "This URL is Domain Restricted.");
	    break;
	case HTAA_LOGIN:
	    strcpy(buf,
		   "This FTP URL is authenticated through logging into the\nFTP server machine.");
	    break;
	case HTAA_UNKNOWN:
	default:
	    strcpy(buf,
		   "The authentication method for this URL is unknown.\nA default of no authentication was used, which was okayed\nby the server.");
	    break;
    }

    application_user_info_wait(buf);

    return;
}


static XmxCallback(encrypt_pressed_cb)
{
    mo_window *win = current_win;
    char buf[BUFSIZ];

    if (!win || !win->current_node)
	return;
#ifdef CCI
    if (cci_event)
	MoCCISendEventOutput(ENCRYPT_BUTTON);
#endif

#ifdef HAVE_SSL
    if (win->current_node->cipher) {
	sprintf(buf,
	    "URL used secure connection with %d bit encryption cipher:\n                %s",
	    win->current_node->cipher_bits, win->current_node->cipher);
    } else {
	strcpy(buf, "A secure connection was not used for this URL.");
    }
#else
    strcpy(buf, "This copy of Mosaic does not include support for encryption.");
#endif

    application_user_info_wait(buf);

    return;
}


/* ----------------------- editable URL field callback -------------------- */
/* If the user hits return in the URL text field at the top of the display, */
/* then go fetch that URL                                                   */

static XmxCallback(url_field_cb)
{
  mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
  char *url, *xurl;

  if (!get_pref_boolean(eFOCUS_FOLLOWS_MOUSE))
    XtSetKeyboardFocus(win->base, win->view);

#ifdef CCI
  if (cci_event)
    MoCCISendEventOutput(MOSAIC_URL_TEXT_FIELD);
#endif

  url = mo_clean_and_escape_url(XmxTextGetString(win->url_text), 1);
  if (!url)
    return;
  if (!*url) {
    free(url);
    return;
  }
  xurl = mo_url_prepend_protocol(url);

  if (max_kiosk_protocols && !kioskSafe(url)) {
    fprintf(stderr, "kioskMode: Protocol not specified as 'safe'.\n");
    application_user_info_wait("kioskMode: Protocol not specified as 'safe'.");
  } else {  /* V2.7b6 fix */
    mo_load_window_text(win, xurl, NULL);
#ifdef CCI
    if (cci_event)
      MoCCISendEventOutput(LINK_LOADED);
#endif
  }

  free(xurl);
  free(url);
  return;
}


/* Public anchor load to disk function -- hack for RBM ltd support */
void pub_anchor_ltd(char *linkurl)
{
  char *href, *reftext;
  static char *referer = NULL;
  mo_window *win = current_win;
  int old_binx_flag;

  if (!win)
    return;

  old_binx_flag = win->binary_transfer;
  win->binary_transfer = 1;

  if (get_pref_boolean(eKIOSK) || 
      get_pref_boolean(eKIOSKNOEXIT) || 
      get_pref_boolean(eDISABLEMIDDLEBUTTON))
    /* Disable load to disk in kiosk */
    win->binary_transfer = 0;

  if (get_pref_boolean(ePROTECT_ME_FROM_MYSELF)) {
    int answer = XmxModalYesOrNo(win->base, app_context,
  /*
   * VAX C has a line length limitation which can be ifdefed around
   */
"BEWARE: absolutely anything could be on the other end of this hyperlink,\nincluding pornography or even nudity.\n\n\
Mosaic disclaims all responsibility regarding your emotional and mental health\nand specifically all responsibility for effects of viewing salacious material via Mosaic.\n\n\
With that in mind, are you *sure* you want to follow this hyperlink??",
         "Yup, I'm sure, really.", "Ack, no!  Get me outta here.");

    if (!answer)
      return;
  }

  /* Free previous one, if any */
  if (referer) {
    free(referer);
    referer = NULL;
  }

  if (!my_strncasecmp(win->current_node->url, "http://", 7) ||
      !my_strncasecmp(win->current_node->url, "https://", 8)) {
    /* What if hostname is a partial local? */
    referer = strdup(win->current_node->url);
    HTReferer = referer;
  } else {
    HTReferer = NULL;
  }
  
  if (linkurl) {
    href = mo_clean_and_escape_url(linkurl, 0);
  } else {
    href = strdup("Unlinked");
  }
/*
  if (((WbAnchorCallbackData *)call_data)->text)
    reftext = strdup(((WbAnchorCallbackData *)call_data)->text);
  else
*/
  reftext = strdup("Untitled");

  mo_load_window_text(win, href, reftext);

  win->binary_transfer = old_binx_flag;
  free(href);
  free(reftext);
  return;
}


static mo_window *find_frame(mo_window *win, char *name)
{
    mo_window *frame;

    while (win) {
        if (win->frames) {
            frame = find_frame(win->frames, name);
	    if (frame)
	        return frame;
        }
        if (!strcmp(name, win->framename))
	    return win;
        win = win->next_frame;
    }
    return NULL;
}


/****************************************************************************
 * name:    anchor_cb
 * purpose: Callback for triggering anchor in HTML widget.
 * inputs:  
 *   - as per XmxCallback
 * returns: 
 *   nothing
 * remarks: 
 *   This is too complex and should be broken down.
 *   We look at the button event passed in through the callback;
 *   button1 == same window, button2 == new window.
 *   Call mo_open_another_window or mo_load_window_text to get
 *   the actual work done.
 ****************************************************************************/
static XmxCallback(anchor_cb)
{
  char *href, *reftext;
  char *cb_href = ((WbAnchorCallbackData *)call_data)->href;
  char *frame = ((WbAnchorCallbackData *)call_data)->frame;
  mo_window *win = (mo_window *) client_data;
  mo_window *parent, *target, *ori_win;
  XButtonReleasedEvent *event = 
           (XButtonReleasedEvent *)(((WbAnchorCallbackData *)call_data)->event);
  int force_newwin = (event->button == Button2 ? 1 : 0);
  int old_binx_flag;
  int reload = 0;
  int refresh = ((WbAnchorCallbackData *)call_data)->refresh;
  static int protect = -1;
  static int kiosk = -1;
  static char *referer = NULL;

  if (!win)
      return;

  /* Get top level mo_window */
  ori_win = parent = target = win;
  while (parent->is_frame)
      parent = parent->parent;

  /* Don't do refresh if user doesn't want them */
  if (refresh && !parent->refresh_url)
      return;

  /* Locate target frame */
  if (frame && *frame) {
      if (!strcmp(frame, "_top") || !strcmp(frame, "_blank")) {
	  /* No longer doing frames */
	  win = parent;
      } else if (!strcmp(frame, "_self")) {
	  /* This is the default */ 
      } else if (!strcmp(frame, "_parent")) {
	  /* Should be frameset of this frame */
	  win = parent;
      } else {
	  target = find_frame(parent->frames, frame);
	  if (!target)
	      win = parent;
      }
  }

#ifdef CCI
  if (cci_event)
      MoCCISendEventOutput(MOSAIC_URL_TRIGGER);
#endif

  /* If shift was down, make this a Load to Local Disk */
  old_binx_flag = parent->binary_transfer;
  if ((event->state & ShiftMask) == ShiftMask)
      parent->binary_transfer = 1;

  if (kiosk) {	/* Avoid bunch of routine calls */
      if ((kiosk == 1) || get_pref_boolean(eKIOSK) || 
	  get_pref_boolean(eKIOSKNOEXIT) || 
	  get_pref_boolean(eDISABLEMIDDLEBUTTON)) { 
 	  /* Disable new window if in kiosk mode */
	  force_newwin = 0;
	  /* Disable load to disk in kiosk */
	  parent->binary_transfer = 0;
	  kiosk = 1;
      } else {
	  kiosk = 0;
      }
  }

  if (protect == -1)
      protect = get_pref_boolean(ePROTECT_ME_FROM_MYSELF);
  if (protect) {
      int answer = XmxModalYesOrNo(parent->base, app_context,
  /*
   * VAX C has a line length limitation
   */
"BEWARE: absolutely anything could be on the other end of this hyperlink,\nincluding pornography or even nudity.\n\n\
Mosaic disclaims all responsibility regarding your emotional and mental health\nand specifically all responsibility for effects of viewing salacious material via Mosaic.\n\n\
With that in mind, are you *sure* you want to follow this hyperlink??",
         "Yup, I'm sure, really.", "Ack, no!  Get me outta here.");

      if (!answer)
          return;
  }

  if (referer) {
      free(referer);
      referer = NULL;
  }
  
  if (cb_href) {
      if (refresh) {
          if (!ori_win->is_frame) {
              href = mo_url_canonicalize_keep_anchor(cb_href,
						     parent->current_node->url);
          } else {
              href = mo_url_canonicalize_keep_anchor(cb_href,ori_win->frameurl);
          }
      } else {
          if (!ori_win->is_frame) {
              href = strdup(cb_href);
          } else {
              href = mo_url_canonicalize_keep_anchor(cb_href,
						     ori_win->cached_url);
          }
      }
      href = mo_clean_and_escape_url(href, 1);
  } else {
      href = strdup("Unlinked");
  }

  if (((WbAnchorCallbackData *)call_data)->text) {
      reftext = strdup(((WbAnchorCallbackData *)call_data)->text);
  } else {
      reftext = strdup("Untitled");
  }
  if (!ori_win->is_frame) {
      if (!my_strncasecmp(parent->current_node->url, "http://", 7) ||
          !my_strncasecmp(parent->current_node->url, "https://", 8)) {
          /* What if hostname is a partial local? */
          referer = strdup(parent->current_node->url);
          HTReferer = referer;
      } else {
          HTReferer = NULL;
          if (!my_strncasecmp(parent->current_node->url, "cookiejar:", 10)) {
	      reload = 1;
	      /* Get new Cookiejar URL */
	      free(parent->current_node->url);
	      parent->current_node->url = strdup(href);
          }
      }
  } else {
      if (!my_strncasecmp(ori_win->frameurl, "http://", 7) ||
          !my_strncasecmp(ori_win->frameurl, "https://", 8)) {
          referer = strdup(ori_win->frameurl);
          HTReferer = referer;
      }
  }

  if (!force_newwin) {
    if (max_kiosk_protocols && !kioskSafe(href)) {
      fprintf(stderr, "kioskMode: Protocol not specified as 'safe'.\n");
      application_user_info_wait
	  ("kioskMode: Protocol not specified as 'safe'.");
#ifdef CCI
      if (cci_event)
	MoCCISendEventOutput(LINK_LOADED);
#endif
      parent->binary_transfer = old_binx_flag;
      free(href);
      free(reftext);

      return;
    }
    /* Just reload current window if called by refresh timer and URL
     * is same as current one, otherwise load specified URL unless
     * called by refresh timer and URL is not refreshable */
    if (win->is_frame) {
      if (reload || (refresh && !my_strcasecmp(href, win->frameurl))) {
        mo_reload_frame_text(win, parent);
      } else if (!refresh || parent->refreshable) {
        target->new_node = 1;
        parent->do_frame = target;
	/* Keep new URL with frame info */
	if (target->frameurl)
	  free(target->frameurl);
	target->frameurl = strdup(href);
        mo_load_window_text(parent, href, reftext);
        target->new_node = 0;
        parent->do_frame = NULL;
	if (target != win)
	  /* Redisplay window with the anchor so it is marked visited */
          mo_redisplay_window(win);
      }
    } else {
      if (reload || (refresh && !my_strcasecmp(href, win->current_node->url))) {
        mo_reload_window_text(win, 0);
      } else if (!refresh || win->refreshable) {
        mo_load_window_text(win, href, reftext);
      }
    }
  } else {
      char *anchor = mo_url_extract_anchor(href);
      char *url;

      if (!ori_win->is_frame) {
	url = mo_url_canonicalize_keep_anchor(href, parent->current_node->url);
      } else {
	url = mo_url_canonicalize_keep_anchor(href, "");
      }
      if (max_kiosk_protocols && !kioskSafe(url)) {
        fprintf(stderr, "kioskMode: Protocol not specified as 'safe'.\n");
        application_user_info_wait(
		"kioskMode: Protocol not specified as 'safe'.");
#ifdef CCI
	if (cci_event)
	  MoCCISendEventOutput(LINK_LOADED);
#endif
	parent->binary_transfer = old_binx_flag;
	free(href);
        free(reftext);
	free(url);
	free(anchor);

        return;
      }

      /* Should we be keeping the anchor here??? */
      if (my_strncasecmp(url, "telnet:", 7) &&
          my_strncasecmp(url, "tn3270:", 7) &&
          my_strncasecmp(url, "rlogin:", 7)) {
        /* Not a telnet anchor. */

        /* Open the window (generating a new cached_url). */
        mo_open_another_window(parent, url, reftext, anchor);

        /* Now redisplay this window. */
        mo_redisplay_window(win);
      } else {
        /* Just do mo_load_window_text to get the xterm forked off. */
	/* Include frame so correct window will be refreshed */
        if (win->is_frame) {
          parent->do_frame = win;
          mo_load_window_text(parent, url, reftext);
          parent->do_frame = NULL;
        } else {
          mo_load_window_text(win, url, reftext);
        }
	free(anchor);
      }
      free(url);
    }

#ifdef CCI
  if (cci_event)
    MoCCISendEventOutput(LINK_LOADED);
#endif
  parent->binary_transfer = old_binx_flag;
  free(href);
  free(reftext);
  return;
}


/****************************************************************************
 * name:    anchor_visited_predicate (PRIVATE)
 * purpose: Called by the HTML widget to determine whether a given URL
 *          has been previously visited.
 * inputs:  
 *   - Widget   w: HTML widget that called this routine.
 *   - char *href: URL to test.
 * returns: 
 *   1 if href has been visited previously; 0 otherwise.
 * remarks: 
 *   All this does is canonicalize the URL and call
 *   mo_been_here_before_huh_dad() to figure out if we've been
 *   there before.
 ****************************************************************************/
int anchor_visited_predicate(Widget w, char *href)
{
  int rv;
  static int checkv = -1;
  static int checkt;

  if (checkv == -1) {	/* Avoid repeating routine call */
      checkv = get_pref_boolean(eTRACK_VISITED_ANCHORS);
      checkt = get_pref_boolean(eTRACK_TARGET_ANCHORS);
  }
  if (!checkv || !href)
      return 0;

  if (checkt) {
      href = mo_url_canonicalize_keep_anchor(href, cached_url);
  } else {
      href = mo_url_canonicalize(href, cached_url);
  }
  href = mo_clean_and_escape_url(href, 1);

  rv = (mo_been_here_before_huh_dad(href) == mo_succeed ? 1 : 0);

  free(href);
  return rv;
}

static void pointer_motion_callback(Widget w, XtPointer clid, XtPointer calld)
{
  char *href = (char *) calld;
  mo_window *win = (mo_window *) clid;
  HTMLWidget hw = (HTMLWidget) w;
  mo_window *parent;
  XmString xmstr;
  char *to_free = NULL;
  char *title;
  static int checkm = -1;
  static int checkn;

  if (checkm == -1) {	/* Avoid repeating routine call */
      checkm = get_pref_boolean(eTRACK_POINTER_MOTION);
      checkn = get_pref_boolean(eTRACK_FULL_URL_NAMES);
  }
  if (!checkm)
      return;

  if (hw->html.title_elem) {
      title = hw->html.title_elem->title;
  } else {
      title = NULL;
  }

  parent = win;
  /* Get top level mo_window */
  while (parent->is_frame)
      parent = parent->parent;

  if (href && *href) {
      if (!my_strncasecmp("cookiejar:", href, 10)) {
	  href += 10;
	  if (!my_strncasecmp("disable", href, 7)) {
	      title = href = "Disable cookie support?";
	  } else if (!my_strncasecmp("enable", href, 6)) {
	      title = href = "Enable cookie support?";
	  } else if (!my_strncasecmp("eatall", href, 6)) {
	      title = href = "Accept all cookies without prompting?";
	  } else if (!my_strncasecmp("noeatall", href, 8)) {
	      title = href = "Prompt before accepting each cookie?";
	  } else if (!my_strncasecmp("file", href, 4)) {
	      title = href = "Enable the cookie file?";
	  } else if (!my_strncasecmp("nofile", href, 6)) {
	      title = href = "Disable the cookie file?";
	  } else if (!my_strncasecmp("save", href, 4)) {
	      title = href = "Update the cookie file?";
	  } else if (!my_strncasecmp("refresh_jar", href, 11)) {
	      title = href = "Refresh the Cookie Jar.";
	  } else {
	      href = " ";
	  }
      } else {
	  href = mo_url_canonicalize_keep_anchor(href, win->cached_url);
	  to_free = href;

	  /* Sigh... */
	  if (mo_convert_newlines_to_spaces(href))
	      removeblanks(href);

	  /* This is now the option wherein the URLs are just spit up there;
             else we put up something more friendly. */
          if (!checkn) {
              /* This is where we go get a good description. */
              href = HTDescribeURL(href);
	      if (to_free)
	          free(to_free);
              to_free = href;
          }
      }
  } else {
      href = " ";
  }

  /* Display it on status line */
  xmstr = XmStringCreateSimple(href);
  XtVaSetValues(parent->tracker_label, XmNlabelString, (XtArgVal)xmstr, NULL);
  XmStringFree(xmstr);

  if (XmxClueIsActive()) {
      if (title) {
	  /* Popup at then current mouse location */
	  XmxClueOncePopup(w, title, 0, 0, True);
      } else {
	  /* Okay to call even if one not there */
	  XmxClueOncePopdown();
      }
  }

  if (to_free)
      free(to_free);
  
  return;
}


static XmxCallback(submit_form_callback)
{
  mo_window *win = (mo_window *) client_data;
  mo_window *parent, *target;
  char *url = NULL;
  char *method = NULL;
  char *enctype = NULL;
  char *query;
  int len, i;
  WbFormCallbackData *cbdata = (WbFormCallbackData *)call_data;
  char *frame = ((WbFormCallbackData *)call_data)->target;
  int do_post_urlencoded = 0;
  int plaintext = 0;
  static char *referer = NULL;

  if (!cbdata || !win)
      return;

  /* Get top level mo_window */
  parent = target = win;
  while (parent->is_frame)
      parent = parent->parent;

  /* Locate target frame */
  if (frame && *frame) {
      if (!strcmp(frame, "_top") || !strcmp(frame, "_blank")) {
	  /* No longer doing frames */
	  win = parent;
      } else if (!strcmp(frame, "_self")) {
	  /* This is the default */ 
      } else if (!strcmp(frame, "_parent")) {
	  /* Should be frameset of this frame */
	  win = parent;
      } else {
	  target = find_frame(parent->frames, frame);
	  if (!target)
	      win = parent;
      }
  }

  if (referer) {
      free(referer);
      referer = NULL;
  }
  if (!win->is_frame) {
      if (!my_strncasecmp(win->current_node->url, "http://", 7) ||
          !my_strncasecmp(win->current_node->url, "https://", 8)) {
          referer = strdup(win->current_node->url);
          HTReferer = referer;
      } else {
          HTReferer = NULL;
      }
  } else {
      if (!my_strncasecmp(win->frameurl, "http://", 7) ||
          !my_strncasecmp(win->frameurl, "https://", 8)) {
          referer = strdup(win->frameurl);
          HTReferer = referer;
      } else {
          HTReferer = NULL;
      }
  }

#ifdef CCI
  if (cci_event)
      MoCCISendEventOutput(FORM_SUBMIT);
#endif
  /* Initial query: Breathing space. */
  len = 16;

  /* Add up lengths of strings. */
  for (i = 0; i < cbdata->attribute_count; i++) {
      if (cbdata->attribute_names[i]) {
          len += strlen(cbdata->attribute_names[i]) * 3;
          if (cbdata->attribute_values[i])
              len += strlen(cbdata->attribute_values[i]) * 3;
      }
      len += 2;
  }

  /* Get the URL. */
  if (cbdata->href && *cbdata->href) {
      url = cbdata->href;
  } else if (!win->is_frame) {
      url = win->current_node->url;
  } else {
      url = win->frameurl;
  }

  if (cbdata->method && *cbdata->method) {
      method = cbdata->method;
  } else {
      method = strdup("GET");
  }
  /* Grab enctype if it's there. */
  if (cbdata->enctype && *cbdata->enctype)
      enctype = cbdata->enctype;

#ifndef DISABLE_TRACE
  if (srcTrace)
      fprintf(stderr, "[submit_form_callback] method is '%s'\n", method);
#endif

  if (!my_strcasecmp(method, "POST")
#ifdef CCI
      || !my_strcasecmp(method, "cciPOST")
#endif
     )
      do_post_urlencoded = 1;

  len += strlen(url);

  query = (char *)malloc(sizeof(char) * len);

  if (!do_post_urlencoded) {
      strcpy(query, url);
      /* Clip out anchor. */
      strtok(query, "#");
      /* Clip out old query. */
      strtok(query, "?");
      if (query[strlen(query) - 1] != '?')
	  strcat(query, "?");
      plaintext = 0;
  } else {
      /* Get ready for cats below. */
      query[0] = 0;
      plaintext = 0;
  }

  /* Take isindex possibility into account. */
  if (cbdata->attribute_count == 1 &&
      !strcmp(cbdata->attribute_names[0], "isindex")) {
      if (cbdata->attribute_values[0]) {
          char *c = mo_escape_part(cbdata->attribute_values[0]);

          strcat(query, c);
          free(c);
      }
  } else {
      for (i = 0; i < cbdata->attribute_count; i++) {
          /* For all but the first, lead off with an ampersand. */
          if (i)
              strcat(query, "&");
          /* Rack 'em up. */
          if (cbdata->attribute_names[i]) {
              char *c = mo_escape_part(cbdata->attribute_names[i]);

              strcat(query, c);
              free(c);
              strcat(query, "=");

              if (cbdata->attribute_values[i]) {
                  char *c = mo_escape_part(cbdata->attribute_values[i]);

                  strcat(query, c);
                  free(c);
              }
          }
      }
  }

  if (win->is_frame) {
      target->new_node = 1;
      parent->do_frame = target;
      /* Keep new URL with frame info */
      if (target->frameurl)
          free(target->frameurl);
  }
  if (do_post_urlencoded) {
#ifdef CCI
      if (!my_strcasecmp(method, "cciPOST"))
	  MoCCIFormToClient(NULL, NULL, NULL, NULL, 1);
#endif
      if (win->is_frame)
          target->frameurl =
	      mo_url_canonicalize_keep_anchor(url, win->cached_url);
      mo_post_load_window_text(parent, url, (plaintext ? "text/plain" :
			       "application/x-www-form-urlencoded"), query);
  } else {
      if (win->is_frame) {
          target->frameurl =
	      mo_url_canonicalize_keep_anchor(query, win->cached_url);
	  mo_load_window_text(parent, query, NULL);
      } else {
          mo_access_document(win, query);
      }
  }
  if (win->is_frame) {
      target->new_node = 0;
      parent->do_frame = NULL;
  }
  
  if (query)
      free(query);

  return;
}


static XmxCallback(title_callback)
{
  mo_window *win = (mo_window *)client_data;
  char *title = (char *)call_data;
  static int init = 0;
  static int iswindowtitle, useiconbar;

  if (!title)
      return;

  if (!init) {
      iswindowtitle = get_pref_boolean(eTITLEISWINDOWTITLE);
      useiconbar = get_pref_boolean(eUSEICONBAR);
      init = 1;
  }

#ifndef DISABLE_TRACE
  if (srcTrace)
      fprintf(stderr, "Title callback with title = %s\n", title);
#endif

  /* Frames don't have titles displayed */
  if (win->is_frame) {
#ifndef DISABLE_TRACE
      if (srcTrace)
          fprintf(stderr, "Title callback in frame\n");
#endif
      return;
  }

  /* Make it presentable */
  mo_convert_newlines_to_spaces(title);
  /* Clean any spaces off the end */
  my_chop(title);

  if ((iswindowtitle || useiconbar) && win && win->base) {
      char *buf = (char *)malloc(strlen(pre_title) + strlen(title) + 15);

      if (!buf) {
	  perror("Title Buffer");
	  return;
      }
      if (*title) {
          sprintf(buf, "%s [%s", pre_title, title);
          strcat(buf, "]");
      } else {
          sprintf(buf, "%s", pre_title);
      }
      XtVaSetValues(win->base, XmNtitle, buf, NULL);
      free(buf);
  }
  return;
}


/* This only gets called by the widget in the middle of HTMLSettext. */
static XmxCallback(base_callback)
{
  mo_window *win = (mo_window *)client_data;
  char *href = (char *)call_data;
  extern char *url_base_override;

  if (!*href)
      return;

  url_base_override = strdup(href);

  /* Set the URL cache variables to the correct values NOW. */
  if (cached_url)
      free(cached_url);
  cached_url = mo_url_canonicalize(url_base_override, "");
  if (win->cached_url)
      free(win->cached_url);
  win->cached_url = strdup(cached_url);

  return;
}

void mo_delete_frames(mo_window *win)
{
    mo_window *tmp;
    HTMLWidget hw;

    while (win) {
        /* Recursively destroy frames */
        if (win->frames)
	    mo_delete_frames(win->frames);

        /* Free up HTMLWidget stuff */
        hw = (HTMLWidget) win->scrolled_win;
        HTMLFreeWidget(hw);
        /* Keep XtSetSensitive from causing SetValues routine to mess around */
        hw->html.ignore_setvalues = 1;
        /* Keep pointer callbacks and other events from happening */
        XtSetSensitive(win->scrolled_win, False);
        /* Get it off the screen */
        XtUnmanageChild(win->scrolled_win);
        XtRemoveAllCallbacks(win->scrolled_win, WbNpointerMotionCallback);
        XtRemoveAllCallbacks(win->scrolled_win, WbNimageCallback);
        XtRemoveAllCallbacks(win->scrolled_win, WbNimageQuantizeCallback);
        XtRemoveAllCallbacks(win->scrolled_win, WbNanchorCallback);
        XtRemoveAllCallbacks(win->scrolled_win, WbNbaseCallback);
        XtRemoveAllCallbacks(win->scrolled_win, WbNtitleCallback);
        XtRemoveAllCallbacks(win->scrolled_win, WbNsubmitFormCallback);
        XtRemoveAllCallbacks(win->scrolled_win, WbNframeCallback);

        if (win->framename)
	    free(win->framename);
        if (win->cached_url)
	    free(win->cached_url);
        if (win->frameurl)
	    free(win->frameurl);
        if (win->target_anchor)
	    free(win->target_anchor);

        tmp = win;
        win = win->next_frame;
        free(tmp);
    }
    return;
}

static void frame_cb(Widget w, XtPointer clid, XtPointer calld)
{
  FrameCbData *frame_info = (FrameCbData *) calld;
  mo_window *parent = (mo_window *) clid;
  HTMLWidget hw = (HTMLWidget) w;
  HTMLWidget nw;
  mo_window *next, *win, *top, *ori_frames;
  FrameInfo *frame;
  mo_frame *node;
  cached_frame_data *cached_data;
  int level = 1;
  int i;
  int niframe = 0;

#ifndef DISABLE_TRACE
  if (srcTrace)
    fprintf(stderr, "Got Frame Callback, type = ");
#endif

  /* Dealloc and destroy previous frames */
  if (frame_info->reason == FRAME_DELETE) {
#ifndef DISABLE_TRACE
    if (srcTrace)
      fprintf(stderr, "FRAME_DELETE\n");
#endif
    mo_delete_frames(parent->frames);
    parent->frames = NULL;
    return;
  }

  /* Get top level mo_window */
  top = parent;
  while (top->is_frame) {
    level++;
    top = top->parent;
  }

  if (frame_info->reason != IFRAME_CREATE) {

#ifndef DISABLE_TRACE
    if (srcTrace)
      fprintf(stderr, "FRAME_CREATE\n");
#endif
    parent->frames = win = (mo_window *)calloc(1, sizeof(mo_window));

  } else {

#ifndef DISABLE_TRACE
    if (srcTrace)
      fprintf(stderr, "IFRAME_CREATE\n");
#endif
    if (hw->html.frames[0]->cw_only) {
      /* When calculating height only, get other frames out of way */
      ori_frames = parent->frames;
      parent->frames = win = (mo_window *)calloc(1, sizeof(mo_window));
    } else if (!parent->frames) {
      /* First one */
      parent->frames = win = (mo_window *)calloc(1, sizeof(mo_window));
    } else {
      /* Not first one */
      win = parent->frames;
      niframe++;
      while (win->next_frame) {
	niframe++;
	win = win->next_frame;
      }
      next = win;
      win = (mo_window *)calloc(1, sizeof(mo_window));
      next->next_frame = win;
    }
  }

  for (i = 0; i < hw->html.nframe; i++) {
    int width;
    int height;
    int x, y;

    if (i > 0) {
      win = (mo_window *)calloc(1, sizeof(mo_window));
      next->next_frame = win;
    }
    win->next_frame = NULL;
    next = win;
    win->new_node = False;
    win->parent = parent;
    win->frames = NULL;
    win->frameurl = NULL;
    win->cached_url = NULL;
    win->target_anchor = NULL;

    win->id = XmxMakeNewUniqid();
    frame = hw->html.frames[i];

    x = frame->frame_x;
    y = frame->frame_y;
    width = frame->frame_width;
    height = frame->frame_height;

    if (frame_info->reason != IFRAME_CREATE) {
      win->is_frame = 1;
      width -= frame->frame_border;
      height -= frame->frame_border;
      x += 2;
      y += 2;
    } else {
      /* IFRAME */
      win->is_frame = 2;
      if (frame->frame_border) {
        width -= 2 * frame->frame_border;
        height -= 2 * frame->frame_border;
        x += 2;
        y += 2;
      }
    }
    win->framename = strdup(frame->frame_name);
#ifndef DISABLE_TRACE
    if (srcTrace)
      fprintf(stderr, "Creating frame with W=%d H=%d at X=%d Y=%d\n",
	      frame->frame_width, frame->frame_height, frame->frame_x,
	      frame->frame_y);
#endif
    if (top->new_node) {	/* We are just moving around in the history */
      int j;

      if (frame_info->reason != IFRAME_CREATE) {
	j = i;
      } else {
	j = niframe;
      }
      node = top->current_node->frames;

      while (node) {
	if ((level == node->level) && (j == node->num)) {
	  win->frameurl = strdup(node->url);
	  break;
	}
	node = node->next;
      }
    }
    if (top->new_node && node) {
      win->scrolled_win = node->scrolled_win;
      nw = (HTMLWidget) win->scrolled_win;
      nw->html.ignore_setvalues = 1;
      XtVaSetValues(win->scrolled_win,
         WbNisFrame, True,
         WbNtext, 0,
         XmNresizePolicy, XmRESIZE_ANY,
         WbNpreviousVisitedTestFunction, anchor_visited_predicate,
         WbNdelayImageLoads, top->delay_image_loads ? True : False,
         WbNframeSupport, top->frame_support,
         WbNblinkingText, top->blink_text,
         WbNbodyColors, top->body_color,
         WbNbodyImages, top->body_images,
         WbNfontColors, top->font_color,
         WbNfontSizes, top->font_sizes,
	 WbNformButtonBackground, top->form_button_bg,
         XmNshadowThickness, 2,
         XmNx, x,
         XmNwidth, width,
         WbNmarginWidth, frame->frame_margin_width,
         XmNy, y,
         XmNheight, height,
         WbNmarginHeight, frame->frame_margin_height,
         XmNborderWidth, frame->frame_border,
	 WbNscrollBars, frame->frame_scroll_type,
	 XmNmappedWhenManaged, frame_info->reason ? False : True,
         NULL);
      XtSetSensitive(win->scrolled_win, True);
      XtVaGetValues(win->scrolled_win, WbNview, (long)(&win->view), NULL);
      XtManageChild(win->scrolled_win);
    } else {
      win->scrolled_win = XtVaCreateManagedWidget
        ("view", htmlWidgetClass, parent->scrolled_win,
         WbNisFrame, True,
         WbNtext, 0,
         XmNresizePolicy, XmRESIZE_ANY,
         WbNpreviousVisitedTestFunction, anchor_visited_predicate,
         WbNdelayImageLoads, top->delay_image_loads ? True : False,
         WbNframeSupport, top->frame_support,
         WbNblinkingText, top->blink_text,
         WbNbodyColors, top->body_color,
         WbNbodyImages, top->body_images,
         WbNfontColors, top->font_color,
         WbNfontSizes, top->font_sizes,
	 WbNformButtonBackground, top->form_button_bg,
         XmNshadowThickness, 2,
         XmNx, x,
         XmNwidth, width,
         WbNmarginWidth, frame->frame_margin_width,
         XmNy, y,
         XmNheight, height,
         WbNmarginHeight, frame->frame_margin_height,
         XmNborderWidth, frame->frame_border,
	 WbNscrollBars, frame->frame_scroll_type,
	 XmNmappedWhenManaged, frame_info->reason ? False : True,
         NULL);

      XtVaGetValues(win->scrolled_win, WbNview, (long)(&win->view), NULL);
      /* Now that the htmlWidget is created we can do this  */
      mo_make_popup(win->view);

      /* Skip if only computing IFRAME size */
      if (!frame->cw_only) {
        XtAddEventHandler(win->view, KeyPressMask, False,
		          mo_view_keypress_handler, win);

        /* Put IFRAMES under scroll bars */
        if (frame_info->reason == IFRAME_CREATE) {
	  Window wins[2];

	  wins[0] = XtWindow(hw->html.vbar);
	  wins[1] = XtWindow(win->scrolled_win);
	  XRestackWindows(XtDisplay(hw), wins, 2);
	}
      }
      nw = (HTMLWidget) win->scrolled_win;
      nw->html.node_count = 0;
    }

    if (frame_info->reason == IFRAME_CREATE)
      frame->iframe = win->scrolled_win;

    /* Skip if in width calculation */
    if (!frame->cw_only) {
      XtAddCallback(win->scrolled_win, WbNpointerMotionCallback,
		    pointer_motion_callback, win);
      XtAddCallback(win->scrolled_win, WbNanchorCallback, anchor_cb, win);
      XtAddCallback(win->scrolled_win, WbNtitleCallback, title_callback, win);
      XtAddCallback(win->scrolled_win, WbNsubmitFormCallback,
                    submit_form_callback, win);
    }
    XtAddCallback(win->scrolled_win, WbNbaseCallback, base_callback, win);
    XtAddCallback(win->scrolled_win, WbNimageCallback, ImageResolve, win);
    XtAddCallback(win->scrolled_win, WbNimageQuantizeCallback,
		  ImageQuantize, win);
    XtAddCallback(win->scrolled_win, WbNframeCallback, frame_cb, win);

    nw->html.ignore_setvalues = 1;

    mo_set_fonts(win, top->font_size);

    win->underlines_snarfed = 0;
    mo_set_underlines(win, top->underlines_state);

    XtVaSetValues(win->scrolled_win,
                  WbNblinkTime,
                  get_pref_int(eBLINK_TIME),
                  WbNfontBase,
                  get_pref_int(eFONTBASESIZE),
                  NULL);

    nw->html.ignore_setvalues = 0;

    /* Load its contents */
    top->do_frame = win;

    if (!top->new_node || !win->frameurl)
      win->frameurl = mo_url_canonicalize_keep_anchor(frame->frame_src,
						      parent->cached_url);
    /* Check for text in cache */
    cached_data = mo_fetch_cached_frame_data(win->frameurl);
    if (cached_data) {
      win->frametext = cached_data->text;
    } else {
      win->frametext = NULL;
    }
    win->frametexthead = win->frametext;

    /* Check for too many levels.  Allow more if not found in cache. */
    if (((level > 1) && !strcmp(win->frameurl, parent->cached_url)) ||
	(win->frametext && (level > 5)) || (level > 10))
      win->frametext = win->frametexthead = "Frame depth too great";

    if (top->reloading && win->frametext) {
      /* Top level window is reloading, so reload frames also */
      win->cached_url = strdup(win->frameurl);  /* Need for image zapping */
      mo_reload_frame_text(win, top);
    } else if (win->frametext) {
      if (cached_data) {
        /* Found it in cache */
        mo_do_window_text(top, win->frameurl, win->frametext, win->frametext, 0,
			  NULL, cached_data->last_modified,
			  cached_data->expires, cached_data->charset);
      } else {
	/* Had too many levels */
        mo_do_window_text(top, win->frameurl, win->frametext, win->frametext, 0,
			  NULL, NULL, NULL, NULL);
      }
    } else if (interrupted) {
      /* Don't try to load if user aborted loading */
      continue;
    } else {
      /* Get it the first time */
      HTReferer = parent->cached_url;
      mo_load_window_text(top, win->frameurl, NULL);
      HTReferer = NULL;
    }
  }
  top->do_frame = NULL;

  /* Must pass IFRAME document size back in case we are in a table */
  if (frame_info->reason == IFRAME_CREATE) {
    frame_info->doc_height = nw->html.doc_height;
    frame_info->doc_width = nw->html.doc_width;
#ifndef DISABLE_TRACE
    if (srcTrace)
      fprintf(stderr, "Created IFRAME with H=%d, W=%d, cw_only=%d\n",
	      nw->html.doc_height, nw->html.doc_width, frame->cw_only);
#endif
  }

  /* Delete size calculation test IFRAME */
  if (frame->cw_only) {
    /* Free up HTMLWidget stuff */
    HTMLFreeWidget(nw);

    /* Keep XtSetSensitive from causing SetValues routine to mess around */
    nw->html.ignore_setvalues = 1;

    /* Keep pointer callbacks and other events from happening */
    XtSetSensitive(win->scrolled_win, False);

    /* Get it off the screen */
    XtUnmanageChild(win->scrolled_win);

    XtRemoveAllCallbacks(win->scrolled_win, WbNimageCallback);
    XtRemoveAllCallbacks(win->scrolled_win, WbNimageQuantizeCallback);
    XtRemoveAllCallbacks(win->scrolled_win, WbNbaseCallback);
    XtRemoveAllCallbacks(win->scrolled_win, WbNframeCallback);

    /* Put original frames back */
    parent->frames = ori_frames;

    if (win->framename)
      free(win->framename);
    if (win->cached_url)
      free(win->cached_url);
    if (win->frameurl)
      free(win->frameurl);
    if (win->target_anchor)
      free(win->target_anchor);
    free(win);
  }

  return;
}


/* Exported to libwww2. */
void mo_gui_notify_progress(char *msg)
{
  XmString xmstr;
  mo_window *win = current_win;
  static int check = -1;

  if (check == -1)	/* Avoid repeating routine call */
      check = get_pref_boolean(eTRACK_POINTER_MOTION);

  if (!check)
      return;

  if (!msg)
      msg = " ";

  xmstr = XmStringCreateSimple(msg);
  XtVaSetValues(win->tracker_label, XmNlabelString, (XtArgVal)xmstr, NULL);
  XmStringFree(xmstr);

  XmUpdateDisplay(win->base);

  return;
}


static void UpdateButtons(Widget w)
{
  XEvent event;
  Display *display = XtDisplay(w);
  
  XSync(display, 0);
  
  while (XCheckMaskEvent(display, (ButtonPressMask|ButtonReleaseMask),&event)) {
      XButtonEvent *bevent = &(event.xbutton);

      if ((bevent->window == XtWindow(current_win->logo)) ||
	  (use_tool[BTN_STOP] &&
	   (bevent->window == XtWindow(current_win->tools[BTN_STOP].w))))
          XtDispatchEvent(&event);

      /* Else just throw it away... users shouldn't be pressing buttons
       * in the middle of transfers anyway...
       */
  }
}


void mo_gui_check_win_security_icon(int type, mo_window *win)
{
  Pixmap pix;
  static int current = HTAA_NONE;
  static int check = -1;

  if (check == -1)	/* Avoid repeating routine call */
      check = get_pref_boolean(eSECURITYICON);

  if (!check)
      return;

  switch (type) {
      case HTAA_UNKNOWN:
	  pix = securityUnknown;
	  current = type;
	  break;

      case HTAA_NONE:
	  pix = securityNone;
	  current = type;
	  break;

      case HTAA_KERBEROS_V4:
	  pix = securityKerberos4;
	  current = type;
	  break;

      case HTAA_KERBEROS_V5:
	  pix = securityKerberos5;
	  current = type;
	  break;

      case HTAA_MD5:
	  pix = securityMd5;
	  current = type;
	  break;

      case HTAA_BASIC:
	  pix = securityBasic;
	  current = type;
	  break;

      case HTAA_DOMAIN:
	  pix = securityDomain;
	  current = type;
	  break;

      case HTAA_LOGIN:
	  pix = securityLogin;
	  current = type;
	  break;

      default:
	  pix = securityUnknown;
	  current = type;
	  break;
  }

  if ((char *)pix)
      DrawSecurityPixmap(win->security, pix);
  
  UpdateButtons(win->base);
  XmUpdateDisplay(win->base);

  return;
}


void mo_gui_check_security_icon(int type)
{
  mo_window *win = current_win;

  mo_gui_check_win_security_icon(type, win);

  return;
}


int logo_count = 0;
static int pix_count;

int mo_gui_check_icon (int twirl)
{
  mo_window *win = current_win;
  int ret;
  static int cnt = 0;
  static int check = -1;

  if (twirl > 0) {
      if (!makeBusy) {
	  cursorAnimCnt = -1;
	  makeBusy = 1;
      }
      if (++cnt == 2) {
	  animateCursor();
	  cnt = 0;
      }

      if (check == -1)	/* Avoid repeating expensive routine calls */
	  check = get_pref_boolean(eTWIRLING_TRANSFER_ICON);

      if (check) {
	  if ((char *)IconPix[logo_count])
	      AnimatePixmapInWidget(win->logo, IconPix[logo_count]);
	  if (++logo_count >= pix_count)
#ifdef VMS
	      logo_count = 1;
#else
	      logo_count = 0;
#endif
      }
  }
  
  UpdateButtons(win->base);
  XmUpdateDisplay(win->base);

  ret = connect_interrupt;
  connect_interrupt = 0;

  return(ret);
}


void mo_gui_clear_icon (void)
{
  connect_interrupt = 0;
}


void mo_gui_apply_default_icon(void)
{
  mo_window *win = current_win;

  XmxApplyPixmapToLabelWidget(win->logo, IconPix[0]);
}


static void ungrab_the_pointer(XtPointer client_data)
{
  XUngrabPointer(dsp, CurrentTime);
}


void mo_gui_done_with_icon(void)
{
    mo_window *win = current_win;

    if (makeBusy) {
	XClearArea(XtDisplay(win->logo), XtWindow(win->logo), 0, 0, 0, 0, True);
	makeBusy = 0;
	stopBusyAnimation();
	logo_count = 0;
	/* This works dammit (trust me) - TPR */
	XtAppAddTimeOut(app_context, 10,
                        (XtTimerCallbackProc)ungrab_the_pointer, NULL);
    }
}


void mo_presentation_mode(mo_window *win)
{
    if (!pres) {
	pres = 1;
	scount = -1;
	mo_duplicate_window(win);
	mo_delete_window(win);
    } else {
	pres = 0;
	scount = -1;
	mo_duplicate_window(win);
	mo_delete_window(win);
    }
}


static void ResizeMeter(Widget meter, XtPointer client, XtPointer call)
{
    XWindowAttributes wattr;
    mo_window *win = (mo_window *) client;

    if (!XtWindow(meter))
	return;
    
    XGetWindowAttributes(dsp, XtWindow(meter), &wattr);

    win->meter_width = wattr.width;
    win->meter_height = wattr.height;
}


/* Callback to redraw the meter */
static void DrawMeter(Widget meter, XtPointer client, XtPointer call)
{
    mo_window *win = (mo_window *) client;
    unsigned long mask = 0;
    XGCValues values;
    int current_len;
    int level = win->meter_level;
    static GC gc;
    static int meter_fontW, meter_fontH;
    static XFontStruct *meter_font = NULL;
    static char *finished = "100%";
    static int init = 0;

    if (!init) {
	init = 1;
	gc = XCreateGC(dsp, XtWindow(meter), 0, NULL);

	XtVaGetValues(win->scrolled_win, WbNmeterFont, &(meter_font), NULL);
	if (!meter_font) {
	    fprintf(stderr,
	      "METER Cannot Get Font -- Please set 'Mosaic*MeterFont: FONT'\n");
	    fprintf(stderr,
	      "  Where FONT is a 14 point font on your system.\n");
	} else {
	    meter_fontW = meter_font->max_bounds.rbearing;
	    meter_fontH = meter_font->max_bounds.ascent;
	    values.font = meter_font->fid;
	    mask = GCFont;
	    XChangeGC(XtDisplay(meter), gc, mask, &values);
	}
    }
    if (win->meter_width == -1)
	ResizeMeter(meter, (XtPointer)win, NULL);

    current_len = (win->meter_width * level) / 100;

    XSetForeground(dsp, gc, win->meter_bg);
    XFillRectangle(dsp, XtWindow(meter), gc,
		   current_len, 0, win->meter_width, win->meter_height);

    XSetForeground(dsp, gc, win->meter_fg);
    XFillRectangle(dsp, XtWindow(meter), gc,
		   0, 0, current_len, win->meter_height);

    if ((level == 100) && !win->meter_text)
	win->meter_text = finished;

    if (meter_font && ((level <= 100) || win->meter_text)) {
	int x, y, l;
	char *ss;
	char s[8];

	if (!win->meter_text) {
	    l = 3;
	    ss = s;
	    s[0] = level > 9 ? '0' + (level / 10) : ' ';
	    s[1] = '0' + (level % 10);
	    s[2] = '%';
	    s[3] = 0;
	} else {
	    ss = win->meter_text;
	    l = strlen(ss);
	}

	x = (win->meter_width / 2) - ((meter_fontW * l) / 2);
	y = (win->meter_height / 2) + (meter_fontH / 2);

	XSetForeground(dsp, gc, win->meter_font_bg);
	XDrawString(dsp, XtWindow(meter), gc, x + 2, y, ss, l);
	XSetForeground(dsp, gc, win->meter_font_fg);
	XDrawString(dsp, XtWindow(meter), gc, x, y - 2, ss, l);
    }
}


/* Exported to libwww2 */
void mo_gui_update_meter(int level, char *text)
{
    if (!current_win->meter)	/* No meter */
	return;

    if (level < 0)
	level = 0;
    if (level > 100)
	level = 100;
    /* Don't update if no change */
    if ((current_win->meter_level == level) &&
	((!current_win->meter_text && !text) ||
	 (current_win->meter_text && text &&
	  !strcmp(current_win->meter_text, text)))) {
        return;
    }
    current_win->meter_text = text;
    current_win->meter_level = level;
                
    DrawMeter(current_win->meter, (XtPointer) current_win, NULL);
}


/* Take a text string of the form "MENU,URL,VIEW,STATUS" and set the
   layout slab stuff from it */

/* WARNING:  The code to draw the interface expects the rules enforced herein
 * to be followed... just taking out the safety checks here could cause some
 * major headaches.
 */
static int parse_slabinfo(char *s)
{
    int k, j, i, done;
    char *p;

    for (p = s, i = 0, done = 0; !done; p++) {
        if (!*p)
	    done = 1;
        if (!*p || (*p == ',')) {
            *p = 0;

            if (i == 7) {
                fprintf(stderr, "layout: too many slabs\n");
                return 0;
            }
            for (j = 0; slab_words[j]; j++) {
                if ((strlen(slab_words[j]) == strlen(s)) &&
                   !strcmp(slab_words[j], s)) {
                    if (j == SLAB_TEXTTOOLS) {
                        j = SLAB_TOOLS;
                        stexttools = 1;
                    }
                    if (j == SLAB_SMALLGLOBE) {
                        j = SLAB_GLOBE;
                        smalllogo = 1;
                    }
                    sarg[i++] = j;
                    goto next1;
                }
            }
            fprintf(stderr, "layout: bad slab name \"%s\"\n", s);
            return 0;
            
          next1:          
            s = p + 1;
            continue;
        }
        if (isalpha(*p)) {
            *p = toupper(*p);
        } else {
            fprintf(stderr, "layout: bad character '%c'\n", *p);
            return 0;
        }
    }

    /* Do some idiot-proofing */
    for (done = 0, j = 0; j < i; j++) {
        if (sarg[j] == SLAB_VIEW)
	    done = 1;
        if (sarg[j] == SLAB_GLOBE) {
            if (smalllogo) {
                if ((j + 1) >= i) {
                    fprintf(stderr,
			    "layout: SMALLGLOBE requires one normal slab\n");
                    return 0;
                }
                if (sarg[j + 1] == SLAB_VIEW || sarg[j + 1] == SLAB_TOOLS) {
                    fprintf(stderr,
			    "layout: SMALLGLOBE may not be next to %s\n",
                            slab_words[sarg[j + 1]]);
                    return 0;
                }
            } else {
                if ((j + 2) >= i) {
                    fprintf(stderr,
			    "layout: GLOBE requires two normal slabs\n");
                    return 0;
                }
                if ((sarg[j + 1] == SLAB_VIEW) || (sarg[j + 2] == SLAB_VIEW)) {
                    fprintf(stderr,
			    "layout: GLOBE requires two normal slabs\n");
                    return 0;
                }
            }
            
        }
    }
    if (!done) {
        fprintf(stderr, "layout: one VIEW slab required\n");
        return 0;
    }

    /* Check for duplicate slabs */
    for (j = 0; j < i; j++) {
        for (k = 0; k < i; k++) {
            if ((k != j) && (sarg[j] == sarg[k])) {
                fprintf(stderr, "layout: only one %s slab allowed\n",
                        slab_words[j]);
                return 0;
            }
        }
    }

    /* Whew.  Made it. */
    scount = i;
    return 1;
}


static struct tool mo_tools[] = { 
    {"<-", "Back", "Previous page", mo_back,
	&toolbarBack, &toolbarBackGRAY, moMODE_ALL, 1, NULL},
    {"->", "Forward", "Next page", mo_forward,
	&toolbarForward, &toolbarForwardGRAY, moMODE_ALL, 1, NULL},
    {"Rel", "Reload", "Reload this page", mo_reload_document,
	&toolbarReload, NULL, moMODE_ALL, 0, NULL},
    {"Home", "Home", "Go Home!", mo_home_document,
	&toolbarHome, NULL, moMODE_ALL, 1, NULL},
    {"Open", "Open", "Open a new URL", mo_open_document,
	&toolbarOpen, NULL, moMODE_ALL, 0, NULL},
    {"Save", "Save", "Save current page as ...", mo_save_document,
	&toolbarSave, NULL, moMODE_ALL, 0, NULL},
    {"New", "New", "Create a new Mosaic window", mo_new_window,
	&toolbarNew, NULL, moMODE_ALL, 0, NULL},
    {"Clone", "Clone", "Clone this Mosaic window", mo_clone_window,
	&toolbarClone, NULL, moMODE_ALL, 0, NULL},
    {"Close", "Close", "Destroy this Mosaic window", mo_close_window,
	&toolbarClose, NULL, moMODE_ALL, 0, NULL},
    {"+ Hot", "Add To Hotlist", "Add current page to hotlist", mo_register_node_in_default_hotlist,
	&toolbarAddHotlist, NULL, moMODE_PLAIN, 0, NULL},
    {"Find", "Find", "Search this document", mo_search,
	&toolbarSearch, NULL, moMODE_ALL, 0, NULL},
    {"Prt", "Print", "Print this document", mo_print_document,
	&toolbarPrint, NULL, moMODE_ALL, 0, NULL},
    {"Grps", "Groups", "Newsgroups index", mo_news_groups,
	&toolbarNewsGroups, NULL, moMODE_ALL, 0, NULL},
    {"Cook", "Cookies", "Open Cookie Jar Manager", mo_cookie_manager,
	&toolbarCookie, NULL, moMODE_ALL, 0, NULL},
    {"Stop", "Stop", "Abort loading or animations", mo_stop,
	&toolbarStop, NULL, moMODE_ALL, 0, NULL},
    /* News Mode */
    {"Idx", "Index", "Newsgroup article index", mo_news_index,
	&toolbarNewsIndex, NULL, moMODE_NEWS, 1, NULL},
    {"<Thr", "< Thread", "Go to previous thread", mo_news_prevt,
	&toolbarNewsFRev, &toolbarNewsFRevGRAY, moMODE_NEWS, 1, NULL},
    {"<Art", "< Article", "Go to previous article", mo_news_prev,
	&toolbarNewsRev, &toolbarNewsRevGRAY, moMODE_NEWS, 1, NULL},
    {"Art>", "Article >", "Go to next article", mo_news_next,
	&toolbarNewsFwd, &toolbarNewsFwdGRAY, moMODE_NEWS, 1, NULL},
    {"Thr>", "Thread >", "Go to next thread", mo_news_nextt,
	&toolbarNewsFFwd, &toolbarNewsFFwdGRAY, moMODE_NEWS, 1, NULL},
    {"Post", "Post", "Post a UseNet Article", mo_news_post,
	&toolbarPost, &toolbarPostGRAY, moMODE_NEWS, 1, NULL},
    {"Foll", "Followup", "Follow-up to UseNet Article", mo_news_follow,
	&toolbarFollow, &toolbarFollowGRAY, moMODE_NEWS, 1, NULL},
    /* FTP Mode */
    {"Put", "Put", "Send file to remote host", mo_ftp_put,
	&toolbarFTPput, NULL, moMODE_FTP, 1, NULL},
    {"Mkdir", "Mkdir", "Make remote directory", mo_ftp_mkdir,
	&toolbarFTPmkdir, NULL, moMODE_FTP, 1, NULL},
    {NULL, NULL, NULL, 0, NULL, NULL, 0, 0, NULL}
};

/* NOTE: THESE MUST COINCIDE EXACTLY WITH mo_tools!!! */
/* If more are added or the order changes, then toolbar.h must be updated */
static char *tool_names[] = {
	"BACK",
	"FORWARD",
	"RELOAD",
	"HOME",
	"OPEN",
	"SAVE",
	"NEW",
	"CLONE",
	"CLOSE",
	"ADD_TO_HOTLIST",
	"FIND",
	"PRINT",
	"GROUPS",
	"COOKIEJAR",
	"STOP",
	"INDEX",
	"PREVIOUS_THREAD",
	"PREVIOUS_ARTICLE",
	"NEXT_ARTICLE",
	"NEXT_THREAD",
	"POST",
	"FOLLOW_UP",
	"PUT",
	"MKDIR",
	NULL
};


static void mo_get_tools_from_res()
{
    int i;
    char *tools, *ptr, *start, *end;
    static int init = 0;

    /* No need to do more than once */
    if (init)
	return;
    init = 1;

    if (get_pref_boolean(eKIOSK)) {
	if (ptr = get_pref_string(eTOOLBAR_LAYOUT)) {
	    fprintf(stderr,
		    "Toolbar Resource is Overiding the Kiosk Toolbar.\n");
	} else if (get_pref_boolean(eKIOSKPRINT)) {
	    ptr = "BACK,FORWARD,HOME,CLOSE,PRINT";
	} else {
	    ptr = "BACK,FORWARD,HOME,CLOSE";
	}
    } else if (get_pref_boolean(eKIOSKNOEXIT)) {
	if (ptr = get_pref_string(eTOOLBAR_LAYOUT)) {
	    fprintf(stderr,
		    "Toolbar Resource is Overiding the Kiosk Toolbar.\n");
	} else if (get_pref_boolean(eKIOSKPRINT)) {
	    ptr = "BACK,FORWARD,HOME,PRINT";
	} else {
	    ptr = "BACK,FORWARD,HOME";
	}
    } else if (!(ptr = get_pref_string(eTOOLBAR_LAYOUT))) {
	ptr = "BACK,FORWARD,RELOAD,HOME,OPEN,SAVE,CLONE,CLOSE,FIND,PRINT,GROUPS,COOKIEJAR,STOP,INDEX,PREVIOUS_THREAD,PREVIOUS_ARTICLE,NEXT_ARTICLE,NEXT_THREAD,POST,FOLLOW_UP,PUT,MKDIR";
    }

    tools = strdup(ptr);

    for (i = 0; tool_names[i]; i++)
	use_tool[i] = 0;

    for (start = tools; start && *start; ) {
	ptr = start;
	for (; *ptr && isspace(*ptr); ptr++)
	    ;
	if (*ptr == ',')
	    ptr++;
	end = strchr(ptr, ',');
	if (end) {
	    start = end + 1;
	    *end = '\0';
	} else {
	    start = NULL;
	}
	for (i = 0; tool_names[i]; i++) {
	    if (!my_strncasecmp(tool_names[i], ptr, strlen(tool_names[i])))
		use_tool[i] = 1;
	}
    }

    free(tools);

    return;
}


static void mo_extra_buttons(mo_window *win, Widget top)
{
    win->security = XmxMakeNamedPushButton(top, NULL, "sec",
                                           security_pressed_cb, 0);
    XmxApplyPixmapToLabelWidget(win->security, securityUnknown);

    XtVaSetValues(win->security,
                  XmNmarginWidth, 0,
                  XmNmarginHeight, 0,
                  XmNmarginTop, 0,
                  XmNmarginBottom, 0,
                  XmNmarginLeft, 0,
                  XmNmarginRight, 0,
                  XmNuserData, (XtPointer) "Authentication Status",
		  XmNtraversalOn, False,
		  NULL);
    XmxAddClue(win->security, "Authentication Status");
    XtOverrideTranslations(win->security, XtParseTranslationTable(xlattab));
    
    win->encrypt = XmxMakeNamedPushButton(top, NULL, "enc",
                                          encrypt_pressed_cb, 0);
    XmxApplyPixmapToLabelWidget(win->encrypt, enc_not_secure);

    XtVaSetValues(win->encrypt,
              XmNmarginWidth, 0,
              XmNmarginHeight, 0,
              XmNmarginTop, 0,
              XmNmarginBottom, 0,
              XmNmarginLeft, 0,
              XmNmarginRight, 0,
#ifdef HAVE_SSL
              XmNuserData, (XtPointer) "Encryption Status",
#else
              XmNuserData, (XtPointer) "Encryption Status (not in this client)",
#endif
	      XmNtraversalOn, False,
              NULL);
    XmxAddClue(win->encrypt, "Encryption Status");
    XtOverrideTranslations(win->encrypt, XtParseTranslationTable(xlattab));
    
    /* Insure we set the security icon! */
    if (win->current_node)
        mo_gui_check_win_security_icon(win->current_node->authType, win);
}


static void mo_make_globe(mo_window *win, Widget parent, int small)
{
    char *message;

    if (!small) {
        IconPix = IconPixBig;
        IconWidth = IconHeight = 64;
        WindowWidth = WindowHeight = 0;
	pix_count = get_pref_int(ePIX_COUNT);
        logo_count = 0;
    } else {
        IconPix = IconPixSmall;
        IconWidth = IconHeight = 32;
        logo_count = 0;
        pix_count = NUMBER_OF_FRAMES;
        WindowWidth = WindowHeight = 0;
    }

    win->logo = XmxMakeNamedPushButton(parent, NULL, "logo",
				       icon_pressed_cb, 0);
    XmxApplyPixmapToLabelWidget(win->logo, IconPix[0]);
    XtVaSetValues(win->logo,
                  XmNmarginWidth, 0,
                  XmNmarginHeight, 0,
                  XmNmarginTop, 0,
                  XmNmarginBottom, 0,
                  XmNmarginLeft, 0,
                  XmNmarginRight, 0,
                  XmNtopAttachment, XmATTACH_FORM,
                  XmNbottomAttachment, XmATTACH_FORM,
                  XmNleftAttachment, !win->biglogo ||
			(!win->smalllogo && win->toolbardetached) ?
			XmATTACH_FORM : XmATTACH_NONE,
                  XmNrightAttachment, XmATTACH_FORM,
		  XmNtraversalOn, False,
                  NULL);

    if (win->biglogo) {
        if (win->smalllogo) {
            mo_extra_buttons(win, win->slab[SLAB_GLOBE]);
            XtVaSetValues(win->security,
                          XmNtopAttachment, XmATTACH_FORM,
                          XmNbottomAttachment, XmATTACH_FORM,
                          XmNleftAttachment, XmATTACH_FORM,
                          XmNrightAttachment, XmATTACH_NONE,
                          NULL);
            XtVaSetValues(win->encrypt,
                          XmNtopAttachment, XmATTACH_FORM,
                          XmNbottomAttachment, XmATTACH_FORM,
                          XmNleftAttachment, XmATTACH_WIDGET,
                          XmNleftWidget, win->security,
                          XmNrightAttachment, XmATTACH_WIDGET,
                          XmNrightWidget, win->logo,
                          NULL);
        } else if (!win->toolbardetached) {
            mo_extra_buttons(win, win->slab[SLAB_GLOBE]);
            XtVaSetValues(win->security,
                          XmNtopAttachment, XmATTACH_FORM,
                          XmNbottomAttachment, XmATTACH_NONE,
                          XmNleftAttachment, XmATTACH_FORM,
                          XmNrightAttachment, XmATTACH_WIDGET,
                          XmNrightWidget, win->logo,
                          NULL);
            XtVaSetValues(win->encrypt,
                          XmNtopAttachment, XmATTACH_WIDGET,
                          XmNtopWidget, win->security,
                          XmNbottomAttachment, XmATTACH_FORM,
                          XmNleftAttachment, XmATTACH_FORM,
                          XmNrightAttachment, XmATTACH_WIDGET,
                          XmNrightWidget, win->logo,
                          NULL);
        }
    }
    if (get_pref_boolean(eKIOSK) || 
        get_pref_boolean(eKIOSKNOEXIT) || 
        get_pref_boolean(eDISABLEMIDDLEBUTTON)) {
	message = "Logo Button - Abort current transfer or animations";
        XtVaSetValues(win->logo, XmNuserData, (XtPointer)message, NULL);
    } else {
	message = "Abort current transfer or animations (button 1) or Paste URL (middle button)";
        XtVaSetValues(win->logo, XmNuserData, (XtPointer)message, NULL);
        XtOverrideTranslations(win->logo,
			       XtParseTranslationTable(logo_translations));
    }
    XmxAddClue(win->logo, message);

    XtOverrideTranslations(win->logo,
                           XtParseTranslationTable(xlattab));
}

static void mo_tool_detach_cb(Widget wx, XtPointer cli, XtPointer call)
{
    Atom WM_DELETE_WINDOW;
    mo_window *win = (mo_window *) cli;
    int i, h, w;

    /* Xmx sucks */
    XmxSetUniqid(win->id);

    XtUnmanageChild(XtParent(win->scrolled_win));
    
    /* Remove widgets from LiteClue list */
    XmxDeleteClue(win->security);
    XmxDeleteClue(win->encrypt);
    XmxDeleteClue(win->logo);
    XmxDeleteClue(win->tearbutton);
    for (i = 0; mo_tools[i].label; i++) {
	if ((mo_tools[i].action > 0) && use_tool[i] && win->tools[i].w)
	    XmxDeleteClue(win->tools[i].w);
    }

    if (win->toolbardetached) {
        win->toolbardetached = 0;

        XtUnmanageChild(win->toolbarwin);
        XtDestroyWidget(win->toolbarwin);
        win->toolbarwin = NULL;
        win->topform = win->slab[SLAB_TOOLS];
        mo_fill_toolbar(win);        
        if (win->biglogo && !win->smalllogo)
            mo_make_globe(win, win->slab[SLAB_GLOBE], 0);
    } else {
        win->toolbardetached = 1;

        XtUnmanageChild(win->button_rc);
        XtUnmanageChild(win->button2_rc);
        XtDestroyWidget(win->button_rc);
        XtDestroyWidget(win->button2_rc);

        if (win->biglogo && !win->smalllogo) {
            XtUnmanageChild(win->logo);
            XtDestroyWidget(win->logo);
            XtUnmanageChild(win->security);
            XtDestroyWidget(win->security);
            XtUnmanageChild(win->encrypt);
            XtDestroyWidget(win->encrypt);            
        }
        h = win->toolbarorientation ? 640 : 40;
        w = win->toolbarorientation ? 40 : 640;
        win->toolbarwin = XtVaCreatePopupShell
            ("ToolBox",
             xmDialogShellWidgetClass,
             win->base,
             XmNminHeight, h,
             XmNminWidth, w,
             XmNmaxHeight, h,
             XmNmaxWidth, w,
             XmNheight, h,
             XmNwidth, w,
             XmNallowShellResize, FALSE,
             NULL);
        XtManageChild(win->toolbarwin);
        win->topform = XtVaCreateWidget
            ("slab_tools",
             xmFormWidgetClass, win->toolbarwin, 
             XmNminHeight, h,
             XmNminWidth, w,
             XmNmaxHeight, h,
             XmNmaxWidth, w,
             XmNheight, h,
             XmNwidth, w,
             NULL);
        mo_fill_toolbar(win);
        XtManageChild(win->topform);
        WM_DELETE_WINDOW = XmInternAtom(dsp, "WM_DELETE_WINDOW", False);
        XmAddWMProtocolCallback(win->toolbarwin, WM_DELETE_WINDOW,
                                mo_tool_detach_cb, (XtPointer)win);
        XtPopup(win->toolbarwin, XtGrabNone);
    }

    XtManageChild(XtParent(win->scrolled_win));
}

void mo_switch_mode(mo_window *win)
{
    int i;

    for (i = 0; mo_tools[i].label; i++) {
        if (use_tool[i] && win->tools[i].w) {
            if (!(mo_tools[i].toolset & win->mode)) {
                if (XtIsManaged(win->tools[i].w))
                    XtUnmanageChild(win->tools[i].w);
            } else if (!XtIsManaged(win->tools[i].w)) {
                XtManageChild(win->tools[i].w);
            }
        }
    }
}

    
void mo_tool_state(struct toolbar *t, int state, int index)
{
    if (use_tool[index])
	XmxSetSensitive(t->w, t->gray = state);
}


/* Create topform and fill it with toolbar bits'n'pieces */
static void mo_fill_toolbar(mo_window *win)
{
    int vert = win->toolbarorientation && win->toolbardetached;
    int textbuttons = win->texttools;
    int long_text = get_pref_boolean(eUSE_LONG_TEXT_NAMES);
    int i;
    static XFontStruct *tmpFont = NULL;
    static XmFontList tmpFontList;

    if (!tmpFont) {
	XtVaGetValues(win->scrolled_win,
		      WbNtoolbarFont,
		      &tmpFont,
		      NULL);
	if (!tmpFont) {
	    fprintf(stderr,
		"Could not load Toolbar font!  The X Resource is Mosaic*ToolbarFont\n");
	    fprintf(stderr,
		"Default font is: -adobe-times-bold-r-normal-*-12-*-*-*-*-*-iso8859-1\nExiting Mosaic.");
	    exit(1);
	}
	tmpFontList = XmFontListCreate(tmpFont, XmSTRING_DEFAULT_CHARSET);
    }

    /* Which tools to show */
    mo_get_tools_from_res();

    XmxSetUniqid(win->id);

    win->button2_rc = XtVaCreateWidget
        ("buttonrc2", xmRowColumnWidgetClass,
         win->topform,
         XmNorientation, vert ? XmVERTICAL : XmHORIZONTAL,
         XmNmarginWidth, 0,
         XmNmarginHeight, 0,
         XmNspacing, 0,
         XmNleftOffset, 0,
         XmNrightOffset, 0,
         XmNtopOffset, 0,
         XmNbottomOffset, 0,
         XmNleftAttachment, XmATTACH_NONE,
         XmNrightAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);
    
    win->button_rc = XtVaCreateWidget
        ("buttonrc", xmRowColumnWidgetClass,
         win->topform,
         XmNorientation, vert ? XmVERTICAL : XmHORIZONTAL,
         XmNpacking, XmPACK_TIGHT,
         XmNmarginWidth, 0,
         XmNmarginHeight, 1,
         XmNspacing, 0,
         XmNleftOffset, 0,
         XmNrightOffset, 0,
         XmNtopOffset, 2,
         XmNbottomOffset, 2,
         XmNleftAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, win->button2_rc,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    win->tearbutton = XtVaCreateManagedWidget
        ("|", xmPushButtonWidgetClass,
         win->button_rc,
         XmNuserData, (XtPointer) "Toolbar Tearoff Control",
         XmNlabelType, textbuttons ? XmSTRING : XmPIXMAP,
         XmNlabelPixmap, vert ? tearh : tearv,
	 XmNfontList, tmpFontList,
	 XmNtraversalOn, False,
         NULL);
    XmxAddClue(win->tearbutton, "Toolbar Tearoff");
    XtOverrideTranslations(win->tearbutton, XtParseTranslationTable(xlattab));
    XtAddCallback(win->tearbutton, XmNactivateCallback, mo_tool_detach_cb,
                  (XtPointer) win);

    for (i = 0; mo_tools[i].label; i++) {
        if (mo_tools[i].action > 0) {
	    Widget w;

	    if (use_tool[i]) {
                win->tools[i].w = w = XtVaCreateManagedWidget
                  ((long_text ? mo_tools[i].long_text : mo_tools[i].text),
		  xmPushButtonWidgetClass,
                  win->button_rc,
                  XmNuserData, (XtPointer) mo_tools[i].label,
                  XmNmarginWidth, 0,
                  XmNmarginHeight, 0,
                  XmNmarginTop, 0,
                  XmNmarginBottom, 0,
/*
                  XmNmarginLeft, textbuttons ? 2 : 0,
                  XmNmarginRight, textbuttons ? 2 : 0,
*/
                  XmNmarginLeft, 0,
                  XmNmarginRight, 0,
                  XmNalignment, XmALIGNMENT_CENTER,
                  XmNlabelType, textbuttons ? XmSTRING : XmPIXMAP,
                  XmNlabelPixmap, *(mo_tools[i].image),
		  XmNfontList, tmpFontList,
		  XmNtraversalOn, False,
                  NULL);

	      XmxAddClue(w, mo_tools[i].label);

              XtOverrideTranslations(w, XtParseTranslationTable(xlattab));
              if (mo_tools[i].greyimage != NULL)
              	  XtVaSetValues(w, XmNlabelInsensitivePixmap,
                                *(mo_tools[i].greyimage),
                                NULL);
              XmxSetSensitive(w, win->tools[i].gray);
              XmxAddCallback(w, XmNactivateCallback, menubar_cb,
                             mo_tools[i].action);
            
              if (!(mo_tools[i].toolset & win->mode))
                  XtUnmanageChild(w);
	    }
	} else {
            win->tools[i].w = NULL;
            XtVaCreateManagedWidget(" ", xmSeparatorWidgetClass, win->button_rc,
                               XmNorientation, vert ? XmHORIZONTAL : XmVERTICAL,
                               vert ? XmNheight : XmNwidth, vert ? 3 : 4,
                               XmNseparatorType, XmNO_LINE,
		               XmNtraversalOn, False,
                               NULL);
	}
    }

    if (!win->biglogo || (!win->smalllogo && win->toolbardetached)) {
        mo_extra_buttons(win, win->button2_rc);
        mo_make_globe(win, win->button2_rc, 1);
    }  

    XtManageChild(win->button2_rc);
    XtManageChild(win->button_rc);
    
    return;
}


/****************************************************************************
 * name:    mo_fill_window (PRIVATE)
 * purpose: Take a new (empty) mo_window struct and fill in all the 
 *          GUI elements.
 * inputs:  
 *   - mo_window *win: The window.
 * returns: 
 *   mo_succeed
 * remarks: 
 *   
 ****************************************************************************/
static mo_status mo_fill_window(mo_window *win)
{
  Widget up, dn;
  int linkup, topatt, botatt;
  Widget form;
  Widget title_label, url_label;
  int i, globe;
  char *s = NULL;
  char *tmp;
  static char *pres_slab = NULL;
  static char *kiosk_slab = NULL;
  static int init = 0;
  static int kiosk;
  static int kiosknoexit;
  static int meter;

  if (!init) {
      init = 1;
      kiosk = get_pref_boolean(eKIOSK);
      kiosknoexit = get_pref_boolean(eKIOSKNOEXIT);
      if (kiosk || kiosknoexit)
          set_pref_boolean(eMETER, False);
      meter = get_pref_boolean(eMETER);
  }

  form = XtVaCreateManagedWidget("form0", xmFormWidgetClass, win->base, NULL);

  if ((scount < 0) && pres) {
      if (!pres_slab)
	  pres_slab = strdup("VIEW");
      s = pres_slab;
  } else {
      if (tmp = get_pref_string(eGUI_LAYOUT)) {
	  s = strdup(tmp);
	  if (kiosk || kiosknoexit)
	      fprintf(stderr,
		  "The Gui Layout Resource is Overiding the Kiosk Resource.\n");
      } else if (kiosk || kiosknoexit) {
	  if (!kiosk_slab)
	      kiosk_slab = strdup("TOOLS,STATUS,VIEW");
	  s = kiosk_slab;
      }
  }

  if ((scount < 0) && s)
      parse_slabinfo(s);

  if (scount < 0) {
      /* Go with the default layout */
      win->smalllogo = 0;
      win->texttools = 0;
      win->slabcount = 6;
      win->slabpart[0] = SLAB_MENU;
      win->slabpart[1] = SLAB_GLOBE;
      win->slabpart[2] = SLAB_TOOLS;
      win->slabpart[3] = SLAB_URL;
      win->slabpart[4] = SLAB_VIEW;
      win->slabpart[5] = SLAB_STATUS;
  } else {
      win->texttools = stexttools;
      win->smalllogo = smalllogo;
      win->slabcount = scount;
      for (i = 0; i < scount; i++)
          win->slabpart[i] = sarg[i];
  }

  win->biglogo = 0;
  for (i = 0; i < win->slabcount; i++) {
      if (win->slabpart[i] == SLAB_GLOBE)
	  win->biglogo = 1;
  }
  
  /* No active toolset, horiz, not detached */
  win->toolset = 0;
  win->toolbarorientation = 0;
  win->toolbardetached = 0;
  win->toolbarwin = NULL;

  /*********************** SLAB_GLOBE ****************************/
  if (win->biglogo) {
      win->slab[SLAB_GLOBE] = XtVaCreateWidget("slab_globe",
                                               xmFormWidgetClass, form, NULL);
      mo_make_globe(win, win->slab[SLAB_GLOBE], win->smalllogo);
  } else {
      win->slab[SLAB_GLOBE] = NULL;
  }
  
  /*********************** SLAB_MENU ****************************/
  win->menubar = mo_make_document_view_menubar(form);
  win->slab[SLAB_MENU] = win->menubar->base;
  XtUnmanageChild(win->slab[SLAB_MENU]);

  /*********************** SLAB_TITLE ****************************/
  win->slab[SLAB_TITLE] = XtVaCreateWidget("slab_title",
                                           xmFormWidgetClass, form,
                                           XmNheight, 36, NULL);
  title_label = XtVaCreateManagedWidget("Title:", xmLabelWidgetClass,
                                        win->slab[SLAB_TITLE],
                                        XmNleftOffset, 3,
                                        XmNleftAttachment, XmATTACH_FORM,
                                        XmNrightAttachment, XmATTACH_NONE,
                                        XmNtopAttachment, XmATTACH_FORM,
                                        XmNbottomAttachment, XmATTACH_FORM,
                                        NULL);
  win->title_text = XtVaCreateManagedWidget("title", xmTextFieldWidgetClass,
                                            win->slab[SLAB_TITLE],
                                            XmNrightOffset, 3,
                                            XmNleftOffset, 3,
                                            XmNtopOffset, 3,
                                            XmNleftAttachment, XmATTACH_WIDGET,
                                            XmNleftWidget, title_label,
                                            XmNrightAttachment, XmATTACH_FORM,
                                            XmNtopAttachment, XmATTACH_FORM,
                                            XmNbottomAttachment, XmATTACH_NONE,
                                            XmNeditable, False,
                                            XmNcursorPositionVisible, False,
                                            NULL);
  
  /*********************** SLAB_URL ****************************/
  win->slab[SLAB_URL] = XtVaCreateWidget("slab_url",
                                         xmFormWidgetClass, form,
                                         XmNheight, 36, NULL);
  url_label = XtVaCreateManagedWidget("URL:", xmLabelWidgetClass,
                                      win->slab[SLAB_URL],
                                      XmNleftOffset, 3,
                                      XmNleftAttachment, XmATTACH_FORM,
                                      XmNrightAttachment, XmATTACH_NONE,
                                      XmNtopAttachment, XmATTACH_FORM,
                                      XmNbottomAttachment, XmATTACH_FORM,
                                      NULL);
  win->url_text = XtVaCreateManagedWidget("text", xmTextFieldWidgetClass,
                                          win->slab[SLAB_URL],
                                          XmNrightOffset, 3,
                                          XmNleftOffset, 3,
                                          XmNtopOffset, 3,
                                          XmNleftAttachment, XmATTACH_WIDGET,
                                          XmNleftWidget, url_label,
                                          XmNrightAttachment, XmATTACH_FORM,
                                          XmNtopAttachment, XmATTACH_FORM,
                                          XmNbottomAttachment, XmATTACH_NONE,
                                          XmNcursorPositionVisible, True,
                                          XmNeditable, True,
					  XmNtraversalOn, False,
                                          NULL);
  /* DO THIS WITH THE SLAB MANAGER */
  if (!(kiosk || kiosknoexit)) {
      XmxAddCallbackToText(win->url_text, url_field_cb, 0);
  } else {
      XtUnmanageChild(url_label);
      XtUnmanageChild(win->url_text);
  }

  XtOverrideTranslations(win->url_text,
			 XtParseTranslationTable(text_translations));
  XtOverrideTranslations(win->title_text,
			 XtParseTranslationTable(text_translations));
  XtOverrideTranslations(win->url_text,
			 XtParseTranslationTable(url_translations));

  /*********************** SLAB_VIEW ****************************/
  win->slab[SLAB_VIEW] = win->scrolled_win = XtVaCreateManagedWidget("view",
                      htmlWidgetClass, form,
                      WbNtext, 0,
                      XmNresizePolicy, XmRESIZE_ANY,
                      WbNpreviousVisitedTestFunction, anchor_visited_predicate,
                      WbNdelayImageLoads, win->delay_image_loads ? True : False,
                      WbNisFrame, False,
                      XmNshadowThickness, 2,
                      NULL);
  XtAddCallback(win->scrolled_win, WbNimageCallback, ImageResolve, win);
  XtAddCallback(win->scrolled_win, WbNimageQuantizeCallback,
		ImageQuantize, win);
  XtAddCallback(win->scrolled_win, WbNpointerMotionCallback,
		pointer_motion_callback, win);
  XtAddCallback(win->scrolled_win, WbNanchorCallback, anchor_cb, win);
  XtAddCallback(win->scrolled_win, WbNbaseCallback, base_callback, win);
  XtAddCallback(win->scrolled_win, WbNtitleCallback, title_callback, win);
  XtAddCallback(win->scrolled_win, WbNsubmitFormCallback,
                submit_form_callback, win);
  XtAddCallback(win->scrolled_win, WbNframeCallback, frame_cb, win);
  XtVaGetValues(win->scrolled_win, WbNview, (long)(&win->view), NULL);
  XtAddEventHandler(win->view, KeyPressMask, False,
		    mo_view_keypress_handler, win);
  /* Now that the htmlWidget is created we can do this  */
  mo_make_popup(win->view);
  
  /*********************** SLAB_STATUS ****************************/
  win->slab[SLAB_STATUS] = XtVaCreateWidget("slab_status",
                                            xmFormWidgetClass, form, NULL);
  /* Meter */
  if (meter) {
      win->meter_text = NULL;
      win->meter_frame = XmxMakeFrame(win->slab[SLAB_STATUS], XmxShadowIn);
      XtVaSetValues(win->meter_frame,
                    XmNrightOffset, 3,
                    XmNtopOffset, 2,
                    XmNbottomOffset, 2,
                    XmNleftAttachment, XmATTACH_NONE,
                    XmNrightAttachment, XmATTACH_FORM,
                    XmNtopAttachment, XmATTACH_FORM,
                    XmNbottomAttachment, XmATTACH_FORM,
                    NULL);
      win->meter = XtVaCreateManagedWidget("meter", xmDrawingAreaWidgetClass,
           			      win->meter_frame,
           			      XmNuserData, (XtPointer) "Progress Meter",
           			      XmNheight, 16,
           			      XmNwidth, 96,
           			      NULL);
      XtOverrideTranslations(win->meter, XtParseTranslationTable(xlattab));
      
      win->meter_level = 0;
      win->meter_width = -1;
      
      XtAddCallback(win->meter, XmNexposeCallback,
		    DrawMeter, (XtPointer) win);
      XtAddCallback(win->meter, XmNresizeCallback,
		    ResizeMeter, (XtPointer) win);
      
      /* Grab some colors */
      {
          XColor color;
	  char *mb;
          Colormap cmap;

	  cmap = installed_colormap ? installed_cmap :
		 DefaultColormapOfScreen(XtScreen(win->base));
	  XParseColor(dsp, cmap, get_pref_string(eMETER_FOREGROUND), &color);
          XAllocColor(dsp, cmap, &color);
          win->meter_fg = color.pixel;

	  /* Hack to convert old default to new default */
	  if (!strcmp(get_pref_string(eMETER_BACKGROUND), "#2F2F4F4F4F4F")) {
		mb = "#333366666666";
	  } else {
		mb = get_pref_string(eMETER_BACKGROUND);
	  }
	  XParseColor(dsp, cmap, mb, &color);
          XAllocColor(dsp, cmap, &color);
          win->meter_bg = color.pixel;
	  XParseColor(dsp, cmap, get_pref_string(eMETER_FONT_FOREGROUND),
		      &color);
          XAllocColor(dsp, cmap, &color);
          win->meter_font_fg = color.pixel;
	  XParseColor(dsp, cmap, get_pref_string(eMETER_FONT_BACKGROUND),
		      &color);
          XAllocColor(dsp, cmap, &color);
          win->meter_font_bg = color.pixel;
      }
  } else {
      win->meter_frame = NULL;
      win->meter = NULL;
  }

  win->tracker_label = XtVaCreateManagedWidget
      (" ", xmLabelWidgetClass,
       win->slab[SLAB_STATUS],
       XmNalignment, XmALIGNMENT_BEGINNING,
       XmNleftAttachment, XmATTACH_FORM,
       XmNrightAttachment, meter ? XmATTACH_WIDGET : XmATTACH_NONE,
       XmNrightWidget, meter ? win->meter_frame : NULL,
       XmNtopAttachment, XmATTACH_FORM,
       XmNbottomAttachment, XmATTACH_NONE,
       NULL);

  /*********************** SLAB_TOOLS ****************************/
  win->topform = win->slab[SLAB_TOOLS] = XtVaCreateWidget("slab_tools",
                                           		  xmFormWidgetClass,
							  form, NULL);
  mo_fill_toolbar(win);

  /* Chain those slabs together 'n stuff */
  for (globe = 0, linkup = 1, i = 0; i < win->slabcount; i++) {
      if (win->slabpart[i] == SLAB_GLOBE) {
	  /* Next two slabs have to attach to the globe */
          globe = 2-win->smalllogo;
          if (linkup) {
              XtVaSetValues
                  (win->slab[SLAB_GLOBE],
                   XmNleftAttachment, XmATTACH_NONE,
                   XmNrightAttachment, XmATTACH_FORM,
                   XmNtopAttachment, i ? XmATTACH_WIDGET : XmATTACH_FORM,
                   XmNtopWidget, i ? win->slab[win->slabpart[i - 1]] : NULL,
                   XmNbottomAttachment, XmATTACH_NONE,
                   NULL);
          } else {
              XtVaSetValues
                  (win->slab[SLAB_GLOBE],
                   XmNleftAttachment, XmATTACH_NONE,
                   XmNrightAttachment, XmATTACH_FORM,
                   XmNbottomAttachment, i + globe + 1 == win->slabcount ?
			XmATTACH_FORM : XmATTACH_WIDGET,
                   XmNbottomWidget, i + globe + 1 == win->slabcount ? NULL :
			win->slab[win->slabpart[i + globe + 1]],
                   XmNtopAttachment, XmATTACH_NONE,
                   NULL);
          }
      } else {
          if (win->slabpart[i] == SLAB_VIEW) {
              /* We change link dir here AND link this slab both ways */
              linkup = 0;
              if (!i || ((i == 1) && (win->slabpart[i - 1] == SLAB_GLOBE))) {
                  up = NULL;
                  topatt = XmATTACH_FORM;
              } else {
                  up = win->slab[win->slabpart[(i - 1) - (globe == 2 ? 1 : 0)]];
                  topatt = XmATTACH_WIDGET;
              }
              if (i == win->slabcount - 1) {
                  dn = NULL;
                  botatt = XmATTACH_FORM;
              } else {
                  dn = win->slab[win->slabpart[i + 1]];
                  botatt = XmATTACH_WIDGET;
              }
          } else {
              if (linkup) {
                  if (!i || ((i == 1) &&
			     (win->slabpart[i - 1] == SLAB_GLOBE))) {
                      up = NULL;
                      topatt = XmATTACH_FORM;
                  } else {
                      if (globe == 1 && win->smalllogo) {
                          up = win->slab[win->slabpart[i - 2]];
                      } else {
                          up = win->slab[win->slabpart[(i - 1) -
				(globe == 2 ? 1 : 0)]];
                      }
                      topatt = XmATTACH_WIDGET;
                  }
                  dn = NULL;
                  botatt = XmATTACH_NONE;
              } else {
                  if (i == win->slabcount - 1) {
                      dn = NULL;
                      botatt = XmATTACH_FORM;
                  } else {
                      dn = win->slab[win->slabpart[i + 1]];
                      botatt = XmATTACH_WIDGET;
                  }
                  up = NULL;
                  topatt = XmATTACH_NONE;
              }    
          }

          XtVaSetValues(win->slab[win->slabpart[i]],
                        XmNleftOffset, 0,
                        XmNrightOffset, 0,
                        XmNtopOffset, 0,
                        XmNbottomOffset, 0,
                        XmNtopAttachment, topatt,
                        XmNtopWidget, up,
                        XmNbottomAttachment, botatt,
                        XmNbottomWidget, dn,
                        XmNleftAttachment, XmATTACH_FORM,
                        XmNrightAttachment,
				 globe ? XmATTACH_WIDGET : XmATTACH_FORM,
                        XmNrightWidget, win->slab[SLAB_GLOBE],
                        NULL);
          if (globe)
	      globe--;
      }
  }
  for (i = 0; i < win->slabcount; i++)
      XtManageChild(win->slab[win->slabpart[i]]);

  XtManageChild(form);

  /* Can't go back or forward if we haven't gone anywhere yet... */
  mo_back_impossible(win);
  mo_forward_impossible(win);

  return mo_succeed;
}


/****************************************************************************
 * name:    mo_delete_window
 * purpose: Shut down a window.
 * inputs:  
 *   - mo_window *win: The window.
 * returns: 
 *   mo_succeed
 * remarks: 
 *   This can be called, among other places, from the WM_DELETE_WINDOW
 *   handler.  By the time we get here, we must assume the window is already
 *   in the middle of being shut down.
 *   We must explicitly close every dialog that be open as a child of
 *   the window, because window managers too stupid to do that themselves
 *   will otherwise allow them to stay up.
 ****************************************************************************/
#define POPDOWN(x) \
  if (win->x) XtUnmanageChild(win->x)

mo_status mo_delete_window(mo_window *win)
{
  mo_node *node;
  mo_node *tofree;
  HTMLWidget hw;
  int i;

  if (!win)
      return mo_fail;

  hw = (HTMLWidget) win->scrolled_win;
  /* Stop animations and refresh */
  hw->html.draw_count++;
  hw->html.refresh_count++;

  /* Free up HTMLWidget stuff */
  HTMLFreeWidget(hw);

  /* Remove widgets from liteclue list */
  XmxSetUniqid(win->id);
  XmxDeleteClueGroup();

  node = win->history;

  POPDOWN(source_win);
  POPDOWN(save_win);
  POPDOWN(savebinary_win);
  POPDOWN(open_win);
  POPDOWN(mail_win);
  POPDOWN(mailhist_win);
  POPDOWN(print_win);
  POPDOWN(history_win);
  POPDOWN(open_local_win);
  if (win->hotlist_win)
      XtDestroyWidget(win->hotlist_win);
  POPDOWN(techsupport_win);
  POPDOWN(annotate_win);
  POPDOWN(search_win);
  POPDOWN(searchindex_win);
  POPDOWN(mailto_win);
  POPDOWN(mailto_form_win);
  POPDOWN(news_win);
  POPDOWN(links_win);
#ifdef HAVE_AUDIO_ANNOTATIONS
  POPDOWN(audio_annotate_win);
#endif
  XtPopdown(win->base);

  /* We really should be doing this :-) */
  XtDestroyWidget(win->base);
  win->base = NULL;

  while (node) {
      tofree = node;
      node = node->next;
      mo_free_node(tofree);
  }
  win->history = NULL;

  if (win->image_file) {
      remove(win->image_file);
      free(win->image_file);
  }

  free(win->search_start);
  win->search_start = NULL;
  free(win->search_end);
  win->search_end = NULL;

  /* This will free the win structure (but none of its elements
   * individually) and exit if this is the last window in the list. */
  mo_remove_window_from_list(win);

  /* Go get another current_win. */
  mo_set_current_cached_win(mo_next_window(NULL));

  return mo_succeed;
}


static int mo_get_font_size_from_res(char *userfontstr, int *fontfamily)
{
  char *lowerfontstr = strdup(userfontstr);
  int  x;

  for (x = 0; x < strlen(userfontstr); x++)
      lowerfontstr[x] = tolower(userfontstr[x]);
  
  *fontfamily = 0;
  if (strstr(lowerfontstr, "times")) {
      if (strstr(lowerfontstr, "large"))
	  return mo_large_fonts;
      if (strstr(lowerfontstr, "regular"))
	  return mo_regular_fonts;
      if (strstr(lowerfontstr, "small"))
	  return mo_small_fonts;
      return mo_regular_fonts;
  }
  if (strstr(lowerfontstr, "helvetica")) {
      *fontfamily = 1;
      if (strstr(lowerfontstr, "large"))
	  return mo_large_helvetica;
      if (strstr(lowerfontstr, "regular"))
	  return mo_regular_helvetica;
      if (strstr(lowerfontstr, "small"))
	  return mo_small_helvetica;
      return mo_regular_helvetica;
  }
  if (strstr(lowerfontstr, "century")) {
      *fontfamily = 2;
      if (strstr(lowerfontstr, "large"))
	  return mo_large_newcentury;
      if (strstr(lowerfontstr, "regular"))
	  return mo_regular_newcentury;
      if (strstr(lowerfontstr, "small"))
	  return mo_small_newcentury;
      return mo_regular_newcentury;
  }
  if (strstr(lowerfontstr, "lucida")) {
      *fontfamily = 3;
      if (strstr(lowerfontstr, "large"))
	  return mo_large_lucidabright;
      if (strstr(lowerfontstr, "regular"))
	  return mo_regular_lucidabright;
      if (strstr(lowerfontstr, "small"))
	  return mo_small_lucidabright;
      return mo_regular_lucidabright;
  }
  return mo_regular_fonts;
}  


static void kill_splash()
{
    if (splash_cc) {
        ReleaseSplashColors(splash);
    } else {
	XtPopdown(splash);
    }
    XtDestroyWidget(splash);
    splash = NULL;
}
                  

static void mo_sync_windows(mo_window *win, mo_window *parent)
{
    win->font_size = parent->font_size;
    mo_set_fonts(win, parent->font_size);

    win->underlines_state = parent->underlines_state;
    mo_set_underlines(win, parent->underlines_state);

    win->agent_state = parent->agent_state;
    mo_set_agents(win, win->agent_state);

    imageViewInternal = win->image_view_internal = parent->image_view_internal;
    XmxRSetToggleState(win->menubar, mo_image_view_internal,
                       (win->image_view_internal ? XmxSet : XmxNotSet));
    
    win->preferences = parent->preferences;
    XmxRSetToggleState(win->menubar, mo_preferences,
                       win->preferences ? XmxSet : XmxNotSet);

    tableSupportEnabled = win->table_support = parent->table_support;
    XmxRSetToggleState(win->menubar, mo_table_support,
                       win->table_support ? XmxSet : XmxNotSet);

    win->frame_support = parent->frame_support;
    XtVaSetValues(win->scrolled_win,
                  WbNframeSupport,
                  win->frame_support,
                  NULL);
    XmxRSetToggleState(win->menubar, mo_frame_support,
                       win->frame_support ? XmxSet : XmxNotSet);

    win->hotkeys = parent->hotkeys;
    XmxRSetToggleState(win->menubar, mo_hotkeys,
                       win->hotkeys ? XmxSet : XmxNotSet);

    XmxRSetToggleState(win->menubar, mo_tooltips,
                       XmxClueIsActive() ? XmxSet : XmxNotSet);

    win->refresh_url = parent->refresh_url;
    XmxRSetToggleState(win->menubar, mo_refresh_url,
                       win->refresh_url ? XmxSet : XmxNotSet);

    progressiveDisplayEnabled = win->progressive_loads =
			parent->progressive_loads;
    XmxRSetToggleState(win->menubar, mo_progressive_loads,
                       win->progressive_loads ? XmxSet : XmxNotSet);

    win->image_animation = parent->image_animation;
    XmxRSetToggleState(win->menubar, mo_animate_images,
                       win->image_animation ? XmxSet : XmxNotSet);

    win->min_animation_delay = parent->min_animation_delay;

    win->safe_colors = parent->safe_colors;
    XmxRSetToggleState(win->menubar, mo_safe_colors,
                       win->safe_colors ? XmxSet : XmxNotSet);

    win->blink_text = parent->blink_text;
    XtVaSetValues(win->scrolled_win,
                  WbNblinkingText,
                  win->blink_text,
                  NULL);
    XmxRSetToggleState(win->menubar, mo_blink_text,
                       win->blink_text ? XmxSet : XmxNotSet);

    win->body_color = parent->body_color;
    XtVaSetValues(win->scrolled_win,
                  WbNbodyColors,
                  win->body_color,
                  NULL);
    XmxRSetToggleState(win->menubar, mo_body_color,
                       win->body_color ? XmxSet : XmxNotSet);

    win->body_images = parent->body_images;
    XtVaSetValues(win->scrolled_win,
                  WbNbodyImages,
                  win->body_images,
                  NULL);
    XmxRSetToggleState(win->menubar, mo_body_images,
                       win->body_images ? XmxSet : XmxNotSet);

    win->font_color = parent->font_color;
    XtVaSetValues(win->scrolled_win,
                  WbNfontColors,
                  win->font_color,
                  NULL);
    XmxRSetToggleState(win->menubar, mo_font_color,
                       win->font_color ? XmxSet : XmxNotSet);

    win->font_sizes = parent->font_sizes;
    XtVaSetValues(win->scrolled_win,
                  WbNfontSizes,
                  win->font_sizes,
                  NULL);
    XmxRSetToggleState(win->menubar, mo_font_sizes,
                       win->font_sizes ? XmxSet : XmxNotSet);

    win->form_button_bg = parent->form_button_bg;
    XtVaSetValues(win->scrolled_win,
		  WbNformButtonBackground,
		  win->form_button_bg,
                  WbNblinkTime,
                  get_pref_int(eBLINK_TIME),
                  WbNfontBase,
                  get_pref_int(eFONTBASESIZE),
                  NULL);

    win->delay_image_loads = parent->delay_image_loads;
    XmxSetArg(WbNdelayImageLoads, win->delay_image_loads ? True : False);
    XmxSetValues(win->scrolled_win);
    XmxRSetSensitive(win->menubar, mo_expand_images_current,
                     win->delay_image_loads ? XmxSensitive : XmxNotSensitive);
    XmxRSetToggleState(win->menubar, mo_delay_image_loads,
                       win->delay_image_loads ? XmxSet : XmxNotSet);
#ifdef HAVE_SSL
    XmxRSetToggleState(win->menubar, mo_verify_certs,
                       verifyCertificates ? XmxSet : XmxNotSet);
#endif

#ifndef DISABLE_TRACE
    if (get_pref_boolean(eDEBUG_MENU)) {
	XmxRSetToggleState(win->menubar, mo_trace_cache,
                           cacheTrace ? XmxSet : XmxNotSet);
#ifdef CCI
	XmxRSetToggleState(win->menubar, mo_trace_cci,
                           cciTrace ? XmxSet : XmxNotSet);
#endif
	XmxRSetToggleState(win->menubar, mo_trace_cookie,
                           cookieTrace ? XmxSet : XmxNotSet);
	XmxRSetToggleState(win->menubar, mo_trace_html,
                           htmlwTrace ? XmxSet : XmxNotSet);
	XmxRSetToggleState(win->menubar, mo_trace_http,
                           httpTrace ? XmxSet : XmxNotSet);
	XmxRSetToggleState(win->menubar, mo_trace_nut,
                           nutTrace ? XmxSet : XmxNotSet);
	XmxRSetToggleState(win->menubar, mo_trace_src,
                           srcTrace ? XmxSet : XmxNotSet);
	XmxRSetToggleState(win->menubar, mo_trace_table,
                           tableTrace ? XmxSet : XmxNotSet);
	XmxRSetToggleState(win->menubar, mo_trace_www2,
                           www2Trace ? XmxSet : XmxNotSet);
	XmxRSetToggleState(win->menubar, mo_trace_refresh,
                           refreshTrace ? XmxSet : XmxNotSet);
	XmxRSetToggleState(win->menubar, mo_report_bugs,
                           reportBugs ? XmxSet : XmxNotSet);
    }
#endif
}

/****************************************************************************
 * name:    mo_open_window_internal (PRIVATE)
 * purpose: Make a mo_window struct and fill up the GUI.
 * inputs:  
 *   - Widget       base: The dialog widget on which this window is 
 *                        to be based.
 *   - mo_window *parent: The parent mo_window struct for this window,
 *                        if one exists; this can be NULL.
 * returns: 
 *   The new window (mo_window *).
 * remarks: 
 *   This routine must set to 0 all elements in the mo_window struct
 *   that can be tested by various routines to see if various things
 *   have been done yet (popup windows created, etc.).
 ****************************************************************************/
static mo_window *mo_open_window_internal(Widget base, mo_window *parent)
{
  mo_window *win;
  HTMLWidget hw;
  XColor color;
  int i;

  win = (mo_window *)calloc(1, sizeof(mo_window));
  win->id = XmxMakeNewUniqid();
  XmxSetUniqid(win->id);

  win->base = base;
  win->mode = moMODE_PLAIN;

  for (i = 0; i < BTN_COUNT; i++)
      win->tools[i].gray = XmxSensitive;

  /****** calloc sets them right  
  win->source_win = 0;
  win->save_win = 0;
  win->upload_win = 0;
  win->savebinary_win = 0;
  win->ftpput_win = win->ftpremove_win = win->ftpmkdir_win = 0;

  win->open_win = win->open_text = win->open_local_win = 0;
  win->mail_win = win->mailhot_win = win->edithot_win = win->mailhist_win =
		  win->inserthot_win = 0;
  win->print_win = 0;
  win->history_win = win->history_list = 0;
  win->hotlist_win = win->hotlist_list = 0;
  win->techsupport_win = win->techsupport_text = 0;
  win->mailto_win = win->mailto_text = 0;
  win->mailto_form_win = win->mailto_form_text = 0;
  win->post_data = 0;
  win->news_win = 0;
  win->links_win = 0;
  win->news_fsb_win = 0;
  win->mail_fsb_win = 0;
  win->annotate_win = 0;
  win->search_win = win->search_win_text = 0;
  win->searchindex_win = win->searchindex_win_label =
			 win->searchindex_win_text = 0;
  win->src_search_win = 0;
  win->src_search_win_text = 0;
#ifdef CCI
  win->cci_win = win->cci_win_text = (Widget) 0;
  win->cci_accept_toggle = win->cci_off_toggle = (Widget) 0;
#endif
#ifdef HAVE_AUDIO_ANNOTATIONS
  win->audio_annotate_win = 0;
#endif

  win->image_file = NULL;
  win->cached_url = NULL;
  win->history = NULL;
  win->current_node = 0;
  win->reloading = 0;
  win->source_text = 0;
  win->format_optmenu = 0;
  win->save_format = 0;

  win->underlines_snarfed = 0;
  ******/

  if (!parent) {
      HTSetCookies = get_pref_boolean(eCOOKIES);
      HTEatAllCookies = get_pref_boolean(eACCEPT_ALL_COOKIES);
      HTCookieFile = get_pref_boolean(eUSE_COOKIE_FILE);
      win->font_size = mo_get_font_size_from_res(
		    get_pref_string(eDEFAULT_FONT_CHOICE), &(win->font_family));
  } else {
      win->font_size = parent->font_size;
      win->font_family = parent->font_family;
  }

  win->agent_state = selectedAgent + mo_last_entry;

  if (!parent) {
      char *defunline = get_pref_string(eDEFAULTUNDERLINES);

      if (!get_pref_boolean(eUSE_PREFERENCES)) {
          win->underlines_state = mo_default_underlines;
      } else if (!my_strcasecmp(defunline, "Default")) {
          win->underlines_state = mo_default_underlines;      
      } else if (!my_strcasecmp(defunline, "Light")) {
          win->underlines_state = mo_l1_underlines;      
      } else if (!my_strcasecmp(defunline, "Medium")) {
          win->underlines_state = mo_l2_underlines;      
      } else if (!my_strcasecmp(defunline, "Heavy")) {
          win->underlines_state = mo_l3_underlines;      
      } else if (!my_strcasecmp(defunline, "No")) {
          win->underlines_state = mo_no_underlines;      
      } else {
          fprintf(stderr,
	         "Error: Preference DEFAULTUNDERLINES has invalid value '%s'\n",
	         defunline); 
          fprintf(stderr, "       Using default underlines\n"); 
          win->underlines_state = mo_default_underlines;
      }
  } else {
      win->underlines_state = parent->underlines_state;
  }

  win->pretty = get_pref_boolean(eDEFAULT_FANCY_SELECTIONS);

  /****** calloc gets them set
  win->mail_format = 0;

#ifdef HAVE_AUDIO_ANNOTATIONS
  win->record_fnam = 0;
  win->record_pid = 0;
#endif
  
  win->print_text = 0;
  win->print_format = 0;

  win->target_anchor = 0;
  ******/

  /* Create search_start and search_end. */
  win->search_start = (void *)malloc(sizeof(ElementRef));
  win->search_end = (void *)malloc(sizeof(ElementRef));
  win->src_search_pos = 0;

#ifdef ISINDEX
  /* We don't know yet. */
  win->keyword_search_possible = -1;
#endif
  
  if (get_pref_boolean(eSECURITYICON) && win->current_node)
      mo_gui_check_win_security_icon(win->current_node->authType, win);

  win->delay_image_loads = get_pref_boolean(eDELAY_IMAGE_LOADS);

  /* Install all the GUI bits & pieces. */
  mo_fill_window(win);

  /* Get browser safe colors, now that GUI colors allocated */
  if ((Vclass == TrueColor) &&
      !get_pref_boolean(eBROWSERSAFECOLORS_IF_TRUECOLOR)) {
      browserSafeColors = win->safe_colors = False;
  } else {
      browserSafeColors = win->safe_colors =
      					 get_pref_boolean(eBROWSER_SAFE_COLORS);
  }
  if (browserSafeColors && !get_safe_colors(toplevel)) {
      browserSafeColors = win->safe_colors = False;
      set_pref_boolean(eBROWSER_SAFE_COLORS, win->safe_colors);
  }
  XmxRSetToggleState(win->menubar, mo_safe_colors,
                     (win->safe_colors ? XmxSet : XmxNotSet));

  win->image_animation = get_pref_boolean(eIMAGE_ANIMATION);
  win->min_animation_delay = get_pref_int(eMIN_ANIMATION_DELAY);

  /* Stop the SetValues routine from messing around */
  hw = (HTMLWidget) win->scrolled_win;
  hw->html.ignore_setvalues = 1;

  win->blink_text = get_pref_boolean(eBLINKING_TEXT);
  XtVaSetValues(win->scrolled_win,
                WbNblinkingText,
                win->blink_text,
                NULL);

  win->body_color = get_pref_boolean(eBODYCOLORS);
  XtVaSetValues(win->scrolled_win,
                WbNbodyColors,
                win->body_color,
                NULL);

  win->body_images = get_pref_boolean(eBODYIMAGES);
  XtVaSetValues(win->scrolled_win,
                WbNbodyImages,
                win->body_images,
                NULL);

  win->font_color = get_pref_boolean(eFONTCOLORS);
  XtVaSetValues(win->scrolled_win,
                WbNfontColors,
                win->font_color,
                NULL);

  win->font_sizes = get_pref_boolean(eFONTSIZES);
  XtVaSetValues(win->scrolled_win,
                WbNfontSizes,
                win->font_sizes,
                NULL);
  XmxRSetToggleState(win->menubar, win->font_size, XmxSet);
          
  /* Get the form buttom background color */
  XParseColor(dsp, DefaultColormapOfScreen(XtScreen(win->base)),
	      get_pref_string(eFORM_BUTTON_BACKGROUND), &color);
  XAllocColor(dsp, (installed_colormap ? installed_cmap :
		    DefaultColormapOfScreen(XtScreen(win->base))),
	      &color);
  win->form_button_bg = color.pixel;
  XtVaSetValues(win->scrolled_win,
		WbNformButtonBackground,
		win->form_button_bg,
                NULL);

  XtVaSetValues(win->scrolled_win,
                WbNblinkTime,
                get_pref_int(eBLINK_TIME),
                WbNfontBase,
                get_pref_int(eFONTBASESIZE),
                NULL);

  /* Setup news default states */
  ConfigView = !get_pref_boolean(eUSETHREADVIEW);
  newsShowAllGroups = get_pref_boolean(eSHOWALLGROUPS);
  newsShowReadGroups = get_pref_boolean(eSHOWREADGROUPS);
  newsShowAllArticles = get_pref_boolean(eSHOWALLARTICLES);
  newsNoThreadJumping = get_pref_boolean(eNOTHREADJUMPING);
  gui_news_updateprefs(win);
   
  /****** calloc sets them right
  win->have_focus = False;
  win->is_frame = 0;
  win->new_node = False;
  win->next_frame = NULL;
  win->do_frame = NULL;
  win->frames = NULL;
  win->frameurl = NULL;
  win->framename = NULL;
  win->framelast_modified = NULL;
  win->frameexpires = NULL;
  win->framecharset = NULL;
  ******/

  win->binary_transfer = 0;
  XmxRSetToggleState(win->menubar, mo_binary_transfer,
                     (win->binary_transfer ? XmxSet : XmxNotSet));
  win->binary_ftp_mode = 1;
  set_pref_boolean(eFTP_BINARY_MODE, win->binary_ftp_mode);
  XmxRSetToggleState(win->menubar, mo_binary_ftp_mode,
                     (win->binary_ftp_mode ? XmxSet : XmxNotSet));

  XmxRSetToggleState(win->menubar, mo_delay_image_loads,
                     (win->delay_image_loads ? XmxSet : XmxNotSet));
  XmxRSetSensitive(win->menubar, mo_expand_images_current,
                   win->delay_image_loads ? XmxSensitive : XmxNotSensitive);
  XmxRSetSensitive(win->menubar, mo_annotate, XmxSensitive);
  XmxRSetSensitive(win->menubar, mo_annotate_edit, XmxNotSensitive);
  XmxRSetSensitive(win->menubar, mo_annotate_delete, XmxNotSensitive);

  win->preferences = get_pref_boolean(eUSE_PREFERENCES);
  XmxRSetToggleState(win->menubar, mo_preferences,
                     (win->preferences ? XmxSet : XmxNotSet));

  tableSupportEnabled = win->table_support = get_pref_boolean(eENABLE_TABLES);
  XmxRSetToggleState(win->menubar, mo_table_support,
                     (win->table_support ? XmxSet : XmxNotSet));

  win->frame_support = get_pref_boolean(eFRAME_SUPPORT);
  XtVaSetValues(win->scrolled_win,
                WbNframeSupport,
                win->frame_support,
                NULL);
  XmxRSetToggleState(win->menubar, mo_frame_support,
                     (win->frame_support ? XmxSet : XmxNotSet));

  win->hotkeys = get_pref_boolean(eHOTKEYS);
  XmxRSetToggleState(win->menubar, mo_hotkeys,
                     (win->hotkeys ? XmxSet : XmxNotSet));

  XmxRSetToggleState(win->menubar, mo_tooltips,
                     XmxClueIsActive() ? XmxSet : XmxNotSet);

  win->refresh_url = get_pref_boolean(eREFRESH_URL);
  XmxRSetToggleState(win->menubar, mo_refresh_url,
                     (win->refresh_url ? XmxSet : XmxNotSet));

  progressiveDisplayEnabled = win->progressive_loads =
			get_pref_boolean(ePROGRESSIVE_DISPLAY);
  XmxRSetToggleState(win->menubar, mo_progressive_loads,
                     (win->progressive_loads ? XmxSet : XmxNotSet));

  imageViewInternal = win->image_view_internal =
			get_pref_boolean(eIMAGEVIEWINTERNAL);
  XmxRSetToggleState(win->menubar, mo_image_view_internal,
                     (win->image_view_internal ? XmxSet : XmxNotSet));

#ifdef HAVE_SSL
  verifyCertificates = get_pref_boolean(eVERIFY_SSL_CERTIFICATES);
  XmxRSetToggleState(win->menubar, mo_verify_certs,
                     (verifyCertificates ? XmxSet : XmxNotSet));
#endif

#ifndef DISABLE_TRACE
  if (get_pref_boolean(eDEBUG_MENU)) {
      XmxRSetToggleState(win->menubar, mo_trace_cache,
                         cacheTrace ? XmxSet : XmxNotSet);
#ifdef CCI
      XmxRSetToggleState(win->menubar, mo_trace_cci,
                         cciTrace ? XmxSet : XmxNotSet);
#endif
      XmxRSetToggleState(win->menubar, mo_trace_cookie,
                         cookieTrace ? XmxSet : XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_trace_html,
                         htmlwTrace ? XmxSet : XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_trace_http,
                         httpTrace ? XmxSet : XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_trace_nut,
                         nutTrace ? XmxSet : XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_trace_src,
                         srcTrace ? XmxSet : XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_trace_table,
                         tableTrace ? XmxSet : XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_trace_www2,
                         www2Trace ? XmxSet : XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_trace_refresh,
                         refreshTrace ? XmxSet : XmxNotSet);
      XmxRSetToggleState(win->menubar, mo_report_bugs,
                         reportBugs ? XmxSet : XmxNotSet);
  }
#endif

  /* Take care of session history for rbm */

  if (get_pref_boolean(eSESSION_HISTORY_ON_RBM)) {
      win->session_menu = NULL;
      win->num_session_items = 0;
      win->session_items = malloc(sizeof(Widget) * 
			         get_pref_int(eNUMBER_OF_ITEMS_IN_RBM_HISTORY));
  }

  /* Pop the window up. */
  XtPopup(win->base, XtGrabNone);
  XFlush(dsp);
  XSync(dsp, False);
  
  /* Register win with internal window list. */
  mo_add_window_to_list(win);

  /* Set the font size. */
  mo_set_fonts(win, win->font_size);

  /* Set the underline state. */
  mo_set_underlines(win, win->underlines_state);

  mo_set_agents(win, win->agent_state);

  /* Set the fancy selections toggle to the starting value. */
  mo_set_fancy_selections_toggle(win);

  if (parent) {
#ifndef DISABLE_TRACE
      if (srcTrace)
          fprintf(stderr, "Window SYNCing\n");
#endif
      mo_sync_windows(win, parent);
  }
  
  hw->html.ignore_setvalues = 0;

  return win;
}


/****************************************************************************
 * name:    delete_cb (PRIVATE)
 * purpose: Callback for the WM_DELETE_WINDOW protocol.
 * inputs:  
 *   - as per XmxCallback
 * returns: 
 *   nothing
 * remarks: 
 *   By the time we get called here, the window has already been popped
 *   down.  Just call mo_delete_window to clean up.
 ****************************************************************************/
static XmxCallback(delete_cb)
{
  mo_window *win = (mo_window *)client_data;

  mo_delete_window(win);
  return;
}


/****************************************************************************
 * name:    mo_make_window (PRIVATE)
 * purpose: Make a new window from scratch.
 * inputs:  
 *   - Widget      parent: Parent for the new window shell (always toplevel)
 *   - mo_window *parentw: Parent window, if one exists (may be NULL).
 * returns: 
 *   The new window (mo_window *).
 * remarks: 
 *   The 'parent window' parentw is the window being cloned, or the
 *   window in which the 'new window' command was triggered, etc.
 *   Some GUI properties are inherited from it, if it exists (fonts,
 *   anchor appearance, etc.).
 ****************************************************************************/
static mo_window *mo_make_window(Widget parent, mo_window *parentw)
{
  Widget base;
  mo_window *win;
  Atom WM_DELETE_WINDOW;
  char buf[80];
  Pixmap icon_pixmap = (Pixmap)NULL;

  sprintf(pre_title, "VMS Mosaic %s", MO_VERSION_STRING);
  sprintf(buf, "%s: ", pre_title);
  XmxSetArg(XmNtitle, (long)buf);
  XmxSetArg(XmNiconName, (long)"Mosaic");
  XmxSetArg(XmNallowShellResize, False);

  if (!icon_pixmap) {
      XIconSize *size_list;
      int num_sizes, biggest, i;
      int best_size = 32;

      if (XGetIconSizes(dsp, DefaultRootWindow(dsp), &size_list, &num_sizes)) {
	  biggest = 0;
	  for (i = 1; i < num_sizes; i++) {
	      if ((size_list[i].max_width >= size_list[biggest].max_width) &&
		  (size_list[i].max_height >= size_list[biggest].max_height))
		  biggest = i;
	  }
	  if ((size_list[biggest].max_width >= 75) &&
	      (size_list[biggest].max_height >= 75)) {
	      best_size = 75;
	  } else if ((size_list[biggest].max_width >= 50) &&
	      (size_list[biggest].max_height >= 50)) {
	      best_size = 50;
	  }
	  XFree(size_list);
      }
      if (best_size == 32) {
	  icon_pixmap = XCreateBitmapFromData(dsp, XDefaultRootWindow(dsp),
					      xmosaic_32_icon_bits,
					      xmosaic_32_icon_width,
					      xmosaic_32_icon_height);
      } else if (best_size == 50) {
	   icon_pixmap = XCreateBitmapFromData(dsp, XDefaultRootWindow(dsp),
					       xmosaic_icon_bits,
					       xmosaic_icon_width,
					       xmosaic_icon_height);
      } else {
	   icon_pixmap = XCreateBitmapFromData(dsp, XDefaultRootWindow(dsp),
					       xmosaic_75_icon_bits,
					       xmosaic_75_icon_width,
					       xmosaic_75_icon_height);
      }
  }
  XmxSetArg(XmNiconPixmap, icon_pixmap);

  if (installed_colormap)
      XmxSetArg(XmNcolormap, installed_cmap);

  base = XtCreatePopupShell("shell", topLevelShellWidgetClass,
                            parent, Xmx_wargs, Xmx_n);
  Xmx_n = 0;

#ifdef EDITRES_SUPPORT  
  XtAddEventHandler(base, (EventMask) 0, TRUE,
                    (XtEventHandler) _XEditResCheckMessages, NULL);
#endif

  XtOverrideTranslations(base, XtParseTranslationTable(toplevel_translations));

  win = mo_open_window_internal(base, parentw);

  WM_DELETE_WINDOW = XmInternAtom(dsp, "WM_DELETE_WINDOW", False);
  XmAddWMProtocolCallback(base, WM_DELETE_WINDOW, delete_cb, (XtPointer)win);

  return win;
}


/****************************************************************************
 * name:    mo_open_another_window_internal (PRIVATE)
 * purpose: Open a new window from an existing window.
 * inputs:  
 *   - mo_window *win: The old window.
 * returns: 
 *   The new window (mo_window *).
 * remarks: 
 *   This routine handles (optional) autoplace of new windows.
 ****************************************************************************/
static mo_window *mo_open_another_window_internal(mo_window *win)
{
  Dimension oldx, oldy;
  Dimension scrx = WidthOfScreen(XtScreen(win->base));
  Dimension scry = HeightOfScreen(XtScreen(win->base));
  Dimension x, y;
  Dimension width, height;
  mo_window *newwin;

  XtVaGetValues(win->base, XmNx, &oldx, XmNy, &oldy,
                XmNwidth, &width, XmNheight, &height, NULL);

  /* Ideally we open down and over 40 pixels... is this possible? */
  /* If not, deal with it... */

  /* Bug fix, thanks to Ken Shores <kss1376@pop.draper.com> */
  
  /* The original test did not handle the case where the old window
   * was exactly the same size as the screen.  Also, it used a looping
   * algorithm which would infinite loop under such a case. */
  
  if ((oldx + width) > (scrx - 40)) {
      x = (scrx - (oldx + width));
  } else {
      x = oldx + 40;
  }  
  if ((oldy + height) > (scry - 40)) {
      y = (scry - (oldy + height));
  } else {
      y = oldy + 40;
  }
  if (x > scrx)
      x = 0;
  if (y > scry)
      y = 0;
  
  XmxSetArg(XmNdefaultPosition, False);
  if (get_pref_boolean(eAUTO_PLACE_WINDOWS)) {
      char geom[20];

      sprintf(geom, "+%d+%d", x, y);
      XmxSetArg(XmNgeometry, (long)geom);
  }
  XmxSetArg(XmNwidth, width);
  XmxSetArg(XmNheight, height);

  newwin = mo_make_window(toplevel, win);
  mo_set_current_cached_win(newwin);
  return newwin;
}


/****************************************************************************
 * name:    mo_duplicate_window
 * purpose: Clone a existing window as intelligently as possible.
 * inputs:  
 *   - mo_window *win: The existing window.
 * returns: 
 *   The new window.
 * remarks: 
 *   
 ****************************************************************************/
mo_window *mo_duplicate_window(mo_window *win)
{
  mo_window *neww;

  if (win && win->current_node)
      securityType = win->current_node->authType;

  neww = mo_open_another_window_internal(win);
  
  mo_duplicate_window_text(win, neww);

  mo_gui_update_meter(100, NULL);

  return neww;
}


/****************************************************************************
 * name:    mo_open_another_window
 * purpose: Open another window to view a given URL, unless the URL
 *          indicates that it's pointless to do so
 * inputs:  
 *   - mo_window      *win: The existing window.
 *   - char           *url: The URL to view in the new window.
 *   - char           *ref: The reference (hyperlink text contents) for this
 *                          URL; can be NULL.
 *   - char *target_anchor: The target anchor to view open opening the
 *                          window, if any.
 * returns: 
 *   The new window.
 * remarks: 
 *   
 ****************************************************************************/
mo_window *mo_open_another_window(mo_window *win, char *url, char *ref,
                                  char *target_anchor)
{
  mo_window *neww;

  /* Check for reference to telnet.  Never open another window
     if reference to telnet exists; instead, call mo_load_window_text,
     which knows how to manage current window's access to telnet. */
  if (!my_strncasecmp(url, "telnet:", 7) ||
      !my_strncasecmp(url, "tn3270:", 7) ||
      !my_strncasecmp(url, "rlogin:", 7)) {
      mo_load_window_text(win, url, NULL);
      return NULL;
  }

  neww = mo_open_another_window_internal(win);
  /* Set it here; hope it gets handled in mo_load_window_text_first
     (it probably won't, now.) */
  neww->target_anchor = target_anchor;

  mo_load_window_text(neww, url, ref);

#ifdef CCI
  if (cci_get && (return_stat == mo_fail))
      return (mo_window *) NULL;
#endif

  return neww;
}


/* ------------------------------------------------------------------------ */

char **gargv;
int gargc;

/****************************************************************************
 * name:    fire_er_up (PRIVATE)
 * purpose: Callback from timer that actually starts up the application,
 *          i.e., opens the first window.
 * inputs:  
 *   - as per XmxCallback
 * returns: 
 *   Nothing.
 * remarks: 
 *   This routine figures out what the home document should be
 *   and then opens a window.
 ****************************************************************************/
static XmxCallback(fire_er_up)
{
  char *home_opt;
  mo_window *win;
  char *init_document;
  char *fname;
  char *tmp;
#ifdef PRERELEASE
  int cnt = 0;
#endif

  /* Value of environment variable WWW_HOME overrides preference. */
  if (home_opt = getenv("WWW_HOME")) {
      home_document = strdup(home_opt);
  } else if (tmp = get_pref_string(eHOME_DOCUMENT)) {
      home_document = strdup(tmp);
  } else {
      home_document = strdup("");
  }
#ifdef PRERELEASE
  /*
   * If this is a pre-release, go to the help-on-version doc for three
   *   start ups.  Then, go to the Pre-Release warning page for three
   *   start ups.  Then go to their defined page or the Mosaic home page.
   */
  if ((cnt = GetCardCount((fname = MakeFilename()))) < MO_GO_NCSA_COUNT) {
      init_document = strdup(MO_HELP_ON_VERSION_DOCUMENT);
  } else if (cnt < (MO_GO_NCSA_COUNT * 2)) {
      init_document = strdup(
          "http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/NewPrereleaseWarningPage.html");
  } else {
      init_document = strdup(home_document);
  }
#else
  /*
   * If this is not a pre-release, go to the help-on-version doc for three
   *   start ups. Then go to their defined page or the Mosaic home page.
   */
  if (GetCardCount((fname = MakeFilename())) < MO_GO_NCSA_COUNT) {
      init_document = strdup(MO_HELP_ON_VERSION_DOCUMENT);
  } else {
      init_document = strdup(home_document);
  }
#endif

  if (fname)
      free(fname);

#ifdef VMS
  /* Value specified on command line overrides everything. */
  if (cmdline_homeDocument) {
      free(home_document);
      home_document = cmdline_homeDocument;
  }
#endif /* VMS, LLL */

  /* Value of argv[1], if it exists, sets startup_document.
   * (All other command-line flags will have been picked up by
   * the X resource mechanism.) */
  /* Unless they are bogus options - then they will break... */
  if (gargc > 1 && gargv[1] && *gargv[1])
      startup_document = mo_url_prepend_protocol(gargv[1]);

  /* Check for proper home document URL construction. */
  if (!strchr(home_document, ':')) {
      tmp = home_document;
      home_document = mo_url_canonicalize_local(home_document);
      free(tmp);
  }

  /* Check for proper init document URL construction. */
  if (!strchr(init_document, ':')) {
      tmp = init_document;
      init_document = mo_url_canonicalize_local(init_document);
      free(tmp);
  }

  /* Set the geometry values */
  if (!userSpecifiedGeometry) {
      /* Then no -geometry was specified on the command line,
       * so we just use the default values from the resources */
      int width = WidthOfScreen(XtScreen(toplevel));
      int height = HeightOfScreen(XtScreen(toplevel));

      /* Limit to screen size */
      if (get_pref_int(eDEFAULT_WIDTH) < width)
	  width = get_pref_int(eDEFAULT_WIDTH);
      if (get_pref_int(eDEFAULT_HEIGHT) < height)
	  height = get_pref_int(eDEFAULT_HEIGHT);

      XmxSetArg(XmNwidth, width);
      XmxSetArg(XmNheight, height);
#ifndef DISABLE_TRACE
      if (srcTrace)
	  fprintf(stderr, "Set default geometry: W=%d, H=%d\n", width, height);
#endif
  } else {
      /* They DID specify a -geometry, so we use that */
      /* Don't place limits on what the user trys */
      XmxSetArg(XmNwidth, userWidth);
      XmxSetArg(XmNheight, userHeight);
      XmxSetArg(XmNx, userX);
      XmxSetArg(XmNx, userY);
#ifndef DISABLE_TRACE
      if (srcTrace)
	  fprintf(stderr, "Set specified geometry: W=%d, H=%d, X=%d, Y=%d\n",
		  userWidth, userHeight, userX, userY);
#endif
  }

  if (get_pref_boolean(eINITIAL_WINDOW_ICONIC) || iconic)
      XmxSetArg(XmNiconic, True);

  /* Get rid of it */
  if (do_splash && splash)
      kill_splash();

  /* Now open the window */
  win = mo_make_window(toplevel, NULL);
  mo_set_current_cached_win(win);
  mo_load_window_text(win,
		      startup_document ? startup_document : init_document,
		      NULL);
  free(init_document);

  /* Check the Comment Card */
#ifdef PRERELEASE
  do_comment = 0; /* Don't display the cc if we aren't in final release */
#endif
  CommentCard(win);

  XmxRSetToggleState(win->menubar, mo_animate_images,
                     (win->image_animation ? XmxSet : XmxNotSet));

  XmxRSetToggleState(win->menubar, mo_blink_text,
                     (win->blink_text ? XmxSet : XmxNotSet));

  XmxRSetToggleState(win->menubar, mo_body_color,
                     (win->body_color ? XmxSet : XmxNotSet));

  XmxRSetToggleState(win->menubar, mo_body_images,
                     (win->body_images ? XmxSet : XmxNotSet));

  XmxRSetToggleState(win->menubar, mo_font_color,
                     (win->font_color ? XmxSet : XmxNotSet));

  XmxRSetToggleState(win->menubar, mo_font_sizes,
                     (win->font_sizes ? XmxSet : XmxNotSet));

  /* Set focus policy of HTMLWidget according to preferences */
  HTMLSetFocusPolicy(win->scrolled_win, get_pref_boolean(eFOCUS_FOLLOWS_MOUSE));

  if (get_pref_boolean(eFOCUS_FOLLOWS_MOUSE))
      XtVaSetValues(toplevel, XmNkeyboardFocusPolicy, XmPOINTER, NULL);

  /*
   * Setup remote control.  Since a remote control command can come in
   * any time afterwards, we must have everything else ready.
   */
#ifndef VMS
  signal(SIGUSR1, (void *)ProcessExternalDirective);
#else
  if (use_mbx) {
      int i;

      /*
       * Reserve event flag and initialize mailbox on VMS for remote control.
       */
      i = lib$free_ef(&mbx_event_flag);
      i = lib$reserve_ef(&mbx_event_flag);
      if (i & 1) {
          mo_InputId = XtAppAddInput(app_context, mbx_event_flag, mbx_iosb,
                                     ProcessExternalDirective, NULL);
          InitExternalDirective(grp_mbx, mbx_name);
      }
  }
#endif /* VMS, BSN, TJA */

  return;
}


/****************************************************************************
 * name:    mo_open_initial_window
 * purpose: This routine is called when we know we want to open the
 *          initial Document View window.
 * inputs:  
 *   none
 * returns: 
 *   mo_succeed
 * remarks: This routine is simply a stub that sets a timeout that
 *          calls fire_er_up() after 10 milliseconds, which does the
 *          actual work.
 ****************************************************************************/
mo_status mo_open_initial_window(void)
{
  /* Set a timer that will actually cause the window to open. */
  XtAppAddTimeOut(app_context, 10, 
                  (XtTimerCallbackProc)fire_er_up, (XtPointer)True);

  return mo_succeed;
}


/****************************************************************************
 * name:    mo_error_handler (PRIVATE)
 * purpose: Handle X errors.
 * inputs:  
 *   - Display       *dsp: The X display.
 *   - XErrorEvent *event: The error event to handle.
 * returns: 
 *   0, if it doesn't force an exit.
 * remarks: 
 *   The main reason for this handler is to keep the application
 *   from crashing on BadAccess errors during calls to XFreeColors().
 ****************************************************************************/
static int mo_error_handler(Display *dsp, XErrorEvent *event)
{
  char buf[128];

  XUngrabPointer(dsp, CurrentTime);   /* In case error occurred in Grab */

  /* BadAlloc errors (on a XCreatePixmap() call) */
  if (event->error_code == BadAlloc) {
      if (LimDim == -1)
	  LimDim = 1;
      return 0;
  } else if ((event->error_code == BadPixmap) && (LimDim == 1)) {
      return 0;
  } else if ((event->error_code == BadWindow && event->request_code == 3) ||
             (event->error_code == BadAccess && event->request_code == 88)) {
      /* BadAccess errors on XFreeColors are 'ignoreable' errors
       * VMS: BadWindow error on XGetWindowAttributes is also ignored.
       * D. Potterveld, Argonne National Lab.
       */
      return 0;
  } else {
      /* All other errors are 'fatal'. */
      XGetErrorText(dsp, event->error_code, buf, 128);
      fprintf(stderr, "X Error: %s\n", buf);
      fprintf(stderr, "  Major Opcode:  %d\n", event->request_code);
      {
	char ans[10];

	printf("Caught X Error -- Press RETURN\n");
	gets(ans);
      }

      /* Try to close down gracefully. */
      mo_exit();
  }
  return 0; /* Never makes it here.... */
}


/****************************************************************************
 * name:    setup_imagekill
 * purpose: Read the imagekill file if it exists and fill in the
 *          imagekill_sites array
 * 
 * returns: 
 *   nothing
 * remarks: 
 *   
 ****************************************************************************/
static void setup_imagekill(void)
{
    char *home;
    char imageselect_file_pathname[512];
    FILE *fp;
    long i, j, num_delay_sites = 0, num_kill_sites = 0;
    int len;
    char buf[512];

    if (get_home(&home) || !home) {
	fprintf(stderr, "home: Could not get your home directory.\n");
	return;
    }
    
#ifndef VMS
    sprintf(imageselect_file_pathname, "%s/%s",
#else
    sprintf(imageselect_file_pathname, "%s%s",
#endif   /* VMS file name, PGE */
            home, get_pref_string(eIMAGEDELAY_FILE));

    free(home);

    /* Check to see if the file exists.  If it doesn't, then exit */
    if (!file_exists(imageselect_file_pathname))
        return;

    /* Try to open it */
    if (!(fp = fopen(imageselect_file_pathname, "r"))) {
        fprintf(stderr, "Error: Can't open image delay file for reading\n");
        return;
    }

    /* Read it */
    while (!(fgets(buf, 512, fp) == NULL)) {
        if (buf[0] == '#' || buf[0] == '\n') {
            continue;
        } else if (buf[0] == 'd' || buf[0] == 'D') {
            num_delay_sites++;
        } else if (buf[0] == 'k' || buf[0] == 'K') {
            num_kill_sites++;
	}
    }
    
    rewind(fp);

    imagekill_sites = (char **)malloc((num_kill_sites+1) * sizeof(char *));
    if (!imagekill_sites)
        return;

    imagedelay_sites = (char **)malloc((num_delay_sites+1) * sizeof(char *));
    if (!imagedelay_sites) {
        free(imagekill_sites);
        return;
    }

    i = j = 0;
    
    while (!(fgets(buf, 512, fp) == NULL)) {
        if (buf[0] == '#' || buf[0] == '\n') {
            continue;
        } else if (buf[0] == 'd' || buf[0] == 'D') {
            len = strlen(buf) - 6 - 1;   /* 6 == strlen("delay ") */
            imagedelay_sites[i] = (char *)malloc((len + 1) * sizeof(char));
            strncpy(imagedelay_sites[i], buf + 6, len);
            imagedelay_sites[i][len] = '\0';
            i++;
        } else if (buf[0] == 'k' || buf[0] == 'K') {
            len = strlen(buf) - 5 - 1;   /* 5 == strlen("kill ") */
            imagekill_sites[j] = (char *)malloc((len + 1) * sizeof(char));
            strncpy(imagekill_sites[j], buf + 5, len);
            imagekill_sites[j][len] = '\0';
            j++;
        }
    }
    imagedelay_sites[i] = NULL;
    imagekill_sites[j] = NULL;

    fclose(fp);
    
    return;
}


/****************************************************************************
 * name:    mo_do_gui
 * purpose: This is basically the real main routine of the application.
 * inputs:  
 *   - int    argc: Number of arguments.
 *   - char **argv: The argument vector.
 * returns: 
 *   nothing
 * remarks: 
 *   
 ****************************************************************************/
void mo_do_gui(int argc, char **argv)
{
#ifdef VMS
#define SYI$_HW_NAME 4362
#define SYI$_VERSION 4096
  int syi_hw_name = SYI$_HW_NAME;
  int syi_version = SYI$_VERSION;
  int j;
  char hardware[32], VMS_version[16], *cp;
  int status;
  unsigned short l_hardware, l_version;

  struct  dsc$descriptor_s {
    unsigned short  dsc$w_length;
    unsigned char   dsc$b_dtype;
    unsigned char   dsc$b_class;
    char            *dsc$a_pointer;
  } hardware_desc = {sizeof(hardware), 14, 1, NULL},
    VMS_version_desc = {sizeof(VMS_version), 14, 1, NULL};

    int no_preferences = 0;
#endif /* VMS, BSN, PGE, GEC */
#ifdef MONO_DEFAULT
    int use_color = 0;
#else
    int use_color = 1;
#endif
    int no_defaults = 0;
    int color_set = 0;
#ifndef VMS   /* PGE */
    char *display_name = getenv("DISPLAY");
#else
    char *display_name = "\0";
#endif
    Display *dpy;
    int i;
    char *tptr, *ptr, *eptr;
    Boolean successful;
    prefsStructP thePrefsStructP;
    int changed_visual = 0;
    int kiosk = 0;
    int kiosknoexit = 0;
    int kioskprint = 0;
    int noglohist = 0;
    int delay_images = 0;
    int ics = 0;
    char *tmpdir = NULL;

#ifdef VMS
    hardware_desc.dsc$a_pointer = hardware;
    VMS_version_desc.dsc$a_pointer = VMS_version;
    mbx_name[0] = '\0';

    /* Handle DCL style command line, GEC */
    status = vms_mosaic_cmdline(&argc, &argv);
    if (!(status & 1))
        exit(1);
#endif /* VMS */

    /* Loop through the args before passing them off to
     * XtAppInitialize() in case we need to catch something first.
     */
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-mono")) {
            use_color = 0;
            color_set = 1;
        } else if (!strcmp(argv[i], "-color")) {
            use_color = 1;
            color_set = 1;
        } else if (!strcmp(argv[i], "-dil")) {
            delay_images = 1;
        } else if (!strcmp(argv[i], "-display")) {
            if (argv[i + 1] == NULL) {
	        fprintf(stderr, "-display argument missing.\n");
	        return;
            }
            display_name = argv[i + 1];
            i++;
        } else if (!strcmp(argv[i], "-geometry")) {
            userSpecifiedGeometry = 1;
        } else if (!strcmp(argv[i], "-install")) {
	    installed_colormap = 1;
            argv[i] = NULL;
        } else if (!strcmp(argv[i], "-iconic")) {
	    splash_cc = 0;
	    iconic = 1;
        } else if (!strcmp(argv[i], "-ics")) {
            if (argv[i + 1] == NULL) {
	        fprintf(stderr, "-ics argument missing.\n");
	        return;
            }
            ics = atoi(argv[i + 1]);
            argv[i] = NULL;
            argv[i + 1] = NULL;
            i++;
        } else if (!strcmp(argv[i], "-kiosk")) {
	    kiosk = 1;
        } else if (!strcmp(argv[i], "-kiosknoexit")) {
	    kiosknoexit = 1;
        } else if (!strcmp(argv[i], "-kioskprint")) {
	    kioskprint = 1;
        } else if (!strcmp(argv[i], "-nd")) {
            no_defaults = 1;
        } else if (!strcmp(argv[i], "-ngh")) {
	    noglohist = 1;
        } else if (!strcmp(argv[i], "-nopref")) {
            no_preferences = 1;
            argv[i] = NULL;
        } else if (!strcmp(argv[i], "-mbx")) {
            use_mbx = 1;
            argv[i] = NULL;
        } else if (!strcmp(argv[i], "-mbx_grp")) {
            use_mbx = 1;
            grp_mbx = 1;
            argv[i] = NULL;
        } else if (!strcmp(argv[i], "-mbx_name")) {
            if (argv[i + 1] == NULL) {
	        fprintf(stderr, "-mbx_name argument missing.\n");
	        return;
            }
            use_mbx = 1;
            strcpy(mbx_name, argv[i + 1]);
            argv[i] = NULL;
            argv[i + 1] = NULL;
            i++;
        } else if (!strcmp(argv[i], "-tmpdir")) {
            if (argv[i + 1] == NULL) {
	        fprintf(stderr, "-tmpdir argument missing.\n");
	        return;
            }
            tmpdir = strdup(argv[i + 1]);
            argv[i] = NULL;
            argv[i + 1] = NULL;
            i++;
        /*
         *    Need to distinguish between Xresource homepage and
         *    commandline homepage; set a pointer to the commandline
         *    argument if present.  LLL
         */
        } else if (!strcmp(argv[i], "-home")) {
            if (argv[i + 1] == NULL) {
	        fprintf(stderr, "-home argument missing.\n");
	        return;
            }
            cmdline_homeDocument = strdup(argv[i + 1]);   
            argv[i] = NULL;                                   
            argv[i + 1] = NULL;
            i++;
        }
    }

    /*
     * Remove used arguments.
     */
    j = 0;
    for (i = 1; i < argc; i++) {
        if (argv[i] == NULL)
            continue;
        argv[++j] = argv[i];
    }
    argc = j + 1;

#ifdef VMS  /* PGE, register to allow multiple fonts in resource file */
    /* For some reason it has to be here and later, after XtAppInitialize */
    XtSetTypeConverter(XtRString, XtRFontStruct, &convertMultiFontStruct,
                       NULL, 0, XtCacheAll, NULL);
    XtSetTypeConverter(XtRString, XmRFontList, &convertMultiFontList,
                       NULL, 0, XtCacheAll, NULL);
#endif
    /* Motif setup */
    XmxStartup();
    XmxSetArg(XmNmappedWhenManaged, False);

#if XtSpecificationRelease > 4
    /* Register default language procedure */
    XtSetLanguageProc(NULL, NULL, NULL);
#endif

    /*
     * Awful expensive to open and close the display just to find
     * the depth information.
     */
    if (dpy = XOpenDisplay(display_name)) {
	int DefScreen = DefaultScreen(dpy);

	if (!color_set)
	    use_color = DisplayPlanes(dpy, DefScreen) > 1;
	LimDimX = DisplayWidth(dpy, DefScreen);
	LimDimY = DisplayHeight(dpy, DefScreen);

	XCloseDisplay(dpy);
    } else {
	fprintf(stderr, "Couldn't open display: %s\n",
		(!display_name ? "(NULL)" : display_name));
	return;
    }

    if (no_defaults) {
        toplevel = XtAppInitialize 
            (&app_context, "Mosaic", options, XtNumber(options),
             &argc, argv, NULL, Xmx_wargs, Xmx_n);
    } else {
        if (use_color) {
            toplevel = XtAppInitialize 
                (&app_context, "Mosaic", options, XtNumber(options),
                 &argc, argv, color_resources, Xmx_wargs, Xmx_n);
        } else {
            toplevel = XtAppInitialize 
                (&app_context, "Mosaic", options, XtNumber(options),
                 &argc, argv, mono_resources, Xmx_wargs, Xmx_n);
        }
    }

    paste_init(app_context); /* Paste URL initialization */

#ifdef VMS  /* PGE, register to allow multiple fonts in resource file */
    XtAppSetTypeConverter(app_context, XtRString, XtRFontStruct,
                          &convertMultiFontStruct,
                          NULL, 0, XtCacheAll, NULL);
    XtAppSetTypeConverter(app_context, XtRString, XmRFontList,
                          &convertMultiFontList,
                          NULL, 0, XtCacheAll, NULL);
#endif

    Xmx_n = 0;

    dsp = XtDisplay(toplevel);

    /* Initialize the preferences stuff */  
    successful = preferences_genesis();
    if (!successful) {  /* Should probably be generating an error here... */
        signal(SIGBUS, 0);
        signal(SIGSEGV, 0);
        signal(SIGILL, 0);
#ifndef VMS
        abort();
#else
	exit(44);
#endif
    }

    thePrefsStructP = get_ptr_to_preferences();

    /* First for the regular resources */
    XtVaGetApplicationResources(toplevel,
        			(XtPointer)thePrefsStructP->RdataP,
        			resources,
        			XtNumber(resources), NULL);

    if (thePrefsStructP->RdataP->app_defaults_version < 1) {
	fprintf(stderr,
		"Could not open MOSAIC.DAT.  This will cause Mosaic's\n");
	fprintf(stderr,
		"color scheme to display wrong.  Please see the Mosaic\n");
	fprintf(stderr,
		"installation instructions on how to create MOSAIC.DAT.\n");
    } else if (thePrefsStructP->RdataP->app_defaults_version < 2) {
	fprintf(stderr,
		"MOSAIC.DAT is an old version.  This will cause Mosaic's\n");
	fprintf(stderr,
	     "pulldown menus to display incorrectly.  Please see the Mosaic\n");
	fprintf(stderr,
		"installation instructions on how to replace MOSAIC.DAT.\n");
    }

/*
  {
    XrmDatabase intDB, appDB;
    Widget intWidget;
    appDB = XrmGetDatabase(dsp);

    if (Rdata.internationalFilename) {
        if ((intDB = XrmGetFileDatabase(Rdata.internationalFilename)) != NULL) {
	    XrmMergeDatabases(intDB, &appDB);
	    XrmSetDatabase(dsp, appDB);
        } else {
	    fprintf(stderr, "There was no language file called:\n  [%s]\n",
		    Rdata.internationalFilename);
        }
    }

    intWidget = XtVaCreateWidget("international", xmRowColumnWidgetClass,
			         toplevel, NULL);
    XtVaGetApplicationResources(intWidget, (XtPointer)&Idata, intResources,
			        XtNumber (intResources), NULL);
  }
*/

    /* Read the preferences file now */
    if (!no_preferences) {
        successful = read_preferences_file(NULL);
        if (!successful) {
            signal(SIGBUS, 0);
            signal(SIGSEGV, 0);
            signal(SIGILL, 0);
#ifndef VMS
            abort();
#else
	    exit(44);
#endif /* VMS, GEC */
        }
    }

    /* Do command line override of preferences */
    if (delay_images)
	set_pref_boolean(eDELAY_IMAGE_LOADS, True);
    if (ics)
	set_pref_int(eIMAGE_CACHE_SIZE, ics);
    if (kiosk)
	set_pref_boolean(eKIOSK, True);
    if (kiosknoexit)
	set_pref_boolean(eKIOSKNOEXIT, True);
    if (kioskprint)
	set_pref_boolean(eKIOSKPRINT, True);
    if (noglohist)
	set_pref_boolean(eUSE_GLOBAL_HISTORY, False);
    if (tmpdir)
	set_pref(eTMP_DIRECTORY, (void *)tmpdir);

#ifndef DISABLE_TRACE
    httpTrace = get_pref_boolean(eHTTPTRACE);
    www2Trace = get_pref_boolean(eWWW2TRACE);
    htmlwTrace = get_pref_boolean(eHTMLWTRACE);
#ifdef CCI
    cciTrace = get_pref_boolean(eCCITRACE);
#endif
    cookieTrace = get_pref_boolean(eCOOKIETRACE);
    srcTrace = get_pref_boolean(eSRCTRACE);
    cacheTrace = get_pref_boolean(eCACHETRACE);
    nutTrace = get_pref_boolean(eNUTTRACE);
    tableTrace = get_pref_boolean(eTABLETRACE);
    refreshTrace = get_pref_boolean(eREFRESHTRACE);
    reportBugs = get_pref_boolean(eREPORTBUGS);
    if (srcTrace)
	fprintf(stderr, "SRC tracing start.\n");

#else
    if (get_pref_boolean(eHTTPTRACE) ||
        get_pref_boolean(eWWW2TRACE) ||
        get_pref_boolean(eHTMLWTRACE) ||
#ifdef CCI
        get_pref_boolean(eCCITRACE) ||
#endif
        get_pref_boolean(eCOOKIETRACE) ||
        get_pref_boolean(eSRCTRACE) ||
        get_pref_boolean(eCACHETRACE) ||
        get_pref_boolean(eNUTTRACE) ||
        get_pref_boolean(eREFRESHTRACE) ||
        get_pref_boolean(eREPORTBUGS) ||
        get_pref_boolean(eTABLETRACE)) {
        fprintf(stderr, "Tracing has been compiled out of this binary.\n");
    }
#endif

    {
	char *serverName;

        serverName = ServerVendor(dsp);
        /* Force enable of max_clip_transitions if a VAX X-server */
        if ((strstr(serverName, "DECWINDOWS") &&
               strstr(serverName, "VAX"))
        /* Force enable of max_clip_transitions if a Multia X-server */
           || (strstr(serverName, "DECWINDOWS") &&
               strstr(serverName, "eXcursion"))
        /* Force enable of max_clip_transitions if a VXT2000 X-server */
           || (strstr(serverName, "DECWINDOWS") &&
               strstr(serverName, "VXT 2000"))
           || (!strcmp(serverName, "DECWINDOWS DigitalEquipmentCorp.") &&
               (VendorRelease(dsp) == 11))) {

            int tmp = get_pref_int(eMAX_CLIP_TRANSITIONS);

            if (tmp < 0) {
                tmp = 2048;
                set_pref(eMAX_CLIP_TRANSITIONS, (void *)&tmp);
            }
        }
    }

    /* Vclass needed for picread.c, pixmaps.c, etc. */
    {
        int cnt;

	theVisual = DefaultVisual(dsp, DefaultScreen(dsp));
	Vclass = theVisual->class;

	/* We do not support DirectColor properly, so try to find TrueColor */
	if (Vclass == DirectColor) {
	    int maxdepth = 0;
	    XVisualInfo vinfo;
	    XVisualInfo *vptr;

	    vinfo.class = TrueColor;
	    vinfo.screen = DefaultScreen(dsp);
	    vptr = XGetVisualInfo(dsp, VisualClassMask | VisualScreenMask,
				  &vinfo, &cnt);
	    while(cnt > 0) {
		if (vptr[cnt - 1].depth > maxdepth)
		    maxdepth = vptr[cnt - 1].depth;
		cnt--;
	    }
	    if (maxdepth >= 16) {
		if (XMatchVisualInfo(dsp, DefaultScreen(dsp), maxdepth,
				     TrueColor, &vinfo)) {
		    theVisual = vinfo.visual;
		    Vclass = vinfo.class;
		    changed_visual = 1;
		}
	    }
	    XFree((char *)vptr);

	    if (!changed_visual) {	/* Try PseudoColor */
		vinfo.class = PseudoColor;
		vptr = XGetVisualInfo(dsp, VisualClassMask | VisualScreenMask,
				      &vinfo, &cnt);
		maxdepth = 0;
		while(cnt > 0) {
		    if (vptr[cnt - 1].depth > maxdepth)
			maxdepth = vptr[cnt - 1].depth;
		    cnt--;
	    	}
		if (maxdepth >= 8) {
		    if (XMatchVisualInfo(dsp, DefaultScreen(dsp), maxdepth,
					 PseudoColor, &vinfo)) {
			theVisual = vinfo.visual;
			Vclass = vinfo.class;
			changed_visual = 1;
		    }
		}
		XFree((char *)vptr);
	    }

	    if (changed_visual)
		XtVaSetValues(toplevel, XmNvisual, theVisual, NULL);
	}
    }
  
    if (get_pref_boolean(eINSTALL_COLORMAP) || changed_visual)
	installed_colormap = 1;

    if (installed_colormap) {
	XColor bcolr;

	installed_cmap = XCreateColormap(dsp,
				         RootWindow(dsp, DefaultScreen(dsp)),
				         theVisual, AllocNone);
	XtVaGetValues(toplevel,
		      XtNbackground,
		      &(bcolr.pixel),
		      NULL);
	XQueryColor(dsp,
		    DefaultColormap(dsp, DefaultScreen(dsp)),
		    &bcolr);

	XtVaSetValues(toplevel,
		      XmNcolormap, installed_cmap,
		      NULL);

        XAllocColor(dsp,
		    installed_cmap,
		    &bcolr);
	XtVaSetValues(toplevel,
		      XmNbackground,
		      bcolr.pixel,
		      NULL);
    }

    /* Get the hostname. */
    machine = (char *)malloc(sizeof(char) * 64);
    gethostname(machine, 64);

#ifndef VMS
    uname(&mo_uname);
#else
    status = lib$getsyi((void *)&syi_hw_name, 0, &hardware_desc, &l_hardware,
	 		0, 0);
    status = lib$getsyi((void *)&syi_version, 0, &VMS_version_desc, &l_version,
	 		0, 0);
    hardware[l_hardware] = '\0';
    VMS_version[l_version] = '\0';
    for (cp = &VMS_version[l_version - 1]; VMS_version; cp--) {
        if (*cp != ' ')
	    break;
        *cp = '\0';
    }
#endif /* VMS, GEC */

    HTAppVersion = 
        (char *)malloc(sizeof(char) * (
            strlen(MO_VERSION_STRING) +
#ifndef VMS
            strlen(mo_uname.sysname) + 
            strlen(mo_uname.release) +
            strlen(mo_uname.machine) + 20));
#else
            strlen("OpenVMS") +
            strlen(VMS_version) +
            strlen(hardware) + 20));
#endif /* VMS, GEC */
    sprintf(HTAppVersion, "%s (Motif;%s %s %s)", 
            MO_VERSION_STRING,
#ifndef VMS
            mo_uname.sysname,
            mo_uname.release,
            mo_uname.machine);
#else
            "OpenVMS",
            VMS_version,
            hardware);
#endif /* VMS, GEC */

    XSetErrorHandler(mo_error_handler);

    /* Transient shell cannot be focussed, so no point in splash screen as it
     * will be complete psychadelic */
    if ((installed_colormap && !changed_visual) || !splash_cc) {
	do_splash = 0;
    } else {
	do_splash = get_pref_boolean(eSPLASHSCREEN);
    }

splash_goto:

    if (do_splash) {
        Pixmap splashpix;
        GC gc;
        XGCValues values;
        XColor ccell_fg;
        XWindowAttributes war;
        Widget sform, spixwid;
        XFontStruct *font;
        char s[64];
        int x, y;
        int l;
        int fontW, fontH;

        if (!XGetWindowAttributes(dsp, DefaultRootWindow(dsp), &war)) {
            fprintf(stderr,
		"Warning: Could not obtain your root window attributes.\n  Splash screen will not be centered.\n");
            x = y = 100;
        } else {
#if defined(VMS) && !defined(__DECC) && !defined(__GNUC__)
            x = (war.width / 2) - (126 / 2);
            y = (war.height / 2) - (126 / 2);
#else
            x = (war.width / 2) - (320 / 2);
            y = (war.height / 2) - (320 / 2);
#endif /* VAX C one is smaller, GEC */
        }

        /* GO GO MOSAIC SPLASH SCREEN - WHOOMP! */
        if (!(font = XLoadQueryFont(dsp,
              "-adobe-helvetica-medium-o-normal-*-*-180-*-*-p-*-iso8859-*"))) {
	    fprintf(stderr,
		"Warning: Cannot get font -adobe-helvetica-medium-o-normal-*-*-180-*-*-p-*-iso8859-*");
	    fprintf(stderr,
		"Warning: Splash Screen has been aborted.\n  Reason: Could not load version font.\n");
	    do_splash = 0;
	    goto splash_goto;
	}

        fontW = font->max_bounds.rbearing;
        fontH = font->max_bounds.ascent + font->max_bounds.descent;
      
        splash = XtVaCreatePopupShell("Hello, World!",
             xmMenuShellWidgetClass,
             toplevel,
#if defined(VMS) && !defined(__DECC) && !defined(__GNUC__)
             XmNheight, 126,
             XmNwidth, 126,
#else
             XmNheight, 320,
             XmNwidth, 320,
#endif /* VAX C one is smaller */
             XmNx, x,
             XmNy, y,
             XmNallowShellResize, FALSE,
             NULL);
      
        splash_cc = 128;
        splashpix = LoadSplashXPM(splash, &splash_cc);
	if (!splash_cc) {
	    XtDestroyWidget(splash);
	    do_splash = 0;
	    goto splash_goto;
	}

        sprintf(s, "version %s", MO_VERSION_STRING);
      
        l = strlen(s);
      
        sform = XtVaCreateManagedWidget("sform", xmRowColumnWidgetClass,
                                        splash,
#if defined(VMS) && !defined(__DECC) && !defined(__GNUC__)
                                        XmNheight, 126,
                                        XmNwidth, 126,
#else
                                        XmNheight, 320,
                                        XmNwidth, 320,
#endif /* VAX C one is smaller */
                                        XmNx, x,
                                        XmNy, y,
                                        NULL);

        XtPopup(splash, XtGrabNone);

        ccell_fg.flags = DoRed | DoGreen | DoBlue;
        ccell_fg.red = 0xFFFF;
        ccell_fg.blue = 0;
        ccell_fg.green = 0;

        /* We use red so we don't bother freeing it */
        if (!XAllocColor(dsp, (installed_colormap ?
			       installed_cmap :
			       DefaultColormapOfScreen(XtScreen(splash))),
                         &ccell_fg))
            ccell_fg.pixel = WhitePixelOfScreen(XtScreen(splash));
        
        values.font = font->fid;
        values.foreground = ccell_fg.pixel;

	gc = XtGetGC(splash, GCFont | GCForeground, &values);

#if !defined(VMS) || defined(__DECC) || defined(__GNUC__)
        XDrawString(dsp, splashpix, gc, 
                    320 - (fontW * l/2),
                    320 - fontH/2, 
                    s, l);
#endif /* Don't do for VAX C, GEC */
      
        spixwid = XtVaCreateManagedWidget(" ", xmLabelWidgetClass,
                                          sform,
                                          XmNlabelType, XmPIXMAP,
                                          XmNlabelPixmap, splashpix,
                                          XmNalignment, XmALIGNMENT_CENTER,
                                          XmNx, x,
                                          XmNy, y,
                                          NULL);
        XFlush(dsp);
        XmUpdateDisplay(splash);
        XFlush(dsp);
        XSync(dsp, False);
        XtReleaseGC(splash, gc);
    }

    XmxInitClue(toplevel, get_pref_boolean(eCLUE_HELP));
    XmxClueTimers(get_pref_int(eCLUE_POPUP_DELAY), 2000,
		  get_pref_int(eCLUE_POPDOWN_DELAY));
    {
        XColor color;
	Colormap cmap;
	char *font = get_pref_string(eCLUE_FONT);
	char *setfont =
	       "-adobe-new century schoolbook-bold-r-normal-*-12-*-*-*-*-*-*-*";

	cmap = installed_colormap ? installed_cmap :
	       DefaultColormap(dsp, DefaultScreen(dsp));

	XParseColor(dsp, cmap, get_pref_string(eCLUE_FOREGROUND), &color);
        if (!XAllocColor(dsp, cmap, &color))
            color.pixel = BlackPixel(dsp, DefaultScreen(dsp));
	XmxClueForeground(color.pixel);

	XParseColor(dsp, cmap, get_pref_string(eCLUE_BACKGROUND), &color);
	/* If cannot alloc it, the default will be fine */
        if (XAllocColor(dsp, cmap, &color))
	    XmxClueBackground(color.pixel);

	if (!my_strncasecmp(font, "New Century", 11)) {
	    ;
	} else if (!my_strncasecmp(font, "Times", 5)) {
	    setfont = "-adobe-times-bold-r-normal-*-12-*-*-*-*-*-*-*";
	} else if (!my_strncasecmp(font, "Helvetica", 9)) {
	    setfont = "-adobe-helvetica-bold-r-normal-*-12-*-*-*-*-*-*-*";
	} else if (!my_strncasecmp(font, "Lucida", 6)) {
	    setfont = "-b&h-lucidabright-demibold-r-normal-*-12-*-*-*-*-*-*-*";
	} else if (!my_strncasecmp(font, "Fixed", 5)) {
	    setfont = "-adobe-courier-bold-r-normal-*-12-*-*-*-*-*-*-*";
	}
	XmxClueFont(setfont);

	XmxClueOval(get_pref_boolean(eCLUE_OVAL));
	XmxClueRounded(get_pref_boolean(eCLUE_ROUNDED));
    }

    XtAppAddActions(app_context, balloon_action, 2);
    XtAppAddActions(app_context, toplevel_actions, 1);
    XtAppAddActions(app_context, url_actions, 2);

    mo_init_menubar();

#ifdef __sgi
    /* Turn on debugging malloc if necessary. */
    if (get_pref_boolean(eDEBUGGING_MALLOC))
        mallopt(M_DEBUG, 1);
#endif

    if (get_pref_string(eACCEPT_LANGUAGE_STR)) {
        char **extras;

        extras = malloc(sizeof(char *) * 2);

        extras[0] = malloc(strlen(get_pref_string(eACCEPT_LANGUAGE_STR)) + 19);
        sprintf(extras[0], "Accept-Language: %s",
                get_pref_string(eACCEPT_LANGUAGE_STR));
        extras[1] = NULL;

        HT_SetExtraHeaders(extras);
    }
          
    global_xterm_str = get_pref_string(eXTERM_COMMAND);

    uncompress_program = get_pref_string(eUNCOMPRESS_COMMAND);
    gunzip_program = get_pref_string(eGUNZIP_COMMAND);

    tweak_gopher_types = get_pref_boolean(eTWEAK_GOPHER_TYPES);
    max_wais_responses = get_pref_int(eMAX_WAIS_RESPONSES);
    ftp_timeout_val = get_pref_int(eFTP_TIMEOUT_VAL);
    ftpRedial = get_pref_int(eFTP_REDIAL);
    ftpRedialSleep = get_pref_int(eFTP_REDIAL_SLEEP);
    ftpFilenameLength = get_pref_int(eFTP_FILENAME_LENGTH);
    ftpEllipsisLength = get_pref_int(eFTP_ELLIPSIS_LENGTH);
    ftpEllipsisMode = get_pref_int(eFTP_ELLIPSIS_MODE);

    max_kiosk_protocols = 0;
    if (get_pref_boolean(eKIOSK)) {
	char *tmp = get_pref_string(eKIOSKPROTOCOLS);

	if (!tmp || !*tmp) {
	    /* Do nothing */
	} else {
	    tptr = strdup(tmp);
	    ptr = tptr;
	    while (1) {
		tptr = ptr;
		if (!(ptr = strchr(ptr, ','))) {
		    for (; *tptr && isspace(*tptr); tptr++)
			;
		    for (eptr = tptr + (strlen(tptr) - 1);
			 tptr <= eptr && (isspace(*eptr) || !isprint(*eptr));
			 eptr--) {
			*eptr = '\0';
		    }
		    if (*tptr)
			strcpy(kioskProtocols[max_kiosk_protocols++], tptr);
		    break;
		} else {
		    *ptr++ = '\0';
		    strcpy(kioskProtocols[max_kiosk_protocols++], tptr);
		}
	    }
	}
	free(tptr);
    } else {
	max_kiosk_protocols = 0;
    }

    sendReferer = get_pref_boolean(eSEND_REFERER);
    sendAgent = get_pref_boolean(eSEND_AGENT);

    useAFS = get_pref_boolean(eUSEAFSKLOG);

    proxy_list = ReadProxies(get_pref_string(ePROXY_SPECFILE));
    noproxy_list = ReadNoProxies(get_pref_string(eNOPROXY_SPECFILE));

    use_default_extension_map = get_pref_boolean(eUSE_DEFAULT_EXTENSION_MAP);
    global_extension_map = get_pref_string(eGLOBAL_EXTENSION_MAP);

    tptr = get_pref_string(ePERSONAL_EXTENSION_MAP);
    if (tptr) {
        char *home = getenv("HOME");
      
#ifndef VMS
        if (!home)
            home = "/tmp";
#endif /* VMS, BSN */
      
        personal_extension_map = (char *)malloc(strlen(home) +
						strlen(tptr) + 8);
#ifndef VMS
        sprintf(personal_extension_map, "%s/%s", home, tptr);
#else
        sprintf(personal_extension_map, "%s%s", home, tptr);
#endif /* VMS, BSN */
    } else {
        personal_extension_map = "\0";
    }

    use_default_type_map = get_pref_boolean(eUSE_DEFAULT_TYPE_MAP);
    global_type_map = get_pref_string(eGLOBAL_TYPE_MAP);

    tptr = get_pref_string(ePERSONAL_TYPE_MAP);
    if (tptr) {
        char *home = getenv("HOME");
      
#ifndef VMS
        if (!home)
            home = "/tmp";
#endif /* VMS, BSN */
      
        personal_type_map = (char *)malloc(strlen(home) + strlen(tptr) + 8);
#ifndef VMS
        sprintf(personal_type_map, "%s/%s", home, tptr);
#else
        sprintf(personal_type_map, "%s%s", home, tptr);
#endif /* VMS, BSN */
    } else {
        personal_type_map = "\0";
    }

    twirl_increment = get_pref_int(eTWIRL_INCREMENT);
  
    /* Then make a copy of the hostname for shortmachine.  Don't even ask. */
    shortmachine = strdup(machine);
  
    /* Then find out the full name, if possible. */
    if (get_pref_string(eFULL_HOSTNAME)) {
        free(machine);
        machine = get_pref_string(eFULL_HOSTNAME);
    } else if (!get_pref_boolean(eGETHOSTBYNAME_IS_EVIL)) {
        struct hostent *phe;
      
        phe = gethostbyname(machine);
        if (phe && phe->h_name) {
            free(machine);
            machine = strdup(phe->h_name);
        }
    }

    /* (Otherwise machine just remains whatever gethostname returned.) */
    machine_with_domain = (strlen(machine) > strlen(shortmachine) ?
                           machine : shortmachine);

    {  /* Author Name & Email init. */
#ifndef VMS
        struct passwd *pw = getpwuid(getuid());
#else
        struct passwd *pw = getpwuid();
        char *vms_mail_prefix = get_pref_string(eVMS_MAIL_PREFIX);
#endif /* VMS, BSN */
        char *cc;
        char *default_author_name = get_pref_string(eDEFAULT_AUTHOR_NAME);
        char *default_author_email = get_pref_string(eDEFAULT_AUTHOR_EMAIL);
      
        if (!default_author_name || !*default_author_name) {
	    if (!pw || !pw->pw_gecos) {
		default_author_name = strdup("Unknown");
	    } else {
		default_author_name = strdup(pw->pw_gecos);
		strcpy(default_author_name, pw->pw_gecos);
		for (cc = default_author_name; *cc; cc++) {
		    if (*cc == ',') {
			*cc = 0;
			break;
		    }
		}
	    }
	}
        if (!default_author_email || !*default_author_email) {
	    if (!pw || !pw->pw_name) {
		default_author_email =
		    (char *) malloc(strlen("UNKNOWN") + strlen(machine) + 2);
		sprintf(default_author_email, "UNKNOWN@%s", machine);
	    } else {
		default_author_email =
		    (char *) malloc(strlen(pw->pw_name) + strlen(machine) + 2);
		sprintf(default_author_email, "%s@%s", pw->pw_name, machine);
	    }
        }    

        /* Check again just to make absolutely sure something is there */
        if (!default_author_name || !*default_author_name)
                default_author_name = strdup("Unknown");

        if (!default_author_email || !*default_author_email) {
            default_author_email =
                        (char *) malloc(strlen("UNKNOWN") + strlen(machine)+2);
            sprintf(default_author_email, "UNKNOWN@%s", machine);
        }

        set_pref(eDEFAULT_AUTHOR_NAME, (void *)default_author_name);
        set_pref(eDEFAULT_AUTHOR_EMAIL, (void *)default_author_email);
#ifdef VMS /* Drop blanks off the mail prefix string. GEC */
        if (vms_mail_prefix) {
            for (cc = vms_mail_prefix; *cc; cc++) {
                if (*cc == ' ') {
                    *cc = 0;
                    break;
                }
            }
        } else {
            vms_mail_prefix = " "; /* But make sure it is not a NULL string */
        }
        set_pref(eVMS_MAIL_PREFIX, (void *)vms_mail_prefix);
#endif
    }
  
        /* If there's no tmp directory assigned by the X resource, then
           look at TMPDIR. */
    {
        char *tmp_dir = get_pref_string(eTMP_DIRECTORY);
          
        if (!tmp_dir) {
            tmp_dir = getenv("TMPDIR");
#ifdef VMS /* Make temp files in SYS$SCRATCH if TMPDIR not defined. PGE */
            if (!tmp_dir)
                tmp_dir = getenv("SYS$SCRATCH");
#endif
                /* It can still be NULL when we leave here -- then we'll just
                   let tmpnam() do what it does best. */
            set_pref(eTMP_DIRECTORY, (void *)tmp_dir);
        }
    }

    /* If there's no docs directory assigned by the X resource,
     * then look at MOSAIC_DOCS_DIRECTORY environment variable
     * and then at hardcoded default. */
    {
        char *docs_dir = get_pref_string(eDOCS_DIRECTORY);

	if (!docs_dir) {
	    docs_dir = getenv("MOSAIC_DOCS_DIRECTORY");
	    if (!docs_dir)
		docs_dir = DOCS_DIRECTORY_DEFAULT;
	    if (!docs_dir || !*(docs_dir)) {
		fprintf(stderr,	"fatal error: nonexistent docs directory\n");
		exit(-1);
	    }
	    set_pref(eDOCS_DIRECTORY, (void *)docs_dir);
	/* NCSA stuff gone */
	} else if (!strcmp(docs_dir,
		   "http://www.ncsa.uiuc.edu/SDG/Software/XMosaic")) {
	    docs_dir = DOCS_DIRECTORY_DEFAULT;
	    set_pref(eDOCS_DIRECTORY, (void *)docs_dir);
	}
    }

    if (get_pref_int(eCOLORS_PER_INLINED_IMAGE) > 256) {
        fprintf(stderr, "WARNING: Colors per inline image preference > 256.\n");
        fprintf(stderr, "         Auto-Setting to 256.\n");
        set_pref_int(eCOLORS_PER_INLINED_IMAGE, 256);
    }

    if (get_pref_boolean(eUSE_GLOBAL_HISTORY)) {
        mo_setup_global_history();
    } else {
        mo_init_global_history();
    }
    mo_setup_default_hotlist();
    if (get_pref_boolean(eUSE_COOKIE_FILE))
        HTLoadCookies(get_pref_string(eCOOKIE_FILE),
		      get_pref_string(ePERM_FILE));
    mo_setup_pan_list();

#ifndef VMS
    /* Write pid into "~/.mosaicpid". */
#else
    /* Write pid into "~/mosaicpid.". and delete a possible previous file */
    /* Changed now, just delete a possible previous file */
#endif /* VMS, BSN */
    {
        char *home = getenv("HOME");
	char *fnam;
#ifndef VMS
        FILE *fp;

        if (!home)
            home = "/tmp";
#endif /* VMS, BSN */
    
        fnam = (char *)malloc(strlen(home) + 32);
#ifndef VMS
        sprintf(fnam, "%s/.mosaicpid", home);
#else
        sprintf(fnam, "%smosaicpid.", home);
        remove(fnam);
#endif /* VMS, BSN */
    
#ifndef VMS
        fp = fopen(fnam, "w");
        if (fp) {
            fprintf(fp, "%d\n", getpid());
            fclose(fp);
        }
#endif /* Not needed, BSN */
    
        free(fnam);
    }
  
    busy_cursor = XCreateFontCursor(dsp, XC_watch);

#ifndef MOTIF1_2
    /* If Motif 1.1, then make them valid geometry values */
    XtVaSetValues(toplevel, 
                  XmNwidth, 1, 
                  XmNheight, 1, 
                  NULL);
#endif
#ifndef DISABLE_TRACE
    if (srcTrace)
	fprintf(stderr, "Realizing toplevel widget.\n");
#endif

    XtRealizeWidget(toplevel);

    /* Get the current geometry values */
    XtVaGetValues(toplevel, 
                  XmNwidth, &userWidth, 
                  XmNheight, &userHeight, 
                  XmNx, &userX,
                  XmNy, &userY,
                  NULL);
#ifndef DISABLE_TRACE
    if (srcTrace)
	fprintf(stderr, "Starting geometry: X=%d, Y=%d, W=%d, H=%d\n",
	        userX, userY, userWidth, userHeight);
#endif

    gargv = argv;
    gargc = argc;

    createBusyCursors(toplevel);
    MakePixmaps(toplevel);
    pix_count = get_pref_int(ePIX_COUNT);

    setup_imagekill();

    mo_open_initial_window();

#ifdef CCI
#ifndef DISABLE_TRACE
    if (srcTrace)
        fprintf(stderr, "cciPort resourced to %d\n", get_pref_int(eCCIPORT));
#endif
    if ((get_pref_int(eCCIPORT) > 1023 ) && (get_pref_int(eCCIPORT) < 65536))
        MoCCIStartListening(toplevel, get_pref_int(eCCIPORT));
#endif

    XtAppMainLoop(app_context);
}


/****************************************************************************
 * name:    mo_process_external_directive
 * purpose: Handle an external directive given to the application via
 *          a config file read in response to a SIGUSR1.
 * inputs:  
 *   - char *directive: The directive; either "goto" or "newwin".
 *   - char       *url: The URL corresponding to the directive.
 * returns: 
 *   nothing
 * remarks: 
 *   
 ****************************************************************************/
#define CLIP_TRAILING_NEWLINE(url) \
  if (url[strlen(url) - 1] == '\n') \
    url[strlen(url) - 1] = '\0';

static XEvent *mo_manufacture_dummy_event(Widget foo)
{
  /* This is hilarious. */
  XAnyEvent *a = (XAnyEvent *)malloc(sizeof(XAnyEvent));

  a->type = 1; /* HAHA! */
  a->serial = 1; /* HAHA AGAIN! */
  a->send_event = False;
  a->display = XtDisplay(foo);
  a->window = XtWindow(foo);
  return (XEvent *)a;
}

/* Process a directive that we received externally. */
void mo_process_external_directive(char *directive, char *url)
{
  mo_window *win = current_win;

  /* Make sure we have a window. */
  if (!win)
      win = mo_next_window(NULL);

  if (!strncmp(directive, "goto", 4)) {
      CLIP_TRAILING_NEWLINE(url);

      mo_access_document(win, url);

      XmUpdateDisplay(win->base);
  } else if (!strncmp(directive, "newwin", 6)) {
      CLIP_TRAILING_NEWLINE(url);

      /* Force a new window to open. */ 
      mo_open_another_window(win, url, NULL, NULL);

      XmUpdateDisplay(win->base);
  } else if (!strncmp(directive, "pagedown", 8)) {
      Widget sb;
      String params[1];
      
      params[0] = "0";
      
      XtVaGetValues(win->scrolled_win, XmNverticalScrollBar, 
                    (long)(&sb), NULL);
      if (sb && XtIsManaged(sb)) {
          XEvent *event = mo_manufacture_dummy_event(sb);

          XtCallActionProc(sb, "PageDownOrRight", event, params, 1);
      }

      XmUpdateDisplay(win->base);
  } else if (!strncmp(directive, "pageup", 6)) {
      Widget sb;
      String params[1];

      params[0] = "0";

      XtVaGetValues(win->scrolled_win, XmNverticalScrollBar, 
                    (long)(&sb), NULL);
      if (sb && XtIsManaged(sb)) {
          XEvent *event = mo_manufacture_dummy_event(sb);

          XtCallActionProc(sb, "PageUpOrLeft", event, params, 1);
      }
      XmUpdateDisplay(win->base);
  } else if (!strncmp(directive, "scrolldown", 9)) {
      Widget sb;
      String params[1];
      
      params[0] = "0";

      XtVaGetValues(win->scrolled_win, XmNverticalScrollBar, 
                    (long)(&sb), NULL);
      if (sb && XtIsManaged(sb)) {
          XEvent *event = mo_manufacture_dummy_event(sb);

          XtCallActionProc(sb, "IncrementDownOrRight", event, params, 1);
      }
      XmUpdateDisplay(win->base);
  } else if (!strncmp(directive, "scrollup", 7)) {
      Widget sb;
      String params[1];

      params[0] = "0";

      XtVaGetValues(win->scrolled_win, XmNverticalScrollBar, 
                    (long)(&sb), NULL);
      if (sb && XtIsManaged(sb)) {
          XEvent *event = mo_manufacture_dummy_event(sb);

          XtCallActionProc(sb, "IncrementUpOrLeft", event, params, 1);
      }
      XmUpdateDisplay(win->base);

  } else if (!strncmp(directive, "flushimagecache", 15)) {
      mo_flush_image_cache();

  } else if (!strncmp(directive, "backnode", 8)) {
      mo_back_node(win);
      XmUpdateDisplay(win->base);

  } else if (!strncmp(directive, "forwardnode", 11)) {
      mo_forward_node(win);
      XmUpdateDisplay(win->base);

  } else if (!strncmp(directive, "reloaddocument", 14)) {
      mo_reload_window_text(win, 0);
      XmUpdateDisplay(win->base);

  } else if (!strncmp(directive, "reloadimages", 12)) {
      mo_reload_window_text(win, 1);
      XmUpdateDisplay(win->base);

  } else if (!strncmp(directive, "refresh", 7)) {
      mo_refresh_window_text(win);
      XmUpdateDisplay(win->base);

  /*
   * The following commands were added by Mark Donszelmann, duns@vxdeop.cern.ch
   */
  } else if (!strncmp(directive, "iconify", 7)) {
      XIconifyWindow(dsp, XtWindow(win->base), DefaultScreen(dsp));

  } else if (!strncmp(directive, "deiconify", 9)) {
      XMapWindow(dsp, XtWindow(win->base));

  } else if (!strncmp(directive, "raise", 5)) {
      XMapRaised(dsp, XtWindow(win->base));

  } else if (!strncmp(directive, "move", 4)) {
      char *status;

      if ((status = strchr(url, '|')) != NULL) {
          *status = '\0';
          status++;
      } else {
	  status = url;  /* Prevent crash if 2nd parameter missing, TJA */
      }
      XMoveWindow(dsp, XtWindow(win->base), atoi(url), atoi(status));
  } else if (!strncmp(directive, "resize", 6)) {
      char *status;

      if ((status = strchr(url, '|')) != NULL) {
          *status = '\0';
          status++;
      } else {
	  status = url;  /* Prevent crash if 2nd parameter missing, TJA */
      }
      XResizeWindow(dsp, XtWindow(win->base), atoi(url), atoi(status));
  }

  return;
}


static void set_current_win(Widget w, XEvent *event,
	                    String *params, Cardinal *num_params)
{
  Widget top = w;
  mo_window *ptr = winlist;
  int i;

  while (!XtIsTopLevelShell(top))
      top = XtParent(top);

  for (i = 0; ptr && (i < wincount); i++) {
      if (ptr->base == top) {
	  if (event->xany.type == EnterNotify) {
	      current_win = ptr;
	      ptr->have_focus = True;
	  } else if (event->xany.type == LeaveNotify) {
	      ptr->have_focus = False;
	  }
	  break;
      } else {
	  ptr = ptr->next;
      }
  }
  if (!ptr)
      fprintf(stderr,
	      "Couldn't find current window.  Mosaic will be crashing soon.\n");
}

static void set_focus_to_view(Widget w, XEvent *event,
	                      String *params, Cardinal *num_params)
{
  XtSetKeyboardFocus(current_win->base, current_win->view);
}

static void take_focus(Widget w, XEvent *event,
	               String *params, Cardinal *num_params)
{
  XtSetKeyboardFocus(current_win->base, w);
}


void mo_flush_passwd_cache(mo_window *win)
{
  HTFTPClearCache();
  HTAAServer_clear();
  HTProgress("Password cache flushed");
}

/* --------------------------- PASTE CODE ----------------------------------*/

static void paste_action(Widget w, XEvent *event, String *params,
			 Cardinal *num_params);

static XtActionsRec actions[] =
  {
    {"URLPaste",    (XtActionProc)paste_action}
  };


static void paste_init(XtAppContext app)
{
  XtAppAddActions(app, actions, XtNumber(actions));
}


static mo_window *mo_fetch_window_by_widget(Widget w)
{
  /* Given any widget in the hierarchy, find the associated window structure */
  mo_window *win = winlist;
  Widget shell = w;

  /* Find Xt's Shell Widget */
  while (shell && !XtIsTopLevelShell(shell))
      shell = XtParent(shell);

  while (win) {
      if (win->base == shell)
          return(win);
      win = win->next;
  }

  return NULL;
}


static void selection_requestor_cb(Widget w, XtPointer client_data,
				   Atom *selection, Atom *type,
				   XtPointer value, unsigned long *length,
				   int *format)
{
  char *text = (char *) value;
  mo_window *win;
  char *url, *cleaned;

  win = mo_fetch_window_by_widget(w);

  if (win && type && (*type == XA_STRING) && text && strlen(text) &&
      length && *length) {
      cleaned = mo_clean_and_escape_url(text, 0);
      url = mo_url_prepend_protocol(cleaned);
      free(cleaned);
      mo_load_window_text(win, url, NULL);
      free(url);
  }
  if (text)
      XtFree(value);
}


static void paste_action(Widget w, XEvent *event, String *params,
			 Cardinal *num_params)
{
  XButtonPressedEvent *mouse_event = (XButtonPressedEvent *) event;

  XtGetSelectionValue(w, XA_PRIMARY, XA_STRING, selection_requestor_cb,
                      NULL, mouse_event->time);
}
