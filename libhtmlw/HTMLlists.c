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

/* Copyright (C) 1998, 1999, 2000, 2004, 2005, 2006 - the VMS Mosaic Project */

#include "../config.h"

#include <stdio.h>
#include <stdlib.h>

#include "HTMLparse.h"
#include "HTMLP.h"
#include "HTMLPutil.h"
#include "HTMLmiscdefs.h"

#include "../src/prefs.h"

static ElemInfo *elem_start;
static MarkInfo *mark_start;

/* Free up the passed linked list of parsed HTML objects, freeing
 * all memory associated with each object.
 */
void FreeMarkUpList(MarkInfo *List)
{
	MarkInfo *current;
	MarkInfo *mptr;

	current = List;
	while (current) {
		mptr = current;
		current = current->next;
		if (mptr->start)
			free(mptr->start);
		if (mptr->text)
			free(mptr->text);
		if (mptr->end)
			free(mptr->end);

		/* Any tag can have one */
		if (mptr->anc_name)
			free(mptr->anc_name);

		if (!mptr->is_end && (mptr->type == M_ANCHOR)) {
			if (mptr->anc_href)
				free(mptr->anc_href);
			if (mptr->anc_title)
				free(mptr->anc_title);
			if (mptr->anc_target)
				free(mptr->anc_target);
		}

		if (mptr->s_aps)	/* Aprog */
			_FreeAprogStruct(mptr->s_aps);
		if (mptr->s_ats)	/* Applet */
			_FreeAppletStruct(mptr->s_ats);
		if (mptr->t_p1)		/* Table */
			_FreeTableStruct(mptr->t_p1);

		if (mptr->preallo) {
			/* Place back on preallocated list */
			mptr->next = mark_start;
			mark_start = mptr;
		} else {
			free(mptr);
		}
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

/* Free up the passed linked list of formatted elements, freeing
 * all memory associated with each element.
 */
void FreeLineList(ElemInfo *list, HTMLWidget hw)
{
	ElemInfo *current;
	ElemInfo *eptr;
	ImageInfo *picd;
	ImageInfo *next;
	ImageInfo *tmp;
	int iframe_done = 0;

	current = list;
	while (current) {
		eptr = current;
		current = current->next;
		if (eptr->edata)
			free(eptr->edata);
		if (eptr->label_id)
			free(eptr->label_id);
		if (eptr->title)
			free(eptr->title);
		if (eptr->type == E_IFRAME) {
			/* Delete all of them if not already done */
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
		if ((eptr->type == E_IMAGE) && eptr->pic_data) {
			picd = eptr->pic_data;
                        /*
                         * Don't free internal image pixmaps or data
                         */
			if ((picd->image != (Pixmap)NULL) && picd->fetched) {
				XFreePixmap(XtDisplay(hw), picd->image);
				if (picd->clip != (Pixmap)NULL)
					XFreePixmap(XtDisplay(hw), picd->clip);
				if (picd->ori_colrs)
					free(picd->ori_colrs);
			}
			/* Free any animation ImageInfo and Pixmaps */
			if (picd->anim_info && picd->next) {
				next = picd->next;
				while (next) {
					tmp = next;
					next = tmp->next;
					if (tmp->image != (Pixmap)NULL) {
						XFreePixmap(XtDisplay(hw),
							    tmp->image);
						tmp->image = (Pixmap)NULL;
					}
					if (tmp->clip != (Pixmap)NULL) {
						XFreePixmap(XtDisplay(hw),
							    tmp->clip);
						tmp->clip = (Pixmap)NULL;
					}
					/* Free rescaled animation data */
					if (picd->fetched && !picd->cached) {
						if (tmp->image_data)
							free(tmp->image_data);
						if (tmp->clip_data)
							free(tmp->clip_data);
						if (tmp->alpha)
							free(tmp->alpha);
					}
					free(tmp);
				}
				if (picd->has_anim_image &&
				    (picd->anim_image != (Pixmap)NULL))
					XFreePixmap(XtDisplay(hw),
						    picd->anim_image);
				if (picd->has_anim_image &&
				    (picd->bg_image != (Pixmap)NULL))
					XFreePixmap(XtDisplay(hw),
						    picd->bg_image);
				if (picd->timer)
					XtRemoveTimeOut(picd->timer);
			}
			/* Background image with alpha channel */
			if (picd->is_bg_image && picd->alpha &&
			    picd->alpha_image_data)
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
			free(picd);
		}
		if (eptr->preallo) {
			/* Place element back on preallocated list */
			eptr->next = elem_start;
			elem_start = eptr;
		} else {
			free(eptr);
		}
	}
}

/* Handles allocation of Element memory */
ElemInfo *GetElemRec()
{
	static int init = 0;
	ElemInfo *next;

	/* Preallocate some elements */
	if (!init) {
		int num = get_pref_int(eELEMENT_MEMORY_PREALLOCATION);
		int i;

		if (num < 1)
			num = 1;
		next = elem_start = (ElemInfo *) calloc(num * sizeof(ElemInfo),
							sizeof(char));
		CHECK_OUT_OF_MEM(next);
		next->preallo = 1;
		for (i = 1; i < num; i++) {
			next->next = next + 1;
			next = next->next;
			next->preallo = 1;
		}
		next->next = NULL;
		init = 1;
	}
	if (!elem_start) {
		/* Preallocated list was empty */
		next = (ElemInfo *) calloc(sizeof(ElemInfo), sizeof(char));
		CHECK_OUT_OF_MEM(next);
	} else {
		/* Return top of preallocated list */
		next = elem_start;
		/* Zero it */
		memset(next, 0, sizeof(ElemInfo));
		elem_start = next->next;
	}
	return next;
}

/* Handles allocation of Markup memory */
MarkInfo *GetMarkRec()
{
	static int init = 0;
	MarkInfo *next;

	/* Preallocate some */
	if (!init) {
		int num = get_pref_int(eMARKUP_MEMORY_PREALLOCATION);
		int i;

		if (num < 1)
			num = 1;
		next = mark_start = (MarkInfo *) calloc(num * sizeof(MarkInfo),
							sizeof(char));
		CHECK_OUT_OF_MEM(next);
		next->preallo = 1;
		for (i = 1; i < num; i++) {
			next->next = next + 1;
			next = next->next;
			next->preallo = 1;
		}
		next->next = NULL;
		init = 1;
	}
	if (!mark_start) {
		/* Preallocated list was empty */
		next = (MarkInfo *) calloc(sizeof(MarkInfo), sizeof(char));
		CHECK_OUT_OF_MEM(next);
	} else {
		/* Return top of preallocated list */
		next = mark_start;
		/* Zero it */
		memset(next, 0, sizeof(MarkInfo));
		mark_start = next->next;
	}

	return next;
}

/*
 * Passed in 2 element pointers, and element positions.
 * Function should return 1 if if start occurs before end.
 * Otherwise return 0.
 */
int ElementLessThan(ElemInfo *start, ElemInfo *end, int start_pos, int end_pos)
{
	/*
	 * Deal with start or end being NULL
	 */
	if (!start && !end)
		return(0);
	if (!start && end)
		return(1);
	if (start && !end)
		return(0);
	/*
	 * Deal with easy identical case
	 */
	if (start == end) {
		if (start_pos < end_pos)
			return(1);
		return(0);
	}
	/* We know element Ids are always equal or increasing within a list.*/
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
	if (!start && !end)
		return(0);
	if (!start && end)
		return(1);
	if (start && !end)
		return(0);
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
