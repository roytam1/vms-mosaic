/* Copyright (C) 1996 - G.Dauphin
 * See the file "license.mMosaic" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * HTMLtext.c
 * Author: Gilles Dauphin
 * Version 3.0 [Sep96]
 *
 * Most of code is rewrite from scratch.  Lot of code came from NCSA Mosaic.
 */

/*
 * Very heavily modified for VMS Mosaic 3.0.  1998 - George Cook
 * Copyright (C) 1998, 1999, 2000, 2005, 2006, 2007 - the VMS Mosaic Project
 */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "HTMLmiscdefs.h"
#include "HTMLparse.h"
#include "HTMLP.h"
#include "HTMLPutil.h"

#if defined(MULTINET) && defined(__DECC) && (__VMS_VER >= 70000000)
#define strdup  decc$strdup
#endif /* VMS V7, VRH, GEC, MPJZ */
extern char *strdup();

#define SKIPWHITE(s)    while ((((unsigned char)*s) < 128) && isspace(*s)) s++
#define SKIPNONWHITE(s) while ((((unsigned char)*s) > 127) || \
			       (!isspace(*s) && *s)) s++
#define COMP_LINE_BUF_LEN 1024

void ConditionalLineFeed(HTMLWidget hw, int state, PhotoComposeContext *pcc)
{
    /* Don't mess with anything if just after list bullet */
    if (pcc->is_bol != 2) {
	/*
	 * For formatted documents there are 3 linefeed states:
	 * 0 = in the middle of a line
	 * 1 = at left margin
	 * 2 = at left margin with blank line above
	 */
	if (pcc->pf_lf_state < state) {
		/* The first line starts with it zero */
		if (!pcc->cur_line_height)
			pcc->cur_line_height = pcc->cur_font->ascent +
					       pcc->cur_font->descent;
		/*
		 * If this function is being used to insert a blank line,
		 * we need to look at the percentVerticalSpace resource
		 * to see how high to make the line.
		 */
		if ((state == 2) && (hw->html.percent_vert_space > 0))
			pcc->cur_line_height = pcc->cur_line_height *
					      hw->html.percent_vert_space / 100;
		LinefeedPlace(hw, pcc);
	} else {
		pcc->cur_line_height = pcc->cur_baseline = 0;
	}
    }
}

/* Terminate current line and move down some space.
 */
void LinefeedPlace(HTMLWidget hw, PhotoComposeContext *pcc)
{
	ElemInfo *eptr;
	int adjx = 0;
	FloatRec *tmp_float;

	/* Reset these in all cases */
	pcc->is_bol = 1;
	pcc->have_space_after = 0;
	pcc->at_top = False;
	pcc->max_line_ascent = 0;
	/*
         * Manipulate linefeed state for special pre-formatted linefeed
         * hack for broken HTMLs.
         */
	if (pcc->preformat == 1) {
		switch(pcc->pf_lf_state) {
			/*
			 * Collapse multiple soft linefeeds within a PRE
			 */
			case 1:
			/*
			 * Ignore soft linefeeds after hard linefeeds
			 * within a PRE
			 */
			case 2:
				return;
			/*
			 * First soft linefeed
			 */
			case 0:
			default:
				pcc->pf_lf_state = 1;
				break;
		}
	} else if (pcc->preformat == 2) {
		switch(pcc->pf_lf_state) {
			/*
			 * Previous soft linefeed should have been ignored, so
			 * ignore this hard linefeed, but set state like it
			 * was not ignored.
			 */
			case 1:
				pcc->pf_lf_state = 2;
				return;
			/*
			 * Honor multiple hard linefeeds.
			 */
			case 2:
				break;
			/*
			 * First hard linefeed
			 */
			case 0:
			default:
				pcc->pf_lf_state = 2;
				break;
		}
	} else if (pcc->pf_lf_state < 2) {
		pcc->pf_lf_state++;
	}
	/* A table cell starts with it zero */
	if (!pcc->cur_line_height)
		pcc->cur_line_height = pcc->cur_font->ascent +
				       pcc->cur_font->descent;

	/* We need to compute center or right adjustment here. */
	if ((pcc->div == DIV_ALIGN_CENTER) || (pcc->div == DIV_ALIGN_RIGHT)) {
		adjx = pcc->cur_line_width - 
				    (pcc->x - pcc->eoffsetx - pcc->left_margin);
		if (pcc->div == DIV_ALIGN_CENTER)
			adjx = adjx / 2;
	}

	if (pcc->cw_only) {	/* Compute width only, don't create Element */
		if (pcc->computed_min_x > pcc->computed_maxmin_x)
			pcc->computed_maxmin_x = pcc->computed_min_x;
		if (pcc->x > pcc->computed_max_x)
			pcc->computed_max_x = pcc->x;
		pcc->y += pcc->cur_line_height;

		while (pcc->float_left && !pcc->ignore_float &&
		       ((pcc->y >= pcc->float_left->y) ||
		        (pcc->float_left->type == -1))) {

			pcc->left_margin -= pcc->float_left->marg;
			pcc->cur_line_width += pcc->float_left->marg;
			if (pcc->y <= pcc->float_left->y)
				pcc->y = pcc->float_left->y + 1;
			tmp_float = pcc->float_left;
			pcc->float_left = tmp_float->next;
			if (pcc->float_left && (tmp_float->type == -1))
				pcc->float_left->type = -1;
			free(tmp_float);
		}
		while (pcc->float_right && !pcc->ignore_float &&
		       ((pcc->y >= pcc->float_right->y) ||
		        (pcc->float_right->type == -1))) {

			pcc->right_margin -= pcc->float_right->marg;
			pcc->cur_line_width += pcc->float_right->marg;
			if (pcc->y <= pcc->float_right->y)
				pcc->y = pcc->float_right->y + 1;
			tmp_float = pcc->float_right;
			pcc->float_right = tmp_float->next;
			if (pcc->float_right && (tmp_float->type == -1))
				pcc->float_right->type = -1;
			free(tmp_float);
		}
		/* Reset for next line */
		pcc->computed_min_x = pcc->left_margin + pcc->eoffsetx;
		pcc->nobr_x = pcc->x = pcc->left_margin + pcc->eoffsetx;
		pcc->cur_baseline = pcc->cur_line_height = 0;
		return;
	}

	/* Linefeed is at end of line */
	eptr = CreateElement(hw, E_LINEFEED, pcc->cur_font, pcc->x, pcc->y, 0,
			     pcc->cur_line_height, pcc->cur_baseline, pcc);

	/* Keep info for Postscript printing */
	if (!hw->html.first_formatted_line)
		hw->html.first_formatted_line = eptr;
	eptr->line_next = NULL;
	if (hw->html.last_formatted_line)
		hw->html.last_formatted_line->line_next = eptr;
	hw->html.last_formatted_line = eptr;
		
	/* We need to center or right adjust the elements here. */
	if (adjx > 0) {
		int orig_x;
		unsigned long orig_bg;
		ElemInfo *septr = eptr;

		/* Back to the list until CR and adjust each x with the adjx */
		while (septr &&
		       ((septr->type != E_CR) && (septr->type != E_HRULE))) {
			if ((septr->type != E_IMAGE) ||
			    !septr->pic_data->aligned)
				septr->x += adjx;
			if (septr->type == E_WIDGET) {
				/* It is already there so don't need markup */
				orig_x = pcc->x;
				pcc->x = septr->x;
				orig_bg = pcc->bg;
				pcc->bg = septr->bg;
				MakeWidget(hw, NULL, pcc,
					   septr->widget_data->id);
				pcc->x = orig_x;
				pcc->bg = orig_bg;
			} else if (septr->type == E_IFRAME) {
				septr->frame->frame_x = septr->x;
			}
			septr = septr->prev;
		}
		pcc->x += adjx;
	}

	/* At the end of every line check if we have a new MaxWidth */
	if (pcc->x + pcc->right_margin > pcc->max_width_return)
		pcc->max_width_return = pcc->x + pcc->right_margin;

	pcc->y += pcc->cur_line_height;

	while (pcc->float_left && !pcc->ignore_float &&
	       ((pcc->y >= pcc->float_left->y) ||
	        (pcc->float_left->type == -1))) {

		pcc->left_margin -= pcc->float_left->marg;
		pcc->cur_line_width += pcc->float_left->marg;
		if (pcc->y <= pcc->float_left->y)
			pcc->y = pcc->float_left->y + 1;
		tmp_float = pcc->float_left;
		pcc->float_left = tmp_float->next;
		if (pcc->float_left && (tmp_float->type == -1))
			pcc->float_left->type = -1;
		free(tmp_float);
	}
	while (pcc->float_right && !pcc->ignore_float &&
	       ((pcc->y >= pcc->float_right->y) ||
	        (pcc->float_right->type == -1))) {

		pcc->right_margin -= pcc->float_right->marg;
		pcc->cur_line_width += pcc->float_right->marg;
		if (pcc->y <= pcc->float_right->y)
			pcc->y = pcc->float_right->y + 1;
		tmp_float = pcc->float_right;
		pcc->float_right = tmp_float->next;
		if (pcc->float_right && (tmp_float->type == -1))
			pcc->float_right->type = -1;
		free(tmp_float);
	}
	pcc->x = pcc->left_margin + pcc->eoffsetx;
	pcc->cur_baseline = pcc->cur_line_height = 0;
	/* CR is at begin of line */
	eptr = CreateElement(hw, E_CR, pcc->cur_font, pcc->x, pcc->y, 0,
			     pcc->cur_line_height, pcc->cur_baseline, pcc);
}


/* Remove Blank, LF, CR, Tab */
static char **split_in_word(char *text, unsigned int *nw, int *hsb4, int *hsa)
{
	char **words;
	char *w_text, *pword, *fin, *t;
	char *deb = text;
	unsigned int nword = 0;
	unsigned int i;

	if ((((unsigned char)*text) < 128) && isspace(*text)) {
		*hsb4 = 1;
	} else {
		*hsb4 = 0;
	}
	fin = text + strlen(text) - 1;
	if ((((unsigned char)*fin) < 128) && isspace(*fin)) {
		*hsa = 1;
	} else {
		*hsa = 0;
	}
	/* Count the number of words */
	while (*deb) {
		SKIPWHITE(deb);
		if (!*deb)
			break;
		SKIPNONWHITE(deb);
		nword++;
	}
	*nw = nword;
	if (!nword)
		return NULL;
	words = (char **) malloc(nword * sizeof(char *));
	deb = w_text = GetMarkText(text);
	for (i = 0; i < nword; i++) {
		SKIPWHITE(deb);
		pword = deb;
		SKIPNONWHITE(deb);
		*deb++ = '\0';
		t = words[i] = GetMarkText(pword);
		/* Set NBSP to white */
		while (*t) {
			if (*t == NBSP_CONST)
				*t = ' ';
			t++;
		}
	}
	FreeMarkText(w_text);
	return words;
}

static void Set_E_TEXT_Element(HTMLWidget hw, ElemInfo *eptr, char *text,
			       PhotoComposeContext *pcc)
{
	eptr->edata = GetMarkText(text);
	eptr->edata_len = strlen(text) + 1;

	/* If this is an anchor, attach its href and name
	 * values to the element. */
	eptr->anchor_tag_ptr = pcc->anchor_tag_ptr;

	/* If blink is on, then link into blink list */
	if (pcc->blink) {
		eptr->blink_next = hw->html.blinking_elements;
		hw->html.blinking_elements = eptr;
	}
}
 
/* Format and place a piece of text.
 * The context is given by pcc:
 *	pcc->x, pcc->y :  this is where to place the text inside of the view.
 *	pcc->width_of_viewable_part:  the width of viewable part (view)
 *
 *	pcc->cur_line_width :  # of pixels for the composed line
 *	pcc->eoffsetx + pcc->left_margin :  where to go when new line.
 *				  	    Relative to view.
 *	pcc->right_margin :  right margin
 *
 * Place the text (mptr->text) at [pcc->x, pcc->y]
 *
 */
void PartOfTextPlace(HTMLWidget hw, MarkInfo *mptr, PhotoComposeContext *pcc)
{
	ElemInfo *eptr;
	char *text = mptr->text;
	char **words;
	char *composed_line, *the_word;
	unsigned int nword;
	int composed_line_width, word_width, font_height;
	int i, baseline, len;
	int have_space_b4 = 0;
	int have_space_after = 0;

	words = split_in_word(text, &nword, &have_space_b4, &have_space_after);
	if (!nword) {
		pcc->have_space_after = have_space_after;
		return;
	}
	if (pcc->have_space_after && !pcc->is_bol) {
		have_space_b4 = 1;
	} else if (pcc->is_bol)	{
		/* If we are at the beginning of line */
		have_space_b4 = 0;
	}
	len = strlen(text) + (nword * 3) + 1;
	/* Alloc enough space to compose a line */
	composed_line = (char *) malloc(len);
	*composed_line = '\0';
	composed_line_width = 0;	/* In pixels */
	the_word = (char *) malloc(len);

	font_height = pcc->cur_font->ascent + pcc->cur_font->descent;
	baseline = pcc->cur_font->ascent;

	if (!pcc->cw_only && have_space_b4) {
		int do_space = 0;
		int do_underline = 0;

		/* Space before anchor or underlining is separate element */
		if ((pcc->anchor_start && !pcc->in_underlined) ||
		    (pcc->underline_start && !pcc->in_anchor)) {
			do_space = 1;
		} else if (pcc->in_underlined || pcc->in_anchor) {
			do_underline = 1;
		}
		/* Space before strike out is separate element */
		if (pcc->strikeout_start)
			do_space = 1;
		if (do_space) {
			static XFontStruct *prev_font = NULL;
			static space_width;

			if (pcc->cur_font != prev_font) {
				space_width = XTextWidth(pcc->cur_font, " ", 1);
				prev_font = pcc->cur_font;
			}
			word_width = space_width;
			eptr = CreateElement(hw, E_TEXT, pcc->cur_font,
					     pcc->x, pcc->y, word_width,
					     font_height, baseline, pcc);
			Set_E_TEXT_Element(hw, eptr, " ", pcc);
			AdjustBaseLine(eptr, pcc);
			pcc->x += word_width;
			if (!do_underline)
				eptr->underline_number = 0;
			if (pcc->strikeout_start)
				eptr->strikeout = 0;
			have_space_b4 = 0;
		}
	}
	pcc->anchor_start = pcc->underline_start = pcc->strikeout_start = 0;

	for (i = 0; i < nword; i++) {	/* Fill the line with words */
		if (have_space_b4) {
			the_word[0] = ' ';
			the_word[1] = '\0';
		} else {
			the_word[0] = '\0';
		}
		strcat(the_word, words[i]);
		word_width = XTextWidth(pcc->cur_font, the_word,
					strlen(the_word));
		if (pcc->cw_only) {
			if (pcc->computed_min_x < 
			    (word_width + pcc->eoffsetx + pcc->left_margin))
				pcc->computed_min_x = word_width +
					       pcc->eoffsetx + pcc->left_margin;
			if (pcc->nobr &&
			    (pcc->computed_min_x <
			     (word_width + pcc->nobr_x + composed_line_width)))
				pcc->computed_min_x = word_width + pcc->nobr_x +
						      composed_line_width;
		}
		/* Several possible cases */
		if (pcc->is_bol && (word_width > pcc->cur_line_width)) {
			/* The word is larger than the line */
			if (!pcc->cw_only) {
				eptr = CreateElement(hw, E_TEXT, pcc->cur_font,
						    pcc->x, pcc->y, word_width,
						    font_height, baseline, pcc);
				Set_E_TEXT_Element(hw, eptr, the_word, pcc);
				/* Align the elements on their baseline */
				AdjustBaseLine(eptr, pcc);
			} else if (pcc->cur_line_height < font_height) {
                       		pcc->cur_line_height = font_height;
        		}
			pcc->pf_lf_state = 0;
			have_space_b4 = 0;
			pcc->x += word_width;
			LinefeedPlace(hw, pcc);
			composed_line_width = 0;
			continue;
		}
		if (!composed_line_width && !pcc->is_bol &&
		    ((pcc->x - pcc->eoffsetx - pcc->left_margin + word_width) >
		     pcc->cur_line_width)) {
			/* Current position + the word: it's too large
			 * and we haven't composed anything yet!
			 * This happens when there are font tags
			 * to interpret */
			if (!pcc->cw_only) {
				AdjustBaseLine(hw->html.last_formatted_elem,
					       pcc);
			} else if (pcc->cur_line_height < font_height) {
                        	pcc->cur_line_height = font_height;
                        }

			/* Just linefeed */
			pcc->pf_lf_state = 0;
			LinefeedPlace(hw, pcc);
			composed_line_width = 0;
			have_space_b4 = 0;
			i--;
			continue;
		}
		if (!pcc->is_bol && (pcc->x - pcc->eoffsetx - pcc->left_margin +
				     composed_line_width + word_width) > 
		    		    pcc->cur_line_width) {
			/* Current position + the word + the line: */
			/* it's too big, we flush the line */
			if (!pcc->cw_only) {
				eptr = CreateElement(hw, E_TEXT, pcc->cur_font,
                                		    pcc->x, pcc->y,
                                		    composed_line_width,
						    font_height, baseline, pcc);
				Set_E_TEXT_Element(hw, eptr, composed_line,pcc);
				AdjustBaseLine(eptr, pcc); 
			} else if (pcc->cur_line_height < font_height) {
                                pcc->cur_line_height = font_height;
                        } 
			*composed_line = '\0';
                        pcc->pf_lf_state = 0;
			pcc->x += composed_line_width;
			LinefeedPlace(hw, pcc);
			composed_line_width = 0;
                        have_space_b4 = 0;
                        i--;
                        continue;
		}
		/* The word fits */
		strcat(composed_line, the_word);
		have_space_b4 = 1;		/* For the next word */
		composed_line_width += word_width;
		pcc->is_bol = 0;
	}
	if (composed_line_width) {  /* There are things to flush */
		int extra = 0;

		/* Hack to fix Italic font overlap problem */
		if (((pcc->cur_font_family == HELVETICA) ||
		     (pcc->cur_font_family == LUCIDA)) &&
		    ((pcc->cur_font_type == ITALIC_FONT) ||
		     (pcc->cur_font_type == BOLDITALIC_FONT))) {
			if (pcc->cur_font_size < 6) {
				extra = 1;
			} else {
				extra = 2;
			}
		}
		if (!pcc->cw_only) {
			eptr = CreateElement(hw, E_TEXT, pcc->cur_font, 
                          		     pcc->x, pcc->y,
					     composed_line_width + extra,
					     font_height, baseline, pcc);
			Set_E_TEXT_Element(hw, eptr, composed_line, pcc);
			AdjustBaseLine(hw->html.last_formatted_elem, pcc);
		} else if (pcc->cur_line_height < font_height) {
                        pcc->cur_line_height = font_height;
                } 
		pcc->x += composed_line_width + extra;
		if (pcc->cw_only && pcc->nobr)
			pcc->nobr_x += composed_line_width + extra;
		pcc->is_bol = 0;
		pcc->pf_lf_state = 0;
	}
	if (pcc->cw_only && (pcc->x > pcc->computed_max_x))
		pcc->computed_max_x = pcc->x;
	pcc->have_space_after = have_space_after;

	for (i = 0; i < nword; i++)
		FreeMarkText(words[i]);
	free(words);
	free(the_word);
	free(composed_line);
}

/*
 * Place a piece of pre-formatted text.  Add an element record for it.
 */
void PartOfPreTextPlace(HTMLWidget hw, MarkInfo *mptr, PhotoComposeContext *pcc)
{
	ElemInfo *eptr;
	int ntab, i;
	int line_str_len = COMP_LINE_BUF_LEN;
	int line_width = 0;
	int char_cnt = 0;
	int cur_char_in_line = 0;
	int tmp_cnt = 0;
	int font_height = pcc->cur_font->ascent + pcc->cur_font->descent;
	char *end = mptr->text;
        char *line = (char *) malloc(COMP_LINE_BUF_LEN);

	*line = '\0';
	while (*end) {
		if (*end == '\f') {  /* Throw out FF */
		    end++;
		    continue;
		} 
		if (*end == '\r') {  /* Throw out CR unless no LF */
		    if (*++end != '\n')
			*--end = '\n';
		} 
		if (*end == '\n') {
		    if (*line) {
			line_width = XTextWidth(pcc->cur_font, line,
						strlen(line));
			if (pcc->cw_only) {
			    if (pcc->computed_min_x < 
				(line_width + pcc->eoffsetx + pcc->left_margin))
			    	pcc->computed_min_x = line_width +
					       pcc->eoffsetx + pcc->left_margin;
			    if (pcc->cur_line_height < font_height)
                		pcc->cur_line_height = font_height;
			} else {
			    eptr = CreateElement(hw, E_TEXT, pcc->cur_font,
                               			 pcc->x, pcc->y, line_width,
						 font_height,
						 pcc->cur_font->ascent, pcc);
			    Set_E_TEXT_Element(hw, eptr, line, pcc);
			    AdjustBaseLine(eptr, pcc);
                        } 
			pcc->pf_lf_state = 0;
		    }
		    end++;
		    pcc->x += line_width;
		    /* Do a hard linefeed */
		    pcc->preformat = 2;
		    LinefeedPlace(hw, pcc);
		    pcc->preformat = 1;
		    char_cnt = 0;
		    *line = '\0';
		    cur_char_in_line = 0;
		    continue;
		}
		/*
		 * Should be only spaces and tabs here, so if it
		 * is not a tab, make it a space.
		 * Break on linefeeds, they must be done separately
		 */
		if (*end == '\t') {
			tmp_cnt = ((char_cnt / 8) + 1) * 8;
			ntab = tmp_cnt - char_cnt;
			char_cnt += ntab;
			if (char_cnt + 1 > line_str_len) {
				line_str_len += char_cnt + 1;
				line = (char *)realloc(line, line_str_len);
			}
			/*
			 * If we have any tabs, expand them into spaces.
			 */
			for (i = 0; i < ntab; i++)
				line[cur_char_in_line++] = ' ';
			line[cur_char_in_line] = '\0';
			end++;
			continue;
		}
		if (++char_cnt > line_str_len) {
			line_str_len += char_cnt;
			line = (char *)realloc(line, line_str_len);
		}
		line[cur_char_in_line++] = *end++;
		line[cur_char_in_line] = '\0';
	}
	if (*line) {
		line_width = XTextWidth(pcc->cur_font, line, strlen(line));
		if (!pcc->cw_only) {
			eptr = CreateElement(hw, E_TEXT, pcc->cur_font,
               			             pcc->x, pcc->y, line_width,
				             font_height, pcc->cur_font->ascent,
					     pcc);
			Set_E_TEXT_Element(hw, eptr, line, pcc);
			AdjustBaseLine(eptr, pcc);
		} else if (pcc->cur_line_height < font_height) {
                       	pcc->cur_line_height = font_height;
        	}
		pcc->x += line_width;
		pcc->is_bol = 0;
		pcc->pf_lf_state = 0;
	}
	if (pcc->cw_only && (pcc->x > pcc->computed_max_x))
		pcc->computed_max_x = pcc->x;
	pcc->have_space_after = 0;
	free(line);
}

/*
 * Redraw part of a formatted text element, in the passed fg and bg
 */
static void PartialRefresh(HTMLWidget hw, ElemInfo *eptr,
			   int start_pos, int end_pos, unsigned long fg,
			   unsigned long bg, Boolean background)
{
	char *tdata;
	int tlen, width;
	int dir, descent, nascent, ndescent;
	int partial = 0;
	int x = eptr->x;
	int y = eptr->y;
	int height = eptr->height;
	XCharStruct all;
	Display *dsp = hw->html.dsp;
	Window win = XtWindow(hw->html.view);
	GC gc = hw->html.drawGC;

	if (eptr->font != hw->html.cur_font) {
		XSetFont(dsp, gc, eptr->font->fid);
		hw->html.cur_font = eptr->font;
	}
	if (start_pos) {
		XTextExtents(eptr->font, (char *)eptr->edata, start_pos,
			     &dir, &nascent, &descent, &all);
		x += all.width;
		tdata = (char *)(eptr->edata + start_pos);
		partial = 1;
	} else {
		tdata = (char *)eptr->edata;
	}
	if (end_pos != (eptr->edata_len - 2)) {
		tlen = end_pos - start_pos + 1;
		partial = 1;
	} else {
		tlen = eptr->edata_len - start_pos - 1;
	}

	x -= hw->html.scroll_x;
	y -= hw->html.scroll_y;

	/*
	 * May be safe to use the cached full width of this
	 * string, and thus avoid a call to XTextExtents
	 */
	if (!partial && eptr->width) {
		all.width = eptr->width;
	} else {
		XTextExtents(eptr->font, tdata, tlen, &dir, &nascent,
			     &ndescent, &all);
	}
	width = all.width;

	/* Do background if blinked out or background changed (e.g. selected) */
	if (background || eptr->selected) {
		if ((hw->html.cur_bg != bg) || hw->html.table_cell_has_bg ||
		    !hw->html.body_images || !hw->html.bg_image) {
			if (hw->html.cur_fg != bg)
				XSetForeground(dsp, gc, bg);
			XFillRectangle(dsp, win, gc, x, y, width, height);
			XSetForeground(dsp, gc, fg);
			if (hw->html.cur_fg != fg)
				hw->html.cur_fg = fg;
		} else {
			if (hw->html.cur_fg != fg) {
				XSetForeground(dsp, gc, fg);
				hw->html.cur_fg = fg;
			}
			HTMLDrawBackgroundImage((Widget)hw,
						x < 0 ? 0 : x, y < 0 ? 0 : y,
						x < 0 ? (width + x) : width,
						y < 0 ? (height + y) : height);
		}
		if (eptr->blink)
			return;
	} else if (hw->html.cur_fg != fg) {
		XSetForeground(dsp, gc, fg);
		hw->html.cur_fg = fg;
	}

	XDrawString(dsp, win, gc, x, y + eptr->baseline, tdata, tlen);

	if (eptr->underline_number) {
		int i, ly, line_style;

		if (eptr->dashed_underline) {
			line_style = LineOnOffDash; 
		} else {
			line_style = LineSolid;
		}
		XSetLineAttributes(dsp, gc, 1, line_style, CapButt, JoinBevel);

		/* Compute it first time refreshed */
		if (eptr->underline_yoffset == -1) {
			if (hw->html.underline_yoffset != -1) {
				ly = hw->html.underline_yoffset;
			} else {
				ly = hw->html.underline_yoffset =
						(int) (eptr->font->descent / 2);
			}
			eptr->underline_yoffset = ly;
		} else {
			ly = eptr->underline_yoffset;
		}
		ly += y + eptr->baseline;
		for (i = 0; i < eptr->underline_number; i++) {
			XDrawLine(dsp, win, gc, x, ly, (int)(x + width), ly);
			ly -= 2;
		}
	}
	if (eptr->strikeout) {
		int ly = (int)(y + eptr->height / 2);

		XSetLineAttributes(dsp, gc, 1, LineSolid, CapButt, JoinBevel);
		XDrawLine(dsp, win, gc, x, ly, (int)(x + width), ly);
	}
}

/* Redraw a formatted text element */
void TextRefresh(HTMLWidget hw, ElemInfo *eptr, int start_pos, int end_pos,
		 Boolean background)
{
	if (!eptr->selected) {
		PartialRefresh(hw, eptr, start_pos, end_pos,
			       eptr->fg, eptr->bg, background);
		return;
	}
	if ((start_pos >= eptr->start_pos) && (end_pos <= eptr->end_pos)) {
		PartialRefresh(hw, eptr, start_pos, end_pos,
			       eptr->bg, eptr->fg, background);
		return;
	}
	if (start_pos < eptr->start_pos) {
		PartialRefresh(hw, eptr, start_pos, eptr->start_pos - 1,
			       eptr->fg, eptr->bg, background);
		start_pos = eptr->start_pos;
	}
	if (end_pos > eptr->end_pos) {
		PartialRefresh(hw, eptr, eptr->end_pos + 1, end_pos,
			       eptr->fg, eptr->bg, background);
		end_pos = eptr->end_pos;
	}
	PartialRefresh(hw, eptr, start_pos, end_pos,
		       eptr->bg, eptr->fg, background);
}

/* Place a horizontal rule across the page.
 * Create and add the element record for it.
 */
void HRulePlace(HTMLWidget hw, MarkInfo *mptr, PhotoComposeContext *pcc)
{
	char *tptr;
	int width, top;
	int size = 1;
	int shade = 1;
	int adjx = 0;
	DivAlignType alignment;

	if (tptr = ParseMarkTag(mptr->start, MT_HRULE, "WIDTH")) {
		width = atoi(tptr);	/* Width wanted by user */
		if (strchr(tptr, '%'))
			width = (width * pcc->cur_line_width) / 100;
		free(tptr);
	} else {
		width = pcc->cur_line_width;
	}
	if (tptr = ParseMarkTag(mptr->start, MT_HRULE, "NOSHADE")) {
		shade = 0;
		free(tptr);
	}
	if (tptr = ParseMarkTag(mptr->start, MT_HRULE, "SIZE")) {
		size = atoi(tptr);	/* Size wanted by user */
		if (size < 1)
			size = 1;
		free(tptr);
	}
	if (pcc->div == DIV_ALIGN_NONE) {
		alignment = DIV_ALIGN_CENTER;
	} else {
		alignment = pcc->div;
	}
	if (tptr = ParseMarkTag(mptr->start, MT_HRULE, "ALIGN")) {
		if (caseless_equal(tptr, "LEFT")) {
			alignment = DIV_ALIGN_LEFT;
		} else if (caseless_equal(tptr, "CENTER")) {
			alignment = DIV_ALIGN_CENTER;
		} else if (caseless_equal(tptr, "RIGHT")) {
			alignment = DIV_ALIGN_RIGHT;
		}
		free(tptr);
	}
	if ((alignment == DIV_ALIGN_CENTER) || (alignment == DIV_ALIGN_RIGHT)) {
		adjx = pcc->cur_line_width - width;
		if (alignment == DIV_ALIGN_CENTER)
			adjx = adjx / 2;
	}
        pcc->x = pcc->eoffsetx + pcc->left_margin + adjx;

	top = ((pcc->cur_font->ascent + pcc->cur_font->descent) / 2) - 1;
	if (!pcc->cw_only) {
		ElemInfo *eptr;

		eptr = CreateElement(hw, E_HRULE, pcc->cur_font, pcc->x,
				     pcc->y + top, width, size, size, pcc);
		eptr->underline_number = 0;  /* Rules can't be underlined! */
		eptr->bwidth = shade;
	} else {
		if (pcc->computed_min_x <
		    (1 + pcc->eoffsetx + pcc->left_margin))
			pcc->computed_min_x = 1 + pcc->eoffsetx +
					      pcc->left_margin;
		if (pcc->computed_min_x > pcc->computed_max_x)
			pcc->computed_max_x = pcc->computed_min_x;
	}
	pcc->cur_line_height = top + size + pcc->cur_font->descent + 1;
	pcc->pf_lf_state = 0;
}

#define shadowpm_width 2
#define shadowpm_height 2
/*
 * Redraw a formatted horizontal rule element
 */
void HRuleRefresh(HTMLWidget hw, ElemInfo *eptr)
{
	static GC NoShadeGC = NULL;
	int width = eptr->width;
	int height = eptr->height;
	int x1 = eptr->x;
	int y1 = eptr->y;
	Display *dsp = hw->html.dsp;
	Window win = XtWindow(hw->html.view);

	if (width < 0)
		width = 0;
	x1 -= hw->html.scroll_x;
	y1 -= hw->html.scroll_y;

	if (!eptr->bwidth && !NoShadeGC) {
		static char shadowpm_bits[] = { 0x02, 0x01 };
		unsigned long valuemask = GCFillStyle | GCStipple;
		XGCValues values;

		values.stipple = XCreateBitmapFromData(dsp,
					       RootWindowOfScreen(XtScreen(hw)),
					       shadowpm_bits,
					       shadowpm_width, shadowpm_height);
		values.fill_style = FillStippled;
		NoShadeGC = XCreateGC(dsp, RootWindow(dsp, DefaultScreen(dsp)),
				      valuemask, &values);
	}
	if (eptr->bwidth) {
		/* Draw shadowed line */
		if (height > 2) {
			if (hw->html.cur_fg != eptr->bg) {
				XSetForeground(dsp, hw->html.drawGC, eptr->bg);
				hw->html.cur_fg = eptr->bg;
			}
			/* Blank out area */
			XFillRectangle(dsp, win, hw->html.drawGC, x1, y1,
				       width, height - 2);
		} else {
			height = 2;
		}
		XDrawLine(dsp, win, hw->manager.bottom_shadow_GC,
			  x1, y1 - 1, (int)(x1 + width), y1 - 1);
		XDrawLine(dsp, win, hw->manager.bottom_shadow_GC,
			  x1, y1 - 1, x1, y1 + height - 2);
		XDrawLine(dsp, win, hw->manager.top_shadow_GC,
			  x1, y1 + height - 2, (int)(x1 + width),
			  y1 + height - 2);
		XDrawLine(dsp, win, hw->manager.top_shadow_GC,
			  (int)(x1 + width), y1 - 1, (int)(x1 + width),
			  y1 + height - 2);
	} else {
		/* Draw solid line */
		XSetForeground(dsp, NoShadeGC, eptr->fg);
		XSetLineAttributes(dsp, NoShadeGC, height,
				   LineSolid, CapButt, JoinBevel);
		XDrawLine(dsp, win, NoShadeGC, x1, y1 + (height / 2),
			  (int)(x1 + width), y1 + (height / 2));
	}
}

/* Place a bullet at the beginning of an unnumbered
 * list item or in the middle of text if not in a list. 
 * Create and add the element record for it.
 */
void BulletPlace(HTMLWidget hw, PhotoComposeContext *pcc, int list)
{
	int width = pcc->cur_font->max_bounds.width / 2;
	int x = pcc->x;
	int swidth;

	if (!list) {
		swidth = (pcc->cur_font->max_bounds.rbearing -
			  pcc->cur_font->max_bounds.lbearing) / 2;
		pcc->x += width + pcc->cur_font->max_bounds.lbearing;
		if (pcc->have_space_after) {
			/* Space in front of it */
			pcc->x += swidth;
			x += swidth;
		}
	} else {
		/* Out in front of current location */
		x -= pcc->cur_font->max_bounds.width;
		pcc->is_bol = 2;
        }
	if (!pcc->cw_only) {
		ElemInfo *eptr= CreateElement(hw, E_BULLET, pcc->cur_font, 
			         x, pcc->y, width, 
			         pcc->cur_font->ascent + pcc->cur_font->descent,
			         pcc->cur_font->ascent, pcc);

		eptr->underline_number = 0;  /* Bullets can't be underlined! */
		AdjustBaseLine(eptr, pcc);
		if (!list)
			eptr->indent_level = 1;
	} else {
		if (!list && pcc->have_space_after)
			width += pcc->cur_font->max_bounds.lbearing + swidth;
		if (pcc->computed_min_x <
		    (width + pcc->eoffsetx + pcc->left_margin))
			pcc->computed_min_x = width + pcc->eoffsetx +
					      pcc->left_margin;
		if (pcc->x > pcc->computed_max_x)
			pcc->computed_max_x = pcc->x;
	}
        pcc->have_space_after = 0;
}

/* Redraw a formatted bullet element */
void BulletRefresh(HTMLWidget hw, ElemInfo *eptr)
{
	int width = eptr->width;
	int x1 = eptr->x;
	int y1 = eptr->y;
	Window win = XtWindow(hw->html.view);
	Display *dsp = hw->html.dsp;
	GC gc = hw->html.drawGC;

	x1 -= hw->html.scroll_x;
	y1 += eptr->baseline - width - (eptr->font->descent / 2);
	y1 -= hw->html.scroll_y;

	if (eptr->font != hw->html.cur_font) {
		XSetFont(dsp, gc, eptr->font->fid);
		hw->html.cur_font = eptr->font;
	}
	if (hw->html.cur_fg != eptr->fg) {
		XSetForeground(dsp, gc, eptr->fg);
		hw->html.cur_fg = eptr->fg;
	}
	if (hw->html.cur_bg != eptr->bg) {
		XSetBackground(dsp, gc, eptr->bg);
		hw->html.cur_bg = eptr->bg;
	}
	/*
	 * Rewritten to alternate circle, square and those pairs alternate
	 * filled and not filled.
	 */
	XSetLineAttributes(dsp, gc, 1, LineSolid, CapButt, JoinBevel);
	if (eptr->indent_level && (eptr->indent_level % 2)) {  /* Odd & !0 */
		if (eptr->indent_level == 1 || (eptr->indent_level % 4) == 1) {
			XFillArc(dsp, win, gc, x1, y1, width, width, 0, 23040);
		} else {
			XDrawArc(dsp, win, gc, x1, y1, width, width, 0, 23040);
		}
	} else {  /* Even */
		if (eptr->indent_level == 0 || (eptr->indent_level % 4) == 2) {
			XFillRectangle(dsp, win, gc, x1, y1, width, width);
		} else {
			XDrawRectangle(dsp, win, gc, x1, y1, width, width);
		}
	}
}

/****** Next four routines borrowed from Lynx *******/

/*
** This function returns OL TYPE="A" strings in
** the range of " A." (1) to "ZZZ." (18278). - FM
*/
static char *UppercaseA_OL_String(int seqnum)
{
    static char OLstring[8];

    if (seqnum <= 1) {
	strcpy(OLstring, " A.");
    } else if (seqnum < 27) {
	sprintf(OLstring, " %c.", seqnum + 64);
    } else if (seqnum < 703) {
	sprintf(OLstring, "%c%c.", (seqnum - 1) / 26 + 64,
		seqnum - ((seqnum - 1) / 26) * 26 + 64);
    } else if (seqnum < 18279) {
	sprintf(OLstring, "%c%c%c.", (seqnum - 27) / 676 + 64,
		((seqnum - ((seqnum - 27) / 676) * 676) - 1) / 26 + 64,
		seqnum - ((seqnum - 1) / 26) * 26 + 64);
    } else {
    	strcpy(OLstring, "ZZZ.");
    }
    return OLstring;
}

/*
** This function returns OL TYPE="a" strings in
** the range of " a." (1) to "zzz." (18278). - FM
*/
static char *LowercaseA_OL_String(int seqnum)
{
    static char OLstring[8];

    if (seqnum <= 1) {
	strcpy(OLstring, " a.");
    } else if (seqnum < 27) {
	sprintf(OLstring, " %c.", seqnum + 96);
    } else if (seqnum < 703) {
	sprintf(OLstring, "%c%c.", (seqnum - 1) / 26 + 96,
		seqnum - ((seqnum - 1) / 26) * 26 + 96);
    } else if (seqnum < 18279) {
	sprintf(OLstring, "%c%c%c.", (seqnum - 27) / 676 + 96,
		((seqnum - ((seqnum - 27) / 676) * 676) - 1) / 26 + 96,
		seqnum - ((seqnum - 1) / 26) * 26 + 96);
    } else {
	strcpy(OLstring, "zzz.");
    }
    return OLstring;
}

/*
** This function returns OL TYPE="I" strings in the
** range of " I." (1) to "MMM." (3000).- FM
*/
static char *UppercaseI_OL_String(int seqnum)
{
    static char OLstring[8];
    int Arabic = seqnum;

    if (Arabic >= 3000) {
	strcpy(OLstring, "MMM.");
	return OLstring;
    }

    OLstring[0] = '\0';

    while (Arabic >= 1000) {
	strcat(OLstring, "M");
	Arabic -= 1000;
    }
    if (Arabic >= 900) {
	strcat(OLstring, "CM");
	Arabic -= 900;
    }
    if (Arabic >= 500) {
	strcat(OLstring, "D");
	Arabic -= 500;
	while (Arabic >= 500) {
	    strcat(OLstring, "C");
	    Arabic -= 10;
	}
    }
    if (Arabic >= 400) {
	strcat(OLstring, "CD");
	Arabic -= 400;
    }
    while (Arabic >= 100) {
	strcat(OLstring, "C");
	Arabic -= 100;
    }
    if (Arabic >= 90) {
	strcat(OLstring, "XC");
	Arabic -= 90;
    }
    if (Arabic >= 50) {
	strcat(OLstring, "L");
	Arabic -= 50;
	while (Arabic >= 50) {
	    strcat(OLstring, "X");
	    Arabic -= 10;
	}
    }
    if (Arabic >= 40) {
	strcat(OLstring, "XL");
	Arabic -= 40;
    }
    while (Arabic > 10) {
	strcat(OLstring, "X");
	Arabic -= 10;
    }

    switch (Arabic) {
      case 1:
	strcat(OLstring, "I.");
	break;
      case 2:
	strcat(OLstring, "II.");
	break;
      case 3:
	strcat(OLstring, "III.");
	break;
      case 4:
	strcat(OLstring, "IV.");
	break;
      case 5:
	strcat(OLstring, "V.");
	break;
      case 6:
	strcat(OLstring, "VI.");
	break;
      case 7:
	strcat(OLstring, "VII.");
	break;
      case 8:
	strcat(OLstring, "VIII.");
	break;
      case 9:
	strcat(OLstring, "IX.");
	break;
      case 10:
	strcat(OLstring, "X.");
	break;
      default:
	strcat(OLstring, ".");
	break;
    }
    return OLstring;
}

/*
** This function returns OL TYPE="i" strings in
** range of " i." (1) to "mmm." (3000).- FM
*/
static char *LowercaseI_OL_String(int seqnum)
{
    static char OLstring[8];
    int Arabic = seqnum;

    if (Arabic >= 3000) {
	strcpy(OLstring, "mmm.");
	return OLstring;
    }

    OLstring[0] = '\0';

    while (Arabic >= 1000) {
	strcat(OLstring, "m");
	Arabic -= 1000;
    }
    if (Arabic >= 900) {
	strcat(OLstring, "cm");
	Arabic -= 900;
    }
    if (Arabic >= 500) {
	strcat(OLstring, "d");
	Arabic -= 500;
	while (Arabic >= 500) {
	    strcat(OLstring, "c");
	    Arabic -= 10;
	}
    }
    if (Arabic >= 400) {
	strcat(OLstring, "cd");
	Arabic -= 400;
    }
    while (Arabic >= 100) {
	strcat(OLstring, "c");
	Arabic -= 100;
    }
    if (Arabic >= 90) {
	strcat(OLstring, "xc");
	Arabic -= 90;
    }
    if (Arabic >= 50) {
	strcat(OLstring, "l");
	Arabic -= 50;
	while (Arabic >= 50) {
	    strcat(OLstring, "x");
	    Arabic -= 10;
	}
    }
    if (Arabic >= 40) {
	strcat(OLstring, "xl");
	Arabic -= 40;
    }
    while (Arabic > 10) {
	strcat(OLstring, "x");
	Arabic -= 10;
    }

    switch (Arabic) {
      case 1:
	strcat(OLstring, "i.");
	break;
      case 2:
	strcat(OLstring, "ii.");
	break;
      case 3:
	strcat(OLstring, "iii.");
	break;
      case 4:
	strcat(OLstring, "iv.");
	break;
      case 5:
	strcat(OLstring, "v.");
	break;
      case 6:
	strcat(OLstring, "vi.");
	break;
      case 7:
	strcat(OLstring, "vii.");
	break;
      case 8:
	strcat(OLstring, "viii.");
	break;
      case 9:
	strcat(OLstring, "ix.");
	break;
      case 10:
	strcat(OLstring, "x.");
	break;
      default:
	strcat(OLstring, ".");
	break;
    }
    return OLstring;
}

/* Place the number at the beginning of a numbered
 * list item.  Create and add the element record for it.
 */
void ListNumberPlace(HTMLWidget hw, PhotoComposeContext *pcc, int val,
		     char type)
{
	XCharStruct all;
	char buf[20];
	int x = pcc->x;
	int font_height = pcc->cur_font->ascent + pcc->cur_font->descent;
	int width, dir, ascent, descent;

	if (type == 'A') {
		sprintf(buf, UppercaseA_OL_String(val));
	} else if (type == 'a') {
		sprintf(buf, LowercaseA_OL_String(val));
	} else if (type == 'I') {
		sprintf(buf, UppercaseI_OL_String(val));
	} else if (type == 'i') {
		sprintf(buf, LowercaseI_OL_String(val));
	} else {
		sprintf(buf, "%d.", val);
	}
	XTextExtents(pcc->cur_font, buf, strlen(buf), &dir,
		     &ascent, &descent, &all);
	width = all.width;
	if (width < (D_INDENT_SPACES + (D_INDENT_SPACES / 4))) {
		x -= width + ((D_INDENT_SPACES / 4) - 2);
	} else {
		x -= width;
	}

	if (!pcc->cw_only) {
		ElemInfo *eptr;

		eptr = CreateElement(hw, E_TEXT, pcc->cur_font, x, pcc->y,
				     width, font_height,
				     pcc->cur_font->ascent, pcc);
        	Set_E_TEXT_Element(hw, eptr, buf, pcc);
		AdjustBaseLine(eptr, pcc);
	} else if (pcc->cur_line_height < font_height) {
                pcc->cur_line_height = font_height;
        }
	pcc->have_space_after = 0;
	pcc->is_bol = 2;
}
