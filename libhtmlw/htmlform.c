/* Some part of this file is Copyright (C) 1996 - G.Dauphin
 * See the file "license.mMosaic" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */ 

/* Copyright (C) 1998, 1999, 2004, 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"
#include <Xm/TextF.h>
#include <stdio.h>
#include <stdlib.h>

#include "HTMLP.h"
#include "HTMLPutil.h"
#include "../src/mosaic.h"
#include "HTMLform.h"
#include "../libnut/str-tools.h"

#ifndef DISABLE_TRACE
extern int reportBugs;
extern int htmlwTrace;
#endif

/* Fillout forms.  Cannot be nested. */

/* Get title, if any, in pcc */
static void GetTitle(MarkInfo *mark, PhotoComposeContext *pcc)
{
	if (pcc->mark_title)
		free(pcc->mark_title);
	pcc->mark_title = ParseMarkTag(mark->start, "A", "title");
	if (pcc->mark_title && !*pcc->mark_title) {
		free(pcc->mark_title);
		pcc->mark_title = NULL;
	}
}


/* <FORM> */
void BeginForm(HTMLWidget hw, MarkInfo **mptr, PhotoComposeContext *pcc)
{
	MarkInfo *mark = *mptr;

	if (!pcc->cw_only && (pcc->in_form || pcc->cur_form)) {
#ifndef DISABLE_TRACE
		if (htmlwTrace || reportBugs)
			fprintf(stderr, "Warning: A Form in Form!\n");
#endif
		return;
	}

	/* Create a Form structure */
	pcc->cur_form = (FormInfo *)malloc(sizeof(FormInfo));
	pcc->cur_form->next = NULL;
	pcc->cur_form->hw = (Widget)hw;
	pcc->cur_form->action = ParseMarkTag(mark->start, MT_FORM, "ACTION");
	pcc->cur_form->target = ParseMarkTag(mark->start, MT_FORM, "TARGET");
	pcc->cur_form->method = ParseMarkTag(mark->start, MT_FORM, "METHOD");
	pcc->cur_form->enctype = ParseMarkTag(mark->start, MT_FORM, "ENCTYPE");
	pcc->cur_form->start = pcc->widget_id;
	pcc->cur_form->end = -1;
	pcc->cur_form->button_pressed = NULL;
	pcc->in_form = True;
	pcc->cur_form->cw_only = pcc->cw_only;
}

/* </FORM> */
void EndForm(HTMLWidget hw, PhotoComposeContext *pcc)
{
	if (!pcc->in_form || !pcc->cur_form)	/* It's an error */
		return;

	if (pcc->cw_only) {
		pcc->in_form = False;  /* Always put False on end Form
				        * because form cannot be nested */
		if (pcc->cur_form->cw_only) {
			if (pcc->cur_form->action)
				free(pcc->cur_form->action);
			if (pcc->cur_form->target)
				free(pcc->cur_form->target);
			if (pcc->cur_form->method)
				free(pcc->cur_form->method);
			if (pcc->cur_form->enctype)
				free(pcc->cur_form->enctype);
			free(pcc->cur_form);
		}
		pcc->cur_form = NULL;
		return;
	}

	/* Here we go to create it really */
	pcc->cur_form->end = pcc->widget_id;
	AddNewForm(hw, pcc->cur_form);
	pcc->cur_form = NULL;
	pcc->in_form = False;  /* Always put False on end Form */
}

/* <INPUT>
 * Special case the type=image stuff to become a special IMG tag.
 */
void FormInputField(HTMLWidget hw, MarkInfo **mptr, PhotoComposeContext *pcc)
{
	MarkInfo *mark = *mptr;
	char *tptr;

	/* Do not check if in form, so we can display example fields
	 * outside of forms.
	 */

	if (!(tptr = ParseMarkTag(mark->start, MT_INPUT, "TYPE"))) {
		WidgetPlace(hw, *mptr, pcc);
		return;
	} else if (caseless_equal(tptr, "image")) {
		char *tptr2;

		free(tptr);
		if (pcc->cw_only) {
			/* Get width and height of image */
			ImagePlace(hw, *mptr, pcc);
			return;
		}
		tptr = (char *)malloc(strlen(mark->start) +
				      strlen(" ISMAP") + strlen(MT_IMAGE) -
				      strlen(MT_INPUT) + 1);
		strcpy(tptr, MT_IMAGE);
		strcat(tptr, (char *) (mark->start + strlen(MT_INPUT)));
		strcat(tptr, " ISMAP");
		tptr2 = mark->start;
		mark->start = tptr;
		ImagePlace(hw, *mptr, pcc);
		mark->start = tptr2;
	} else if (caseless_equal(tptr, "hidden")) {
		/* Hidden inputs have no element associated with them,
		 * just a widget record. */
		pcc->widget_id++;
		(void)MakeWidget(hw, mark->start, pcc, pcc->widget_id);
	} else {
		WidgetPlace(hw, *mptr, pcc);
	}
	free(tptr);
}

/* TEXTAREA is a replacement for INPUT type=text size=rows,cols
 * name REQUIRED
 * rows REQUIRED
 * cols REQUIRED
 */             
void FormTextAreaBegin(MarkInfo **mptr, PhotoComposeContext *pcc)
{
	char *buf;
	int len;        
	MarkInfo *mark = *mptr;

	/* Do not check if in form, so we can display example fields
	 * outside of forms.
	 */

	if (!pcc->text_area_buf) {
		/* Construct the start of a fake INPUT tag. */
		len = strlen(MT_INPUT) + strlen(" type=textarea value=\"\"");
		buf = (char *)malloc(len + strlen(mark->start) + 1);
		strcpy(buf, MT_INPUT);
		strcat(buf, (char *) (mark->start + strlen(MT_TEXTAREA)));
		strcat(buf, " type=textarea value=\"");
		pcc->text_area_buf = buf;  
	}                      
}

void FormTextAreaEnd(HTMLWidget hw, MarkInfo **mptr, PhotoComposeContext *pcc)
{
	MarkInfo *mark = *mptr;
	char *buf;

	if (!pcc->text_area_buf)
		return;

	/* Finish a fake INPUT tag. */
	buf = (char *)malloc(strlen(pcc->text_area_buf) + 2); 
	strcpy(buf, pcc->text_area_buf);
	strcat(buf, "\"");
	/* Stick the fake in. */
	mark->start = buf;
	GetTitle(mark, pcc);
	mark->is_end = 0;
	WidgetPlace(hw, mark, pcc);
	/* Free the fake */
	free(buf);
	free(pcc->text_area_buf);
	mark->start = NULL;	/* Always NULL for end tag. */
	mark->is_end = 1;
	pcc->text_area_buf = NULL;
}

/* We've just terminated the current OPTION.
 * Put it in the proper place in the SelectInfo structure.
 * Move option_buf into options, and maybe copy into
 * value if is_value is set.    
 */     
static void ProcessOption(SelectInfo *sptr)
{       
        int cnt;             
        
	if (!sptr->options) {
		sptr->options = (char **)malloc(1024 * sizeof(char *));
		sptr->returns = (char **)malloc(1024 * sizeof(char *));
		sptr->labels = (char **)malloc(1024 * sizeof(char *));
	} else if (sptr->option_cnt == 1023) {
		sptr->options = (char **)realloc(sptr->options,
						 2048 * sizeof(char *));
		sptr->returns = (char **)realloc(sptr->returns,
						 2048 * sizeof(char *));
		sptr->labels = (char **)realloc(sptr->labels,
						2048 * sizeof(char *));
	} else if (sptr->option_cnt == 2047) {
		return;
	}
        cnt = sptr->option_cnt++;

	if (sptr->option_buf)
	        clean_white_space(sptr->option_buf);
        sptr->options[cnt] = sptr->option_buf;

	/* Use text string if no VALUE specified */
	if (sptr->retval_buf) {
	        sptr->returns[cnt] = sptr->retval_buf;
	} else if (sptr->option_buf) {
		sptr->returns[cnt] = strdup(sptr->option_buf);
	} else {
		sptr->returns[cnt] = NULL;
	}
        sptr->labels[cnt] = sptr->label_buf;

        if (sptr->is_value) {
                cnt = sptr->value_cnt++;
                sptr->value = (char **)realloc(sptr->value,
					      sptr->value_cnt * sizeof(char *));
		if (sptr->option_buf) {
	                sptr->value[cnt] = strdup(sptr->option_buf);
		} else {
	                sptr->value[cnt] = NULL;
		}
        }       
}        

/* <OPTION>  Can only be inside a SELECT tag. */
void FormSelectOptionField(MarkInfo **mptr, PhotoComposeContext *pcc)
{
	if (pcc->in_select && pcc->current_select) {
		MarkInfo *mark = *mptr;
		char *tptr;

		if (pcc->current_select->option_buf)
			ProcessOption(pcc->current_select);
		pcc->current_select->option_buf = (char *)calloc(1, 1);
		/* Check if this option starts selected */
		if (tptr = ParseMarkTag(mark->start, MT_OPTION, "SELECTED")) {
			pcc->current_select->is_value = 1;
			free(tptr);
		} else {
			pcc->current_select->is_value = 0;
		}       
		/* Check if this option has a different return value field. */
		if (tptr = ParseMarkTag(mark->start, MT_OPTION, "VALUE")) {
			pcc->current_select->retval_buf = tptr;
		} else {       
			pcc->current_select->retval_buf = NULL;
		}              
		if (tptr = ParseMarkTag(mark->start, MT_OPTION, "LABEL")) {
			pcc->current_select->label_buf = tptr;
		} else {       
			pcc->current_select->label_buf = NULL;
		}              
	}                      
}

/* <OPTGROUP> Can only be inside a SELECT tag. */
void FormSelectOptgroup(MarkInfo **mptr, PhotoComposeContext *pcc)
{
	if (pcc->in_select && pcc->current_select) {
		MarkInfo *mark = *mptr;
		char *tptr;

		if (pcc->current_select->option_buf)
			ProcessOption(pcc->current_select);
		pcc->current_select->option_buf = strdup("MOSAIC_OPTGROUP");
		if (tptr = ParseMarkTag(mark->start, MT_OPTGROUP, "LABEL")) {
			pcc->current_select->label_buf = tptr;
		} else {       
			pcc->current_select->label_buf = strdup(" ");
		}              
		pcc->current_select->is_value = 0;
		pcc->current_select->retval_buf = NULL;
		ProcessOption(pcc->current_select);
		pcc->current_select->option_buf = NULL;
	}                      
}

/* <SELECT>  Allows an option menu or a scrolled list.
 * Due to a restriction in SGML, this can't just be a subset of
 * the INPUT markup.  However, can be treated that way to avoid duplicating
 * code.  As a result combine SELECT and OPTION into a faked up INPUT mark.
 */
void FormSelectBegin(HTMLWidget hw, MarkInfo **mptr, PhotoComposeContext *pcc)
{
	/* Do not check if in form, so we can display examples
	 * outside of forms.
	 */

	if (!pcc->current_select) {
		pcc->current_select = (SelectInfo *)calloc(1,
							    sizeof(SelectInfo));
		pcc->current_select->hw = (Widget)hw;
		pcc->current_select->mptr = *mptr;
		/** calloc sets them
		pcc->current_select->option_cnt = 0;
		pcc->current_select->returns = NULL;
		pcc->current_select->retval_buf = NULL;
		pcc->current_select->options = NULL;
		pcc->current_select->option_buf = NULL;
		pcc->current_select->labels = NULL;
		pcc->current_select->label_buf = NULL;
		pcc->current_select->value_cnt = 0;
		pcc->current_select->value = NULL;
		**/
		pcc->current_select->is_value = -1;
		pcc->ignore = 1;
	}                      
	pcc->in_select = True;
}

void FormSelectEnd(HTMLWidget hw, PhotoComposeContext *pcc)
{
	int len;  
	char *start, *buf, *options, *returns, *labels, *value;

	if (!pcc->current_select)
		return;

	GetTitle(pcc->current_select->mptr, pcc);

	if (pcc->current_select->option_buf)
		ProcessOption(pcc->current_select);
	options = ComposeCommaList(pcc->current_select->options,
				   pcc->current_select->option_cnt);
	returns = ComposeCommaList(pcc->current_select->returns,
				   pcc->current_select->option_cnt);
	labels = ComposeCommaList(pcc->current_select->labels,
				  pcc->current_select->option_cnt);
	value = ComposeCommaList(pcc->current_select->value,
				 pcc->current_select->value_cnt);
	FreeCommaList(pcc->current_select->options,
		      pcc->current_select->option_cnt);   
	FreeCommaList(pcc->current_select->returns,
		      pcc->current_select->option_cnt);
	FreeCommaList(pcc->current_select->labels,
		      pcc->current_select->option_cnt);
	FreeCommaList(pcc->current_select->value,
		      pcc->current_select->value_cnt);
	/* Construct a fake INPUT tag. */
	len = strlen(MT_INPUT) + strlen(options) +
		   strlen(returns) + strlen(value) + strlen(labels) +
		   strlen(" type=select options=\"\" returns=\"\" value=\"\"") +
		   strlen(" labels=\"\"");
	buf = (char *)malloc(len + strlen(pcc->current_select->mptr->start) +1);
	strcpy(buf, MT_INPUT);
	strcat(buf, " type=select options=\"");
	strcat(buf, options);
	strcat(buf, "\" returns=\"");
	strcat(buf, returns);
	strcat(buf, "\" labels=\"");
	strcat(buf, labels);
	strcat(buf, "\" value=\"");
	strcat(buf, value);
	strcat(buf, "\"");  
	strcat(buf, (char *) (pcc->current_select->mptr->start +
			      strlen(MT_SELECT)));
	/* Stick the fake in, saving the real one. */
	start = pcc->current_select->mptr->start;
	pcc->current_select->mptr->start = buf;
	WidgetPlace(hw, pcc->current_select->mptr, pcc);
	/* Free the fake, put the original back */
	free(buf);
	free(options);
	free(returns);
	free(labels);
	free(value);
	pcc->current_select->mptr->start = start;
	free((char *)pcc->current_select);
	pcc->current_select = NULL;
	pcc->ignore = 0;
	pcc->in_select = False;
}

/* <BUTTON> */
void FormButtonBegin(HTMLWidget hw, MarkInfo **mptr, PhotoComposeContext *pcc)
{
	int len;        
	MarkInfo *mark = *mptr;
	char *type, *buf;

	if (pcc->button_buf)
		return;

	type = ParseMarkTag(mark->start, MT_BUTTON, "TYPE");
	/* Construct the start of a fake INPUT tag. */
	len = strlen(MT_INPUT) + strlen(" type=submit ISBUTTON text=\"\"");
	buf = (char *)malloc(len + strlen(mark->start) + 1);
	strcpy(buf, MT_INPUT);
	if (!type || !*type)
		/* Give it the default type */
		strcat(buf, " type=submit");
	if (type)
		free(type);
	strcat(buf, (char *) (mark->start + strlen(MT_BUTTON)));
	strcat(buf, " ISBUTTON text=\"");
	pcc->button_buf = buf;  
	pcc->button_has_text = False;
	pcc->button_has_image = False;
}

/* </BUTTON> */
void FormButtonEnd(HTMLWidget hw, MarkInfo **mptr, PhotoComposeContext *pcc)
{
	MarkInfo *mark = *mptr;
	char *buf;

	if (!pcc->button_buf)
		return;

	if (pcc->button_has_text || !pcc->button_has_image) {
		/* Finish fake INPUT tag. */
		buf = (char *)malloc(strlen(pcc->button_buf) + 8); 
		strcpy(buf, pcc->button_buf);
		if (!pcc->button_has_text) {
			char *tptr = ParseMarkTag(buf, MT_INPUT, "TYPE");

			if (tptr) {
				if (!my_strcasecmp(tptr, "reset")) {
					strcat(buf, "Reset");
				} else {
					strcat(buf, "Submit");
				}
				free(tptr);
			}
		}
		strcat(buf, "\"");
		/* Stick the fake in. */
		mark->start = buf;
		GetTitle(mark, pcc);
		mark->is_end = 0;
		WidgetPlace(hw, mark, pcc);
		/* Free the fake. */
		free(buf);
		mark->start = NULL;	/* Always NULL for end tag. */
		mark->is_end = 1;
	}
	free(pcc->button_buf);
	pcc->button_buf = NULL;
}
