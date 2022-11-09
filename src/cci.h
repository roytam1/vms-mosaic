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

#ifndef __CCI_H__
#define __CCI_H__

#define MCCI_MAX_RETURN_TEXT	1024

#define MCCI_VERSION		"1.0"

/* Return codes from cci api */
#define MCCI_OK			0
#define MCCI_FAIL		1
#define MCCI_OUTOFMEMORY	2
#define MCCI_REQUEST_FAIL	3
#define MCCI_NETWORK_ERROR	4

/* CCI interface tokens used with API */
#define MCCI_DEFAULT		0
#define MCCI_OUTPUT_NONE	100
#define MCCI_OUTPUT_CURRENT	101
#define MCCI_OUTPUT_NEW		102
#define MCCI_ABSOLUTE		110
#define MCCI_RELATIVE		111
#define MCCI_PUBLIC_ANNOTATION	120
#define MCCI_GROUP_ANNOTATION	121
#define MCCI_PRIVATE_ANNOTATION	122
#define MCCI_ALL_ANNOTATION	123

#define MCCI_SEND_BEFORE	210
#define MCCI_SEND_AFTER		220
#define MCCI_SEND_HANDLER       230     /* ADC ZZZ */


/* Protocol token strings */
#define MCCI_S_GET		"GET"
#define MCCI_S_DISPLAY		"DISPLAY"
#define MCCI_S_DISCONNECT	"DISCONNECT"
#define MCCI_S_QUIT		"QUIT"
#define MCCI_S_SEND		"SEND"
#define MCCI_S_OUTPUT		"OUTPUT"
#define MCCI_S_ANCHOR		"ANCHOR" 
#define MCCI_S_BEFORE		"BEFORE"
#define MCCI_S_AFTER		"AFTER"
#define MCCI_S_HANDLER          "HANDLER"   /* ADC ZZZ */
#define MCCI_S_BROWSERVIEW	"BROWSERVIEW"
#define MCCI_S_GETANNOTATION	"GETANNOTATION"
#define MCCI_S_PUTANNOTATION	"PUTANNOTATION"
#define MCCI_S_PUBLIC_ANN	"PUBLIC"
#define MCCI_S_GROUP_ANN	"GROUP"
#define MCCI_S_PRIVATE_ANN	"PRIVATE"
#define MCCI_S_ALL_ANN		"ALL"
#define MCCI_S_FILE_TO_URL	"FILETOURL"
#define MCCI_S_FORM	 	"FORM"
#define MCCI_S_EVENT		"EVENT"
#define MCCI_S_DOCOMMAND	"DOCOMMAND"

#define MCCI_S_TO		"TO"
#define MCCI_S_STOP		"STOP"
#define MCCI_S_START		"START"
#define MCCI_S_CURRENT		"CURRENT"
#define MCCI_S_NEW		"NEW"
#define MCCI_S_NONE		"NONE"
#define MCCI_S_HEADER		"HEADER"
#define MCCI_S_POST		"POST"
#define MCCI_POST             	"cciPOST"

/* For DOCOMMAND options */
#define MCCI_ON			"ON"
#define MCCI_OFF		"OFF"
#define MCCI_UP			"UP"
#define MCCI_DOWN		"DOWN"
#define MCCI_CASE		"CASE"
#define MCCI_NOCASE		"NOCASE"

#define MCCI_UNDERLINE_DEFAULT	"UNDERLINE_DEFAULT"
#define MCCI_UNDERLINE_LIGHT	"UNDERLINE_LIGHT"
#define MCCI_UNDERLINE_MEDIUM	"UNDERLINE_MEDIUM"
#define MCCI_UNDERLINE_HEAVY	"UNDERLINE_HEAVY"
#define MCCI_UNDERLINE_NONE	"UNDERLINE_NONE"

#define MCCI_TIMES_REGULAR	"TIMES_REGULAR"
#define MCCI_TIMES_SMALL	"TIMES_SMALL"
#define MCCI_TIMES_LARGE	"TIMES_LARGE"
#define MCCI_HELVETICA_LARGE	"HELVETICA_LARGE"
#define MCCI_HELVETICA_SMALL	"HELVETICA_SMALL"
#define MCCI_HELVETICA_REGULAR	"HELVETICA_REGULAR"
#define MCCI_NEWCENTURY_LARGE	"NEWCENTURY_LARGE"
#define MCCI_NEWCENTURY_SMALL	"NEWCENTURY_SMALL"
#define MCCI_NEWCENTURY_REGULAR	"NEWCENTURY_REGULAR"
#define MCCI_LUCIDABRIGHT_LARGE	"LUCIDABRIGHT_LARGE"
#define MCCI_LUCIDABRIGHT_SMALL	"LUCIDABRIGHT_SMALL"
#define MCCI_LUCIDABRIGHT_REGULAR "LUCIDABRIGHT_REGULAR"

#define MCCI_PLAINTEXT		"PLAINTEXT"
#define MCCI_FORMATTEDTEXT	"FORMATTEDTEXT"
#define MCCI_POSTSCRIPT		"POSTSCRIPT"
#define MCCI_HTML		"HTML" 

#define MCCI_BACK		"BACK"
#define MCCI_FORWARD		"FORWARD"
#define MCCI_HOME		"HOME"
#define MCCI_RELOAD		"RELOAD"
#define MCCI_SAVEAS		"SAVEAS"
#define MCCI_CLONE		"CLONE"
#define MCCI_NEWWINDOW		"NEWWINDOW"
#define MCCI_CLOSEWINDOW	"CLOSEWINDOW"
#define MCCI_OPENLOCAL		"OPENLOCAL"
#define MCCI_RELOADIMAGES	"RELOADIMAGES"
#define MCCI_REFRESHCURRENT	"REFRESHCURRENT"
#define MCCI_FINDINCURRENT	"FINDINCURRENT"
#define MCCI_VIEWSOURCE		"VIEWSOURCE"
#define MCCI_EDITSOURCE		"EDITSOURCE"
#define MCCI_PRINT		"PRINT"
#define MCCI_CLOSEWINDOW	"CLOSEWINDOW"
#define MCCI_EXITPROGRAM	"EXITPROGRAM"
#define MCCI_FANCYSELECTIONS	"FANCYSELECTIONS"
#define MCCI_LOADTOLOCALDISK	"LOADTOLOCALDISK"
#define MCCI_DELAYIMAGELOAD	"DELAYIMAGELOAD"
#define MCCI_RELOADCONFIG	"RELOADCONFIG"
#define MCCI_FLUSHIMAGECACHE	"FLUSHIMAGECACHE"
#define MCCI_CLEARGLOBALHISTORY	"CLEARGLOBALHISTORY"
#define MCCI_WINDOWHISTORY	"WINDOWHISTORY"
#define MCCI_HOTLIST		"HOTLIST"
#define MCCI_FONT		"FONT"
#define MCCI_ANCHORUNDERLINE	"ANCHORUNDERLINE"


/******************* RETURN CODES ***********************/

/* Successful return codes in protocol */
#define MCCIR_OK			200
#define MCCIR_GET_OK			210
#define MCCIR_DISPLAY_OK		211
#define MCCIR_DISCONNECT_OK		212
#define MCCIR_QUIT_OK			213
#define MCCIR_SEND_OUTPUT_OK		214
#define MCCIR_SEND_O_STOP_OK		215
#define MCCIR_SEND_ANCHOR_OK		216
#define MCCIR_SEND_A_STOP_OK		217
#define MCCIR_POST_OK			218
#define MCCIR_BROWSERVIEW_OK		219
#define MCCIR_BROWSERVIEW_STOP_OK	220
#define MCCIR_ANNOTATION_REQ_OK		221
#define MCCIR_NO_ANNOTATION		222  /* No annotation for url */
#define MCCIR_FILE_TO_URL		223
#define MCCIR_FORM_OK			224
#define MCCIR_SEND_EVENT_OK		225
#define MCCIR_SEND_EVENT_STOP_OK	226
#define MCCIR_DOCOMMAND_OK 		227

/* Send Anchor Before return codes */
#define MCCIR_SEND_ANCH_BEF_LINK_OK	280  /* clicked link  */
#define MCCIR_SEND_ANCH_BEF_OPEN_OK	281  /* used open dialog */
#define MCCIR_SEND_ANCH_BEF_EDIT_OK	282  /* edited URL field  */
#define MCCIR_SEND_ANCH_BEF_FORW_OK	283  /* clicked forward */
#define MCCIR_SEND_ANCH_BEF_BACK_OK	284  /* clicked back */
#define MCCIR_SEND_ANCH_BEF_RELO_OK	285  /* clicked reload */
#define MCCIR_SEND_ANCH_BEF_HIST_OK	286  /* from history  */
#define MCCIR_SEND_ANCH_BEF_HOTL_OK	287  /* from hotlist */
#define MCCIR_SEND_ANCH_BEF_MCCI_OK	288  /* from CCI */
#define MCCIR_SEND_ANCH_BEF_OTHR_OK	289  /* from other source */      

/* Send Anchor After return codes */
#define MCCIR_SEND_ANCH_AFT_LINK_OK	290  /* clicked link  */          
#define MCCIR_SEND_ANCH_AFT_OPEN_OK	291  /* used open dialog */       
#define MCCIR_SEND_ANCH_AFT_EDIT_OK	292  /* edited URL field  */      
#define MCCIR_SEND_ANCH_AFT_FORW_OK	293  /* clicked forward */        
#define MCCIR_SEND_ANCH_AFT_BACK_OK	294  /* clicked back */           
#define MCCIR_SEND_ANCH_AFT_RELO_OK	295  /* clicked reload */         
#define MCCIR_SEND_ANCH_AFT_HIST_OK	296  /* from history  */          
#define MCCIR_SEND_ANCH_AFT_HOTL_OK	297  /* from hotlist */           
#define MCCIR_SEND_ANCH_AFT_MCCI_OK	298  /* from CCI */               
#define MCCIR_SEND_ANCH_AFT_OTHR_OK	299  /* from other source */      

#define MCCIR_SEND_ANCH_HAN_OTHR_OK     270  /*  ADC ZZZ */

/* Additional data follows repsonse code */
#define MCCIR_ANCHOR_INFO	301  /* Anchor visited information */
#define MCCIR_SEND_DATA_OUTPUT	302  /* Output from Send Output protocol */
#define MCCIR_SEND_BROWSERVIEW	303  /* Output from Send Browserview proto */
#define MCCIR_POST_OUTPUT	304  /* Output from post */
#define MCCIR_PUBLIC_ANNOTATION 305  /* Public annotation reply */
#define MCCIR_GROUP_ANNOTATION  306  /* Group annotation reply */
#define MCCIR_PRIVATE_ANNOTATION 307 /* Private annotation  reply */
#define MCCIR_FORM_RESPONSE	308  /* Form submission reply */
#define MCCIR_SEND_EVENT	309  /* Output form send event protocol */
#define MCCIR_SEND_MOUSE_ANCHOR 310  /* Output from Send Mouse Anchor */

/* Problem response codes... client problems */
#define MCCIR_UNRECOGNIZED	401  /* What's this? */
#define MCCIR_ERROR		402  /* Does not follow protocol */

/* Problem response codes... browser problems */
#define MCCIR_REQ_FAILED	500  /* Request failed */
#define MCCIR_GET_FAILED	501  /* Request failed */
#define MCCIR_MAX_CONNECTIONS   502  /* Max number of connections exceeded */
#define MCCIR_NO_URL_FOR_FILE	503  /* Couldn't translate filename to url */
#define MCCIR_DOCOMMAND_FAILED	504  /* Command not implemented yet */

/* All possible events on the Web browser */
typedef enum {
	/* Selected from menu */
	MOSAIC_NEW, MOSAIC_CLONE, MOSAIC_OPEN_URL, FILE_OPEN_LOCAL, 
  	MOSAIC_RELOAD_CURRENT,
	FILE_RELOAD_IMAGES, FILE_REFRESH_CURRENT, FILE_FIND_IN_CURRENT,
	FILE_VIEW_SOURCE, FILE_EDIT_SOURCE, FILE_SOURCE_DATE, MOSAIC_SAVE_AS,
	FILE_PRINT, FILE_MAIL_TO, FILE_CCI, MOSAIC_CLOSE, FILE_EXIT_PROGRAM,

	OPTIONS_FANCY_SELECTIONS_ON, OPTIONS_FANCY_SELECTIONS_OFF, 
	OPTIONS_LOAD_TO_LOCAL_DISK_ON, OPTIONS_LOAD_TO_LOCAL_DISK_OFF,
	OPTIONS_DELAY_IMAGE_LOADING_ON, OPTIONS_DELAY_IMAGE_LOADING_OFF,
	OPTIONS_LOAD_IMAGES_IN_CURRENT, OPTIONS_RELOAD_CONFIG_FILES, 
	OPTIONS_FLUSH_IMAGE_CACHE, OPTIONS_CLEAR_GLOBAL_HISTORY, 
	OPTIONS_FONTS_TR, OPTIONS_FONTS_TS, OPTIONS_FONTS_TL,
	OPTIONS_FONTS_HR, OPTIONS_FONTS_HS, OPTIONS_FONTS_HL, 
	OPTIONS_FONTS_NCR, OPTIONS_FONTS_NCS, OPTIONS_FONTS_NCL,
	OPTIONS_FONTS_LBR, OPTIONS_FONTS_LBS, OPTIONS_FONTS_LBL,
	OPTIONS_ANCHOR_UNDERLINES_DU, OPTIONS_ANCHOR_UNDERLINES_LU,
	OPTIONS_ANCHOR_UNDERLINES_MU, OPTIONS_ANCHOR_UNDERLINES_HU,
	OPTIONS_ANCHOR_UNDERLINES_NU,
	MOSAIC_BACK, MOSAIC_FORWARD, MOSAIC_HOME_DOCUMENT, 
	NAVIGATE_WINDOW_HISTORY, NAVIGATE_HOTLIST, 
	NAVIGATE_ADD_CURRENT_TO_HOTLIST, NAVIGATE_SEARCH,
	NAVIGATE_USENET_SEARCH, 
	NAVIGATE_PEOPLE_SEARCH, 
	NAVIGATE_META_SEARCH, 
	NAVIGATE_LIST_SEARCH, 
	NAVIGATE_MAP_SEARCH, 
	NAVIGATE_AUCTION_SEARCH, 
	NAVIGATE_ENCYCLOPEDIA_SEARCH, 
	NAVIGATE_INTERNET_RESOURCES_META_INDEX,
	ANNOTATE_ANNOTATE, ANNOTATE_AUDIO_ANNOTATE,
	ANNOTATE_EDIT_THIS_ANNOTATION, 
	ANNOTATE_DELETE_THIS_ANNOTATION,
	NEWS_NEXT, NEWS_PREV, NEWS_NEXT_THREAD, NEWS_PREV_THREAD, NEWS_INDEX,
	NEWS_LIST_GROUPS, NEWS_POST, NEWS_FOLLOW_UP, NEWS_FORMAT_TV,
	NEWS_FORMAT_GV,
	/* When user clicks on globe */
	MOSAIC_GLOBE,
	AUTHENTICATION_BUTTON,
	ENCRYPT_BUTTON,

	/* When user submits a form, has to be valid */
	FORM_SUBMIT,

	/* When user edits the url text field in mosaic window and hit return */
	MOSAIC_URL_TEXT_FIELD,

	/* Triggering anchor in HTML widget */
	MOSAIC_URL_TRIGGER,

	HELP_MANUAL, HELP_ABOUT, HELP_ON_WINDOW,
	HELP_ON_VERSION, HELP_ON_FAQ, HELP_ON_HTML, HELP_ON_URLS,
	HELP_MAIL_TECH_SUPPORT, FILE_OPEN_DTM_OUTPORT, FILE_BROADCAST_DOCUMENT,
	FILE_KERBEROS_V4_LOGIN, FILE_KERBEROS_V5_LOGIN, HELP_COMMENT_CARD,
	IMAGE_LOADED, LINK_LOADED, STARTUP,

	/* Events for FTP transfer/commands. */
	FTP_PUT, FTP_REMOVE, FTP_MKDIR,

	OPTIONS_FLUSH_PASSWD_CACHE,
	OPTIONS_BINARY_FTP_MODE_ON, OPTIONS_BINARY_FTP_MODE_OFF,
	HELP_ON_VMS_VERSION, OPEN_COOKIEJAR,
	OPTIONS_VERIFY_CERTS_ON, OPTIONS_VERIFY_CERTS_OFF

} CCI_events;


/*****************************************************************
 * Used to be in port.h
 *****************************************************************/
#define PORTBUFFERSIZE 1024

typedef struct {
	int socketFD;
	char *serverAddress;
	int serverTCPPort;
	int connected;
	int numInBuffer;
	char buffer[PORTBUFFERSIZE * 2 + 2];
	/* callBack(void *callBackData) */
	void (*callBack) ();  /* Called when connection dropped */
	void *callBackData;
} PortDescriptor;
/*****************************************************************/

typedef PortDescriptor *MCCIPort;

#endif
