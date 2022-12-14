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

/* Copyright (C) 1998, 1999, 2000, 2003, 2004, 2005, 2006, 2007
 * The VMS Mosaic Project
 */

/* 
 * Created: Fri Dec 22 21:14:56 CDT 1995
 * Author: Dan Pape
 *
 */

/* This should only be compiled once per source file */

enum pref_item {
    eUSE_PREFERENCES,
    eTRACK_VISITED_ANCHORS, 
    eDISPLAY_URLS_NOT_TITLES, 
    eTRACK_POINTER_MOTION, 
    eTRACK_FULL_URL_NAMES, 
    eANNOTATIONS_ON_TOP, 
    eCONFIRM_DELETE_ANNOTATION, 
    eANNOTATION_SERVER, 
    eRECORD_COMMAND_LOCATION, 
    eRECORD_COMMAND, 
    eRELOAD_PRAGMA_NO_CACHE, 
    eSENDMAIL_COMMAND, 
    eEDIT_COMMAND, 
    eXTERM_COMMAND, 
    eMAIL_FILTER_COMMAND, 
    ePRIVATE_ANNOTATION_DIRECTORY, 
    eHOME_DOCUMENT, 
    eTMP_DIRECTORY, 
    eDOCS_DIRECTORY, 
    eDEFAULT_FONT_CHOICE, 
    eGLOBAL_HISTORY_FILE, 
    eUSE_GLOBAL_HISTORY,
    eHISTORY_FILE,
    eDEFAULT_HOTLIST_FILE, 
    eDEFAULT_HOT_FILE, 
    eADD_HOTLIST_ADDS_RBM,
    eADD_RBM_ADDS_RBM,
    eDOCUMENTS_MENU_SPECFILE, 
    eCOLORS_PER_INLINED_IMAGE, 
    eIMAGE_CACHE_SIZE, 
    eRELOAD_RELOADS_IMAGES, 
    eREVERSE_INLINED_BITMAP_COLORS, 
    eDELAY_IMAGE_LOADS, 
    eSCREEN_GAMMA,
    eDEFAULT_AUTHOR_NAME, 
    eDEFAULT_AUTHOR_EMAIL, 
    eSIGNATURE, 
    eMAIL_MODE, 
    ePRINT_COMMAND, 
    eUNCOMPRESS_COMMAND, 
    eGUNZIP_COMMAND, 
    eUSE_DEFAULT_EXTENSION_MAP, 
    eUSE_DEFAULT_TYPE_MAP, 
    eGLOBAL_EXTENSION_MAP, 
    ePERSONAL_EXTENSION_MAP, 
    eGLOBAL_TYPE_MAP, 
    ePERSONAL_TYPE_MAP, 
    eTWEAK_GOPHER_TYPES,
    eGUI_LAYOUT,
    ePRINT_MODE, 
    ePRINT_BANNERS, 
    ePRINT_FOOTNOTES, 
    ePRINT_PAPER_SIZE_US, 
    ePROXY_SPECFILE, 
    eNOPROXY_SPECFILE, 
    eCCIPORT, 
    eMAX_NUM_OF_CCI_CONNECTIONS, 
    eMAX_WAIS_RESPONSES, 
    eKIOSK, 
    eKIOSKPRINT,
    eKIOSKNOEXIT, 
    eKEEPALIVE, 
    eFTP_TIMEOUT_VAL, 
    eENABLE_TABLES, 
    eDEFAULT_WIDTH, 
    eDEFAULT_HEIGHT, 
    eAUTO_PLACE_WINDOWS, 
    eINITIAL_WINDOW_ICONIC, 
    eTITLEISWINDOWTITLE, 
    eTWIRLING_TRANSFER_ICON, 
    eSECURITYICON, 
    eTWIRL_INCREMENT, 
    eSAVE_MODE, 
    eFULL_HOSTNAME, 
    eLOAD_LOCAL_FILE, 
    eEDIT_COMMAND_USE_XTERM, 
    eCONFIRM_EXIT, 
    eDEFAULT_FANCY_SELECTIONS, 
    eCATCH_PRIOR_AND_NEXT, 
    eSIMPLE_INTERFACE, 
    ePROTECT_ME_FROM_MYSELF, 
    eGETHOSTBYNAME_IS_EVIL, 
#ifdef __sgi
    eDEBUGGING_MALLOC,
#endif
    eUSEAFSKLOG,
    
    eSEND_REFERER,
    eSEND_AGENT,
    eEXPAND_URLS,
    eEXPAND_URLS_WITH_NAME,
    eDEFAULT_PROTOCOL,
    eMETER_FOREGROUND,
    eMETER_BACKGROUND,
    eMETER_FONT_FOREGROUND,
    eMETER_FONT_BACKGROUND,
    eMETER,
    eBACKUP_FILES,
    ePIX_BASENAME,
    ePIX_COUNT,
    eACCEPT_LANGUAGE_STR,

    eFTP_REDIAL,
    eFTP_REDIAL_SLEEP,
    eFTP_FILENAME_LENGTH,
    eFTP_ELLIPSIS_LENGTH,
    eFTP_ELLIPSIS_MODE,
    eTITLE_ISWINDOW_TITLE,
    eUSE_ICONBAR,
    eUSE_TEXTBUTTONBAR,
    eUSE_SCREEN_GAMMA,
    eDISABLEMIDDLEBUTTON,

    eHTTPTRACE,
    eWWW2TRACE,
    eHTMLWTRACE,
    eCCITRACE,
    eSRCTRACE,
    eCACHETRACE,
    eNUTTRACE,
    eTABLETRACE,
    eANIMATEBUSYICON,
    
    eSPLASHSCREEN,
    eINSTALL_COLORMAP,
    eIMAGEVIEWINTERNAL,
    eURLEXPIRED,
    ePOPUPCASCADEMAPPINGDELAY,
    eFRAME_SUPPORT,

    eUSETHREADVIEW,
    eSHOWREADGROUPS,
    eNOTHREADJUMPING,
    eSHOWALLGROUPS,
    eSHOWALLARTICLES,
    eUSEBACKGROUNDFLUSH,
    eBACKGROUNDFLUSHTIME,

    eCLIPPING,
    eMAX_CLIP_TRANSITIONS,

    eUSE_LONG_TEXT_NAMES,
    eTOOLBAR_LAYOUT,

    ePREVISUNREAD,
    eNEXTISUNREAD,
    eUSENEWSRC,
    eNEWSRCPREFIX,
    eNEWSAUTHORWIDTH,
    eNEWSSUBJECTWIDTH,

    eFOCUS_FOLLOWS_MOUSE,

    eSESSION_HISTORY_ON_RBM,
    eNUMBER_OF_ITEMS_IN_RBM_HISTORY,
    eHOTLIST_ON_RBM,
    eUSESHORTNEWSRC,

    /* 2.7b6 */
    eBODYCOLORS,
    eBODYIMAGES,
    eDEFAULTUNDERLINES,
    eFTP_BINARY_MODE,
    eVMS_MAIL_PREFIX,
    eBACKUPFILEVERSIONS,
    eKIOSKPROTOCOLS,

    /* 3.0 */
    eFONTCOLORS,
    ePROGRESSIVE_DISPLAY,
    eFONTSIZES,
    eFONTBASESIZE,
    eTRACK_TARGET_ANCHORS,
    eDEBUG_MENU,
    eREPORTBUGS,
    eIMAGE_ANIMATION,
    eMIN_ANIMATION_DELAY,
    eREFRESHTRACE,

    /* 3.1 */
    eREFRESH_URL,

    /* 3.2 */
    eBROWSER_SAFE_COLORS,

    /* 3.4 */
    eBLINKING_TEXT,
    eBLINK_TIME,
    eCOOKIES,
    eACCEPT_ALL_COOKIES,

    /* 3.5-1 */
    eMAXPIXMAPWIDTH,
    eMAXPIXMAPHEIGHT,

    /* 3.6 */
    eUSE_COOKIE_FILE,
    eCOOKIE_FILE,

    /* 3.6-2 */
    eIMAGEDELAY_FILE,

    /* 3.7 */
    eBROWSERSAFECOLORS_IF_TRUECOLOR,
    eHOTKEYS,
    eINVALID_COOKIE_PROMPT,
    eMAX_COOKIES,
    eCOOKIE_DOMAIN_LIMIT,

    /* 3.8 */
    ePERM_FILE,
    eFORM_BUTTON_BACKGROUND,

    /* 3.9 */
    eVERIFY_SSL_CERTIFICATES,
    eHOTLIST_MENU_HEIGHT,
    eHOTLIST_MENU_WIDTH,
    eMARKUP_MEMORY_PREALLOCATION,
    eELEMENT_MEMORY_PREALLOCATION,
    eCOOKIETRACE,

    /* 4.0 */
    eCLUE_HELP,
    eCLUE_FOREGROUND,
    eCLUE_BACKGROUND,
    eCLUE_POPUP_DELAY,
    eCLUE_POPDOWN_DELAY,
    eCLUE_FONT,
    eCLUE_OVAL,
    eCLUE_ROUNDED,
    ePRINT_DUPLEX,
    eMENUBAR_TEAROFF,

    /* 4.1 */
    eTIFF_ERROR_MESSAGES,
    ePNG_ERROR_MESSAGES,
    eJPEG_ERROR_MESSAGES,

    /* 4.2 */
    eDETACHED_TOOLBAR,
    eDETACHED_TOOLBAR_VERTICAL,
    eTEXT_TOOLBAR,
    eENCRYPTION_ICON,
    ePRESENTATION_MODE_ON_RBM,
    eMULTIPLE_IMAGE_LOAD,
    eMULTIPLE_IMAGE_LIMIT,
    eJPEG2000_ERROR_MESSAGES,

    /* 4.3 */
    eNEWS_AUTHENTICATION_FILE,
    ePRINT_LANDSCAPE,
    ePRINT_DUPLEX_TUMBLED
};
