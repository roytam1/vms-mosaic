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
#include "../config.h"

/* Some part of this file is Copyright (C) 1996 - G.Dauphin
 *
 * Copyright (C) 1998, 1999, 2000, 2003, 2004, 2005, 2006, 2007
 * The VMS Mosaic Project
 */

#include <time.h>
#ifndef VMS
struct timeval Tv;
struct timezone Tz;
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "HTMLmiscdefs.h"
#include "HTMLparse.h"
#include "HTMLP.h"
#include "HTMLPutil.h"
#include "../src/mosaic.h"

#include "HTMLform.h"
#include "HTMLframe.h"
#include "HTMLfont.h"

#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/Label.h>

#if defined(MULTINET) && defined(__DECC) && (__VMS_VER >= 70000000)
#define strdup  decc$strdup
#endif /* VMS V7, VRH, GEC, MPJZ */
extern char *strdup();

#include "../libnut/str-tools.h"

/* Locale-independent */
#define ISSPACE(x) ((x) > 0 && ((x) <= ' ') || ((x) >= 127) && ((x) < 160))

int appletSupportEnabled = 0;
int tableSupportEnabled;
extern int progressiveDisplayEnabled;

#ifndef DISABLE_TRACE
int refreshTrace;
extern int htmlwTrace;
extern int reportBugs;
#endif

static MarkInfo NULL_ANCHOR = {
	M_ANCHOR,		/* MarkType */
	1,			/* is_end */
	NULL,			/* start */
	NULL,			/* text */
	0,			/* is_white_text */
	NULL,			/* next */
	NULL, NULL, NULL,	/* Saved stuff */
	NULL,			/* anchor_name */
	NULL,			/* anchor_href */
	NULL,			/* anchor_title */
	NULL			/* anchor_target */
};

MarkInfo *NULL_ANCHOR_PTR = &NULL_ANCHOR;

static AlignRec AlignBase;
static AlignRec *AlignStack;

static DescRec BaseDesc;
DescRec *DescType;
static char *TitleText = NULL;
static XFontStruct *nonScriptFont;
static int InDocHead;
static int InDocBody;
static int header1_align;  /* Counters for header alignments */
static int header2_align;
static int header3_align;
static int header4_align;
static int header5_align;
static int header6_align;
static int header1;        /* Header counters */
static int header2;
static int header3;
static int header4;
static int header5;
static int header6;

/* Set the formatted element into the format list. */
ElemInfo *CreateElement(HTMLWidget hw, int type, XFontStruct *fp,
			int x, int y, int width, int height, int baseline,
			PhotoComposeContext *pcc)
{
	ElemInfo *eptr;

	if (pcc->cw_only)
		return NULL;

	/* Get an element */
	eptr = GetElemRec();
	if (hw->html.last_formatted_elem) {
		hw->html.last_formatted_elem->next = eptr;
		eptr->prev = hw->html.last_formatted_elem;
	}
	hw->html.last_formatted_elem = eptr;

	if (!hw->html.formatted_elements)	/* The first element */
		hw->html.formatted_elements = eptr;

	/* Now we work with 'eptr' and start the stuff */
	eptr->type = type;
	eptr->font = fp;
	eptr->x = x;
	eptr->y = y;
	eptr->width = width;
	eptr->height = height;
	eptr->baseline = baseline;
	eptr->ele_id = ++pcc->element_id;
	eptr->indent_level = pcc->indent_level;
	eptr->valignment = VALIGN_BOTTOM;
	eptr->halignment = HALIGN_LEFT;
	eptr->selected = False;
	eptr->bwidth = IMAGE_DEFAULT_BORDER;
	eptr->underline_number = pcc->underline_number;
	eptr->dashed_underline = pcc->dashed_underlines;
	eptr->strikeout = pcc->Strikeout;
	eptr->fg = pcc->fg;
	eptr->bg = pcc->bg;
        eptr->fixed_anchor_color = pcc->fixed_anchor_color;
        eptr->anchor_tag_ptr = pcc->anchor_tag_ptr;  /* It's in struct markup */
	/** GetElemRec zeros it
        eptr->edata = NULL;
	**/
	eptr->font_size = pcc->cur_font_size;
	eptr->font_type = pcc->cur_font_type;
	eptr->font_family = pcc->cur_font_family;
	eptr->underline_yoffset = -1;
	if (pcc->label_id)
		eptr->label_id = strdup(pcc->label_id);
	if (pcc->mark_title)
		eptr->title = strdup(pcc->mark_title);
	return eptr;
}

void AdjustBaseLine(ElemInfo *eptr, PhotoComposeContext *pcc)
{
	int add_clh, dbase, supsubBaseline;
	int cur_baseline = pcc->cur_baseline;
	int ori_line_height = pcc->cur_line_height;
	int dtoptext = 0;

        if (pcc->subscript || pcc->superscript) {
		if (!nonScriptFont)
			nonScriptFont = pcc->cur_font;
		supsubBaseline = nonScriptFont->max_bounds.ascent;
		cur_baseline += (supsubBaseline * .4) * pcc->subscript;
		cur_baseline -= (supsubBaseline * .4) * pcc->superscript;
	}

	/* Compute TEXTTOP alignment stuff */
	if (pcc->cur_font->ascent > pcc->max_line_ascent) {
		dtoptext = pcc->cur_font->ascent - pcc->max_line_ascent;
		pcc->max_line_ascent = pcc->cur_font->ascent;
	}

	/* Adjust baseline of element */
	if (eptr->baseline <= cur_baseline) {
		if ((eptr->type == E_TEXT) || (eptr->type == E_BULLET)) {
			eptr->height += cur_baseline - eptr->baseline;
			eptr->baseline = cur_baseline;
		} else if ((eptr->type == E_IMAGE) ||
			   (eptr->type == E_SPACER) ||
			   (eptr->type == E_IFRAME)) {
			if (eptr->baseline >= 0) {
				if (pcc->cur_line_height < (eptr->height +
				    cur_baseline - eptr->baseline))
					pcc->cur_line_height = eptr->height +
						  cur_baseline - eptr->baseline;
				eptr->y += cur_baseline - eptr->baseline;
				eptr->baseline = cur_baseline;
			} else if (eptr->baseline == -1) {
				/* Center on middle of line */
				if (pcc->cur_line_height < eptr->height)
					pcc->cur_line_height = eptr->height;
				eptr->y += (pcc->cur_line_height -
					    eptr->height) / 2;
			} else if (eptr->baseline == -2) {
				/* Place on line bottom */
				if (pcc->cur_line_height < eptr->height)
					pcc->cur_line_height = eptr->height;
				eptr->y += pcc->cur_line_height - eptr->height;
			} else {
				/* Place at top of text */
				int add = cur_baseline - pcc->max_line_ascent;

				if (add > 0) {
					eptr->y += add;
					if ((eptr->height + add) >
					    pcc->cur_line_height)
						pcc->cur_line_height =
							     eptr->height + add;
				}
			}
		}
		if (pcc->cur_line_height < eptr->height)
			pcc->cur_line_height = eptr->height;
		/* Adjust any ABSMIDDLE and ABSBOTTOM stuff */
		if (pcc->cur_line_height > ori_line_height) {
			add_clh = pcc->cur_line_height - ori_line_height;
			while (eptr->prev) {
				eptr = eptr->prev;
				if (eptr->type == E_CR)
					break;
				if ((eptr->type == E_IMAGE) ||
				    (eptr->type == E_SPACER) ||
				    (eptr->type == E_IFRAME)) {
					if (eptr->baseline == -1) {
						eptr->y += add_clh / 2;
					} else if (eptr->baseline == -2) {
						eptr->y += add_clh;
					}
				}
			}
		}
		return;
	}

	/* eptr->baseline > cur_baseline */
	dbase = add_clh = eptr->baseline - cur_baseline;
	/* Doing superscript? */
        if (pcc->superscript || pcc->subscript) {
		pcc->cur_baseline += dbase;
	} else {
		pcc->cur_baseline = eptr->baseline;
	}
	pcc->cur_line_height += dbase;
	if (pcc->cur_line_height < eptr->height) {
		add_clh += eptr->height - pcc->cur_line_height;
		pcc->cur_line_height = eptr->height;
	}
	while (eptr->prev) {	/* Adjust baseline and height of rest of line */
		eptr = eptr->prev;
		if (eptr->type == E_CR)
			break;
		if ((eptr->type == E_TEXT) || (eptr->type == E_BULLET)) {
			eptr->height += dbase;
			eptr->baseline += dbase;
		} else if (((eptr->type == E_IMAGE) &&
			    !eptr->pic_data->aligned) ||
			   ((eptr->type == E_IFRAME) &&
			    !eptr->frame->aligned) ||
			   (eptr->type == E_SPACER)) {
			if (eptr->baseline >= 0) {
				eptr->y += dbase;
				eptr->baseline += dbase;
			} else if (eptr->baseline == -1) {
				eptr->y += add_clh / 2;
			} else if (eptr->baseline == -2) {
				eptr->y += add_clh;
			} else {
				eptr->y += dbase - dtoptext;
			}
		}
	}
}

static void PushAlign(DivAlignType align)
{
	AlignRec *aptr = (AlignRec *) malloc(sizeof(AlignRec));

	CHECK_OUT_OF_MEM(aptr);
	aptr->align = align;
	aptr->next = AlignStack;
	AlignStack = aptr;
}

static DivAlignType PopAlign()
{
	DivAlignType align;

	if (AlignStack->next) {
		AlignRec *aptr = AlignStack;

		AlignStack = AlignStack->next;
		align = aptr->align;
		free(aptr);
	} else {
#ifndef DISABLE_TRACE
		if (htmlwTrace || reportBugs) 
		       fprintf(stderr, "Warning: popping empty align stack!\n");
#endif
		align = AlignStack->align;
	}
	return(align);
}

/* Horrible code for the TEXTAREA element.  Escape '\' and ''' by
 * putting a '\' in front of them, then replace all '"' with '''.
 * This lets us safely put the resultant value between double quotes.
 */
static char *TextAreaAddValue(char *value, char *text)
{
	char *buf, *bptr;
	char *tptr = text;
	int extra = 0;
        int vlen;

	if (!text || !*text)
		return(value);

	while (*tptr) {
		if ((*tptr == '\\') || (*tptr == '\''))
			extra++;
		tptr++;
	}

	vlen = strlen(value);
	value = (char *)realloc(value, vlen + strlen(text) + extra + 1);
	CHECK_OUT_OF_MEM(value);

	bptr = (char *)(value + vlen);
	tptr = text;
	while (*tptr) {
		if ((*tptr == '\\') || (*tptr == '\'')) {
			*bptr++ = '\\';
			*bptr++ = *tptr++;
		} else if (*tptr == '\"') {
			*bptr++ = '\'';
			tptr++;
		} else {
			*bptr++ = *tptr++;
		}
	}
	*bptr = '\0';
	return(value);
}

/* Create anchor element to hold NAME or ID attribute */ 
void CreateAnchorElement(HTMLWidget hw, MarkInfo *mark,
			 PhotoComposeContext *pcc)
{
	char *tptr = mark->anc_name;
	MarkInfo *tmp = pcc->anchor_tag_ptr;

	if (!tptr || !*tptr)
		return;

	pcc->anchor_tag_ptr = mark;
	CreateElement(hw, E_ANCHOR, pcc->cur_font, pcc->x, pcc->y, 0, 0, 0,pcc);
	pcc->anchor_tag_ptr = tmp;

        /* Strip out any spaces and linefeeds */
	if (strchr(tptr, ' ') || strchr(tptr, '\n')) {
		char *ptr;
		char *ptr2 = tptr;

		for (ptr = tptr; *ptr; ptr++, ptr2++) {
			while (*ptr && ((*ptr == ' ') || (*ptr == '\n')))
				ptr++;
			*ptr2 = *ptr;
		}
		*ptr2 = '\0';
	}
}

/*
 * Make necessary changes to formatting, based on the type of the
 * parsed HTML text we are formatting.
 * Some calls create elements that are added to the formatted element list.
 */
static void TriggerMarkChanges(HTMLWidget hw, MarkInfo **mptr,
			       PhotoComposeContext *pcc)
{
	MarkInfo *mark = *mptr;
	int type;
	char *tptr;

	type = mark->type;

	/* If we are not in a tag that belongs in the HEAD, end HEAD section */
	if ((InDocHead == 1) && (type != M_TITLE) && (type != M_NONE) &&
	    (type != M_BASE) && (type != M_INDEX) && (type != M_COMMENT) &&
	    (type != M_META) && (type != M_LINK) && (type != M_STYLE) &&
	    (type != M_SCRIPT) && (type != M_DOC_HEAD) && (type != M_UNKNOWN) &&
	    (type != M_MOSAIC)) {
		pcc->ignore = 0;
		InDocHead = -1;
		/* Finish processing the title; too late for </title> */
		if (TitleText) {
			hw->html.title = TitleText;
			TitleText = NULL;
			XtCallCallbackList((Widget)hw, hw->html.title_callback,
				           hw->html.title);
		}
	}
	/* If pcc->ignore is set, we ignore all further elements until we get
	 * to the end of the pcc->ignore.
	 * Let text through so we can grab the title text.
	 * Let title through so we can hit the end title.
	 * Also used for SELECT parsing.
	 * Let SELECT through so we can hit the end SELECT.
	 * Let OPTION and OPTGROUP through so we can hit them.
	 */
	if (pcc->ignore && (InDocHead != 1) && (type != M_TITLE) &&
	    (type != M_NONE) && (type != M_SELECT) && (type != M_OPTION) &&
	    (type != M_OPTGROUP) && (type != M_DOC_HEAD))
		return;

	/* If in non-formattable stuff, only let certain end marks thru */
	if (pcc->noformat && (type != M_DIV) && (type != M_SPAN) &&
	    (type != M_DOC_HEAD))
		return;

	/* A block element ends current paragraph */
	if (pcc->in_paragraph && (type >= M_ADDRESS)) {
		ConditionalLineFeed(hw, 1, pcc);
		pcc->div = PopAlign();
		/* Major big time ugly hack */
		if ((type == M_PARAGRAPH) && mark->is_end) {
			/* Was a floating image between empty <P></P> tags? */
			if ((pcc->in_paragraph > 1) &&
			    (pcc->para_y == pcc->y)) {
				pcc->pf_lf_state = 0;
				ConditionalLineFeed(hw, 1, pcc);
			}
		}
		pcc->in_paragraph = 0;
	}

	/* Check all non-end tags for ID and TITLE */
	if ((type != M_NONE) && !mark->is_end && !pcc->cw_only) {
		if (mark->anc_name = ParseMarkTag(mark->start, "A", "id"))
			CreateAnchorElement(hw, mark, pcc);
		if (pcc->mark_title)
			free(pcc->mark_title);
		pcc->mark_title = ParseMarkTag(mark->start, "A", "title");
		if (pcc->mark_title && !*pcc->mark_title) {
			free(pcc->mark_title);
			pcc->mark_title = NULL;
		}
		if (!pcc->mark_title && pcc->span_title)
			pcc->mark_title = strdup(pcc->span_title);
	}

	switch(type) {
	    /*
	     * Place the text.  Different functions based on whether it
	     * is pre-formatted or not.
	     */
	    case M_NONE:
		/* First translate any &quot characters */
		tptr = mark->text;
		while (*tptr) {
			if (*tptr == QUOT_CONST)
				*tptr = '\"';
			tptr++;
		}
		if (!pcc->cw_only && pcc->mark_title) {
			free(pcc->mark_title);
			pcc->mark_title = NULL;
		}
		if (pcc->ignore && !pcc->current_select) {
			/* Did we finish title already ? */
			if (!hw->html.title) {
				/* Did we start it yet? */
				if (!TitleText) {
					TitleText = strdup(mark->text);
				} else {
					TitleText = (char *)realloc(TitleText,
							strlen(TitleText) +
							strlen(mark->text) + 1);
					strcat(TitleText, mark->text);
				}
			}
			break;
		}
		if (pcc->ignore && pcc->current_select) {
			if (pcc->current_select->option_buf) {
				char *tptr = pcc->current_select->option_buf;

				tptr = (char *)realloc(tptr, strlen(tptr) +
					      	       strlen(mark->text) + 1);
				if (tptr) {
					strcat(tptr, mark->text);
					pcc->current_select->option_buf = tptr;
				}
			}
			break;
		}
		if (pcc->text_area_buf) {
			pcc->text_area_buf = TextAreaAddValue(
						pcc->text_area_buf, mark->text);
			break;
		}
		if (pcc->button_buf) {
			pcc->button_buf = TextAreaAddValue(pcc->button_buf,
							   mark->text);
			pcc->button_has_text = True;
			break;
		}
		if (!pcc->cw_only && pcc->anchor_tag_ptr->start) {
			pcc->mark_title = ParseMarkTag(
						     pcc->anchor_tag_ptr->start,
						     "A", "title");
			if (pcc->mark_title && !*pcc->mark_title) {
				free(pcc->mark_title);
				pcc->mark_title = NULL;
			}
			if (!pcc->mark_title && pcc->span_title)
				pcc->mark_title = strdup(pcc->span_title);
		}
		if (pcc->preformat) {
			PartOfPreTextPlace(hw, *mptr, pcc);
			break;
		} 
		PartOfTextPlace(hw, *mptr, pcc);
		break;
	    case M_CENTER:
		if (mark->is_end) {
			ConditionalLineFeed(hw, 1, pcc);
			pcc->div = PopAlign();
		} else {
			ConditionalLineFeed(hw, 1, pcc);
			PushAlign(pcc->div);
			pcc->div = DIV_ALIGN_CENTER;
		}
		break;
	    case M_DIV:
		if (mark->is_end) {
			if (!pcc->nobr) {
				ConditionalLineFeed(hw, 1, pcc);
			} else {
				pcc->have_space_after = 1;
			}
			pcc->div = PopAlign();
			pcc->in_div_hidden = False;
			pcc->noformat = False;
		} else {
			if (!pcc->nobr) {
				ConditionalLineFeed(hw, 1, pcc);
			} else {
				pcc->have_space_after = 1;
			}
			PushAlign(pcc->div);
			if (tptr = ParseMarkTag(mark->start, MT_DIV, "ALIGN")) {
				if (caseless_equal(tptr, "LEFT")) {
					pcc->div = DIV_ALIGN_LEFT;
				} else if (caseless_equal(tptr, "CENTER")) {
					pcc->div = DIV_ALIGN_CENTER;
				} else if (caseless_equal(tptr, "RIGHT")) {
					pcc->div = DIV_ALIGN_RIGHT;
				}
				free(tptr);
			}
			if (tptr = ParseMarkTag(mark->start, MT_DIV, "STYLE")) {
			    char *sptr = tptr;

			    while (*sptr) {
				if (!my_strncasecmp(sptr, "visibility:", 11)) {
				    char *cptr = sptr + 11;

				    while (ISSPACE((int)*cptr))
					cptr++;
				    if (!my_strncasecmp(cptr, "hidden", 6)) {
					pcc->in_div_hidden = True;
					pcc->noformat = True;
					break;
				    }
				}
				sptr++;
			    }
			    free(tptr);
			}
		}
		break;
	    case M_SPAN:
		if (mark->is_end) {
			if (pcc->span_title) {
				free(pcc->span_title);
				pcc->span_title = NULL;
			}
		} else {
			if (pcc->span_title)
				free(pcc->span_title);
			if (pcc->span_title = ParseMarkTag(mark->start, MT_SPAN,
							   "TITLE")) {
				if (!*pcc->span_title) {
					free(pcc->span_title);
					pcc->span_title = NULL;
				}
			}
		}
		break;
	    /*
	     * Just insert a linefeed, or ignore if this is preformatted
	     * text because the <P> will be followed by a linefeed.
	     * See above for additional end of paragraph processing.
	     */
	    case M_PARAGRAPH:
		ConditionalLineFeed(hw, 1, pcc);
		/* Don't force linefeed at top of page or table cell */
		if (!pcc->at_top)
			ConditionalLineFeed(hw, 2, pcc);
		pcc->at_top = False;
		if (mark->is_end)
			break;
		PushAlign(pcc->div);
		pcc->in_paragraph = 1;
		pcc->para_y = pcc->y;
		if (tptr = ParseMarkTag(mark->start, MT_PARAGRAPH, "ALIGN")) {
			if (caseless_equal(tptr, "LEFT")) {
				pcc->div = DIV_ALIGN_LEFT;
			} else if (caseless_equal(tptr, "CENTER")) {
				pcc->div = DIV_ALIGN_CENTER;
			} else if (caseless_equal(tptr, "RIGHT")) {
				pcc->div = DIV_ALIGN_RIGHT;
			}
			free(tptr);
		}
		break;

	    case M_HRULE:
		ConditionalLineFeed(hw, 1, pcc);
		HRulePlace(hw, *mptr, pcc);
		ConditionalLineFeed(hw, 1, pcc);
		break;
	    /*
	     * Titles are just set into the widget for retrieval by
	     * XtGetValues().
	     */
	    case M_TITLE:
		if (mark->is_end) {
		        if (InDocHead != 1)
				pcc->ignore = 0;
			if (!hw->html.title)
				hw->html.title = TitleText;
		        TitleText = NULL;
			XtCallCallbackList((Widget)hw, hw->html.title_callback,
				           hw->html.title);
		} else {
			pcc->ignore = 1;
			TitleText = NULL;
		}
		break;
	    /*
	     * Formatting commands just change the current font.
	     */
	    case M_CODE:
	    case M_SAMPLE:
	    case M_KEYBOARD:
		if (mark->is_end) {
			pcc->cur_font = PopFont(hw, pcc);
		} else {
			PushFont(hw, pcc);
			pcc->cur_font = hw->html.fixed_font;
			pcc->cur_font_type = FIXED_FONT;
		}
		break;
	    case M_FIXED:
		if (mark->is_end) {
			pcc->cur_font = PopFont(hw, pcc);
		} else {
			PushFont(hw, pcc);
			if (pcc->cur_font_type == ITALIC_FONT) {
				pcc->cur_font = hw->html.fixeditalic_font;
				pcc->cur_font_type = FIXEDITALIC_FONT;
			} else if (pcc->cur_font_type == BOLD_FONT) {
				pcc->cur_font = hw->html.fixedbold_font;
				pcc->cur_font_type = FIXEDBOLD_FONT;
			} else {
				pcc->cur_font = hw->html.fixed_font;
				pcc->cur_font_type = FIXED_FONT;
			}
		}
		break;
	    case M_STRONG:
	    case M_BOLD:
		if (mark->is_end) {
			pcc->cur_font = PopFont(hw, pcc);
		} else {
			PushFont(hw, pcc);
			if ((pcc->cur_font_type == FIXED_FONT) ||
			    (pcc->cur_font_type == FIXEDITALIC_FONT)) {
				pcc->cur_font = hw->html.fixedbold_font;
				pcc->cur_font_type = FIXEDBOLD_FONT;
			} else if ((pcc->cur_font_type == PLAIN_FONT) ||
				   (pcc->cur_font_type == PLAINITALIC_FONT)) {
				pcc->cur_font = hw->html.plainbold_font;
				pcc->cur_font_type = PLAINBOLD_FONT;
			} else if ((pcc->cur_font_type == ITALIC_FONT) ||
				   (pcc->cur_font_type == ADDRESS_FONT)) {
				pcc->cur_font = hw->html.bolditalic_font;
				pcc->cur_font_type = BOLDITALIC_FONT;
			} else {
				pcc->cur_font = hw->html.bold_font;
				pcc->cur_font_type = BOLD_FONT;
			}
		}
		break;
	    case M_EMPHASIZED:
	    case M_VARIABLE:
	    case M_CITATION:
	    case M_ITALIC:
	    case M_DEFINE:
		if (mark->is_end) {
			int size = pcc->cur_font_size;

			pcc->cur_font = PopFont(hw, pcc);
			/* Reset size if we changed it while in a header */
			if (size != pcc->cur_font_size)
				SetFontSize(hw, pcc, 0);
		} else {
			PushFont(hw, pcc);
			if ((pcc->cur_font_type == FIXED_FONT) ||
			    (pcc->cur_font_type == FIXEDBOLD_FONT)) {
				pcc->cur_font = hw->html.fixeditalic_font;
				pcc->cur_font_type = FIXEDITALIC_FONT;
			} else if ((pcc->cur_font_type == PLAIN_FONT) ||
				   (pcc->cur_font_type == PLAINBOLD_FONT)) {
				pcc->cur_font = hw->html.plainitalic_font;
				pcc->cur_font_type = PLAINITALIC_FONT;
			} else if (pcc->cur_font_type == BOLD_FONT) {
				pcc->cur_font = hw->html.bolditalic_font;
				pcc->cur_font_type = BOLDITALIC_FONT;
			} else if ((pcc->cur_font_type >= HEADER1_FONT) &&
				   (pcc->cur_font_type <= HEADER6_FONT)) {
				int adjust = 0;

				/* This is a major hack, but no other solution
				 * except a complete header font rework */
				switch (pcc->cur_font_size) {
				    case 1:
					adjust = 1;
					break;
				    case 6:
				    case 7:
					adjust = -1;
				}
				switch (pcc->cur_font_type) {
				    case HEADER1_FONT:
					if ((pcc->cur_font_size == 2) ||
					    (pcc->cur_font_size == 3)) {
						adjust += 3;
					} else if ((pcc->cur_font_size == 1) ||
					         (pcc->cur_font_size == 6)) {
						adjust += 1;
					} else {
						adjust += 2;
					}
					break;
				    case HEADER2_FONT:
					if ((pcc->cur_font_size == 2) ||
					    (pcc->cur_font_size == 3)) {
						adjust += 2;
					} else {
						adjust += 1;
					}
					break;
				    case HEADER3_FONT:
					break;
				    case HEADER4_FONT:
					if ((pcc->cur_font_size == 3) ||
					    (pcc->cur_font_size == 4) ||
					    (pcc->cur_font_size == 5))
						adjust -= 1;
					break;
				    case HEADER5_FONT:
					if ((pcc->cur_font_size == 4) ||
					    (pcc->cur_font_size == 5)) {
						adjust -= 2;
					} else {
						adjust -= 1;
					}
					break;
				    case HEADER6_FONT:
					if (pcc->cur_font_size == 5) {
						adjust -= 3;
					} else {
						adjust -= 2;
					}
				}
				pcc->cur_font_size += adjust;
				if (pcc->cur_font_size  > 7) {
					pcc->cur_font_size = 7;
				} else if (pcc->cur_font_size < 1) {
					pcc->cur_font_size = 1;
				}
				SetFontSize(hw, pcc, 0);
				pcc->cur_font = hw->html.bolditalic_font;
				pcc->cur_font_type = BOLDITALIC_FONT;
			} else {
				pcc->cur_font = hw->html.italic_font;
				pcc->cur_font_type = ITALIC_FONT;
			}
		}
		break;
	    /*
	     * Strikeout means draw a line through the text.
	     * Right now we just set a boolean flag which gets shoved in
	     * the element record for all elements in the strikeout zone.
	     */
	    case M_STRIKEOUT:
		if (mark->is_end) {
			pcc->Strikeout = False;
		} else {
			pcc->Strikeout = True;
			pcc->strikeout_start = True;
		}
		break;
            case M_SUP:
      		if (mark->is_end && pcc->superscript) {
               		pcc->superscript--;
       			pcc->cur_font = PopFont(hw, pcc);
			SetFontSize(hw, pcc, 0);
         	} else if (!mark->is_end) {
             		pcc->superscript++;
			pcc->sub_or_sup = 2;
       			nonScriptFont = pcc->cur_font;
       			PushFont(hw, pcc);
			if (pcc->cur_font_size > 1) {
				pcc->cur_font_size--;
				SetFontSize(hw, pcc, 0);
			}
                }
                break;
	    case M_SUB:
		if (mark->is_end && pcc->subscript) {
                        pcc->subscript--;
                        pcc->cur_font = PopFont(hw, pcc);
			SetFontSize(hw, pcc, 0);
                } else if (!mark->is_end) {
                        pcc->subscript++;
			pcc->sub_or_sup = 1;
                       	nonScriptFont = pcc->cur_font;
                       	PushFont(hw, pcc);
			if (pcc->cur_font_size > 1) {
				pcc->cur_font_size--;
				SetFontSize(hw, pcc, 0);
			}
                }
                break;
	    /* Ignore text inside a HEAD element */
	    case M_DOC_HEAD:
		if (mark->is_end && (InDocHead == 1)) {
		        InDocHead = -1;
			pcc->ignore = 0;
			/* </HEAD> terminates style sheets, etc. in the Head */
			if (pcc->noformat && !pcc->in_div_hidden)
				pcc->noformat = False;
			/* Finish processing the title; too late for </title> */
			if (TitleText) {
				hw->html.title = TitleText;
				TitleText = NULL;
				XtCallCallbackList((Widget)hw,
						   hw->html.title_callback,
						   hw->html.title);
			}
		} else if (!mark->is_end && !InDocHead) {
			InDocHead = 1;
			pcc->ignore = 1;
		}
		break;
	    case M_DOC_BODY:
		if (mark->is_end)
			break;

		/* JavaScript redirect */ 
		if (!pcc->cw_only &&
		    (tptr = ParseMarkTag(mark->start, MT_DOC_BODY, "onload"))) {
			int len = strlen(tptr);

			if ((len > 21) && 
			    !my_strncasecmp(tptr, "document.location='", 19)) {
			    XtAppContext app_con;
			    RefreshInfo *rinfo;
			    char *url;

			    /* Ends with ' or '; */
			    if (tptr[len - 1] == '\'')
				tptr[len - 1] = '\0';
			    if ((tptr[len - 1] == ';') &&
				(tptr[len - 2] == '\''))
				tptr[len - 2] = '\0';

			    url = strdup(tptr + 19);
#ifndef DISABLE_TRACE
			    if (htmlwTrace || refreshTrace)
				fprintf(stderr, "Body redirect URL = %s\n",url);
#endif
			    rinfo = (RefreshInfo *)malloc(sizeof(RefreshInfo));
			    rinfo->refresh = hw->html.refresh_count;
			    rinfo->url = url;
			    rinfo->hw = hw;
			    app_con = XtWidgetToApplicationContext((Widget)hw);
			    hw->html.refreshdata = rinfo;
			    hw->html.refresh_timer = XtAppAddTimeOut(app_con,
						1000,
						(XtTimerCallbackProc)RefreshURL,
						(XtPointer)rinfo);
			}
			free(tptr);
		}

		/* Allow additional body tags if nothing formatted yet */
		if (!InDocBody || !hw->html.formatted_elements) {
			static char *atts[] = { "text", "bgcolor", "alink",
						"vlink", "link", NULL };
			int has_bg = 0;

			InDocBody = 1;
			if (hw->html.body_colors) {
				int i;

				for (i = 0; atts[i]; i++) {
				    tptr = ParseMarkTag(mark->start,
						        MT_DOC_BODY, atts[i]);
				    if (tptr) {
					hw_do_color(hw, atts[i], tptr, pcc);
					free(tptr);
					if (i == 1)
					    has_bg = 1;
				    }
				}
			}
			if (hw->html.body_images) {
				tptr = ParseMarkTag(mark->start,
						    MT_DOC_BODY, "background");
				if (tptr) {
					hw_do_bg(hw, tptr, pcc);
					free(tptr);
					has_bg = 1;
				}
			}
			if (tptr = ParseMarkTag(mark->start, MT_DOC_BODY,
						"marginwidth")) {
				pcc->cur_line_width += pcc->left_margin +
						       pcc->right_margin;
				pcc->left_margin = atoi(tptr);
				if (pcc->left_margin > 0) {
					pcc->cur_line_width -= 2 *
							       pcc->left_margin;
					pcc->right_margin = pcc->left_margin;
				} else {
					pcc->right_margin = pcc->left_margin =0;
				}
				if (!hw->html.formatted_elements)
					pcc->x = pcc->left_margin;
				free(tptr);
			}
			if (tptr = ParseMarkTag(mark->start, MT_DOC_BODY,
						"leftmargin")) {
				pcc->cur_line_width += pcc->left_margin;
				pcc->left_margin = atoi(tptr);
				if (pcc->left_margin > 0) {
					pcc->cur_line_width -= pcc->left_margin;
				} else {
					pcc->left_margin = 0;
				}
				if (!hw->html.formatted_elements)
					pcc->x = pcc->left_margin;
				free(tptr);
			}
			if (tptr = ParseMarkTag(mark->start, MT_DOC_BODY,
						"rightmargin")) {
				pcc->cur_line_width += pcc->right_margin;
				pcc->right_margin = atoi(tptr);
				if (pcc->right_margin > 0) {
					pcc->cur_line_width -=
							      pcc->right_margin;
				} else {
					pcc->right_margin = 0;
				}
				free(tptr);
			}
			if (tptr = ParseMarkTag(mark->start, MT_DOC_BODY,
						"marginheight")) {
				pcc->margin_height = atoi(tptr);
				if (pcc->margin_height >= 0) {
					if (!hw->html.formatted_elements)
						pcc->y = pcc->margin_height;
				} else {
					pcc->margin_height = 0;
				}
				free(tptr);
			}
			if (tptr = ParseMarkTag(mark->start, MT_DOC_BODY,
						"topmargin")) {
				int tmp = atoi(tptr);

				if ((tmp >= 0) && !hw->html.formatted_elements)
					pcc->y = tmp;
				free(tptr);
			}
			/* Do background right away if progressive display */
			if (!pcc->cw_only && has_bg &&
			    progressiveDisplayEnabled) {
			    ElemInfo *eptr;

			    /* Need dummy element if none created yet */
			    if (!hw->html.formatted_elements) {
				eptr = CreateElement(hw, E_CR, pcc->cur_font,
						     0, 0, 0, 0, 0, pcc);
			    } else {
				eptr = hw->html.last_formatted_elem;
			    }
			    if (pcc->last_progressive_ele) {
				if (pcc->last_progressive_ele->next)
				    eptr = pcc->last_progressive_ele->next;
				ProgressiveDisplay(hw, eptr, pcc);
			    } else {
				ProgressiveDisplay(hw,
						   hw->html.formatted_elements,
						   pcc);
			    }
	     		    pcc->last_progressive_ele =
						   hw->html.last_formatted_elem;
			}
		        InDocHead = -1;   /* End <head> section */
			pcc->ignore = 0;
		}
		break;
	    case M_UNDERLINED:
		pcc->underline_start = 1;
		if (mark->is_end) {
			pcc->underline_number = 0;
			pcc->in_underlined = 0;
		} else {
			pcc->underline_number = 1;
			pcc->in_underlined = 1;
		}
		break;
	    /*
	     * Headers are preceeded (except after list bullets) and
	     * followed by a linefeed.
	     */
	    case M_HEADER_1:
		ConditionalLineFeed(hw, 1, pcc);
		if (mark->is_end && header1) {
			header1--;
			pcc->cur_font = PopFont(hw, pcc);
			ConditionalLineFeed(hw, 2, pcc);
			if (header1_align) {
				header1_align--;
				pcc->div = PopAlign();
			}
		} else if (!mark->is_end) {
			header1++;
			if (!pcc->at_top)
				ConditionalLineFeed(hw, 2, pcc);
			pcc->at_top = False;
			PushFont(hw, pcc);
			pcc->cur_font = hw->html.header1_font;
			pcc->cur_font_type = HEADER1_FONT;
			tptr = ParseMarkTag(mark->start, MT_HEADER_1, "ALIGN");
			if (tptr) {
				header1_align++;
				PushAlign(pcc->div);
				if (caseless_equal(tptr, "LEFT")) {
					pcc->div = DIV_ALIGN_LEFT;
				} else if (caseless_equal(tptr, "CENTER")) {
					pcc->div = DIV_ALIGN_CENTER;
				} else if (caseless_equal(tptr, "RIGHT")) {
					pcc->div = DIV_ALIGN_RIGHT;
				} else {
					header1_align--;
					pcc->div = PopAlign();
				}
				free(tptr);
			}
		}
		break;
	    case M_HEADER_2:
		ConditionalLineFeed(hw, 1, pcc);
		if (mark->is_end && header2) {
			header2--;
			pcc->cur_font = PopFont(hw, pcc);
			ConditionalLineFeed(hw, 2, pcc);
			if (header2_align) {
				header2_align--;
				pcc->div = PopAlign();
			}
		} else if (!mark->is_end) {
			header2++;
			if (!pcc->at_top)
				ConditionalLineFeed(hw, 2, pcc);
			pcc->at_top = False;
			PushFont(hw, pcc);
			pcc->cur_font = hw->html.header2_font;
			pcc->cur_font_type = HEADER2_FONT;
			tptr = ParseMarkTag(mark->start, MT_HEADER_2, "ALIGN");
			if (tptr) {
				header2_align++;
				PushAlign(pcc->div);
				if (caseless_equal(tptr, "LEFT")) {
					pcc->div = DIV_ALIGN_LEFT;
				} else if (caseless_equal(tptr, "CENTER")) {
					pcc->div = DIV_ALIGN_CENTER;
				} else if (caseless_equal(tptr, "RIGHT")) {
					pcc->div = DIV_ALIGN_RIGHT;
				} else {
					header2_align--;
					pcc->div = PopAlign();
				}
				free(tptr);
			}
		}
		break;
	    case M_HEADER_3:
		ConditionalLineFeed(hw, 1, pcc);
		if (mark->is_end && header3) {
			header3--;
			pcc->cur_font = PopFont(hw, pcc);
			ConditionalLineFeed(hw, 2, pcc);
			if (header3_align) {
				header3_align--;
				pcc->div = PopAlign();
			}
		} else if (!mark->is_end) {
			header3++;
			if (!pcc->at_top)
				ConditionalLineFeed(hw, 2, pcc);
			pcc->at_top = False;
			PushFont(hw, pcc);
			pcc->cur_font = hw->html.header3_font;
			pcc->cur_font_type = HEADER3_FONT;
			tptr = ParseMarkTag(mark->start, MT_HEADER_3, "ALIGN");
			if (tptr) {
				header3_align++;
				PushAlign(pcc->div);
				if (caseless_equal(tptr, "LEFT")) {
					pcc->div = DIV_ALIGN_LEFT;
				} else if (caseless_equal(tptr, "CENTER")) {
					pcc->div = DIV_ALIGN_CENTER;
				} else if (caseless_equal(tptr, "RIGHT")) {
					pcc->div = DIV_ALIGN_RIGHT;
				} else {
					header3_align--;
					pcc->div = PopAlign();
				}
				free(tptr);
			}
		}
		break;
	    case M_HEADER_4:
		ConditionalLineFeed(hw, 1, pcc);
		if (mark->is_end && header4) {
			header4--;
			pcc->cur_font = PopFont(hw, pcc);
			ConditionalLineFeed(hw, 2, pcc);
			if (header4_align) {
				header4_align--;
				pcc->div = PopAlign();
			}
		} else if (!mark->is_end) {
			header4++;
			if (!pcc->at_top)
				ConditionalLineFeed(hw, 2, pcc);
			pcc->at_top = False;
			PushFont(hw, pcc);
			pcc->cur_font = hw->html.header4_font;
			pcc->cur_font_type = HEADER4_FONT;
			tptr = ParseMarkTag(mark->start, MT_HEADER_4, "ALIGN");
			if (tptr) {
				header4_align++;
				PushAlign(pcc->div);
				if (caseless_equal(tptr, "LEFT")) {
					pcc->div = DIV_ALIGN_LEFT;
				} else if (caseless_equal(tptr, "CENTER")) {
					pcc->div = DIV_ALIGN_CENTER;
				} else if (caseless_equal(tptr, "RIGHT")) {
					pcc->div = DIV_ALIGN_RIGHT;
				} else {
					header4_align--;
					pcc->div = PopAlign();
				}
				free(tptr);
			}
		}
		break;
	    case M_HEADER_5:
		ConditionalLineFeed(hw, 1, pcc);
		if (mark->is_end && header5) {
			header5--;
			pcc->cur_font = PopFont(hw, pcc);
			ConditionalLineFeed(hw, 2, pcc);
			if (header5_align) {
				header5_align--;
				pcc->div = PopAlign();
			}
		} else if (!mark->is_end) {
			header5++;
			if (!pcc->at_top)
				ConditionalLineFeed(hw, 2, pcc);
			pcc->at_top = False;
			PushFont(hw, pcc);
			pcc->cur_font = hw->html.header5_font;
			pcc->cur_font_type = HEADER5_FONT;
			tptr = ParseMarkTag(mark->start, MT_HEADER_5, "ALIGN");
			if (tptr) {
				header5_align++;
				PushAlign(pcc->div);
				if (caseless_equal(tptr, "LEFT")) {
					pcc->div = DIV_ALIGN_LEFT;
				} else if (caseless_equal(tptr, "CENTER")) {
					pcc->div = DIV_ALIGN_CENTER;
				} else if (caseless_equal(tptr, "RIGHT")) {
					pcc->div = DIV_ALIGN_RIGHT;
				} else {
					header5_align--;
					pcc->div = PopAlign();
				}
				free(tptr);
			}
		}
		break;
	    case M_HEADER_6:
		ConditionalLineFeed(hw, 1, pcc);
		if (mark->is_end && header6) {
			header6--;
			pcc->cur_font = PopFont(hw, pcc);
			ConditionalLineFeed(hw, 2, pcc);
			if (header6_align) {
				header6_align--;
				pcc->div = PopAlign();
			}
		} else if (!mark->is_end) {
			header6++;
			if (!pcc->at_top)
				ConditionalLineFeed(hw, 2, pcc);
			pcc->at_top = False;
			PushFont(hw, pcc);
			pcc->cur_font = hw->html.header6_font;
			pcc->cur_font_type = HEADER6_FONT;
			tptr = ParseMarkTag(mark->start, MT_HEADER_6, "ALIGN");
			if (tptr) {
				header6_align++;
				PushAlign(pcc->div);
				if (caseless_equal(tptr, "LEFT")) {
					pcc->div = DIV_ALIGN_LEFT;
				} else if (caseless_equal(tptr, "CENTER")) {
					pcc->div = DIV_ALIGN_CENTER;
				} else if (caseless_equal(tptr, "RIGHT")) {
					pcc->div = DIV_ALIGN_RIGHT;
				} else {
					header6_align--;
					pcc->div = PopAlign();
				}
				free(tptr);
			}
		}
		break;
	    case M_FRAMESET:
		if (hw->html.frame_support) {
			HTMLCreateFrameSet(hw, mptr, pcc);
		} else {
			FramePlace(hw, *mptr, pcc);
		}
		break;
	    case M_FRAME:
	    case M_NOFRAMES:
		/* Will be handled by FRAMESET processing */
		if (!hw->html.frame_support)
			FramePlace(hw, *mptr, pcc);
		break;
	    /*
	     * Anchors change the text color, and may set
	     * underlining attributes.
	     * No linefeeds, so they can be imbedded anywhere.
	     */
	    case M_ANCHOR:
		if (pcc->cw_only) {
			if (mark->is_end) {
				pcc->in_anchor = 0;
			} else {
				pcc->in_anchor = 1;
			}
			break;
		}
		if (mark->is_end || pcc->in_anchor) {
			/* At end of anchor or at start of another without
			 * finding end of previous one */
			pcc->fg = pcc->cur_font_color;
			pcc->underline_number = pcc->in_underlined;
			pcc->dashed_underlines = False;
			if (mark->is_end) {
				pcc->anchor_tag_ptr = NULL_ANCHOR_PTR;
				pcc->in_anchor = 0;
				break;
			}
		}
		pcc->in_anchor = 1;
		pcc->fixed_anchor_color = 0;
		/* Only change the color of anchors with HREF tags, 
		 * because other anchors are not active.
		 */
		pcc->anchor_tag_ptr = *mptr;
		tptr = ParseMarkTag(mark->start, MT_ANCHOR, AT_HREF);
		if (tptr) { 
			mark->anc_href = GetMarkText(tptr);
			pcc->anchor_start = 1;
		        if (hw->html.previously_visited_test &&
			    ((*(visitTestProc)
			     (hw->html.previously_visited_test))
			     ((Widget)hw, tptr))) {
			        pcc->fg = hw->html.visitedAnchor_fg;
			        pcc->underline_number =
					  hw->html.num_visitedAnchor_underlines;
			        pcc->dashed_underlines =
					    hw->html.dashed_visitedAnchor_lines;
			} else {
				pcc->fg = hw->html.anchor_fg;
				pcc->underline_number =
					         hw->html.num_anchor_underlines;
				pcc->dashed_underlines =
						   hw->html.dashed_anchor_lines;
			}
			free(tptr);
		}
		if (pcc->in_underlined) {
			pcc->dashed_underlines = False;
			if (!pcc->underline_number)
				pcc->underline_number = 1;
		}
		/* May have already found ID */
		if (!mark->anc_name) {
			mark->anc_name = ParseMarkTag(mark->start, MT_ANCHOR,
						      AT_NAME);
			if (mark->anc_name)
				CreateAnchorElement(hw, mark, pcc);
		}
		mark->anc_title = ParseMarkTag(mark->start,
					       MT_ANCHOR, AT_TITLE);
		mark->anc_target = ParseMarkTag(mark->start,
						MT_ANCHOR, "target");
		if (pcc->basetarget && !mark->anc_target)
			mark->anc_target = strdup(pcc->basetarget);
		break;

	    /* Blockquotes increase the margin width.  They cannot be nested. */
	    case M_BLOCKQUOTE:
		if (mark->is_end && pcc->blockquote) {
			pcc->left_margin -= D_INDENT_SPACES;
			pcc->right_margin -= D_INDENT_SPACES;
			pcc->cur_line_width += D_INDENT_SPACES * 2;
			ConditionalLineFeed(hw, 1, pcc);
			pcc->x = pcc->left_margin + pcc->eoffsetx;
			ConditionalLineFeed(hw, 2, pcc);
			pcc->blockquote--;
		} else {
			ConditionalLineFeed(hw, 1, pcc);
			if ((D_INDENT_SPACES * 2) < pcc->cur_line_width) {
				pcc->left_margin += D_INDENT_SPACES;
				pcc->right_margin += D_INDENT_SPACES;
				pcc->cur_line_width -= D_INDENT_SPACES * 2;
				pcc->blockquote++;
			}
			ConditionalLineFeed(hw, 2, pcc);
			pcc->x = pcc->left_margin + pcc->eoffsetx;
		}
		break;

	    /* Can only be inside a SELECT tag. */
	    case M_OPTION:
		if (mark->is_end)
			return;
		FormSelectOptionField(mptr, pcc);
		break;

	    case M_OPTGROUP:
		if (mark->is_end)
			return;
		FormSelectOptgroup(mptr, pcc);
		break;

	    /* Special INPUT tag. */
	    case M_SELECT:
		if (mark->is_end) {
			FormSelectEnd(hw, pcc);
		} else {
			FormSelectBegin(hw, mptr, pcc);
		}
		break;

	    case M_TEXTAREA:
		if (mark->is_end) {
			FormTextAreaEnd(hw, mptr, pcc);
		} else {
			FormTextAreaBegin(mptr, pcc);
		}
		break;

	    case M_BUTTON:
		if (mark->is_end) {
			FormButtonEnd(hw, mptr, pcc);
		} else {
			FormButtonBegin(hw, mptr, pcc);
		}
		break;

	    /* Just insert the widget. */
	    case M_INPUT:
		if (mark->is_end)	/* No end mark on <input> */
			return;
		FormInputField(hw, mptr, pcc);
		break;

	    case M_LABEL:
		if (pcc->label_id) {
			free(pcc->label_id);
			pcc->label_id = NULL;
		}
		if (mark->is_end) {
			pcc->in_label = 0;
		} else {
			if (tptr = ParseMarkTag(mark->start, MT_LABEL, "for")) {
				pcc->label_id = tptr;
			} else {
				/* An implicit label */
				pcc->in_label = 1;
				pcc->label_id = strdup(tmpnam(NULL));
			}
		}
		break;

	    /* Fillout forms.  Cannot be nested. */
	    case M_FORM:
		/* Don't linefeed if in non-display HTML in table */
		if (pcc->in_table > -1) {
			ConditionalLineFeed(hw, 1, pcc);
		} else {
			/* Indicate LF needed at start of table */
			pcc->in_table = -2;
		}
		if (mark->is_end) {
			if (pcc->in_table > -1)
				ConditionalLineFeed(hw, 2, pcc);
			EndForm(hw, pcc);
		} else {
			if (pcc->in_table > -1)
				ConditionalLineFeed(hw, 2, pcc);
			BeginForm(hw, mptr, pcc);
		}
		break;

	    /*
	     * Addresses are just like headers.  A linefeed before and
	     * after, and change the font.
	     */
	    case M_ADDRESS:
		ConditionalLineFeed(hw, 1, pcc);
		if (mark->is_end) {
			pcc->cur_font = PopFont(hw, pcc);
		} else {
			PushFont(hw, pcc);
			pcc->cur_font = hw->html.address_font;
			pcc->cur_font_type = ADDRESS_FONT;
		}
		break;
	    /*
	     * Plain and listing text.  A single pre-formatted chunk of
	     * text in its own font.
	     */
	    case M_PREFORMAT:
	    case M_PLAIN_TEXT:
	    case M_LISTING_TEXT:
	    case M_PLAIN_FILE:
		if (mark->is_end) {
			/*
			 * Properly convert the Linefeed state variable
			 * from preformat to formatted state.
			 */
			if (pcc->pf_lf_state == 2) {
				pcc->pf_lf_state = 1;
			} else {
				pcc->pf_lf_state = 0;
			}
			pcc->preformat = 0;
			ConditionalLineFeed(hw, 1, pcc);
			pcc->cur_font = PopFont(hw, pcc);
			ConditionalLineFeed(hw, 2, pcc);
		} else {
			ConditionalLineFeed(hw, 1, pcc);
			ConditionalLineFeed(hw, 2, pcc);
			pcc->preformat = 1;
			pcc->pf_lf_state = 2;
			PushFont(hw, pcc);
			pcc->cur_font = hw->html.plain_font;
			pcc->cur_font_type = PLAIN_FONT;
			if (type == M_LISTING_TEXT) {
				pcc->cur_font = hw->html.listing_font;
				pcc->cur_font_type = LISTING_FONT;
			}
		}
		break;
	    /*
	     * Numbered lists, Unnumbered lists and Menus.
	     * Currently also lump directory listings into this.
	     * Save state for each indent level.
	     * Change the value of the TxtIndent (can be nested).
	     * Linefeed at the end of the list.
	     */
	    case M_NUM_LIST:
	    case M_UNUM_LIST:
	    case M_MENU:
	    case M_DIRECTORY:
		ConditionalLineFeed(hw, 1, pcc);
		if (mark->is_end) {
			/* Restore the old state if there is one */
			if (DescType->next) {
				DescRec *dptr = DescType;

				DescType = DescType->next;
				pcc->left_margin -= dptr->indent;
				pcc->cur_line_width += dptr->indent;
				pcc->indent_level = dptr->save_indent_level;
				if (!pcc->at_top && !DescType->next)
					ConditionalLineFeed(hw, 2, pcc);
				free((char *)dptr);
			}
		} else {
			DescRec *dptr = (DescRec *)malloc(sizeof(DescRec));

			/*
			 * If this is the outermost level of indentation
			 * and we are not at top of page or table cell, then
			 * add another linefeed for more white space.
			 */
			if (!pcc->at_top && !DescType->next)
				ConditionalLineFeed(hw, 2, pcc);
			/* Save the old state, and start a new */
			dptr->compact = 0;
			dptr->save_indent_level = pcc->indent_level++;
			if (type == M_NUM_LIST) {
				dptr->type = D_OLIST;
				dptr->count = 1;
				dptr->style = '1';
				if (tptr = ParseMarkTag(mark->start,
				    			MT_NUM_LIST, "TYPE")) {
					if (*tptr)
						dptr->style = tptr[0];
					free(tptr);
				}
				if (tptr = ParseMarkTag(mark->start,
				    			MT_NUM_LIST, "START")) {
					if (*tptr)
						dptr->count = atoi(tptr);
					if (dptr->count < 1)
						dptr->count = 1;
					free(tptr);
				}
			} else if (type == M_UNUM_LIST) {
				dptr->type = D_ULIST;
				dptr->count = 0;
				if (tptr = ParseMarkTag(mark->start,
				    			MT_UNUM_LIST, "TYPE")) {
				    if (!my_strcasecmp(tptr, "disc")) {
					pcc->indent_level = 1;
				    } else if (!my_strcasecmp(tptr, "square")) {
					pcc->indent_level = 4;
				    } else if (!my_strcasecmp(tptr, "circle")) {
					pcc->indent_level = 3;
				    } else if (!my_strcasecmp(tptr, "block")) {
					pcc->indent_level = 2;
				    }
				    free(tptr);
				}
			} else {
				dptr->type = D_ULIST;
				dptr->count = 0;
			}
			if ((pcc->cur_line_width - D_INDENT_SPACES) <=
			    D_INDENT_SPACES) {
				dptr->indent = 0;
			} else {
				dptr->indent = D_INDENT_SPACES;
			}
			pcc->left_margin += dptr->indent;
			pcc->cur_line_width -= dptr->indent;
			dptr->next = DescType;
			DescType = dptr;
			/* In case people mix and match */
			DescType->in_title = 0;
		}
		pcc->at_top = False;
		pcc->x = pcc->eoffsetx + pcc->left_margin;
		pcc->is_bol = 1;
		break;
	    /* Place bullet or number element at the beginning of item */
	    case M_LIST_ITEM:
		if (!mark->is_end) {
			ConditionalLineFeed(hw, 1, pcc);
			/* If no type, than no current list, so do a hack */
			if (DescType->type == D_NONE) {
				pcc->x = pcc->eoffsetx + pcc->left_margin +
					 pcc->cur_font->max_bounds.width;
			} else {
				pcc->x = pcc->eoffsetx + pcc->left_margin;
			}
			pcc->is_bol = 1;
			/* Ordered lists have numbers instead of bullets. */
			if (DescType->type == D_OLIST) {
				if (tptr = ParseMarkTag(mark->start,
				    			MT_LIST_ITEM, "VALUE")){
					if (*tptr)
						DescType->count = atoi(tptr);
					if (DescType->count < 1)
						DescType->count = 1;
					free(tptr);
				}
				ListNumberPlace(hw, pcc, DescType->count,
						DescType->style);
				DescType->count++;
			} else {
				if (tptr = ParseMarkTag(mark->start,
							MT_LIST_ITEM, "TYPE")) {
				    if (!my_strcasecmp(tptr, "disc")) {
					pcc->indent_level = 1;
				    } else if (!my_strcasecmp(tptr, "square")) {
					pcc->indent_level = 4;
				    } else if (!my_strcasecmp(tptr, "circle")) {
					pcc->indent_level = 3;
				    } else if (!my_strcasecmp(tptr, "block")) {
					pcc->indent_level = 2;
				    }
				    free(tptr);
				}
				BulletPlace(hw, pcc, 1);
			}	
		}
		break;
	    /* Description lists */
	    case M_DESC_LIST:
		if (mark->is_end) {
			/* Restore the old state if there is one */
			if (DescType->next) {
				DescRec *dptr = DescType;

				DescType = DescType->next;
				if (!dptr->in_title) {
					pcc->left_margin -= dptr->indent;
					pcc->cur_line_width += dptr->indent;
				}
				pcc->indent_level = dptr->save_indent_level;
				ConditionalLineFeed(hw, 1, pcc);
				ConditionalLineFeed(hw, 2, pcc);
				free((char *)dptr);
			}
		} else {
			DescRec *dptr = (DescRec *)malloc(sizeof(DescRec));

			ConditionalLineFeed(hw, 1, pcc);
			ConditionalLineFeed(hw, 2, pcc);
			dptr->save_indent_level = pcc->indent_level++;
			/* Check if this is a compact list */
			tptr = ParseMarkTag(mark->start, MT_DESC_LIST,
					    "COMPACT");
			if (tptr) {
				dptr->compact = 1;
				free(tptr);
			} else {
				dptr->compact = 0;
			}
			if ((pcc->cur_line_width - D_INDENT_SPACES) <=
			    D_INDENT_SPACES) {
				dptr->indent = 0;
			} else {
				dptr->indent = D_INDENT_SPACES;
			}
			pcc->left_margin += dptr->indent;
			pcc->cur_line_width -= dptr->indent;
			/* Save the old state, and start a new */
			dptr->type = D_DESC_LIST_START;
			dptr->next = DescType;
			DescType = dptr;
			DescType->in_title = 0;
		}
		pcc->x = pcc->eoffsetx + pcc->left_margin;
		pcc->is_bol = 1;
		break;
	    case M_DESC_TITLE:
		if (mark->is_end)
			break;
		ConditionalLineFeed(hw, 1, pcc);
		if (!DescType->in_title) {
			pcc->left_margin -= DescType->indent;
			pcc->cur_line_width += DescType->indent;
			DescType->in_title = 1;
		}
		pcc->x = pcc->eoffsetx + pcc->left_margin;
		pcc->is_bol = 1;
		break;
	    case M_DESC_TEXT:
		if (mark->is_end)
			break;
		/* For a compact list we want to stay on the same
		 * line if there is room and we are the first line
		 * after a title.
		 */
		if (!DescType->compact) {
			ConditionalLineFeed(hw, 1, pcc);
			pcc->have_space_after = 0;
			/* If no type, than no current list, so do a hack */
			if (DescType->type == D_NONE) {
				pcc->x = pcc->eoffsetx + pcc->left_margin +
					 D_INDENT_SPACES;
			} else if (DescType->in_title) {
				pcc->left_margin += DescType->indent;
				pcc->cur_line_width -= DescType->indent;
				pcc->x = pcc->eoffsetx + pcc->left_margin;
				DescType->in_title = 0;
			}
			pcc->is_bol = 1;
		} else if (DescType->in_title) {
			pcc->left_margin += DescType->indent;
			pcc->cur_line_width -= DescType->indent;
			DescType->in_title = 0;
		}
		break;

	    /*
	     * Now with forms, <INDEX> is the same as:
	     * <FORM>
	     * <HR>
	     * This is a searchable index.  Enter search keywords:
	     * <INPUT NAME="isindex">
	     * <HR>
	     * </FORM>
	     * Also, <INDEX> will take an ACTION tag to specify a
	     * different URL to submit the query to.
	     */
	    case M_INDEX: {
		static MarkInfo mark_tmp = { M_INPUT, 0, NULL, NULL };

		if (pcc->cur_form)
			break;  /* No index inside a form */

		hw->html.is_index = True;
		/* Start the form */
		ConditionalLineFeed(hw, 1, pcc);
		ConditionalLineFeed(hw, 2, pcc);
		pcc->cur_form = (FormInfo *)malloc(sizeof(FormInfo));
		pcc->cur_form->next = NULL;
		pcc->cur_form->hw = (Widget)hw;
		pcc->cur_form->target = NULL;
		pcc->cur_form->action = ParseMarkTag(mark->start, MT_INDEX,
						     "ACTION");
		pcc->cur_form->method = ParseMarkTag(mark->start, MT_INDEX,
						     "METHOD");
		pcc->cur_form->enctype = ParseMarkTag(mark->start, MT_INDEX,
						      "ENCTYPE");
		pcc->cur_form->start = pcc->widget_id;
		pcc->cur_form->end = -1;
		pcc->cur_form->cw_only = pcc->cw_only;

		/* Horizontal rule */
		ConditionalLineFeed(hw, 1, pcc);
		HRulePlace(hw, *mptr ,pcc);
		ConditionalLineFeed(hw, 1, pcc);

		if (!mark_tmp.text) {
			/* Must be writeable strings for ParseMarkTag */
			mark_tmp.text = strdup(
		        "This is a searchable index.  Enter search keywords: ");
			mark_tmp.start = strdup(
					      "INPUT SIZE=25 NAME=\"isindex\"");
		}
		/* Display text. */
		PartOfTextPlace(hw, &mark_tmp, pcc);

		/* Fake up the text INPUT tag.  */
		{
		    /* Must keep CreateAnchorElement from touching dummy mark */
		    Boolean save = pcc->in_label;

		    pcc->in_label = 0;
		    WidgetPlace(hw, &mark_tmp, pcc);
		    pcc->in_label = save;
		}

		/*  Horizontal rule */
		ConditionalLineFeed(hw, 1, pcc);
		HRulePlace(hw, *mptr, pcc);
		ConditionalLineFeed(hw, 1, pcc);

		/* Close the form  */
		pcc->cur_form->end = pcc->widget_id;
		ConditionalLineFeed(hw, 2, pcc);
		AddNewForm(hw, pcc->cur_form);
		pcc->cur_form = NULL;
		break;
	    }
	    case M_LINEBREAK:
		/* Force a hard linefeed */
		if (!pcc->cur_line_height)
			pcc->cur_line_height = pcc->cur_font->ascent +
					       pcc->cur_font->descent;
		if (tptr = ParseMarkTag(mark->start, MT_LINEBREAK, "clear")) {
			if (!my_strcasecmp(tptr, "all")) {
				if (pcc->float_right)
					pcc->float_right->type = -1;
				if (pcc->float_left)
					pcc->float_left->type = -1;
			} else if (!my_strcasecmp(tptr, "right")) {
				if (pcc->float_right)
					pcc->float_right->type = -1;
			} else if (!my_strcasecmp(tptr, "left")) {
				if (pcc->float_left)
					pcc->float_left->type = -1;
			}
			free(tptr);
		}
		LinefeedPlace(hw, pcc);
		break;
	    case M_NOBR:
		if (mark->is_end) {
			pcc->nobr = 0;
		} else {
			pcc->nobr = 1;
			pcc->nobr_x = pcc->left_margin + pcc->eoffsetx;
		}
		break;
	    case M_BUGGY_TABLE:
		break;
	    case M_TABLE:
		if (mark->is_end)
			break;
		if (tableSupportEnabled) {
			int popsave = hw->html.font_save_count;
			FontRec *fptr;

			fptr = PushFont(hw, pcc);
			hw->html.font_save_count = hw->html.pushfont_count;
			pcc->cur_font = hw->html.font;
			pcc->cur_font_size = hw->html.font_base;
			pcc->cur_font_type = FONT;
			pcc->cur_font_family = hw->html.font_family;
			pcc->fg = pcc->cur_font_color = GetDefaultFontColor(hw);
			fptr->color_ch = 1;
			PushAlign(pcc->div);

			TablePlace(hw, mptr, pcc);

			pcc->div = PopAlign();
			PopFontSaved(hw, pcc);
			hw->html.font_save_count = popsave;
			pcc->cur_font = PopFont(hw, pcc);
		}
		break;
	    case M_FIGURE:
	    case M_IMAGE:
		if (mark->is_end)
			break;
		if (pcc->button_buf && !pcc->cw_only) {
			/* Image in <BUTTON> */
			char *tptr2;

			tptr = malloc(strlen(mark->start) +
				      strlen(" ISMAP=2 "));
			strcpy(tptr, mark->start);
			strcat(tptr, " ISMAP");
			tptr2 = ParseMarkTag(pcc->button_buf, MT_INPUT, "TYPE");
			if (tptr2) {
				if (!my_strcasecmp(tptr2, "reset"))
					strcat(tptr, "=2");
				free(tptr2);
			}				
			tptr2 = mark->start;
			mark->start = tptr;
			ImagePlace(hw, *mptr, pcc);
			mark->start = tptr2;
			free(tptr);
			pcc->button_has_image = True;
		} else {
			if (pcc->button_buf)
				pcc->button_has_image = True;
			ImagePlace(hw, *mptr, pcc);
		}
		break;
	    case M_APPLET:
		if (mark->is_end)
			break;
		if (appletSupportEnabled)
			AppletPlace(hw, mptr, pcc, 0);
		break;
	    case M_SMALL:
		if (mark->is_end) {
			pcc->cur_font = PopFont(hw, pcc);
			SetFontSize(hw, pcc, 0);
		} else {
			PushFont(hw, pcc);
			if (pcc->cur_font_size > 2) {
				pcc->cur_font_size = 2;
			} else {
				pcc->cur_font_size = 1;
			}
			SetFontSize(hw, pcc, 0);
		}
		break;
	    case M_BIG:
		if (mark->is_end) {
			pcc->cur_font = PopFont(hw, pcc);
			SetFontSize(hw, pcc, 0);
		} else {
			PushFont(hw, pcc);
			if (pcc->cur_font_size < 5) {
				pcc->cur_font_size = 5;
			} else if (pcc->cur_font_size == 5) {
				pcc->cur_font_size = 6;
			} else {
				pcc->cur_font_size = 7;
			}
			SetFontSize(hw, pcc, 0);
		}
		break;
	    case M_BLINK:
		if (mark->is_end) {
			pcc->blink = 0;
		} else {
			pcc->blink = 1;
		}
		break;
	    case M_FONT:
		if (mark->is_end) {
			pcc->cur_font = PopFont(hw, pcc);
			SetFontSize(hw, pcc, 0);
			pcc->fixed_anchor_color = 0;
		} else {
			FontRec *fptr = PushFont(hw, pcc);

			if (hw->html.font_colors) {
				tptr = ParseMarkTag(mark->start, MT_FONT,
						    "color");
				if (tptr) {
				    hw_do_color(hw, "color", tptr, pcc);
				    fptr->color_ch = 1;
				    free(tptr);
				    if (pcc->in_anchor)
					pcc->fixed_anchor_color = 1;
				}
			}
			if (hw->html.font_sizes) {
				tptr = ParseMarkTag(mark->start, MT_FONT,
						    "size");
				if (tptr) {
				    int size;

				    if (*tptr == '+') {
					size = atoi(tptr + 1);
					pcc->cur_font_size = size +
							     pcc->cur_font_base;
					if (pcc->cur_font_size > 7)
					    pcc->cur_font_size = 7;
				    } else if (*tptr == '-') {
					size = atoi(tptr + 1);
					pcc->cur_font_size =
						      pcc->cur_font_base - size;
					if (pcc->cur_font_size < 1)
					    pcc->cur_font_size = 1;
				    } else {
					size = atoi(tptr);
					if ((size > 0) && (size < 8)) {
					    pcc->cur_font_size = size;
					} else if (size > 7) {
					    pcc->cur_font_size = 7;
					} else if (size < 1) {
					    pcc->cur_font_size = 1;
					}
				    }
				    SetFontSize(hw, pcc, 0);
				    free(tptr);
				}
				tptr = ParseMarkTag(mark->start, MT_FONT,
						    "face");
				if (tptr) {
				    char *face = strtok(tptr, ", ");

				    pcc->cur_font_family = TIMES;
				    while (face) {
					if (!my_strncasecmp(face, "Times", 5) ||
					    !my_strncasecmp(face, "Book", 4) ||
					    !my_strncasecmp(face,"Georgia",7) ||
					    !my_strncasecmp(face, "Minion",6) ||
					    !my_strncasecmp(face, "Roman", 5)) {
					    pcc->cur_font_family = TIMES;
					    break;
					} else if (!my_strncasecmp(face,
						   "Lucida", 6) ||
						   !my_strncasecmp(face,
						   "Comic", 5) ||
						   !my_strncasecmp(face,
						   "Letter", 6) ||
						   !my_strncasecmp(face,
						   "Trebuchet", 9)) {
					    pcc->cur_font_family = LUCIDA;
					    break;
					} else if (!my_strncasecmp(face,
						   "Helvetica", 9) ||
						   !my_strncasecmp(face,
						   "Albertus", 8) ||
						   !my_strncasecmp(face,
						   "Modern", 6) ||
						   !my_strncasecmp(face,
						   "Tohoma", 6) ||
						   !my_strncasecmp(face,
						   "Univers", 7) ||
						   !my_strncasecmp(face,
						   "Verdana", 7) ||
						   !my_strncasecmp(face,
						   "Arial", 5)) {
					    pcc->cur_font_family = HELVETICA;
					    break;
					} else if (!my_strncasecmp(face,
						   "New Century Schoolbook",
						   22) ||
						   !my_strncasecmp(face,
						   "Century", 7)) {
					    pcc->cur_font_family = CENTURY;
					    break;
					} else if (!my_strncasecmp(face,
						   "Symbol", 6)) {
					    pcc->cur_font_family = SYMBOL;
					    break;
					}
					face = strtok(NULL, ", ");
				    }
				    SetFontSize(hw, pcc, 0);
				    free(tptr);
				}
			}
		}
		break;
	    case M_BASEFONT:
		if (mark->is_end)
			break;
		if (hw->html.font_sizes) {
			tptr = ParseMarkTag(mark->start, MT_BASEFONT, "size");
			if (tptr) {
				int size = atoi(tptr);

				if ((size > 0) && (size < 8))
					pcc->cur_font_base = size;
				free(tptr);
			}
			tptr = ParseMarkTag(mark->start, MT_BASEFONT, "face");
			if (tptr) {
			    CurFontFamily family = TIMES;
			    char *face = strtok(tptr, ", ");

			    while (face) {
				if (!my_strncasecmp(face, "Times", 5) ||
				    !my_strncasecmp(face, "Book", 4) ||
				    !my_strncasecmp(face, "Georgia", 7) ||
				    !my_strncasecmp(face, "Minion", 6) ||
				    !my_strncasecmp(face, "Roman", 5)) {
					family = TIMES;
					break;
				} else if (!my_strncasecmp(face, "Lucida", 6) ||
					   !my_strncasecmp(face, "Comic", 5) ||
					   !my_strncasecmp(face, "Letter", 6) ||
					   !my_strncasecmp(face, "Trebuchet",
					   9)) {
					family = LUCIDA;
					break;
				} else if (!my_strncasecmp(face,
					   "Helvetica", 9) ||
					   !my_strncasecmp(face,"Albertus",8) ||
					   !my_strncasecmp(face, "Arial", 5) ||
					   !my_strncasecmp(face, "Modern", 6) ||
					   !my_strncasecmp(face, "Tohoma", 6) ||
					   !my_strncasecmp(face,"Univers", 7) ||
					   !my_strncasecmp(face,"Verdana", 7)) {
					family = HELVETICA;
					break;
				} else if (!my_strncasecmp(face,"Century", 7) ||
					   !my_strncasecmp(face,
					   "New Century Schoolbook", 22)) {
					family = CENTURY;
					break;
				} else if (!my_strncasecmp(face, "Symbol", 6)) {
					family = SYMBOL;
					break;
				}
				face = strtok(NULL, ", ");
			    }
			    DefaultFontFamily(hw, pcc, family);
			    free(tptr);
			}
		}
		if (hw->html.font_colors) {
			tptr = ParseMarkTag(mark->start, MT_BASEFONT, "color");
			if (tptr) {
				hw_do_color(hw, "base", tptr, pcc);
				free(tptr);
			}
		}
		break;

	    case M_BASE:
		if (mark->is_end)
			break;
		if (tptr = ParseMarkTag(mark->start, MT_BASE, "href")) {
			XtCallCallbackList((Widget)hw, hw->html.base_callback,
				           (XtPointer)tptr);
			free(tptr);
		}
		if (!pcc->basetarget) {
			if (pcc->basetarget = ParseMarkTag(mark->start, MT_BASE,
							   "target")) {
				if (!*pcc->basetarget) {
					free(pcc->basetarget);
					pcc->basetarget = NULL;
				}
			}
		}
		break;
	    case M_META:
		if (mark->is_end || pcc->cw_only)
		    break;
		if (tptr = ParseMarkTag(mark->start, MT_META, "http-equiv")) {
		    if (!my_strcasecmp(tptr, "refresh")) {
			char *cptr;

			if (cptr = ParseMarkTag(mark->start, MT_META,
				    		"content")) {
			    XtAppContext app_con;
			    int seconds;
			    RefreshInfo *rinfo;
			    char *sptr;
			    char *url = ParseMarkTag(cptr, "", "url");

			    if (!url && (sptr = strpbrk(cptr, ",;")))
				/* No blank in front of URL= */
				url = ParseMarkTag(++sptr, "", "url");

			    if (!(sptr = strtok(cptr, ",; "))) {
				free(cptr);
				free(tptr);
				if (url)
				    free(url);
				break;
			    }
			    seconds = atoi(sptr);

			    if (!url && !(sptr = strtok(NULL, " "))) {
				url = strdup("");
			    } else if (!url) {
				/* No "URL=" in front of it */
				url = strdup(sptr);
			    }
#ifndef DISABLE_TRACE
			    if (htmlwTrace || refreshTrace)
				fprintf(stderr, "Refresh = %d, URL = %s\n",
					seconds, url);
#endif
			    rinfo = (RefreshInfo *)malloc(sizeof(RefreshInfo));
			    rinfo->refresh = hw->html.refresh_count;
			    rinfo->url = url;
			    rinfo->hw = hw;
			    app_con = XtWidgetToApplicationContext((Widget)hw);
			    hw->html.refreshdata = rinfo;
			    hw->html.refresh_timer = XtAppAddTimeOut(app_con,
						seconds * 1000,
						(XtTimerCallbackProc)RefreshURL,
						(XtPointer)rinfo);
			    free(cptr);
			}
		    }
		    free(tptr);
		}
		break;

	    case M_SPACER: {
		int width = 1;
		int height = 1;
		int baseline = 1;
		int vertical = 0;
		int orig_x;
		DivAlignType aligned = ALIGN_NONE;

		if (mark->is_end)
			break;
		if (tptr = ParseMarkTag(mark->start, MT_SPACER, "type")) {
			char *sptr;

			if (!my_strcasecmp(tptr, "block")) {
				if (sptr = ParseMarkTag(mark->start, MT_SPACER,
							"WIDTH")) {
					width = atoi(sptr);
					if (width <= 0)
						width = 1;
					free(sptr);
				}
				if (sptr = ParseMarkTag(mark->start, MT_SPACER,
							"HEIGHT")) {
					height = atoi(sptr);
					if (height <= 0)
						height = 1;
					baseline = height;
					free(sptr);
				}
				if (sptr = ParseMarkTag(mark->start, MT_SPACER,
							"ALIGN")) {
				    /* BOTTOM is default */
				    if (!my_strcasecmp(sptr, "TOP")) {
					if (!pcc->cur_line_height) {
					    baseline = pcc->cur_font->ascent;
					} else {
					    baseline = pcc->cur_baseline;
					}
				    } else if (!my_strcasecmp(sptr,
					       "TEXTTOP")) {
					baseline = -3;
				    } else if (!my_strcasecmp(sptr, "CENTER") ||
					       !my_strcasecmp(sptr, "MIDDLE")) {
					baseline = height / 2;
				    } else if (!my_strcasecmp(sptr,
					       "ABSMIDDLE")) {
					baseline = -1;
				    } else if (!my_strcasecmp(sptr,
					       "ABSBOTTOM")) {
					baseline = -2;
				    } else if (!my_strcasecmp(sptr, "RIGHT")) {
					aligned = HALIGN_RIGHT;
				    } else if (!my_strcasecmp(sptr, "LEFT")) {
					aligned = HALIGN_LEFT;
				    }
				    free(sptr);
				}
			} else if (!my_strcasecmp(tptr, "vertical")) {
				if (sptr = ParseMarkTag(mark->start, MT_SPACER,
							"SIZE")) {
					ConditionalLineFeed(hw, 1, pcc);
					vertical = 1;
					pcc->is_bol = False;
					pcc->pf_lf_state = 0;
					height = atoi(sptr);
					if (height <= 0)
						height = 1;
					baseline = height;
					free(sptr);
				}
			} else if (!my_strcasecmp(tptr, "horizontal")) {
				if (sptr = ParseMarkTag(mark->start, MT_SPACER,
							"SIZE")) {
					width = atoi(sptr);
					if (width <= 0)
						width = 1;
					free(sptr);
				}
			} else {
				free(tptr);
				break;
			}
			free(tptr);
		} else {
			break;
		}
		/* Left aligned spacers go at beginning of line */
		if ((aligned == HALIGN_LEFT) && !pcc->is_bol)
			ConditionalLineFeed(hw, 1, pcc);

		/* Now look if the spacer is too wide, if so insert linefeed. */
		if (!pcc->preformat && !pcc->cw_only && !vertical &&
		    (!pcc->is_bol || pcc->float_right) &&
		    ((pcc->x + width) > (pcc->eoffsetx +
		      pcc->left_margin + pcc->cur_line_width))) {
			ConditionalLineFeed(hw, 1, pcc);
			/* If still no room, then have a previous right
			 * floating object.  Force a linefeed past it. */
			if (pcc->float_right && ((pcc->x + width) >
			     (pcc->eoffsetx + pcc->left_margin +
			      pcc->cur_line_width))) {
				if (pcc->cur_line_height <
				    (pcc->float_right->y - pcc->y + 1)) {
					pcc->cur_line_height =
					       pcc->float_right->y - pcc->y + 1;
					LinefeedPlace(hw, pcc);
				}
			}
		}
		if (aligned == HALIGN_RIGHT) {
		    orig_x = pcc->x;
		    if (((pcc->eoffsetx + pcc->left_margin +
			  pcc->cur_line_width - width) >= pcc->x) &&
		        (width <= (pcc->eoffsetx + pcc->left_margin +
				   pcc->cur_line_width))) {
			pcc->x = pcc->eoffsetx + pcc->left_margin +
				 pcc->cur_line_width - width;
		    } else {
			ConditionalLineFeed(hw, 1, pcc);
			orig_x = pcc->eoffsetx + pcc->left_margin;
			if (width < (pcc->eoffsetx +
			             pcc->left_margin + pcc->cur_line_width)) {
				pcc->x = pcc->eoffsetx + pcc->left_margin +
					 pcc->cur_line_width - width;
			} else {
				/* No room to do any right alignment */
				aligned = ALIGN_NONE;
			}
		    }
		}
		if (!pcc->cw_only) {
			if (aligned == ALIGN_NONE) {
			    ElemInfo *eptr;

			    eptr = CreateElement(hw, E_SPACER, pcc->cur_font,
						 pcc->x, pcc->y, width, height,
						 baseline, pcc);
			    eptr->underline_number = 0;
			    AdjustBaseLine(eptr, pcc);
			}
		} else {                   
			if (pcc->computed_min_x < (width + pcc->eoffsetx +
			    			   pcc->left_margin))
                		pcc->computed_min_x = width + pcc->eoffsetx +
						      pcc->left_margin;
			if (pcc->nobr && (pcc->computed_min_x <
			    		  (pcc->nobr_x + width)))
				pcc->computed_min_x = pcc->nobr_x + width;
			if ((pcc->x + width) > pcc->computed_max_x)
                		pcc->computed_max_x = pcc->x + width;
                	if (pcc->cur_line_height < height)       
				pcc->cur_line_height = height;        
	        }
		if (vertical) {
			ConditionalLineFeed(hw, 1, pcc);
		} else if (aligned == HALIGN_LEFT) {
			FloatRec *tmp_float;

			tmp_float = (FloatRec *)malloc(sizeof(FloatRec));
			tmp_float->next = pcc->float_left;
			pcc->float_left = tmp_float;
			tmp_float->type = 1;
			tmp_float->marg = width;
			pcc->left_margin += width;
			pcc->cur_line_width -= width;
			tmp_float->y = pcc->y + height;
			pcc->is_bol = True;
			pcc->pf_lf_state = 1;
			pcc->x += width;
			if (pcc->in_paragraph)
				pcc->in_paragraph++;
		} else if (aligned == HALIGN_RIGHT) {
			FloatRec *tmp_float;

			tmp_float = (FloatRec *)malloc(sizeof(FloatRec));
			tmp_float->next = pcc->float_right;
			pcc->float_right = tmp_float;
			tmp_float->type = 1;
			tmp_float->marg = width;
			pcc->right_margin += width;
			pcc->cur_line_width -= width;
			tmp_float->y = pcc->y + height;
			pcc->x = orig_x;
			if (pcc->in_paragraph)
				pcc->in_paragraph++;
		} else {
			pcc->x += width;
			if (pcc->cw_only && pcc->nobr)
				pcc->nobr_x += width;
			pcc->have_space_after = 0;
			pcc->is_bol = False;
			pcc->pf_lf_state = 0;
		}
		break;
	    }
	    /* The "IFRAME in tables" support works in the general case,
             * but may have problems otherwise
	     */
	    case M_IFRAME: {
		int width = 1;
		int height = 1;
		int baseline = 1;
		int found_h = 0;
		int found_w = 0;
		int orig_x;
		DivAlignType aligned = ALIGN_NONE;
		FrameInfo *frame;
		FrameCbData cbs;
		ElemInfo *eptr;

		if (mark->is_end || !hw->html.frame_support)
			break;

		/* Hidden by style sheet? */
		if (tptr = ParseMarkTag(mark->start, MT_IFRAME, "STYLE")) {
			char *sptr = tptr;

			while (*sptr) {
			        if (!my_strncasecmp(sptr, "visibility:", 11)) {
					char *cptr = sptr + 11;

					while (ISSPACE((int)*cptr))
						cptr++;
					if (!my_strncasecmp(cptr, "hidden", 6))
						break;
				}
				sptr++;
			}
			free(tptr);
			if (sptr)
				break;
		}
		frame = calloc(1, sizeof(FrameInfo));

		if (!(frame->frame_src = ParseMarkTag(mark->start,
						      MT_IFRAME, "SRC"))) {
			free(frame);
			break;
		}
		/* Hack for UPS web site */
		if (!strncmp(frame->frame_src, "javascript:false", 16)) {
			free(frame->frame_src);
			free(frame);
			break;
		}
		if (!(frame->frame_name = ParseMarkTag(mark->start,
						       MT_IFRAME, "NAME"))) {
			char buf[24];

			sprintf(buf, "_frame%i", hw->html.draw_count);
			frame->frame_name = strdup(buf);
		}
		if (tptr = ParseMarkTag(mark->start, MT_IFRAME, "marginwidth")){
			frame->frame_margin_width = atoi(tptr);
			free(tptr);
		} else {
			frame->frame_margin_width = 5;
		}
		frame->frame_border = 2;
		if (tptr = ParseMarkTag(mark->start, MT_IFRAME, "frameborder")){
			if (!my_strcasecmp(tptr, "no") || (*tptr == '0'))
				frame->frame_border = 0;
			free(tptr);
		}

		if (tptr = ParseMarkTag(mark->start, MT_IFRAME, "WIDTH")) {
			found_w = 1;
			width = atoi(tptr);
			if (strchr(tptr, '%')) {
			    if ((width == 100) && !frame->frame_margin_width) {
				/* Keep it out of scrollbar area */
				width = pcc->cur_line_width - 2;
			    } else {
				width = (pcc->cur_line_width * width) / 100;
			    }
			}
			if (width <= 0)
			    width = 1;
			free(tptr);
		}

		if (width < (1 + (2 *
			    (frame->frame_border + frame->frame_margin_width))))
			width = 1 + (2 *
			     (frame->frame_border + frame->frame_margin_width));

		frame->frame_width = width;

		if (tptr = ParseMarkTag(mark->start, MT_IFRAME,"marginheight")){
			frame->frame_margin_height = atoi(tptr);
			free(tptr);
		} else {
			frame->frame_margin_height = 5;
		}
		if (tptr = ParseMarkTag(mark->start, MT_IFRAME, "HEIGHT")) {
			/* Cannot do percentage */
			if (!strchr(tptr, '%')) {
				found_h = 1;
				height = atoi(tptr);
				if (height <= 0)
					height = 1;
				baseline = height;
				free(tptr);
			}
		}
		if (height < (1 + (2 * frame->frame_border)))
			height = 1 + (2 * frame->frame_border);

		frame->frame_height = height;

		frame->frame_scroll_type = FRAME_SCROLL_AUTO;
		if (tptr = ParseMarkTag(mark->start, MT_IFRAME, "SCROLLING")) {
			if (!my_strcasecmp(tptr, "yes")) {
				frame->frame_scroll_type = FRAME_SCROLL_YES;
			} else if (!my_strcasecmp(tptr, "no")) {
				frame->frame_scroll_type = FRAME_SCROLL_NONE;
			}
			free(tptr);
		}

		/* If in table, must compute dimensions if not given.
		 * Compute width only if in table size calculation pass.
		 * Compute height only in final pass.
		 * This is really inefficient, but rarely happens */
		if ((pcc->in_table && !found_h && !pcc->cw_only) ||
		    (pcc->cw_only && !found_w)) {
			frame->cw_only = 1;

			frame->frame_x = pcc->x;
			frame->frame_y = pcc->y;
			cbs.reason = IFRAME_CREATE;
			hw->html.frames = (FrameInfo **)calloc(1,
							   sizeof(FrameInfo *));
			hw->html.frames[0] = frame;
			hw->html.nframe = 1;
			XtCallCallbackList((Widget)hw, hw->html.frame_callback,
				           &cbs);
			hw->html.nframe = 0;
			free(hw->html.frames);
			hw->html.frames = NULL;

			if (!found_h) {
				frame->frame_height = height = cbs.doc_height;
#ifndef DISABLE_TRACE
				if (htmlwTrace)
					fprintf(stderr,
						"Computed IFRAME H=%d\n",
					        frame->frame_height);
#endif
			}
			if (!found_w) {
				frame->frame_width = width = cbs.doc_width;
#ifndef DISABLE_TRACE
				if (htmlwTrace)
					fprintf(stderr,
						"Computed IFRAME W=%d\n",
					        frame->frame_width);
#endif
			}
		}
		frame->cw_only = 0;

		if (tptr = ParseMarkTag(mark->start, MT_IFRAME, "ALIGN")) {
			/* BOTTOM is default */
			if (!my_strcasecmp(tptr, "TOP")) {
				if (!pcc->cur_line_height) {
					baseline = pcc->cur_font->ascent;
				} else {
					baseline = pcc->cur_baseline;
				}
			} else if (!my_strcasecmp(tptr, "TEXTTOP")) {
				baseline = -3;
			} else if (!my_strcasecmp(tptr, "CENTER") ||
				   !my_strcasecmp(tptr, "MIDDLE")) {
				baseline = height / 2;
			} else if (!my_strcasecmp(tptr, "ABSMIDDLE")) {
				baseline = -1;
			} else if (!my_strcasecmp(tptr, "ABSBOTTOM")) {
				baseline = -2;
			} else if (!my_strcasecmp(tptr, "RIGHT")) {
				aligned = HALIGN_RIGHT;
			} else if (!my_strcasecmp(tptr, "LEFT")) {
				aligned = HALIGN_LEFT;
			}
			free(tptr);
		}
		/* Left aligned frames go at beginning of line */
		if ((aligned == HALIGN_LEFT) && !pcc->is_bol)
			ConditionalLineFeed(hw, 1, pcc);

		/* Now look if the iframe is too wide, if so insert linefeed. */
		if (!pcc->preformat && !pcc->cw_only &&
		    (!pcc->is_bol || pcc->float_right) &&
		    ((pcc->x + width) > (pcc->eoffsetx +
		      pcc->left_margin + pcc->cur_line_width))) {
			ConditionalLineFeed(hw, 1, pcc);
			/* If still no room, then have a previous right
			 * floating object.  Force a linefeed past it. */
			if (pcc->float_right && ((pcc->x + width) >
			     (pcc->eoffsetx + pcc->left_margin +
			      pcc->cur_line_width))) {
				if (pcc->cur_line_height <
				    (pcc->float_right->y - pcc->y + 1)) {
					pcc->cur_line_height =
					       pcc->float_right->y - pcc->y + 1;
					LinefeedPlace(hw, pcc);
				}
			}
		}
		if (aligned == HALIGN_RIGHT) {
		    orig_x = pcc->x;
		    if (((pcc->eoffsetx + pcc->left_margin +
			  pcc->cur_line_width - width) >= pcc->x) &&
		        (width <= (pcc->eoffsetx + pcc->left_margin +
			           pcc->cur_line_width))) {
			pcc->x = pcc->eoffsetx + pcc->left_margin +
				 pcc->cur_line_width - width;
		    } else {
			ConditionalLineFeed(hw, 1, pcc);
			orig_x = pcc->eoffsetx + pcc->left_margin;
			if (width < (pcc->eoffsetx +
				     pcc->left_margin + pcc->cur_line_width)) {
				pcc->x = pcc->eoffsetx + pcc->left_margin +
					 pcc->cur_line_width - width;
			} else {
				/* No room to do any right alignment */
				aligned = ALIGN_NONE;
			}
		    }
		}
		if (!pcc->cw_only) {
			eptr = CreateElement(hw, E_IFRAME, pcc->cur_font,
				 	     pcc->x, pcc->y, width, height,
					     baseline, pcc);
			eptr->underline_number = 0;
			if ((aligned != HALIGN_RIGHT) &&
			    (aligned != HALIGN_LEFT)) {
				AdjustBaseLine(eptr, pcc);
				frame->aligned = 0;
			} else {
				frame->aligned = 1;
			}
			eptr->frame = frame;
		} else {                   
			if (pcc->computed_min_x < (width + pcc->eoffsetx +
			    			   pcc->left_margin))
                		pcc->computed_min_x = width + pcc->eoffsetx +
						      pcc->left_margin;
			if (pcc->nobr && (pcc->computed_min_x <
			    		  (pcc->nobr_x + width)))
				pcc->computed_min_x = pcc->nobr_x + width;
			if ((pcc->x + width) > pcc->computed_max_x)
                		pcc->computed_max_x = pcc->x + width;
                	if (pcc->cur_line_height < height)       
				pcc->cur_line_height = height;        
	        }
		if (aligned == HALIGN_LEFT) {
			FloatRec *tmp_float;

			tmp_float = (FloatRec *)malloc(sizeof(FloatRec));
			tmp_float->next = pcc->float_left;
			pcc->float_left = tmp_float;
			tmp_float->type = 1;
			tmp_float->marg = width;
			pcc->left_margin += width;
			pcc->cur_line_width -= width;
			tmp_float->y = pcc->y + height;
			pcc->is_bol = True;
			pcc->pf_lf_state = 1;
			pcc->x += width;
			if (pcc->in_paragraph)
				pcc->in_paragraph++;
		} else if (aligned == HALIGN_RIGHT) {
			FloatRec *tmp_float;

			tmp_float = (FloatRec *)malloc(sizeof(FloatRec));
			tmp_float->next = pcc->float_right;
			pcc->float_right = tmp_float;
			tmp_float->type = 1;
			tmp_float->marg = width;
			pcc->right_margin += width;
			pcc->cur_line_width -= width;
			tmp_float->y = pcc->y + height;
			pcc->x = orig_x;
			if (pcc->in_paragraph)
				pcc->in_paragraph++;
		} else {
			pcc->x += width;
			if (pcc->cw_only && pcc->nobr)
				pcc->nobr_x += width;
			pcc->have_space_after = 0;
			pcc->is_bol = False;
			pcc->pf_lf_state = 0;
		}
		/* Skip contents between IFRAME tags */
		for (mark = mark->next; mark; mark = mark->next) {
			if (mark->type == M_IFRAME)
				break;
		}
		*mptr = mark;

		if (!pcc->cw_only) {	/* Create it */
			FrameInfo *fptr = hw->html.iframe_list;

			frame->frame_x = eptr->x;
			frame->frame_y = eptr->y;

			cbs.reason = IFRAME_CREATE;
			hw->html.frames = (FrameInfo **)calloc(1,
							   sizeof(FrameInfo *));
			hw->html.frames[0] = frame;
			hw->html.nframe = 1;
			XtCallCallbackList((Widget)hw, hw->html.frame_callback,
				           &cbs);
			hw->html.nframe = 0;
			free(hw->html.frames);
			hw->html.frames = NULL;
			frame->seeable = False;
			frame->mapped = False;

			/* Put it in the IFRAME list */
			frame->frame_next = NULL;
			if (!fptr) {
				hw->html.iframe_list = frame;
			} else {
				while (fptr->frame_next)
					fptr = fptr->frame_next;
				fptr->frame_next = frame;
			}
		} else {
			free(frame);
		}
		break;
	    }

	    /* Mosaic internal tags */
	    case M_MOSAIC:
		if (!pcc->ignore && (InDocHead != 1)) {
			if (tptr = ParseMarkTag(mark->start, MT_MOSAIC,
						"BULLET")) {
				BulletPlace(hw, pcc, 0);
				free(tptr);
			}
		}
		break;

	    case M_MAP:			/* Done in HTMLParse */
	    case M_AREA:
	    case M_SCRIPT:
	    case M_STYLE:

	    case M_HTML:		/* Don't know what to do with */
	    case M_COMMENT:
	    case M_PARAM:		/* May be seen in APPLET */

	    case M_TABLE_HEADER:	/* Handled in TablePlace */
	    case M_TABLE_DATA:
	    case M_TABLE_ROW:
	    case M_CAPTION:
	    case M_COL:
	    case M_COLGROUP:
	    case M_THEAD:
	    case M_TBODY:
	    case M_TFOOT:

	    case M_NOSCRIPT:		/* Just iqnore for now */
	    case M_DOCTYPE:		/* Unused */
	    case M_LINK:		/* Unused */
	    case M_NCSA:		/* NCSA annotation mark */
	    case M_NOINDEX:		/* Non-standard Atomz mark */
	    case M_UNKNOWN:		/* Already reported error elsewhere */
		break;

	    default:
#ifndef DISABLE_TRACE
		if (htmlwTrace || reportBugs)
			fprintf(stderr,
				"[TriggerMarkChanges] Unknown marker %d\n",
				mark->type);
#endif
		break;
	}
}


/* Format all the objects in the passed Widget's parsed object list to fit
 * the locally global Width.  Passes in the x,y coords of where to start 
 * placing the formatted text.  Returns the ending x,y in same variables.
 * The locally global variables are assumed to have been initialized
 * before this function was called.
 * FormatChunk also builds an internal 'ElemInfo' that contain
 * information for later X-Window code and placement.  So it needs to
 * be recursive.  For example a table can contain another table.
 */
void FormatChunk(HTMLWidget hw, MarkInfo *start_mark, MarkInfo *end_mark,
		 PhotoComposeContext *pcc)
{
	MarkInfo *mptr = start_mark;

	while (mptr) {
		TriggerMarkChanges(hw, &mptr, pcc);
		if (mptr == end_mark)
			break;
		if (mptr)
			mptr = mptr->next;
	}
	/* Now clean up any outstanding floating alignment */
	if ((pcc->float_left || pcc->float_right) && !pcc->ignore_float) {
		if (pcc->float_right)
			pcc->float_right->type = -1;
		if (pcc->float_left)
			pcc->float_left->type = -1;
		LinefeedPlace(hw, pcc);
	}
}

/* GD: added PhotoComposeContext struct */
/*
 * Called by the widget to format all the objects in the
 * parsed object list to fit its current window size.
 * Returns the max_height of the entire document.
 */
int FormatAll(HTMLWidget hw, int *Fwidth)
{
	int saved_width = *Fwidth;
	PhotoComposeContext pcc;

#ifndef DISABLE_TRACE
	if (htmlwTrace) {
#ifndef VMS
		gettimeofday(&Tv, &Tz);
		fprintf(stderr, "FormatAll enter (%d.%d)\n", Tv.tv_sec,
			Tv.tv_usec);
#else
		time_t clock = time(NULL);

                fprintf(stderr, "FormatAll enter (%s)\n",
			asctime(localtime(&clock)));
#endif
	}
#endif
	/* Stop any current animations */
	hw->html.draw_count++;

	/* Stop any outstanding refresh timers */
	hw->html.refresh_count++;

	hw->html.is_index = False;	/* Clear the is_index flag */

	memset(&pcc, 0, sizeof(PhotoComposeContext));

	/* hw->core.width - swidth - (2 * st) */
	pcc.width_of_viewable_part = saved_width;  /* Never change */
						   /* during computation */
	pcc.right_margin = hw->html.margin_width;  /* Initial margin */
	pcc.left_margin = hw->html.margin_width;
	pcc.cur_line_width = saved_width - pcc.right_margin - pcc.left_margin;
	pcc.x = pcc.left_margin;
	pcc.y = hw->html.margin_height;
	pcc.margin_height = hw->html.margin_height;
	pcc.is_bol = True;	/* We are at begin of line */
	pcc.anchor_tag_ptr = NULL_ANCHOR_PTR;	/* Are we in anchor? */
	pcc.pf_lf_state = 2;	/* Linefeed state is at start of line */
	pcc.div = DIV_ALIGN_NONE;
	pcc.valign = VALIGN_MIDDLE;
	pcc.fg = hw->manager.foreground;
	pcc.bg = hw->core.background_pixel;
	pcc.at_top = True;

	/**** memset sets them
	pcc.eoffsetx = 0;	* I am the master element *
	pcc.cur_baseline = 0;	* All objects in a line must have the same
				* baseline.  If baseline changes then adjust
				* pcc.y, pcc.cur_line_height 
				* and the y value in each element of line *
	pcc.cur_line_height = 0;
	pcc.max_line_ascent = 0;
	pcc.element_id = 0;		* To get unique number *
	pcc.have_space_after = False;	* Does word have a space after? *
	pcc.max_width_return = 0;  * We compute the MaxWidth of hyper text to
				   * adjust scrollbar.
				   * Initial value is saved_width *
	pcc.preformat = 0;
	pcc.anchor_start = 0;
	pcc.underline_start = 0;
	pcc.underline_number = 0;
	pcc.in_underlined = 0;
	pcc.dashed_underlines = False;
	pcc.cw_only = False;
	pcc.computed_min_x = 0;
	pcc.computed_max_x = 0;
	pcc.cur_form = NULL;
	pcc.in_form = False;
	pcc.widget_id = 0;
	pcc.applet_id = 0;
        pcc.superscript = 0;
        pcc.subscript = 0;
	pcc.indent_level = 0;
	pcc.text_area_buf = NULL;
	pcc.button_buf = NULL;
	pcc.button_has_text = False;
	pcc.button_has_image = False;
	pcc.ignore = 0;
	pcc.current_select = NULL;
	pcc.in_paragraph = 0;
	pcc.in_select = False;
	pcc.in_div_hidden = False;
	pcc.in_table = False;
	pcc.noformat = False;
	pcc.last_progressive_ele = NULL;
	pcc.in_anchor = False;
	pcc.fixed_anchor_color = False;
	pcc.in_label = False;
	pcc.label_id = NULL;
	pcc.float_left = NULL;
	pcc.float_right = NULL;
	pcc.nobr = 0;
	pcc.blockquote = 0;
	pcc.ignore_float = 0;
	pcc.frameset = 0;
	pcc.noframes = 0;
	pcc.blink = 0;
	pcc.Strikeout = False;
	pcc.strikeout_start = False;
	pcc.basetarget = NULL;
	pcc.mark_title = NULL;
	pcc.span_title = NULL;
	****/

	header1_align = header2_align = header3_align = 0;
	header4_align = header5_align = header6_align = 0;
	header1 = header2 = header3 = header4 = header5 = header6 = 0;

	/* Initialize local variables, some from the widget */
	DescType = &BaseDesc;
	DescType->type = D_NONE;
	DescType->count = 0;
	DescType->compact = 0;
	DescType->next = NULL;
	DescType->indent = 0;
	DescType->in_title = 0;
	InDocHead = InDocBody = 0;

	/* Free the old title, if there is one. */
	if (hw->html.title) {
		free(hw->html.title);
		hw->html.title = NULL;
	}
	/* Free any partially processed title */
	if (TitleText) {
		free(TitleText);
		TitleText = NULL;
	}
	/* Clear displayed title */
	XtCallCallbackList((Widget)hw, hw->html.title_callback, "");

	/* Free up previously formatted elements */
	FreeLineList(hw->html.formatted_elements, hw);
	hw->html.iframe_list = NULL;

	/* Destroy any previous frames */
	if (hw->html.nframe)
		HTMLDestroyFrames(hw);

	/* Start a null element list, to be filled in as we go. */
	hw->html.formatted_elements = NULL;
        hw->html.last_formatted_elem = NULL;
        hw->html.last_formatted_line = NULL;
        hw->html.first_formatted_line = NULL;
        hw->html.blinking_elements = NULL;

	/* Clear any previous selections */
	hw->html.select_start = hw->html.select_end = NULL;
	hw->html.new_start = hw->html.new_end = NULL;

	/* Get correct font in html.font and pcc.cur_font */ 
	InitFontStack(hw, &pcc);
	SetFontSize(hw, &pcc, 0);

	/* Set up alignment stack */
	AlignStack = &AlignBase;
	AlignStack->align = DIV_ALIGN_NONE;

	/*
	 * If we have parsed refresh text, process it now.
	 */
	if (hw->html.html_refresh_objects)
		FormatChunk(hw,	hw->html.html_refresh_objects, NULL, &pcc);

	/*
	 * If we have parsed special header text, fill it in now.
	 */
	if (hw->html.html_header_objects) {
		FormatChunk(hw,	hw->html.html_header_objects, NULL, &pcc);
		LinefeedPlace(hw, &pcc);
	}

	/* Format the main text */
	FormatChunk(hw, hw->html.html_objects, NULL, &pcc);

	/* Reset the font stack */
	InitFontStack(hw, &pcc);

	/*
	 * If we have parsed special footer text, fill it in now.
	 */
	if (hw->html.html_footer_objects) {
		pcc.preformat = 0;
		pcc.pf_lf_state = 0;
		LinefeedPlace(hw, &pcc);
		FormatChunk(hw, hw->html.html_footer_objects, NULL, &pcc);
	}

	/* Ensure a linefeed after the final element. */
	LinefeedPlace(hw, &pcc);

	/* Clean up any unterminated form stuff */
	if (pcc.label_id)
		free(pcc.label_id);
	if (pcc.button_buf)
		free(pcc.button_buf);
	if (pcc.text_area_buf)
		free(pcc.text_area_buf);

	/* Free them */
	if (pcc.mark_title)
		free(pcc.mark_title);
	if (pcc.span_title)
		free(pcc.span_title);

	/* Add the bottom margin to the max height. */
	pcc.y += pcc.margin_height;

	/* If the passed in MaxWidth was wrong, correct it. */
	if (pcc.max_width_return > saved_width)
		*Fwidth = pcc.max_width_return;

	/* If height is too high, tell the widget to use the vbar */
	if ((pcc.y > (hw->core.height - HbarHeight(hw))) &&
	    (!hw->html.is_frame ||
	     (hw->html.scroll_bars != FRAME_SCROLL_NONE))) {
		hw->html.use_vbar = True;
	} else {
		hw->html.use_vbar = False;
	}
	/* Needs freed if one was in document */
	if (pcc.basetarget)
		free(pcc.basetarget);

	return(pcc.y);
}

/*
 * Refresh an element (or table cell contents) into the widget's window
 */
ElemInfo *RefreshElement(HTMLWidget hw, ElemInfo *eptr)
{
#ifndef DISABLE_TRACE
    if (refreshTrace) {
	fprintf(stderr, "RefreshElement type %d at ", eptr->type);
	fprintf(stderr, "x=%d, y=%d, w=%d, h=%d, ",
		eptr->x, eptr->y, eptr->width, eptr->height);
	fprintf(stderr, "bline=%d, sx=%d, sy=%d\n",
		eptr->baseline, hw->html.scroll_x, hw->html.scroll_y);
    }
#endif
    switch (eptr->type) {
	case E_TEXT:
	    TextRefresh(hw, eptr, 0, eptr->edata_len - 2, False);
	    break;
	case E_BULLET:
	    BulletRefresh(hw, eptr);
	    break;
	case E_HRULE:
	    HRuleRefresh(hw, eptr);
	    break;
	case E_CR:
	    hw->html.underline_yoffset = -1;
	    break;
	case E_IMAGE:
	    ImageRefresh(hw, eptr, NULL);
	    break;
	case E_IFRAME:
	    IframeRefresh(eptr);
	    break;
	case E_WIDGET:
	    WidgetRefresh(hw, eptr);
	    break;
	case E_TABLE:
	    /* Do table borders */
	    TableRefresh(hw, eptr);
	    break;
	case E_CELL_TABLE:
	    /* Do table cell contents */
	    hw->html.underline_yoffset = -1;
	    eptr = CellRefresh(hw, eptr);
	    break;
	case E_APPLET:
	    AppletRefresh(hw, eptr);
	    break;
	case E_LINEFEED:
	case E_SPACER:
	case E_ANCHOR:
	    break;
	default:
#ifndef DISABLE_TRACE
    	    if (reportBugs)
		fprintf(stderr,	"Unknown Element type %d during refresh\n",
			eptr->type);
#endif
	    break;
    }
    return eptr;
}

/*
 * Locate the element (if any) that is at the passed location
 * in the widget.  If there is no corresponding element, return
 * NULL.  If an element is found, return the position of the character
 * you are at in the pos pointer passed.
 */
ElemInfo *LocateElement(HTMLWidget hw, int x, int y, int *pos)
{
	ElemInfo *eptr = hw->html.formatted_elements;
	ElemInfo *rptr = NULL;
	int tx1, tx2, ty1, ty2;

	x += hw->html.scroll_x;
	y += hw->html.scroll_y;

	/* Search element by element, for now we only search
	 * text elements, form widgets and images.
	 */
	while (eptr) {
		ty1 = eptr->y;
		ty2 = eptr->y + eptr->height;
		tx1 = eptr->x;
		tx2 = eptr->x + eptr->width;

		switch (eptr->type) {
		    case E_TEXT:
		    case E_WIDGET:
			if ((x >= tx1) && (x <= tx2) && (y >= ty1) &&(y <= ty2))
				rptr = eptr;
			break;
		    case E_IMAGE:
			/* Ignore if a blank image */
			if (eptr->pic_data->fetched ||
			    (eptr->pic_data->internal &&
			     (eptr->pic_data->internal != 3))) {
				if ((x >= tx1) && (x <= tx2) && (y >= ty1) &&
				    (y <= ty2))
					rptr = eptr;
			}
			break;
		    default:
			break;
		}
		if (rptr)
			break;
		eptr = eptr->next;
	}
	/*
	 * If we found an element, locate the exact character position within
	 * that element.
	 */
	if (rptr && rptr->type == E_TEXT) {
		int dir, ascent, descent, epos;
		XCharStruct all;

		/*
		 * Start assuming fixed width font.  The real position should
		 * always be <= to this, but just in case, start at the end
		 * of the string if it is not.
		 */
		epos = ((x - rptr->x) / rptr->font->max_bounds.width) + 1;
		if (epos >= rptr->edata_len - 1)
			epos = rptr->edata_len - 2;
		XTextExtents(rptr->font, (char *)rptr->edata, epos + 1,
			     &dir, &ascent, &descent, &all);
		if (x > (int)(rptr->x + all.width)) {
			epos = rptr->edata_len - 3;
		} else {
			epos--;
		}

		while (epos >= 0) {
			XTextExtents(rptr->font, (char *)rptr->edata, epos + 1,
				     &dir, &ascent, &descent, &all);
			if ((int)(rptr->x + all.width) <= x)
				break;
			epos--;
		}
		*pos = ++epos;
	}
	return(rptr);
}

/*
 * Used by ParseTextToPrettyString to let it be sloppy about its
 * string creation, and never overflow the buffer.
 * It concatenates the passed string to the current string, managing
 * both the current string length, and the total buffer length.
 */
static void strcpy_or_grow(char **str, int *slen, int *blen, char *add)
{
	int newlen, addlen;
	char *buf;

	/*
	 * If necessary, initialize this string buffer
	 */
	if (!*str) {
		*str = (char *)malloc(1024 * sizeof(char));
		CHECK_OUT_OF_MEM(*str);
		*blen = 1024;
		strcpy(*str, "");
		*slen = 0;
	}
	buf = *str;
	if (!buf || !add)
		return;
	addlen = strlen(add);
	newlen = *slen + addlen;
	if (newlen >= *blen) {
		newlen = ((newlen / 1024) + 1) * 1024;
		buf = (char *)malloc(newlen * sizeof(char));
		CHECK_OUT_OF_MEM(buf);
		memcpy(buf, *str, *blen);
		free((char *)*str);
		*str = buf;
		*blen = newlen;
	}
	memcpy((char *)(buf + *slen), add, addlen + 1);
	*slen = *slen + addlen;
}

/*
 * Parse all the formatted text elements from start to end
 * into an ascii text string, and return it.
 * space_width and lmargin tell us how many spaces
 * to indent lines.
 */
char *ParseTextToString(ElemInfo *elist, ElemInfo *startp, ElemInfo *endp,
			int start_pos, int end_pos, int space_width,
			int lmargin)
{
	int newline = 0;
	int t_slen, t_blen, epos, i, spaces;
	char *text = NULL;
	char *tptr;
	ElemInfo *eptr, *start, *end;

	if (!startp)
		return(NULL);

	if (SwapElements(startp, endp, start_pos, end_pos)) {
		start = endp;
		end = startp;
		epos = start_pos;
		start_pos = end_pos;
		end_pos = epos;
	} else {
		start = startp;
		end = endp;
	}
	eptr = start;
	while (eptr && (eptr != end)) {
		if (eptr->type == E_TEXT) {
			if (eptr == start) {
				tptr = (char *)(eptr->edata + start_pos);
			} else {
				tptr = (char *)eptr->edata;
			}

			if (newline) {
				spaces = (eptr->x - lmargin) / space_width;
				if (spaces < 0)
					spaces = 0;
				for (i = 0; i < spaces; i++)
					strcpy_or_grow(&text, &t_slen, &t_blen,
						       " ");
			}
			strcpy_or_grow(&text, &t_slen, &t_blen, tptr);
			newline = 0;
		} else if (eptr->type == E_LINEFEED) {
			strcpy_or_grow(&text, &t_slen, &t_blen, "\n");
			newline = 1;
		}
		eptr = eptr->next;
	}
	if (eptr) {
		if (eptr->type == E_TEXT) {
			char *tend, tchar;

			if (eptr == start) {
				tptr = (char *)(eptr->edata + start_pos);
			} else {
				tptr = (char *)eptr->edata;
			}

			if (eptr == end) {
				tend = (char *)(eptr->edata + end_pos + 1);
				tchar = *tend;
				*tend = '\0';
			}
			if (newline) {
				spaces = (eptr->x - lmargin) / space_width;
				if (spaces < 0)
					spaces = 0;
				for (i = 0; i < spaces; i++)
					strcpy_or_grow(&text, &t_slen,
						       &t_blen, " ");
			}
			strcpy_or_grow(&text, &t_slen, &t_blen, tptr);
			if (eptr == end)
				*tend = tchar;
		} else if (eptr->type == E_LINEFEED) {
			strcpy_or_grow(&text, &t_slen, &t_blen, "\n");
		}
	}
	return(text);
}

/*
 * Parse all the formatted text elements from start to end
 * into an ascii text string, and return it.
 * Very like ParseTextToString() except the text is prettied up
 * to show headers and the like.
 * space_width and lmargin tell us how many spaces to indent lines.
 */
char *ParseTextToPrettyString(HTMLWidget hw, ElemInfo *elist,
			      ElemInfo *startp, ElemInfo *endp,
			      int start_pos, int end_pos,
			      int space_width, int lmargin)
{
	int newline = 0;
	int lead_spaces = 0;
	char *text = NULL;
	char *line_buf = NULL;
	char *tptr;
	int i, spaces, epos;
	int t_slen, t_blen, l_slen, l_blen;
	char lchar;
	ElemInfo *eptr, *start, *end, *last;

	if (!startp)
		return(NULL);

	if (SwapElements(startp, endp, start_pos, end_pos)) {
		start = endp;
		end = startp;
		epos = start_pos;
		start_pos = end_pos;
		end_pos = epos;
	} else {
		start = startp;
		end = endp;
	}

	/* We need to know if we should consider the indentation or bullet
	 * that might be just before the first selected element to also be
	 * selected.  This current hack looks to see if they selected the
	 * whole line, and assumes if they did, they also wanted the beginning.
	 *
	 * If we are at the beginning of the list, or the beginning of
	 * a line, or just behind a bullet, assume this is the start of
	 * a line that we may want to include the indent for.
	 */
	if (!start_pos && (!start->prev || (start->prev->type == E_BULLET))) {
		eptr = start;
		while (eptr && (eptr != end) && (eptr->type != E_LINEFEED))
			eptr = eptr->next;
		if (eptr && (eptr->type == E_LINEFEED)) {
			newline = 1;
			if (start->prev && (start->prev->type == E_BULLET))
				start = start->prev;
		}
	}
	last = eptr = start;
	while (eptr && (eptr != end)) {
		if (eptr->type == E_BULLET) {
			if (newline) {
				spaces = (eptr->x - lmargin) / space_width;
				spaces -= 2;
				if (spaces < 0)
					spaces = 0;
				lead_spaces = spaces;
				for (i = 0; i < spaces; i++)
					strcpy_or_grow(&line_buf, &l_slen, 
						       &l_blen, " ");
			}
			newline = 0;
			strcpy_or_grow(&line_buf, &l_slen, &l_blen, "o ");
			lead_spaces += 2;
		} else if (eptr->type == E_TEXT) {
			if (eptr == start) {
				tptr = (char *)(eptr->edata + start_pos);
			} else {
				tptr = (char *)eptr->edata;
			}
			if (newline) {
				spaces = (eptr->x - lmargin) / space_width;
				if (spaces < 0)
					spaces = 0;
				lead_spaces = spaces;
				for (i = 0; i < spaces; i++)
					strcpy_or_grow(&line_buf, &l_slen,
						       &l_blen, " ");
			}
			strcpy_or_grow(&line_buf, &l_slen, &l_blen, tptr);
			newline = 0;
		} else if (eptr->type == E_LINEFEED) {
			strcpy_or_grow(&text, &t_slen, &t_blen, line_buf);
			strcpy_or_grow(&text, &t_slen, &t_blen, "\n");
			newline = 1;
			lchar = '\0';
			if (eptr->font == hw->html.header1_font) {
				lchar = '*';
			} else if (eptr->font == hw->html.header2_font) {
				lchar = '=';
			} else if (eptr->font == hw->html.header3_font) {
				lchar = '+';
			} else if (eptr->font == hw->html.header4_font) {
				lchar = '-';
			} else if (eptr->font == hw->html.header5_font) {
				lchar = '~';
			} else if (eptr->font == hw->html.header6_font) {
				lchar = '.';
			}
			if (lchar) {
				char *ptr = line_buf;
				int cnt = 0;

				while (ptr && *ptr) {
					if (++cnt > lead_spaces)
						*ptr = lchar;
					ptr++;
				}
				strcpy_or_grow(&text,&t_slen,&t_blen, line_buf);
				strcpy_or_grow(&text, &t_slen, &t_blen, "\n");
			}
			if (line_buf) {
				free(line_buf);
				line_buf = NULL;
			}
		}
		last = eptr;
		eptr = eptr->next;
	}
	if (eptr) {
		if (eptr->type == E_BULLET) {
			if (newline) {
				spaces = (eptr->x - lmargin) / space_width;
				spaces -= 2;
				if (spaces < 0)
					spaces = 0;
				lead_spaces = spaces;
				for (i = 0; i < spaces; i++)
					strcpy_or_grow(&line_buf, &l_slen,
						       &l_blen, " ");
			}
			strcpy_or_grow(&line_buf, &l_slen, &l_blen, "o ");
			lead_spaces += 2;
		} else if (eptr->type == E_TEXT) {
			char *tend, tchar;

			if (eptr == start) {
				tptr = (char *)(eptr->edata + start_pos);
			} else {
				tptr = (char *)eptr->edata;
			}
			if (eptr == end) {
				tend = (char *)(eptr->edata + end_pos + 1);
				tchar = *tend;
				*tend = '\0';
			}
			if (newline) {
				spaces = (eptr->x - lmargin) / space_width;
				if (spaces < 0)
					spaces = 0;
				lead_spaces = spaces;
				for (i = 0; i < spaces; i++)
					strcpy_or_grow(&line_buf, &l_slen,
						       &l_blen, " ");
			}
			strcpy_or_grow(&line_buf, &l_slen, &l_blen, tptr);
			if (eptr == end)
				*tend = tchar;
		} else if (eptr->type == E_LINEFEED) {
			strcpy_or_grow(&text, &t_slen, &t_blen, line_buf);
			strcpy_or_grow(&text, &t_slen, &t_blen, "\n");
			lchar = '\0';
			if (eptr->font == hw->html.header1_font) {
				lchar = '*';
			} else if (eptr->font == hw->html.header2_font) {
				lchar = '=';
			} else if (eptr->font == hw->html.header3_font) {
				lchar = '+';
			} else if (eptr->font == hw->html.header4_font) {
				lchar = '-';
			} else if (eptr->font == hw->html.header5_font) {
				lchar = '~';
			} else if (eptr->font == hw->html.header6_font) {
				lchar = '.';
			}
			if (lchar) {
				char *ptr = line_buf;
				int cnt = 0;

				while (ptr && *ptr) {
					if (++cnt > lead_spaces)
						*ptr = lchar;
					ptr++;
				}
				strcpy_or_grow(&text,&t_slen,&t_blen, line_buf);
				strcpy_or_grow(&text, &t_slen, &t_blen, "\n");
			}
			if (line_buf) {
				free(line_buf);
				line_buf = NULL;
			}
		}
		last = eptr;
	}
	if (line_buf) {
		strcpy_or_grow(&text, &t_slen, &t_blen, line_buf);
		lchar = '\0';
		if (last->font == hw->html.header1_font) {
			lchar = '*';
		} else if (last->font == hw->html.header2_font) {
			lchar = '=';
		} else if (last->font == hw->html.header3_font) {
			lchar = '+';
		} else if (last->font == hw->html.header4_font) {
			lchar = '-';
		} else if (last->font == hw->html.header5_font) {
			lchar = '~';
		} else if (last->font == hw->html.header6_font) {
			lchar = '.';
		}
		if (lchar) {
			char *ptr = line_buf;
			int cnt = 0;

			while (ptr && *ptr) {
				if (++cnt > lead_spaces)
					*ptr = lchar;
				ptr++;
			}
			strcpy_or_grow(&text, &t_slen, &t_blen, "\n");
			strcpy_or_grow(&text, &t_slen, &t_blen, line_buf);
		}
	}
	if (line_buf) {
		free(line_buf);
		line_buf = NULL;
	}
	return(text);
}

/* Used to find the longest line (in characters) in a collection of text blocks.
 * cnt is the running count of characters, and txt is the pointer to the current
 * text block.  Since we are finding line widths, a newline resets the width
 * count.
 */
static char *MaxTextWidth(char *txt, int *cnt)
{
	char *start = txt;
	char *end;
	int width = *cnt;

	if (!start)
		return(NULL);

	/* If this blocks starts with a newline, reset the width
	 * count, and skip the newline.
	 */
	while ((*start == '\n') || (*start == '\r')) {
		width = 0;
		start++;
	}
	end = start;
	/* Count characters, stopping either at a newline, or at the
	 * end of this text block.  Expand tabs.
	 */
	while (*end && (*end != '\n') && (*end != '\r')) {
		if (*end == '\t') {
			width = ((width / 8) + 1) * 8;
		} else {
			width++;
		}
		end++;
	}
	*cnt = width;
	return(end);
}

/* Find the preferred width of a parsed HTML document.
 * Currently unformatted plain text, unformatted listing text, plain files
 * and preformatted text require special width.
 * Preferred width = (width of longest plain text line in document) *
 *		     (width of that text's font)
 */
int DocumentWidth(HTMLWidget hw, MarkInfo *list)
{
	MarkInfo *mptr = list;
	int plain_text = 0;
	int listing_text = 0;
	int pwidth = 0;
	int lwidth = 0;
	int width = 0;
	int pcnt, lcnt;
	char *ptr;

	/* Loop through object list looking at the plain, preformatted text */
	while (mptr) {
		/* All text blocks between the starting and ending
		 * plain and pre text markers are plain text blocks.
		 * Manipulate flags so we recognize these blocks.
		 */
		if ((mptr->type == M_PLAIN_TEXT) ||
		    (mptr->type == M_PLAIN_FILE) ||
		    (mptr->type == M_PREFORMAT)) {
			if (mptr->is_end) {
				if (--plain_text < 0)
					plain_text = 0;
			} else {
				plain_text++;
			}
			pcnt = lcnt = 0;
		}
		/*
		 * All text blocks between the starting and ending
		 * listing markers are listing text blocks.
		 */
		else if (mptr->type == M_LISTING_TEXT) {
			if (mptr->is_end) {
				if (--listing_text < 0)
					listing_text = 0;
			} else {
				listing_text++;
			}
			lcnt = 	pcnt = 0;
		}
		/* If this is a plain text block, add to line length.
		 * Find the Max of all line lengths.
		 */
		else if (plain_text && (mptr->type == M_NONE)) {
			ptr = mptr->text;
			while (ptr && *ptr) {
				ptr = MaxTextWidth(ptr, &pcnt);
				if (pcnt > pwidth)
					pwidth = pcnt;
			}
		}
		/*
		 * If this is a listing text block, add to line length.
		 * Find the Max of all line lengths.
		 */
		else if (listing_text && (mptr->type == M_NONE)) {
			ptr = mptr->text;
			while (ptr && *ptr) {
				ptr = MaxTextWidth(ptr, &lcnt);
				if (lcnt > lwidth)
					lwidth = lcnt;
			}
		}
		mptr = mptr->next;
	}
	width = pwidth * hw->html.plain_font->max_bounds.width;
	lwidth = lwidth * hw->html.listing_font->max_bounds.width;
	if (lwidth > width)
		width = lwidth;
	return(width);
}
