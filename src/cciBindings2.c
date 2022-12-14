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

/* Copyright (C) 2005, 2006, 2008 - The VMS Mosaic Project */

#include "../config.h"
#ifdef CCI

#include <stdio.h>
#include <sys/types.h>
#ifdef SOCKETSHR
#define _DECC_V4_SOURCE
#endif
#include "../libwww2/HTFormat.h"
#include "mosaic.h"

#ifndef WIN_TCP
#if defined(__DECC) && (__VMS_VER >= 70000000)
#define _VMS_V6_SOURCE
#endif /* avoid __UTC_STAT in VMS V7.0, GEC */
#include <sys/stat.h>
#if defined(__DECC) && (__VMS_VER >= 70000000)
#undef _VMS_V6_SOURCE
#endif
#else
#include "sys$library:stat.h"
#endif /* WIN_TCP */

#include "gui.h"
#include "cci.h"
#include "cciBindings2.h"
#include <Xm/ToggleB.h>
#include "cciServer.h"
#include "../libhtmlw/list.h"
#include "accept.h"

static Boolean cciAccepting = 0; 
static int listenPortNumber = 0;
static XtInputId connectInputID;
static List listOfConnections;
static List listOfSendOutput;
static List listOfSendAnchorTo;  /* Client in list if should receive */
static List listOfSendBrowserView;
static List listOfSendEvent;
static List listOfFileToURLs;
static List listOfForm;		 /* List of form submitted by application */

/* ADC ugly hack below  ZZZZZ */
extern int CCIprotocol_handler_found;
extern int MCCIanchorcached;

extern XtAppContext app_context;
/***/ extern HTStream* CCIPresent();
extern char *machine_with_domain;  /* Host name */

/* From cciBindings.c */
extern int cci_event;

#ifndef DISABLE_TRACE
extern int cciTrace;
#endif

struct Connection {
	XtInputId inputId;
	MCCIPort client;
};

struct SendWhatToWhom {
	MCCIPort client;
	char	*contentType;
};

struct FileURL {
	char *fileName;
	char *url;
};

struct FormSubmit{
	MCCIPort client;
	char *actionID;
	int valid;
};

cciStat *cciStatListFindEntry(MCCIPort findMe)
{
  /* This finds an entry in listOfSendAnchorTo based on MCCIPort client. */
  cciStat *current;

  current = (cciStat *) ListHead(listOfSendAnchorTo);
  while (current) {
      if (current->client == findMe)
	  break;
      current = (cciStat *) ListNext(listOfSendAnchorTo);
  }
  return current;
}
  
cciStat *cciStatListDeleteEntry(MCCIPort deleteMe)
{
  cciStat *current;

  if (current = cciStatListFindEntry(deleteMe)) {
      ListDeleteEntry(listOfSendAnchorTo, current);
      return current;
  } else {
      return NULL;
  }
}

void cciStatPreventSendAnchor(MCCIPort client, char *url)
{
  /* This sets the flag that is checked for in cciSafeToSend, but
   * only if sendAnchor is on for this client.
   */
  cciStat *current;

  if ((current = cciStatListFindEntry(client)) && url)
      current->url = strdup(url);
}

/* Ensures that Mosaic does not send an ANCHOR in response to a GET
 * if SEND ANCHOR is turned on.
 *
 * INPUTS:
 * 		url	The url for which the flag is to be set.
 * RETURNS:
 * 		returns 0 if flag set for (client,url) pair (not safe to send)
 *                      1 if flag is not set for (client,url) pair
 *			(safe to send)
 */
int cciSafeToSend(cciStat *current, char *url)
{
  int rc = 1;

  if (current->url) {
      rc = strcmp(url, current->url);
      free(current->url);
      current->url = NULL;
  }
  return rc;
}

void cciStatFree(cciStat *i)
{
  free(i);
}

cciStat *cciStatNew(MCCIPort client, int status)
{
  cciStat *new = (cciStat *) malloc(sizeof(cciStat));

  if (new) {
      new->client = client;
      new->status = status;
      new->url = NULL;
  }
  return new;
}

/* Set the state of sending anchors */
	/* 0, don't send, 1 send before browser gets document, */
	/* 2 send after browser gets document */
        /* 3 send it before and then let the CCI app handle it  ADC ZZZ */
void MoCCISendAnchor(MCCIPort client, int sendIt)
{
  cciStat *statElt;

  statElt = cciStatListDeleteEntry(client);
  switch (sendIt) {
    case 0:
      if (statElt)
	cciStatFree(statElt);
      break;
    case 1:  /* Send before browser gets document */
      if (statElt) {
	statElt->status = 1;
      } else {
	statElt = cciStatNew(client, 1);
      }
      if (statElt)
	ListAddEntry(listOfSendAnchorTo, statElt);
      break;
    case 2:  /* Send after browser gets document */
      if (statElt) {
	statElt->status = 2;
      } else {
	statElt = cciStatNew(client, 2);
      }
      if (statElt)
	ListAddEntry(listOfSendAnchorTo, statElt);
      break;
    case 3: 
      /* Send before browser gets document, then let it handle it  ADC ZZZ */
      if (statElt) {
        statElt->status = 3;
      } else {
        statElt = cciStatNew(client, 3);
      }
      if (statElt)
        ListAddEntry(listOfSendAnchorTo, statElt);
      break;
    default:
      ;
  }
}

void MoCCISendAnchorToCCI(char *url, int beforeAfter)
/***************************************************************
 * beforeAfter - 	0: send to all clients in list
 * 			1: send to all clients with status == 1;
 *			2: send to all clients with status == 2:
 *                      3: send to all clients with status == 3:  ADC ZZZ
 ***************************************************************/
{
  cciStat *client;

  client = (cciStat *) ListHead(listOfSendAnchorTo);
  while(client) {
      if (cciSafeToSend(client, url)) {  /* Test flag */
	switch (beforeAfter) {
	  case 0:
            if (client->status == 3) {                  /* ADC ZZZ */
              MCCIanchorcached = 1;
              MCCISendAnchorHistory(client->client, url);
              MCCIanchorcached = 0;
            } else {
	      MCCISendAnchorHistory(client->client, url);
	    }
	    break;
	  case 1:
	    if (client->status == 1)
	      MCCISendAnchorHistory(client->client, url);
	    break;
	  case 2:
	    if (client->status == 2)
	      MCCISendAnchorHistory(client->client, url);
	    break;                                     /* ADC ZZZ */
          case 3:
            if (client->status == 3) {
              MCCISendAnchorHistory(client->client, url);
              CCIprotocol_handler_found = 1;
            }
	    break;
	  default:
	    ;
	}
      }
    client = (cciStat *) ListNext(listOfSendAnchorTo);
  }
}

void MoCCISendEvent(MCCIPort client, int on)
{
	MCCIPort tmp;

	if (on) {
		ListAddEntry(listOfSendEvent, client);
	} else {
		ListDeleteEntry(listOfSendEvent, client);

		/* Turn off cci_event only if list of clients is NULL */
		if (!(tmp = (MCCIPort) ListHead(listOfSendEvent)))
			cci_event = 0;
	}
}

void MoCCISendMouseAnchor(MCCIPort client, int on)
{
	MCCIPort tmp;

	if (on) {
		ListAddEntry(listOfSendEvent, client);
	} else {
		ListDeleteEntry(listOfSendEvent, client);

		/* Turn off cci_event only if list of clients is NULL */
		if (!(tmp = (MCCIPort) ListHead(listOfSendEvent)))
			cci_event = 0;
	}
}

void MoCCISendBrowserView(MCCIPort client, int on)
{
	if (on) {
		ListAddEntry(listOfSendBrowserView, client);
	} else {
		ListDeleteEntry(listOfSendBrowserView, client);
	}
}

void MoCCIForm(MCCIPort client, char *actionID, int status,
	       int close_connection)
{
	struct FormSubmit *sendForm;
	int registered = 0;

	if (status) {
		sendForm = (struct FormSubmit *) ListHead(listOfForm);
		while (sendForm) {
			if ((sendForm->client == client) &&
			    !strcmp(sendForm->actionID, actionID)) {
				sendForm->valid = 1;
				registered = 1;
			}
			sendForm = (struct FormSubmit *)ListNext(listOfForm);
		}

		/* Only register if not done so alreday */
		if (!registered) {
			if (!(sendForm = (struct FormSubmit *)
					   malloc(sizeof(struct FormSubmit)))) {
				return;  /* Memory problem */
			}

			sendForm->client = client;
			sendForm->actionID = strdup(actionID);
			sendForm->valid = 1;
			ListAddEntry(listOfForm, sendForm);
		}
	} else if (close_connection) {
		sendForm = (struct FormSubmit *) ListHead(listOfForm);
		while (sendForm) {
			if (sendForm->client == client) {
				ListDeleteEntry(listOfForm, sendForm);
				free(sendForm->actionID);
				free(sendForm);
				sendForm = (struct FormSubmit *)
							ListCurrent(listOfForm);
#if 0
				sendForm->valid = 0;
#endif
			} else {
				sendForm = (struct FormSubmit *)
							   ListNext(listOfForm);
			}
		}
	} else {  /* Remove entry from list, forget the valid bit, delete it! */
		sendForm = (struct FormSubmit *) ListHead(listOfForm);
		while (sendForm) {
			if ((sendForm->client == client) &&
			    !strcmp(sendForm->actionID, actionID)) {
				ListDeleteEntry(listOfForm, sendForm);
				free(sendForm->actionID);
				free(sendForm);
				sendForm = (struct FormSubmit *)
							ListCurrent(listOfForm);
#if 0
				sendForm->valid = 0;
#endif
			} else {
				sendForm = (struct FormSubmit *)
							   ListNext(listOfForm);
			}
		}
	}

#ifndef DISABLE_TRACE
	if (cciTrace) {
		sendForm = (struct FormSubmit *) ListHead(listOfForm);
		fprintf(stderr, "***** begin mosaic list *****\n");
		while(sendForm){
			fprintf(stderr, "actionID 	%s\n",
				sendForm->actionID);
			fprintf(stderr, "clientaddress %s\n",
				sendForm->client->serverAddress);
			sendForm = (struct FormSubmit *) ListNext(listOfForm);
		}
	}
#endif
}

int MoCCIPreInitialize()
{
	static int donePreInit = 0;

	if (donePreInit)
		return 0;
	donePreInit = 1;

	listOfConnections = ListCreate();
	listOfSendOutput = ListCreate();
	listOfSendAnchorTo = ListCreate();
	listOfSendBrowserView = ListCreate();
	listOfSendEvent = ListCreate();
	listOfFileToURLs = ListCreate();
	listOfForm = ListCreate();
}


int MoCCIInitialize(int portNumber)
{
	int retVal;

	MoCCIPreInitialize();

	retVal = MCCIServerInitialize(portNumber);
	if (retVal) {
		  /* Write port number to .mosaiccciport */
    		char *home = getenv("HOME");
		char *fnam;
    		FILE *fp;

#ifndef VMS   /* PGE */
                if (!home)
                        home = "/tmp";
#endif
                fnam = (char *)malloc(strlen(home) + 32);
#ifndef VMS   /* PGE */
                sprintf(fnam, "%s/.mosaiccciport", home);
#else
                sprintf(fnam, "%smosaic.cciport", home);
#endif
    		if (fp = fopen(fnam, "w")) {
			fprintf(fp, "%s:%d\n", machine_with_domain, portNumber);
        		fclose(fp);
		}
    		free(fnam);
	}
	return(retVal);
}

void MoCCITerminateAllConnections()
{
	struct Connection *con;
	struct SendWhatToWhom *sendOutput;

	con = (struct Connection *) ListHead(listOfConnections);
	while(con) {
		XtRemoveInput(con->inputId);
		NetCloseConnection(con->client);
		MoCCISendAnchor(con->client, 0);
		MoCCISendBrowserView(con->client,0);
		MoCCISendEvent(con->client, 0);
		MoCCIForm(con->client, NULL, 0, 1);

		sendOutput =(struct SendWhatToWhom *)ListHead(listOfSendOutput);
		while (sendOutput) {          /* Remove sendOutputs */
			HTRemoveConversion(sendOutput->contentType,
				           "www/present", CCIPresent);
			ListDeleteEntry(listOfSendOutput, sendOutput);
			free(sendOutput->contentType);
			free(sendOutput);

			sendOutput = (struct SendWhatToWhom *)
						  ListCurrent(listOfSendOutput);
		}

		ListDeleteEntry(listOfConnections, con);
		con = (struct Connection *) ListHead(listOfConnections);
	}
}

void MoCCITerminateAConnection(MCCIPort client)
{
	struct Connection *con;
	struct SendWhatToWhom *sendOutput;

	con = (struct Connection *) ListHead(listOfConnections);
	while (con) {
		if (con->client == client) {
			XtRemoveInput(con->inputId);
			MoCCISendAnchor(con->client, 0);
			MoCCISendBrowserView(con->client, 0);
			MoCCISendEvent(con->client, 0);
			MoCCIForm(con->client, NULL, 0, 1);

			sendOutput = (struct SendWhatToWhom *)
				    		     ListHead(listOfSendOutput);
			while (sendOutput) {          /* Remove sendOutputs */
			  if (sendOutput->client == client) {
			    HTRemoveConversion(sendOutput->contentType,
					       "www/present", CCIPresent);
			    ListDeleteEntry(listOfSendOutput, sendOutput);
			    free(sendOutput->contentType);
			    free(sendOutput);
			  }
			  sendOutput = (struct SendWhatToWhom *)
			  			     ListNext(listOfSendOutput);
			}

			ListDeleteEntry(listOfConnections,con);
			con = (struct Connection *)
						 ListCurrent(listOfConnections);
		} else {
			con = (struct Connection *) ListNext(listOfConnections);
		}
	}
}

void MoCCIHandleInput(MCCIPort client, int source)
{
	if (MCCIIsThereInput(client))
		MCCIHandleInput(client);
}

void MoCCINewConnection(XtAppContext app_context, int *source,
			XtInputId *inputID)
{
	MCCIPort client;
	struct Connection *con;

#ifndef DISABLE_TRACE
	if (cciTrace)
		fprintf(stderr, "MoCCINewConnection(): I've been called\n");
#endif
	client = MCCICheckAndAcceptConnection();
	if (!client)
		/* Nothing here */
		return;

#ifndef DISABLE_TRACE
	if (cciTrace)
		fprintf(stderr, "Mosaic: got a CCI connection\n");
#endif
	/*
	 * This determines wether there is one or many cci clients connecting
	 * XtRemoveInput(*inputID);
	 */
	MCCISendResponseLine(client, MCCIR_OK, "VERSION 01 X Mosaic 3.0");

	con = (struct Connection *) malloc(sizeof(struct Connection));
	con->client = client;
	con->inputId = XtAppAddInput(app_context,
		    MCCIGetSocketDescriptor(client),
		    (XtPointer) XtInputReadMask,
		    (XtInputCallbackProc) MoCCIHandleInput, (XtPointer) client);
	ListAddEntry(listOfConnections, con);
}

static XmxCallback (MoCCIWindowCallBack)
{
   Boolean newState;
   mo_window *win = mo_fetch_window_by_id(XmxExtractUniqid((int)client_data));
   int newPort;
   char *portString;
   char buff[256];

   newState = cciAccepting;
   switch (XmxExtractToken((int)client_data)) {
       case 0:  /* OK */
  	    newState = XmToggleButtonGetState(win->cci_accept_toggle);
	    XtUnmanageChild(win->cci_win);
	    break;
       case 1:  /* Cancel */
	    if (XmToggleButtonGetState(win->cci_accept_toggle) !=
		cciAccepting) {
		/* Reset toggle state */
		XmxSetToggleButton(win->cci_accept_toggle, cciAccepting);
		XmxSetToggleButton(win->cci_off_toggle, !cciAccepting);
	    }
	    XtUnmanageChild(win->cci_win);
	    break;
       case 2:  /* Help */
	   mo_open_another_window (win,
		          mo_assemble_help_url("docview-menubar-file.html#cci"),
			  NULL, NULL);
	   break;
   }
   if (newState != cciAccepting) {
	/* Toggle state has changed */
	cciAccepting = newState;
	if (cciAccepting) {
		newPort = 0;
	        portString = XmxTextGetString(win->cci_win_text);
		if (portString) {
			newPort = atoi(portString);
			XtFree(portString);
		}
		if ((newPort < 1024) || (newPort > 65535)) {
                        XmxMakeErrorDialog(win->cci_win,
                              "CCI port address must be between 1024 and 65535",
                              "CCI port address error");
			cciAccepting = False;
			listenPortNumber = 0;
			XmxSetToggleButton(win->cci_accept_toggle,
					   cciAccepting);
			XmxSetToggleButton(win->cci_off_toggle, !cciAccepting);
			return;
		}

		if (MoCCIInitialize(newPort)) {
			listenPortNumber = newPort;
			connectInputID = XtAppAddInput(app_context,
                        	       MCCIReturnListenPortSocketDesc(),
	                               (XtPointer) XtInputReadMask,
				       (XtInputCallbackProc) MoCCINewConnection,
				       app_context);
			sprintf(buff, "%s %d", "CCI Now listening on port",
				newPort);
			XmxMakeInfoDialog(win->cci_win, buff,
					  "CCI port status");
		} else {
			cciAccepting = False;
			listenPortNumber = 0;
			XmxSetToggleButton(win->cci_accept_toggle,
					   cciAccepting);
			XmxSetToggleButton(win->cci_off_toggle, !cciAccepting);
			/* Can't accept on this port...*/
                        XmxMakeErrorDialog (win->cci_win,
                           "CCI Can't accept requests on this port.  May be in use already.",
                           "CCI port address error");
		}
	} else {
		XtRemoveInput(connectInputID);  /* Stop accepting connections */
		MCCICloseAcceptPort(); 		/* Kill accept port */

		/* Terminate existing connections */
		MoCCITerminateAllConnections();
		listenPortNumber = 0;
		sprintf(buff, "CCI Is no longer listening");
		XmxMakeInfoDialog(win->cci_win, buff, "CCI port status");
	}
    }
}

mo_status MoDisplayCCIWindow(mo_window *win)
{
	Widget dialogFrame, dialogSeparator;
	Widget cciForm, label, toggleBox, buttonBox;

	if (!win->cci_win) {
		XmxSetUniqid(win->id);
		win->cci_win = XmxMakeFormDialog(win->base,
				         "VMS Mosaic: Common Client Interface");
		dialogFrame = XmxMakeFrame(win->cci_win, XmxShadowOut);
		XmxSetConstraints
		        (dialogFrame, XmATTACH_FORM, XmATTACH_FORM,
		         XmATTACH_FORM, XmATTACH_FORM, NULL, NULL, NULL, NULL);
		/* Main form. */
		cciForm = XmxMakeForm(dialogFrame);

		label = XmxMakeLabel(cciForm, "CCI Port Address: ");
		XmxSetArg(XmNcolumns, 25);
			win->cci_win_text = XmxMakeText(cciForm);
			XmxAddCallbackToText(win->cci_win_text,
					     MoCCIWindowCallBack, 0);

		toggleBox = XmxMakeRadioBox(cciForm);

		win->cci_accept_toggle = XmxMakeToggleButton(toggleBox,
							     "Accept requests",
							      NULL, 0);

		win->cci_off_toggle = XmxMakeToggleButton(toggleBox,
							  "Interface off",
							  NULL, 0);

		dialogSeparator = XmxMakeHorizontalSeparator(cciForm);
		buttonBox = XmxMakeFormAndThreeButtons(cciForm,
						       MoCCIWindowCallBack,
						       "Ok", "Dismiss", "Help",
						       0, 1, 2);
		XmxSetOffsets(label, 13, 0, 10, 0);
		XmxSetConstraints
			(label, XmATTACH_FORM, XmATTACH_NONE,
			XmATTACH_FORM, XmATTACH_NONE, NULL, NULL, NULL, NULL);

		XmxSetOffsets(win->cci_win_text, 10, 0, 5, 8);
		XmxSetConstraints
		        (win->cci_win_text, XmATTACH_FORM, XmATTACH_NONE,
			XmATTACH_WIDGET, XmATTACH_FORM,
			NULL, NULL, label, NULL);

		XmxSetConstraints(toggleBox, XmATTACH_WIDGET,
			XmATTACH_NONE, XmATTACH_WIDGET, XmATTACH_NONE,
			win->cci_win_text, NULL, label, NULL);
		XmxSetOffsets(toggleBox, 8, 0, 2, 0);

		XmxSetConstraints(dialogSeparator, XmATTACH_WIDGET,
			XmATTACH_NONE, XmATTACH_FORM, XmATTACH_FORM,
			toggleBox, NULL, NULL, NULL);

		XmxSetConstraints(buttonBox, XmATTACH_WIDGET,
			XmATTACH_FORM, XmATTACH_FORM, XmATTACH_FORM,
			dialogSeparator, NULL, NULL, NULL);
	}

	if (cciAccepting) {
		char buff[10];

		sprintf(buff, "%d", listenPortNumber);
		XmxTextSetString(win->cci_win_text, buff);
	} else {
		XmxTextSetString(win->cci_win_text, "");
	}
	XmxSetToggleButton(win->cci_accept_toggle, cciAccepting);
	XmxSetToggleButton(win->cci_off_toggle, !cciAccepting);

	XmxManageRemanage(win->cci_win);
	return mo_succeed;
}

void MoCCISendOutputToClient(char *contentType, char *fileName)
{
	struct SendWhatToWhom *sendOutput;

	/* Send data to all clients that registered for this content type */
	sendOutput = (struct SendWhatToWhom *)ListHead(listOfSendOutput);
	while (sendOutput) {
		if (!strcmp(sendOutput->contentType, contentType)) {
#ifndef DISABLE_TRACE
			if (cciTrace)
				fprintf(stderr,
				      "Sending output through cci of type %s\n",
				      sendOutput->contentType);
#endif
			/* Prep data for sending here */

			/* Send it back to client */
			MCCISendOutputFile(sendOutput->client,
					   contentType, fileName);
		}
		sendOutput =(struct SendWhatToWhom *)ListNext(listOfSendOutput);
	}
	return;
}

/* In order to send content to application, mosaic must call this function
 * twice, the first to set the variable send, the second to do the actual
 * sending.
*/
int MoCCIFormToClient(char *actionID, char *query, char *contentType,
		      char *post_data, int status)
{
	static int send = 0;
	struct FormSubmit *sendForm;
	int found = 0;

	/* Just make a note that it's a cciPOST */
	if (status && !send)) {
	    send = 1;
	    return(0);
	}

	if (send) {
	    send = 0;
	    sendForm = (struct FormSubmit *) ListHead(listOfForm);
	    while (sendForm) {
		if (!strcmp(actionID, sendForm->actionID)) {
		    /* Found correct client */
		    if (sendForm->valid)
			MCCIFormQueryToClient(sendForm->client, actionID,
					      query, contentType, post_data);
		    found = 1;
		}
		sendForm = (struct FormSubmit *)ListNext(listOfForm);
	    }
	}
	return found;
}

/* This function is a callback from the libwww2 Converter */

void MoCCISendOutput(MCCIPort client, Boolean sendIt, char *contentType)
{
	struct SendWhatToWhom *sendOutput;

	if (sendIt) {
		if (!(sendOutput = (struct SendWhatToWhom *) 
		      malloc(sizeof(struct SendWhatToWhom))))
			return;

		sendOutput->client = client;
		sendOutput->contentType = strdup(contentType);
		ListAddEntry(listOfSendOutput, sendOutput);

		/* Set up call back for this content Type */
		HTSetConversion(contentType, "www/present", CCIPresent,
				1.0, 0.0, 0.0);
		/* If we use this, out put a message like....sent to cci
		*/
/**		HTSetPresentation(contentType, MoCCISendOutputCB,
				  1.0, 0.0, 0.0);
*/
	} else {
		sendOutput = (struct SendWhatToWhom *)
						     ListHead(listOfSendOutput);
		while (sendOutput) {
			if ((sendOutput->client == client) &&
			    !strcmp(sendOutput->contentType, contentType)) {
				HTRemoveConversion(contentType,
						   "www/present", CCIPresent);
				ListDeleteEntry(listOfSendOutput, sendOutput);
				free(sendOutput->contentType);
				free(sendOutput);

				sendOutput = (struct SendWhatToWhom *)
						  ListCurrent(listOfSendOutput);
			} else {
				sendOutput = (struct SendWhatToWhom *)
						     ListNext(listOfSendOutput);
			}
		}
	}
}

void MoCCIStartListening(Widget w, int port)
{

        if (MoCCIInitialize(port)) {
		connectInputID = XtAppAddInput(app_context,
                        	       MCCIReturnListenPortSocketDesc(),
                        	       (XtPointer) XtInputReadMask,
                        	       (XtInputCallbackProc) MoCCINewConnection,
				       app_context);
		cciAccepting = True;
		listenPortNumber = port;
 /*
		XmxMakeInfoDialog(w, "Mosaic CCI port is listening",
				  "CCI port status");
 */
        } else {
		char buf[80];

		sprintf(buf, "%s %d", "Could not listen on port", port);
		XmxMakeErrorDialog(w, buf, "CCI Error");
	}
}

void MoCCISendEventOutput(CCI_events event_type)
/* Handle SendEventOutput if needed */
/* Send Event data to all the clients that have requested it */
{
	MCCIPort client = (MCCIPort) ListHead(listOfSendEvent);

	while (client) {
		MCCISendEventOutput(client, event_type);
		client = (MCCIPort) ListNext(listOfSendEvent);
	}
}


void MoCCISendMouseAnchorOutput(char *anchor)
/* Handle SendMouseAnchorOutput if needed */
/* Send MouseAnchor data to all the clients that have requested it */
{
	MCCIPort client = (MCCIPort) ListHead(listOfSendEvent);

	while (client) {
		MCCISendMouseAnchorOutput(client, anchor);
		client = (MCCIPort) ListNext(listOfSendEvent);
	}
}


void MoCCISendBrowserViewOutput(Char *url, char *contentType, char *data,
				int dataLength)
/* Handle SendBrowserViewOutput if needed */
/* Send BrowserView data to all the clients that have requested it */
{
	MCCIPort client;

	if (!dataLength)
		return;
	if (!data)
		return;
	if (!url || !strlen(url))
		return;
	if (!contentType || !strlen(contentType))
		contentType = "unknown";

	client = (MCCIPort) ListHead(listOfSendBrowserView);
	while (client) {
		MCCISendBrowserViewOutput(client, url, contentType,
					  data, dataLength);
		client = (MCCIPort) ListNext(listOfSendBrowserView);
	}
}

int MoCCISendBrowserViewFile(char *url, char *contentType, char *filename)
{
#if (stat != decc$stat) || !defined(MULTINET)
	struct stat fileInfo;
#else
#undef stat
 	struct stat fileInfo;
#define stat decc$stat
#endif  /* VMS MultiNet work around */
	FILE *fp;
	char *data;

	if (!ListHead(listOfSendBrowserView))
		return(MCCI_OK);
	if (!filename || !strlen(filename))
		return(MCCI_OK);
	if (!url || !strlen(url))
		return(MCCI_OK);
	if (!contentType || !strlen(contentType))
		contentType = "unknown";

        if (stat(filename, &fileInfo))  /* get the length of the file */
                return(MCCI_FAIL);

	if (!(fp = fopen(filename, "r")))
                return(MCCI_FAIL);
	if (!(data = (char *) malloc(fileInfo.st_size))) {
		fclose(fp);
                return(MCCI_OUTOFMEMORY);
	}
	if (fileInfo.st_size !=
	    fread(data, sizeof(char), fileInfo.st_size, fp)) {
		fclose(fp);
		free(data);
                return(MCCI_FAIL);
	}
	MoCCISendBrowserViewOutput(url, contentType, data, fileInfo.st_size);

	free(data);
	fclose(fp);
	return(MCCI_OK);
}

int MoCCIMaxConnectionsAllowed()
/* Return number of connections allowed.  This is set in the X resources.
 * If it's zero, then treat it as unlimited.
 */
{
    return(get_pref_int(eMAX_NUM_OF_CCI_CONNECTIONS));
}

int MoCCICurrentNumberOfConnections()
{
    return(ListCount(listOfConnections));
}

void MoCCIAddFileURLToList(char *fileName, char *url)
/* This routine should be called each time a url has been down loaded
 * and stored as file.  This routine adds the fileName,url pair to the
 * list for later query over cci by an external viewer (cci app).
 */
{
	struct FileURL *fileURL;

#ifndef DISABLE_TRACE
	if (cciTrace)
		fprintf(stderr,
			"MoCCIAddFileURLToList():fileName=\"%s\", url=\"%s\"\n",
			(fileName ? fileName : "NULL"), (url ? url : "NULL"));
#endif
	if (!fileName || !url)
		return;

	if (!(fileURL = (struct FileURL *) malloc(sizeof(struct FileURL))))
		return;

	fileURL->fileName = strdup(fileName);
	fileURL->url = strdup(url);
	ListAddEntry(listOfFileToURLs, fileURL);
	return;
}

char *MoReturnURLFromFileName(char *fileName)
/* Given filename, return associated URL in the list */
/* If not found, return NULL */
{
	struct FileURL *fileURL;

	fileURL = (struct FileURL *) ListHead(listOfFileToURLs);
	while (fileURL) {
		if (!strncmp(fileURL->fileName,fileName,
			     strlen(fileURL->fileName)))
			return(fileURL->url);
		fileURL = (struct FileURL *) ListNext(listOfFileToURLs);
	}
	return(NULL);
}

/* This replaces a vanilla url name with the name it was actually called with,
 * which is what is needed by external apps.  It is indeed a huge hack, and
 * should be fixed.
 */
void MoCCIAddAnchorToURL(char *url, char *urlAndAnchor)
{
  struct FileURL *fileURL;

  fileURL = (struct FileURL *) ListTail(listOfFileToURLs);
  if (fileURL && !strcmp(fileURL->url, url)) {
      free(fileURL->url);
      fileURL->url = strdup(urlAndAnchor);
  }
}

#else
int ccidummy2;  /* Shut the freaking stupid compiler up */
#endif /* CCI */
