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

#ifndef __MOSAIC_H__
#define __MOSAIC_H__

/* --------------------------- SYSTEM INCLUDES ---------------------------- */

#ifdef __sgi
#ifndef _SVR4_SOURCE
#define _SVR4_SOURCE
#include <stdio.h>
#undef _SVR4_SOURCE
#else
#include <stdio.h>
#endif
#else
#include <stdio.h>
#endif

#ifdef __sgi
#ifndef _SVR4_SOURCE
#define _SVR4_SOURCE
#include <string.h>
#undef _SVR4_SOURCE
#else
#include <string.h>
#endif
#else
#include <string.h>
#endif

#include <ctype.h>
#if !defined(VMS) && !defined(NeXT)
#include <unistd.h>
#endif

#ifdef __sgi
#ifndef _SVR4_SOURCE
#define _SVR4_SOURCE
#include <stdlib.h>
#undef _SVR4_SOURCE
#else
#include <stdlib.h>
#endif
#else
#include <stdlib.h>
#endif

#if defined(WIN_TCP)
#define __CADDR_T
#define __STAT                  /* Works except with OLD versions of VAX C */
#endif
#include <sys/types.h>

#ifdef __sgi
#ifndef _SVR4_SOURCE
#define _SVR4_SOURCE
#include <errno.h>
#undef _SVR4_SOURCE
#else
#include <errno.h>
#endif
#else
#include <errno.h>
#endif

#ifdef __sgi
#ifndef _SGI_SOURCE
#define _SGI_SOURCE
#include <malloc.h>
#undef _SGI_SOURCE
#else
#include <malloc.h>
#endif
#endif

#ifdef VMS
#if defined(MULTINET) && defined(__DECC)
#define getpid  decc$getpid
#define getcwd  decc$getcwd
#define stat    decc$stat
#define cuserid decc$cuserid
#define mkdir   decc$mkdir
#define write   decc$write
#if defined(__VMS_VER) && (__VMS_VER >= 70000000)
#define strdup  decc$strdup
#endif
#ifndef fileno
#define fileno	decc$fileno
#endif /* DEC C V5.5 made an undocumented change to the prefixing, GEC */
#endif /* Some non-ANSI routines need a prefix */
#endif /* VMS, BSN */

#if defined(VMS) && !defined(__DECC)  /* VAXC only */
#if !defined(__CADDR_T) && !defined(CADDR_T) && !defined(__SOCKET_TYPEDEFS)
typedef char *caddr_t;
#endif /* Do if tcp.h not included previously, GEC */
#ifndef __CADDR_T
#define __CADDR_T 1   /* DECwindows xresource.h wants __CADDR_T, PGE */
#endif
#ifndef CADDR_T
#define CADDR_T 1     /* DECwindows Motif 1.1 xresource.h wants CADDR_T, GEC */
#endif
#endif
#ifndef __GNUC__
#include "../libXmx/Xmx.h"
#else
#include "Xmx.h"
#endif
#include "toolbar.h"

typedef enum {
  mo_plaintext = 0,
  mo_formatted_text,
  mo_html,
  mo_latex,
  mo_postscript,
  mo_mif
} mo_format_token;

/* ------------------------------------------------------------------------ */
/* -------------------------------- ICONS --------------------------------- */
/* ------------------------------------------------------------------------ */
#define NUMBER_OF_FRAMES	25
#define ANIMATION_PIXMAPS	0
#define SECURITY_PIXMAPS	1
#define DIALOG_PIXMAPS		2

/* ------------------------------------------------------------------------ */
/* -------------------------------- MACROS -------------------------------- */
/* ------------------------------------------------------------------------ */

#define MO_VERSION_STRING "4.2"
#define MO_VERSION_STRING2 "4_2"  /* For file names */
#define MO_GO_NCSA_COUNT 0  /* Not the VMS style, AV */

#define MO_HELP_ON_VERSION_DOCUMENT \
  mo_assemble_help_url("help-on-version-2.7b5.html")

/* No one left at NCSA */
#define MO_DEVELOPER_ADDRESS "mosaic@wvnvms.wvnet.edu"

#ifndef DOCS_DIRECTORY_DEFAULT
#define DOCS_DIRECTORY_DEFAULT "http://wvnvms.wvnet.edu/vmswww/mosaic"
#endif

#ifndef DOCS_DIRECTORY_VMS
#define DOCS_DIRECTORY_VMS "http://wvnvms.wvnet.edu/vmswww/mosaic"
#endif

#ifndef HOME_PAGE_DEFAULT
/* This must be a straight string as it is included into a struct; no tricks. */
#define HOME_PAGE_DEFAULT "http://wvnvms.wvnet.edu/vmswww/vms_mosaic.html"
#endif

#ifndef HTMLPRIMER_PAGE_DEFAULT
#define HTMLPRIMER_PAGE_DEFAULT \
  "http://wvnvms.wvnet.edu/vmswww/mosaic/htmlprimerall.html"
#endif

#ifndef URLPRIMER_PAGE_DEFAULT
#define URLPRIMER_PAGE_DEFAULT \
  "http://wvnvms.wvnet.edu/vmswww/mosaic/url-primer.html"
#endif

#ifndef NETWORK_SEARCH_DEFAULT
#define NETWORK_SEARCH_DEFAULT "http://www.google.com/"
#endif
#ifndef NETWORK_SEARCH_NAME
#define NETWORK_SEARCH_NAME "Web - Google"
#endif

#ifndef USENET_SEARCH_DEFAULT
#define USENET_SEARCH_DEFAULT "http://groups.google.com/"
#endif
#ifndef USENET_SEARCH_NAME
#define USENET_SEARCH_NAME "Usenet -  Google Groups"
#endif

#ifndef META_SEARCH_DEFAULT
#define META_SEARCH_DEFAULT "http://www.dogpile.com/"
#endif
#ifndef META_SEARCH_NAME
#define META_SEARCH_NAME "Meta-Search - Dogpile"
#endif

#ifndef INTERNET_METAINDEX_DEFAULT
#define INTERNET_METAINDEX_DEFAULT "http://www.yahoo.com"
#endif
#ifndef INTERNET_METAINDEX_NAME
#define INTERNET_METAINDEX_NAME "Web Directory - Yahoo"
#endif

#ifndef AUCTION_SEARCH_DEFAULT
#define AUCTION_SEARCH_DEFAULT "http://www.ebay.com"
#endif
#ifndef AUCTION_SEARCH_NAME
#define AUCTION_SEARCH_NAME "Auctions - eBay"
#endif

#ifndef LIST_SEARCH_DEFAULT
#define LIST_SEARCH_DEFAULT "http://groups.yahoo.com"
#endif
#ifndef LIST_SEARCH_NAME
#define LIST_SEARCH_NAME "Lists - Yahoo Groups"
#endif

#ifndef MAP_SEARCH_DEFAULT
#define MAP_SEARCH_DEFAULT "http://www.mapquest.com"
#endif
#ifndef MAP_SEARCH_NAME
#define MAP_SEARCH_NAME "Maps - MapQuest"
#endif

#ifndef PEOPLE_SEARCH_DEFAULT
#define PEOPLE_SEARCH_DEFAULT "http://www.whowhere.com"
#endif
#ifndef PEOPLE_SEARCH_NAME
#define PEOPLE_SEARCH_NAME "People - WhoWhere"
#endif

#ifndef ENCYCLOPEDIA_SEARCH_DEFAULT
#define ENCYCLOPEDIA_SEARCH_DEFAULT "http://en.wikipedia.org"
#endif
#ifndef ENCYCLOPEDIA_SEARCH_NAME
#define ENCYCLOPEDIA_SEARCH_NAME "Encyclopedia - Wikipedia"
#endif

#ifndef DOCUMENTS_MENU_SPECFILE
#ifndef VMS
#define DOCUMENTS_MENU_SPECFILE "/usr/local/lib/mosaic/documents.menu"
#else
#define DOCUMENTS_MENU_SPECFILE "DECW$System_Defaults:documents.menu"
#endif /* VMS, BSN */
#endif

#ifndef GLOBAL_EXTENSION_MAP
#ifndef VMS
#define GLOBAL_EXTENSION_MAP "/usr/local/lib/mosaic/mime.types"
#else
#define GLOBAL_EXTENSION_MAP "Mosaic_Mailcap_Dir:MIME.Types"
#endif /* VMS, BSN */
#endif

#ifndef GLOBAL_TYPE_MAP
#ifndef VMS
#define GLOBAL_TYPE_MAP "/usr/local/lib/mosaic/mailcap"
#else
#define GLOBAL_TYPE_MAP "Mosaic_Mailcap_Dir:mailcap."
#endif /* VMS, BSN */
#endif

#ifdef VMS
#ifndef MAIL_PREFIX_DEFAULT
#define MAIL_PREFIX_DEFAULT ""
#endif /* Mail prefix for VMS MAIL */

#ifndef PRINT_DEFAULT
#define PRINT_DEFAULT "Print/Name=\"Mosaic print\"/Notify/Identify/Delete"
#endif /* Default print command for VMS */

#ifndef EDITOR_DEFAULT
#define EDITOR_DEFAULT "Edit"
#endif /* Editor command for source editing */
#endif /* VMS, BSN, GEC */

#ifndef VMS
#if defined(bsdi)
#define MO_MACHINE_TYPE "BSD/OS"
#endif
#if defined(__hpux)
#define MO_MACHINE_TYPE "HP-UX"
#endif
#if defined(__sgi)
#define MO_MACHINE_TYPE "Silicon Graphics"
#endif
#if defined(ultrix)
#define MO_MACHINE_TYPE "DEC Ultrix"
#endif
#if defined(linux)
#define MO_MACHINE_TYPE "Linux"
#endif
#if defined(_IBMR2)
#define MO_MACHINE_TYPE "RS/6000 AIX"
#endif
#if defined(sun) && !defined(SOLARIS)
#define MO_MACHINE_TYPE "Sun"
#else
#if defined(SOLARIS)
#define MO_MACHINE_TYPE "SOLARIS"
#endif
#endif
#if defined(__alpha)
#define MO_MACHINE_TYPE "DEC Alpha"
#endif
#if defined(__ia64)
#define MO_MACHINE_TYPE "IA64"
#endif
#if defined(NEXT)
#define MO_MACHINE_TYPE "NeXT BSD"
#endif
#if defined(cray)
#define MO_MACHINE_TYPE "Cray"
#endif
#if defined(NeXT)
#define MO_MACHINE_TYPE "NeXT"
#endif
#if defined (SCO)
#if defined (_SCO_DS)
#define MO_MACHINE_TYPE "SCO OpenServer 5"
#else /* _SCO_DS */
#define MO_MACHINE_TYPE "SCO Unix"
#endif /* _SCO_DS */
#endif /* SCO */
#else
#if defined(vax)
#define MO_MACHINE_TYPE "OpenVMS VAX"
#endif
#if defined(__alpha)
#define MO_MACHINE_TYPE "OpenVMS Alpha"
#endif
#if defined(__ia64)
#define MO_MACHINE_TYPE "OpenVMS IA64"
#endif
#if !defined(vax) && !defined(__alpha) && !defined(__ia64)
#define MO_MACHINE_TYPE "OpenVMS"
#endif
#endif /* VMS, BSN, GEC */

#ifndef MO_MACHINE_TYPE
#define MO_MACHINE_TYPE "Unknown Platform"
#endif

#ifdef __hpux
#define HAVE_AUDIO_ANNOTATIONS
#else
#if defined(__sgi) || defined(sun)
#define HAVE_AUDIO_ANNOTATIONS
#endif
#endif

/* Be safe... some URL's get very long. */
#define MO_LINE_LENGTH 2048

/* Use builtin strdup when appropriate -- code duplicated in tcp.h. */
#if defined(ultrix) || (defined(VMS) && (!defined(__GNUC__) || defined(vax))) || defined(NeXT)
extern char *strdup();
#endif

#define public
#define private static

/* String #defs for Print/Mail/Save */
#ifndef MODE_HTML
#define MODE_HTML "html"
#endif

#ifndef MODE_POSTSCRIPT
#define MODE_POSTSCRIPT "postscript"
#endif

#ifndef MODE_FORMATTED
#define MODE_FORMATTED "formatted"
#endif

#ifndef MODE_PLAIN
#define MODE_PLAIN "plain"
#endif


/* ------------------------------------------------------------------------ */
/* ------------------------------ MAIN TYPES ------------------------------ */
/* ------------------------------------------------------------------------ */

/* ------------------------------ mo_window ------------------------------- */

#define moMODE_PLAIN  0x0001
#define moMODE_FTP    0x0002
#define moMODE_NEWS   0x0004
#define moMODE_ALL    0x0007


/* mo_window contains everything related to a single Document View
 * window, including subwindow details.
 */
typedef struct mo_window {
  int id;
  Widget base;
  int mode;
    
  /* Subwindows. */
  Widget source_win;
  Widget save_win;
  Widget upload_win;
  Widget savebinary_win;  /* For binary transfer mode */
  Widget open_win;
  Widget mail_fsb_win;
  Widget mail_win;
  Widget mailhot_win;
  Widget edithot_win;
  Widget inserthot_win;
  Widget mailhist_win;
  Widget print_win;
  Widget history_win;
  Widget open_local_win;
  Widget hotlist_win;
  Widget techsupport_win;
  Widget news_win;           /* News Post/Followup*/
  Widget news_fsb_win;
  Widget news_sub_win;       /* News Subscribe Window */
  Widget annotate_win;
  Widget src_search_win;     /* Source window document search */
  Widget search_win;         /* Internal document search */
  Widget searchindex_win;    /* Network index search */
  Widget cci_win;	     /* Common client interface control window */
  Widget mailto_win;
  Widget mailto_form_win;
  Widget links_win;          /* Window with list of links */
  Widget links_list;         /* Widget holding the list itself */
  XmStringTable links_items;
  int links_count;

  Widget ftpput_win, ftpremove_win, ftpremove_text, ftpmkdir_win, ftpmkdir_text;
  char *ftp_site;

  Widget session_menu;
  Widget *session_items;
  int num_session_items;
    
#ifdef HAVE_AUDIO_ANNOTATIONS
  Widget audio_annotate_win;
#endif

  /* USER INTERFACE BITS 'n PIECES */
  struct toolbar tools[BTN_COUNT];

  Widget slab[7];
  int slabpart[8];
  int slabcount, haslogo, smalllogo, texttools;

  XmxMenuRecord *menubar;

  Widget url_text;
  Widget title_text;
  Widget scrolled_win, view;
  Widget rightform;
  Widget tracker_label, logo, security, encrypt;
  Widget tearbutton;
  Widget button_rc, button2_rc;
  Widget toolbarwin, topform;
  int has_toolbar;
  int toolbardetached;
  int toolbarorientation;
    
  Widget meter, meter_frame;
  int meter_level, meter_width, meter_height;
  Pixel meter_fg, meter_bg, meter_font_fg, meter_font_bg;
  char *meter_text;

  Widget searchindex_button;   /* Pushbutton, says "Search Index" */
  Widget searchindex_win_label, searchindex_win_text;
  Widget searchindex_win_searchbut;

  Widget home_button;

  int last_width;

  struct mo_node *history;
  struct mo_node *current_node;
  int reloading;

  char *target_anchor;

  /* Document source window. */
  Widget source_text;
  Widget source_url_text;
  Widget source_date_text;
  XmxMenuRecord *format_optmenu;
  int save_format; /* Starts at 0 */

  Widget open_text;

  Widget mail_to_text;
  Widget mail_subj_text;
  XmxMenuRecord *mail_fmtmenu;
  int mail_format;

  Widget mailhot_to_text;
  Widget mailhot_subj_text;
  Widget mailhist_to_text;
  Widget mailhist_subj_text;

  Widget print_text;
  XmxMenuRecord *print_fmtmenu;
  int print_format;
  Widget hotlist_rbm_toggle;
  Widget print_header_toggle_save;
  Widget print_header_toggle_print;
  Widget print_header_toggle_mail;
  Widget print_footer_toggle_save;
  Widget print_footer_toggle_print;
  Widget print_footer_toggle_mail;
  Widget print_a4_toggle_save;
  Widget print_a4_toggle_print;
  Widget print_a4_toggle_mail;
  Widget print_us_toggle_save;
  Widget print_us_toggle_print;
  Widget print_us_toggle_mail;
  Widget print_duplex_toggle_save;
  Widget print_duplex_toggle_print;
  Widget print_duplex_toggle_mail;
  Widget print_url_only;
  Widget print_doc_only;
    
  Widget history_list;

  Widget hotlist_list;
  Widget hotlist_label;
  Widget save_hotlist_win;
  Widget load_hotlist_win;
  struct mo_hotlist *current_hotlist;
  union mo_hot_item *hot_cut_buffer;

  Widget techsupport_text;

  Widget news_text;
  Widget news_text_from, news_text_subj, news_text_group;
  /* News followup storage */
  char *newsfollow_artid;
  char *newsfollow_grp, *newsfollow_subj, *newsfollow_ref, *newsfollow_from;

  Widget mailto_text;
  Widget mailto_fromfield;
  Widget mailto_tofield;
  Widget mailto_subfield;
  
  Widget mailto_form_text;
  Widget mailto_form_fromfield;
  Widget mailto_form_tofield;
  Widget mailto_form_subfield;

  char *post_data;

  int font_size;
  int font_family;
  int pretty;

  int underlines_snarfed;
  int underlines_state;
  /* Default values only, mind you. */
  int underlines;
  int visited_underlines;
  Boolean dashed_underlines;
  Boolean dashed_visited_underlines;

#ifdef HAVE_AUDIO_ANNOTATIONS
  Widget audio_start_button;
  Widget audio_stop_button;
  pid_t record_pid;
  char *record_fnam;
#endif

  Widget annotate_author;
  Widget annotate_title;
  Widget annotate_text;
  Widget delete_button;
  Widget include_fsb;
  int annotation_mode;
  int editing_id;

  char *cached_url;

  Widget search_win_text;
  Widget search_caseless_toggle;
  Widget search_backwards_toggle;
  void *search_start;
  void *search_end;

  Widget src_search_win_text;
  Widget src_search_caseless_toggle;
  Widget src_search_backwards_toggle;
  int src_search_pos;
#ifdef CCI
  Widget cci_win_text;
  Widget cci_accept_toggle;
  Widget cci_off_toggle;
#endif
  int preferences;
  int binary_transfer;
  int binary_ftp_mode;
  int delay_image_loads;
  int table_support;
  Boolean body_color;
  Boolean body_images;
  Boolean font_color;
  int image_view_internal;
  int progressive_loads;
  Boolean font_sizes;
  Boolean image_animation;
  int min_animation_delay;
  Boolean refresh_url;
  Boolean refreshable;
  Boolean safe_colors;
  Boolean blink_text;
  Boolean frame_support;
  Boolean hotkeys;
  Boolean multi_image_load;

  Widget subgroup;
  Widget unsubgroup;

  struct mo_window *next;

#ifdef GRPAN_PASSWD
  Widget passwd_label;
  Widget annotate_passwd;
  Widget passwd_toggle;
#endif
  XmxMenuRecord *pubpri_menu;
  int pubpri;        /* One of mo_annotation_[public, private] */
  XmxMenuRecord *audio_pubpri_menu;
  int audio_pubpri;  /* One of mo_annotation_[public, private] */

  int agent_state;
  Boolean have_focus;

  char *image_file;

  struct mo_window *frames;
  struct mo_window *next_frame;
  int is_frame;
  Boolean new_node;
  struct mo_window *frame;
  struct mo_window *parent;
  struct mo_window *do_frame;
  char *frametext;
  char *frametexthead;
  char *frameurl;
  char *framename;
  char *framelast_modified;
  char *frameexpires;
  char *framecharset;

  Pixel form_button_bg;

  Boolean mo_back;
  Boolean mo_forward;
  int current_authType;
  int presentation_mode;
} mo_window;

/* ------------------------------- mo_node -------------------------------- */

/* mo_node is a component of the linear history list.  A single
 * mo_node will never be effective across multiple mo_window's;
 * each window has its own linear history list.
 */
typedef struct mo_node {
  char *title;
  char *url;
  char *last_modified;
  char *expires;
  char *ref;	    /* How the node was referred to from a previous anchor,
		     * if such an anchor existed. */
  char *text;
  char *texthead;   /* Head of the alloc'd text -- this should
		     * be freed, not text */
  /* Position in the list, starting at 1; last item is
   * effectively 0 (according to the XmList widget). */
  int position;

  /* The type of annotation this is (if any) */
  int annotation_type;

  /* This is returned from HTMLPositionToId. */
  int docid;

  /* This is returned from HTMLGetWidgetInfo. */
  void *cached_widgets;

  /* This is returned from HTMLGetFormInfo. */
  void *cached_forms;

  /* Type of authorization */
  int authType;

  /* Internal Image Viewer file (if any) */
  char *image_file;

  /* Encryption cipher (if any) */
  char *cipher;
  char *cipher_issuer;
  int cipher_bits;
  int cipher_status;

  /* Character set (if any) */
  char *charset;

  /* FTP server type (if any) */
  char *ftp_type;

  /* Frame info */
  struct mo_frame *frames;

  struct mo_node *previous;
  struct mo_node *next;
} mo_node;

typedef struct mo_frame {
  char *url;
  int docid;
  void *cached_widgets;
  Widget scrolled_win;
  int level;
  int num;
  struct mo_frame *next;
} mo_frame;

/* ------------------------------------------------------------------------ */
/* ------------------------------ MISC TYPES ------------------------------ */
/* ------------------------------------------------------------------------ */

typedef enum {
  mo_fail = 0,
  mo_succeed
} mo_status;

typedef enum {
  mo_annotation_public = 0,
  mo_annotation_workgroup,
  mo_annotation_private
} mo_pubpri_token;


/* ---------------------------- A few globals ----------------------------- */

extern Display *dsp;

/* ------------------------------- menubar -------------------------------- */

typedef enum {
#ifdef KRB4
  mo_kerberosv4_login,
#endif
#ifdef KRB5
  mo_kerberosv5_login,
#endif
  mo_proxy, mo_no_proxy,
  mo_reload_document, mo_reload_document_and_images,
  mo_refresh_document, mo_clear_image_cache,
#ifdef CCI
  mo_cci,
#endif
  mo_document_source, mo_document_edit, mo_document_date, mo_search,
  mo_open_document, mo_open_local_document, mo_save_document,
  mo_mail_document, mo_print_document,
  mo_new_window, mo_clone_window,
  mo_close_window, mo_exit_program, mo_stop,
  mo_home_document, mo_ncsa_document,
  mo_mosaic_manual, mo_cookie_manager,
  mo_back, mo_forward, mo_history_list,
  mo_clear_global_history,
  mo_hotlist_postit, mo_register_node_in_default_hotlist,
  mo_all_hotlist_to_rbm, mo_all_hotlist_from_rbm,
  mo_network_search, mo_usenet_search, mo_people_search, mo_meta_search,
  mo_internet_metaindex, mo_list_search, mo_map_search, mo_auction_search,
  mo_encyclopedia_search,
  mo_large_fonts, mo_regular_fonts, mo_small_fonts,
  mo_large_helvetica, mo_regular_helvetica, mo_small_helvetica,
  mo_large_newcentury, mo_regular_newcentury, mo_small_newcentury,
  mo_large_lucidabright, mo_regular_lucidabright, mo_small_lucidabright,
  mo_help_about, mo_help_onwindow, mo_help_onversion, mo_help_faq,
  mo_help_vmsmosaic,
  mo_techsupport, mo_help_html, mo_help_url, mo_cc,
  mo_annotate,
#ifdef HAVE_AUDIO_ANNOTATIONS
  mo_audio_annotate,
#endif
  mo_annotate_edit, mo_annotate_delete,
  mo_checkout, mo_checkin,
  mo_fancy_selections,
  mo_default_underlines, mo_l1_underlines, mo_l2_underlines, mo_l3_underlines,
  mo_no_underlines, mo_binary_transfer,
  mo_binary_ftp_mode,

  /* Links window */
  mo_links_window,

  /* News Menu & Stuff */
  mo_news_prev, mo_news_next, mo_news_prevt, mo_news_nextt,
  mo_news_post, mo_news_cancel, mo_news_reply, mo_news_follow,
  mo_news_fmt0, mo_news_fmt1, mo_news_index, mo_news_list,
  mo_news_groups, mo_news_flush, mo_news_flushgroup,
  mo_news_grp0, mo_news_grp1, mo_news_grp2,
  mo_news_art0, mo_news_art1, mo_use_flush,
  mo_news_sub, mo_news_unsub, mo_news_sub_anchor, mo_news_unsub_anchor,
  mo_news_mread, mo_news_mread_anchor, mo_news_munread, mo_news_maunread,

  /* Other stuff */
  mo_re_init, mo_delay_image_loads, mo_table_support, mo_expand_images_current,
  mo_image_view_internal, mo_progressive_loads, mo_animate_images,
  mo_preferences, mo_save_preferences, mo_refresh_url, mo_blink_text,
  mo_frame_support, mo_hotkeys, mo_tooltips, mo_verify_certs, mo_multi_load,

  /* FTP */
  mo_ftp_put, mo_ftp_remove, mo_ftp_mkdir,

  mo_body_color, mo_body_images, mo_font_color, mo_font_sizes, mo_safe_colors,

  /* Debug stuff */
  mo_trace_cache, mo_trace_cci, mo_trace_html, mo_trace_http, mo_trace_nut,
  mo_trace_src, mo_trace_table, mo_trace_www2, mo_trace_refresh, mo_report_bugs,
  mo_trace_cookie,

  /* Password cash stuff */
  mo_clear_passwd_cache,

  /* NOTE!!!!!! THIS MUST ALWAYS BE LAST!!!!!! */
  mo_last_entry
} mo_token;

#include "prefs.h"

#endif
