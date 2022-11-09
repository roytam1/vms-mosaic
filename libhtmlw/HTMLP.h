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

#ifndef HTMLP_H
#define HTMLP_H

#include "../libhtmlw/HTML.h"
#include "../libwww2/HTBTree.h"

#include <Xm/XmP.h>
#ifdef MOTIF1_2
#include <Xm/ManagerP.h>
#endif

#include <X11/Xatom.h>
#ifndef VMS
#include <X11/Xmu/Atoms.h>
#else
#include <XMU/Atoms.h>
#endif

/* New fields for the HTML widget class */
typedef struct _HTMLClassPart {
	int none;		/* No extra HTML class stuff */
} HTMLClassPart;

typedef struct _HTMLClassRec {
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	XmManagerClassPart	manager_class;
	HTMLClassPart		html_class;
} HTMLClassRec;


extern HTMLClassRec htmlClassRec;

#define MARGIN_DEFAULT		20
#define IMAGE_DEFAULT_BORDER	2
#define DEF_IMAGE_HSPACE	0
#define DEF_IMAGE_VSPACE	0
#define D_INDENT_SPACES		40

#define D_NONE    0
#define D_TITLE   1
#define D_TEXT    2
#define D_OLIST   3
#define D_ULIST   4
#define D_DESC_LIST_START 5

/*
 * To allow arbitrary nesting of lists
 */
typedef struct dtype_rec {
        int type;               /* D_NONE, D_TITLE, D_TEXT, D_OLIST, D_ULIST */
        int count;
        int compact;
	int indent;
	char style;		/* 1, A, a, I, i */
	int in_title;
	int save_indent_level;
        struct dtype_rec *next;
} DescRec;
 
/*
 * To allow arbitrary nesting of alignment changes
 */
typedef struct align_rec {
        DivAlignType align;
        struct align_rec *next;
} AlignRec;

/*
 * To allow arbitrary nesting of alignment changes
 */
typedef struct float_rec {
	int type;		/* 1 is image, 2 is table */
	int marg;
	int y;
	int table_extra;	/* Size of extra space after table */
	int image_extra;	/* Space before right floating image */
        struct float_rec *next;
} FloatRec;

/* A struc to maintain current HTML formatting context */
typedef struct _PhotoComposeContext {
	int width_of_viewable_part;  /* Never change during computation */
	int right_margin;
	int left_margin;
	int eoffsetx;	      /* The element offset relative to View */
	int cur_line_width;   /* WidthOfViewablePart-right_margin-left_margin */
	int x;		      /* x, y (relative to View) to put next element */
	int y;
	/* When cw_only we never create Element but compute 3 values:
	 *    computed_min_x, computed_max_x, computed_maxmin_x
	 * This part is for first pass of table to compute cell sizes
	 */
	Boolean cw_only;	/* Compute width only if True */
	int computed_min_x;	/* Minimum cell width for this line */
	int computed_maxmin_x;	/* Max of all min_x in cell */
	int computed_max_x;	/* Maximum cell width */
	int margin_height;
	int cur_baseline;	/* All objects in a line must have the same
				 * baseline.  If baseline changes then adjust
				 * the element's y and cur_line_height and
				 * the y or height of each element on line */
	int cur_line_height;
	int element_id;    	/* Unique number */
	int is_bol;      	/* At begin of line if 1
				 * Right after list bullet if 2 */
	char have_space_after;  /* Word has a space after */
	XFontStruct     *cur_font;
	int	        cur_font_size;
	int	        cur_font_base;
	CurFontFamily   cur_font_family;
	CurFontType     cur_font_type;
	MarkInfo *anchor_tag_ptr;  /* Anchor info */
	int max_width_return;	 /* Compute the MaxWidth of hyper text
				  * to adjust scrollbar.
				  * Initial value is WidthOfViewablePart */
	int pf_lf_state; 	 /* Linefeed state */
	int preformat;		 /* In <PRE>? */
	DivAlignType	div;	 /* Current horizontal alignment */
	AlignType	valign;	 /* Current vertical alignment */
	unsigned long	fg;	 /* Current foreground */
	unsigned long	bg;	 /* Current background */
	unsigned long	cur_font_color;
	int		underline_number;
	int		in_underlined;
	Boolean		dashed_underlines;
	Boolean		underline_start;  /* Indicate starting underlining */
	FormInfo	*cur_form;	  /* Current form */
	Boolean		in_form;	  /* In form? */
	int		widget_id;
	int		applet_id;
	int		superscript;
	int		subscript;
	int		indent_level;
	char		*text_area_buf;	  /* Buffer for Form TextArea */
	char		*button_buf;	  /* Buffer for Form Button */
	Boolean		button_has_text;  /* Form Button text? */
	Boolean		button_has_image; /* Form Button image? */
	int		ignore;		  /* Ignore some tags when formating */
	SelectInfo	*current_select;  /* SELECT in FORM */
	Boolean		in_select;	  /* A select? */
	int		is_index;
	int		Width;
	Boolean		Strikeout;
	Boolean		strikeout_start;
	DescRec		DescType;
	int		InDocHead;
	char		*TitleText;
	Boolean		in_table;	/* In a table? */
	int		in_paragraph;   /* In a paragraph? */
	Boolean		in_div_hidden;	/* Visibility:hidden in DIV? */
	ElemInfo	*last_progressive_ele;	/* Last element displayed progressively */
	Boolean		anchor_start;	/* Mark start of anchor in text */
	Boolean		at_top;		/* At top of a page or cell? */
	Boolean		in_anchor;	/* In an anchor? */
	Boolean		fixed_anchor_color;  /* Fixed anchor color? */
	Boolean		in_label;	/* In a label? */
	char 		*label_id;	/* Label FOR=id */
	FloatRec	*float_left;	/* Current float stacks */
	FloatRec	*float_right;
	Boolean		ignore_float;
	Boolean		nobr;		/* In <NOBR>? */
	int		nobr_x;		/* Starting x of <NOBR> */
	int		blockquote;	/* <BLOCKQUOTE> counter */
	int		frameset;	/* <FRAMESET> counter */
	int		noframes;	/* <NOFRAMES> counter */
	int		blink;
	int		max_line_ascent;
	int		para_y;		/* Starting x of paragraph */
	Boolean		noformat;	/* Are we formatting? */
	int		sub_or_sup;	/* Is it subscript or superscript? */
	char		*basetarget;	/* Target set by <BASE> */
	char		*mark_title;	/* Mark title */
	char		*span_title;	/* Span title */
} PhotoComposeContext;

/* New fields for the HTML widget */
typedef struct _HTMLPart {
	/* Resources */
	Dimension		margin_width;
	Dimension		margin_height;

	Widget			view;
	Widget			hbar;
	Widget			vbar;

	XtCallbackList		anchor_callback;
	XtCallbackList		base_callback;
	XtCallbackList		form_callback;
	XtCallbackList		title_callback;

	char			*title;
	char			*raw_text;
	char			*header_text;
	char			*footer_text;
	char			*charset;
	/*
	 * Without Motif we have to define our own foreground resource
	 * instead of using the manager's
	 */
	Pixel			anchor_fg;
	Pixel			visitedAnchor_fg;
	Pixel			activeAnchor_fg;
	Pixel			activeAnchor_bg;

        Boolean                 body_colors;
        Boolean                 body_images;
        Boolean                 font_colors;
	Boolean			font_sizes;
	CurFontFamily		font_family;
	int			font_base;

	int			max_colors_in_image;
	int			bg_image;
	Pixmap			bgmap_SAVE;
	Pixmap			bgclip_SAVE;
        int                     bg_height;
        int                     bg_width; 
	ImageInfo		*bg_pic_data;

        Pixel                   foreground_SAVE;
	Pixel			anchor_fg_SAVE;
	Pixel			visitedAnchor_fg_SAVE;
	Pixel			activeAnchor_fg_SAVE;
	Pixel			activeAnchor_bg_SAVE;
	Pixel			top_color_SAVE;
	Pixel			bottom_color_SAVE;    
        Pixel                   background_SAVE;
        Pixel                   formbuttonbackground;
    
	int			num_anchor_underlines;
	int			num_visitedAnchor_underlines;
	Boolean			dashed_anchor_lines;
	Boolean			dashed_visitedAnchor_lines;
	Boolean			fancy_selections;
	Boolean			is_index;
	int			percent_vert_space;

	XFontStruct		*font;
	XFontStruct		*italic_font;
	XFontStruct		*bold_font;
	XFontStruct		*bolditalic_font;
	XFontStruct		*meter_font;
	XFontStruct		*toolbar_font;
	XFontStruct		*fixed_font;
	XFontStruct		*fixedbold_font;
	XFontStruct		*fixeditalic_font;
	XFontStruct		*header1_font;
	XFontStruct		*header2_font;
	XFontStruct		*header3_font;
	XFontStruct		*header4_font;
	XFontStruct		*header5_font;
	XFontStruct		*header6_font;
	XFontStruct		*address_font;
	XFontStruct		*plain_font;
	XFontStruct		*plainbold_font;
	XFontStruct		*plainitalic_font;
	XFontStruct		*listing_font;

        XtPointer		previously_visited_test;
	XtCallbackList		image_callback;
	XtCallbackList		quantize_callback;
	Boolean			delay_image_loads;
	XtCallbackList		get_url_data_cb;
        XtCallbackList		pointer_motion_callback;
	Boolean			blinking_text;
	int			blink_time;
	Boolean			frame_support;
	XtCallbackList		frame_callback;
	Boolean			is_frame;
	FrameScrolling		scroll_bars;
	Boolean			multi_image_load;
	XtCallbackList		multi_load_callback;

	/* Private */
	Dimension		max_pre_width;
	Dimension		view_width;
	Dimension		view_height;
	int			doc_width;
	int			doc_height;
	int			scroll_x;
	int			scroll_y;
	Boolean			use_hbar;
	Boolean			use_vbar;
	ElemInfo		*formatted_elements;
	ElemInfo		*select_start;
	ElemInfo		*select_end;
	int			sel_start_pos;
	int			sel_end_pos;
	ElemInfo		*new_start;
	ElemInfo		*new_end;
	int			new_start_pos;
	int			new_end_pos;
	ElemInfo		*active_anchor;
	GC			drawGC;
	Pixel			cur_fg;
	Pixel			cur_bg;
	XFontStruct		*cur_font;
	Display			*dsp;
	int			press_x;
	int			press_y;
	Time			but_press_time;
	Time			selection_time;
	MarkInfo		*html_objects;
	MarkInfo		*html_header_objects;
	MarkInfo		*html_footer_objects;
	MarkInfo		*html_refresh_objects;
	WidgetInfo		*widget_list;
	WidgetInfo		*widget_focus;
	FormInfo		*form_list;
	MapInfo                 *map_list;
        Boolean 		drawing;
        unsigned int		draw_count;
        Boolean                 focus_follows_mouse;
        ElemInfo                *cached_tracked_ele;
	ElemInfo		*last_formatted_elem;
	ElemInfo		*last_formatted_line;
	ElemInfo		*first_formatted_line;
	MapInfo			*cur_map;	/* usemap being parsed */
	AreaInfo		*cur_area;	/* usemap area last parsed */
	Boolean			cursor_in_anchor;
	int			underline_yoffset;
	Boolean			table_cell_has_bg;
	int			redisplay_x;
	int			redisplay_y;
	int			redisplay_width;
	int			redisplay_height;
        unsigned int		refresh_count;
	Boolean			changing_font;
	int			pushfont_count;
	int			font_save_count;
	FontRec			*fontstack;
	Boolean			ignore_setvalues;
	ElemInfo		*blinking_elements;
	XtIntervalId		refresh_timer;
	RefreshInfo		*refreshdata;
	XtIntervalId		blink_timer;
	BlinkInfo		*blinkdata;
	XtWorkProcId		workprocid;
	WorkInfo		*workprocdata;
	int			allocation_index[256];
	ElemInfo		*title_elem;
	HTBTree			*image_src;

	/* Frame resources */
	FrameInfo	**frames;
	int		nframe;		/* Number of frames */
	int		node_count;	/* Number of history node cached in */
	FrameInfo	*iframe_list;	/* iframe list */
} HTMLPart;

typedef struct _HTMLRec {
	CorePart		core;
	CompositePart		composite;
	ConstraintPart		constraint;
	XmManagerPart		manager;
	HTMLPart		html;
} HTMLRec;

#endif  /* HTMLP_H */
