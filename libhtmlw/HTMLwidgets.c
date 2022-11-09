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

/* Copyright (C) 2004, 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"

#include "../libnut/str-tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <Xm/Xm.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/Scrollbar.h>

#include "HTMLmiscdefs.h"
#include "HTMLparse.h"
#include "HTMLP.h"
#include "HTMLPutil.h"
#include "HTMLwidgets.h"

#define X_NAME	"x"
#define Y_NAME	"y"

#ifndef DISABLE_TRACE
extern int reportBugs;
extern int htmlwTrace;
#endif

extern int LimDimY;

int skip_traversal_current = 0;

static char **ParseCommaList(char *str, int *count);
static char *MapOptionReturn(char *val, char **mapping);

static char traversal_table[] = "\
    ~Shift ~Meta ~Ctrl <Key> Tab:  traversal_forward()\n\
    Shift ~Meta ~Ctrl <Key> Tab:   traversal_back()\n\
    Ctrl <Key> Tab:    		   traversal_end()";

/* Disable dran and drop */
static char button_translations[] = "\
    <Btn2Down>:       take_focus()";

static char text_translations[] = "\
   ~Meta ~Alt Ctrl<Key>u:	beginning-of-line()		\
				delete-to-end-of-line()		\n\
   ~Meta ~Alt Ctrl<Key>x:	beginning-of-line()		\
				delete-to-end-of-line()		\n\
   ~Meta ~Alt Ctrl<Key>k:	delete-to-end-of-line()		\n\
   ~Meta ~Alt Ctrl<Key>a:	beginning-of-line()		\n\
   ~Meta ~Alt Ctrl<Key>e:	end-of-line()   		\n\
   ~Meta ~Alt Ctrl<Key>w:	key-select()			\
				delete-selection()		\n\
   ~Meta ~Alt Ctrl<Key>y:	paste-clipboard()		\n\
   Meta ~Ctrl     <Key>d:	delete-next-word()		\n\
   Alt ~Ctrl      <Key>d:	delete-next-word()		\n\
   ~Meta ~Alt Ctrl<Key>d:       delete-next-character()         \n\
   Meta ~Ctrl<Key>osfBackSpace:	delete-previous-word()		\n\
   Alt ~Ctrl<Key>osfBackSpace:  delete-previous-word()		\n\
   Meta ~Ctrl<Key>osfDelete:	delete-next-word()		\n\
   Alt ~Ctrl<Key>osfDelete:	delete-next-word()		\n\
              <Btn1Down>:       take_focus() grab-focus() traversal_current()";

void AddNewForm(HTMLWidget hw, FormInfo *fptr)
{
	FormInfo *ptr = hw->html.form_list;

	if (!ptr) {
		hw->html.form_list = fptr;
		fptr->cached = 0;
	} else {
		while (ptr->next)
			ptr = ptr->next;
		ptr->next = fptr;
	}
	fptr->next = NULL;
}

static int CollectSubmitInfo(FormInfo *fptr, char ***name_list,
			     char ***value_list)
{
	HTMLWidget hw = (HTMLWidget)fptr->hw;
	WbFormCallbackData cbdata;
	WidgetInfo *wptr;
	int cnt;

	if (fptr->end == -1) {  /* Unterminated FORM tag */
	    wptr = hw->html.widget_list;
	    cnt = 0;
	    while (wptr) {
		cnt++;
		wptr = wptr->next;
	    }
	    cbdata.attribute_count = cnt;
	} else {
	    cbdata.attribute_count = fptr->end - fptr->start;
	}
	cbdata.attribute_names = (char **)malloc(cbdata.attribute_count *
						 sizeof(char *));
	cbdata.attribute_values = (char **)calloc(cbdata.attribute_count *
						  sizeof(char *), 1);
	wptr = hw->html.widget_list;
	if (fptr->start) {
	    while (wptr) {
		if (wptr->id == fptr->start) {
		    wptr = wptr->next;
		    break;
		}
		wptr = wptr->next;
	    }
	}
	cnt = 0;
	while (wptr && (cnt < cbdata.attribute_count)) {
	    /***   cvarela@ncsa.uiuc.edu:  August 17, 1994
             ***   Adding multiple submit buttons support
             ***   changed to match widgets -- amb ***/
            if (wptr->name) {
		Widget child;
		XmStringTable str_list;
		int list_cnt;
		char *val;
		XmString label;
		Arg arg[3];

		cbdata.attribute_names[cnt] = wptr->name;
		switch (wptr->type) {
		    case W_TEXTFIELD:
			val = XmTextFieldGetString(wptr->w);
			if (val && *val)
			    cbdata.attribute_values[cnt] = strdup(val);
			if (val)
			    XtFree(val);
			break;
		    case W_TEXTAREA:
			XtSetArg(arg[0], XmNworkWindow, &child);
			XtGetValues(wptr->w, arg, 1);
			val = XmTextGetString(child);
			if (val && *val)
			    cbdata.attribute_values[cnt] = strdup(val);
			if (val)
			    XtFree(val);
			break;
		    case W_PASSWORD:
			if (wptr->password && *wptr->password)
			    cbdata.attribute_values[cnt] =
							 strdup(wptr->password);
			break;
		    case W_LIST:
			/*
			 * First get the Widget ID of the proper
			 * list element
			 */
			XtSetArg(arg[0], XmNworkWindow, &child);
			XtGetValues(wptr->w, arg, 1);
			/*
			 * Now get the list of selected items.
			 */
			XtSetArg(arg[0], XmNselectedItemCount, &list_cnt);
			XtSetArg(arg[1], XmNselectedItems, &str_list);
			XtGetValues(child, arg, 2);

			if (list_cnt == 0) {
			    cnt--;
			    cbdata.attribute_count--;
			} else {  /* list_cnt >= 1 */
			    int j, new_cnt;
			    char **names, **values;

			    if (list_cnt > 1) {
				new_cnt = cbdata.attribute_count + list_cnt - 1;
				names = (char **)malloc(new_cnt *
							sizeof(char *));
				values = (char **)calloc(1, new_cnt *
							 sizeof(char *));
				for (j = 0; j < cnt; j++) {
				    names[j] = cbdata.attribute_names[j];
				    values[j] = cbdata.attribute_values[j];
				}
				free((char *) cbdata.attribute_names);
				free((char *) cbdata.attribute_values);
				cbdata.attribute_names = names;
				cbdata.attribute_values = values;
				cbdata.attribute_count = new_cnt;
			    }
			    for (j = 0; j < list_cnt; j++) {
				char *mval = NULL;

				cbdata.attribute_names[cnt + j] = wptr->name;
				XmStringGetLtoR(str_list[j],
					        XmSTRING_DEFAULT_CHARSET, &val);
				if (val && *val)
				    mval = MapOptionReturn(val, wptr->mapping);
				if (mval)
				    cbdata.attribute_values[cnt + j] =
								   strdup(mval);
				if (val)
				    XtFree(val);
			    }
			    cnt += list_cnt - 1;
			}
			break;
		    /*
		     * For an option menu, first get the label gadget
		     * which holds the current value.
		     * Now get the text from that label as a character
		     * string.
		     */
		    case W_OPTIONMENU: {
		        char *mval;

			child = XmOptionButtonGadget(wptr->w);
			XtSetArg(arg[0], XmNlabelString, &label);
			XtGetValues(child, arg, 1);
			val = NULL;
			XmStringGetLtoR(label, XmSTRING_DEFAULT_CHARSET, &val);
			XmStringFree(label);
			if (val && *val) {
			    mval = MapOptionReturn(val, wptr->mapping);
			    if (mval && *mval)
			        cbdata.attribute_values[cnt] = strdup(mval);
			}
			if (val)
			    XtFree(val);
			break;
		    }
		    case W_CHECKBOX:
		    case W_RADIOBOX:
			if (XmToggleButtonGetState(wptr->w) == True) {
			    if (wptr->value)
			       	cbdata.attribute_values[cnt] =
							    strdup(wptr->value);
			} else {
			    cnt--;
			    cbdata.attribute_count--;
			}
			break;

	            /*** cvarela@ncsa.uiuc.edu:  August 17, 1994
	             *** Adding multiple submit buttons support ***/
		    /* mods 3/11/95  -- amb */
		    case W_PUSHBUTTON:
                	if (fptr->button_pressed == wptr->w) {
			    if (wptr->value)
			       	cbdata.attribute_values[cnt] =
							    strdup(wptr->value);
                        } else {
                            cnt--;
                            cbdata.attribute_count--;
                        }
                        break;
		    /**/

		    case W_HIDDEN:
			if (wptr->value)
		            cbdata.attribute_values[cnt] = strdup(wptr->value);
			break;
		}
		cnt++;
	    } else {
		cbdata.attribute_count--;
	    }
	    wptr = wptr->next;
	}
	cbdata.attribute_count = cnt;

	*name_list = cbdata.attribute_names;
	*value_list = cbdata.attribute_values;
	return(cbdata.attribute_count);
}

void ImageSubmitForm(FormInfo *fptr, XEvent *event, char *name, int x, int y)
{
	HTMLWidget hw = (HTMLWidget)fptr->hw;
	WbFormCallbackData cbdata;
	int i, cnt;
	char **name_list = NULL;
	char **value_list = NULL;
	char valstr[100];

	cbdata.event = event;
	cbdata.href = fptr->action;
	cbdata.target = fptr->target;
        cbdata.method = fptr->method;
        cbdata.enctype = fptr->enctype;

	cnt = CollectSubmitInfo(fptr, &name_list, &value_list);

	cbdata.attribute_count = cnt + 2;
	cbdata.attribute_names = (char **)malloc(cbdata.attribute_count *
						 sizeof(char *));
	cbdata.attribute_values = (char **)malloc(cbdata.attribute_count *
						  sizeof(char *));
	for (i = 0; i < cnt; i++) {
		cbdata.attribute_names[i] = name_list[i];
		cbdata.attribute_values[i] = value_list[i];
	}
	if (name_list)
		free((char *)name_list);
	if (value_list)
 		free((char *)value_list);

	if (name && *name) {
		cbdata.attribute_names[cnt] = (char *)malloc(strlen(name) + 2 +
							     strlen(X_NAME));
		strcpy(cbdata.attribute_names[cnt], name);
		strcat(cbdata.attribute_names[cnt], ".");
		strcat(cbdata.attribute_names[cnt], X_NAME);
	} else {
		cbdata.attribute_names[cnt] = strdup(X_NAME);
	}
	cbdata.freeit1 = cbdata.attribute_names[cnt];

	sprintf(valstr, "%d", x);
	cbdata.attribute_values[cnt] = strdup(valstr);

	cnt++;
	if (name && *name) {
		cbdata.attribute_names[cnt] = (char *)malloc(strlen(name) +
							    strlen(Y_NAME) + 2);
		strcpy(cbdata.attribute_names[cnt], name);
		strcat(cbdata.attribute_names[cnt], ".");
		strcat(cbdata.attribute_names[cnt], Y_NAME);
	} else {
		cbdata.attribute_names[cnt] = strdup(Y_NAME);
	}
	cbdata.freeit2 = cbdata.attribute_names[cnt];

	sprintf(valstr, "%d", y);
	cbdata.attribute_values[cnt] = strdup(valstr);

	XtCallCallbackList((Widget)hw, hw->html.form_callback,
			   (XtPointer)&cbdata);
}

void CBSubmitForm(Widget w, XtPointer client_data, XtPointer call_data)
{
	FormInfo *fptr = (FormInfo *)client_data;
	HTMLWidget hw;
	WbFormCallbackData cbdata;

	if (!fptr)
		return;

	hw = (HTMLWidget)fptr->hw;

	cbdata.href = fptr->action;
	cbdata.target = fptr->target;
        cbdata.method = fptr->method;
        cbdata.enctype = fptr->enctype;
	cbdata.freeit1 = cbdata.freeit2 = NULL;
	fptr->button_pressed = w;

	cbdata.attribute_count = CollectSubmitInfo(fptr,
						   &cbdata.attribute_names,
						   &cbdata.attribute_values);
	XtCallCallbackList((Widget)hw, hw->html.form_callback,
			   (XtPointer)&cbdata);
}

void CBOnClickForm(Widget w, XtPointer client_data, XtPointer call_data)
{
	HTMLWidget hw = (HTMLWidget)client_data;
	WbAnchorCallbackData cbdata;
	XButtonReleasedEvent event;
	WidgetInfo *wptr;

	wptr = hw->html.widget_list;

	while (wptr) {
		if (wptr->w == w)
			break;
		wptr = wptr->next;
	}
	if (!wptr)
		return;

        event.button = Button1;
	event.state = 0;
	cbdata.event = (XEvent *)&event;
	cbdata.href = wptr->onclick;
	cbdata.text = NULL;
	cbdata.title = NULL;
	cbdata.frame = NULL;
	cbdata.refresh = False;

	XtCallCallbackList((Widget)hw, hw->html.anchor_callback,
			   (XtPointer)&cbdata);
}

/*
 * A radio buttom was toggled on in a form.
 * If there are other radios of the same name, turn them off.
 */
void CBChangeRadio(Widget w, XtPointer client_data, XtPointer call_data)
{
	FormInfo *fptr = (FormInfo *)client_data;
	HTMLWidget hw;
	WidgetInfo *wptr, *wtmp;
	char *name;
	int cnt, count;
	XmToggleButtonCallbackStruct *tb =
				      (XmToggleButtonCallbackStruct *)call_data;

	/*
	 * Bad button
	 */
	if (!tb)
		return;
	/*
	 * Only do stuff when the button is turned on.
	 * Don't let the button be turned off, by clicking on
	 * it, as that would leave all buttons off.
	 */
	if (tb->set == False) {
		XmToggleButtonSetState(w, True, False);
		return;
	}
	if (!fptr)
		return;

	hw = (HTMLWidget)fptr->hw;
	/*
	 * Terminate the form if it was never properly terminated.
	 */
	if (fptr->end == -1) {  /* Unterminated FORM tag */
		wptr = hw->html.widget_list;
		count = 0;
		while (wptr) {
			count++;
			wptr = wptr->next;
		}
	} else {
		count = fptr->end - fptr->start;
	}
	/*
	 * Locate the start of the form.
	 */
	wptr = hw->html.widget_list;
	if (fptr->start) {
		while (wptr) {
			if (wptr->id == fptr->start) {
				wptr = wptr->next;
				break;
			}
			wptr = wptr->next;
		}
	}
	/*
	 * Find the name of the toggle button just pressed.
	 */
	name = NULL;
	wtmp = wptr;
	while (wtmp) {
		if (wtmp->w == w) {
			name = wtmp->name;
			break;
		}
		wtmp = wtmp->next;
	}
	/*
	 * Check for other checked radioboxes of the same name.
	 */
	cnt = 0;
	while (wptr && (cnt++ < count)) {
		if ((wptr->type == W_RADIOBOX) && (wptr->w != w) &&
		    (XmToggleButtonGetState(wptr->w) == True) &&
		    wptr->name && name && !strcmp(wptr->name, name))
			XmToggleButtonSetState(wptr->w, False, False);
		wptr = wptr->next;
	}
}

/*
 * Catch all attempted modifications to the textfield for password
 * entry.  This is so we can prevent the password from showing
 * up on the screen.
 * I would prefer that for all inserted characters a random 1-3 '*'s
 * were added, and any delete deleted the whole string, but due to
 * bugs in some versions of Motif 1.1 this won't work.
 */
void CBPasswordModify(Widget w, XtPointer client_data, XtPointer call_data)
{
	FormInfo *fptr = (FormInfo *)client_data;
	XmTextVerifyCallbackStruct *tv =(XmTextVerifyCallbackStruct *)call_data;
	HTMLWidget hw;
	WidgetInfo *wptr;
	int i, len;

	/* Only accept text modification of password fields */
	if (!fptr || (tv->reason != XmCR_MODIFYING_TEXT_VALUE))
		return;

	hw = (HTMLWidget)fptr->hw;

	/* Find the structure for this widget */
	wptr = hw->html.widget_list;
	while (wptr) {
		if (wptr->w == w)
			break;
		wptr = wptr->next;
	}
	if (!wptr)
		return;

	/*  Deletion.  */
	if (!tv->text->ptr) {
		tv->doit = True;
		/*
		 * Only can delete if we have stuff to delete.
		 */
		if (wptr->password && *wptr->password) {
			int start;
			char *tptr;

			len = strlen(wptr->password);
			/*
			 * Find the start of the chunk of text to
			 * delete.
			 */
			if (tv->startPos < len) {
				start = tv->startPos;
			} else {
				start = len - 1;
			}
			/*
			 * Might be more stuff after the end that we
			 * want to move up
			 */
			if (tv->endPos > len) {
				tptr = &wptr->password[len];
			} else {
				tptr = &wptr->password[tv->endPos];
			}
			wptr->password[start] = '\0';
			strcat(wptr->password, tptr);
		}
	} else if (tv->text->length >= 1) {
		/*
		 * Else insert character.
		 */
		int maxlength = 1000000;
		int plen = 0;
		Arg arg[2];

		/*
		 * No insertion if it makes you exceed maxLength
		 */
		if (wptr->password)
			plen = strlen(wptr->password);
		XtSetArg(arg[0], XmNmaxLength, &maxlength);
		XtGetValues(w, arg, 1);
		if ((plen + tv->text->length) > maxlength)
			return;

		if (!wptr->password) {
			wptr->password = (char *)malloc(tv->text->length + 1);
			for (i = 0; i < tv->text->length; i++)
				wptr->password[i] = tv->text->ptr[i];
			wptr->password[tv->text->length] = '\0';
		} else {
			/* Else insert a char somewhere.
			 * Make a new buffer.  Put everything from before the
			 * insert postion into it.  Now insert the character.
			 * Finally append any remaining text.
			 */
			char *buf, *tptr;
			char tchar;
			int start;

			len = strlen(wptr->password);
			if (tv->startPos < len) {
				start = tv->startPos;
			} else {
				start = len;
			}
			tptr = &wptr->password[start];
			tchar = *tptr;
			*tptr = '\0';
			buf = (char *)malloc(len + tv->text->length + 1);
			strcpy(buf, wptr->password);
			for (i = 0; i < tv->text->length; i++)
				buf[start + i] = tv->text->ptr[i];
			buf[start + tv->text->length] = '\0';
			*tptr = tchar;
			strcat(buf, tptr);
			free(wptr->password);
			wptr->password = buf;
		}
		tv->doit = True;
		/*
		 * Make a '*' show up instead of what they typed
		 */
		for (i = 0; i < tv->text->length; i++)
			tv->text->ptr[i] = '*';
	}
}

/*
 * RETURN was hit in a textfield in a form.
 * If this is the only textfield in this form, submit the form.
 */
void CBActivateField(Widget w, XtPointer client_data, XtPointer call_data)
{
	FormInfo *fptr = (FormInfo *)client_data;
	HTMLWidget hw;
	WidgetInfo *wptr;
	int cnt = 0;
	int count = 0;

	if (!fptr)
		return;

	hw = (HTMLWidget)fptr->hw;
	/*
	 * Terminate the form if it was never properly terminated.
	 */
	if (fptr->end == -1) {    /* Unterminated FORM tag */
		wptr = hw->html.widget_list;
		while (wptr) {
			count++;
			wptr = wptr->next;
		}
	} else {
		count = fptr->end - fptr->start;
	}
	/*
	 * Locate the start of the form.
	 */
	wptr = hw->html.widget_list;
	if (fptr->start) {
		while (wptr) {
			if (wptr->id == fptr->start) {
				wptr = wptr->next;
				break;
			}
			wptr = wptr->next;
		}
	}
	/*
	 * Count the textfields in this form.
	 */
	while (wptr && count--) {
		if ((wptr->type == W_TEXTFIELD) || (wptr->type == W_PASSWORD))
			cnt++;
		wptr = wptr->next;
	}
	/*
	 * If this is the only textfield in this form, submit the form.
	 */
	if (cnt == 1)
		CBSubmitForm(w, client_data, call_data);
}

void CBResetForm(Widget w, XtPointer client_data, XtPointer call_data)
{
	FormInfo *fptr = (FormInfo *)client_data;
	HTMLWidget hw;
	WidgetInfo *wptr;
	int widget_count = 0;
	int cnt = 0;
	Widget child;
	Arg arg[3];

	if (!fptr)
		return;

	hw = (HTMLWidget)fptr->hw;

	if (fptr->end == -1) {  /* Unterminated FORM tag */
		wptr = hw->html.widget_list;
		while (wptr) {
			widget_count++;
			wptr = wptr->next;
		}
	} else {
		widget_count = fptr->end - fptr->start;
	}

	wptr = hw->html.widget_list;
	if (fptr->start) {
		while (wptr) {
			if (wptr->id == fptr->start) {
				wptr = wptr->next;
				break;
			}
			wptr = wptr->next;
		}
	}

	while (wptr && (cnt < widget_count)) {
		switch (wptr->type) {
		    case W_TEXTFIELD:
			if (!wptr->value) {
			    XmTextFieldSetString(wptr->w, "");
			} else {
			    XmTextFieldSetString(wptr->w, wptr->value);
			}
			break;
		    case W_TEXTAREA:
			XtSetArg(arg[0], XmNworkWindow, &child);
			XtGetValues(wptr->w, arg, 1);
			if (!wptr->value) {
			    XmTextSetString(child, "");
			} else {
			    XmTextSetString(child, wptr->value);
			}
			break;
		    case W_PASSWORD:
			if (!wptr->value) {
			    /*
			     * Due to errors in Motif 1.1, I can't
			     * call XmTextFieldSetString() here.
			     * Because I have a modifyVerify callback
			     * registered for this widget.
			     * I don't know if this error exists
			     * in Motif 1.2 or not.
			     */
			    XtSetArg(arg[0], XmNvalue, "");
			    XtSetValues(wptr->w, arg, 1);
			    if (wptr->password) {
				free(wptr->password);
				wptr->password = NULL;
			    }
			} else {
			    int i;
			    int len = strlen(wptr->value);

			    if (wptr->password)
				free(wptr->password);
			    wptr->password = (char *)malloc(len + 1);
			    for (i = 0; i < len; i++)
				wptr->password[i] = '*';
			    wptr->password[len] = '\0';
			    XmTextFieldSetString(wptr->w, wptr->password);
			    strcpy(wptr->password, wptr->value);
			}
			break;
		    case W_LIST:
			XtSetArg(arg[0], XmNworkWindow, &child);
			XtGetValues(wptr->w, arg, 1);

			if (wptr->value) {
			    char **vlist;
			    int i, vlist_cnt;
			    XmStringTable val_list;

			    vlist = ParseCommaList(wptr->value, &vlist_cnt);
			    val_list = (XmStringTable)XtMalloc(vlist_cnt *
						              sizeof(XmString));
			    XmListDeselectAllItems(child);
			    for (i = 0; i < vlist_cnt; i++)
				val_list[i] = XmStringCreateSimple(vlist[i]);
			    FreeCommaList(vlist, vlist_cnt);
			    if (vlist_cnt > 0) {
				XtSetArg(arg[0], XmNselectedItems, val_list);
				XtSetArg(arg[1], XmNselectedItemCount,
					 vlist_cnt);
				XtSetValues(child, arg, 2);
			    }
			    for (i = 0; i < vlist_cnt; i++)
				XmStringFree(val_list[i]);
			    if (val_list)
				XtFree((char *)val_list);
			} else {
			    XmListDeselectAllItems(child);
			}
		        break;
		    /*
		     * Gack, we saved the widget id of the starting default
		     * into the value character pointer, just so we could
		     * yank it out here, and restore the default.
		     */
		    case W_OPTIONMENU:
			if (wptr->value) {
				XtSetArg(arg[0], XmNmenuHistory,
					 (Widget)wptr->value);
				XtSetValues(wptr->w, arg, 1);
			}
			break;
		    case W_CHECKBOX:
		    case W_RADIOBOX:
			if (wptr->checked == True) {
				XmToggleButtonSetState(wptr->w, True, False);
			} else {
				XmToggleButtonSetState(wptr->w, False, False);
			}
			break;
		    case W_HIDDEN:
		    default:
			break;
		}
		cnt++;
		wptr = wptr->next;
	}
}

void HideWidgets(HTMLWidget hw)
{
	WidgetInfo *wptr;
	XEvent event;
	Display *dsp = hw->html.dsp;

	/*
	 * Make sure all expose events have been dealt with first.
	 */
	XmUpdateDisplay((Widget)hw);
	wptr = hw->html.widget_list;
	while (wptr) {
		if (wptr->w && wptr->mapped) {
			XtSetMappedWhenManaged(wptr->w, False);
			wptr->mapped = False;
		}
		wptr = wptr->next;
	}

	/* Force the exposure events into the queue */
	XSync(dsp, False);

	/* Remove all Expose events for the view window */
	while (XCheckWindowEvent(dsp, XtWindow(hw->html.view),
				 ExposureMask, &event))
		;
}

static void MapWidgets(HTMLWidget hw)
{
	WidgetInfo *wptr = hw->html.widget_list;

	while (wptr) {
		if (wptr->w && !wptr->mapped && wptr->seeable) {
			wptr->mapped = True;
			XtSetMappedWhenManaged(wptr->w, True);
		}
		wptr = wptr->next;
	}
}

static Boolean AlreadyChecked(HTMLWidget hw, FormInfo *fptr, char *name)
{
	WidgetInfo *wptr = hw->html.widget_list;
	Boolean radio_checked = False;

	while (wptr) {
		if ((wptr->id >= fptr->start) && (wptr->type == W_RADIOBOX) &&
		    (wptr->checked == True) && wptr->name && name &&
		    !strcmp(wptr->name, name)) {
			radio_checked = True;
			break;
		}
		wptr = wptr->next;
	}
	return(radio_checked);
}

static WidgetInfo *AddNewWidget(HTMLWidget hw, Widget w, int type, int id,
				int x, int y, int width, int height,
				char *name, char *value, char *onclick,
				char **mapping,	Boolean checked,
				Boolean display, PhotoComposeContext *pcc)
{
	WidgetInfo *wptr = hw->html.widget_list;
	WidgetInfo *new = (WidgetInfo *)calloc(1, sizeof(WidgetInfo));

	if (pcc->cw_only || !wptr) {
		wptr = new;
		/** calloc sets them
		wptr->prev = NULL;
		wptr->cache_invalid = 0;
		wptr->cached_forms = NULL;
		**/
		if (!pcc->cw_only)
			hw->html.widget_list = wptr;
	} else {
		WidgetInfo *lptr;

		while (wptr->next)
			wptr = wptr->next;
		wptr->next = new;
		lptr = wptr;  /* Save this to fill in prev field */
		wptr = wptr->next;
		wptr->prev = lptr;
	}
	wptr->w = w;
	wptr->type = type;
	wptr->id = id;
	wptr->x = x;
	wptr->y = y;
	wptr->width = width;
	wptr->height = height;
	wptr->name = name;
	wptr->value = value;
	wptr->onclick = onclick;
	wptr->mapping = mapping;
	wptr->checked = checked;
	wptr->display = display;
	/** Calloc sets them
	wptr->password = NULL;
        wptr->seeable = 0;
	wptr->mapped = False;
	wptr->next = NULL;
	wptr->cache_count = 0;
	wptr->data_cached = False;
	wptr->cached_text = NULL;
	**/

	if ((wptr->type == W_PASSWORD) && wptr->value)
		wptr->password = strdup(wptr->value);

	return(wptr);
}

/*
 * Get the next value in a comma separated list.
 * Also unescape the '\' escaping done in ComposeCommaList
 * and convert the single ''' characters back to '"'
 * characters
 */
static char *NextComma(char *string)
{
        char *tptr = string;

        while (*tptr) {
                if (*tptr == '\\') {
                        *tptr = '\0';
                        strcat(string, (char *)(++tptr));
                } else if (*tptr == '\'') {
                        *tptr++ = '\"';
                } else if (*tptr == ',') {
                        return(tptr);
                } else {
                        tptr++;
                }
        }
        return(tptr);
}

static char **ParseCommaList(char *str, int *count)
{
	char *str_copy, *tptr, *val;
	char **list, **tlist;
	int i;
	int cnt = 0;
	int max_cnt = 50;

	if (!str || !*str) {
		*count = 0;
		return((char **)NULL);
	}
	val = str_copy = strdup(str);
	CHECK_OUT_OF_MEM(str_copy);

	list = (char **)malloc(max_cnt * sizeof(char *));
	CHECK_OUT_OF_MEM(list);

	/*
	 * This loop counts the number of objects
	 * in this list.
	 * As a side effect, NextComma() unescapes in place so
	 * "\\" becomes '\' and "\," becomes ',' and "\"" becomes '"'
	 */
	tptr = NextComma(val);
	while (*tptr) {
		if ((cnt + 1) == max_cnt) {
			max_cnt += 50;
			tlist = (char **)malloc(max_cnt * sizeof(char *));
			CHECK_OUT_OF_MEM(tlist);
			for (i = 0; i < cnt; i++)
				tlist[i] = list[i];
			free((char *)list);
			list = tlist;
		}
		*tptr = '\0';
		list[cnt] = strdup(val);
		CHECK_OUT_OF_MEM(list[cnt]);
		cnt++;
		val = (char *)(tptr + 1);
		tptr = NextComma(val);
	}
	list[cnt] = strdup(val);
	CHECK_OUT_OF_MEM(list[cnt]);
	cnt++;

	free(str_copy);
	tlist = (char **)malloc(cnt * sizeof(char *));
	CHECK_OUT_OF_MEM(tlist);
	for (i = 0; i < cnt; i++)
		tlist[i] = list[i];
	free((char *)list);
	list = tlist;

	*count = cnt;
	return(list);
}

/*
 * Compose a single string comma separated list from
 * an array of strings.  Any '\', or ',' in the
 * list are escaped with a prepending '\'.
 * So they become '\\' and '\,'
 * Also we want to allow '"' characters in the list, but
 * they would get eaten by the later parsing code, so we will
 * turn '"' into ''', and turn ''' into '\''
 */
char *ComposeCommaList(char **list, int cnt)
{
	char *buf = NULL;
	char *tbuf, *option, *tptr;
	int i, olen;
	int len = 0;
	int max_len = 1024;

	if (!cnt || !(buf = (char *)malloc(max_len)))
		goto failed;
	*buf = '\0';

	for (i = 0; i < cnt; i++) {
		option = list[i];
		if (!option) {
			olen = 0;
		} else {
			olen = strlen(option);
		}
		if ((len + (olen * 2)) >= (max_len - 2)) {
			max_len += olen + 1024;
			tbuf = (char *)malloc(max_len);
			if (!tbuf)
				goto failed;
			strcpy(tbuf, buf);
			free(buf);
			buf = tbuf;
		}
		tptr = (char *)(buf + len);
		while (option && *option) {
			if ((*option == '\\') || (*option == ',') ||
				(*option == '\'')) {
				*tptr++ = '\\';
				*tptr++ = *option;
				len += 2;
			} else if (*option == '\"') {
				*tptr++ = '\'';
				len++;
			} else {
				*tptr++ = *option;
				len++;
			}
			option++;
		}
		if (i != (cnt - 1)) {
			*tptr++ = ',';
			len++;
		}
		*tptr = '\0';
	}
	tbuf = (char *)malloc(len + 1);
	if (!tbuf)
		goto failed;
	strcpy(tbuf, buf);
	free(buf);
	return(tbuf);

 failed:
	if (buf)
		free(buf);
	return((char *)calloc(1, 1));
}

void FreeCommaList(char **list, int cnt)
{
	int i;

	for (i = 0; i < cnt; i++) {
		if (list[i])
			free(list[i]);
	}
	if (list)
		free((char *)list);
}

/*
 * Clean up the mucked value field for a TEXTAREA.
 * Unescape the things with '\' in front of them, and transform
 * lone ' back to "
 */
static void UnMuckTextAreaValue(char *value)
{
	char *tptr = value;

	if (!tptr)
		return;
        while (*tptr) {
                if (*tptr == '\\') {
                        *tptr++ = '\0';
                        strcat(value, (char *)tptr);
                } else if (*tptr == '\'') {
                        *tptr++ = '\"';
                } else {
                        tptr++;
                }
        }
}

static char *MapOptionReturn(char *val, char **mapping)
{
	int cnt = 0;

	if (!mapping)
		return(val);
	while (mapping[cnt]) {
		if (!strcmp(mapping[cnt], val))
			return(mapping[cnt + 1]);
		cnt += 2;
	}
	return(val);
}

static char **MakeOptionMappings(char **list1, char **list2, int list_cnt)
{
	int i;
	int cnt = 0;
	char **list;

	/*
	 * Pass through to see how many mappings we have.
	 */
	for (i = 0; i < list_cnt; i++) {
		/* Can be empty string */
		if (list2[i])
			cnt++;
	}
	if (!cnt || !(list = (char **)malloc(((2 * cnt) + 1) * sizeof(char *))))
		return(NULL);
	cnt = 0;
	for (i = 0; i < list_cnt; i++) {
		if (list2[i]) {
			list[cnt++] = strdup(list1[i]);
			list[cnt++] = strdup(list2[i]);
		}
	}
	list[cnt] = NULL;
	return(list);
}

/* Make the appropriate widget for this tag, and fill in a
 * WidgetInfo structure and return it.
 */
WidgetInfo *MakeWidget(HTMLWidget hw, char *text,
		       PhotoComposeContext *pcc, int id)
{
	Arg arg[30];
	Cardinal argcnt;
	WidgetInfo *wlist = hw->html.widget_list;
	WidgetInfo *wptr;
	Position x = pcc->x;
	Position y = pcc->y;
	unsigned long bgcolor = hw->html.background_SAVE;

	while (wlist) {
		if (wlist->id == id)
			break;
		wlist = wlist->next;
	}

	/* If this widget is not on the list, we have never
	 * used it before.  Create it now.
	 */
	if (!wlist || wlist->data_cached) {
		Widget w;
		Widget ChildWidget = NULL;
		FormInfo *fptr = pcc->cur_form;
		char widget_name[100];
		char **mapping = NULL;
		char *tptr, *value, *name, *type_str;
		char *onclick = NULL;
		Dimension width, height;
		int type, maxlength;
		short size;
		Boolean checked = False;
		Boolean display = True;
		static int init = 0;
		static XmString blank;

		if (!init) {
			blank = XmStringCreateSimple("");
			init = 1;
		}

		name = ParseMarkTag(text, MT_INPUT, "NAME");
		/*
		 * We may need to shorten the name for the widgets,
		 * which can't handle long names.
		 */
		if (!name) {
			widget_name[0] = '\0';
		} else if (strlen(name) > 99) {
			strncpy(widget_name, name, 99);
			widget_name[99] = '\0';
		} else {
			strcpy(widget_name, name);
		}

		if (tptr = ParseMarkTag(text, MT_INPUT, "STYLE")) {
			if (!strcmp(tptr, "display:none"))
				display = False;
			free(tptr);
		}
		if (!(type_str = ParseMarkTag(text, MT_INPUT, "TYPE")))
			type_str = strdup("None");

		if (!my_strcasecmp(type_str, "checkbox")) {
			type = W_CHECKBOX;
			if (!(value = ParseMarkTag(text, MT_INPUT, "VALUE")))
				value = strdup("on");
			if (!wlist) {
				tptr = ParseMarkTag(text, MT_INPUT, "CHECKED");
			} else {
				tptr = NULL;
			}
			argcnt = 0;
			/* We want no text on our toggles */
			XtSetArg(arg[argcnt], XmNlabelString, blank);
			argcnt++;
			/* No spacing before blank label */
			XtSetArg(arg[argcnt], XmNspacing, 0);
			argcnt++;
			XtSetArg(arg[argcnt], XmNx, x);
			argcnt++;
			XtSetArg(arg[argcnt], XmNy, y);
			argcnt++;
			if (tptr || (wlist && wlist->toggled)) {
				XtSetArg(arg[argcnt], XmNset, True);
				argcnt++;
				checked = True;
				if (tptr)
					free(tptr);
			}
			w = XmCreateToggleButton(hw->html.view, widget_name,
						 arg, argcnt);
			XtOverrideTranslations(w,
				  XtParseTranslationTable(button_translations));
			if (!hw->html.focus_follows_mouse) {
			      XtOverrideTranslations(w, 
				      XtParseTranslationTable(traversal_table));
			      XtOverrideTranslations(w, XtParseTranslationTable(
				      "<Btn1Down>: Arm() traversal_current()"));
			}
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
			bgcolor = pcc->bg;

		} else if (!my_strcasecmp(type_str, "hidden")) {
			type = W_HIDDEN;
			if (!(value = ParseMarkTag(text, MT_INPUT, "VALUE")))
				value = (char *)calloc(1, 1);
			w = NULL;

		} else if (!my_strcasecmp(type_str, "radio")) {
			type = W_RADIOBOX;
			if (!(value = ParseMarkTag(text, MT_INPUT, "VALUE")))
				value = strdup("on");
			if (!wlist) {
				tptr = ParseMarkTag(text, MT_INPUT, "CHECKED");
			} else {
				tptr = NULL;
			}
			/*
			 * Only one checked radio button with the
			 * same name per form
			 */
			if (tptr && fptr && AlreadyChecked(hw, fptr, name)) {
				free(tptr);
				tptr = NULL;
			}
			argcnt = 0;
			/* We want no text on our toggles */
			XtSetArg(arg[argcnt], XmNlabelString, blank);
			argcnt++;
			/* No spacing before blank label */
			XtSetArg(arg[argcnt], XmNspacing, 0);
			argcnt++;
			XtSetArg(arg[argcnt], XmNx, x);
			argcnt++;
			XtSetArg(arg[argcnt], XmNy, y);
			argcnt++;
			XtSetArg(arg[argcnt], XmNindicatorType, XmONE_OF_MANY);
			argcnt++;
			if (tptr || (wlist && wlist->toggled)) {
				XtSetArg(arg[argcnt], XmNset, True);
				argcnt++;
				checked = True;
				if (tptr)
					free(tptr);
			}
			w = XmCreateToggleButton(hw->html.view, widget_name,
						 arg, argcnt);
			XtOverrideTranslations(w,
				  XtParseTranslationTable(button_translations));
			if (!hw->html.focus_follows_mouse)
				XtOverrideTranslations(w, 
				      XtParseTranslationTable(traversal_table));
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
			XtAddCallback(w, XmNvalueChangedCallback,
				      (XtCallbackProc)CBChangeRadio,
				      (XtPointer)fptr);
			bgcolor = pcc->bg;

		} else if (!my_strcasecmp(type_str, "submit")) {
			XmString label;
			char *btext = ParseMarkTag(text, MT_INPUT, "TEXT");

			type = W_PUSHBUTTON;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			if (!value || !*value) {
				if (value)
					free(value);
				value = strdup("Submit");
			}
			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, x);
			argcnt++;
			XtSetArg(arg[argcnt], XmNy, y);
			argcnt++;
			if (btext) {
				label = XmStringCreateSimple(btext);
				free(btext);
			} else {
				label = XmStringCreateSimple(value);
			}
			XtSetArg(arg[argcnt], XmNlabelString, label);
			argcnt++;
			w = XmCreatePushButton(hw->html.view, widget_name,
					       arg, argcnt);
			XtOverrideTranslations(w,
				  XtParseTranslationTable(button_translations));
			if (!hw->html.focus_follows_mouse)
				XtOverrideTranslations(w, 
				      XtParseTranslationTable(traversal_table));
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
			if (label)
				XmStringFree(label);
			if (fptr)
				XtAddCallback(w, XmNactivateCallback,
					      (XtCallbackProc)CBSubmitForm,
					      (XtPointer)fptr);
			bgcolor = hw->html.formbuttonbackground;

		} else if (!my_strcasecmp(type_str, "reset")) {
			XmString label;
			char *btext = ParseMarkTag(text, MT_INPUT, "TEXT");

			type = W_PUSHBUTTON;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			if (!value || !*value) {
				if (value)
					free(value);
				value = strdup("Reset");
			}
			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, x);
			argcnt++;
			XtSetArg(arg[argcnt], XmNy, y);
			argcnt++;
			if (btext) {
				label = XmStringCreateSimple(btext);
				free(btext);
			} else {
				label = XmStringCreateSimple(value);
			}
			XtSetArg(arg[argcnt], XmNlabelString, label);
			argcnt++;
			w = XmCreatePushButton(hw->html.view, widget_name,
					       arg, argcnt);
			XtOverrideTranslations(w,
				  XtParseTranslationTable(button_translations));
			if (!hw->html.focus_follows_mouse)
				XtOverrideTranslations(w, 
				      XtParseTranslationTable(traversal_table));
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
			if (label)
				XmStringFree(label);
			if (fptr)
				XtAddCallback(w, XmNactivateCallback,
					      (XtCallbackProc)CBResetForm,
					      (XtPointer)fptr);
			bgcolor = hw->html.formbuttonbackground;

		} else if (!my_strcasecmp(type_str, "button")) {
			XmString label = NULL;
			char *btext = ParseMarkTag(text, MT_INPUT, "TEXT");

			type = W_PUSHBUTTON;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			onclick = ParseMarkTag(text, MT_INPUT, "ONCLICK");

			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, x);
			argcnt++;
			XtSetArg(arg[argcnt], XmNy, y);
			argcnt++;
			if (btext) {
				label = XmStringCreateSimple(btext);
				free(btext);
			} else if (value) {
				label = XmStringCreateSimple(value);
			}
			XtSetArg(arg[argcnt], XmNlabelString, label);
			argcnt++;
			w = XmCreatePushButton(hw->html.view, widget_name,
					       arg, argcnt);
			XtOverrideTranslations(w,
				  XtParseTranslationTable(button_translations));
			if (!hw->html.focus_follows_mouse)
				XtOverrideTranslations(w, 
				      XtParseTranslationTable(traversal_table));
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
			if (label)
				XmStringFree(label);
			if (onclick)
				XtAddCallback(w, XmNactivateCallback,
					      (XtCallbackProc)CBOnClickForm,
					      (XtPointer)hw);
			bgcolor = hw->html.formbuttonbackground;

		} else if (!my_strcasecmp(type_str, "select")) {
			XmString label = NULL;
			Widget scroll, pulldown, button;
			Widget hist = NULL;
			char *options, *returns, *labels;
			char **list, **ret_list, **label_list, **vlist;
			int list_cnt, return_cnt, label_cnt, vlist_cnt, i;
			int size = 5;
			int mult = 0;
			int scol_max = 38;

			/* Compute column max based on screen height */
			if (LimDimY < 1024)
				scol_max = 36;
			if (tptr = ParseMarkTag(text, MT_INPUT, "HINT")) {
				if (!my_strcasecmp(tptr, "list")) {
					type = W_LIST;
				} else if (!my_strcasecmp(tptr, "menu")) {
					type = W_OPTIONMENU;
				}
				free(tptr);
			} else {
				type = -1;
			}
			if (tptr = ParseMarkTag(text, MT_INPUT, "SIZE")) {
				size = atoi(tptr);
				if ((size > 1) && (type == -1))
					type = W_LIST;
				free(tptr);
			}
			if (tptr = ParseMarkTag(text, MT_INPUT, "MULTIPLE")) {
				if (type == -1)
					type = W_LIST;
				mult = 1;
				free(tptr);
			}
			if (type == -1)
				type = W_OPTIONMENU;
			value = ParseMarkTag(text, MT_INPUT, "VALUE");
			options = ParseMarkTag(text, MT_INPUT, "OPTIONS");
			returns = ParseMarkTag(text, MT_INPUT, "RETURNS");
			labels = ParseMarkTag(text, MT_INPUT, "LABELS");
			list = ParseCommaList(options, &list_cnt);
			if (options) 
				free(options);
			ret_list = ParseCommaList(returns, &return_cnt);
			if (returns)
				free(returns);
			/*
			 * If return_cnt is less than list_cnt, the user made
			 * a serious error.  Try to recover by padding out
			 * ret_list with NULLs
			 */
			if (list_cnt > return_cnt) {
				int rcnt;
				char **rlist = (char **)calloc(1, list_cnt *
							        sizeof(char *));

				for (rcnt = 0; rcnt < return_cnt; rcnt++)
					rlist[rcnt] = ret_list[rcnt];
				if (ret_list)
					free((char *)ret_list);
				ret_list = rlist;
			}
			label_list = ParseCommaList(labels, &label_cnt);
			if (labels)
				free(labels);
			vlist = ParseCommaList(value, &vlist_cnt);
			if (value) {
			    free(value);
			    value = NULL;
			}
			if (size > list_cnt)
				size = list_cnt;
			if (size < 1)
				size = 1;
			mapping = MakeOptionMappings(list, ret_list, list_cnt);
#ifndef DISABLE_TRACE
			if (htmlwTrace)
				fprintf(stderr,
					"Menu items = %d, scolmax = %d\n",
					list_cnt, scol_max);
#endif
			/* Force scrolled window if won't fit in two columns */
			if ((type == W_OPTIONMENU) &&
			    (list_cnt > (2 * scol_max))) {
				type = W_LIST;
				if (size < 3)
					size = 3;
			}
			if (type == W_OPTIONMENU) {
				argcnt = 0;
				if (list_cnt > scol_max) {
				    XtSetArg(arg[argcnt], XmNpacking,
					     XmPACK_COLUMN);
				    argcnt++;
				    XtSetArg(arg[argcnt], XmNnumColumns, 2);
				    argcnt++;
				}
				pulldown = XmCreatePulldownMenu(hw->html.view,
						      widget_name, arg, argcnt);
				for (i = 0; i < list_cnt; i++) {
				    char bname[30];
				    int is_title = 0;
				    char *item;

				    if ((label_cnt > i) && *label_list[i]) {
					item = label_list[i];
					if (!strcmp(list[i], "MOSAIC_OPTGROUP"))
					    /* OPTGROUP label */
					    is_title = 1;
				    } else {
					item = list[i];
				    }
				    sprintf(bname, "Button%d", i + 1);
				    label = XmStringCreateSimple(item);
				    argcnt = 0;
				    XtSetArg(arg[argcnt], XmNlabelString,label);
				    argcnt++;
				    if (is_title) {
	 	               		/* Kill margin */
 	 	               		XtSetArg(arg[argcnt], XmNmarginWidth,0);
					argcnt++;
					button = XmCreateLabel(pulldown, bname,
							       arg, argcnt);
				    } else {
					button = XmCreatePushButton(pulldown,
							    bname, arg, argcnt);
				    }
				    XtManageChild(button);
				    XmStringFree(label);
				    if (!hw->html.focus_follows_mouse)
					XtOverrideTranslations(button, 
				                        XtParseTranslationTable(
					                      traversal_table));
				    if (wlist && wlist->cached_text &&
					!strcmp(wlist->cached_text, list[i]))
					    hist = button;
				    if ((vlist_cnt > 0) && vlist[0] &&
					!strcmp(vlist[0], list[i])) {
					if (wlist && wlist->cached_text) {
					    /* Need for form resets */
					    value = (char *)button;
					} else {
					    hist = button;
					}
				    }
				    /*
				     * Start hist out as the first button so
				     * that if the user didn't set a default
				     * we always default to the first element.
				     */
				    if (!hist && !is_title)
					hist = button;
				}
				if (wlist && wlist->cached_text) {
				    free(wlist->cached_text);
				    wlist->cached_text = NULL;
				}
				FreeCommaList(list, list_cnt);
				FreeCommaList(ret_list, list_cnt);
				FreeCommaList(label_list, label_cnt);
				FreeCommaList(vlist, vlist_cnt);
				argcnt = 0;
				XtSetArg(arg[argcnt], XmNx, x);
				argcnt++;
				XtSetArg(arg[argcnt], XmNy, y);
				argcnt++;
                                /* Kill margins */
                                XtSetArg(arg[argcnt], XmNspacing, 0);
                                argcnt++;
                                XtSetArg(arg[argcnt], XmNmarginWidth, 0);
                                argcnt++;
                                XtSetArg(arg[argcnt], XmNmarginHeight, 0);
                                argcnt++;
				XtSetArg(arg[argcnt], XmNsubMenuId, pulldown);
				argcnt++;
				if (hist) {
				    XtSetArg(arg[argcnt], XmNmenuHistory, hist);
				    argcnt++;
				    /*
				     * A gaggage.  Value is used to later
				     * restore defaults.  For option menu
				     * this means we need to save a child
				     * widget id as opposed to the
				     * character string everyone else uses.
				     */
				    if (!wlist || !value)
					value = (char *)hist;
				} else {
				    value = NULL;
				}
                                XtSetArg(arg[argcnt], XmNlabelString, blank);
                                argcnt++;
				w = XmCreateOptionMenu(hw->html.view,
						      widget_name, arg, argcnt);
				if (!hw->html.focus_follows_mouse) {
				    XtOverrideTranslations(w, 
				      XtParseTranslationTable(traversal_table));
				    XtOverrideTranslations(pulldown, 
				      XtParseTranslationTable(traversal_table));
				}
                                argcnt = 0;
                                XtSetArg(arg[argcnt], XmNhighlightThickness, 0);
                                argcnt++;
                                XtSetArg(arg[argcnt], XmNshadowThickness, 0);
                                argcnt++;
                                XtSetArg(arg[argcnt], XmNborderWidth, 0);
                                argcnt++;
                                XtSetArg(arg[argcnt], XmNwidth, 0);
                                argcnt++;
                                XtSetArg(arg[argcnt], XmNmarginWidth, 0);
                                argcnt++;
                                XtSetArg(arg[argcnt], XmNmarginLeft, 0);
                                argcnt++;
                                XtSetValues(XmOptionLabelGadget(w),
					    arg, argcnt);
			} else if (wlist) {
				/* Cached W_LIST */
				w = wlist->w;
                        } else { 
				/* type == W_LIST */
				XmStringTable string_list, val_list;
				char *item;

				if (!mult && (vlist_cnt > 1)) {
				    free(value);
				    value = strdup(vlist[0]);
				}
				string_list = (XmStringTable)XtMalloc(list_cnt *
							      sizeof(XmString));
				val_list = (XmStringTable)XtMalloc(vlist_cnt *
							      sizeof(XmString));
				for (i = 0; i < list_cnt; i++) {
				    if ((label_cnt > i) && *label_list[i]) {
					if (!strcmp(list[i],
						    "MOSAIC_OPTGROUP")) {
					    item = "----";
					} else {
					    item = label_list[i];
					}
				    } else {
					item = list[i];
				    }
				    string_list[i] = XmStringCreateSimple(item);
				}
				for (i = 0; i < vlist_cnt; i++)
				    val_list[i] =XmStringCreateSimple(vlist[i]);
				FreeCommaList(list, list_cnt);
				FreeCommaList(ret_list, list_cnt);
				FreeCommaList(vlist, vlist_cnt);
				FreeCommaList(label_list, label_cnt);

				argcnt = 0;
				XtSetArg(arg[argcnt], XmNx, x);
				argcnt++;
				XtSetArg(arg[argcnt], XmNy, y);
				argcnt++;
				scroll = XmCreateScrolledWindow(hw->html.view,
							 "Scroll", arg, argcnt);
				argcnt = 0;
				XtSetArg(arg[argcnt], XmNitems, string_list);
				argcnt++;
				XtSetArg(arg[argcnt], XmNitemCount, list_cnt);
				argcnt++;
				XtSetArg(arg[argcnt], XmNvisibleItemCount,size);
				argcnt++;
				if (mult) {
				    XtSetArg(arg[argcnt], XmNselectionPolicy,
					     XmEXTENDED_SELECT);
				    argcnt++;
				} else {
				    XtSetArg(arg[argcnt], XmNselectionPolicy,
					     XmBROWSE_SELECT);
				    argcnt++;
				}
				if ((vlist_cnt > 0) && mult) {
				    XtSetArg(arg[argcnt], XmNselectedItems,
					     val_list);
				    argcnt++;
				    XtSetArg(arg[argcnt], XmNselectedItemCount,
					     vlist_cnt);
				    argcnt++;
				} else if ((vlist_cnt > 0) && !mult) {
				    XtSetArg(arg[argcnt], XmNselectedItems,
					     &val_list[0]);
				    argcnt++;
				    XtSetArg(arg[argcnt],
					     XmNselectedItemCount, 1);
				    argcnt++;
				}
				w = XmCreateList(scroll, widget_name, arg,
						 argcnt);
				ChildWidget = w;
				if (!hw->html.focus_follows_mouse)
				    XtOverrideTranslations(w, 
				      XtParseTranslationTable(traversal_table));
                                XtManageChild(w);
				w = scroll;
				for (i = 0; i < list_cnt; i++)
				     XmStringFree(string_list[i]);
				if (string_list)
				     XtFree((char *)string_list);
				for (i = 0; i < vlist_cnt; i++)
				     XmStringFree(val_list[i]);
				if (val_list)
				     XtFree((char *)val_list);
			}
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);

		} else if (!my_strcasecmp(type_str, "password")) {
			char *bval = NULL;

			type = W_PASSWORD;
			if (!wlist) {
				value = ParseMarkTag(text, MT_INPUT, "VALUE");
			} else {
				value = wlist->cached_text;
				wlist->cached_text = NULL;
			}
			if (tptr = ParseMarkTag(text, MT_INPUT, "SIZE")) {
				size = atoi(tptr);
				free(tptr);
			} else {
				size = -1;
			}
			if (tptr = ParseMarkTag(text, MT_INPUT, "MAXLENGTH")) {
				maxlength = atoi(tptr);
				free(tptr);
			} else {
				maxlength = -1;
			}
			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, x);
			argcnt++;
			XtSetArg(arg[argcnt], XmNy, y);
			argcnt++;
			if (size > 0) {
				XtSetArg(arg[argcnt], XmNcolumns, size);
				argcnt++;
			}
			if (maxlength > 0) {
				XtSetArg(arg[argcnt], XmNmaxLength, maxlength);
				argcnt++;
			}
			if (value) {
				int i;
				int len = strlen(value);

				bval = (char *)malloc(len + 1);
				for (i = 0; i < len; i++)
					bval[i] = '*';
				bval[len] = '\0';
				XtSetArg(arg[argcnt], XmNvalue, bval);
				argcnt++;
			}
			w = XmCreateTextField(hw->html.view, widget_name,
					      arg, argcnt);
			if (bval)
				free(bval);
			XtOverrideTranslations(w,
				XtParseTranslationTable(text_translations));
			if (!hw->html.focus_follows_mouse)
				XtOverrideTranslations(w, 
				      XtParseTranslationTable(traversal_table));
  /*
   * The proper order here is XtSetMappedWhenManaged, XtManageChild.  But a bug
   * in some versions of Motif 1.1 makes us do it the other way.  All versions
   * of 1.2 should have this fixed.
   */
#ifdef MOTIF1_2
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
#else
			XtManageChild(w);
			XtSetMappedWhenManaged(w, False);
#endif /* MOTIF1_2 */
			if (fptr) {
				XtAddCallback(w, XmNactivateCallback,
					      (XtCallbackProc)CBActivateField,
					      (XtPointer)fptr);
				XtAddCallback(w, XmNmodifyVerifyCallback,
					      (XtCallbackProc)CBPasswordModify,
					      (XtPointer)fptr);
			}

		} else if (!my_strcasecmp(type_str, "textarea")) {
			int rows = 4;
			int cols = 40;

			type = W_TEXTAREA;

			/* Look for ROWS and COLS */
			if (tptr = ParseMarkTag(text, MT_INPUT, "ROWS")) {
				rows = atoi(tptr);
				free(tptr);
				if (rows <= 0)
					rows = 4;
			}
			if (tptr = ParseMarkTag(text, MT_INPUT, "COLS")) {
				cols = atoi(tptr);
				free(tptr);
				if (cols <= 0)
					cols = 40;
			}
			if (!wlist) {
				/* Grab the starting value of the text here.
				 * NULL if none. */
				value = ParseMarkTag(text, MT_INPUT, "VALUE");
				UnMuckTextAreaValue(value);
			} else {
				value = wlist->cached_text;
				wlist->cached_text = NULL;
			}
			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, x);
			argcnt++;
			XtSetArg(arg[argcnt], XmNy, y);
			argcnt++;
			w = XmCreateScrolledWindow(hw->html.view, "Scroll",
						   arg, argcnt);
			argcnt = 0;
			XtSetArg(arg[argcnt], XmNeditMode, XmMULTI_LINE_EDIT);
			argcnt++;
			XtSetArg(arg[argcnt], XmNcolumns, cols);
			argcnt++;
			XtSetArg(arg[argcnt], XmNrows, rows);
			argcnt++;
			if (value) {
				XtSetArg(arg[argcnt], XmNvalue, value);
				argcnt++;
			}
			ChildWidget = XmCreateText(w, widget_name, arg, argcnt);
			XtManageChild(ChildWidget);
			XtOverrideTranslations(ChildWidget,
				XtParseTranslationTable(text_translations));
			if (!hw->html.focus_follows_mouse)
				XtOverrideTranslations(ChildWidget,
				      XtParseTranslationTable(traversal_table));
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);
		} else {
			/* If no type, assume type=text.  Single line field */
			int cols = 40;

			/* SIZE can be COLUMNS, assume a TEXTFIELD */
			type = W_TEXTFIELD;
			if (tptr = ParseMarkTag(text, MT_INPUT, "SIZE")) {
				cols = atoi(tptr);
				free(tptr);
			}
			/* Grab the starting value of text.  NULL if none. */
			if (!wlist) {
				value = ParseMarkTag(text, MT_INPUT, "VALUE");
			} else {
				value = wlist->cached_text;
				wlist->cached_text = NULL;
			}
			/* Parse maxlength and set up the widget. */
			if (tptr = ParseMarkTag(text, MT_INPUT, "MAXLENGTH")) {
				maxlength = atoi(tptr);
				free(tptr);
			} else {
				maxlength = -1;
			}
			argcnt = 0;
			XtSetArg(arg[argcnt], XmNx, x);
			argcnt++;
			XtSetArg(arg[argcnt], XmNy, y);
			argcnt++;
			XtSetArg(arg[argcnt], XmNcolumns, cols);
			argcnt++;
			if (maxlength > 0) {
				XtSetArg(arg[argcnt], XmNmaxLength, maxlength);
				argcnt++;
			}
			if (value) {
				XtSetArg(arg[argcnt], XmNvalue, value);
				argcnt++;
			}
			w = XmCreateTextField(hw->html.view, widget_name,
					      arg, argcnt);
			XtOverrideTranslations(w,
				XtParseTranslationTable(text_translations));
			if (!hw->html.focus_follows_mouse)
				XtOverrideTranslations(w, 
				      XtParseTranslationTable(traversal_table));
			XtSetMappedWhenManaged(w, False);
			XtManageChild(w);

			/* For textfields, a CR might be an activate */
			if (fptr)
				XtAddCallback(w, XmNactivateCallback,
					      (XtCallbackProc)CBActivateField,
					      (XtPointer)fptr);
		}
		free(type_str);
		/*
		 * Don't want to do SetValues if this is HIDDEN input
		 * tag with no widget.
		 */
		if (w) {
			argcnt = 0;
			XtSetArg(arg[argcnt], XmNwidth, &width);
			argcnt++;
			XtSetArg(arg[argcnt], XmNheight, &height);
			argcnt++;
			XtGetValues(w, arg, argcnt);
			/* Set it to default so we don't lose it on "back" */
                        XtVaSetValues(w,
                               XmNbackground, bgcolor,
                               XmNtopShadowColor, hw->html.top_color_SAVE,
                               XmNbottomShadowColor, hw->html.bottom_color_SAVE,
                               NULL);
		} else {
			width = 0;
			height = 0;
		}
		if (!wlist) {
			wptr = AddNewWidget(hw, w, type, id, x, y, width,
					    height, name, value, onclick,
					    mapping, checked, display, pcc);
		} else {
			/* Already there with cached data */ 
			wptr = wlist;
			wptr->w = w;
			wptr->data_cached = False;
			wptr->x = x;
			wptr->y = y;
			wptr->mapping = mapping;
			if (type == W_OPTIONMENU) {
				wptr->value = value;
			} else if (value) {
				free(value);
			}
			if (name)
				free(name);
			if (onclick)
				free(onclick);
		}
		wptr->child = ChildWidget;
	} else {
		/*
		 * We found this widget on the list of already created widgets.
		 * Put it in place for reuse.
		 */
		wlist->x = x;
		wlist->y = y;
		/*
		 * Don't want to SetValues if type HIDDEN which
		 * has no widget.
		 */
		if (wlist->w) {
			if ((wlist->type == W_CHECKBOX) ||
			    (wlist->type == W_RADIOBOX)) {
				/* Make them match current background */
				bgcolor = pcc->bg;
			} else if (wlist->type == W_PUSHBUTTON) {
				bgcolor = hw->html.formbuttonbackground;
			}
			/* Set it to default so we don't lose it on "back" */
                        XtVaSetValues(wlist->w,
			       XmNx, x,
			       XmNy, y,
                               XmNbackground, bgcolor,
                               XmNtopShadowColor, hw->html.top_color_SAVE,
                               XmNbottomShadowColor, hw->html.bottom_color_SAVE,
                               NULL);
		}
		wptr = wlist;
	}
	return(wptr);
}

void WidgetRefresh(HTMLWidget hw, ElemInfo *eptr)
{
	WidgetInfo *widget_data = eptr->widget_data;

	if (widget_data && !widget_data->mapped && widget_data->w &&
	    widget_data->display) {
		widget_data->mapped = True;
		widget_data->seeable = 1;
		XtSetMappedWhenManaged(widget_data->w, True);
	}
}

/* Place a Widget and add an element record for it. */
void WidgetPlace(HTMLWidget hw, MarkInfo *mptr, PhotoComposeContext *pcc)
{
	WidgetInfo *widget_data;
	int width, height, baseline;
	int extra_after = 0;
	int extra = 0;

        pcc->widget_id++;      /* Get a unique element id */

	/* Force space in front of it if preceeded by text or not in table */
	if ((pcc->have_space_after || !pcc->in_table) && !pcc->is_bol) {
	        int dir, ascent, descent;
		XCharStruct all;

		XTextExtents(pcc->cur_font, " ", 1, &dir, &ascent,
                	     &descent, &all);
        	extra = all.width;
        }
	pcc->x += extra;

	widget_data = MakeWidget(hw, mptr->start, pcc, pcc->widget_id);
	if (!widget_data) {
#ifndef DISABLE_TRACE
		if (reportBugs || htmlwTrace)
			fprintf(stderr,
				"[WidgetPlace] Failure in MakeWidget\n");
#endif
		return;
	}
	width = widget_data->width;
	widget_data->extra_before = extra;
        /*
         * Only after we have placed the widget do we know its dimensions.
         * So now look and see if the widget is too wide, and if so go
         * back and insert a linebreak.
         */
	/* Don't miss with if preformatted or in table size calculation */
        if (!pcc->preformat && !pcc->cw_only) {
                if ((pcc->x + width) >
		    (pcc->eoffsetx + pcc->left_margin + pcc->cur_line_width)) {
			ConditionalLineFeed(hw, 1, pcc);
			widget_data = MakeWidget(hw, mptr->start, pcc,
						 pcc->widget_id);
			widget_data->extra_before = 0;
		}
        }
     	/* Add a little space after in most cases */
	if ((widget_data->type != W_CHECKBOX) &&
	    (widget_data->type != W_RADIOBOX))
		extra_after = IMAGE_DEFAULT_BORDER;

	height = widget_data->height + pcc->cur_font->descent;
	baseline = widget_data->height / 2 + pcc->cur_font->ascent / 2;

	if (!pcc->cw_only) { 
	        ElemInfo *eptr;

		/* Implicit label */
		if (pcc->in_label && pcc->label_id) {
			/* Attached to only one form element */
			mptr->anc_name = pcc->label_id;
			pcc->label_id = NULL;
			CreateAnchorElement(hw, mptr, pcc);
			pcc->in_label = 0;
		}
		eptr = CreateElement(hw, E_WIDGET, pcc->cur_font, pcc->x,
			             pcc->y, width, height, baseline, pcc);
                eptr->underline_number = 0;  /* Widgets can't be underlined */
		AdjustBaseLine(eptr, pcc);
		eptr->widget_data = widget_data;
		widget_data->eptr = eptr;
	} else {                   
		if (pcc->computed_min_x < (width + pcc->eoffsetx +
					   pcc->left_margin + extra))
                	pcc->computed_min_x = width + pcc->eoffsetx +
					      pcc->left_margin + extra;
		if (pcc->nobr && (pcc->computed_min_x <
		     (pcc->nobr_x + width + extra + extra_after)))
			pcc->computed_min_x = pcc->nobr_x + width + extra +
					      extra_after;
		if ((pcc->x + width + extra_after) > pcc->computed_max_x)
                	pcc->computed_max_x = pcc->x + width + extra_after;
                if (pcc->cur_line_height < height)           
			pcc->cur_line_height = height;        
		HTMLFreeWidgetInfo(widget_data);
        }
	pcc->x += width + extra_after;
	pcc->have_space_after = 0;
        pcc->is_bol = False;
	pcc->pf_lf_state = 0;
	if (pcc->cw_only && pcc->nobr)
		pcc->nobr_x += width + extra_after;
}

void *HTMLGetWidgetInfo(Widget w)
{
	HTMLWidget hw = (HTMLWidget)w;

	return((void *)hw->html.widget_list);
}

void *HTMLGetFormInfo(Widget w)
{
	HTMLWidget hw = (HTMLWidget)w;

	return((void *)hw->html.form_list);
}

void HTMLCacheWidgetInfo(void *ptr)
{
	WidgetInfo *wptr = (WidgetInfo *)ptr;

#ifndef DISABLE_TRACE
	if (htmlwTrace)
		fprintf(stderr,	"HTML: Entering HTMLCacheWidgetInfo!\n");
#endif
	 while (wptr) {
		if (wptr->w && (wptr->type != W_LIST)) {
			if (wptr->mapped)
				XtMoveWidget(wptr->w, -1000, -1000);

			switch (wptr->type) {
			    case W_CHECKBOX:
			    case W_RADIOBOX:
				if (XmToggleButtonGetState(wptr->w)) {
					wptr->toggled = True;
				} else {
					wptr->toggled = False;
				}
				break;
			    case W_TEXTFIELD: {
				char *xstr = XmTextFieldGetString(wptr->w);

				if (xstr) {
					wptr->cached_text = strdup(xstr);
					XtFree(xstr);
				} else {
					wptr->cached_text = NULL;
				}
				break;
			    }
			    case W_TEXTAREA: {
				char *xstr;
				Widget child;
				Arg arg[2];

				XtSetArg(arg[0], XmNworkWindow, &child);
				XtGetValues(wptr->w, arg, 1);
				xstr = XmTextGetString(child);
				if (xstr) {
					wptr->cached_text = strdup(xstr);
					XtFree(xstr);
				} else {
					wptr->cached_text = NULL;
				}
				break;
			    }
			    case W_OPTIONMENU: {
				char *val = NULL;
				Widget child;
				Arg arg[2];
				XmString label;

				child = XmOptionButtonGadget(wptr->w);
				XtSetArg(arg[0], XmNlabelString, &label);
				XtGetValues(child, arg, 1);
				XmStringGetLtoR(label, XmSTRING_DEFAULT_CHARSET,
						&val);
				XmStringFree(label);
				if (val) {
					if (*val) {
				        	wptr->cached_text = strdup(val);
					} else {
						wptr->cached_text = NULL;
					}
					XtFree(val);
				}
				if (wptr->mapping) {
					int cnt = 0;
					char **map = wptr->mapping;

					while (map[cnt])
						free(map[cnt++]);
					free((char *)map);
					wptr->mapping = NULL;
				}
				/* Must destroy the submenu separately */
				child = NULL;
				XtSetArg(arg[0], XmNsubMenuId, &child);
				XtGetValues(wptr->w, arg, 1);
				if (child)
					 XtDestroyWidget(child);
				break;
			    }
			    case W_PASSWORD:
				if (wptr->password) {
					wptr->cached_text = 
							 strdup(wptr->password);
				} else {
					wptr->cached_text = NULL;
				}
			}
			wptr->data_cached = True;
			wptr->mapped = False;
			wptr->seeable = 0;
			XtDestroyWidget(wptr->w);
			wptr->w = NULL;
		} else if (wptr->w && (wptr->type == W_LIST) && wptr->mapping) {
			int cnt = 0;
			char **map = wptr->mapping;

			while (map[cnt])
				free(map[cnt++]);
			free((char *)map);
			wptr->mapping = NULL;
			/* Mark cached, so mapping gets recreated */
			wptr->data_cached = True;
		}
		wptr = wptr->next;
	}
}

void HTMLFreeWidgetInfo(void *ptr)
{
	WidgetInfo *wptr = (WidgetInfo *)ptr;
	WidgetInfo *tptr;

#ifndef DISABLE_TRACE
	if (htmlwTrace)
		fprintf(stderr,	"HTML: Entering HTMLFreeWidgetInfo!\n");
#endif
	while (wptr) {
		tptr = wptr;
		wptr = wptr->next;
		if (tptr->w) {
			/* This is REALLY DUMB, but X generates an expose event
			 * for the destruction of the Widget, even if it isn't
			 * mapped at the time it is destroyed.
			 * So I move the invisible widget to -1000,-1000
			 * before destroying it, to avoid a visible flash.
			 */     
			if (tptr->mapped)
				XtMoveWidget(tptr->w, -1000, -1000);
			if (tptr->type == W_OPTIONMENU) {
				/* Must destroy the submenu separately */
				Widget child = NULL;
				Arg arg[2];

				XtSetArg(arg[0], XmNsubMenuId, &child);
				XtGetValues(tptr->w, arg, 1);
				if (child)
					 XtDestroyWidget(child);
			}
			XtDestroyWidget(tptr->w);
		}
		if (tptr->name)
			free(tptr->name);
		if (tptr->value && (tptr->type != W_OPTIONMENU))
			free(tptr->value);
		if (tptr->onclick)
			free(tptr->onclick);
		if (tptr->password)
			free(tptr->password);
		if (tptr->mapping) {
			int cnt = 0;
			char **map = tptr->mapping;

			while (map[cnt])
				free(map[cnt++]);
			free((char *)map);
		}
		free((char *)tptr);
	}
}

/* Free up the passed linked list of parsed HTML forms, freeing
 * all memory associated with each form.
 */
void HTMLFreeFormInfo(void *ptr)
{
	FormInfo *fptr = (FormInfo *)ptr;
	FormInfo *tptr;

	while (fptr) {
		tptr = fptr->next;
		if (fptr->action)
			free(fptr->action);
		if (fptr->target)
			free(fptr->target);
		if (fptr->method)
			free(fptr->method);
		if (fptr->enctype)
			free(fptr->enctype);
		free(fptr);
		fptr = tptr;
	}
}


void traversal_forward(Widget w, XEvent *event, String *params,
		       Cardinal *num_params)
{ 
  HTMLTraverseTabGroups(w, XmTRAVERSE_NEXT_TAB_GROUP);
}


void traversal_back(Widget w, XEvent *event, String *params,
		    Cardinal *num_params)
{ 
  HTMLTraverseTabGroups(w, XmTRAVERSE_PREV_TAB_GROUP);
}


void traversal_current(Widget w, XEvent *event, String *params,
		       Cardinal *num_params)
{
  if (!skip_traversal_current) {
      HTMLTraverseTabGroups(w, XmTRAVERSE_CURRENT);
  } else {
      skip_traversal_current = 0;
  }
}


void traversal_end(Widget w, XEvent *event, String *params,
		   Cardinal *num_params)
{
  HTMLWidget hw = (HTMLWidget) w;
  Widget top;
  int i = 0;

  while (i++ < 5) {
      if (XtClass((Widget) hw) != htmlWidgetClass) {
	  hw = (HTMLWidget) XtParent((Widget) hw);
      } else {
	  break;
      }
  }

  top = (Widget) hw;
  while (!XtIsTopLevelShell(top))
      top = XtParent(top);

  if (XtClass((Widget) hw) != htmlWidgetClass) {
#ifndef DISABLE_TRACE
      if (reportBugs || htmlwTrace)
          fprintf(stderr, "Error in traversal_end action.\n");
#endif
      return;
  }
  if (!hw->html.focus_follows_mouse) {
      XtSetKeyboardFocus(top, hw->html.view);
      hw->html.widget_focus = NULL;
      /* Does nothing if we call it */
      /* HTMLTraverseTabGroups(w, XmTRAVERSE_HOME); */
  }
}

/* This function is intended to imitate XmProcessTraversal */
void HTMLTraverseTabGroups(Widget w, int how)
{
  HTMLWidget hw = (HTMLWidget) w;
  WidgetInfo *lptr;
  Widget top;
  int i;

  /* Due to the generality of this function the HTMLwidget could be anywhere */
  for (i = 0; i < 4; i++) {
      if (hw && (XtClass((Widget) hw) != htmlWidgetClass)) {
	  hw = (HTMLWidget) XtParent((Widget) hw);
      } else {
	  break;
      }
  }
  if (!hw || (XtClass((Widget) hw) != htmlWidgetClass))
      return;

  /* Make sure we have business to do */
  if (!hw->html.widget_list || hw->html.focus_follows_mouse)
      return;

  top = (Widget) hw;
  while (!XtIsTopLevelShell(top))
      top = XtParent(top);
  
  /* Get the saved one */
  lptr = hw->html.widget_focus;

  switch (how) {
    case XmTRAVERSE_NEXT_TAB_GROUP:
      if (!lptr) {
	  lptr = hw->html.widget_list;
      } else {
	  lptr = lptr->next;
	  if (!lptr)
	      /* Loop to top if at end */
	      lptr = hw->html.widget_list;
      }

      /* Skip hidden fields, buttons, etc. */
      while (lptr && (lptr->type != W_PASSWORD) &&
	     (lptr->type != W_TEXTFIELD) && (lptr->type != W_TEXTAREA))
          lptr = lptr->next;

      if (!lptr)
	  break;

      /* Automagickly scroll */
      if (XtIsManaged(hw->html.vbar) && ((lptr->y < hw->html.scroll_y) ||
	  (lptr->y > (hw->html.view_height + hw->html.scroll_y - 20)))) {
	  int val, ss, in, pg_in;
	  int amount = lptr->y - hw->html.view_height / 2;

	  if (amount < 0)
	      amount = 0;
	  XmScrollBarGetValues(hw->html.vbar, &val, &ss, &in, &pg_in);

	  if (amount > (hw->html.doc_height - ss - 5))
	      amount = hw->html.doc_height - ss - 5;

	  XmScrollBarSetValues(hw->html.vbar, amount, ss, in, pg_in, True);
      }
      if (XtClass(lptr->w) == xmScrolledWindowWidgetClass) {
	  Widget text;

	  XtVaGetValues(lptr->w,
			XmNworkWindow, &text,
			NULL);
	  XtSetKeyboardFocus(top, text);
      } else {
	  XtSetKeyboardFocus(top, lptr->w);
      }
      break;

    case XmTRAVERSE_PREV_TAB_GROUP:
      if (!lptr) {
	  lptr = hw->html.widget_list;
      } else {
	  lptr = lptr->prev;
      }

      /* Skip hidden fields, buttons, etc. */
      while (lptr && (lptr->type != W_PASSWORD) &&
	     (lptr->type != W_TEXTFIELD) && (lptr->type != W_TEXTAREA))
          lptr = lptr->prev;

      if (!lptr)
	  break;

      /* Automagickly scroll */
      if (XtIsManaged(hw->html.vbar) && (lptr->y < hw->html.scroll_y + 10)) {
	  int val, ss, in, pg_in;
	  int amount = lptr->y - hw->html.view_height / 2;

	  if (amount < 0)
	      amount = 0;
	  XmScrollBarGetValues(hw->html.vbar, &val, &ss, &in, &pg_in);

	  XmScrollBarSetValues(hw->html.vbar, amount, ss, in, pg_in, True);
      }
      if (XtClass(lptr->w) == xmScrolledWindowWidgetClass) {
	  Widget text;

	  XtVaGetValues(lptr->w,
			XmNworkWindow, &text,
			NULL);
	  XtSetKeyboardFocus(top, text);
      } else {
	  XtSetKeyboardFocus(top, lptr->w);
      }
      break;

    case XmTRAVERSE_HOME:
      if (!lptr)
	  break;

      if (XtClass(lptr->w) == xmScrolledWindowWidgetClass) {
	  Widget text;

	  XtVaGetValues(lptr->w,
			XmNworkWindow, &text,
			NULL);
	  XtSetKeyboardFocus(top, text);
      } else {
	  XtSetKeyboardFocus(top, lptr->w);
      }
      break;

    case XmTRAVERSE_CURRENT:
      lptr = hw->html.widget_list;
      
      /* Check parent to allow for text areas (lptr->w would be scroll) */
      while (lptr) {
	  if ((lptr->w == w) || (lptr->w == XtParent(w)))
	      break;
	  lptr = lptr->next;
      }
      XtSetKeyboardFocus(top, w);
  }

  /* Track it */
  hw->html.widget_focus = lptr;
}
