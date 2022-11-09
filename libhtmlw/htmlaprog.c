/* HTMLaprog.c
 * Version 3.0 [Sep96]
 *
 * Copyright (C) 1996 - G.Dauphin
 * See the file "license.mMosaic" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES. 
 */

#include "../config.h"
#include "HTMLP.h"
#include "HTMLPutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <Xm/Xm.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/RowColumn.h>
#include <Xm/List.h>
#include <Xm/Scrollbar.h>
#include <Xm/Label.h>

void _FreeAprogStruct(AprogInfo *aps)
{
	int i;

	if (aps->src)
		free(aps->src);
	if (aps->name)
		free(aps->name);
	for (i=0; i < aps->param_count; i++) {
		free(aps->param_name_t[i]);
		free(aps->param_value_t[i]);
	}
	free(aps->param_name_t);
	free(aps->param_value_t);
	for (i=0; i < aps->url_arg_count; i++) {
		free(aps->url_arg[i]);
		if (aps->ret_filenames[i])
			free(aps->ret_filenames[i]);
	}
	free(aps->url_arg);
	free(aps->ret_filenames);

	if (aps->frame){
		static Atom delete_atom = 0;
		static Atom proto_atom = 0;
		XClientMessageEvent ev;

		if (!delete_atom)
			delete_atom = XInternAtom(XtDisplay(aps->frame),
				"WM_DELETE_WINDOW", False);
		if (!proto_atom)
			proto_atom = XInternAtom(XtDisplay(aps->frame),
				"WM_PROTOCOLS", False);
		ev.type = ClientMessage;
		ev.window = XtWindow(aps->frame); 
		ev.message_type = proto_atom;
		ev.format = 32;
		ev.data.l[0] = delete_atom;   
		ev.data.l[1] = CurrentTime;
		XSendEvent (XtDisplay(aps->frame), XtWindow(aps->frame),
				True, 0x00ffffff, (XEvent *) &ev);

		XFlush(XtDisplay(aps->frame));
		XtSetMappedWhenManaged(aps->frame, False);
		XFlush(XtDisplay(aps->frame));
		XtDestroyWidget(aps->frame);
	}
	free(aps);
}

void AprogPlace(HTMLWidget hw, MarkInfo **mptr, PhotoComposeContext *pcc,
	Boolean save_obj)
{
	char *param_namePtr;
	char *param_valuePtr;
	MarkInfo *amptr = *mptr;
	MarkInfo *pmptr ;
	char *codetypePtr, *srcPtr, *wPtr, *hPtr, *bwPtr, *alignPtr;
	char *namePtr;
	CodeType codetype = CODE_TYPE_UNKNOW;
	int border_width;
	AlignType valignment;
	ElemInfo *eptr;
	AprogInfo *aps = NULL;
	AprogInfo *saved_aps = amptr->s_aps;
	int extra = 0;
	int argcnt ;
	Arg arg[10];
	int baseline = 0;

	fprintf(stderr, "AprogPlace: *x=%d,*y=%d,Width=%d)\n",
			pcc->x,pcc->y,pcc->width_of_viewable_part);

	codetypePtr = ParseMarkTag(amptr->start, MT_APROG, "CODETYPE");
	if (caseless_equal(codetypePtr, "BINARY")) /* If binary, we need to have it locally */
				/* This is a a pluggin */
		codetype = CODE_TYPE_BIN;
	if (caseless_equal(codetypePtr, "SOURCE")) /* If source we need to */
				/* download it, compile and make it a pluggin */
		codetype = CODE_TYPE_SRC;
	if (codetypePtr)
		free(codetypePtr);
	if(codetype == CODE_TYPE_UNKNOW) {
		fprintf(stderr,"Unknow code type in <APROG>\n");
		return;
	}
	srcPtr = ParseMarkTag(amptr->start, MT_APROG, "SRC"); /* src is the url */
				/* of the source file or the name of a pluggin (binary) */
				/* REQUIRED */
	if (!srcPtr && codetype == CODE_TYPE_SRC) {
		fprintf(stderr, "SRC is required in <APROG>\n");
		if(srcPtr)
			free(srcPtr);
		return;
	}
	wPtr = ParseMarkTag(amptr->start, MT_APROG, "WIDTH"); /* REQUIRED */
	hPtr = ParseMarkTag(amptr->start, MT_APROG, "HEIGHT"); /* REQUIRED */
	if (!wPtr || !hPtr ) {
		fprintf(stderr, "WIDTH & HEIGHT required in <APROG>\n");
		if (srcPtr) free(srcPtr);
		if (wPtr) free(wPtr);
		if (hPtr) free(hPtr);
		return;
	}
	bwPtr = ParseMarkTag(amptr->start, MT_APROG, "BORDER"); /* IMPLIED */
	if (!bwPtr || !*bwPtr)
		border_width = IMAGE_DEFAULT_BORDER;
	else 
		if ((border_width = atoi(bwPtr))<0)
			border_width = 0;
				/* In case we have no source or bin get name */
	if (bwPtr) free(bwPtr);
	namePtr = ParseMarkTag(amptr->start, MT_APROG, "NAME");
	if (!namePtr && codetype == CODE_TYPE_BIN) {
		fprintf(stderr, "NAME is required in <APROG>\n");
		if (srcPtr) free(srcPtr);
		if (wPtr) free(wPtr);
		if (hPtr) free(hPtr);
		if (namePtr) free (namePtr);
		return;
	}
	if (strchr(namePtr,'/')) {
		fprintf(stderr, "NAME not secure in <APROG>\n");
		if (srcPtr) free(srcPtr);
		if (wPtr) free(wPtr);
		if (hPtr) free(hPtr);
		if (namePtr) free (namePtr);
		return;
	}
				/* Check if this image will be top aligned */
	alignPtr = ParseMarkTag(amptr->start, MT_APROG, "ALIGN");
	if (caseless_equal(alignPtr, "TOP")) {
		valignment = VALIGN_TOP;
	} else if (caseless_equal(alignPtr, "MIDDLE")) {
		valignment = VALIGN_MIDDLE;
	} else {
		valignment = VALIGN_BOTTOM;
	}
	free(alignPtr);

	aps = (AprogInfo *) calloc(1,sizeof(AprogInfo));
	aps->src = srcPtr;
	aps->name = namePtr;
	aps->height = (atoi(hPtr) * pcc->width_of_viewable_part) / 100;
	aps->width = (atoi(wPtr) * pcc->width_of_viewable_part) / 100;
	aps->frame = NULL;
	free(hPtr);
	free(wPtr);

/* We need to step through mptr in order to find <PARAM name=nnn value="a value"> */
/* Do a Loop as far as we encounter <PARAM>, then loop until we find </APROG> */
	pmptr = amptr->next;
	aps->param_count = 0;
	aps->param_name_t = (char **) malloc(sizeof(char *)); /* alloc one */
	aps->param_value_t = (char**) malloc(sizeof(char *));
	aps->param_name_t[aps->param_count] = NULL;
	aps->param_value_t[aps->param_count] = NULL;

	aps->url_arg_count = 0;
	aps->url_arg = (char **) malloc(sizeof(char *)); /* alloc one */
	aps->url_arg[aps->url_arg_count] = NULL;

	aps->ret_filenames = (char **) malloc(sizeof(char *)); /* alloc one */
	aps->ret_filenames[aps->url_arg_count] = NULL;

	aps->cw_only = pcc->cw_only;

	while (pmptr && ((pmptr->type == M_PARAM) || (pmptr->type == M_NONE))) {
		if (pmptr->type == M_NONE) {
			pmptr = pmptr->next;
			continue;
		}
		param_namePtr = ParseMarkTag(pmptr->start, MT_PARAM, "NAME");
		param_valuePtr = ParseMarkTag(pmptr->start, MT_PARAM, "VALUE");
		if (!param_namePtr)
			continue;
		if (!strcmp(param_namePtr, "_URL_TYPED_ARG")) {
			aps->url_arg[aps->url_arg_count] = param_valuePtr;
			aps->url_arg_count++;
			aps->url_arg = (char**)realloc(aps->url_arg,
				(aps->url_arg_count +1) * sizeof(char *));
			aps->ret_filenames = (char**)realloc(aps->ret_filenames,
				(aps->url_arg_count +1) * sizeof(char *));
			aps->ret_filenames[aps->url_arg_count] = NULL;
			aps->url_arg[aps->url_arg_count] = NULL;
			pmptr = pmptr->next;
			continue;
		}
		aps->param_name_t[aps->param_count] = param_namePtr;
		aps->param_value_t[aps->param_count] = param_valuePtr;
		aps->param_count++;
		aps->param_name_t = (char**)realloc(aps->param_name_t,
					(aps->param_count+1) * sizeof(char *));
		aps->param_value_t = (char**)realloc(aps->param_value_t,
					(aps->param_count+1) * sizeof(char *));
		aps->param_name_t[aps->param_count] = NULL;
		aps->param_value_t[aps->param_count] = NULL;

		pmptr = pmptr->next;
	}
	/* pmptr points on NULL or the next element     */
	while (pmptr && (pmptr->type != M_APROG) && (!pmptr->is_end)) {
		/* Unwind until </APROG>  */
		pmptr = pmptr->next;
	}
	if (!pmptr ) {		/* The end is mandatory */
		fprintf(stderr, "[TriggerMarkChanges] Tag </APROG> not seen\n");
		*mptr = pmptr;
		_FreeAprogStruct(aps);
		return;
	}

	/* Update mptr.  Make it to point to </APROG> */
	*mptr = pmptr;

	baseline = aps->height;

	if (!pcc->preformat) {	 /* If line too long, add LINEFEED  */
		if( (pcc->x + aps->width + extra) >
		    (pcc->eoffsetx + pcc->left_margin + pcc->cur_line_width)) {
			LinefeedPlace(hw, pcc);
		}
	}
        if (pcc->computed_min_x < (aps->width+pcc->eoffsetx+pcc->left_margin)) {
                pcc->computed_min_x = aps->width + pcc->eoffsetx +
			pcc->left_margin;
        }
        if (pcc->x + aps->width > pcc->computed_max_x)
                pcc->computed_max_x = pcc->x + aps->width;

	if (valignment == VALIGN_TOP) {
		baseline = 0;
	} else if (valignment == VALIGN_MIDDLE) {
		baseline = baseline / 2;
	} else {
		valignment = VALIGN_BOTTOM;
	}

	/* Update the item. 'aps' has all the information: size parameter */
	/* etc... set some info in aps */
	aps->ctype = codetype;
	aps->x = pcc->x;
	aps->y = pcc->y;
	aps->border_width = border_width;
	aps->valignment = valignment;
        if (!pcc->cw_only) {            
		pcc->aprog_id++;
                eptr = CreateElement(hw, E_APROG, pcc->cur_font,
                                pcc->x, pcc->y,
				aps->width, aps->height, baseline, pcc);
                eptr->underline_number = 0; /* APROG can't be underlined! */
                eptr->anchor_tag_ptr = pcc->anchor_tag_ptr;
                /* check the max line height. */
                AdjustBaseLine(eptr, pcc); 
                eptr->bwidth=border_width ;  
		eptr->valignment = valignment;
		eptr->aprog_id = pcc->aprog_id;
        } else {
                if (pcc->cur_line_height < aps->height)
                        pcc->cur_line_height = aps->height;
        }


	/* Update pcc */                       
	/* Calculate position */
        pcc->have_space_after = 0;     
        pcc->x = pcc->x + aps->width;
        pcc->is_bol = False; 

	if (pcc->cw_only) { /* Just compute size */
		_FreeAprogStruct(aps);
		return;
	}

	/* Check if all is really freed */
	/* Do callback to get the source.  No callback for binary */
	/* The callback returns the path of the compiled binary */
	/* All we need now is to "plugg-in" */
	/* To do so, we need to create a widget container with right position */
	/* and size.  Unmap up to the first refresh */
	/* We have to fork to activate the program, memorizing it's 'pid' */
	/* and pass as a parameter, XtWindow of the created widget */

	/* Widget creation */
	if (save_obj == False) {		/* It's a creation */
		int i;

		if (aps->ctype == CODE_TYPE_BIN) {
			char cmdline[15000];
			char allcmdline[16000];
			int get_cnt = 0;

			strcpy(cmdline, " ");
			for (i=0; aps->param_name_t[i] != NULL; i++){
				strcat(cmdline, " ");
				strcat(cmdline, aps->param_name_t[i]);
				if (aps->param_value_t[i]){
					strcat(cmdline, " ");
					strcat(cmdline,aps->param_value_t[i]);
				}
			}
			for (i=0; aps->url_arg[i] != NULL; i++){
				EODataStruct eo;

				eo.src = aps->url_arg[i];
				eo.ret_filename = NULL;
				eo.cw_only = pcc->cw_only;
				if (hw->html.get_url_data_cb) {
					strcat(cmdline," ");
					XtCallCallbackList((Widget) hw,
						hw->html.get_url_data_cb,
						(XtPointer) &eo);
					if (eo.ret_filename) {
						strcat(cmdline, eo.ret_filename);
						get_cnt++;
						aps->ret_filenames[i] = 
							eo.ret_filename;
					}
				}
			}
			if (get_cnt == aps->url_arg_count) {
				/* All data id here.  Create */
				XWindowAttributes xwa;
				XSetWindowAttributes xswa;

				argcnt = 0;
				XtSetArg(arg[argcnt], XmNx, aps->x); argcnt++;
				XtSetArg(arg[argcnt], XmNy, aps->y); argcnt++;
				XtSetArg(arg[argcnt], XmNwidth, aps->width); argcnt++;
				XtSetArg(arg[argcnt], XmNheight, aps->height); argcnt++;
				aps->frame = XmCreateLabel(hw->html.view, 
					"If this text appear then APROG is not running", arg, argcnt);
				XtSetMappedWhenManaged(aps->frame, False);
				XtManageChild(aps->frame);
				XFlush(XtDisplay(hw));
				XGetWindowAttributes(XtDisplay(aps->frame),
					XtWindow(aps->frame),&xwa);
/*				printf("xwa.do_not_propagate_mask = %x\n",
					xwa.do_not_propagate_mask);
*/
				xswa.do_not_propagate_mask = xwa.do_not_propagate_mask & (~(PointerMotionMask));
				XChangeWindowAttributes(XtDisplay(aps->frame),
					XtWindow(aps->frame),CWDontPropagate,
					&xswa);
				XFlush(XtDisplay(hw));
/*
				XGetWindowAttributes(XtDisplay(aps->frame),
					XtWindow(aps->frame),&xwa);
				printf("xwa.do_not_propagate_mask = %x\n",
					xwa.do_not_propagate_mask);

				printf("Aprog create Window = %d\n",
						XtWindow(aps->frame));
*/
				sprintf(allcmdline,"/usr/local/mMosaic/bin/%s -windowId %d %s &",
					namePtr, XtWindow(aps->frame),cmdline );

/*
				printf("Executing : %s\n", allcmdline);
*/
				system(allcmdline);
				if (saved_aps)
					fprintf(stderr,
						"APROG bug when Create\n");
				amptr->s_aps = aps ;
			}
		}
		eptr->aps = aps;
	} else {		/* Use the old window.  It's a Resize */
		if (!saved_aps) {
			fprintf(stderr, "Abug in APROG when resizing\n");
		} else {
			eptr->aps = saved_aps;
			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, aps->x); argcnt++;
			XtSetArg(arg[argcnt], XmNy, aps->y); argcnt++;
			XtSetArg(arg[argcnt], XmNwidth, aps->width); argcnt++;
			XtSetArg(arg[argcnt], XmNheight, aps->height); argcnt++;
			XtSetValues(saved_aps->frame, arg, argcnt);
			XtSetMappedWhenManaged(saved_aps->frame, False);
			XtManageChild(saved_aps->frame);
			XFlush(XtDisplay(hw));
			_FreeAprogStruct(aps);
		}
	}
}

void AprogRefresh(HTMLWidget hw, ElemInfo *eptr)
{
	int x;
	int y;
	Position px;
	Position py;
	Arg args[3];

/* fprintf(stderr,"[PlaceLine] need E_APROG tool\n"); */
/* Do a unmap/map of the created widget in 'case M_APROG' */
/* See ImageRefresh as an example of how to place a Widget */
/* Careful with X and Y position which MUST be 'short' */
/* x,y in eptr are integers, X-Window uses shorts */
/* calculate the scroll-bar so it "fits" */
/*	x = x - hw->html.scroll_x; */
/*	y = y - hw->html.scroll_y; */

	if (!eptr->aps || !eptr->aps->frame) return;
	x = eptr->x;
	y = eptr->y;
	px = x - hw->html.scroll_x;
	py = y - hw->html.scroll_y;
	XtSetArg(args[0], XtNx, px);
	XtSetArg(args[1], XtNy, py);
	XtSetValues(eptr->aps->frame, args, 2);
	XtSetMappedWhenManaged(eptr->aps->frame, True);
}
