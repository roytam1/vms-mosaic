/* Copyright G.Dauphin Sep 97 */

/*****
* XmHTML Widget code:
* Copyright (C) 1994-1997 by Ripley Software Development
* All Rights Reserved
*
* This file is part of the XmHTML Widget Library.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the Free
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****/

/* Copyright (C) 1998, 1999, 2000, 2004, 2005, 2006, 2007
 * The VMS Mosaic Project
 */

#include "../config.h"
#include "HTMLP.h"
#include "HTMLPutil.h"
#include "HTMLframe.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "../libnut/str-tools.h"

#ifndef DISABLE_TRACE
extern int reportBugs;
extern int htmlwTrace;
#endif

#if defined(MULTINET) && defined(__DECC) && (__VMS_VER >= 70000000)
#define strdup decc$strdup
#endif
extern char *strdup();

extern MarkInfo *NULL_ANCHOR_PTR;

/* Basically we show the URLs that appear within the frameset tag
 * as URLs and add some text explaining that these are the URLs they
 * were supposed to see as frames.  We also show the NOFRAMES stuff.
 */
void FramePlace(HTMLWidget hw, MarkInfo *mptr, PhotoComposeContext *pcc)
{
    char *sptr, *nptr;

    switch (mptr->type) {
	case M_FRAME:
		pcc->anchor_tag_ptr = mptr;
		if (sptr = ParseMarkTag(mptr->start, MT_FRAME, "SRC")) {
		    mptr->anc_href = GetMarkText(sptr);
		    if (hw->html.previously_visited_test &&
			((*(visitTestProc)(hw->html.previously_visited_test)) 
			 ((Widget)hw, sptr))) {   
			pcc->fg = hw->html.visitedAnchor_fg;
			pcc->underline_number =
					  hw->html.num_visitedAnchor_underlines;
			pcc->dashed_underlines =
					    hw->html.dashed_visitedAnchor_lines;
		    } else {       
			pcc->fg = hw->html.anchor_fg;
			pcc->underline_number = hw->html.num_anchor_underlines;
			pcc->dashed_underlines = hw->html.dashed_anchor_lines;
		    } 
		}
		if (pcc->in_underlined) {
		    pcc->dashed_underlines = False;
		    if (!pcc->underline_number)
			pcc->underline_number = 1;
		}                      

		ConditionalLineFeed(hw, 1, pcc);
		if (sptr) {
		    mptr->text = sptr;
		    PartOfTextPlace(hw, mptr, pcc);
		}
		pcc->fg = hw->manager.foreground;
		pcc->underline_number = pcc->in_underlined;
		pcc->dashed_underlines = False;
		pcc->anchor_tag_ptr = NULL_ANCHOR_PTR;

		nptr = ParseMarkTag(mptr->start, MT_FRAME, "NAME");
		if (nptr && sptr) {
		    pcc->x += 10;
		    mptr->text = nptr;
		    PartOfTextPlace(hw, mptr, pcc);
		    free(nptr);
		}
		mptr->text = NULL;
		if (sptr)
		    free(sptr);
		break;

	case M_FRAMESET:
		if (mptr->is_end) {
			if (pcc->frameset) {
				pcc->frameset--;
				ConditionalLineFeed(hw, 1, pcc);
				mptr->text = "-----------------------";
				PartOfTextPlace(hw, mptr, pcc);
			}
		} else {
			pcc->frameset++;
			ConditionalLineFeed(hw, 1, pcc);
			mptr->text = "------- Frame Set -------";
			PartOfTextPlace(hw, mptr, pcc);
		}
		mptr->text = NULL;
		break;

	case M_NOFRAMES:
		if (mptr->is_end) {
			if (pcc->noframes) {
				pcc->noframes--;
				ConditionalLineFeed(hw, 1, pcc);
				mptr->text = "*************************";
				PartOfTextPlace(hw, mptr, pcc);
				ConditionalLineFeed(hw, 1, pcc);
			}
		} else {
			pcc->noframes++;
			ConditionalLineFeed(hw, 1, pcc);
			mptr->text = "***** No Frames View *****";
			PartOfTextPlace(hw, mptr, pcc);
			ConditionalLineFeed(hw, 1, pcc);
		}
		mptr->text = NULL;
		break;

	default:
		break;
    }
}

static void adjustFrame(FrameInfo *parent, int *p_width, int *p_height);
 
/*
 * Original code from XmHTML Widget :
 * Copyright (C) 1994-1997 by Ripley Software Development
 */

/*** External Function Prototype Declarations ***/

/*** Public Variable Declarations ***/
#define ROW     1
#define COL     2

/* Useful defines */
#define IS_FRAMESET(F) \
	((F)->frame_type & FRAMESET_TYPE)
#define IS_FRAME_SIZE_RELATIVE(F) \
	((F)->frame_size_type == FRAME_SIZE_RELATIVE)
#define IS_FRAME_SIZE_OPTIONAL(F) \
	((F)->frame_size_type == FRAME_SIZE_OPTIONAL)
#define IS_FRAME_SIZE_FIXED(F) \
	((F)->frame_size_type == FRAME_SIZE_FIXED)
#define IS_FRAMESET_LAYOUT_ROWS(F) \
	(IS_FRAMESET(F) && ((F)->frame_layout == FRAMESET_LAYOUT_ROWS))
#define IS_FRAMESET_LAYOUT_COLS(F) \
	(IS_FRAMESET(F) && ((F)->frame_layout == FRAMESET_LAYOUT_COLS))

/* Definition of an HTML frameset */
typedef struct _frameSet {
        int type;	            /* Type of this set, either ROW or COL */
        int border;                 /* Frame border value */
        int *sizes;                 /* Array of child sizes */
        FrameSize *size_types;      /* Array of possible size specifications */
        int nchilds;                /* Max num of children */
        int childs_done;            /* Num of children processed so far */
        int insert_pos;             /* Insertion position of current child */
        struct _frameSet *parent;   /* Parent frameset of this frameset */
        struct _frameSet *childs;   /* List of children */
        struct _frameSet *next;     /* Next frameSet */
	FrameInfo *actualFrameSet;  /* Saved FrameSet */
} frameSet;

/* Stack of framesets */
typedef struct _frameStack {
        frameSet *frame_set;
        struct _frameStack *next;
} frameStack;

static int current_frame;                  /* Running frame counter */
static frameSet *frame_sets;               /* List of all framesets processed */
static frameStack frame_base;
static frameStack *frame_stack;


/*****
* Name:                 pushFrameSet
* Return Type:  void
* Description:  pushes a frameset on the stack
* In:
*       frame_set:      frameset to push
* Returns:
*       nothing
*****/
static void pushFrameSet(frameSet *frame_set)
{
        frameStack *tmp = (frameStack *)malloc(sizeof(frameStack));

        tmp->frame_set = frame_set;
        tmp->next = frame_stack;
        frame_stack = tmp;
}

/*****
* Name:                 popFrameSet
* Return Type:  frameSet*
* Description:  pops a frameset of the stack
* In:
*       nothing
* Returns:
*       the next frameset on the stack, or NULL when stack is empty
*****/
static frameSet *popFrameSet(void)
{
        frameStack *tmp;
        frameSet *frame_set;

        if (frame_stack->next) {
                tmp = frame_stack;
                frame_stack = frame_stack->next;
                frame_set = tmp->frame_set;
                free(tmp);
                return(frame_set);
        }
        return(NULL);
}

/*****
* Name:         doFrameSet
* Return Type:  frameSet *
* Description:  creates and fills a frameSet structure with the info in its
*               attributes
* In:
*       attributes:     attributes for this frameset
* Returns:
*       a newly created frameset.
* Note:
*       this routine inserts each frameset it creates in a linked list which
*       is used for stack purposes.
*****/
static frameSet *doFrameSet(String attributes, int *is_grid)
{
        frameSet *list, *tmp;
        String rPtr, cPtr, tmpPtr, ptr;
        int i, j;
	int nrow = 0;
	int ncol = 0;

        /* Nothing to do if no attributes */
        if (!attributes)
                return(frame_sets);

        /* Create new entry */
        list = (frameSet *)calloc(1, sizeof(frameSet));

        /*
         * Count how many children this frameset has: the num of children
	 * is given by the num of entries within the COLS or ROWS tag
         * Note that children can be frames and/or framesets as well.
         */
        if (!*is_grid &&
	    (rPtr = ParseMarkTag(attributes, MT_FRAMESET, "rows"))) {
		nrow++;
		for (tmpPtr = rPtr; tmpPtr && *tmpPtr; tmpPtr++) {
                	if (*tmpPtr == ',')
                        	nrow++;
		}
	} else {
		rPtr = NULL;
	}
        if (cPtr = ParseMarkTag(attributes, MT_FRAMESET, "cols")) {
		ncol++;
		for (tmpPtr = cPtr; tmpPtr && *tmpPtr; tmpPtr++) {
                	if (*tmpPtr == ',')
                        	ncol++;
		}
	}

	/* Hack to support frameset grids */
	if (nrow && ncol) {
#ifndef DISABLE_TRACE
		if (htmlwTrace || reportBugs)
			fprintf(stderr,	"Frameset is %d by %d grid\n",
				nrow, ncol);
#endif
		if (nrow == 1) {
			nrow = 0;
		} else if (ncol == 1) {
			ncol = 0;
		} else {
			*is_grid = 1;
			ncol = 0;
		}
	}
	if (nrow) {
        	list->nchilds = nrow;
        	list->type = ROW;
	} else if (ncol) {
        	list->nchilds = ncol;
        	list->type = COL;
	} else {
        	list->nchilds = 1;
        	list->type = COL;
	}
        list->sizes = (int *)calloc(list->nchilds, sizeof(int));
        list->size_types = (FrameSize *)calloc(list->nchilds,sizeof(FrameSize));
        list->childs = (frameSet *)calloc(list->nchilds, sizeof(frameSet));

        /*
         * Get dimensions: when we encounter a ``*'' in a size definition it
         * means we are free to choose any size we want.  When it's a number
         * followed by a ``%'' we must choose the size relative against the
	 * total width of the render area.  When it's a number not followed
	 * by anything we have an absolute size.
         */
	if (list->type == COL) {
        	tmpPtr = ptr = cPtr;
	} else {
        	tmpPtr = ptr = rPtr;
	}
        /* Remove blanks */
	if (ptr) {
        	for (i = j = 0; ptr[i]; i++) {
			if (!isspace((unsigned char)(ptr[i])))
				ptr[j++] = ptr[i];
		}
		ptr[j] = '\0';
	}

        i = 0;
        while (tmpPtr && (i < list->nchilds)) {
                if (*tmpPtr == ',' || !*tmpPtr) {
                        if (*(tmpPtr - 1) == '*') {
                                list->size_types[i] = FRAME_SIZE_OPTIONAL;
                        } else if (*(tmpPtr - 1) == '%') {
                                list->size_types[i] = FRAME_SIZE_RELATIVE;
                        } else {
                                list->size_types[i] = FRAME_SIZE_FIXED;
			}
                        list->sizes[i++] = atoi(ptr);

                        if (!*tmpPtr)
                                break;
                        ptr = tmpPtr + 1;
                }
                tmpPtr++;
        }
        if (rPtr)
		free(rPtr);
        if (cPtr)
		free(cPtr);

	/* Frame borders can be specified by both frameborder or border,
	 * they are equal.
	 */
	if (frame_stack->frame_set) {
		list->border = frame_stack->frame_set->border;
	} else {
	        list->border = 2;
	}
	/* Sigh, stupid Netscape frameset definition allows a tag to have
	 * a textvalue or a number.
	 */
        if (tmpPtr = ParseMarkTag(attributes, MT_FRAMESET, "frameborder")) {
                if (!my_strcasecmp(tmpPtr, "no") || (*tmpPtr == '0'))
                        list->border = 0;
                free(tmpPtr);
        } else if (tmpPtr = ParseMarkTag(attributes, MT_FRAMESET, "border")) {
                if (!my_strcasecmp(tmpPtr, "no") || (*tmpPtr == '0'))
                        list->border = 0;
                free(tmpPtr);
        }

	/* Insert this new frame in the overall frameset list. */
        if (!frame_sets) {
                frame_sets = list;
        } else {
                for (tmp = frame_sets; tmp && tmp->next; tmp = tmp->next)
			;
                tmp->next = list;
        }

	/* Create actual representation of frameset */
        {
            FrameInfo *actualFrameSet;

            actualFrameSet = (FrameInfo *)calloc(1, sizeof(FrameInfo));
            actualFrameSet->frame_type |= FRAMESET_TYPE;
            actualFrameSet->frame_layout =
              (list->type == ROW) ? FRAMESET_LAYOUT_ROWS : FRAMESET_LAYOUT_COLS;
            list->actualFrameSet = actualFrameSet;
        }
        return(list);
}

/*****
* Name:         doFrame
* Return Type:  HTMLFrameWidget*
* Description:  fills a HTML frame structure with data from it's attributes
* In:
*       html:           HTMLWidget id;
*       attributes:     frame attributes
* Returns:
*       updated frame
* Note:
*       This routine takes the frame to update from an already allocated list
*       of frames and increments the running frame counter when it returns.
*****/

static FrameInfo *doFrame(HTMLWidget html, String attributes)
{
        FrameInfo *frame;
        String chPtr;

        if (!attributes)
		return NULL;

        frame = html->html.frames[current_frame];

	/* Required to have SRC */
        frame->frame_src = ParseMarkTag(attributes, MT_FRAME, "src");
	if (!frame->frame_src)
		return NULL;
	if (!*frame->frame_src) {
		free(frame->frame_src);
		frame->frame_src = NULL;
		return NULL;
	}

	/* Not in a table */
	frame->cw_only = 0;

	/* Default frame sizing and scrolling */
	frame->frame_size_type = FRAME_SIZE_FIXED;
        frame->frame_scroll_type = FRAME_SCROLL_AUTO;

	/* Get frame name, default to _frame if not present */
        if (!(frame->frame_name = ParseMarkTag(attributes, MT_FRAME, "name"))) {
                char buf[24];

                sprintf(buf, "_frame%i", current_frame);
                frame->frame_name = strdup(buf);
        }

	if (chPtr = ParseMarkTag(attributes, MT_FRAME, "marginwidth")) {
		frame->frame_margin_width = atoi(chPtr);
		free(chPtr);
	} else {
		frame->frame_margin_width = 5;
	}
	if (chPtr = ParseMarkTag(attributes, MT_FRAME, "marginheight")) {
		frame->frame_margin_height = atoi(chPtr);
		free(chPtr);
	} else {
		frame->frame_margin_height = 5;
	}
	if (chPtr = ParseMarkTag(attributes, MT_FRAME, "noresize")) {
		frame->frame_resize = False;
		free(chPtr);
	}
	if (chPtr = ParseMarkTag(attributes, MT_FRAME, "frameborder")) {
		if (!my_strcasecmp(chPtr, "no") || (*chPtr == '0')) {
			frame->frame_border = 0;
		} else {
			frame->frame_border = 2;
		}
		free(chPtr);
	} else {
		frame->frame_border = -1;
	}
	/* What about scrolling? */
        if (chPtr = ParseMarkTag(attributes, MT_FRAME, "scrolling")) {
                if (!my_strcasecmp(chPtr, "yes")) {
			frame->frame_scroll_type = FRAME_SCROLL_YES;
                } else if (!my_strcasecmp(chPtr, "no")) {
                        frame->frame_scroll_type = FRAME_SCROLL_NONE;
		}
                free(chPtr);
        }

#ifndef DISABLE_TRACE
	if (htmlwTrace) {
	    fprintf(stderr,
		"doFrame, frame %i name: %s\n\tsrc: %s\n\tmargin width: %i\n\tmargin height: %i\n\tresize: %s\n\tscrolling: %s\n",
		current_frame,
		frame->frame_name,
                frame->frame_src,
		frame->frame_margin_width,
                frame->frame_margin_height,
		frame->frame_resize ? "yes" : "no",
                frame->frame_scroll_type == FRAME_SCROLL_AUTO ? "auto" :
                (frame->frame_scroll_type == FRAME_SCROLL_YES ? "always" :
		 "none"));
	}
#endif
        /* Increment running frame counter */
        current_frame++;
        return(frame);
}

/*
* Description:  inserts a child frameset in it's parent list
* In:
*       parent:         parent of this frameset
*       child:          obvious
*/
static void insertFrameSetChild(frameSet *parent, frameSet *child)
{
        if (parent && parent->childs_done < parent->nchilds) {
                int idx = parent->childs_done;
		FrameInfo *c, *dad, *son;

                child->parent = parent;
                child->insert_pos = idx;

                dad = parent->actualFrameSet;
                son = child->actualFrameSet;

                son->frame_size_s = parent->sizes[child->insert_pos];
                son->frame_size_type = parent->size_types[child->insert_pos];

                if (son->frame_size_s == 0)
                        son->frame_size_type = FRAME_SIZE_OPTIONAL;

                /* Set additional constraints for this frame */
                son->frame_border = parent->border;

                /* Disable resizing if we don't have a border */
                if (!son->frame_border)
                        son->frame_resize = False;

                for (c = dad->frame_children; c; c = c->frame_next) {
                        if (!c->frame_next)
                                break;
		}
                if (c) {
                        c->frame_next = son;
                } else {
                        dad->frame_children = son;
		}
                son->frame_parent_frameset = dad;

                parent->childs[parent->childs_done] = *child;
                parent->childs_done++;
        }
}


/*****
* Name:                 insertFrameChild
* Return Type:  void
* Description:  sets the geometry constraints on a HTML frame
* In:
*       frame_set:      frameset parent of this frame;
*       frame:          frame for which to set the constraints
* Returns:
*       nothing, but frame is updated.
*****/
static void insertFrameChild(frameSet *frame_set, FrameInfo *frame)
{
	FrameInfo *c, *dad;
        int insert_pos = frame_set->childs_done;

	if (!frame)
		return;

	frame->frame_size_s = frame_set->sizes[insert_pos];
        frame->frame_size_type = frame_set->size_types[insert_pos];

        if (frame->frame_size_s == 0)
                frame->frame_size_type = FRAME_SIZE_OPTIONAL;

        /* Set additional constraints for this frame */
	if (frame->frame_border == -1)
        	frame->frame_border = frame_set->border;

        /* Disable resizing if we don't have a border */
        if (!frame->frame_border)
        	frame->frame_resize = False;

        dad = frame_set->actualFrameSet;
        for (c = dad->frame_children; c; c = c->frame_next) {
                if (!c->frame_next)
                        break;
	}
        if (c) {
                c->frame_next = frame;
        } else {
                dad->frame_children = frame;
	}
        frame->frame_parent_frameset = dad;

        frame_set->childs_done++;
}

/* *mptr is a pointer to frameset */
static void makeFrameset(HTMLWidget hw, MarkInfo **mptr)
{
	MarkInfo *tmp;
	frameSet *current_set = NULL;
	frameSet *parent_set = NULL;
	FrameInfo *frame;
	int idx = 0;

	for (tmp = *mptr; tmp; tmp = tmp->next) {
		switch (tmp->type) {
		    case M_FRAMESET:
			if (tmp->is_end) {  /* End frameset  Pop stack */
				current_set = popFrameSet();
				/* No more sets on the stack: we've reached
				 * end of outermost frameset tag */
				if (!current_set)
				    return;
			} else {  /* New one, push current frameset on stack */
				pushFrameSet(current_set);
				parent_set = frame_stack->frame_set;
				/* Check if we still have room for this thing */
				if (!parent_set || (parent_set->childs_done <
				    parent_set->nchilds)) {
				    int grid = 0;
				    int i, childs;
				    MarkInfo *mark, *temp;

				    /* Create a new frameset */
				    current_set = doFrameSet(tmp->start, &grid);
				    insertFrameSetChild(parent_set,current_set);
				    /* Grid hack */
				    if (grid) for (i = current_set->nchilds,
						   temp = tmp; i; --i) {
					pushFrameSet(current_set);
					parent_set = frame_stack->frame_set;
					current_set = doFrameSet(temp->start,
					    			 &grid);
				        insertFrameSetChild(parent_set,
					    		    current_set);
					childs = current_set->nchilds;
					mark = tmp->next;
					while (mark && childs &&
					       (mark->type != M_FRAMESET)) {
					    if ((mark->type == M_FRAME) &&
						!mark->is_end) {
						frame = doFrame(hw,mark->start);
						insertFrameChild(current_set,
								 frame);
						tmp = mark;
						childs--;
						if (!childs)
						    current_set = popFrameSet();
					    }
					    mark = mark->next;
					}
				    }
				    idx = 0;
				} else {
				    /* No more room available, this is an
				     * unspecified frameset, kill it and
				     * all children it might have.
				     */
				    int depth = 1;

				    for (tmp = tmp->next; tmp; tmp = tmp->next){
					if (tmp->type == M_FRAMESET) {
					    if (tmp->is_end) {
						if (--depth == 0)
						    break;
					    } else {
						/* Child frameset */
						depth++;
					    }
					}
				    }
#ifndef DISABLE_TRACE
				    if (htmlwTrace || reportBugs)
					fprintf(stderr,
					  "Bad <FRAMESET>: missing COLS or ROWS attribute on parent set\n");
#endif
				}
			}
			break;
		    case M_FRAME:
			if (tmp->is_end)	/* Ignore it */
				break;
			/* Check if we have room left */
			if (current_set->childs_done < current_set->nchilds) {
				/* Insert child in current frameset */
				frame = doFrame(hw, tmp->start);
				insertFrameChild(current_set, frame);
				idx++;
			} else {
				/* Hack: move to previous FrameSet, if any */
				while (current_set) {
				    current_set = popFrameSet();
				    if (!current_set) {
#ifndef DISABLE_TRACE
					if (htmlwTrace || reportBugs)
					    fprintf(stderr,
						"Bad <FRAME>: too few COLS or ROWS available\n");
#endif
					return;
				    }
				    if (current_set->childs_done <
					current_set->nchilds)
					break;
				}
				frame = doFrame(hw, tmp->start);
				insertFrameChild(current_set, frame);
				idx++;
#ifndef DISABLE_TRACE
				if (htmlwTrace || reportBugs)
				    fprintf(stderr,
					"Bad <FRAME>: placed in higher <FRAMESET>\n");
#endif
			}
		    default:
			break;
		}
		if (idx == hw->html.nframe)
			return;
	}
}

static FrameInfo *getRootFrameset(HTMLWidget hw)
{
	FrameInfo *frame;

	for (frame = hw->html.frames[0]; frame && frame->frame_parent_frameset;
	     frame = frame->frame_parent_frameset)
		;

        return(frame);
}

/* length is :
 *	nn	: number of pixel
 *	nn%	: relative to lenght available
 *	n*	: option relative to remaining space
 */

static void adjustFramesetRows(FrameInfo *parent, int *p_width, int *p_height)
{
        FrameInfo *child;
        int width, height, cum_fixed_size, cum_rel_size;
        int nb_opt = 0;
	int nb_rel = 0;
	int nb_fix = 0;

	/* Begin with fixed-sized children */
	/* Then do relative-sized children */
	/* Finally, end up with optional-sized children */
        cum_fixed_size = 0;	/* pixel */
        cum_rel_size = 0;	/* % */

        for (child = parent->frame_children; child; child = child->frame_next) {
                if (IS_FRAME_SIZE_FIXED(child)) {
                        width = *p_width;
                        height = child->frame_size_s;
                        adjustFrame(child, &width, &height);
                        child->frame_width = width;
                        child->frame_height = height;
                        cum_fixed_size += height;
			nb_fix++;
                } else if (IS_FRAME_SIZE_RELATIVE(child)) {
                        width = *p_width;
                        height = child->frame_size_s * (*p_height) / 100;
                        adjustFrame(child, &width, &height);
                        child->frame_width = width;
                        child->frame_height = height;
                        cum_rel_size += height;
			nb_rel++;
                } else if (IS_FRAME_SIZE_OPTIONAL(child)) {
			/* Count how many optional there is */
                        ++nb_opt;
			child->frame_height = 1;
			child->frame_width = *p_width;
		}
        }

	/* Check total usage */
	if (cum_fixed_size + cum_rel_size < *p_height) {
		/* Distribute the rest to optional ones */
        	if (nb_opt > 0) {
                	int cum_size = cum_fixed_size + cum_rel_size;
			int remain_size, mean_opt_size;

                	remain_size = *p_height - cum_size;
                	if (remain_size <= nb_opt)
                        	remain_size = nb_opt ;
                	mean_opt_size = remain_size / nb_opt;

                	/* Go adjust */
                	for (child = parent->frame_children; child;
			     child = child->frame_next) {
                        	if (IS_FRAME_SIZE_OPTIONAL(child)) {
                                	width = *p_width;
                                	height = mean_opt_size;
                                	adjustFrame(child, &width, &height);
                                	child->frame_width = width;
                                	child->frame_height = height;
                        	}
                	}
        	} else if (cum_rel_size > 0) {
			/* Distribute % */
			int to_add;

			to_add = (*p_height - cum_fixed_size - cum_rel_size) /
				nb_rel;
			for (child = parent->frame_children; child;
			     child = child->frame_next) {
				if (IS_FRAME_SIZE_RELATIVE(child)) {
					width = *p_width;
					height = child->frame_height + to_add;
					adjustFrame(child, &width, &height);
					child->frame_width = width;
					child->frame_height = height;
				}
			}
		} else { /* Distribute pixels */
			int to_add = (*p_height - cum_fixed_size) / nb_fix;

			for (child = parent->frame_children; child;
			     child = child->frame_next) { 
				width = *p_width;
				height = child->frame_height + to_add;
				adjustFrame(child, &width, &height);
				child->frame_width = width;
				child->frame_height = height;
			}
		}
	} else if (cum_fixed_size + cum_rel_size > *p_height) {
		/* Too much space is allocated */
		if (cum_fixed_size <= *p_height) {  /* Too much % */
			int to_sub = (*p_height - cum_fixed_size);

			for (child = parent->frame_children; child;
			     child = child->frame_next) {
				if (IS_FRAME_SIZE_RELATIVE(child)) {
					width = *p_width;
					height = (child->frame_height *
						  to_sub) / cum_rel_size;
					if (height < 1)
						height = 1;
					adjustFrame(child, &width, &height);
					child->frame_width = width;
					child->frame_height = height;
				}
			}
		} else {  /* Too much pixel */
			for (child = parent->frame_children; child;
			     child = child->frame_next) { 
				width = *p_width;
				height = (child->frame_height * *p_height) /
					 (cum_fixed_size + cum_rel_size);
				if (height < 1)
                                	height = 1;
				adjustFrame(child, &width, &height);
				child->frame_width = width; 
                                child->frame_height = height;
			}
		}
	}
}

static void adjustFramesetColumns(FrameInfo *parent, int *p_width,
				  int *p_height)
{
        FrameInfo *child;
        int width, height;
        int cum_fixed_size = 0;
	int cum_rel_size = 0;
        int nb_opt = 0;
	int nb_rel = 0;
	int nb_fix = 0;

	/* Begin with fixed-sized children */
	/* Then do relative-sized children */
	/* Finally, end up with optional-sized children */
        for (child = parent->frame_children; child; child = child->frame_next) {
                if (IS_FRAME_SIZE_FIXED(child)) {
                        width = child->frame_size_s;
                        height = *p_height;
                        adjustFrame(child, &width, &height);
                        child->frame_width = width;
                        child->frame_height = height;
                        cum_fixed_size += width;
			nb_fix++;
                } else if (IS_FRAME_SIZE_RELATIVE(child)) {
                        width = child->frame_size_s * *p_width / 100;
                        height = *p_height;
                        adjustFrame(child, &width, &height);
                        child->frame_width = width;
                        child->frame_height = height;
                        cum_rel_size += width;
			nb_rel++;
                } else if (IS_FRAME_SIZE_OPTIONAL(child)) {
                        nb_opt++;
			child->frame_height = *p_height;
			child->frame_width = 1;
		}
	}
        if (cum_fixed_size + cum_rel_size < *p_width) {
                if (nb_opt > 0) {       
                        int cum_size = cum_fixed_size + cum_rel_size;
			int remain_size, mean_opt_size;
                                       
                        remain_size = *p_width - cum_size;
                        if (remain_size <= nb_opt)
                                remain_size = nb_opt;
                        mean_opt_size = remain_size / nb_opt;
                	/* Adjust */        
                        for (child = parent->frame_children; child;
			     child = child->frame_next) {      
                                if (IS_FRAME_SIZE_OPTIONAL(child)) {
                                        width = mean_opt_size;
                                        height = *p_height;
                                        adjustFrame(child, &width, &height);
                                        child->frame_width = width;
                                        child->frame_height = height;
                                }      
                        }              
                } else if (cum_rel_size > 0) {
                        int to_add;    
                                       
                        to_add = (*p_width - cum_fixed_size - cum_rel_size) /
				 nb_rel;                                   
                        for (child = parent->frame_children; child;
			     child = child->frame_next) {      
                                if (IS_FRAME_SIZE_RELATIVE(child)) {
					height = *p_height;
					width = child->frame_width + to_add;
                                        adjustFrame(child, &width, &height);
                                        child->frame_width = width;
                                        child->frame_height = height;
                                }      
                        }
                } else {  /* Pixels */
                        int to_add = (*p_width - cum_fixed_size) / nb_fix;

                        for (child = parent->frame_children; child;
			     child = child->frame_next) {      
				height = *p_height;
				width = child->frame_width + to_add;
                                adjustFrame(child, &width, &height);
                                child->frame_width = width;
                                child->frame_height = height;
                        }              
                }
        } else if ((cum_fixed_size + cum_rel_size) > *p_width) {
		/* Too much space is allocated */                        
                if (cum_fixed_size <= *p_width ) {
                        int to_sub = *p_width - cum_fixed_size;

                        for (child = parent->frame_children; child;
			     child = child->frame_next) { 
                                if (IS_FRAME_SIZE_RELATIVE(child)) {
                                        height = *p_height;
                                        width = (child->frame_width * to_sub) /
						cum_rel_size;
                                        if (width < 1)
                                                width = 1;
                                        adjustFrame(child, &width, &height);
                                        child->frame_width = width;
                                        child->frame_height = height;
                                }
                        }
                } else {  /* Too many pixels */ 
                        for (child = parent->frame_children; child;
			     child = child->frame_next) {      
                                height = *p_height;
                                width = (child->frame_width * *p_width) /
					(cum_fixed_size + cum_rel_size);
                                if (width < 1)
                                        width = 1;
                                adjustFrame(child, &width, &height);
                                child->frame_width = width;
                                child->frame_height = height;
                        }
                }
        } 
}

static void adjustFrame(FrameInfo *parent, int *p_width, int *p_height)
{
        if (*p_width <= 0)
                *p_width = 1;
        if (*p_height <= 0)
                *p_height = 1;

        if (IS_FRAMESET(parent)) {
		/* Do recursion only if it is a frameset */
                if (parent->frame_layout == FRAMESET_LAYOUT_ROWS) {
                        adjustFramesetRows(parent, p_width, p_height);
                } else if (parent->frame_layout == FRAMESET_LAYOUT_COLS) {
                        adjustFramesetColumns(parent, p_width, p_height);
		}
        }
}

static void locateFrame(FrameInfo *parent, int x, int y)
{
        parent->frame_x = x;
        parent->frame_y = y;

        if (IS_FRAMESET(parent)) {
		/* Do recursion only if it is a frameset */
                FrameInfo *frame;

                if (IS_FRAMESET_LAYOUT_ROWS(parent)) {
                        for (frame = parent->frame_children; frame;
			     frame = frame->frame_next) {
                                locateFrame(frame, x, y);
                                y += frame->frame_height;
                        }
                }
                if (IS_FRAMESET_LAYOUT_COLS(parent)) {
                        for (frame = parent->frame_children; frame;
			     frame = frame->frame_next) {
                                locateFrame(frame, x, y);
                                x += frame->frame_width;
                        }
                }
        }
}


static void adjustConstraints(HTMLWidget hw /* top HTMLWidget */)
{
        FrameInfo *root_frame;
        int work_width, work_height;

        /* This uses the core dimensions */
        work_width = hw->core.width - (2 * hw->manager.shadow_thickness);
        work_height = hw->core.height - (2 * hw->manager.shadow_thickness);

        /* Get the root frame */
        root_frame = getRootFrameset(hw);

        /* Adjust frames' dimensions */
        adjustFrame(root_frame, &work_width, &work_height);

        /* Adjust frames' positions */
        locateFrame(root_frame, 0, 0);
}


/*****
* Name:                 destroyFrameSets
* Return Type:  void
* Description:  destroys the memory used by the framesets
* In:
*       set:            list of framesets to be destroyed
* Returns:
*       nothing
*****/
static void destroyFrameSets(frameSet *set)
{
        frameSet *tmp;

        while (set) {
                tmp = set->next;
                if (set->sizes)
                        free(set->sizes);
                if (set->size_types)
                        free(set->size_types);
                if (set->childs)
                        free(set->childs);
                free(set);
                set = tmp;
        }
}


/*****
* Name:                 HTMLCheckForFrames
* Return Type:  int
* Description:  checks if the given list of objects contains HTML frames
* In:
*       objects:        parser output to check
* Returns:
*       no of frames found in the current document.
*****/
static int HTMLCheckForFrames(MarkInfo **mptr)
{
        MarkInfo *tmp;
        int nframes = 0;

        /*
         * Frames are not allowed to appear inside the BODY tag.
         * So we never have to walk the entire contents of the current document
         * but simply break out of the loop once we encounter the <BODY> tag.
         * This is a fairly huge performance increase.  Unfortunately
	 * morons put <BODY> tags inside the frameset tags, so we
	 * cannot do this.
         */
        for (tmp = *mptr; tmp; tmp = tmp->next) {
                if ((tmp->type == M_FRAME) && !tmp->is_end)
                        nframes++;
	}
        return(nframes);
}


static void recursiveDestroyFrameset(FrameInfo *frame)
{
        if (!frame)  /* Sanity */
    		return;

        if (IS_FRAMESET(frame)) {
                FrameInfo *child, *tmp;

                for (child = frame->frame_children; child; ) {
                        tmp = child->frame_next;
                        recursiveDestroyFrameset(child);
                        child = tmp;
                }
                frame->frame_children = NULL;
        }
        if (frame->frame_src) {
		free(frame->frame_src);
		frame->frame_src = NULL;  /* Sanity */
        }
        if (frame->frame_name) {
                free(frame->frame_name);
                frame->frame_name = NULL;  /* Sanity */
        }
        frame->frame_parent_frameset = NULL;  /* Sanity */

        free(frame);
}


/*****
* Name:                 HTMLDestroyFrames
* Return Type:  void
* Description:  frame destroyer
* In:
*       hw:           HTMLWidget id
* Returns:
*       nothing, but the frames list of the widget is destroyed.
*****/
void HTMLDestroyFrames(HTMLWidget hw)
{
        FrameCbData cbs;
 	FrameInfo *root_frame = getRootFrameset(hw);

        /* Free them */
        recursiveDestroyFrameset(root_frame);

	cbs.reason = FRAME_DELETE;
	/* Call the callback list */
	XtCallCallbackList((Widget)hw, hw->html.frame_callback, &cbs);
	hw->html.nframe = 0;
}


/* Description:  main frame parser
* In:
*       hw:           HTMLWidget id;
* Returns:
*       True when all frames could be created, False otherwise.
*/
Boolean HTMLCreateFrameSet(HTMLWidget hw, MarkInfo **mptr,
			   PhotoComposeContext *pcc)
{
        int i;
        MarkInfo **tmp;
        FrameCbData cbs;
        FrameInfo *frame_w;

        frame_stack = &frame_base;
        frame_stack->next = NULL;
        frame_stack->frame_set = NULL;

        /* First destroy all previous frames of this widget */
        if (hw->html.nframe)
                HTMLDestroyFrames(hw);

        if (frame_sets)
                destroyFrameSets(frame_sets);
        frame_sets = NULL;

	hw->html.nframe = HTMLCheckForFrames(mptr);
	/*
	 * Don't do a thing if we are destroying previous list, we don't have
	 * a frame callback or the new widget doesn't have any frames at all
	 */
        if (!hw || !hw->html.frame_callback || !hw->html.nframe) {
		hw->html.frames = NULL;
                return(False);
	}

        /* Create the list of HTML frame children */
        hw->html.frames = (FrameInfo **)calloc(hw->html.nframe,
					       sizeof(FrameInfo *));

        /* Create individual HTML frame children ptrs */
        for (i = 0; i < hw->html.nframe; i++) {
                frame_w = (FrameInfo *)calloc(1, sizeof(FrameInfo));
                hw->html.frames[i] = frame_w;
        }

	/* Move to the first frameset declaration */
	tmp = mptr;
        current_frame = 0;
	/* Create all frames (and possibly nested framesets also) */
	makeFrameset(hw, tmp);
	*tmp = NULL;	/* This ends the scanning of Mark loop */
			/* No more HTML is done after outermost frameset tag */

        /* Adjust framecount, makeFrameSets might have found invalid sets */
	if (!current_frame) {
	        for (i = 0; i < hw->html.nframe; i++)
			free(hw->html.frames[i]);
		free(hw->html.frames);
		hw->html.frames = NULL;
	        hw->html.nframe = 0;
                return(False);
	}
	/* Memory leak.  Unused frame records not freed later. */
        hw->html.nframe = current_frame;

        adjustConstraints(hw);

	/* Clean the window we are building the frames in */
        XClearWindow(hw->html.dsp, XtWindow(hw->html.view));

	cbs.reason = FRAME_CREATE;
	/* Call the callback list */
	XtCallCallbackList((Widget)hw, hw->html.frame_callback, &cbs);

        return(True);
}

void IframeRefresh(ElemInfo *eptr)
{
	if (eptr->frame && !eptr->frame->mapped && eptr->frame->iframe) {
		eptr->frame->mapped = True;
		eptr->frame->seeable = True;
		XtSetMappedWhenManaged(eptr->frame->iframe, True);
	}
}
