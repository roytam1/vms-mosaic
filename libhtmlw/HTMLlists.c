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

#include <stdio.h>
#include <stdlib.h>

#include "HTMLparse.h"
#include "HTMLP.h"
#include "HTMLPutil.h"
#include "HTMLmiscdefs.h"

#include "../src/prefs.h"

#ifndef DISABLE_TRACE
extern int htmlwTrace;
extern int reportBugs;
#endif

typedef struct text_rec {
        struct text_rec *next;
} TextBlock;

static ElemInfo *elem_start;
static MarkInfo *mark_start;
static ImageInfo *image_start = NULL;
static TextBlock *textA_start = NULL;
static TextBlock *textB_start = NULL;
static TextBlock *textC_start = NULL;
static TextBlock *textD_start = NULL;
static TextBlock *textE_start = NULL;
static TextBlock *textF_start = NULL;
static int count_big = 0;

/* Free up the passed linked list of parsed HTML objects, freeing
 * all memory associated with each object.
 */
void FreeMarkUpList(MarkInfo *List)
{
	MarkInfo *current = List;
	MarkInfo *mptr;

	while (current) {
		mptr = current;
		current = current->next;
		if (mptr->start)
			FreeMarkText(mptr->start);
		if (mptr->text)
			FreeMarkText(mptr->text);

		/* Any tag can have one */
		if (mptr->anc_name)
			free(mptr->anc_name);

		if (!mptr->is_end && (mptr->type == M_ANCHOR)) {
			if (mptr->anc_href)
				FreeMarkText(mptr->anc_href);
			if (mptr->anc_title)
				free(mptr->anc_title);
			if (mptr->anc_target)
				free(mptr->anc_target);
		}
		if (mptr->s_ats)	/* Applet */
			_FreeAppletStruct(mptr->s_ats);
		if (mptr->t_p1)		/* Table */
			_FreeTableStruct(mptr->t_p1);

		/* Place back on free list */
		mptr->next = mark_start;
		mark_start = mptr;
	}
}

/* Free up the passed linked list of parsed HTML usemaps, freeing
 * all memory associated with each map.
 */
void FreeMapList(MapInfo *map)
{
	MapInfo *nextmap;
	AreaInfo *area, *nextarea;
	CoordInfo *coords, *nextcoords;

	while (map) {
		nextmap = map->next;
		area = map->areaList;
		while (area) {
			nextarea = area->next;
			coords = area->coordList;			
			while (coords) {
				nextcoords = coords->next;
				free(coords);
				coords = nextcoords;
			}
			if (area->href)
				free(area->href);
			if (area->alt)
				free(area->alt);
			free(area);
			area = nextarea;
		}
		if (map->name)
			free(map->name);
		free(map);
		map = nextmap;
	}
}

/* Free image data */
void FreeImageInfo(ImageInfo *picd, HTMLWidget hw)
{
	Display *dsp;

	/* Will be NULL when deleting cached data */
	if (hw)
		dsp = hw->html.dsp;
        /*
         * Don't free internal image pixmaps or data
         */
	if (picd->image && picd->fetched) {
		XFreePixmap(dsp, picd->image);
		if (picd->clip)
			XFreePixmap(dsp, picd->clip);
		if (picd->ori_colrs)
			free(picd->ori_colrs);
	}
	/* Free any animation ImageInfo and Pixmaps */
	if (picd->anim_info && picd->next) {
		ImageInfo *next = picd->next;
		ImageInfo *tmp;

		while (next) {
			tmp = next;
			next = tmp->next;
			if (tmp->image)
				XFreePixmap(dsp, tmp->image);
			if (tmp->clip)
				XFreePixmap(dsp, tmp->clip);
			/* Free rescaled animation data */
			if (picd->fetched && !picd->cached) {
				if (tmp->image_data)
					free(tmp->image_data);
				if (tmp->clip_data)
					free(tmp->clip_data);
				if (tmp->alpha)
					free(tmp->alpha);
			}
			/* Put on free list */
			tmp->next = image_start;
			image_start = tmp;
		}
		if (picd->has_anim_image && picd->anim_image)
			XFreePixmap(dsp, picd->anim_image);
		if (picd->has_anim_image && picd->bg_image)
			XFreePixmap(dsp, picd->bg_image);
		if (picd->timer)
			XtRemoveTimeOut(picd->timer);
	}
	/* Background image with alpha channel */
	if (picd->is_bg_image && picd->alpha && picd->alpha_image_data)
		free(picd->alpha_image_data);

	/* Private copy if it was rescaled */
	if (picd->fetched && !picd->cached) {
		if (picd->image_data)
			free(picd->image_data);
		if (picd->clip_data)
			free(picd->clip_data);
		if (picd->alpha)
			free(picd->alpha);
	}
	/* Internal images can have these */
	if (picd->src)
		free(picd->src);
	if (picd->text)
		free(picd->text);
	if (picd->alt_text)
		free(picd->alt_text);
	if (picd->usemap)
		free(picd->usemap);

	/* Place back on free list */
	picd->next = image_start;
	image_start = picd;
}

/* Free up the passed linked list of formatted elements, freeing
 * all memory associated with each element.
 */
void FreeLineList(ElemInfo *list, HTMLWidget hw)
{
	ElemInfo *current = list;
	ElemInfo *eptr;
	int iframe_done = 0;

	while (current) {
		eptr = current;
		current = current->next;
		if (eptr->edata)
			FreeMarkText(eptr->edata);
		if (eptr->label_id)
			free(eptr->label_id);
		if (eptr->title)
			free(eptr->title);
		if (eptr->type == E_IFRAME) {
			/* Delete all of them if not already done. */
			if (!iframe_done) {
				FrameCbData cbs;

				cbs.reason = FRAME_DELETE;
				XtCallCallbackList((Widget)hw,
					         hw->html.frame_callback, &cbs);
				iframe_done = 1;
			}
			if (eptr->frame) {
				if (eptr->frame->frame_src)
					free(eptr->frame->frame_src);
				if (eptr->frame->frame_name)
					free(eptr->frame->frame_name);
				free(eptr->frame);
			}
		}
		if ((eptr->type == E_IMAGE) && eptr->pic_data)
			FreeImageInfo(eptr->pic_data, hw);

		/* Place element back on preallocated list */
		eptr->next = elem_start;
		elem_start = eptr;
	}
}

/* Free text block */
void FreeMarkText(char *text)
{
	text--;
	switch (*text) {
	    case '0':
		/* Actually free oversize ones */
		free(text);
		count_big--;
		break;
	    case 'A':
		((TextBlock *)text)->next = textA_start;
		textA_start = (TextBlock *)text;
		break;
	    case 'B':
		((TextBlock *)text)->next = textB_start;
		textB_start = (TextBlock *)text;
		break;
	    case 'C':
		((TextBlock *)text)->next = textC_start;
		textC_start = (TextBlock *)text;
		break;
	    case 'D':
		((TextBlock *)text)->next = textD_start;
		textD_start = (TextBlock *)text;
		break;
	    case 'E':
		((TextBlock *)text)->next = textE_start;
		textE_start = (TextBlock *)text;
		break;
	    case 'F':
		((TextBlock *)text)->next = textF_start;
		textF_start = (TextBlock *)text;
		break;
	    default:
		fprintf(stderr, "Free of bad text block\n");
	}
}

/* Handles allocation of Element memory */
ElemInfo *GetElemRec()
{
	ElemInfo *next;
	static int init = 0;
	static int ecount;

	/* Preallocate some elements */
	if (!init) {
		int num = get_pref_int(eELEMENT_MEMORY_PREALLOCATION);
		int i;

		if (num < 1)
			num = 1;
		next = elem_start = (ElemInfo *) malloc(num * sizeof(ElemInfo) *
							sizeof(char));
		CHECK_OUT_OF_MEM(next);
		ecount = num;
		for (i = 1; i < num; i++) {
			next->next = next + 1;
			next = next->next;
		}
		next->next = NULL;
		init = 1;
	}
	if (!elem_start) {
		int i;

		/* Preallocated list was empty, so allo another block */
		next = elem_start = (ElemInfo *) malloc(512 * sizeof(ElemInfo) *
							sizeof(char));
		CHECK_OUT_OF_MEM(next);
		ecount += 512;
		for (i = 1; i < 512; i++) {
			next->next = next + 1;
			next = next->next;
		}
		next->next = NULL;
#ifndef DISABLE_TRACE
                if (htmlwTrace || reportBugs)
                       fprintf(stderr,
			       "Allocated 512 elements.  Total = %d\n", ecount);
#endif
	}

	/* Return top of preallocated list */
	next = elem_start;
	elem_start = next->next;
	/* Zero it */
	memset(next, 0, sizeof(ElemInfo));
	return next;
}

/* Handles allocation of Markup memory */
MarkInfo *GetMarkRec()
{
	MarkInfo *next;
	static int init = 0;
	static int mcount;

	/* Preallocate some */
	if (!init) {
		int num = get_pref_int(eMARKUP_MEMORY_PREALLOCATION);
		int i;

		if (num < 1)
			num = 1;
		next = mark_start = (MarkInfo *) malloc(num * sizeof(MarkInfo) *
							sizeof(char));
		CHECK_OUT_OF_MEM(next);
		mcount = num;
		for (i = 1; i < num; i++) {
			next->next = next + 1;
			next = next->next;
		}
		next->next = NULL;
		init = 1;
	}
	if (!mark_start) {
		int i;

		/* Preallocated list was empty, so allo another block */
		next = mark_start = (MarkInfo *) malloc(512 * sizeof(MarkInfo) *
							sizeof(char));
		CHECK_OUT_OF_MEM(next);
		mcount += 512;
		for (i = 1; i < 512; i++) {
			next->next = next + 1;
			next = next->next;
		}
		next->next = NULL;
#ifndef DISABLE_TRACE
                if (htmlwTrace || reportBugs)
                       fprintf(stderr,
			       "Allocated 512 marks.  Total = %d\n", mcount);
#endif
	}

	/* Return top of preallocated list */
	next = mark_start;
	mark_start = next->next;
	memset(next, 0, sizeof(MarkInfo));
	return next;
}

void FreeMarkRec(MarkInfo *mark)
{
	/* Place back on free list */
	mark->next = mark_start;
	mark_start = mark;
}

/* Handles allocation of ImageInfo memory */
ImageInfo *GetImageRec()
{
	ImageInfo *next;
	static int icount = 0;

	if (!image_start) {
		int i;

		/* Allocate a block */
		next = image_start = (ImageInfo *) malloc(32 *
							  sizeof(ImageInfo) *
							  sizeof(char));
		CHECK_OUT_OF_MEM(next);
		icount += 32;
		for (i = 1; i < 32; i++) {
			next->next = next + 1;
			next = next->next;
		}
		next->next = NULL;
#ifndef DISABLE_TRACE
                if (htmlwTrace || reportBugs)
                       fprintf(stderr,
			       "Allocated 32 ImageInfo.  Total = %d\n", icount);
#endif
	}

	/* Return top of preallocated list */
	next = image_start;
	image_start = next->next;
	memset(next, 0, sizeof(ImageInfo));
	return next;
}

/* Allocate a block of text memory */
static TextBlock *AlloBlock(int num, int size)
{
	TextBlock *text = (TextBlock *)malloc(num * size * sizeof(char));
	TextBlock *next;
	char *block;
	int i;

	CHECK_OUT_OF_MEM(text);
	next = text;
	for (i = 1; i < num; i++) {
		block = (char *)next;
		block += size;
		next->next = (TextBlock *)block;
		next = next->next;
	}
	next->next = NULL;
	return text;
}

/* Handles allocation of Markup text memory */
char *GetMarkText(char *text)
{
	TextBlock *next;
	char *block;
	int len;
	static int Acount = 0;
	static int Bcount = 0;
	static int Ccount = 0;
	static int Dcount = 0;
	static int Ecount = 0;
	static int Fcount = 0;

	if (!text)
		return NULL;

	len = strlen(text);

	if (len < 7) {
		if (!textA_start) {
			textA_start = AlloBlock(512, 8);
			Acount += 512;
#ifndef DISABLE_TRACE
	                if (htmlwTrace || reportBugs)
				fprintf(stderr,
			       		"Allocated 512 AText.  Total = %d\n",
					Acount);
#endif
		}
		/* Return top of A list */
		next = textA_start;
		textA_start = next->next;
		block = (char *)next;
		*block++ = 'A';
		strcpy(block, text);
	} else if (len < 15) {
		if (!textB_start) {
			textB_start = AlloBlock(128, 16);
			Bcount += 128;
#ifndef DISABLE_TRACE
	                if (htmlwTrace || reportBugs)
				fprintf(stderr,
			       		"Allocated 128 BText.  Total = %d\n",
					Bcount);
#endif
		}
		/* Return top of B list */
		next = textB_start;
		textB_start = next->next;
		block = (char *)next;
		*block++ = 'B';
		strcpy(block, text);
	} else if (len < 31) {
		if (!textC_start) {
			textC_start = AlloBlock(128, 32);
			Ccount += 128;
#ifndef DISABLE_TRACE
	                if (htmlwTrace || reportBugs)
				fprintf(stderr,
			       		"Allocated 128 CText.  Total = %d\n",
					Ccount);
#endif
		}
		/* Return top of C list */
		next = textC_start;
		textC_start = next->next;
		block = (char *)next;
		*block++ = 'C';
		strcpy(block, text);
	} else if (len < 63) {
		if (!textD_start) {
			textD_start = AlloBlock(256, 64);
			Dcount += 256;
#ifndef DISABLE_TRACE
	                if (htmlwTrace || reportBugs)
				fprintf(stderr,
			       		"Allocated 256 DText.  Total = %d\n",
					Dcount);
#endif
		}
		/* Return top of D list */
		next = textD_start;
		textD_start = next->next;
		block = (char *)next;
		*block++ = 'D';
		strcpy(block, text);
	} else if (len < 127) {
		if (!textE_start) {
			textE_start = AlloBlock(128, 128);
			Ecount += 128;
#ifndef DISABLE_TRACE
	                if (htmlwTrace || reportBugs)
				fprintf(stderr,
			       		"Allocated 128 EText.  Total = %d\n",
					Ecount);
#endif
		}
		/* Return top of E list */
		next = textE_start;
		textE_start = next->next;
		block = (char *)next;
		*block++ = 'E';
		strcpy(block, text);
	} else if (len < 255) {
		if (!textF_start) {
			textF_start = AlloBlock(64, 256);
			Fcount += 64;
#ifndef DISABLE_TRACE
	                if (htmlwTrace || reportBugs)
				fprintf(stderr,
			       		"Allocated 64 FText.  Total = %d\n",
					Fcount);
#endif
		}
		/* Return top of F list */
		next = textF_start;
		textF_start = next->next;
		block = (char *)next;
		*block++ = 'F';
		strcpy(block, text);
	} else {
		static int lcount = 64;

		block = malloc(len + 2);
		*block++ = '0';
		strcpy(block, text);
		count_big++;
#ifndef DISABLE_TRACE
                if ((htmlwTrace || reportBugs) && (count_big == lcount)) {
			fprintf(stderr, "Big text number = %d\n", count_big);
			lcount += 64;
		}
#endif
	}
	return block;
}

/*
 * Passed in 2 element pointers, and element positions.
 * Function should return 1 if start occurs before end.
 * Otherwise return 0.
 */
int ElementLessThan(ElemInfo *start, ElemInfo *end, int start_pos, int end_pos)
{
	/*
	 * Deal with start or end being NULL
	 */
	if ((!start && !end) || (start && !end))
		return(0);
	if (!start && end)
		return(1);
	/*
	 * Deal with easy identical case
	 */
	if (start == end) {
		if (start_pos < end_pos)
			return(1);
		return(0);
	}
	/* We know element Ids are always equal or increasing within a list. */
	if (start->ele_id < end->ele_id)
		return(1);
	if (start->ele_id == end->ele_id) {
		ElemInfo *current = start;

		while (current) {
			if (current->ele_id != start->ele_id)
				break;
			if (current == end)
				break;
			current = current->next;
		}
		if (current == end)
			return(1);
	}
	return(0);
}

/*
 * Passed in 2 element pointers, and element positions.
 * Function should return 1 if they need to be swapped in order for then
 * to proceed left to right and top to bottom in the text.
 * Otherwise return 0.
 */
int SwapElements(ElemInfo *start, ElemInfo *end, int start_pos, int end_pos)
{
	/* Deal with start or end being NULL */
	if ((!start && !end) || (start && !end))
		return(0);
	if (!start && end)
		return(1);
	/* Deal with easy identical case */
	if (start == end) {
		if (start_pos > end_pos)
			return(1);
		return(0);
	}

	/* We know element Ids are always equal or increasing within a list. */
	if (start->ele_id < end->ele_id)
		return(0);
	if (start->ele_id == end->ele_id) {
		ElemInfo *current = start;

		while (current) {
			if (current->ele_id != start->ele_id)
				break;
			if (current == end)
				break;
			current = current->next;
		}
		if (current == end)
			return(0);
	}
	return(1);
}
