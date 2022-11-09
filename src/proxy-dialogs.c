/****************************************************************************
 * NCSA Mosaic for the X Window System                                      *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 * mosaic@ncsa.uiuc.edu                                                     *
 *                                                                          *
 * Copyright 1993-1995, Board of Trustees of the University of Illinois     *
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
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/Form.h>
#include <Xm/DialogS.h>
#include <Xm/List.h>
#include <Xm/MenuShell.h>
#include <Xm/Separator.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>

#ifndef VMS
#ifdef __bsdi__
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#endif /* VMS, GEC */

#include <string.h>
#include "proxy.h"
#include "mosaic.h"
#include "proxy-dialogs.h"
#include "gui.h"

#define EDIT_ERROR \
 "No entry in the proxy list is currently selected.\n\nTo edit an entry in the proxy list, select\nit with a double mouse click."
#define SAVE_ERROR "You do not have permission to write the\nfile %s to disk."
#define SAVED_AOK "%s has been saved."

#define REMOVE_ERROR \
 "No entry in the list is currently selected.\n\nTo remove an entry in the list, select\n it and then click on the remove widget."
#define COMMIT_PROXY_EMPTY_ERROR \
 "A scheme must be specified for this proxy entry."
#define COMMIT_ADDR_EMPTY_ERROR \
 "An address must be specified for this proxy entry."
#define COMMIT_PORT_EMPTY_ERROR \
 "A port number must be specified for this proxy entry."
#define COMMIT_DOMAIN_EMPTY_ERROR "A domain must be specified for this entry."

#define EDIT 0
#define ADD  1

extern Widget toplevel;
extern Widget popup_shell;

extern struct Proxy *proxy_list, *noproxy_list;
static struct ProxyDomain *pdList = NULL;

Widget ProxyDialog, EditProxyDialog, EditNoProxyDialog, EditProxyDomainDialog;

static void AddProxyToList(), ShowProxyList(), EditProxyInfo(),
	CommitProxyInfo(), DismissProxy(), ClearProxyText(), FillProxyText(),
	WriteProxies(), RemoveProxyInfo(), EditProxyDomainInfo(),
        ShowProxyDomainList(), CommitProxyDomainInfo(),
        CallEdit(), CallAdd(), CallEditDomain(), CallAddDomain(),
        CallRemoveProxy(), DestroyDialog(), PopProxyDialog(), DeleteProxy(),
        EditNoProxyInfo(), CenterDialog(), ProxyHelpWindow(),
	HelpWindow(), AppendProxy();

struct InfoFields {
	Widget proxy_text;
	Widget addr_text;
	Widget port_text;
	Widget alive;
	Widget domain_text;
	Widget trans_menu;
};

#ifdef OTHER_TRANSPORT
static void SetOptionMenuButtonLabel();
#endif

static struct Proxy *FindProxyEntry();
static struct ProxyDomain *FindProxyDomainEntry();

static mo_window *mo_main_window;

void ShowProxyDialog(mo_window *win)
{
	PopProxyDialog(win, proxy_list, TRUE);
}

void ShowNoProxyDialog(mo_window *win)
{
	PopProxyDialog(win, noproxy_list, FALSE);
}

static void PopProxyDialog(mo_window *win, struct Proxy *list, int fProxy)
{
	Widget main_form, action_area, save, scrolled;
	Widget add, edit, remove, dismiss, help;
	int n;
	Arg args[20];
	XFontStruct *font;
	XmFontList fontlist = NULL;
	static struct EditInfo *pEditInfo;
	static int fProxyDialog = 0;

	mo_main_window = win;

	if (fProxyDialog) {
		pEditInfo->proxy_list = list;
		pEditInfo->fProxy = fProxy;

		if (fProxy) {
			pEditInfo->help_file = "help-proxylist.html";
		} else {
			pEditInfo->help_file = "help-noproxylist.html";
		}
		ShowProxyList(pEditInfo);
		XtPopup(ProxyDialog, XtGrabNone);
		return;
	}

	/*
	** Try and get a nice non-proportional font.  If we can't get 
	** it, then the heck with it, just use the default.
	*/
	font = XLoadQueryFont(dsp, FONTNAME);
	if (!font)
		font = XLoadQueryFont(dsp, "fixed");
	if (font)
		fontlist = XmFontListCreate(font, XmSTRING_DEFAULT_CHARSET);
	if (!popup_shell)
		popup_shell = XtCreatePopupShell("popup_shell",
					         topLevelShellWidgetClass,
					         toplevel, NULL, 0);
	ProxyDialog = XtVaCreatePopupShell("Proxies",
					  xmDialogShellWidgetClass, popup_shell,
					  XmNdeleteResponse, XmDESTROY,
					  XmNtitle, "Proxies",
					  NULL);
	fProxyDialog = 1;

	main_form = XtVaCreateWidget("proxy_form",
		xmFormWidgetClass, ProxyDialog,
		NULL);
	/*
	** Create action area widgets
	*/
	action_area = XtVaCreateWidget("proxy_action",
		xmFormWidgetClass,	main_form,
		XmNleftAttachment,	XmATTACH_FORM,
		XmNrightAttachment,	XmATTACH_FORM,
		XmNbottomAttachment,    XmATTACH_FORM,
		XmNfractionBase, 6,
		NULL);
	save = XtVaCreateManagedWidget("Save",
		xmPushButtonWidgetClass, action_area,
		XmNtopAttachment,	XmATTACH_FORM,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	1,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	2,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	edit = XtVaCreateManagedWidget("Edit",
		xmPushButtonWidgetClass, action_area,
		XmNtopAttachment,	XmATTACH_FORM,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	2,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	3,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	add = XtVaCreateManagedWidget("Add",
		xmPushButtonWidgetClass, action_area,
		XmNtopAttachment,	XmATTACH_FORM,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	3,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	4,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	remove = XtVaCreateManagedWidget("Remove",
		xmPushButtonWidgetClass, action_area,
		XmNtopAttachment,	XmATTACH_FORM,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	4,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	5,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	dismiss = XtVaCreateManagedWidget("Dismiss",
		xmPushButtonWidgetClass, action_area,
		XmNtopAttachment,	XmATTACH_FORM,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	0,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	1,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	help = XtVaCreateManagedWidget("Help...",
		xmPushButtonWidgetClass, action_area,
		XmNtopAttachment,	XmATTACH_FORM,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	5,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	6,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);

	XtManageChild(action_area);

	/*
	** Create Scrolled List
	*/
	n = 0;
	XtSetArg(args[n], XmNwidth, 150); n++;
	XtSetArg(args[n], XmNvisibleItemCount, 10); n++;
	XtSetArg(args[n], XmNmargin, 1); n++;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNbottomWidget, action_area); n++;
	if (fontlist) {
		XtSetArg(args[n], XmNfontList, fontlist);
		n++;
	}
	scrolled = XmCreateScrolledList(main_form, "proxy_info", args, n);

	if (fontlist)
		XmFontListFree(fontlist);
	XtManageChild(scrolled);

	pEditInfo = (struct EditInfo *)calloc(1, sizeof(struct EditInfo));
	pEditInfo->scrolled = scrolled;
	pEditInfo->proxy_list = list;
	pEditInfo->fProxy = fProxy;
	if (fProxy) {
		pEditInfo->help_file = "help-proxylist.html";
	} else {
		pEditInfo->help_file = "help-noproxylist.html";
	}
        XtAddCallback(edit, XmNactivateCallback, CallEdit, pEditInfo);
	XtAddCallback(scrolled, XmNdefaultActionCallback, CallEdit, pEditInfo);
	XtAddCallback(add, XmNactivateCallback, CallAdd, pEditInfo);
        XtAddCallback(remove, XmNactivateCallback, CallRemoveProxy, pEditInfo);
        XtAddCallback(dismiss, XmNactivateCallback, DismissProxy, ProxyDialog);
	XtAddCallback(save, XmNactivateCallback, WriteProxies, pEditInfo);
	XtAddCallback(ProxyDialog, XmNdestroyCallback,
		      DestroyDialog, &fProxyDialog);
	XtAddCallback(ProxyDialog, XmNpopupCallback, CenterDialog, NULL);
	XtAddCallback(help, XmNactivateCallback, ProxyHelpWindow, pEditInfo);

	XtManageChild(main_form);

	ShowProxyList(pEditInfo);

	XtPopup(ProxyDialog, XtGrabNone);
}

static void ProxyHelpWindow(Widget w, XtPointer client, XtPointer call)
{
	HelpWindow(w, ((struct EditInfo *)client)->help_file, call);
}

static void HelpWindow(Widget w, XtPointer client, XtPointer call)
{
	mo_open_another_window(mo_main_window,
			       mo_assemble_help_url((char *)client),
			       NULL, NULL);
}

static void CenterDialog(Widget dialog, XtPointer client, XtPointer call)
{
	Dimension width, height, dia_width, dia_height;
	Position x, y;

	XtVaGetValues(mo_main_window->base,
		      XmNx, &x,
		      XmNy, &y,
		      XmNwidth, &width,
		      XmNheight, &height,
		      NULL);
	XtVaGetValues(dialog,
		      XmNwidth, &dia_width,
		      XmNheight, &dia_height,
		      NULL);
	x += (width / 2) - (dia_width / 2);
	y += (height / 2) - (dia_height / 2);

	XtVaSetValues(dialog,
		      XmNx, x,
		      XmNy, y,
		      NULL);
}

static void CallEdit(Widget w, XtPointer client, XtPointer call)
{
	struct EditInfo *pEditInfo = (struct EditInfo *)client;

	if (pEditInfo->fProxy) {
		EditProxyInfo(w, client, call, EDIT);
	} else {
		EditNoProxyInfo(w, client, call, EDIT);
	}
}

static void CallAdd(Widget w, XtPointer client, XtPointer call)
{
	struct EditInfo *pEditInfo = (struct EditInfo *)client;
	struct ProxyDomain *p, *next;

	if (pdList) {
		p = pdList; 
		while (p) {
			next = p->next;
			DeleteProxyDomain(p);
			p = next;
		}
		pdList = NULL; 
	}

	if (pEditInfo->fProxy) {
		EditProxyInfo(w, client, call, ADD);
	} else {
		EditNoProxyInfo(w, client, call, ADD);
	}
}

static void CallRemoveProxy(Widget w, XtPointer client, XtPointer call)
{
	RemoveProxyInfo(w, client, call, PROXY);
}

static void CallRemoveProxyDomain(Widget w, XtPointer client, XtPointer call)
{
	RemoveProxyInfo(w, client, call, PROXY_DOMAIN);
}


static XmString GetStringFromScrolled(Widget w)
{
	int selected_count;
	XmStringTable selected_table;

	XtVaGetValues(w,
		      XmNselectedItemCount, &selected_count,
		      XmNselectedItems, &selected_table,
		      NULL);
	if (!selected_count)
		return NULL;

	return selected_table[0];
}

static void EditNoProxyInfo(Widget w, XtPointer client, XtPointer call,int type)
{
	Widget text_form, form, address, port;
	Widget main_form, rc, rc2, commit;
	Widget action_area, sep, dismiss, help;
	XmString selected_string;
	char *selected_text;
	struct EditInfo *pEditInfo = (struct EditInfo *)client;
	static int fEditProxyDialog = 0;

	/*
	** We obtain information from the client pointer, rather than getting
	** it from call->item because this routine can be called from
	** a pushbutton as well as from double clicking the list.
	*/
	pEditInfo->type = type;
	if (type == EDIT) {
		selected_string = GetStringFromScrolled(
						   (Widget)pEditInfo->scrolled);
		if (!selected_string) {
			XmxMakeErrorDialog(mo_main_window->base, EDIT_ERROR,
					   "No Entry Selected");
			return;
		}
		XmStringGetLtoR(selected_string, XmSTRING_DEFAULT_CHARSET,
				&selected_text);
		pEditInfo->editing = FindProxyEntry(pEditInfo, selected_text);

		XtFree(selected_text);

		if (!pEditInfo->editing)
			return;  /* How did *that* happen? */
	} else {
		pEditInfo->editing = NULL;
	}

	if (fEditProxyDialog) {
		if (type == EDIT) {
			FillProxyText(pEditInfo);
		} else {
			ClearProxyText(pEditInfo);
		}
		XtPopup(EditNoProxyDialog, XtGrabNone);
		return;
	}
	EditNoProxyDialog = XtVaCreatePopupShell("Proxies",
		xmDialogShellWidgetClass, XtParent(w),
		XmNdeleteResponse, XmDESTROY,
		XmNtitle, "No_Proxy Information",
		NULL);

	fEditProxyDialog = 1;
	main_form = XtVaCreateWidget("edit_form",
		xmFormWidgetClass, EditNoProxyDialog,
		NULL);
	/*
	** Create action area widgets
	*/
	action_area = XtVaCreateWidget("edit_action",
		xmFormWidgetClass,	main_form,
		XmNleftAttachment,	XmATTACH_FORM,
		XmNrightAttachment,	XmATTACH_FORM,
		XmNbottomAttachment,    XmATTACH_FORM,
		XmNfractionBase, 3,
		NULL);
	sep = XtVaCreateManagedWidget("separator",
		xmSeparatorWidgetClass, action_area,
		XmNleftAttachment,	XmATTACH_FORM,
		XmNrightAttachment,	XmATTACH_FORM,
		NULL);
	commit = XtVaCreateManagedWidget("Commit",
		xmPushButtonWidgetClass, action_area,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	1,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	2,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	dismiss = XtVaCreateManagedWidget("Abort",
		xmPushButtonWidgetClass, action_area,
		XmNtopAttachment,	XmATTACH_WIDGET,
		XmNtopWidget,		sep,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	0,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	1,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	help = XtVaCreateManagedWidget("Help...",
		xmPushButtonWidgetClass, action_area,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	2,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	3,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);

	XtManageChild(action_area);

	rc = XtVaCreateWidget("rowcolumn",
		xmRowColumnWidgetClass, main_form,
		XmNtopAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_FORM,
		XmNorientation,		XmHORIZONTAL,
		XmNbottomAttachment,	XmATTACH_WIDGET,
		XmNbottomWidget,	action_area,
		NULL);
	text_form = XtVaCreateWidget("text_form",
		xmFormWidgetClass, rc,
		NULL);
	rc2 = XtVaCreateWidget("rowcolumn2",
		xmRowColumnWidgetClass, text_form,
		XmNtopAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_FORM,
		NULL);

	pEditInfo->IF = (struct InfoFields *)calloc(1,
						    sizeof(struct InfoFields));
	/** Calloc sets them
	pEditInfo->IF->proxy_text = NULL;
	pEditInfo->IF->domain_text = NULL;
	pEditInfo->IF->trans_menu = NULL;
	pEditInfo->IF->alive = NULL;
	**/

	form = XtVaCreateWidget("form1",
		xmFormWidgetClass, rc2,
		XmNfractionBase, 10,
		NULL);
	address = XtVaCreateManagedWidget("Address",
		xmLabelWidgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 0,
		XmNalignment, XmALIGNMENT_END,
		NULL);
	pEditInfo->IF->addr_text = XtVaCreateManagedWidget("addr_text",
		xmTextFieldWidgetClass, form,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 3,
		XmNrightAttachment, XmATTACH_FORM,
		NULL);

	XtManageChild(form);
		
	form = XtVaCreateWidget("form2",
		xmFormWidgetClass, rc2,
		XmNfractionBase, 10,
		NULL);
	port = XtVaCreateManagedWidget("Port",
		xmLabelWidgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 0,
		XmNalignment, XmALIGNMENT_END,
		NULL);
	pEditInfo->IF->port_text = XtVaCreateManagedWidget("port_text",
		xmTextFieldWidgetClass, form,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 3,
		XmNmaxLength, 5,
		XmNcolumns, 6,
		NULL);
	XtManageChild(form);
	XtManageChild(rc2);
	XtManageChild(text_form);
	XtManageChild(rc);
	XtManageChild(main_form);

        XtAddCallback(commit, XmNactivateCallback, CommitProxyInfo, pEditInfo);
        XtAddCallback(dismiss, XmNactivateCallback, DismissProxy,
		      EditNoProxyDialog);
	XtAddCallback(EditNoProxyDialog, XmNdestroyCallback, DestroyDialog,
		      &fEditProxyDialog);
	XtAddCallback(EditNoProxyDialog, XmNpopupCallback, CenterDialog, NULL);
	XtAddCallback(help, XmNactivateCallback, HelpWindow,
		      "help-noproxy-edit.html");

	if (type == EDIT) {
		FillProxyText(pEditInfo);
	} else {
		ClearProxyText(pEditInfo);
	}
	XtPopup(EditNoProxyDialog, XtGrabNone);
}

static void EditProxyInfo(Widget w, XtPointer client, XtPointer call, int type)
{
	Widget text_form, form, protocol, address, port;
	Widget trans_rc, label, main_form, rc, rc2, rc3;
	Widget action_area, sep, dismiss, help, commit;
	XmString selected_string;
	char *selected_text;
	struct EditInfo *pEditInfo = (struct EditInfo *)client;
#ifdef OTHER_TRANSPORT
	XmString trans_string, http_string, cci_string;
	int trans_val;
#endif
	static int fEditProxyDialog = 0;
	static Widget add, remove, edit;

	/*
	** We obtain information from the client pointer, rather than getting
	** it from call->item because this routine can be called from
	** a pushbutton as well as from double clicking the list.
	*/
	pEditInfo->type = type;
	if (type == EDIT) {
		selected_string = GetStringFromScrolled(
						   (Widget)pEditInfo->scrolled);
		if (!selected_string) {
			XmxMakeErrorDialog(mo_main_window->base, EDIT_ERROR,
					   "No Entry Selected");
			return;
		}
		XmStringGetLtoR(selected_string, XmSTRING_DEFAULT_CHARSET,
				&selected_text);
		pEditInfo->editing = FindProxyEntry(pEditInfo, selected_text);

		XtFree(selected_text);

		if (!pEditInfo->editing)
			return;  /* How did *that* happen? */
	} else {
		pEditInfo->editing = NULL;
	}

	if (fEditProxyDialog) {
		if (type == EDIT) {
			FillProxyText(pEditInfo);
#ifdef OTHER_TRANSPORT
			SetOptionMenuButtonLabel(pEditInfo->IF->trans_menu,
				 		 pEditInfo->editing->transport);
#endif
		} else {
			ClearProxyText(pEditInfo);
#ifdef OTHER_TRANSPORT
			SetOptionMenuButtonLabel(pEditInfo->IF->trans_menu,
				 		 "http");
#endif
		}
		XtPopup(EditProxyDialog, XtGrabNone);
		return;
	}

	EditProxyDialog = XtVaCreatePopupShell("Proxies",
		xmDialogShellWidgetClass, XtParent(w),
		XmNdeleteResponse, XmDESTROY,
		XmNtitle, "Proxy Information",
		NULL);

	fEditProxyDialog = 1;
	main_form = XtVaCreateWidget("edit_form",
		xmFormWidgetClass, EditProxyDialog,
		NULL);
	/*
	** Create action area widgets
	*/
	action_area = XtVaCreateWidget("edit_action",
		xmFormWidgetClass,	main_form,
		XmNleftAttachment,	XmATTACH_FORM,
		XmNrightAttachment,	XmATTACH_FORM,
		XmNbottomAttachment,    XmATTACH_FORM,
		XmNfractionBase, 3,
		NULL);
	sep = XtVaCreateManagedWidget("separator",
		xmSeparatorWidgetClass, action_area,
		XmNleftAttachment,	XmATTACH_FORM,
		XmNrightAttachment,	XmATTACH_FORM,
		NULL);
	commit = XtVaCreateManagedWidget("Commit",
		xmPushButtonWidgetClass, action_area,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	1,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	2,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	dismiss = XtVaCreateManagedWidget("Abort",
		xmPushButtonWidgetClass, action_area,
		XmNtopAttachment,	XmATTACH_WIDGET,
		XmNtopWidget,		sep,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	0,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	1,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	help = XtVaCreateManagedWidget("Help...",
		xmPushButtonWidgetClass, action_area,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	2,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	3,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);

	XtManageChild(action_area);

	rc = XtVaCreateWidget("rowcolumn",
		xmRowColumnWidgetClass, main_form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNorientation, XmHORIZONTAL,
		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget, action_area,
		NULL);
	text_form = XtVaCreateWidget("text_form",
		xmFormWidgetClass, rc,
		NULL);
	rc2 = XtVaCreateWidget("rowcolumn2",
		xmRowColumnWidgetClass, text_form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		NULL);
	form = XtVaCreateWidget("form1",
		xmFormWidgetClass, rc2,
		XmNfractionBase, 10,
		NULL);
	protocol = XtVaCreateManagedWidget("Scheme",
		xmLabelWidgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 0,
		XmNalignment, XmALIGNMENT_END,
		NULL);

	pEditInfo->IF = (struct InfoFields *)calloc(1,
						    sizeof(struct InfoFields));
	pEditInfo->IF->proxy_text= XtVaCreateManagedWidget("proxy_text",
		xmTextFieldWidgetClass, form,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 3,
		XmNrightAttachment, XmATTACH_FORM,
		NULL);

	XtManageChild(form);

	form = XtVaCreateWidget("form2",
		xmFormWidgetClass, rc2,
		XmNfractionBase, 10,
		NULL);
	label = XtVaCreateManagedWidget("Proxy",
		xmLabelWidgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 0,
		XmNalignment, XmALIGNMENT_BEGINNING,
		NULL);
	address = XtVaCreateManagedWidget("Address",
		xmLabelWidgetClass, form,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, label,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 0,
		XmNalignment, XmALIGNMENT_BEGINNING,
		NULL);
	pEditInfo->IF->addr_text = XtVaCreateManagedWidget("addr_text",
		xmTextFieldWidgetClass, form,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 3,
		XmNrightAttachment, XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_FORM,
		NULL);

	XtManageChild(form);

	form = XtVaCreateWidget("form3",
		xmFormWidgetClass, rc2,
		XmNfractionBase, 10,
		NULL);
	label = XtVaCreateManagedWidget("Proxy",
		xmLabelWidgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 0,
		XmNalignment, XmALIGNMENT_BEGINNING,
		NULL);
	port = XtVaCreateManagedWidget("Port",
		xmLabelWidgetClass, form,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, label,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 0,
		XmNalignment, XmALIGNMENT_BEGINNING,
		NULL);
	pEditInfo->IF->port_text = XtVaCreateManagedWidget("port_text",
		xmTextFieldWidgetClass, form,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 3,
		XmNtopAttachment, XmATTACH_FORM,
		XmNmaxLength, 5,
		XmNcolumns, 6,
		NULL);

	XtManageChild(form);

	form = XtVaCreateWidget("form4",
		xmFormWidgetClass, rc2,
		XmNfractionBase, 10,
		NULL);
	label = XtVaCreateManagedWidget("Alive",
		xmLabelWidgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 0,
		XmNalignment, XmALIGNMENT_BEGINNING,
		NULL);
	/* Minor memory leak on XmNlabelString */
	pEditInfo->IF->alive = XtVaCreateManagedWidget("alive",
		xmToggleButtonWidgetClass, form,
		XmNlabelString, XmStringCreateSimple(""),
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 3,
		XmNrightAttachment, XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_FORM,
		NULL);

	XtManageChild(form);

#ifdef OTHER_TRANSPORT
	trans_string = XmStringCreateSimple("Transport Method");
	http_string = XmStringCreateSimple("http");
	cci_string = XmStringCreateSimple("cci");

	/*
	** If we're editing, start option menu with the value specified
	** otherwise, we're adding, so default to TRANS_HTTP.
	*/
	if (type == EDIT) {
		trans_val = pEditInfo->editing->trans_val;
	} else {
		trans_val = TRANS_HTTP;
	}
	pEditInfo->IF->trans_menu = XmVaCreateSimpleOptionMenu(rc2,
		"trans_menu",
		trans_string, 'T', trans_val, NULL,
		XmVaPUSHBUTTON, http_string, 'H', NULL, NULL,
		XmVaPUSHBUTTON, cci_string, 'C', NULL, NULL,
		NULL);

	XmStringFree(trans_string);
	XmStringFree(http_string);
	XmStringFree(cci_string);

	XtManageChild(pEditInfo->IF->trans_menu);
#endif
	XtManageChild(rc2);
	XtManageChild(text_form);

	trans_rc = XtVaCreateWidget("trans_rc",
		xmRowColumnWidgetClass, rc,
		XmNorientation, XmVERTICAL, 
		NULL);
	label = XtVaCreateManagedWidget("Scheme Info",
		xmLabelWidgetClass, trans_rc,
		NULL);
	pEditInfo->translation = XmCreateScrolledList(trans_rc, "trans_info",
		NULL, 0);
	XtVaSetValues(pEditInfo->translation,
		XmNwidth, 150,
		XmNvisibleItemCount, 3,
		XmNmargin, 1,
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		XmNwidth, 150,
		XmNvisibleItemCount, 5,
		XmNmargin, 1,
		NULL);

	XtManageChild(pEditInfo->translation);

	rc3 = XtVaCreateWidget("rowcolumn3",
		xmRowColumnWidgetClass, trans_rc,
		XmNtopAttachment, XmATTACH_WIDGET,
		XmNtopWidget, pEditInfo->translation,
		XmNorientation,	XmHORIZONTAL,
		NULL);
	add = XtVaCreateManagedWidget("Add",
		xmPushButtonWidgetClass, rc3,
		NULL);
	remove = XtVaCreateManagedWidget("Remove",
		xmPushButtonWidgetClass, rc3,
		NULL);
	edit = XtVaCreateManagedWidget("Edit",
		xmPushButtonWidgetClass, rc3,
		NULL);

	XtManageChild(rc3);
	XtManageChild(trans_rc);
	XtManageChild(rc);
	XtManageChild(main_form);

        XtAddCallback(edit, XmNactivateCallback, CallEditDomain, pEditInfo);
	XtAddCallback(pEditInfo->translation, XmNdefaultActionCallback,
		      CallEditDomain, pEditInfo);
	XtAddCallback(add, XmNactivateCallback, CallAddDomain, pEditInfo);
	XtAddCallback(remove, XmNactivateCallback, CallRemoveProxyDomain,
		      pEditInfo);
        XtAddCallback(commit, XmNactivateCallback, CommitProxyInfo, pEditInfo);
        XtAddCallback(dismiss, XmNactivateCallback, DismissProxy,
		      EditProxyDialog);
	XtAddCallback(EditProxyDialog, XmNdestroyCallback, DestroyDialog,
		      &fEditProxyDialog);
	XtAddCallback(EditProxyDialog, XmNpopupCallback, CenterDialog, NULL);
	XtAddCallback(help, XmNactivateCallback, HelpWindow,
		      "help-proxy-edit.html");

	if (type == EDIT) {
		FillProxyText(pEditInfo);
	} else {
		ClearProxyText(pEditInfo);
	}
	XtPopup(EditProxyDialog, XtGrabNone);
}

static void CallEditDomain(Widget w, XtPointer client, XtPointer call)
{
	EditProxyDomainInfo(w, client, call, EDIT);
}

static void CallAddDomain(Widget w, XtPointer client, XtPointer call)
{
	EditProxyDomainInfo(w, client, call, ADD);
}

static void EditProxyDomainInfo(Widget w, XtPointer client, XtPointer call,
				int type)
{
	Widget main_form, action_area, sep, dismiss, commit, help;
	Widget rc, form, domain;
	XmString selected_string;
	char *selected_text;
	struct EditInfo *pEditInfo = (struct EditInfo *)client;
	static int fEditProxyDomainDialog = 0;

	/*
	** We obtain information from the client pointer, rather than getting
	** it from call->item because this routine can be called from
	** a pushbutton as well as from double clicking the list.
	*/
	pEditInfo->domaintype = type;
	if (type == EDIT) {
		selected_string = GetStringFromScrolled(
						(Widget)pEditInfo->translation);
		if (!selected_string) {
			XmxMakeErrorDialog(mo_main_window->base, EDIT_ERROR,
					   "No Entry Selected");
			return;
		}
		XmStringGetLtoR(selected_string, XmSTRING_DEFAULT_CHARSET,
				&selected_text);
		if (pdList) {
			pEditInfo->editingDomain = FindProxyDomainEntry(pdList,
				 				 selected_text);
		} else {
			pEditInfo->editingDomain = FindProxyDomainEntry(
				       pEditInfo->editing->list, selected_text);
		}
		XtFree(selected_text);
	}
	if (fEditProxyDomainDialog) {
		if ((type == EDIT) && pEditInfo->editingDomain) {
			XmTextSetString(pEditInfo->IF->domain_text,
					pEditInfo->editingDomain->domain);
		} else {
			XmTextSetString(pEditInfo->IF->domain_text, "");
		}
		XtPopup(EditProxyDomainDialog, XtGrabNone);
		return;
	}

	EditProxyDomainDialog = XtVaCreatePopupShell("Proxy Domain",
		xmDialogShellWidgetClass, XtParent(w),
		XmNdeleteResponse, XmDESTROY,
		XmNtitle, "Proxy Domain Information",
		NULL);

	fEditProxyDomainDialog = 1;

	main_form = XtVaCreateWidget("edit_form",
		xmFormWidgetClass, EditProxyDomainDialog,
		NULL);
	/*
	** Create action area widgets
	*/
	action_area = XtVaCreateWidget("action",
		xmFormWidgetClass,	main_form,
		XmNleftAttachment,	XmATTACH_FORM,
		XmNrightAttachment,	XmATTACH_FORM,
		XmNbottomAttachment,    XmATTACH_FORM,
		XmNfractionBase,	3,
		NULL);
	sep = XtVaCreateManagedWidget("separator",
		xmSeparatorWidgetClass, action_area,
		XmNleftAttachment,	XmATTACH_FORM,
		XmNrightAttachment,	XmATTACH_FORM,
		NULL);
	dismiss = XtVaCreateManagedWidget("Abort",
		xmPushButtonWidgetClass, action_area,
		XmNtopAttachment,	XmATTACH_WIDGET,
		XmNtopWidget,		sep,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	0,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	1,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	commit = XtVaCreateManagedWidget("Commit",
		xmPushButtonWidgetClass, action_area,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	1,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	2,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	help = XtVaCreateManagedWidget("Help...",
		xmPushButtonWidgetClass, action_area,
		XmNbottomAttachment,	XmATTACH_FORM,
		XmNleftAttachment,	XmATTACH_POSITION,
		XmNleftPosition,	2,
		XmNrightAttachment,	XmATTACH_POSITION,
		XmNrightPosition,	3,
		XmNshowAsDefault,	True,
		XmNdefaultButtonShadowThickness, 1,
		NULL);
	rc = XtVaCreateWidget("rowcolumn",
		xmRowColumnWidgetClass, main_form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_FORM,
		XmNrightAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget, action_area,
		NULL);
	form = XtVaCreateWidget("form1",
		xmFormWidgetClass, rc,
		XmNfractionBase, 10,
		NULL);
	domain = XtVaCreateManagedWidget("Scheme Info",
		xmLabelWidgetClass, form,
		XmNtopAttachment, XmATTACH_FORM,
		XmNbottomAttachment, XmATTACH_FORM,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 0,
		XmNalignment, XmALIGNMENT_END,
		NULL);
	pEditInfo->IF->domain_text= XtVaCreateManagedWidget("domain_text",
		xmTextFieldWidgetClass, form,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition, 4,
		XmNrightAttachment, XmATTACH_FORM,
		NULL);

	if ((type == EDIT) && pEditInfo->editingDomain) {
		XmTextSetString(pEditInfo->IF->domain_text,
				pEditInfo->editingDomain->domain);
	} else {
		XmTextSetString(pEditInfo->IF->domain_text, "");
	}
	XtManageChild(form);
	XtManageChild(rc);
	XtManageChild(action_area);
	XtManageChild(main_form);

        XtAddCallback(dismiss, XmNactivateCallback, DismissProxy,
		      EditProxyDomainDialog);
	XtAddCallback(commit, XmNactivateCallback, CommitProxyDomainInfo,
		      pEditInfo);
	XtAddCallback(EditProxyDomainDialog, XmNdestroyCallback, DestroyDialog,
		      &fEditProxyDomainDialog);
	XtAddCallback(EditProxyDomainDialog, XmNpopupCallback, CenterDialog,
		      NULL);
        XtAddCallback(help, XmNactivateCallback, HelpWindow,
		      "help-proxy-domain-edit.html");

	XtPopup(EditProxyDomainDialog, XtGrabNone);
}

static void ShowProxyList(struct EditInfo *pEditInfo)
{
	struct Proxy *proxy = pEditInfo->proxy_list;

	XmListDeleteAllItems(pEditInfo->scrolled);

	while (proxy) {
		AddProxyToList(pEditInfo, proxy);
		proxy = proxy->next;
	}
}

static void AddProxyToList(struct EditInfo *pEditInfo, struct Proxy *proxy)
{
	char p[256];
	XmString string;

	if (pEditInfo->fProxy) {
		sprintf(p, "%-12.12s %s:%s", proxy->scheme, proxy->address,
			proxy->port);
	} else if (proxy->port) {
		sprintf(p, "%s:%s", proxy->address, proxy->port);
	} else {
		sprintf(p, "%s", proxy->address);
	}

	string = XmStringCreateSimple(p);

	XmListAddItem(pEditInfo->scrolled, string, 0);
	XmStringFree(string);
}

static struct Proxy *FindProxyEntry(struct EditInfo *pEditInfo, char *txt)
{
	struct Proxy *p;
	int fProxy = pEditInfo->fProxy;
	char proxy[30], address[50], port[8];
	char *ptr;

	if (fProxy) {
		sscanf(txt, "%s %s", proxy, address); 
		ptr = strchr(address, ':');
		if (ptr) {  /* Which should always be true.... */
			*ptr++ = '\0';
			strcpy(port, ptr);
		}
	} else if (ptr = strchr(txt, ':')) {
		*ptr = ' '; 
		sscanf(txt, "%s %s", address, port);
	} else {
		sscanf(txt, "%s", address);
		port[0] = '\0';
	}

	p = pEditInfo->proxy_list;
	while (p) {
		if (!strcmp(p->address, address)) {
			if (fProxy == FALSE) {
				if (!port[0] && !p->port)
					break;
				if (!strcmp(p->port, port))
					break;
			} else if (!strcmp(p->scheme, proxy)) {
				if (!strcmp(p->port, port))
					break;
			}
		}
		p = p->next;
	}
	if (!p)
		return NULL;  /* Whoops */
	return p;
}

static struct ProxyDomain *FindProxyDomainEntry(struct ProxyDomain *pDomain,
						char *txt)
{
	struct ProxyDomain *p = pDomain;

	while (p) {
		if (!strcmp(p->domain, txt))
			return p;
		p = p->next;
	}
	return NULL;
}

static void FillProxyText(struct EditInfo *p)
{
	ClearProxyText(p);
	if (p->IF->proxy_text)
		XmTextSetString(p->IF->proxy_text, p->editing->scheme);
	XmTextSetString(p->IF->addr_text, p->editing->address);

	if (p->editing->port)
		XmTextSetString(p->IF->port_text, p->editing->port);

	if (p->IF->alive) {
		if (p->editing->alive) {
			XmToggleButtonSetState(p->IF->alive, False, False);
		} else {
			XmToggleButtonSetState(p->IF->alive, True, False);
		}
	}
	if (!p->editing->list)
		return;

	ShowProxyDomainList(p);
}

static void ShowProxyDomainList(struct EditInfo *pEditInfo)
{
	XmString string;
	struct ProxyDomain *p = NULL;

	XmListDeleteAllItems(pEditInfo->translation);

	/*
	** Fill in the translation domain list
	*/
	if (pdList) {
		p = pdList;
	} else if (pEditInfo->editing) {
		p = pEditInfo->editing->list;
	}
	while (p) {
		if (p->domain) {
			string = XmStringCreateSimple(p->domain);
			XmListAddItem(pEditInfo->translation, string, 0);
			XmStringFree(string);
		}
		p = p->next;
	}
}

static void ClearProxyText(struct EditInfo *p)
{
	if (p->IF->proxy_text)
		XmTextSetString(p->IF->proxy_text, "");
	if (p->IF->addr_text)
		XmTextSetString(p->IF->addr_text, "");
	if (p->IF->port_text)
		XmTextSetString(p->IF->port_text, "");
	if (p->IF->alive)
		XmToggleButtonSetState(p->IF->alive, True, False);
	if (p->translation)
		XmListDeleteAllItems(p->translation);
} 

static void CommitProxyInfo(Widget w, XtPointer client, XtPointer call)
{
#ifdef OTHER_TRANSPORT
	Widget label;
	XmString label_string;
	char *trans;
#endif
	struct EditInfo *pEditInfo = (struct EditInfo *)client;
	struct Proxy *p;
	char *proxy = NULL;
        char *addr, *port;

	if (pEditInfo->IF->proxy_text) {
		proxy = XmTextGetString(pEditInfo->IF->proxy_text);
		if (!*proxy) {
			XmxMakeErrorDialog(mo_main_window->base,
				           COMMIT_PROXY_EMPTY_ERROR,
					   "No Proxy Entered");
			XtFree(proxy);
			return;
		}
	}
	addr = XmTextGetString(pEditInfo->IF->addr_text);
	if (!*addr) {
		XmxMakeErrorDialog(mo_main_window->base,
				   COMMIT_ADDR_EMPTY_ERROR,
				   "No Address Entered");
		if (proxy)
			XtFree(proxy);
		XtFree(addr);
		return;
	}

	/* Make sure it is all lowercase */
	{
	    char *ptr;

	    for (ptr = addr; ptr && *ptr; ptr++)
		    *ptr = tolower(*ptr);
        }

	port = XmTextGetString(pEditInfo->IF->port_text);
	if (!*port) {
		XtFree(port);
		if (pEditInfo->fProxy) {
			XmxMakeErrorDialog(mo_main_window->base,
				           COMMIT_PORT_EMPTY_ERROR,
					   "No Port Entered");
			if (proxy)
				XtFree(proxy);
			if (addr)
				XtFree(addr);
			return;
		}
		port = NULL;
	}

	if (pEditInfo->type == EDIT) {
		p = pEditInfo->editing;
	} else {
		p = (struct Proxy *)calloc(1, sizeof(struct Proxy));
		pEditInfo->editing = p;
	}

	if (pEditInfo->fProxy) {
		if (p->scheme)
			free(p->scheme);
		p->scheme = strdup(proxy);
	} else {
		p->scheme = NULL;
	}
	if (proxy)
		XtFree(proxy);

	if (p->address)
		free(p->address);
	p->address = strdup(addr);
	XtFree(addr);

	if (port) {
		if (p->port)
			free(p->port);
		p->port = strdup(port);
		XtFree(port);
	} else if (p->port) {
		free(p->port);
		p->port = NULL;
	}

	if (pEditInfo->IF->alive)
		p->alive =
		 (XmToggleButtonGetState(pEditInfo->IF->alive) == True) ? 0 : 1;

#ifdef OTHER_TRANSPORT
	label = XmOptionButtonGadget(pEditInfo->IF->trans_menu);
	XtVaGetValues(label, XmNlabelString, &label_string, NULL);

	XmStringGetLtoR(label_string, XmSTRING_DEFAULT_CHARSET, &trans);
	if (p->transport)
		free(p->transport);
	p->transport = strdup(trans);
	XmStringFree(label_string);
	XtFree(trans);
#else
	if (!p->transport && pEditInfo->fProxy)
		p->transport = strdup("http");
#endif
	if (pEditInfo->type == ADD) {
		AddProxyToList(pEditInfo, p);
		AppendProxy(pEditInfo, p);
	}
	if (pdList) {
		pEditInfo->editing->list = pdList;
		pdList = NULL;
	}
	ShowProxyList(pEditInfo);

	if (pEditInfo->fProxy) {
		XtPopdown(EditProxyDialog);
	} else {
		XtPopdown(EditNoProxyDialog);
	}
}

static void CommitProxyDomainInfo(Widget w, XtPointer client, XtPointer call)
{
	char *domain;
	struct EditInfo *p = (struct EditInfo *)client;

	domain = XmTextGetString(p->IF->domain_text);
	if (!*domain) {
		XmxMakeErrorDialog(mo_main_window->base,
				   COMMIT_DOMAIN_EMPTY_ERROR,
				  "No Entry Selected");
		XtFree(domain);
		return;
	}

	/* Make sure it is all lowercase */
	{
	    char *ptr;

	    for (ptr = domain; ptr && *ptr; ptr++)
		    *ptr = tolower(*ptr);
        }

	if (p->domaintype == ADD) {
		struct ProxyDomain *pd;

		if (!pdList) {
			/*
			** Temporary list is null
			*/
			if (p->editing == NULL) {  /* Scheme being used yet? */
				pd = NULL;  /* No */
			} else {
				pd = p->editing->list;  /* Yes!  Use it */
			}
		} else {
			pd = pdList;
		}
		if (!pd) {  /* This will be the first thing in list */
			AddProxyDomain(domain, &pd);
			if (p->editing) {
				p->editing->list = pd;
			} else {
				pdList = pd;
			}
		} else {
			while (pd->next)
				pd = pd->next;
			AddProxyDomain(domain, &pd);
		}
	} else {
		p->editingDomain->domain = (char *)realloc(
				  p->editingDomain->domain, strlen(domain) + 1);
		strcpy(p->editingDomain->domain, domain);
	}
	XtFree(domain);

	ShowProxyDomainList(p);
	XtPopdown(EditProxyDomainDialog);
	return;
}

static void DismissProxy(Widget w, XtPointer client, XtPointer call)
{
	XtPopdown((Widget)client);
}

#ifdef OTHER_TRANSPORT
static void SetOptionMenuButtonLabel(Widget w, char *s)
{
	Widget label = XmOptionButtonGadget(w);
	XmString label_string = XmStringCreateSimple(s);

	XtVaSetValues(label,
		      XmNlabelString, label_string,
		      NULL);
	XmStringFree(label_string);
}
#endif

static void RemoveProxyInfo(Widget w, XtPointer client, XtPointer call,int type)
{
	XmString selected_string;
	char *selected_text;
	struct EditInfo *pEditInfo = (struct EditInfo *)client;
	
	if (type == PROXY) {
		selected_string = GetStringFromScrolled(pEditInfo->scrolled);
	} else {
		selected_string = GetStringFromScrolled(pEditInfo->translation);
	}
	if (!selected_string) {
		XmxMakeErrorDialog(mo_main_window->base, REMOVE_ERROR,
				   "No Entry Selected");
		return;
	}

	XmStringGetLtoR(selected_string, XmSTRING_DEFAULT_CHARSET,
		 	&selected_text);
	if (type == PROXY) {
		struct Proxy *pEditing = FindProxyEntry(pEditInfo,
			 				selected_text);

		DeleteProxy(pEditInfo, pEditing);
		ShowProxyList(pEditInfo);
	} else {  /* PROXY_DOMAIN */
		struct ProxyDomain *pdEntry;

		if (pdList) {
 			pdEntry = FindProxyDomainEntry(pdList, selected_text);
		} else {
 			pdEntry = FindProxyDomainEntry(
				       pEditInfo->editing->list, selected_text);
		}
		if (pdList) {
			if (pdEntry == pdList)
				pdList = pdEntry->next;
		} else if (pdEntry == pEditInfo->editing->list) {
				pEditInfo->editing->list = pdEntry->next;
		}
		if (pdEntry)
			DeleteProxyDomain(pdEntry);
		ShowProxyDomainList(pEditInfo);
	}
	XtFree(selected_text);
}

static void WriteProxies(Widget w, XtPointer client, XtPointer call)
{
	struct Proxy *p;
	struct ProxyDomain *pd;
	struct EditInfo *pEditInfo = (struct EditInfo *)client;
	FILE *fp = NULL;
	int flag;
	char msgbuf[256];
	char *specfile;

	p = pEditInfo->proxy_list;

	if (pEditInfo->fProxy) {
		if (specfile = get_pref_string(ePROXY_SPECFILE))
			fp = fopen(specfile, "w");
                if (!fp) {
                        sprintf(msgbuf, SAVE_ERROR, specfile);
			XmxMakeErrorDialog(mo_main_window->base, msgbuf,
					   "Error writing file");
			return;
		}
	} else {
		if (specfile = get_pref_string(eNOPROXY_SPECFILE))
                	fp = fopen(specfile, "w");
                if (!fp) {
                        sprintf(msgbuf, SAVE_ERROR, specfile);
			XmxMakeErrorDialog(mo_main_window->base, msgbuf,
					   "Error writing file");
			return;
		}
	}
	while (p) {
		if (p->scheme)
			fprintf(fp, "%s ", p->scheme);
		if (p->address)
			fprintf(fp, "%s ", p->address);
		if (p->port)
			fprintf(fp, "%s ", p->port);
		if (p->transport)
			fprintf(fp, "%s ", p->transport);
		pd = p->list;
		if (pd) {
			flag = 0;
			while (pd) {
				if (flag) {
					fprintf(fp, "\\\n");
				} else {
					flag = 1;
				}
				fprintf(fp, "%s ", pd->domain);
				pd = pd->next;
			}
		}
		fprintf(fp, "\n");
		p = p->next;
	}

	if (pEditInfo->fProxy) {
                sprintf(msgbuf, SAVED_AOK, specfile);
	} else {
                sprintf(msgbuf, SAVED_AOK, specfile);
	}
	XmxMakeInfoDialog(mo_main_window->base, msgbuf, "File Saved");
	fclose(fp);
}

static void DestroyDialog(Widget w, XtPointer client, XtPointer call)
{
	int *flag = (int *)client;

	*flag = 0;
}

static void AppendProxy(struct EditInfo *pEditInfo, struct Proxy *p)
{
	struct Proxy *cur = pEditInfo->proxy_list;

	p->next = NULL;
	p->prev = NULL;

	if (cur) {
		while (cur->next)
			cur = cur->next;
		p->prev = cur;
		cur->next = p;
	} else {
		pEditInfo->proxy_list = p;
		if (pEditInfo->fProxy) {
			proxy_list = p;
		} else {
			noproxy_list = p;
		}
	}
}

static void DeleteProxy(struct EditInfo *pEditInfo, struct Proxy *p)
{
	struct Proxy *cur = p;
	extern struct Proxy *proxy_list, *noproxy_list;
	struct Proxy *pl;

	if (pEditInfo->fProxy) {
		pl = proxy_list;
	} else {
		pl = noproxy_list;
	}
	/*
	** Delete proxy from list
	*/
	if (cur == pl) {
		pl = cur->next;
		if (!pl) {
			pEditInfo->proxy_list = NULL;
			if (pEditInfo->fProxy) {
				proxy_list = NULL;
			} else {
				noproxy_list = NULL;
			}
		} else {
			pEditInfo->proxy_list = pl;
			if (pEditInfo->fProxy) {
				proxy_list = pl;
			} else {
				noproxy_list = pl;
			}
		}
	}
	if (cur->next)
		cur->next->prev = p->prev;
	if (cur->prev)
		cur->prev->next = p->next;
	/*
	** Delete allocated space from proxy entry
	*/
	if (p->scheme)
		free(p->scheme);
	if (p->address)
		free(p->address);
	if (p->port)
		free(p->port);
	if (p->transport)
		free(p->transport);
	free(p);
}
