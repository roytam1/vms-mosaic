/* HTMLparse.h
 * Author: Gilles Dauphin
 * Version 3.2.1 [May97]
 *
 * Copyright (C) 1997 - G.Dauphin
 * Copyright (C) 1998, 1999, 2000, 2003, 2004, 2005, 2006 -
 * The VMS Mosaic Project
 */

#ifndef HTML_PARSE_H
#define HTML_PARSE_H

typedef enum _MarkType {
	M_INIT_STATE = -2,
	M_UNKNOWN = -1,		/* The first two must have this value */
	M_NONE = 0,		/* for compatibility */
    /* See comment below before adding to list */
	M_ANCHOR,
	M_APPLET,
	M_AREA,
	M_BASE,
	M_BASEFONT,
	M_BIG,
	M_BLINK,
	M_BOLD,
	M_BUTTON,
	M_CAPTION,
	M_CITATION,
	M_COMMENT,
	M_CODE,
	M_COL,
	M_COLGROUP,
	M_DESC_TITLE,
	M_DESC_TEXT,
	M_DEFINE,
	M_DOCTYPE,
	M_EMPHASIZED,
	M_FIGURE,
	M_FIXED,
	M_FONT,
	M_IFRAME,
	M_IMAGE,
	M_INPUT,
	M_ITALIC,
	M_KEYBOARD,
	M_LABEL,
	M_LEGEND,
	M_LINEBREAK,
	M_LINK,
	M_LIST_ITEM,
	M_MAP,
	M_META,
	M_MOSAIC,
	M_NCSA,
	M_NOBR,
	M_NOINDEX,		/* Non-standard Atomz tag */
	M_NOSCRIPT,
	M_OPTGROUP,
	M_OPTION,
	M_PARAM,
	M_BUGGY_TABLE,
	M_TBODY,
	M_TD_CELL_PAD,
	M_TD_CELL_FREE,
	M_TD_CELL_PROPAGATE,
	M_TEXTAREA,
	M_TFOOT,
	M_THEAD,
	M_TITLE,
	M_SAMPLE,
	M_SCRIPT,
	M_SELECT,
	M_SMALL,
	M_SPACER,
	M_SPAN,
	M_STRIKEOUT,
	M_STRONG,
	M_STYLE,
	M_SUB,
	M_SUP,
	M_UNDERLINED,
	M_VARIABLE,
    /* Block elements below M_ADDRESS, text elements above */
	M_ADDRESS,
	M_BLOCKQUOTE,
	M_CENTER,
	M_DESC_LIST,
	M_DIRECTORY,
	M_DIV,
	M_DOC_BODY,
	M_DOC_HEAD,
	M_FIELDSET,
	M_FORM,
	M_FRAME,
	M_FRAMESET,
	M_HEADER_1,
	M_HEADER_2,
	M_HEADER_3,
	M_HEADER_4,
	M_HEADER_5,
	M_HEADER_6,
	M_HRULE,
	M_HTML,
	M_INDEX,
	M_LISTING_TEXT,
	M_MENU,
	M_NOFRAMES,
	M_NUM_LIST,
	M_PARAGRAPH,
	M_PLAIN_FILE,
	M_PLAIN_TEXT,
	M_PREFORMAT,
	M_TABLE,
	M_TABLE_DATA,
	M_TABLE_HEADER,
	M_TABLE_ROW,
	M_UNUM_LIST
    /* See comment above before adding to list */
} MarkType;

/* Syntax of Mark types */
#define	MT_ANCHOR	"a"
#define	MT_ADDRESS	"address"
#define MT_APPLET	"applet"
#define MT_AREA		"area"
#define MT_BOLD		"b"
#define MT_BASE		"base"
#define MT_BASEFONT	"basefont"
#define MT_BIG		"big"
#define MT_BLINK	"blink"
#define MT_BLOCKQUOTE	"blockquote"
#define MT_DOC_BODY     "body"
#define MT_LINEBREAK	"br"
#define MT_BUTTON	"button"
#define MT_CAPTION	"caption"
#define MT_CENTER	"center"
#define MT_CITATION	"cite"
#define MT_CODE		"code"
#define MT_COL		"col"
#define MT_COLGROUP	"colgroup"
#define	MT_DELETED	"del"
#define	MT_DESC_TEXT	"dd"
#define MT_DEFINE	"dfn"
#define MT_DIRECTORY	"dir"
#define MT_DIV		"div"
#define	MT_DESC_LIST	"dl"
#define	MT_DESC_TITLE	"dt"
#define MT_DOCTYPE	"!DOCTYPE"
#define MT_EMPHASIZED	"em"
#define MT_FIELDSET	"fieldset"
#define MT_FIGURE	"fig"
#define MT_FONT		"font"
#define MT_FORM		"form"
#define MT_FRAME	"frame"
#define MT_FRAMESET	"frameset"
#define	MT_HEADER_1	"h1"
#define	MT_HEADER_2	"h2"
#define	MT_HEADER_3	"h3"
#define	MT_HEADER_4	"h4"
#define	MT_HEADER_5	"h5"
#define	MT_HEADER_6	"h6"
#define MT_DOC_HEAD     "head"
#define MT_HRULE	"hr"
#define MT_HTML		"html"
#define MT_ITALIC	"i"
#define MT_IFRAME	"iframe"
#define MT_IMAGE	"img"
#define MT_INPUT	"input"
#define MT_INSERTED	"ins"
#define MT_INDEX	"isindex"
#define MT_KEYBOARD	"kbd"
#define	MT_LABEL	"label"
#define	MT_LEGEND	"legend"
#define	MT_LIST_ITEM	"li"
#define MT_LINK		"link"
#define	MT_LISTING_TEXT	"listing"
#define MT_MAP		"map"
#define MT_MENU		"menu"
#define MT_META		"meta"
#define MT_MOSAIC	"mosaic"
#define MT_NCSA		"ncsa-"
#define MT_NOBR		"nobr"
#define	MT_NOINDEX	"noindex"	/* Non-standard Atomz tag */
#define MT_NOFRAMES	"noframes"
#define MT_NOSCRIPT	"noscript"
#define	MT_NUM_LIST	"ol"
#define MT_OPTGROUP	"optgroup"
#define MT_OPTION	"option"
#define	MT_PARAGRAPH	"p"
#define	MT_PARAM	"param"
#define	MT_PLAIN_FILE	"plaintext"
#define	MT_PREFORMAT	"pre"
#define MT_SAMPLE	"samp"
#define MT_SCRIPT	"script"
#define MT_SELECT	"select"
#define MT_SMALL	"small"
#define MT_SPACER	"spacer"
#define MT_SPAN		"span"
#define MT_STRIKEOUT	"strike"
#define MT_STRIKEOUT2	"s"
#define MT_STRONG	"strong"
#define MT_STYLE	"style"
#define MT_SUB          "sub"
#define MT_SUP          "sup"
#define MT_TABLE	"table"
#define MT_TBODY	"tbody"
#define MT_TABLE_DATA	"td"
#define MT_TEXTAREA	"textarea"
#define MT_TFOOT	"tfoot"
#define MT_TABLE_HEADER	"th"
#define MT_THEAD	"thead"
#define	MT_TITLE	"title"
#define MT_TABLE_ROW	"tr"
#define MT_FIXED	"tt"
#define MT_UNDERLINED   "u"
#define	MT_UNUM_LIST	"ul"
#define MT_VARIABLE	"var"
#define	MT_PLAIN_TEXT	"xmp"

/* &quot marker character */
#define QUOT_CONST '\237'
/* Non blank space character */
#define NBSP_CONST '\240'

typedef struct _AppletRec	*AppletPtr;
typedef struct _TableRec	*TablePtr;
typedef struct image_rec	*ImageInfoPtr;

typedef struct mark_up {
	MarkType type;
	int is_end;
	char *start;
	char *text;
	int is_white_text;	/* Is text only with 'white-space' chars ? */
	struct mark_up *next;
	AppletPtr s_ats;	/* Applet saved */
	ImageInfoPtr s_picd;	/* Saved image */
	TablePtr t_p1;		/* First table pass */
	char *anc_name;
	char *anc_href;
	char *anc_title;
	char *anc_target;
} MarkInfo;

extern int caseless_equal(char *str1, char *str2);
extern void clean_white_space(char *txt);
extern char *ParseMarkTag(char *text, char *mtext, char *mtag);

#endif  /* HTML_PARSE_H */
