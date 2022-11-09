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

/* Copyright (C) 1998, 1999, 2000, 2002, 2004, 2005, 2006, 2007
 * The VMS Mosaic Project
 */

#ifndef HTMLW_HTML_H
#define HTMLW_HTML_H

#include <Xm/Xm.h>
#if (XmVERSION == 1) && (XmREVISION >= 2)
#undef MOTIF1_2
#define MOTIF1_2
#endif

#include <X11/StringDefs.h>

#ifndef HTML_PARSE_H
#include "../libhtmlw/HTMLparse.h"
#endif

/* Defines and structures used for the HTML parser, and parsed object list. */

typedef struct _HTMLClassRec *HTMLWidgetClass;
typedef struct _HTMLRec      *HTMLWidget;

extern WidgetClass htmlWidgetClass;

typedef enum {
	DIV_ALIGN_NONE,
	DIV_ALIGN_LEFT,
	DIV_ALIGN_CENTER,
	DIV_ALIGN_RIGHT
} DivAlignType;

typedef int (*visitTestProc)(Widget, char*);
typedef void (*pointerTrackProc)();

typedef struct ele_ref_rec {
	int id;
	int pos;
} ElementRef;

typedef struct link_rec {
	char *href;
} LinkInfo;

/*
 * Public functions
 */
extern char *HTMLGetText(Widget w, int pretty, char *url, char *time_str);
extern char *HTMLGetTextAndSelection(Widget w, char **startp, char **endp,
				     char **insertp);
extern char **HTMLGetHRefs(Widget w, int *num_hrefs);
extern char **HTMLGetImageSrcs(Widget w, int *num_srcs);
extern void *HTMLGetWidgetInfo(Widget w);
extern void *HTMLGetFormInfo(Widget w);
extern void HTMLCacheWidgetInfo(void *ptr);
extern void HTMLFreeWidgetInfo(void *ptr);
extern void HTMLFreeFormInfo(void *ptr);
extern LinkInfo *HTMLGetLinks(Widget w, int *num_links);
extern int HTMLPositionToId(Widget w, int x, int y);
extern int HTMLIdToPosition(Widget w, int element_id, int *x, int *y);
extern int HTMLAnchorToPosition(Widget w, char *name, int *x, int *y);
extern int HTMLAnchorToId(Widget w, char *name);
extern void HTMLGotoId(Widget w, int element_idi, int correction);
extern int HTMLLastId(Widget w);
extern void HTMLRetestAnchors(Widget w, visitTestProc testFunc);
extern void HTMLClearSelection(Widget w);
extern void HTMLSetSelection(Widget w, ElementRef *start, ElementRef *end);
extern void HTMLSetText(Widget w, char *text, char *header_text,
			char *footer_text, int element_id, char *target_anchor,
			void *ptr, char *refresh_text, char *charset);
extern int HTMLSearchText(Widget w, char *pattern, ElementRef *m_start,
			  ElementRef *m_end, int backward, int caseless);
extern int HTMLSearchNews(Widget w, ElementRef *m_start, ElementRef *m_end);
extern void HTMLTraverseTabGroups();
extern void HTMLSetFocusPolicy(Widget w, int to);
extern void HTMLDrawBackgroundImage(Widget wid, int x, int y, int width,
				    int height);
extern void HTMLFreeWidget(HTMLWidget hw);
extern void GetMailtoKludgeInfo(char **url, char **subject);

/*
 * Public Structures
 */
typedef struct acall_rec {
	XEvent *event;
	char *text;
	char *href;
	char *title;
	char *frame;
	Boolean refresh;	/* If true, reload if URL is same as current */
} WbAnchorCallbackData;

typedef struct fcall_rec {
	XEvent *event;
	char *href;
	char *target;
        char *method;
        char *enctype;
	int attribute_count;
	char **attribute_names;
	char **attribute_values;
	char *freeit1;
	char *freeit2;
} WbFormCallbackData;

typedef struct form_rec {
	Widget hw;
	char *action;
	char *target;
        char *method;
        char *enctype;
	int start;
	int end;
        Widget button_pressed;  /* Match button pressed to one of submits */
	Boolean cw_only;	/* Mark it used by EstimateMinMaxTable */
	int cached;
	struct form_rec *next;
} FormInfo;

/* Client-Side Ismap */
typedef struct coord_rec {
      int x;
      int y;
      struct coord_rec *next;
} CoordInfo;

typedef struct area_rec {
      int shape;
      CoordInfo *coordList;
      char *href;
      char *alt;
      char *target;
      struct area_rec *next;
} AreaInfo;

typedef struct map_rec {
      char *name;
      AreaInfo *areaList;
      struct map_rec *next;
} MapInfo;

/*
 * Defines for client-side ismap
 */
#define AREA_RECT 0
#define AREA_CIRCLE 1
#define AREA_POLY 2

/* Define alignment values */
typedef enum {
	ALIGN_NONE,
	VALIGN_BOTTOM,
	VALIGN_MIDDLE,
	VALIGN_TOP,
	HALIGN_LEFT,
	HALIGN_CENTER,
	HALIGN_RIGHT
} AlignType;

typedef struct image_rec {
        char *src;
	char *alt_text;		/* Alternative text */
	AlignType align;
	AlignType valign;
	int height;
	int req_height;		/* Requested height specified in HEIGHT=nnn */
	int width;
	int req_width;		/* Requested width specified in WIDTH=nnn */
	int percent_width;	/* Width specified with WIDTH=nn% */
	int border;
	int has_border;
	int hspace;
	int vspace;
	char *usemap;
        MapInfo *map;
        AreaInfo *area;
	int ismap;
	FormInfo *fptr;
	int internal;
	int delayed;
	int urldelayed;
	int fetched;
	int cached;
	int num_colors;
	XColor colrs[256];
        int bg_index;
	unsigned char *image_data;
	unsigned char *clip_data;
	int transparent;
	Pixel background_pixel;
	Pixmap image;
	Pixmap clip;
	int cw_only;
	char *text;		/* Special ISMAP Form NAME text */
	int is_bg_image;	/* Is it the background image? */
	struct anim_rec *anim_info; /* Animation stuff */
	int delay;		/* Image display time */
	int disposal;		/* GIF image disposal */
	int iterations;		/* Animation cycles */
	int x;			/* x and y offsets for animations */
	int y;
	int awidth;		/* Animation screen size */
	int aheight;
	int running;		/* 1 = running, 2 = ran, 0 = not started */
	Pixmap anim_image;	/* Current animation Pixmap */
	Pixmap bg_image;	/* Saved background Pixmap for animation */
	int bg_visible;		/* Is the background fully within view? */
	int has_anim_image;	/* 0 = None, -1 = empty, 1 = in use */
	struct image_rec *last;	/* Last image of animation */	
	struct image_rec *prev;
	struct image_rec *next;	/* Linked list of animation images */
	int aligned;
	XtIntervalId timer;	/* Animation timer id */
	unsigned char *alpha;	/* Alpha channel */
	unsigned char *alpha_image_data; /* Alpha background image data */
	int saved_x;		/* Current Eleminfo x,y of image with alpha */
	int saved_y;
	unsigned char *rgb;	/* Used by ImageQuantize callback */
	XColor *ori_colrs;	/* Original colors for Alpha channel use */
	int ori_num_colors;
} ImageInfo;

typedef struct anim_rec {
	HTMLWidget hw;
	struct ele_rec *eptr;
	int drawing;
	ImageInfo *start;
	ImageInfo *next;
	ImageInfo *current;
	int count;
	int *window;
} AnimInfo;

typedef struct refresh_rec {
	HTMLWidget hw;
	unsigned int refresh;
	char *url;
} RefreshInfo;

typedef struct wid_rec {
	Widget w;
	int type;
	int id;
	int x;
	int y;
	int width;
	int height;
        Boolean seeable;
        Boolean display;
	int extra_before;
	char *name;
	char *value;
	char *onclick;
	char *password;
	char **mapping;
	Boolean checked;
	Boolean mapped;
	Boolean data_cached;
        Boolean toggled;
	char *cached_text;
	unsigned long bgcolor;
	unsigned long fgcolor;
	int cache_count;
	int cache_invalid;
	FormInfo *cached_forms;
	Widget child;
	struct ele_rec *eptr;
	struct wid_rec *next;
	struct wid_rec *prev;
} WidgetInfo;

typedef struct _CellStruct {
        MarkType cell_type;
        int td_count;
        int tr_count;
        int colspan;
        int rowspan;
	int back_cs;
	int back_rs;
        MarkInfo *td_start;
        MarkInfo *td_end;
        MarkInfo *tr_start;
	struct ele_rec *start_elem;
	struct ele_rec *end_elem;
	int x;
	int y;
        int height;
        int width;
        int max_width;
        int min_width;
	int relative_width;
	int absolute_width;
	int req_height;
	int treq_height;
	int line_bottom;
        AlignType valignment;
        AlignType halignment;
	int nowrap;
	Boolean has_content;
	int content_height;
	int group;
} CellStruct;

typedef struct _ColumnList {
        CellStruct *cells;
        int cell_count;
        int max_row_span;
} ColumnList;

typedef struct _RowList {
        CellStruct **cells_lines;
        int row_count;
        int max_cell_count_in_line;
        int low_cur_line_num;
} RowList;

typedef struct {
        unsigned int halign;
        unsigned int valign;
        int rel_width;
	int abs_width;
	int prop_width;
	int group;
} ColElemInfo;

typedef enum {
	VOID,
	ABOVE,
	BELOW,
	HSIDES,
	VSIDES,
	LHS,
	RHS,
	BOX
} FrameType;

typedef enum {
	NONE,
	GROUPS,
	ROWS,
	COLS,
	ALL
} RulesType;

typedef struct _TableRec {
	int borders;
	int outer_border;		/* Outer border width */
	FrameType frame;		/* Outer border type */
	RulesType rules;		/* Rules type */
	unsigned int relative_width;	/* For <table width=50%> */
					/* it's relative to window width */
	unsigned int absolute_width;	/* Width in pixels */
	int num_col;
	int num_row;
	MarkInfo *caption_start_mark;
	MarkInfo *caption_end_mark;
	int captionAlignment;
	int captionIsLegend;
	MarkInfo *tb_start_mark;
	MarkInfo *tb_end_mark;
	MarkInfo *start_other_mark;
	MarkInfo *end_other_mark;
	int other_before_caption;
	MarkInfo *first_tr_mark;
	RowList  *row_list;
	int width, height;
	int min_width, max_width;
	int *col_max_w;		/* Min and max widths of each columns */
	int *col_min_w;
	int *col_w;		/* Definitive size of columns */
	int *col_req_w;		/* Requested width, (0 if none, -1 if */
				/* absolute, percentage if relative) */
	int *col_abs_w;		/* Suggested absolute width */
	int estimate_height;	/* Computed estimated height */
	DivAlignType align;
	int align_adjx;		/* x alignment adjustment */
	int valign_adjx;	/* y alignment adjustment */
	int cellpadding;
	int cellspacing;
	ColElemInfo *col_info;
} TableInfo;

typedef struct sel_rec {
	Widget hw;
	MarkInfo *mptr;
	int is_value;
	char *retval_buf;
	char *option_buf;
	char *label_buf;
	char **returns;
	char **options;
	char **labels;
	int option_cnt;
	char **value;
	int value_cnt;
} SelectInfo;

typedef ImageInfo *(*resolveImageProc)();

/*
 * Defines and structures used for the formatted element list
 */
#define E_TEXT		(1 << 0)
#define E_BULLET	(1 << 1)
#define E_LINEFEED	(1 << 2)
#define E_IMAGE		(1 << 3)
#define E_WIDGET	(1 << 4)
#define E_HRULE		(1 << 5)
#define E_TABLE		(1 << 6)
#define E_ANCHOR	(1 << 7)
#define E_SPACER	(1 << 8)
#define E_CR		(1 << 9)
#define E_CELL_TABLE	(1 << 10)
#define E_APPLET	(1 << 11)
#define E_IFRAME	(1 << 12)

typedef enum {
	CODE_TYPE_UNKNOW,
	CODE_TYPE_BIN,
	CODE_TYPE_SRC,
	CODE_TYPE_APPLET
} CodeType;

typedef struct _AppletRec {
	CodeType ctype;
	char *src;
	int width;
	int height;
	int x;
	int y;
	int border_width;
	AlignType valignment;
	int param_count;
	char **param_name_t;
	char **param_value_t;
	int url_arg_count;
	char **url_arg;
	char **ret_filenames;
	Boolean cw_only;
	Widget w;
	Widget frame;
} AppletInfo;

typedef struct _EODataStruct {
	char *src;
	char *ret_filename;
	Boolean cw_only;
} EODataStruct;

typedef enum {
	TIMES,
	HELVETICA,
	CENTURY,
	LUCIDA,
	SYMBOL
} CurFontFamily;

typedef enum {
	FONT,
	ITALIC_FONT,
	BOLD_FONT,
	BOLDITALIC_FONT,
	FIXED_FONT,
	FIXEDBOLD_FONT,
	FIXEDITALIC_FONT,
	HEADER1_FONT,
	HEADER2_FONT,
	HEADER3_FONT,
	HEADER4_FONT,
	HEADER5_FONT,
	HEADER6_FONT,
	ADDRESS_FONT,
	PLAIN_FONT,
	PLAINBOLD_FONT,
	PLAINITALIC_FONT,
	LISTING_FONT
} CurFontType;

/*
 * To allow arbitrary nesting of font changes
 */
typedef struct font_rec {
        XFontStruct *font;
	int size;
	CurFontType type;
	CurFontFamily family;
	unsigned long color;
	Boolean color_ch;
        struct font_rec *next;
} FontRec;

/*
 * Possible types of frame sizes
 */
typedef enum {
        FRAME_SIZE_FIXED = 1,                 /* Size specified in pixels */
        FRAME_SIZE_RELATIVE,                  /* Size is relative */
        FRAME_SIZE_OPTIONAL                   /* Size is optional */
} FrameSize;

/*
 * What type of scrolling a frame should employ.
 */
typedef enum {
        FRAME_SCROLL_NONE = 1,
        FRAME_SCROLL_AUTO,
        FRAME_SCROLL_YES
} FrameScrolling;

/*
 * Possible Frame layout policies
 */
typedef enum {
        FRAMESET_LAYOUT_ROWS = 1,	/* Rows only */
        FRAMESET_LAYOUT_COLS = 2,	/* Columns only */
        FRAMESET_LAYOUT_ROW_COLS = 4    /* Left to right, top to bottom */
} FramesetLayout;

#define	FRAME_TYPE    1	 /* This is a frame with HTML inside */
#define	FRAMESET_TYPE 2	 /* HTML begins with frameset tag */
/* Note:
 *	A frameset may have the FRAME_TYPE because son of frameset
 *	The upper level frameset does not set the FRAME_TYPE
 */

typedef struct frame_rec {
	int		frame_type;	/* FRAMESET_TYPE, FRAME_TYPE */
	FrameScrolling	frame_scroll_type; /* Frame scrolling */
	int             frame_border;   /* Add a border to the frames? */
        int		frame_x;        /* Computed frame x-position */
        int		frame_y;        /* Computed frame y-position */
        Dimension	frame_width;    /* Computed frame width */
        Dimension	frame_height;   /* Computed frame height */
	Dimension       frame_size_s;	/* Saved frame size */
	FrameSize       frame_size_type; /* Horizontal frame size spec */
        String   	frame_src;      /* Source document */
        String   	frame_name;     /* Internal frame name */
        Dimension	frame_margin_width;   /* Frame margin width */
        Dimension	frame_margin_height;  /* Frame margin height */
        Boolean  	frame_resize;   /* May we resize this frame? */
	struct frame_rec *frame_parent_frameset; /* Parent frameset, if any */
        struct frame_rec *frame_next;   /* Next frame child, if any */
        struct frame_rec *frame_children; /* List of frames */
        FramesetLayout  frame_layout;	/* Frameset layout policy */
	/* IFRAME stuff */
	Widget		iframe;
        Boolean		seeable;
	Boolean 	mapped;
	Boolean		aligned;	/* Is it left/right aligned? */
	Boolean		cw_only;	/* Compute size only? */
} FrameInfo;

/*
 * Frame callback request type
 */
typedef enum {
        FRAME_CREATE = 0,
        FRAME_DELETE = 1,
        IFRAME_CREATE = 2
} FrameRequest;

typedef struct HTMLFrameCallbackStruct {
	FrameRequest reason;
	int doc_height;
	int doc_width;
} FrameCbData;

typedef struct ele_rec {
	int type;
	ImageInfo *pic_data;
	WidgetInfo *widget_data;
	TableInfo *table_data;
	CellStruct *cell_data;
	AppletInfo *ats;
	XFontStruct *font;
	AlignType valignment;
	AlignType halignment;
	Boolean selected;
	Boolean is_in_form;	    /* Indicate special ISMAP in form */
	int indent_level;
	int start_pos, end_pos;
	int x, y;		    /* The upper left corner of Bounding box */
	int baseline;		    /* Baseline for alignment */
	int bwidth;
	int width;
	int height;
	int ele_id;
	int applet_id;
	int underline_number;
	Boolean dashed_underline;
	Boolean strikeout;
	unsigned long fg;
	unsigned long bg;
	MarkInfo *anchor_tag_ptr;   /* Put it in struct mark_up */
	int fixed_anchor_color;     /* Has fixed anchor font color */
	char *edata;
	int edata_len;
	struct ele_rec *next;
	struct ele_rec *prev;
	struct ele_rec *line_next;
	int font_size;
	CurFontFamily font_family;
	CurFontType font_type;
	Boolean blink;
	struct ele_rec *blink_next;
	int underline_yoffset;
	FrameInfo *frame;
	char *label_id;
	char *title;
} ElemInfo;

typedef struct {
        ElemInfo *eptr;
        int drawing;
        HTMLWidget hw;
} WorkInfo;

typedef struct {
        int drawing;
        HTMLWidget hw;
} BlinkInfo;

/* Anchor tags */
#define	AT_NAME		"name"
#define	AT_HREF		"href"
#define	AT_TITLE	"title"

/*
 * New resource names
 */
#define	WbNmarginWidth		"marginWidth"
#define	WbNmarginHeight		"marginHeight"
#define	WbNtext			"text"
#define	WbNheaderText		"headerText"
#define	WbNfooterText		"footerText"
#define	WbNcharSet		"charSet"
#define	WbNtitleText		"titleText"
#define	WbNanchorUnderlines	"anchorUnderlines"
#define	WbNvisitedAnchorUnderlines	 "visitedAnchorUnderlines"
#define	WbNdashedAnchorUnderlines	 "dashedAnchorUnderlines"
#define	WbNdashVisitedAnchorUnderlines   "dashVisitedAnchorUnderlines"
#define	WbNanchorColor		"anchorColor"
#define	WbNvisitedAnchorColor	"visitedAnchorColor"
#define	WbNactiveAnchorFG	"activeAnchorFG"
#define	WbNactiveAnchorBG	"activeAnchorBG"
#define	WbNfancySelections	"fancySelections"
#define	WbNimageBorders		"imageBorders"
#define	WbNdelayImageLoads	"delayImageLoads"
#define	WbNisIndex		"isIndex"
#define	WbNitalicFont		"italicFont"
#define	WbNboldFont		"boldFont"
#define	WbNboldItalicFont	"boldItalicFont"
#define	WbNfixedFont		"fixedFont"
#define	WbNmeterFont		"meterFont"
#define	WbNtoolbarFont		"toolbarFont"
#define	WbNfixedboldFont	"fixedboldFont"
#define	WbNfixeditalicFont	"fixeditalicFont"
#define	WbNheader1Font		"header1Font"
#define	WbNheader2Font		"header2Font"
#define	WbNheader3Font		"header3Font"
#define	WbNheader4Font		"header4Font"
#define	WbNheader5Font		"header5Font"
#define	WbNheader6Font		"header6Font"
#define	WbNaddressFont		"addressFont"
#define	WbNplainFont		"plainFont"
#define	WbNplainboldFont	"plainboldFont"
#define	WbNplainitalicFont	"plainitalicFont"
#define	WbNlistingFont		"listingFont"
#define	WbNanchorCallback	"anchorCallback"
#define	WbNbaseCallback		"baseCallback"
#define	WbNtitleCallback	"titleCallback"
#define	WbNsubmitFormCallback	"submitFormCallback"
#define	WbNpreviousVisitedTestFunction "previousVisitedTestFunction"
#define WbNmaxColorsInImage	"maxColorsInImage"
#define WbNimageCallback	"imageCallback"
#define WbNimageQuantizeCallback "imageQuantizeCallback"
#define WbNgetUrlDataCB		"getUrlDataCB"

#define	WbNpercentVerticalSpace "percentVerticalSpace"
#define WbNpointerMotionCallback "pointerMotionCallback"
#define WbNview			 "view"
#define WbNverticalScrollBar	 "verticalScrollBar"
#define WbNhorizontalScrollBar	 "horizontalScrollBar"
#define WbNbodyColors            "bodyColors"
#define WbNbodyImages            "bodyImages"
#define WbNbodyBG                "bodyBG"
#define WbNformButtonBackground  "formButtonBackground"
#define WbNfontColors            "fontColors"
#define WbNfontSizes             "fontSizes"
#define WbNfontBase              "fontBase"
#define WbNfontFamily            "fontFamily"
#define WbNblinkingText          "blinkingText"
#define WbNblinkTime             "blinkTime"
#define WbNframeCallback	 "frameCallback"
#define WbNframeSupport		 "frameSupport"
#define WbNisFrame		 "isFrame"
#define WbNscrollBars		 "scrollBars"
#define WbNmultiImage		 "multiImage"
#define WbNmultiLoadCallback	 "multiLoadCallback"
/*
 * New resource classes
 */
#define	WbCMarginWidth		"MarginWidth"
#define	WbCMarginHeight		"MarginHeight"
#define	WbCText			"Text"
#define	WbCHeaderText		"HeaderText"
#define	WbCFooterText		"FooterText"
#define	WbCCharSet		"CharSet"
#define	WbCTitleText		"TitleText"
#define	WbCAnchorUnderlines	"AnchorUnderlines"
#define	WbCVisitedAnchorUnderlines	 "VisitedAnchorUnderlines"
#define	WbCDashedAnchorUnderlines	 "DashedAnchorUnderlines"
#define	WbCDashVisitedAnchorUnderlines   "DashVisitedAnchorUnderlines"
#define	WbCAnchorColor		"AnchorColor"
#define	WbCVisitedAnchorColor	"VisitedAnchorColor"
#define	WbCActiveAnchorFG	"ActiveAnchorFG"
#define	WbCActiveAnchorBG	"ActiveAnchorBG"
#define	WbCFancySelections	"FancySelections"
#define	WbCImageBorders		"ImageBorders"
#define	WbCDelayImageLoads	"DelayImageLoads"
#define	WbCIsIndex		"IsIndex"
#define	WbCItalicFont		"ItalicFont"
#define	WbCBoldFont		"BoldFont"
#define	WbCBoldItalicFont	"BoldItalicFont"
#define	WbCFixedFont		"FixedFont"
#define	WbCMeterFont		"MeterFont"
#define	WbCToolbarFont		"ToolbarFont"
#define	WbCFixedboldFont	"FixedboldFont"
#define	WbCFixeditalicFont	"FixeditalicFont"
#define	WbCHeader1Font		"Header1Font"
#define	WbCHeader2Font		"Header2Font"
#define	WbCHeader3Font		"Header3Font"
#define	WbCHeader4Font		"Header4Font"
#define	WbCHeader5Font		"Header5Font"
#define	WbCHeader6Font		"Header6Font"
#define	WbCAddressFont		"AddressFont"
#define	WbCPlainFont		"PlainFont"
#define	WbCPlainboldFont	"PlainboldFont"
#define	WbCPlainitalicFont	"PlainitalicFont"
#define	WbCListingFont		"ListingFont"
#define	WbCPreviousVisitedTestFunction "PreviousVisitedTestFunction"
#define WbCMaxColorsInImage	"MaxColorsInImage"
#define WbCImageCallback	"ImageCallback"
#define WbCImageQuantizeCallback "ImageQuantizeCallback"
#define WbCGetUrlDataCB		"GetUrlDataCB"

#define	WbCPercentVerticalSpace "PercentVerticalSpace"
#define WbCPointerMotionCallback "PointerMotionCallback"
#define WbCVerticalScrollOnRight "VerticalScrollOnRight"
#define WbCHorizontalScrollOnTop "HorizontalScrollOnTop"
#define WbCView			 "View"
#define WbCVerticalScrollBar	 "VerticalScrollBar"
#define WbCHorizontalScrollBar	 "HorizontalScrollBar"
#define WbCBodyColors            "BodyColors"
#define WbCBodyImages            "BodyImages"
#define WbCBodyBG                "BodyBG"
#define WbCFormButtonBackground  "FormButtonBackground"
#define WbCFontColors            "FontColors"
#define WbCFontSizes             "FontSizes"
#define WbCFontBase              "FontBase"
#define WbCFontFamily            "FontFamily"
#define WbCBlinkingText          "BlinkingText"
#define WbCBlinkTime             "BlinkTime"
#define WbCFrameCallback	 "FrameCallback"
#define WbCFrameSupport		 "FrameSupport"
#define WbCIsFrame		 "IsFrame"
#define WbCScrollBars		 "ScrollBars"
#define WbCMultiImage		 "MultiImage"
#define WbCMultiLoadCallback	 "MultiLoadCallback"

#endif  /* HTMLW_HTML_H */
