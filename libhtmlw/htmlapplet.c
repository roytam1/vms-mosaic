/* HTMLapplet.c
 * Version 3.0.14 [Apr97]
 *
 * Copyright (C) 1997 - G.Dauphin
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

void _FreeAppletStruct(AppletInfo * ats)
{
	int i;

	if (ats->src)
		free(ats->src);
	for (i = 0; i < ats->param_count; i++) {
		free(ats->param_name_t[i]);
		free(ats->param_value_t[i]);
	} 
	free(ats->param_name_t);
	free(ats->param_value_t);
	for (i = 0; i < ats->url_arg_count; i++) {
		free(ats->url_arg[i]);
		if (ats->ret_filenames[i])
			free(ats->ret_filenames[i]);
	} 
	free(ats->url_arg);
	free(ats->ret_filenames);
	if (ats->frame) {
		static Atom delete_atom = 0;
		static Atom proto_atom = 0;
		XClientMessageEvent ev;

		if (!delete_atom)
			delete_atom = XInternAtom(XtDisplay(ats->frame),
				"WM_DELETE_WINDOW", False);
		if (!proto_atom)
			proto_atom = XInternAtom(XtDisplay(ats->frame),
				"WM_PROTOCOLS", False);
		fprintf(stderr, "****Mosaic*** send WM_DELETE_WINDOW****\n");
		ev.type = ClientMessage;
		ev.window = XtWindow(ats->frame);
		ev.message_type = proto_atom;
		ev.format = 32;
		ev.data.l[0] = delete_atom;
		ev.data.l[1] = CurrentTime;
		XSendEvent (XtDisplay(ats->frame),
			XtWindow(ats->frame), True, 0x00ffffff, (XEvent *) &ev);
		XFlush(XtDisplay(ats->frame));
		XtSetMappedWhenManaged(ats->frame, False);
		XFlush(XtDisplay(ats->frame));
		XtDestroyWidget(ats->frame);
	}
	free(ats); 
}

void AppletPlace(HTMLWidget hw, MarkInfo **mptr, PhotoComposeContext *pcc,
		 Boolean save_obj)
{
	char *param_namePtr;
	char *param_valuePtr;
	MarkInfo *amptr = *mptr;
	MarkInfo *pmptr;
	char *srcPtr, *wPtr, *hPtr, *bwPtr, *alignPtr;
	CodeType codetype = CODE_TYPE_APPLET;
	int border_width;
	AlignType valignment;
	ElemInfo *eptr;
	AppletInfo *ats = NULL;
	AppletInfo *saved_ats = amptr->s_ats;
	int extra = 0;
	int argcnt;
	Arg arg[10];
	int baseline = 0;

	fprintf(stderr, "AppletPlace: *x=%d,*y=%d,Width=%d)\n",
		pcc->x,pcc->y,pcc->width_of_viewable_part);

	/* code=applet.class is the file name we need to download */
	/* this is a java-code	*/
	/* The absolute name to consider is made of URL_du_document +
	 * applet.class */
	srcPtr = ParseMarkTag(amptr->start, MT_APPLET, "code"); 
	if (!srcPtr) {
		fprintf(stderr, "CODE is required in <APPLET>\n");
		return;
	}
	wPtr = ParseMarkTag(amptr->start, MT_APPLET, "WIDTH");  /* REQUIRED */
	hPtr = ParseMarkTag(amptr->start, MT_APPLET, "HEIGHT");  /* REQUIRED */
	if (!wPtr || !hPtr) {
		fprintf(stderr, "WIDTH & HEIGHT required in <APPLET>\n");
		if (srcPtr)
			free(srcPtr);
		if (wPtr)
			free(wPtr);
		if (hPtr)
			free(hPtr);
		return;
	}
	bwPtr = ParseMarkTag(amptr->start, MT_APPLET, "BORDER");  /* IMPLIED */
	if (!bwPtr || !*bwPtr) {
		border_width = IMAGE_DEFAULT_BORDER;
	} else if ((border_width = atoi(bwPtr)) < 0) {
		border_width = 0;
	}
	if (bwPtr)
		free(bwPtr);
	/* Check if this image will be top aligned */
	alignPtr = ParseMarkTag(amptr->start, MT_APPLET, "ALIGN");
	if (caseless_equal(alignPtr, "TOP")) {
		valignment = VALIGN_TOP;
	} else if (caseless_equal(alignPtr, "MIDDLE")) {
		valignment = VALIGN_MIDDLE;
	} else {
		valignment = VALIGN_BOTTOM;
	}
	free(alignPtr);

	ats = (AppletInfo *) malloc(sizeof(AppletInfo));
	ats->height = atoi(hPtr);	/* In pixels */
	ats->width = atoi(wPtr);
	ats->frame = NULL;
	free(hPtr);
	free(wPtr);

	/* We must move mptr until we find <PARAM name=nnn value="a value"> */
	/* Loop as far as we find <PARAM>, the loop up to </APPLET> */
	pmptr = amptr->next;
	ats->param_count = 0;
	ats->param_name_t = (char **) malloc(sizeof(char *));  /* alloc one */
	ats->param_value_t = (char **) malloc(sizeof(char *));
	ats->param_name_t[ats->param_count] = NULL;
	ats->param_value_t[ats->param_count] = NULL;

	ats->url_arg_count = 0;
	ats->url_arg = (char **) malloc(sizeof(char *));  /* alloc one */
	ats->url_arg[ats->url_arg_count] = NULL;

	ats->ret_filenames = (char **) malloc( sizeof(char *));  /* alloc one */
	ats->ret_filenames[ats->url_arg_count] = NULL;

	/*  Prepare take-java-code request */
	ats->url_arg[ats->url_arg_count] = srcPtr;
	ats->url_arg_count++;
	ats->url_arg = (char**)realloc(ats->url_arg,
		(ats->url_arg_count + 1) * sizeof(char *));
	ats->ret_filenames = (char**)realloc(ats->ret_filenames,
		(ats->url_arg_count + 1) * sizeof(char *));
	ats->ret_filenames[ats->url_arg_count] = NULL;
	ats->url_arg[ats->url_arg_count] = NULL;

	ats->cw_only = pcc->cw_only;

	while (pmptr && ((pmptr->type == M_PARAM) || (pmptr->type == M_NONE))) {
		if (pmptr->type == M_NONE){ 	/* We skip the text */
			pmptr = pmptr->next;
			continue;
		}
			/* Unwind and save all PARAM */
			/*####<PARAM NAME=param_name VALUE=param_value> */
		param_namePtr = ParseMarkTag(pmptr->start, MT_PARAM, "NAME");
		param_valuePtr = ParseMarkTag(pmptr->start, MT_PARAM, "VALUE");
		if ( !param_namePtr)
			continue;
		ats->param_name_t[ats->param_count] = param_namePtr;
		ats->param_value_t[ats->param_count] = param_valuePtr;
		ats->param_count++;
		ats->param_name_t = (char **)realloc(ats->param_name_t,
				       (ats->param_count + 1) * sizeof(char *));
		ats->param_value_t = (char **)realloc(ats->param_value_t,
				       (ats->param_count + 1) * sizeof(char *));
		ats->param_name_t[ats->param_count] = NULL;
		ats->param_value_t[ats->param_count] = NULL;

		pmptr = pmptr->next;
	}
	/* pmptr points on NULL or the next element */
	while (pmptr && (pmptr->type != M_APPLET) && !pmptr->is_end)
		pmptr = pmptr->next;

	if (!pmptr) {
		/* The end is mandatory */
		fprintf(stderr, "Tag </APPLET> not seen\n");
		*mptr = pmptr;
		_FreeAppletStruct(ats);
		return;
	}

	/* Update mptr. Position it on </APPLET> */
	*mptr = pmptr;

	baseline = ats->height;

	if (!pcc->preformat) {	 /* If line too long, add LINEFEED */
		if ((pcc->x + ats->width + extra) >
		    (pcc->eoffsetx + pcc->left_margin + pcc->cur_line_width))
			LinefeedPlace(hw, pcc);
	}
        if (pcc->computed_min_x <
	    (ats->width + pcc->eoffsetx + pcc->left_margin))
                pcc->computed_min_x = ats->width + pcc->eoffsetx +
				      pcc->left_margin;

        if (pcc->x + ats->width > pcc->computed_max_x)
                pcc->computed_max_x = pcc->x + ats->width;

	if (valignment == VALIGN_TOP) {
		baseline = 0;
	} else if (valignment == VALIGN_MIDDLE) {
		baseline = baseline / 2;
	} else {
		valignment = VALIGN_BOTTOM;
	}

	/* Update the item. 'ats' contains all informations : size parameter */
	/* etc...  set some info in ats */

	ats->ctype = codetype;
	ats->src = srcPtr;
	ats->x = pcc->x;
	ats->y = pcc->y;
	ats->border_width = border_width;
	ats->valignment = valignment;
        if (!pcc->cw_only) {
		pcc->applet_id++;
                eptr = CreateElement(hw, E_APPLET, pcc->cur_font,
                                     pcc->x, pcc->y,
				     ats->width, ats->height, baseline, pcc);
                eptr->underline_number = 0;  /* APPLET can't be underlined! */
                eptr->anchor_tag_ptr = pcc->anchor_tag_ptr;
                /* Check the max line height. */
                AdjustBaseLine(eptr, pcc); 
		eptr->ats = ats;
                eptr->bwidth = border_width;  
		eptr->valignment = valignment;
		eptr->applet_id = pcc->applet_id;
        } else if (pcc->cur_line_height < ats->height) {
                pcc->cur_line_height = ats->height;
        }

	/* Update pcc */                       
	/* Calculate position */
        pcc->have_space_after = 0;     
        pcc->x = pcc->x + ats->width;
        pcc->is_bol = False; 

	if (pcc->cw_only) {
		_FreeAppletStruct(ats);
		return;
	}

	/* No callback for binary */
	/* The callback returns the path of the compiled binary */
	/* All we need now is to "plugg-in" */
	/* To do so, we need to create a widget container with right position */
	/* and the right size (?) */
	/* Unmap up to the first refresh */
	/* We have to fork to activate the program, memorizing it's 'pid' */ 
	/* and pass as a parameter, XtWindow of the created widget */

/* Widget creation */

/*	argcnt = 0;
 *	XtSetArg(arg[argcnt], XmNx, ats->x); argcnt++;
 *	XtSetArg(arg[argcnt], XmNy, ats->y); argcnt++;
 *	XtSetArg(arg[argcnt], XmNshadowType, XmSHADOW_IN); argcnt++;
 *	ats->frame = XmCreateFrame(hw->html.view, "Frame",arg, argcnt);
 *	argcnt = 0;
 *	XtSetArg(arg[argcnt], XmNwidth, ats->width); argcnt++;
 *	XtSetArg(arg[argcnt], XmNheight, ats->height); argcnt++;
 *	ats->w = XmCreateDrawingArea(ats->frame, "Applet", arg, argcnt);
 *	XtManageChild(ats->w);
 *	XtSetArg(arg[argcnt], XmNuserData, (XtPointer)ats->w); argcnt++;
 *	XtSetValues(ats->frame, arg, argcnt);
 *	XtSetMappedWhenManaged(ats->frame, False);
 *	XtManageChild(ats->frame);
 *	XFlush(hw->html.dsp);
*/
	if (save_obj == False) {		/* It's a creation */
		int i;
		EODataStruct eo;
		char cmdline[15000];
		char allcmdline[16000];
		char zfile[1000];
		char *tmp, *indot;
		int get_cnt = 0;

		/* Load applet's code	*/
		eo.src = ats->url_arg[0];
		tmp = eo.src;
		strcpy(zfile, eo.src);
		indot = strrchr(zfile, '.');
		if (indot && !strcmp(indot, ".class")) {
			*indot = '.';
			indot[1] = 'z';
			indot[2] = 'i' ;
			indot[3] = 'p';
			indot[4] = '\0';
		}
		eo.src = zfile;
		eo.ret_filename = NULL;
		eo.cw_only = pcc->cw_only;
		strcpy(cmdline, " ");
		if (hw->html.get_url_data_cb) {
			strcat(cmdline, " ");
			XtCallCallbackList((Widget) hw,
					   hw->html.get_url_data_cb,
					   (XtPointer) &eo);
			if (eo.ret_filename) {
				sprintf(allcmdline,
					"mv %s $HOME/.mMosaic/classes/%s",
					eo.ret_filename, eo.src);
				system(allcmdline);
				eo.src = tmp;
/*				strcat(cmdline, eo.ret_filename); */
				indot = strrchr(eo.src, '.');
				if (indot && !strcmp(indot, ".class"))
					*indot = '\0';
				strcat(cmdline, eo.src);
				/* The applet is in the file eo.ret_filename */
				get_cnt++;
			}
		}

		/* parameters */
		for (i = 0; ats->param_name_t[i]; i++) {
			strcat(cmdline, " ");
			strcat(cmdline, ats->param_name_t[i]);
			if (ats->param_value_t[i]) {
				strcat(cmdline, " ");
				strcat(cmdline, ats->param_value_t[i]);
			}
		}

		if (get_cnt == ats->url_arg_count) {
			XWindowAttributes xwa;
			XSetWindowAttributes xswa;

			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, ats->x); argcnt++;
			XtSetArg(arg[argcnt], XmNy, ats->y); argcnt++;
			XtSetArg(arg[argcnt], XmNwidth, ats->width); argcnt++;
			XtSetArg(arg[argcnt], XmNheight, ats->height); argcnt++;
			ats->frame = XmCreateLabel(hw->html.view, 
				"If this text appears, then APPLET is not running", arg, argcnt);
			XtSetMappedWhenManaged(ats->frame, False);
			XtManageChild(ats->frame);
			XFlush(hw->html.dsp);
			XGetWindowAttributes(XtDisplay(ats->frame),
					     XtWindow(ats->frame), &xwa);
/*			printf("xwa.do_not_propagate_mask = %x\n",
				xwa.do_not_propagate_mask);
*/
			xswa.do_not_propagate_mask = xwa.do_not_propagate_mask &
						     (~(PointerMotionMask));
			XChangeWindowAttributes(XtDisplay(ats->frame),
				XtWindow(ats->frame), CWDontPropagate, &xswa);
			XFlush(hw->html.dsp);
/*
			XGetWindowAttributes(XtDisplay(ats->frame),
				XtWindow(ats->frame), &xwa);
			printf("xwa.do_not_propagate_mask = %x\n",
				xwa.do_not_propagate_mask);

			printf("Applet create Window = %d\n",
			       XtWindow(ats->frame));
*/
			sprintf(allcmdline,
				"/usr/local/mMosaic/bin/mMosaicAppletViewer -windowId %d %s &",
				XtWindow(ats->frame), cmdline );

			printf("Executing : %s\n", allcmdline);
			system(allcmdline);
			amptr->s_ats = ats;
		}
		eptr->ats = ats;
	} else {		/* Use the old window.  It's a Resize */
		if (!saved_ats){
			fprintf(stderr, "Abug in APPLET when resizing\n");
		} else {
			eptr->ats = saved_ats;
			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, ats->x); argcnt++;
			XtSetArg(arg[argcnt], XmNy, ats->y); argcnt++;
			XtSetArg(arg[argcnt], XmNwidth, ats->width); argcnt++;
			XtSetArg(arg[argcnt], XmNheight, ats->height); argcnt++;
			XtSetValues(saved_ats->frame, arg, argcnt);
			XtSetMappedWhenManaged(saved_ats->frame, False);
			XtManageChild(saved_ats->frame);
			XFlush(hw->html.dsp);
			_FreeAppletStruct(ats);	
		}
	}

	if (srcPtr)
		free(srcPtr);
	if (wPtr)
		free(wPtr);
	if (hPtr)
		free(hPtr);
	if (bwPtr)
		free(bwPtr);
}

void AppletRefresh(HTMLWidget hw, ElemInfo *eptr)
{
	int x, y;
	Position px, py;
	Arg args[3];

/* fprintf(stderr,"[PlaceLine] need E_APPLET tool\n"); */
/* Do a unmap/map of the created widget in 'case M_APPLET' */
/* See ImageRefresh as an example of how to place a Widget */
/* Careful with X and Y position which MUST be 'short' */
/* x,y in eptr are integers, X-Window uses shorts */
/* calculate the scroll-bar so it "fits" */
/*	x = x - hw->html.scroll_x; */
/*	y = y - hw->html.scroll_y; */

	if (!eptr->ats || !eptr->ats->frame) return;
	x = eptr->x;
	y = eptr->y;
	px = x - hw->html.scroll_x;
	py = y - hw->html.scroll_y;
	XtSetArg(args[0], XtNx, px);
	XtSetArg(args[1], XtNy, py);
	XtSetValues(eptr->ats->frame, args,2);
	XtSetMappedWhenManaged(eptr->ats->frame, True);
}
