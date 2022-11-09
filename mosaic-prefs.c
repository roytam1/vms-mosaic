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
 * Created: Wed Sep 20 11:05:19 CDT 1995
 * Modified: All the time.
 * Author: Dan Pape
 *
 */
#include "../config.h"
#include "mosaic.h"
#include "../libnut/str-tools.h"

#ifndef VMS
#include <pwd.h>
#include <sys/utsname.h>
#else
#include "vms_pwd.h"
#include "../libnut/system.h"
#endif

/***********/
/* Defines */
/***********/

#define MAXLINE 512

#ifndef VMS
#define PREFERENCES_FILENAME ".mosaic-preferences"
#else
#define PREFERENCES_FILENAME "mosaic.preferences"
#endif
#define PREFERENCES_MAJOR_VERSION 3
#define PREFERENCES_MINOR_VERSION 0

/***************************/
/* Static Global Variables */
/***************************/

static prefsStructP thePrefsStructP;
static char prefs_file_pathname[512];

/****************************************************************************
 ****************************************************************************
 *            Preference initialization and closing functions
 *
 ****************************************************************************
 ***************************************************************************/

/****************************************************************************
   Function: preferences_genesis(void)
   Desc:     Initializes the preferences manager.
 ***************************************************************************/
Boolean preferences_genesis(void)
{
    /* Initialize preferences structure */
    if (!(thePrefsStructP = (prefsStructP) malloc(sizeof(prefsStruct)))) {
        fprintf(stderr, "Error: no memory for preferences structure\n");
        return False;
    }
    if (!(thePrefsStructP->RdataP = (AppDataPtr) malloc(sizeof(AppData)))) {
        free(thePrefsStructP);
        fprintf(stderr, "Error: no memory for appdata structure\n");
        return False;
    }
    return True;
}

/* Not used */
/****************************************************************************
   Function: preferences_armegeddon(void)
   Desc:     Kills the preferences manager.
 ***************************************************************************/
static Boolean preferences_armegeddon(void)
{
    /* Write the prefs file just to be safe */
    write_preferences_file(thePrefsStructP);

    /* Free preferences structure */
    free(thePrefsStructP);
    
    return True;
}

/****************************************************************************
 ****************************************************************************
 *                    Preference File access functions
 *
 ****************************************************************************
 ***************************************************************************/

/****************************************************************************
   Function: create_prefs_filename(char *fname) 
   Desc:     Generates a full path name for the preferences file
 ***************************************************************************/
static Boolean create_prefs_filename(char *fname)
{
    char *home_ptr;
    char home[256];
#ifndef VMS
    struct passwd *pwdent;
#endif
    
    /*
     * Try the HOME environment variable, then the password file, and
     * finally give up.
     */
    if (!(home_ptr = getenv("HOME"))) {
#ifndef VMS
        if (!(pwdent = getpwuid(getuid()))) {
            return(0);
        } else {
            strcpy(home, pwdent->pw_dir);
        }
#else
	return False;
#endif
    } else {
        strcpy(home, home_ptr);
    }
    
#ifndef VMS
    sprintf(fname, "%s/%s", home, PREFERENCES_FILENAME);
#else
    sprintf(fname, "%s%s", home, PREFERENCES_FILENAME);
#endif
    
    return True;
}

/****************************************************************************
   Function: read_line(char *line)
   Desc:     Get line from preference file 
 ***************************************************************************/
static Boolean read_line(FILE *fp, char *line)
{
    if (!fgets(line, MAXLINE, fp)) {
        fprintf(stderr, "Error: Preference file corrupted.  Premature EOF.\n");
	return False;
    }
    return True;
}

/****************************************************************************
   Function: check_pref(char *file_str, char *read_str)
   Desc:     Check if we are looking at same preference 
 ***************************************************************************/
static Boolean check_pref(char *file_str, char *read_str)
{
    if (my_strncasecmp(file_str, read_str, strlen(file_str) - 1)) {
        fprintf(stderr, "Error: Preference file corrupted\n");
        fprintf(stderr, "       Found preference %s when expecting %s:\n",
		file_str, read_str);
        return False;
    }
    return True;
}

/****************************************************************************
   Function: read_pref_string(FILE *fp, int pref_id, char *string)
   Desc:     
 ***************************************************************************/
static Boolean read_pref_string(FILE *fp, int pref_id, char *string)
{
    char line[MAXLINE], preference[50];
    char *setting;

    if (read_line(fp, line)) {
        sscanf(line, "%49s", preference);
        if (!check_pref(preference, string))
            return False;
    } else {
	return False;
    }
    if (strstr(line, ": ")) {
        setting = strdup(strstr(line, ": ") + 2);
        setting[strlen(setting) - 1] = '\0';	/* Get rid of LF */
    } else {
        setting = strdup("");
    }
    set_pref(pref_id, (void *)setting);

    return True;
}

/****************************************************************************
   Function: read_pref_int(FILE *fp, int pref_id, char *string)
   Desc:     
 ***************************************************************************/
static Boolean read_pref_int(FILE *fp, int pref_id, char *string)
{
    char line[MAXLINE], preference[50];
    int number;

    if (read_line(fp, line)) {
	sscanf(line, "%49s%d", preference, &number);
	if (!check_pref(preference, string))
	    return False;
    } else {
	return False;
    }

    set_pref_int(pref_id, number);

    return True;
}

/****************************************************************************
   Function: read_pref_skip(FILE *fp, char *string)
   Desc:     
 ***************************************************************************/
static Boolean read_pref_skip(FILE *fp, char *string)
{
    char line[MAXLINE], preference[50];

    if (read_line(fp, line)) {
        sscanf(line, "%49s", preference);
        if (!check_pref(preference, string))
            return False;
    } else {
	return False;
    }
    return True;
}

/****************************************************************************
   Function: read_pref_boolean(FILE *fp, int pref_id, char *string)
   Desc:     
 ***************************************************************************/
static Boolean read_pref_boolean(FILE *fp, int pref_id, char *string)
{
    char line[MAXLINE], preference[50];
    char setting[7] = "False";

    if (read_line(fp, line)) {
        sscanf(line, "%49s%6s", preference, setting);
        if (!check_pref(preference, string))
            return False;
    } else {
	return False;
    }
    if (!my_strcasecmp(setting, "True")) {
        set_pref_boolean(pref_id, 1);
    } else {
        set_pref_boolean(pref_id, 0);
    }
    return True;
}

/****************************************************************************
   Function: read_pref_float(FILE *fp, int pref_id, char *string)
   Desc:     
 ***************************************************************************/
static Boolean read_pref_float(FILE *fp, int pref_id, char *string)
{
    char line[MAXLINE], preference[50];
    float number;

    if (read_line(fp, line)) {
        sscanf(line, "%49s%f", preference, &number);
        if (!check_pref(preference, string))
            return False;
    } else {
	return False;
    }
    set_pref_float(pref_id, number);
    
    return True;
}

/****************************************************************************
   Function: read_preferences_file(prefsStructP inPrefsStruct)
   Desc:     Read the prefs file into the incoming prefs struct.          
 ***************************************************************************/
Boolean read_preferences_file(prefsStructP inPrefsStruct)
{
    FILE *fp;
    char line[MAXLINE];
    char version[5];
    char cp;
    Boolean status = True;
    Boolean good = False;

    /* If the incoming pointer is NULL, then we use the main structure */
    if (!inPrefsStruct)
        inPrefsStruct = thePrefsStructP;
    
    /* Look for the file */    
    if (!create_prefs_filename(prefs_file_pathname)) {
        fprintf(stderr, "Error: Can't generate pathname for preference file\n");
        return False;
    }

    /* Check to see if the file exists.  If it doesn't, then create it */
    if (!file_exists(prefs_file_pathname) && !write_preferences_file(NULL)) {
        fprintf(stderr, "Error: Can't find or create preferences file\n");
        return False;
    }
    /* Open it and read all the stuff from the file into the prefs struct */
    if (!(fp = fopen(prefs_file_pathname, "r"))) {
        fprintf(stderr, "Error: Can't open preferences file for reading\n");
        return False;
    }
    
    /* But first, check the version number of the prefs file */
    if (!read_line(fp, line) || !read_line(fp, line))
	return False;
    sscanf(line, "%*s%*s%*s%4s", version);

    if (version[1] == '.') {
	cp = version[2];
        switch (version[0]) {
	    case '1':
		if ((cp >= '0') && (cp <= '8'))
		    good = True;
		break;
	    case '2':
		if ((cp >= '0') && (cp <= '9'))
		    good = True;
		break;
	    case '3':
		if ((cp >= '0') && (cp <= '9'))
		    good = True;
		break;
	    default: ;
	}
    }
    if (!good) {
         fprintf(stderr, "Error: Bad version in preferences file\n");
         fprintf(stderr, "       Skipping it and using defaults instead\n");
         fclose(fp);
         return True;
    }

    if (!read_line(fp, line) || !read_line(fp, line) || !read_line(fp, line))
	return False;

    if (!read_pref_boolean(fp, eUSE_PREFERENCES, "USE_PREFERENCES")) {
        fclose(fp);
        return False;
    }
    if (!get_pref_boolean(eUSE_PREFERENCES)) {
        fclose(fp);
        return True;
    }

    status = read_pref_boolean(fp,
	eTRACK_VISITED_ANCHORS, "TRACK_VISITED_ANCHORS") ? status : 0;
    status = read_pref_boolean(fp,
	eDISPLAY_URLS_NOT_TITLES, "DISPLAY_URLS_NOT_TITLES") ? status : 0;
    status = read_pref_boolean(fp,
	eTRACK_POINTER_MOTION, "TRACK_POINTER_MOTION") ? status : 0;
    status = read_pref_boolean(fp,
	eTRACK_FULL_URL_NAMES, "TRACK_FULL_URL_NAMES") ? status : 0;
    status = read_pref_boolean(fp,
	eANNOTATIONS_ON_TOP, "ANNOTATIONS_ON_TOP") ? status : 0;
    status = read_pref_boolean(fp,
	eCONFIRM_DELETE_ANNOTATION, "CONFIRM_DELETE_ANNOTATION") ? status : 0;
    status = read_pref_string(fp,
	eANNOTATION_SERVER, "ANNOTATION_SERVER") ? status : 0;
    status = read_pref_string(fp,
	eRECORD_COMMAND_LOCATION, "RECORD_COMMAND_LOCATION") ? status : 0;
    status = read_pref_string(fp,
	eRECORD_COMMAND, "RECORD_COMMAND") ? status : 0;
    status = read_pref_boolean(fp,
	eRELOAD_PRAGMA_NO_CACHE, "RELOAD_PRAGMA_NO_CACHE") ? status : 0;
    status = read_pref_string(fp,
	eSENDMAIL_COMMAND, "SENDMAIL_COMMAND") ? status : 0;
    status = read_pref_string(fp,
	eEDIT_COMMAND, "EDIT_COMMAND") ? status : 0;
    status = read_pref_string(fp,
	eXTERM_COMMAND, "XTERM_COMMAND") ? status : 0;
    status = read_pref_string(fp,
	eMAIL_FILTER_COMMAND, "MAIL_FILTER_COMMAND") ? status : 0;
    status = read_pref_string(fp,
	ePRIVATE_ANNOTATION_DIRECTORY, "PRIVATE_ANNOTATION_DIRECTORY") ?
	status : 0;
    status = read_pref_string(fp,
	eHOME_DOCUMENT, "HOME_DOCUMENT") ? status : 0;
    status = read_pref_string(fp,
	eTMP_DIRECTORY, "TMP_DIRECTORY") ? status : 0;
    status = read_pref_string(fp,
	eDOCS_DIRECTORY, "DOCS_DIRECTORY") ? status : 0;
    status = read_pref_string(fp,
	eDEFAULT_FONT_CHOICE, "DEFAULT_FONT_CHOICE") ? status : 0;
    status = read_pref_string(fp,
	eGLOBAL_HISTORY_FILE, "GLOBAL_HISTORY_FILE") ? status : 0;
    status = read_pref_boolean(fp,
	eUSE_GLOBAL_HISTORY, "USE_GLOBAL_HISTORY") ? status : 0;
    status = read_pref_string(fp,
	eHISTORY_FILE, "HISTORY_FILE") ? status : 0;
    status = read_pref_string(fp,
	eDEFAULT_HOT_FILE, "DEFAULT_HOT_FILE") ? status : 0;
    status = read_pref_boolean(fp,
	eADD_HOTLIST_ADDS_RBM, "ADD_HOTLIST_ADDS_RBM") ? status : 0;
    status = read_pref_boolean(fp,
	eADD_RBM_ADDS_RBM, "ADD_RBM_ADDS_RBM") ? status : 0;
    status = read_pref_string(fp,
	eDOCUMENTS_MENU_SPECFILE, "DOCUMENTS_MENU_SPECFILE") ? status : 0;
    status = read_pref_int(fp,
	eCOLORS_PER_INLINED_IMAGE, "COLORS_PER_INLINED_IMAGE") ? status : 0;
    status = read_pref_int(fp,
	eIMAGE_CACHE_SIZE, "IMAGE_CACHE_SIZE") ? status : 0;
    status = read_pref_boolean(fp,
	eRELOAD_RELOADS_IMAGES, "RELOAD_RELOADS_IMAGES") ? status : 0;
    status = read_pref_boolean(fp,
	eREVERSE_INLINED_BITMAP_COLORS, "REVERSE_INLINED_BITMAP_COLORS") ?
	status : 0;
    status = read_pref_boolean(fp,
	eDELAY_IMAGE_LOADS, "DELAY_IMAGE_LOADS") ? status : 0;
    status = read_pref_float(fp,
	eSCREEN_GAMMA, "SCREEN_GAMMA") ? status : 0;
    status = read_pref_string(fp,
	eDEFAULT_AUTHOR_NAME, "DEFAULT_AUTHOR_NAME") ? status : 0;
    status = read_pref_string(fp,
	eDEFAULT_AUTHOR_EMAIL, "DEFAULT_AUTHOR_EMAIL") ? status : 0;
    status = read_pref_string(fp,
	eSIGNATURE, "SIGNATURE") ? status : 0;
    status = read_pref_string(fp,
	eMAIL_MODE, "MAIL_MODE") ? status : 0;
    status = read_pref_string(fp,
	ePRINT_COMMAND, "PRINT_COMMAND") ? status : 0;
    status = read_pref_string(fp,
	eUNCOMPRESS_COMMAND, "UNCOMPRESS_COMMAND") ? status : 0;
    status = read_pref_string(fp,
	eGUNZIP_COMMAND, "GUNZIP_COMMAND") ? status : 0;
    status = read_pref_boolean(fp,
	eUSE_DEFAULT_EXTENSION_MAP, "USE_DEFAULT_EXTENSION_MAP") ? status : 0;
    status = read_pref_boolean(fp,
	eUSE_DEFAULT_TYPE_MAP, "USE_DEFAULT_TYPE_MAP") ? status : 0;
    status = read_pref_string(fp,
	eGLOBAL_EXTENSION_MAP, "GLOBAL_EXTENSION_MAP") ? status : 0;
    status = read_pref_string(fp,
	ePERSONAL_EXTENSION_MAP, "PERSONAL_EXTENSION_MAP") ? status : 0;
    status = read_pref_string(fp,
	eGLOBAL_TYPE_MAP, "GLOBAL_TYPE_MAP") ? status : 0;
    status = read_pref_string(fp,
	ePERSONAL_TYPE_MAP, "PERSONAL_TYPE_MAP") ? status : 0;
    status = read_pref_boolean(fp,
	eTWEAK_GOPHER_TYPES, "TWEAK_GOPHER_TYPES") ? status : 0;
    status = read_pref_string(fp,
	ePRINT_MODE, "PRINT_MODE") ? status : 0;
    status = read_pref_string(fp,
	eGUI_LAYOUT, "GUI_LAYOUT") ? status : 0;
    status = read_pref_boolean(fp,
	ePRINT_BANNERS, "PRINT_BANNERS") ? status : 0;
    status = read_pref_boolean(fp,
	ePRINT_FOOTNOTES, "PRINT_FOOTNOTES") ? status : 0;
    status = read_pref_boolean(fp,
	ePRINT_PAPER_SIZE_US, "PRINT_PAPER_SIZE_US") ? status : 0;
    status = read_pref_string(fp,
	ePROXY_SPECFILE, "PROXY_SPECFILE") ? status : 0;
    status = read_pref_string(fp,
	eNOPROXY_SPECFILE, "NOPROXY_SPECFILE") ? status : 0;
    status = read_pref_int(fp,
	eCCIPORT, "CCIPORT") ? status : 0;
    status = read_pref_int(fp,
	eMAX_NUM_OF_CCI_CONNECTIONS, "MAX_NUM_OF_CCI_CONNECTIONS") ? status : 0;
    status = read_pref_int(fp,
	eMAX_WAIS_RESPONSES, "MAX_WAIS_RESPONSES") ? status : 0;
    status = read_pref_boolean(fp,
	eKIOSK, "KIOSK") ? status : 0;
    status = read_pref_boolean(fp,
	eKIOSKPRINT, "KIOSKPRINT") ? status : 0;
    status = read_pref_boolean(fp,
	eKIOSKNOEXIT, "KIOSKNOEXIT") ? status : 0;
    status = read_pref_boolean(fp,
	eKEEPALIVE, "KEEPALIVE") ? status : 0;
    status = read_pref_int(fp,
	eFTP_TIMEOUT_VAL, "FTP_TIMEOUT_VAL") ? status : 0;
    status = read_pref_boolean(fp,
	eENABLE_TABLES, "ENABLE_TABLES") ? status : 0;
    status = read_pref_int(fp,
	eDEFAULT_WIDTH, "DEFAULT_WIDTH") ? status : 0;
    status = read_pref_int(fp,
	eDEFAULT_HEIGHT, "DEFAULT_HEIGHT") ? status : 0;
    status = read_pref_boolean(fp,
	eAUTO_PLACE_WINDOWS, "AUTO_PLACE_WINDOWS") ? status : 0;
    status = read_pref_boolean(fp,
	eINITIAL_WINDOW_ICONIC, "INITIAL_WINDOW_ICONIC") ? status : 0;
    status = read_pref_boolean(fp,
	eTITLEISWINDOWTITLE, "TITLEISWINDOWTITLE") ? status : 0;

    if (strcmp(version, "2.9")) {
        status = read_pref_skip(fp,"USEICONBAR") ? status : 0;
        status = read_pref_skip(fp, "USETEXTBUTTONBAR") ? status : 0;
    }
    status = read_pref_boolean(fp,
	eTWIRLING_TRANSFER_ICON, "TWIRLING_TRANSFER_ICON") ? status : 0;
    status = read_pref_boolean(fp,
	eSECURITYICON, "SECURITYICON") ? status : 0;
    status = read_pref_int(fp,
	eTWIRL_INCREMENT, "TWIRL_INCREMENT") ? status : 0;
    status = read_pref_string(fp,
	eSAVE_MODE, "SAVE_MODE") ? status : 0;

    if (strcmp(version, "2.0") < 0) {
	status = read_pref_skip(fp, "HDF_MAX_IMAGE_DIMENSION") ? status : 0;
	status = read_pref_skip(fp, "HDF_MAX_DISPLAYED_DATASETS") ? status : 0;
	status = read_pref_skip(fp, "HDF_MAX_DISPLAYED_ATTRIBUTES") ? status :0;
	status = read_pref_skip(fp, "HDF_POWER_USER") ? status : 0;
	status = read_pref_skip(fp, "HDFLONGNAME") ? status : 0;
    }
    status = read_pref_string(fp,
	eFULL_HOSTNAME, "FULL_HOSTNAME") ? status : 0;
    status = read_pref_int(fp,
	eLOAD_LOCAL_FILE, "LOAD_LOCAL_FILE") ? status : 0;
    status = read_pref_boolean(fp,
	eEDIT_COMMAND_USE_XTERM, "EDIT_COMMAND_USE_XTERM") ? status : 0;
    status = read_pref_boolean(fp,
	eCONFIRM_EXIT, "CONFIRM_EXIT") ? status : 0;
    status = read_pref_boolean(fp,
	eDEFAULT_FANCY_SELECTIONS, "DEFAULT_FANCY_SELECTIONS") ? status : 0;
    status = read_pref_boolean(fp,
	eCATCH_PRIOR_AND_NEXT, "CATCH_PRIOR_AND_NEXT") ? status : 0;
    status = read_pref_boolean(fp,
	eSIMPLE_INTERFACE, "SIMPLE_INTERFACE") ? status : 0;
    status = read_pref_boolean(fp,
	ePROTECT_ME_FROM_MYSELF, "PROTECT_ME_FROM_MYSELF") ? status : 0;
    status = read_pref_boolean(fp,
	eGETHOSTBYNAME_IS_EVIL, "GETHOSTBYNAME_IS_EVIL") ? status : 0;
#ifdef __sgi
    status = read_pref_boolean(fp,
	eDEBUGGING_MALLOC, "DEBUGGING_MALLOC") ? status : 0;
#endif
    status = read_pref_boolean(fp,
	eUSEAFSKLOG, "USEAFSKLOG") ? status : 0;
    status = read_pref_boolean(fp,
	eSEND_REFERER, "SEND_REFERER") ? status : 0;
    status = read_pref_boolean(fp,
	eSEND_AGENT, "SEND_AGENT") ? status : 0;
    status = read_pref_boolean(fp,
	eEXPAND_URLS, "EXPAND_URLS") ? status : 0;
    status = read_pref_boolean(fp,
	eEXPAND_URLS_WITH_NAME, "EXPAND_URLS_WITH_NAME") ? status : 0;
    status = read_pref_string(fp,
	eDEFAULT_PROTOCOL, "DEFAULT_PROTOCOL") ? status : 0;
    status = read_pref_string(fp,
	eMETER_FOREGROUND, "METER_FOREGROUND") ? status : 0;
    status = read_pref_string(fp,
	eMETER_BACKGROUND, "METER_BACKGROUND") ? status : 0;
    status = read_pref_string(fp,
	eMETER_FONT_FOREGROUND, "METER_FONT_FOREGROUND") ? status : 0;
    status = read_pref_string(fp,
	eMETER_FONT_BACKGROUND, "METER_FONT_BACKGROUND") ? status : 0;
    status = read_pref_boolean(fp,
	eMETER, "METER") ? status : 0;
    status = read_pref_boolean(fp,
	eBACKUP_FILES, "BACKUP_FILES") ? status : 0;
    status = read_pref_string(fp,
	ePIX_BASENAME, "PIX_BASENAME") ? status : 0;
    status = read_pref_int(fp,
	ePIX_COUNT, "PIX_COUNT") ? status : 0;
    status = read_pref_string(fp,
	eACCEPT_LANGUAGE_STR, "ACCEPT_LANGUAGE_STR") ? status : 0;
    status = read_pref_int(fp,
	eFTP_REDIAL, "FTP_REDIAL") ? status : 0;
    status = read_pref_int(fp,
	eFTP_REDIAL_SLEEP, "FTP_REDIAL_SLEEP") ? status : 0;
    status = read_pref_int(fp,
	eFTP_FILENAME_LENGTH, "FTP_FILENAME_LENGTH") ? status : 0;
    status = read_pref_int(fp,
	eFTP_ELLIPSIS_LENGTH, "FTP_ELLIPSIS_LENGTH") ? status : 0;
    status = read_pref_int(fp,
	eFTP_ELLIPSIS_MODE, "FTP_ELLIPSIS_MODE") ? status : 0;
    status = read_pref_boolean(fp,
	eTITLE_ISWINDOW_TITLE, "TITLE_ISWINDOW_TITLE") ? status : 0;
    status = read_pref_boolean(fp,
	eUSE_SCREEN_GAMMA, "USE_SCREEN_GAMMA") ? status : 0;
    status = read_pref_boolean(fp,
	eDISABLEMIDDLEBUTTON, "DISABLEMIDDLEBUTTON") ? status : 0;
    status = read_pref_boolean(fp,
	eHTTPTRACE, "HTTPTRACE") ? status : 0;
    status = read_pref_boolean(fp,
	eWWW2TRACE, "WWW2TRACE") ? status : 0;
    status = read_pref_boolean(fp,
	eHTMLWTRACE, "HTMLWTRACE") ? status : 0;
    status = read_pref_boolean(fp,
	eCCITRACE, "CCITRACE") ? status : 0;
    status = read_pref_boolean(fp,
	eSRCTRACE, "SRCTRACE") ? status : 0;
    status = read_pref_boolean(fp,
	eCACHETRACE, "CACHETRACE") ? status : 0;
    status = read_pref_boolean(fp,
	eNUTTRACE, "NUTTRACE") ? status : 0;

    if (strcmp(version, "1.0") != 0)
	    status = read_pref_boolean(fp,
		eTABLETRACE, "TABLETRACE") ? status : 0;

    status = read_pref_boolean(fp,
	eANIMATEBUSYICON, "ANIMATEBUSYICON") ? status : 0;
    status = read_pref_boolean(fp,
	eSPLASHSCREEN, "SPLASHSCREEN") ? status : 0;
    status = read_pref_boolean(fp,
	eINSTALL_COLORMAP, "INSTALL_COLORMAP") ? status : 0;
    status = read_pref_boolean(fp,
	eIMAGEVIEWINTERNAL, "IMAGEVIEWINTERNAL") ? status : 0;
    status = read_pref_int(fp,
	eURLEXPIRED, "URLEXPIRED") ? status : 0;

    if (strcmp(version, "2.0") < 0)
	status = read_pref_skip(fp, "RBM_CASCADE_OFFSET") ? status : 0;

    status = read_pref_int(fp,
	ePOPUPCASCADEMAPPINGDELAY, "POPUPCASCADEMAPPINGDELAY") ? status : 0;

    if (strcmp(version, "2.0") < 0) {
	status = read_pref_skip(fp, "FRAME_HACK") ? status : 0;
	set_pref_boolean(eFRAME_SUPPORT, 1);
    } else {
	status = read_pref_boolean(fp,
	    eFRAME_SUPPORT, "FRAME_SUPPORT") ? status : 0;
    }
    status = read_pref_boolean(fp,
	eCLIPPING, "CLIPPING") ? status : 0;
    status = read_pref_int(fp,
	eMAX_CLIP_TRANSITIONS, "MAX_CLIP_TRANSITIONS") ? status : 0;
    status = read_pref_boolean(fp,
	eUSE_LONG_TEXT_NAMES, "USE_LONG_TEXT_NAMES") ? status : 0;
    status = read_pref_string(fp,
	eTOOLBAR_LAYOUT, "TOOLBAR_LAYOUT") ? status : 0;

    status = read_pref_boolean(fp,
	eUSETHREADVIEW, "USETHREADVIEW") ? status : 0;
    status = read_pref_boolean(fp,
	eSHOWREADGROUPS, "SHOWREADGROUPS") ? status : 0;
    status = read_pref_boolean(fp,
	eNOTHREADJUMPING, "NOTHREADJUMPING") ? status : 0;
    status = read_pref_boolean(fp,
	eSHOWALLGROUPS, "SHOWALLGROUPS") ? status : 0;
    status = read_pref_boolean(fp,
	eSHOWALLARTICLES, "SHOWALLARTICLES") ? status : 0;
    status = read_pref_boolean(fp,
	eUSEBACKGROUNDFLUSH, "USEBACKGROUNDFLUSH") ? status : 0;
    status = read_pref_int(fp,
	eBACKGROUNDFLUSHTIME, "BACKGROUNDFLUSHTIME") ? status : 0;
    status = read_pref_boolean(fp,
	ePREVISUNREAD, "PREVISPREVUNREAD") ? status : 0;
    status = read_pref_boolean(fp,
	eNEXTISUNREAD, "NEXTISNEXTUNREAD") ? status : 0;
    status = read_pref_boolean(fp,
	eUSENEWSRC, "USENEWSRC") ? status : 0;
    status = read_pref_string(fp,
	eNEWSRCPREFIX, "NEWSRCPREFIX") ? status : 0;
    status = read_pref_int(fp,
	eNEWSAUTHORWIDTH, "NEWSAUTHORWIDTH") ? status : 0;
    status = read_pref_int(fp,
	eNEWSSUBJECTWIDTH, "NEWSSUBJECTWIDTH") ? status : 0;

    status = read_pref_boolean(fp,
	eFOCUS_FOLLOWS_MOUSE, "FOCUS_FOLLOWS_MOUSE") ? status : 0;
    status = read_pref_boolean(fp,
	eSESSION_HISTORY_ON_RBM, "SESSION_HISTORY_ON_RBM") ? status : 0;
    status = read_pref_int(fp,
	eNUMBER_OF_ITEMS_IN_RBM_HISTORY, "NUMBER_OF_ITEMS_IN_RBM_HISTORY") ?
	status : 0;

    status = read_pref_boolean(fp,
	eUSESHORTNEWSRC, "USESHORTNEWSRC") ? status : 0;
    status = read_pref_boolean(fp,
	eHOTLIST_ON_RBM, "HOTLIST_ON_RBM") ? status : 0;
    status = read_pref_boolean(fp,
	eBODYCOLORS, "BODYCOLORS") ? status : 0;
    status = read_pref_boolean(fp,
	eBODYIMAGES, "BODYIMAGES") ? status : 0;
    status = read_pref_string(fp,
	eDEFAULTUNDERLINES, "DEFAULTUNDERLINES") ? status : 0;
    status = read_pref_boolean(fp,
	eFTP_BINARY_MODE, "FTP_BINARY_MODE") ? status : 0;
    status = read_pref_string(fp,
	eKIOSKPROTOCOLS, "KIOSKPROTOCOLS") ? status : 0;
    status = read_pref_string(fp,
	eVMS_MAIL_PREFIX, "VMS_MAIL_PREFIX") ? status : 0;
    status = read_pref_int(fp,
	eBACKUPFILEVERSIONS, "BACKUPFILEVERSIONS") ? status : 0;

    if (strcmp(version, "1.1") > 0) {
	    status = read_pref_boolean(fp,
		eFONTCOLORS, "FONTCOLORS") ? status : 0;
	    status = read_pref_boolean(fp,
		ePROGRESSIVE_DISPLAY, "PROGRESSIVE_DISPLAY") ? status : 0;
    }
    if (strcmp(version, "1.2") > 0) {
	    status = read_pref_boolean(fp,
		eFONTSIZES, "FONTSIZES") ? status : 0;
	    status = read_pref_int(fp,
		eFONTBASESIZE, "FONTBASESIZE") ? status : 0;
    }
    if (strcmp(version, "1.3") > 0) {
	    status = read_pref_boolean(fp,
		eTRACK_TARGET_ANCHORS, "TRACK_TARGET_ANCHORS") ? status : 0;
	    status = read_pref_boolean(fp,
		eDEBUG_MENU, "DEBUG_MENU") ? status : 0;
	    status = read_pref_boolean(fp,
		eREPORTBUGS, "REPORTBUGS") ? status : 0;
    }
    if (strcmp(version, "1.4") > 0) {
	    status = read_pref_boolean(fp,
		eIMAGE_ANIMATION, "IMAGE_ANIMATION") ? status : 0;
	    status = read_pref_int(fp,
		eMIN_ANIMATION_DELAY, "MIN_ANIMATION_DELAY") ? status : 0;
    }
    if (strcmp(version, "1.5") > 0) {
	    status = read_pref_boolean(fp,
		eREFRESHTRACE, "REFRESHTRACE") ? status : 0;
    }
    if (strcmp(version, "1.6") > 0) {
	    status = read_pref_boolean(fp,
		 eREFRESH_URL, "REFRESH_URL") ? status : 0;
    }
    if (strcmp(version, "1.7") > 0) {
	    status = read_pref_boolean(fp,
		 eBROWSER_SAFE_COLORS, "BROWSER_SAFE_COLORS") ? status : 0;
    }
    if (strcmp(version, "1.9") > 0) {
	    status = read_pref_boolean(fp,
		 eBLINKING_TEXT, "BLINKING_TEXT") ? status : 0;
	    status = read_pref_int(fp,
		 eBLINK_TIME, "BLINK_TIME") ? status : 0;
	    status = read_pref_boolean(fp,
		 eCOOKIES, "COOKIES") ? status : 0;
	    status = read_pref_boolean(fp,
		 eACCEPT_ALL_COOKIES, "ACCEPT_ALL_COOKIES") ? status : 0;
    }
    if (strcmp(version, "2.0") > 0) {
	    status = read_pref_int(fp,
		 eMAXPIXMAPWIDTH, "MAXPIXMAPWIDTH") ? status : 0;
	    status = read_pref_int(fp,
		 eMAXPIXMAPHEIGHT, "MAXPIXMAPHEIGHT") ? status : 0;
    }
    if (strcmp(version, "2.1") > 0) {
	    status = read_pref_boolean(fp,
		 eUSE_COOKIE_FILE, "USE_COOKIE_FILE") ? status : 0;
	    status = read_pref_string(fp,
		 eCOOKIE_FILE, "COOKIE_FILE") ? status : 0;
    }
    if (strcmp(version, "2.2") > 0) {
	    status = read_pref_string(fp,
		 eIMAGEDELAY_FILE, "IMAGEDELAY_FILE") ? status : 0;
    }
    if (strcmp(version, "2.3") > 0) {
	    status = read_pref_boolean(fp,
		 eBROWSERSAFECOLORS_IF_TRUECOLOR,
		 "BROWSERSAFECOLORS_IF_TRUECOLOR") ? status : 0;
	    status = read_pref_boolean(fp,
		 eHOTKEYS, "HOTKEYS") ? status : 0;
	    status = read_pref_boolean(fp,
		 eINVALID_COOKIE_PROMPT, "INVALID_COOKIE_PROMPT") ? status : 0;
	    status = read_pref_int(fp,
		 eMAX_COOKIES, "MAX_COOKIES") ? status : 0;
	    status = read_pref_int(fp,
		 eCOOKIE_DOMAIN_LIMIT, "COOKIE_DOMAIN_LIMIT") ? status : 0;
    }
    if (strcmp(version, "2.4") > 0) {
	    status = read_pref_string(fp,
		 ePERM_FILE, "PERM_FILE") ? status : 0;
	    status = read_pref_string(fp,
		 eFORM_BUTTON_BACKGROUND, "FORM_BUTTON_BACKGROUND") ? status :0;
    }
    if (strcmp(version, "2.5") > 0) {
	    status = read_pref_boolean(fp,
		 eVERIFY_SSL_CERTIFICATES, "VERIFY_SSL_CERTIFICATES") ?
		 status : 0;
	    status = read_pref_int(fp,
		 eHOTLIST_MENU_HEIGHT, "HOTLIST_MENU_HEIGHT") ? status : 0;
	    status = read_pref_int(fp,
		 eHOTLIST_MENU_WIDTH, "HOTLIST_MENU_WIDTH") ? status : 0;
	    status = read_pref_int(fp,
		 eMARKUP_MEMORY_PREALLOCATION, "MARKUP_MEMORY_PREALLOCATION") ?
		 status : 0;
	    status = read_pref_int(fp,
		 eELEMENT_MEMORY_PREALLOCATION,
		 "ELEMENT_MEMORY_PREALLOCATION") ? status : 0;
	    status = read_pref_boolean(fp,
		 eCOOKIETRACE, "COOKIETRACE") ? status : 0;
    }
    if (strcmp(version, "2.6") > 0) {
	    status = read_pref_boolean(fp,
		 eCLUE_HELP, "CLUE_HELP") ? status : 0;
	    status = read_pref_string(fp,
		 eCLUE_FOREGROUND, "CLUE_FOREGROUND") ? status :0;
	    status = read_pref_string(fp,
		 eCLUE_BACKGROUND, "CLUE_BACKGROUND") ? status :0;
	    status = read_pref_int(fp,
		 eCLUE_POPUP_DELAY, "CLUE_POPUP_DELAY") ? status : 0;
	    status = read_pref_int(fp,
		 eCLUE_POPDOWN_DELAY, "CLUE_POPDOWN_DELAY") ? status : 0;
	    status = read_pref_string(fp,
		 eCLUE_FONT, "CLUE_FONT") ? status :0;
	    status = read_pref_boolean(fp,
		 eCLUE_OVAL, "CLUE_OVAL") ? status : 0;
	    status = read_pref_boolean(fp,
		 eCLUE_ROUNDED, "CLUE_ROUNDED") ? status : 0;
	    status = read_pref_boolean(fp,
		 ePRINT_DUPLEX, "PRINT_DUPLEX") ? status : 0;
	    status = read_pref_boolean(fp,
		 eMENUBAR_TEAROFF, "MENUBAR_TEAROFF") ? status : 0;
    }
    if (strcmp(version, "2.7") > 0) {
	    status = read_pref_boolean(fp,
		 eTIFF_ERROR_MESSAGES, "TIFF_ERROR_MESSAGES") ? status : 0;
	    status = read_pref_boolean(fp,
		 ePNG_ERROR_MESSAGES, "PNG_ERROR_MESSAGES") ? status : 0;
	    status = read_pref_boolean(fp,
		 eJPEG_ERROR_MESSAGES, "JPEG_ERROR_MESSAGES") ? status : 0;
    }
    if (strcmp(version, "2.8") > 0) {
	    status = read_pref_boolean(fp,
		 eDETACHED_TOOLBAR, "DETACHED_TOOLBAR") ? status : 0;
	    status = read_pref_boolean(fp,
		 eDETACHED_TOOLBAR_VERTICAL, "DETACHED_TOOLBAR_VERTICAL") ?
		 status : 0;
	    status = read_pref_boolean(fp,
		 eTEXT_TOOLBAR, "TEXT_TOOLBAR") ? status : 0;
	    status = read_pref_boolean(fp,
		 eENCRYPTION_ICON, "ENCRYPTION_ICON") ? status : 0;
	    status = read_pref_boolean(fp,
		 ePRESENTATION_MODE_ON_RBM, "PRESENTATION_MODE_ON_RBM") ?
		 status : 0;
	    status = read_pref_boolean(fp,
		 eMULTIPLE_IMAGE_LOAD, "MULTIPLE_IMAGE_LOAD") ? status : 0;
	    status = read_pref_int(fp,
		 eMULTIPLE_IMAGE_LIMIT, "MULTIPLE_IMAGE_LIMIT") ? status : 0;
	    status = read_pref_boolean(fp,
		 eJPEG2000_ERROR_MESSAGES, "JPEG2000_ERROR_MESSAGES") ?
		 status : 0;
    }
    fclose(fp);

    return status;
}


/****************************************************************************
   Function: write_pref_string(FILE *fp, int pref_id, char *string)
   Desc:     
 ***************************************************************************/
static Boolean write_pref_string(FILE *fp, int pref_id, char *string)
{
    char *narf = get_pref_string(pref_id);

    if (!narf) {
        fprintf(fp, "%s:\n", string);
    } else {
        fprintf(fp, "%s: %s\n", string, narf);
    }
    return True;
}

/****************************************************************************
   Function: write_pref_int(FILE *fp, int pref_id, char *string)
   Desc:     
 ***************************************************************************/
static Boolean write_pref_int(FILE *fp, int pref_id, char *string)
{
    fprintf(fp, "%s: %d\n", string, get_pref_int(pref_id));

    return True;
}

/****************************************************************************
   Function: write_pref_boolean(FILE *fp, int pref_id, char *string)
   Desc:     
 ***************************************************************************/
static Boolean write_pref_boolean(FILE *fp, int pref_id, char *string)
{
    if (get_pref_boolean(pref_id)) {
        fprintf(fp, "%s: True\n", string);
    } else {
        fprintf(fp, "%s: False\n", string);
    }
    return True;
}

/****************************************************************************
   Function: write_pref_float(FILE *fp, int pref_id, char *string)
   Desc:     
 ***************************************************************************/
static Boolean write_pref_float(FILE *fp, int pref_id, char *string)
{
    fprintf(fp, "%s: %f\n", string, get_pref_float(pref_id));

    return True;
}


/****************************************************************************
   Function: write_preferences_file(void)
   Desc:     Writes the incoming preferences structure to disk. If the
             incoming pointer is NULL, then we are creating
	     the prefs file for the first time - although, the prefs struct
             is already filled in, becuase all the default values are in
             xresources.h.
 ***************************************************************************/
Boolean write_preferences_file(prefsStructP inPrefsStruct)
{
    FILE *fp;
    
#ifdef VMS
    if (file_exists(prefs_file_pathname)) {
        if (get_pref_boolean(eBACKUP_FILES)) {
	    char *tf = NULL;
	    char retBuf[BUFSIZ];

	    tf = (char *)malloc(strlen(prefs_file_pathname) + 
		                strlen("_backup") + 5);
	    sprintf(tf, "%s_backup", prefs_file_pathname);
	    if (my_copy(prefs_file_pathname, tf, retBuf, BUFSIZ - 1,
			get_pref_int(eBACKUPFILEVERSIONS)) != SYS_SUCCESS) {
		fprintf(stderr, "%s\n", retBuf);
	    }
	    free(tf);
        }
    }

    remove(prefs_file_pathname);
#endif /* VMS, GEC */

    if (!(fp = fopen(prefs_file_pathname, "w"))) {
        fprintf(stderr, "Error: Can't open preferences file for writing\n");
        return False;
    }

    /* Write out our little header... */
    fprintf(fp, "# VMS Mosaic preferences file\n");
    fprintf(fp, "# File Version: %d.%d\n",
            PREFERENCES_MAJOR_VERSION,
            PREFERENCES_MINOR_VERSION);
    fprintf(fp, "# Warning - only the values are editable!\n");
    fprintf(fp, "# If a line is out of order or missing... it will be very bad!\n\n");

    /* Access all the fields in the prefs structure, and write them out */

    write_pref_boolean(fp, eUSE_PREFERENCES, "USE_PREFERENCES");
    write_pref_boolean(fp, eTRACK_VISITED_ANCHORS, "TRACK_VISITED_ANCHORS");
    write_pref_boolean(fp, eDISPLAY_URLS_NOT_TITLES, "DISPLAY_URLS_NOT_TITLES");
    write_pref_boolean(fp, eTRACK_POINTER_MOTION, "TRACK_POINTER_MOTION");
    write_pref_boolean(fp, eTRACK_FULL_URL_NAMES, "TRACK_FULL_URL_NAMES");
    write_pref_boolean(fp, eANNOTATIONS_ON_TOP, "ANNOTATIONS_ON_TOP");
    write_pref_boolean(fp, eCONFIRM_DELETE_ANNOTATION, "CONFIRM_DELETE_ANNOTATION");
    write_pref_string(fp, eANNOTATION_SERVER, "ANNOTATION_SERVER");
    write_pref_string(fp, eRECORD_COMMAND_LOCATION, "RECORD_COMMAND_LOCATION");
    write_pref_string(fp, eRECORD_COMMAND, "RECORD_COMMAND");
    write_pref_boolean(fp, eRELOAD_PRAGMA_NO_CACHE, "RELOAD_PRAGMA_NO_CACHE");
    write_pref_string(fp, eSENDMAIL_COMMAND, "SENDMAIL_COMMAND");
    write_pref_string(fp, eEDIT_COMMAND, "EDIT_COMMAND");
    write_pref_string(fp, eXTERM_COMMAND, "XTERM_COMMAND");
    write_pref_string(fp, eMAIL_FILTER_COMMAND, "MAIL_FILTER_COMMAND");
    write_pref_string(fp, ePRIVATE_ANNOTATION_DIRECTORY, "PRIVATE_ANNOTATION_DIRECTORY");
    write_pref_string(fp, eHOME_DOCUMENT, "HOME_DOCUMENT");
    write_pref_string(fp, eTMP_DIRECTORY, "TMP_DIRECTORY");
    write_pref_string(fp, eDOCS_DIRECTORY, "DOCS_DIRECTORY");
    write_pref_string(fp, eDEFAULT_FONT_CHOICE, "DEFAULT_FONT_CHOICE");
    write_pref_string(fp, eGLOBAL_HISTORY_FILE, "GLOBAL_HISTORY_FILE");
    write_pref_boolean(fp, eUSE_GLOBAL_HISTORY, "USE_GLOBAL_HISTORY");
    write_pref_string(fp, eHISTORY_FILE, "HISTORY_FILE");
    write_pref_string(fp, eDEFAULT_HOT_FILE, "DEFAULT_HOT_FILE");
    write_pref_boolean(fp, eADD_HOTLIST_ADDS_RBM, "ADD_HOTLIST_ADDS_RBM");
    write_pref_boolean(fp, eADD_RBM_ADDS_RBM, "ADD_RBM_ADDS_RBM");
    write_pref_string(fp, eDOCUMENTS_MENU_SPECFILE, "DOCUMENTS_MENU_SPECFILE");
    write_pref_int(fp, eCOLORS_PER_INLINED_IMAGE, "COLORS_PER_INLINED_IMAGE");
    write_pref_int(fp, eIMAGE_CACHE_SIZE, "IMAGE_CACHE_SIZE");
    write_pref_boolean(fp, eRELOAD_RELOADS_IMAGES, "RELOAD_RELOADS_IMAGES");
    write_pref_boolean(fp, eREVERSE_INLINED_BITMAP_COLORS, "REVERSE_INLINED_BITMAP_COLORS");
    write_pref_boolean(fp, eDELAY_IMAGE_LOADS, "DELAY_IMAGE_LOADS");
    write_pref_float(fp, eSCREEN_GAMMA, "SCREEN_GAMMA");
    write_pref_string(fp, eDEFAULT_AUTHOR_NAME, "DEFAULT_AUTHOR_NAME");
    write_pref_string(fp, eDEFAULT_AUTHOR_EMAIL, "DEFAULT_AUTHOR_EMAIL");
    write_pref_string(fp, eSIGNATURE, "SIGNATURE");
    write_pref_string(fp, eMAIL_MODE, "MAIL_MODE");
    write_pref_string(fp, ePRINT_COMMAND, "PRINT_COMMAND");
    write_pref_string(fp, eUNCOMPRESS_COMMAND, "UNCOMPRESS_COMMAND");
    write_pref_string(fp, eGUNZIP_COMMAND, "GUNZIP_COMMAND");
    write_pref_boolean(fp, eUSE_DEFAULT_EXTENSION_MAP, "USE_DEFAULT_EXTENSION_MAP");
    write_pref_boolean(fp, eUSE_DEFAULT_TYPE_MAP, "USE_DEFAULT_TYPE_MAP");
    write_pref_string(fp, eGLOBAL_EXTENSION_MAP, "GLOBAL_EXTENSION_MAP");
    write_pref_string(fp, ePERSONAL_EXTENSION_MAP, "PERSONAL_EXTENSION_MAP");
    write_pref_string(fp, eGLOBAL_TYPE_MAP, "GLOBAL_TYPE_MAP");
    write_pref_string(fp, ePERSONAL_TYPE_MAP, "PERSONAL_TYPE_MAP");
    write_pref_boolean(fp, eTWEAK_GOPHER_TYPES, "TWEAK_GOPHER_TYPES");
    write_pref_string(fp, ePRINT_MODE, "PRINT_MODE");
    write_pref_string(fp, eGUI_LAYOUT, "GUI_LAYOUT");
    write_pref_boolean(fp, ePRINT_BANNERS, "PRINT_BANNERS");
    write_pref_boolean(fp, ePRINT_FOOTNOTES, "PRINT_FOOTNOTES");
    write_pref_boolean(fp, ePRINT_PAPER_SIZE_US, "PRINT_PAPER_SIZE_US");
    write_pref_string(fp, ePROXY_SPECFILE, "PROXY_SPECFILE");
    write_pref_string(fp, eNOPROXY_SPECFILE, "NOPROXY_SPECFILE");
    write_pref_int(fp, eCCIPORT, "CCIPORT");
    write_pref_int(fp, eMAX_NUM_OF_CCI_CONNECTIONS, "MAX_NUM_OF_CCI_CONNECTIONS");
    write_pref_int(fp, eMAX_WAIS_RESPONSES, "MAX_WAIS_RESPONSES");
    write_pref_boolean(fp, eKIOSK, "KIOSK");
    write_pref_boolean(fp, eKIOSKPRINT, "KIOSKPRINT");
    write_pref_boolean(fp, eKIOSKNOEXIT, "KIOSKNOEXIT");
    write_pref_boolean(fp, eKEEPALIVE, "KEEPALIVE");
    write_pref_int(fp, eFTP_TIMEOUT_VAL, "FTP_TIMEOUT_VAL");
    write_pref_boolean(fp, eENABLE_TABLES, "ENABLE_TABLES");
    write_pref_int(fp, eDEFAULT_WIDTH, "DEFAULT_WIDTH");
    write_pref_int(fp, eDEFAULT_HEIGHT, "DEFAULT_HEIGHT");
    write_pref_boolean(fp, eAUTO_PLACE_WINDOWS, "AUTO_PLACE_WINDOWS");
    write_pref_boolean(fp, eINITIAL_WINDOW_ICONIC, "INITIAL_WINDOW_ICONIC");
    write_pref_boolean(fp, eTITLEISWINDOWTITLE, "TITLEISWINDOWTITLE");
    fprintf(fp, "USEICONBAR: False\n");
    fprintf(fp, "USETEXTBUTTONBAR: False\n");

    write_pref_boolean(fp, eTWIRLING_TRANSFER_ICON, "TWIRLING_TRANSFER_ICON");
    write_pref_boolean(fp, eSECURITYICON, "SECURITYICON");
    write_pref_int(fp, eTWIRL_INCREMENT, "TWIRL_INCREMENT");
    write_pref_string(fp, eSAVE_MODE, "SAVE_MODE");
    write_pref_string(fp, eFULL_HOSTNAME, "FULL_HOSTNAME");
    write_pref_int(fp, eLOAD_LOCAL_FILE, "LOAD_LOCAL_FILE");
    write_pref_boolean(fp, eEDIT_COMMAND_USE_XTERM, "EDIT_COMMAND_USE_XTERM");
    write_pref_boolean(fp, eCONFIRM_EXIT, "CONFIRM_EXIT");
    write_pref_boolean(fp, eDEFAULT_FANCY_SELECTIONS, "DEFAULT_FANCY_SELECTIONS");
    write_pref_boolean(fp, eCATCH_PRIOR_AND_NEXT, "CATCH_PRIOR_AND_NEXT");
    write_pref_boolean(fp, eSIMPLE_INTERFACE, "SIMPLE_INTERFACE");
    write_pref_boolean(fp, ePROTECT_ME_FROM_MYSELF, "PROTECT_ME_FROM_MYSELF");
    write_pref_boolean(fp, eGETHOSTBYNAME_IS_EVIL, "GETHOSTBYNAME_IS_EVIL");
#ifdef __sgi
    write_pref_boolean(fp, eDEBUGGING_MALLOC, "DEBUGGING_MALLOC");
#endif
    write_pref_boolean(fp, eUSEAFSKLOG, "USEAFSKLOG");
    write_pref_boolean(fp, eSEND_REFERER, "SEND_REFERER");
    write_pref_boolean(fp, eSEND_AGENT, "SEND_AGENT");
    write_pref_boolean(fp, eEXPAND_URLS, "EXPAND_URLS");
    write_pref_boolean(fp, eEXPAND_URLS_WITH_NAME, "EXPAND_URLS_WITH_NAME");
    write_pref_string(fp, eDEFAULT_PROTOCOL, "DEFAULT_PROTOCOL");
    write_pref_string(fp, eMETER_FOREGROUND, "METER_FOREGROUND");
    write_pref_string(fp, eMETER_BACKGROUND, "METER_BACKGROUND");
    write_pref_string(fp, eMETER_FONT_FOREGROUND, "METER_FONT_FOREGROUND");
    write_pref_string(fp, eMETER_FONT_BACKGROUND, "METER_FONT_BACKGROUND");
    write_pref_boolean(fp, eMETER, "METER");
    write_pref_boolean(fp, eBACKUP_FILES, "BACKUP_FILES");
    write_pref_string(fp, ePIX_BASENAME, "PIX_BASENAME");
    write_pref_int(fp, ePIX_COUNT, "PIX_COUNT");
    write_pref_string(fp, eACCEPT_LANGUAGE_STR, "ACCEPT_LANGUAGE_STR");
    write_pref_int(fp, eFTP_REDIAL, "FTP_REDIAL");
    write_pref_int(fp, eFTP_REDIAL_SLEEP, "FTP_REDIAL_SLEEP");
    write_pref_int(fp, eFTP_FILENAME_LENGTH, "FTP_FILENAME_LENGTH");
    write_pref_int(fp, eFTP_ELLIPSIS_LENGTH, "FTP_ELLIPSIS_LENGTH");
    write_pref_int(fp, eFTP_ELLIPSIS_MODE, "FTP_ELLIPSIS_MODE");
    write_pref_boolean(fp, eTITLE_ISWINDOW_TITLE, "TITLE_ISWINDOW_TITLE");
    write_pref_boolean(fp, eUSE_SCREEN_GAMMA, "USE_SCREEN_GAMMA");
    write_pref_boolean(fp, eDISABLEMIDDLEBUTTON, "DISABLEMIDDLEBUTTON");
    write_pref_boolean(fp, eHTTPTRACE, "HTTPTRACE");
    write_pref_boolean(fp, eWWW2TRACE, "WWW2TRACE");
    write_pref_boolean(fp, eHTMLWTRACE, "HTMLWTRACE");
    write_pref_boolean(fp, eCCITRACE, "CCITRACE");
    write_pref_boolean(fp, eSRCTRACE, "SRCTRACE");
    write_pref_boolean(fp, eCACHETRACE, "CACHETRACE");
    write_pref_boolean(fp, eNUTTRACE, "NUTTRACE");
    write_pref_boolean(fp, eTABLETRACE, "TABLETRACE");
    write_pref_boolean(fp, eANIMATEBUSYICON, "ANIMATEBUSYICON");
    write_pref_boolean(fp, eSPLASHSCREEN, "SPLASHSCREEN");
    write_pref_boolean(fp, eINSTALL_COLORMAP, "INSTALL_COLORMAP");
    write_pref_boolean(fp, eIMAGEVIEWINTERNAL, "IMAGEVIEWINTERNAL");
    write_pref_int(fp, eURLEXPIRED, "URLEXPIRED");
    write_pref_int(fp, ePOPUPCASCADEMAPPINGDELAY, "POPUPCASCADEMAPPINGDELAY");
    write_pref_boolean(fp, eFRAME_SUPPORT, "FRAME_SUPPORT");
    write_pref_boolean(fp, eCLIPPING, "CLIPPING");
    write_pref_int(fp, eMAX_CLIP_TRANSITIONS, "MAX_CLIP_TRANSITIONS");
    write_pref_boolean(fp, eUSE_LONG_TEXT_NAMES, "USE_LONG_TEXT_NAMES");
    write_pref_string(fp, eTOOLBAR_LAYOUT, "TOOLBAR_LAYOUT");

    write_pref_boolean(fp, eUSETHREADVIEW, "USETHREADVIEW");
    write_pref_boolean(fp, eSHOWREADGROUPS, "SHOWREADGROUPS");
    write_pref_boolean(fp, eNOTHREADJUMPING, "NOTHREADJUMPING");
    write_pref_boolean(fp, eSHOWALLGROUPS, "SHOWALLGROUPS");
    write_pref_boolean(fp, eSHOWALLARTICLES, "SHOWALLARTICLES");
    write_pref_boolean(fp, eUSEBACKGROUNDFLUSH, "USEBACKGROUNDFLUSH");
    write_pref_int(fp, eBACKGROUNDFLUSHTIME, "BACKGROUNDFLUSHTIME");
    write_pref_boolean(fp, ePREVISUNREAD, "PREVISPREVUNREAD");
    write_pref_boolean(fp, eNEXTISUNREAD, "NEXTISNEXTUNREAD");
    write_pref_boolean(fp, eUSENEWSRC, "USENEWSRC");
    write_pref_string(fp, eNEWSRCPREFIX, "NEWSRCPREFIX");
    write_pref_int(fp, eNEWSAUTHORWIDTH, "NEWSAUTHORWIDTH");
    write_pref_int(fp, eNEWSSUBJECTWIDTH, "NEWSSUBJECTWIDTH");

    write_pref_boolean(fp, eFOCUS_FOLLOWS_MOUSE, "FOCUS_FOLLOWS_MOUSE");
    write_pref_boolean(fp, eSESSION_HISTORY_ON_RBM, "SESSION_HISTORY_ON_RBM");
    write_pref_int(fp, eNUMBER_OF_ITEMS_IN_RBM_HISTORY, "NUMBER_OF_ITEMS_IN_RBM_HISTORY");

    write_pref_boolean(fp, eUSESHORTNEWSRC, "USESHORTNEWSRC");
    write_pref_boolean(fp, eHOTLIST_ON_RBM, "HOTLIST_ON_RBM");
    write_pref_boolean(fp, eBODYCOLORS, "BODYCOLORS");
    write_pref_boolean(fp, eBODYIMAGES, "BODYIMAGES");
    write_pref_string(fp, eDEFAULTUNDERLINES, "DEFAULTUNDERLINES");
    write_pref_boolean(fp, eFTP_BINARY_MODE, "FTP_BINARY_MODE");
    write_pref_string(fp, eKIOSKPROTOCOLS, "KIOSKPROTOCOLS");
    write_pref_string(fp, eVMS_MAIL_PREFIX, "VMS_MAIL_PREFIX");
    write_pref_int(fp, eBACKUPFILEVERSIONS, "BACKUPFILEVERSIONS");
    write_pref_boolean(fp, eFONTCOLORS, "FONTCOLORS");
    write_pref_boolean(fp, ePROGRESSIVE_DISPLAY, "PROGRESSIVE_DISPLAY");
    write_pref_boolean(fp, eFONTSIZES, "FONTSIZES");
    write_pref_int(fp, eFONTBASESIZE, "FONTBASESIZE");
    write_pref_boolean(fp, eTRACK_TARGET_ANCHORS, "TRACK_TARGET_ANCHORS");
    write_pref_boolean(fp, eDEBUG_MENU, "DEBUG_MENU");
    write_pref_boolean(fp, eREPORTBUGS, "REPORTBUGS");
    write_pref_boolean(fp, eIMAGE_ANIMATION, "IMAGE_ANIMATION");
    write_pref_int(fp, eMIN_ANIMATION_DELAY, "MIN_ANIMATION_DELAY");
    write_pref_boolean(fp, eREFRESHTRACE, "REFRESHTRACE");
    write_pref_boolean(fp, eREFRESH_URL, "REFRESH_URL");
    write_pref_boolean(fp, eBROWSER_SAFE_COLORS, "BROWSER_SAFE_COLORS");
    write_pref_boolean(fp, eBLINKING_TEXT, "BLINKING_TEXT");
    write_pref_int(fp, eBLINK_TIME, "BLINK_TIME");
    write_pref_boolean(fp, eCOOKIES, "COOKIES");
    write_pref_boolean(fp, eACCEPT_ALL_COOKIES, "ACCEPT_ALL_COOKIES");
    write_pref_int(fp, eMAXPIXMAPWIDTH, "MAXPIXMAPWIDTH");
    write_pref_int(fp, eMAXPIXMAPHEIGHT, "MAXPIXMAPHEIGHT");
    write_pref_boolean(fp, eUSE_COOKIE_FILE, "USE_COOKIE_FILE");
    write_pref_string(fp, eCOOKIE_FILE, "COOKIE_FILE");
    write_pref_string(fp, eIMAGEDELAY_FILE, "IMAGEDELAY_FILE");
    write_pref_boolean(fp, eBROWSERSAFECOLORS_IF_TRUECOLOR, "BROWSERSAFECOLORS_IF_TRUECOLOR");
    write_pref_boolean(fp, eHOTKEYS, "HOTKEYS");
    write_pref_boolean(fp, eINVALID_COOKIE_PROMPT, "INVALID_COOKIE_PROMPT");
    write_pref_int(fp, eMAX_COOKIES, "MAX_COOKIES");
    write_pref_int(fp, eCOOKIE_DOMAIN_LIMIT, "COOKIE_DOMAIN_LIMIT");
    write_pref_string(fp, ePERM_FILE, "PERM_FILE");
    write_pref_string(fp, eFORM_BUTTON_BACKGROUND, "FORM_BUTTON_BACKGROUND");
    write_pref_boolean(fp, eVERIFY_SSL_CERTIFICATES, "VERIFY_SSL_CERTIFICATES");
    write_pref_int(fp, eHOTLIST_MENU_HEIGHT, "HOTLIST_MENU_HEIGHT");
    write_pref_int(fp, eHOTLIST_MENU_WIDTH, "HOTLIST_MENU_WIDTH");
    write_pref_int(fp, eMARKUP_MEMORY_PREALLOCATION, "MARKUP_MEMORY_PREALLOCATION");
    write_pref_int(fp, eELEMENT_MEMORY_PREALLOCATION, "ELEMENT_MEMORY_PREALLOCATION");
    write_pref_boolean(fp, eCOOKIETRACE, "COOKIETRACE");
    write_pref_boolean(fp, eCLUE_HELP, "CLUE_HELP");
    write_pref_string(fp, eCLUE_FOREGROUND, "CLUE_FOREGROUND");
    write_pref_string(fp, eCLUE_BACKGROUND, "CLUE_BACKGROUND");
    write_pref_int(fp, eCLUE_POPUP_DELAY, "CLUE_POPUP_DELAY");
    write_pref_int(fp, eCLUE_POPDOWN_DELAY, "CLUE_POPDOWN_DELAY");
    write_pref_string(fp, eCLUE_FONT, "CLUE_FONT");
    write_pref_boolean(fp, eCLUE_OVAL, "CLUE_OVAL");
    write_pref_boolean(fp, eCLUE_ROUNDED, "CLUE_ROUNDED");
    write_pref_boolean(fp, ePRINT_DUPLEX, "PRINT_DUPLEX");
    write_pref_boolean(fp, eMENUBAR_TEAROFF, "MENUBAR_TEAROFF");
    write_pref_boolean(fp, eTIFF_ERROR_MESSAGES, "TIFF_ERROR_MESSAGES");
    write_pref_boolean(fp, ePNG_ERROR_MESSAGES, "PNG_ERROR_MESSAGES");
    write_pref_boolean(fp, eJPEG_ERROR_MESSAGES, "JPEG_ERROR_MESSAGES");
    write_pref_boolean(fp, eDETACHED_TOOLBAR, "DETACHED_TOOLBAR");
    write_pref_boolean(fp, eDETACHED_TOOLBAR_VERTICAL, "DETACHED_TOOLBAR_VERTICAL");
    write_pref_boolean(fp, eTEXT_TOOLBAR, "TEXT_TOOLBAR");
    write_pref_boolean(fp, eENCRYPTION_ICON, "ENCRYPTION_ICON");
    write_pref_boolean(fp, ePRESENTATION_MODE_ON_RBM, "PRESENTATION_MODE_ON_RBM");
    write_pref_boolean(fp, eMULTIPLE_IMAGE_LOAD, "MULTIPLE_IMAGE_LOAD");
    write_pref_int(fp, eMULTIPLE_IMAGE_LIMIT, "MULTIPLE_IMAGE_LIMIT");
    write_pref_boolean(fp, eJPEG2000_ERROR_MESSAGES, "JPEG2000_ERROR_MESSAGES");

    fclose(fp);
    return True;
}

/****************************************************************************
 ****************************************************************************
 *                   Preference Structure access functions
 *
 ****************************************************************************
 ***************************************************************************/

/****************************************************************************
   Function: get_ptr_to_preferences(void)
   Desc:     Returns a pointer to the main preferences structure
 ***************************************************************************/
prefsStructP get_ptr_to_preferences(void)
{
    return thePrefsStructP;
}

/****************************************************************************
   Function: get_pref(int pref_id)
   Desc:     Returns a pointer to the single preference variable
                 denoted by pref_id
 ***************************************************************************/
void *get_pref(int pref_id)
{
    switch(pref_id) {
        case  eUSE_PREFERENCES:
            return (void *)&(thePrefsStructP->RdataP->use_preferences);
        case  eTRACK_VISITED_ANCHORS:
            return (void *)&(thePrefsStructP->RdataP->track_visited_anchors);
        case  eDISPLAY_URLS_NOT_TITLES:
            return (void *)&(thePrefsStructP->RdataP->display_urls_not_titles);
        case  eTRACK_POINTER_MOTION:
            return (void *)&(thePrefsStructP->RdataP->track_pointer_motion);
        case  eTRACK_FULL_URL_NAMES:
            return (void *)&(thePrefsStructP->RdataP->track_full_url_names);
        case  eANNOTATIONS_ON_TOP:
            return (void *)&(thePrefsStructP->RdataP->annotations_on_top);
        case  eCONFIRM_DELETE_ANNOTATION:
            return (void *)&(thePrefsStructP->RdataP->confirm_delete_annotation);
        case  eANNOTATION_SERVER:
            return (void *)(thePrefsStructP->RdataP->annotation_server);
        case  eRECORD_COMMAND_LOCATION:
            return (void *)(thePrefsStructP->RdataP->record_command_location);
        case  eRECORD_COMMAND:
            return (void *)(thePrefsStructP->RdataP->record_command);
        case  eRELOAD_PRAGMA_NO_CACHE:
            return (void *)&(thePrefsStructP->RdataP->reload_pragma_no_cache);
        case  eSENDMAIL_COMMAND:
            return (void *)(thePrefsStructP->RdataP->sendmail_command);
        case  eEDIT_COMMAND:
            return (void *)(thePrefsStructP->RdataP->edit_command);
        case  eXTERM_COMMAND:
            return (void *)(thePrefsStructP->RdataP->xterm_command);
        case  eMAIL_FILTER_COMMAND:
            return (void *)(thePrefsStructP->RdataP->mail_filter_command);
        case  ePRIVATE_ANNOTATION_DIRECTORY:
            return (void *)(thePrefsStructP->RdataP->private_annotation_directory);
        case  eHOME_DOCUMENT:
            return (void *)(thePrefsStructP->RdataP->home_document);
        case  eTMP_DIRECTORY:
            return (void *)(thePrefsStructP->RdataP->tmp_directory);
        case  eDOCS_DIRECTORY:
            return (void *)(thePrefsStructP->RdataP->docs_directory);
        case  eDEFAULT_FONT_CHOICE:
            return (void *)(thePrefsStructP->RdataP->default_font_choice);
        case  eGLOBAL_HISTORY_FILE:
            return (void *)(thePrefsStructP->RdataP->global_history_file);
        case  eHISTORY_FILE:
            return (void *)(thePrefsStructP->RdataP->history_file);
        case  eUSE_GLOBAL_HISTORY:
            return (void *)&(thePrefsStructP->RdataP->use_global_history);
        case  eDEFAULT_HOTLIST_FILE:
            return (void *)(thePrefsStructP->RdataP->default_hotlist_file);
        case  eDEFAULT_HOT_FILE:
            return (void *)(thePrefsStructP->RdataP->default_hot_file);
        case  eADD_HOTLIST_ADDS_RBM:
            return (void *)&(thePrefsStructP->RdataP->addHotlistAddsRBM);
        case  eADD_RBM_ADDS_RBM:
            return (void *)&(thePrefsStructP->RdataP->addRBMAddsRBM);
        case  eDOCUMENTS_MENU_SPECFILE:
            return (void *)(thePrefsStructP->RdataP->documents_menu_specfile);
        case  eCOLORS_PER_INLINED_IMAGE:
            return (void *)&(thePrefsStructP->RdataP->colors_per_inlined_image);
        case  eIMAGE_CACHE_SIZE:
            return (void *)&(thePrefsStructP->RdataP->image_cache_size);
        case  eRELOAD_RELOADS_IMAGES:
            return (void *)&(thePrefsStructP->RdataP->reload_reloads_images);
        case  eREVERSE_INLINED_BITMAP_COLORS:
            return (void *)&(thePrefsStructP->RdataP->reverse_inlined_bitmap_colors);
        case  eDELAY_IMAGE_LOADS:
            return (void *)&(thePrefsStructP->RdataP->delay_image_loads);
        case  eDEFAULT_AUTHOR_NAME:
            return (void *)(thePrefsStructP->RdataP->default_author_name);
        case  eDEFAULT_AUTHOR_EMAIL:
            return (void *)(thePrefsStructP->RdataP->default_author_email);
        case  eSIGNATURE:
            return (void *)(thePrefsStructP->RdataP->signature);
        case  eMAIL_MODE:
            return (void *)(thePrefsStructP->RdataP->mail_mode);
        case  ePRINT_COMMAND:
            return (void *)(thePrefsStructP->RdataP->print_command);
        case  eUNCOMPRESS_COMMAND:
            return (void *)(thePrefsStructP->RdataP->uncompress_command);
        case  eGUNZIP_COMMAND:
            return (void *)(thePrefsStructP->RdataP->gunzip_command);
        case  eUSE_DEFAULT_EXTENSION_MAP:
            return (void *)&(thePrefsStructP->RdataP->use_default_extension_map);
        case  eUSE_DEFAULT_TYPE_MAP:
            return (void *)&(thePrefsStructP->RdataP->use_default_type_map);
        case  eGLOBAL_EXTENSION_MAP:
            return (void *)(thePrefsStructP->RdataP->global_extension_map);
        case  ePERSONAL_EXTENSION_MAP:
            return (void *)(thePrefsStructP->RdataP->personal_extension_map);
        case  eGLOBAL_TYPE_MAP:
            return (void *)(thePrefsStructP->RdataP->global_type_map);
        case  ePERSONAL_TYPE_MAP:
            return (void *)(thePrefsStructP->RdataP->personal_type_map);
        case  eTWEAK_GOPHER_TYPES:
            return (void *)&(thePrefsStructP->RdataP->tweak_gopher_types);
        case eGUI_LAYOUT:
            return (void *)(thePrefsStructP->RdataP->gui_layout);
        case  ePRINT_MODE:
            return (void *)(thePrefsStructP->RdataP->print_mode);
        case  ePRINT_BANNERS:
            return (void *)&(thePrefsStructP->RdataP->print_banners);
        case  ePRINT_FOOTNOTES:
            return (void *)&(thePrefsStructP->RdataP->print_footnotes);
        case  ePRINT_PAPER_SIZE_US:
            return (void *)&(thePrefsStructP->RdataP->print_us);
        case  ePROXY_SPECFILE:
            return (void *)(thePrefsStructP->RdataP->proxy_specfile);
        case  eNOPROXY_SPECFILE:
            return (void *)(thePrefsStructP->RdataP->noproxy_specfile);
        case  eCCIPORT:
            return (void *)&(thePrefsStructP->RdataP->cciPort);
        case  eMAX_NUM_OF_CCI_CONNECTIONS:
            return (void *)&(thePrefsStructP->RdataP->max_num_of_cci_connections);
        case  eMAX_WAIS_RESPONSES:
            return (void *)&(thePrefsStructP->RdataP->max_wais_responses);
        case  eKIOSK:
            return (void *)&(thePrefsStructP->RdataP->kiosk);
        case  eKIOSKPRINT:
            return (void *)&(thePrefsStructP->RdataP->kioskPrint);
        case  eKIOSKNOEXIT:
            return (void *)&(thePrefsStructP->RdataP->kioskNoExit);
        case  eKEEPALIVE:
            return (void *)&(thePrefsStructP->RdataP->keepAlive);
        case  eFTP_TIMEOUT_VAL:
            return (void *)&(thePrefsStructP->RdataP->ftp_timeout_val);
        case  eENABLE_TABLES:
            return (void *)&(thePrefsStructP->RdataP->enable_tables);
        case  eDEFAULT_WIDTH:
            return (void *)&(thePrefsStructP->RdataP->default_width);
        case  eDEFAULT_HEIGHT:
            return (void *)&(thePrefsStructP->RdataP->default_height);
        case  eAUTO_PLACE_WINDOWS:
            return (void *)&(thePrefsStructP->RdataP->auto_place_windows);
        case  eINITIAL_WINDOW_ICONIC:
            return (void *)&(thePrefsStructP->RdataP->initial_window_iconic);
        case  eTITLEISWINDOWTITLE:
            return (void *)&(thePrefsStructP->RdataP->titleIsWindowTitle);
        case  eTWIRLING_TRANSFER_ICON:
            return (void *)&(thePrefsStructP->RdataP->twirling_transfer_icon);
        case  eSECURITYICON:
            return (void *)&(thePrefsStructP->RdataP->securityIcon);
        case  eTWIRL_INCREMENT:
            return (void *)&(thePrefsStructP->RdataP->twirl_increment);
        case  eSAVE_MODE:
            return (void *)(thePrefsStructP->RdataP->save_mode);
        case  eFULL_HOSTNAME:
            return (void *)(thePrefsStructP->RdataP->full_hostname);
        case  eLOAD_LOCAL_FILE:
            return (void *)&(thePrefsStructP->RdataP->load_local_file);
        case  eEDIT_COMMAND_USE_XTERM:
            return (void *)&(thePrefsStructP->RdataP->edit_command_use_xterm);
        case  eCONFIRM_EXIT:
            return (void *)&(thePrefsStructP->RdataP->confirm_exit);
        case  eDEFAULT_FANCY_SELECTIONS:
            return (void *)&(thePrefsStructP->RdataP->default_fancy_selections);
        case  eCATCH_PRIOR_AND_NEXT:
            return (void *)&(thePrefsStructP->RdataP->catch_prior_and_next);
        case  eSIMPLE_INTERFACE:
            return (void *)&(thePrefsStructP->RdataP->simple_interface);
        case  ePROTECT_ME_FROM_MYSELF:
            return (void *)&(thePrefsStructP->RdataP->protect_me_from_myself);
        case  eGETHOSTBYNAME_IS_EVIL:
            return (void *)&(thePrefsStructP->RdataP->gethostbyname_is_evil);
#ifdef __sgi
        case  eDEBUGGING_MALLOC:
            return (void *)&(thePrefsStructP->RdataP->debugging_malloc);
#endif
        case  eUSEAFSKLOG:
            return (void *)&(thePrefsStructP->RdataP->useAFSKlog);

	/* New in 2.7 */
        case eSEND_REFERER:
            return (void *)&(thePrefsStructP->RdataP->sendReferer);
        case eSEND_AGENT:
            return (void *)&(thePrefsStructP->RdataP->sendAgent);
        case eEXPAND_URLS:
            return (void *)&(thePrefsStructP->RdataP->expandUrls);
        case eEXPAND_URLS_WITH_NAME:
            return (void *)&(thePrefsStructP->RdataP->expandUrlsWithName);
        case eDEFAULT_PROTOCOL:
            return (void *)(thePrefsStructP->RdataP->defaultProtocol);
        case eMETER_FOREGROUND:
            return (void *)(thePrefsStructP->RdataP->meterForeground);
        case eMETER_BACKGROUND:
            return (void *)(thePrefsStructP->RdataP->meterBackground);
        case eMETER_FONT_FOREGROUND:
            return (void *)(thePrefsStructP->RdataP->meterFontForeground);
        case eMETER_FONT_BACKGROUND:
            return (void *)(thePrefsStructP->RdataP->meterFontBackground);
        case eMETER:
            return (void *)&(thePrefsStructP->RdataP->use_meter);
        case eBACKUP_FILES:
            return (void *)&(thePrefsStructP->RdataP->backup_files);
        case ePIX_BASENAME:
            return (void *)(thePrefsStructP->RdataP->pix_basename);
        case ePIX_COUNT:
            return (void *)&(thePrefsStructP->RdataP->pix_count);
        case eACCEPT_LANGUAGE_STR:
            return (void *)(thePrefsStructP->RdataP->acceptlanguage_str);
        case eFTP_REDIAL:
            return (void *)&(thePrefsStructP->RdataP->ftpRedial);
        case eFTP_REDIAL_SLEEP:
            return (void *)&(thePrefsStructP->RdataP->ftpRedialSleep);
        case eFTP_FILENAME_LENGTH:
            return (void *)&(thePrefsStructP->RdataP->ftpFilenameLength);
        case eFTP_ELLIPSIS_LENGTH:
            return (void *)&(thePrefsStructP->RdataP->ftpEllipsisLength);
        case eFTP_ELLIPSIS_MODE:
            return (void *)&(thePrefsStructP->RdataP->ftpEllipsisMode);
        case eTITLE_ISWINDOW_TITLE:
            return (void *)&(thePrefsStructP->RdataP->titleIsWindowTitle);
        case eUSE_SCREEN_GAMMA:
            return (void *)&(thePrefsStructP->RdataP->useScreenGamma);
        case eSCREEN_GAMMA:
            return (void *)&(thePrefsStructP->RdataP->screen_gamma);
        case eDISABLEMIDDLEBUTTON:
            return (void *)&(thePrefsStructP->RdataP->disableMiddleButton);
        case eHTTPTRACE:
            return (void *)&(thePrefsStructP->RdataP->httpTrace);
        case eWWW2TRACE:
            return (void *)&(thePrefsStructP->RdataP->www2Trace);
        case eHTMLWTRACE:
            return (void *)&(thePrefsStructP->RdataP->htmlwTrace);
        case eCCITRACE:
            return (void *)&(thePrefsStructP->RdataP->cciTrace);
        case eSRCTRACE:
            return (void *)&(thePrefsStructP->RdataP->srcTrace);
        case eCACHETRACE:
            return (void *)&(thePrefsStructP->RdataP->cacheTrace);
        case eNUTTRACE:
            return (void *)&(thePrefsStructP->RdataP->nutTrace);
        case eTABLETRACE:
            return (void *)&(thePrefsStructP->RdataP->tableTrace);
        case eANIMATEBUSYICON:
            return (void *)&(thePrefsStructP->RdataP->animateBusyIcon);
        case eSPLASHSCREEN:
            return (void *)&(thePrefsStructP->RdataP->splashScreen);
        case eINSTALL_COLORMAP:
            return (void *)&(thePrefsStructP->RdataP->instamap);
        case eIMAGEVIEWINTERNAL:
            return (void *)&(thePrefsStructP->RdataP->imageViewInternal);
        case eURLEXPIRED:
            return (void *)&(thePrefsStructP->RdataP->urlExpired);
        case ePOPUPCASCADEMAPPINGDELAY:
            return (void *)&(thePrefsStructP->RdataP->popupCascadeMappingDelay);
        case eFRAME_SUPPORT:
            return (void *)&(thePrefsStructP->RdataP->frame_support);

        case eUSETHREADVIEW:
	    return (void *)&(thePrefsStructP->RdataP->newsConfigView);
      
        case eSHOWREADGROUPS:
	    return (void *)&(thePrefsStructP->RdataP->newsShowReadGroups);

        case eNOTHREADJUMPING:
	    return (void *)&(thePrefsStructP->RdataP->newsNoThreadJumping);

        case eSHOWALLGROUPS:
	    return (void *)&(thePrefsStructP->RdataP->newsShowAllGroups);
  
        case eSHOWALLARTICLES:
	    return (void *)&(thePrefsStructP->RdataP->newsShowAllArticles);

        case eUSEBACKGROUNDFLUSH:
	    return (void *)&(thePrefsStructP->RdataP->newsUseBackgroundFlush);

        case eBACKGROUNDFLUSHTIME:
	    return (void *)&(thePrefsStructP->RdataP->newsBackgroundFlushTime);

        case eCLIPPING:
	    return (void *)&(thePrefsStructP->RdataP->clipping);

        case eMAX_CLIP_TRANSITIONS:
            return (void *)&(thePrefsStructP->RdataP->max_clip_transitions);

        case eUSE_LONG_TEXT_NAMES:
	    return (void *)&(thePrefsStructP->RdataP->long_text_names);

        case eTOOLBAR_LAYOUT:
            return (void *)(thePrefsStructP->RdataP->toolbar_layout);

        case eNEXTISUNREAD:
	    return (void *)&(thePrefsStructP->RdataP->newsNextIsUnread);
        case ePREVISUNREAD:
	    return (void *)&(thePrefsStructP->RdataP->newsPrevIsUnread);
        case eUSENEWSRC:
	    return (void *)&(thePrefsStructP->RdataP->newsUseNewsrc);
        case eNEWSRCPREFIX:
	    return (void *)(thePrefsStructP->RdataP->newsNewsrcPrefix);
        case eNEWSSUBJECTWIDTH:
	    return (void *)&(thePrefsStructP->RdataP->newsSubjectWidth);
        case eNEWSAUTHORWIDTH:
	    return (void *)&(thePrefsStructP->RdataP->newsAuthorWidth);

        case eFOCUS_FOLLOWS_MOUSE:
            return (void *)&(thePrefsStructP->RdataP->focusFollowsMouse);

        case eSESSION_HISTORY_ON_RBM:
            return (void *)&(thePrefsStructP->RdataP->sessionHistoryOnRBM);

        case eNUMBER_OF_ITEMS_IN_RBM_HISTORY:
	    return (void *)&(thePrefsStructP->RdataP->numberOfItemsInRBMHistory);

        case eHOTLIST_ON_RBM:
	    return (void *)&(thePrefsStructP->RdataP->hotlistOnRBM);

        case eUSESHORTNEWSRC:
	    return (void *)&(thePrefsStructP->RdataP->newsUseShortNewsrc);

	/* 2.7b6 */
	case eKIOSKPROTOCOLS:
	    return (void *)(thePrefsStructP->RdataP->kioskProtocols);

	case eBODYCOLORS:
	    return (void *)&(thePrefsStructP->RdataP->bodyColors);

	case eBODYIMAGES:
	    return (void *)&(thePrefsStructP->RdataP->bodyImages);

	case eDEFAULTUNDERLINES:
	    return (void *)(thePrefsStructP->RdataP->defaultUnderlines);

	case eFTP_BINARY_MODE:
	    return (void *)&(thePrefsStructP->RdataP->ftp_binary_mode);

        case eVMS_MAIL_PREFIX:
            return (void *)(thePrefsStructP->RdataP->vms_mail_prefix);

        case eBACKUPFILEVERSIONS:
            return (void *)&(thePrefsStructP->RdataP->backupFileVersions);

	case eFONTCOLORS:
	    return (void *)&(thePrefsStructP->RdataP->fontColors);

	case ePROGRESSIVE_DISPLAY:
	    return (void *)&(thePrefsStructP->RdataP->progressive_display);

	case eFONTSIZES:
	    return (void *)&(thePrefsStructP->RdataP->fontSizes);

	case eFONTBASESIZE:
	    return (void *)&(thePrefsStructP->RdataP->fontBaseSize);

        case eTRACK_TARGET_ANCHORS:
            return (void *)&(thePrefsStructP->RdataP->track_target_anchors);

        case eDEBUG_MENU:
            return (void *)&(thePrefsStructP->RdataP->debug_menu);

        case eREPORTBUGS:
            return (void *)&(thePrefsStructP->RdataP->reportBugs);

        case eIMAGE_ANIMATION:
            return (void *)&(thePrefsStructP->RdataP->image_animation);

	case eMIN_ANIMATION_DELAY:
	    return (void *)&(thePrefsStructP->RdataP->min_animation_delay);

        case eREFRESHTRACE:
            return (void *)&(thePrefsStructP->RdataP->refreshTrace);

        case eREFRESH_URL:
            return (void *)&(thePrefsStructP->RdataP->refresh_URL);

        case eBROWSER_SAFE_COLORS:
            return (void *)&(thePrefsStructP->RdataP->browser_safe_colors);

        case eBLINKING_TEXT:
            return (void *)&(thePrefsStructP->RdataP->blinking_text);

	case eBLINK_TIME:
	    return (void *)&(thePrefsStructP->RdataP->blink_time);

        case eCOOKIES:
            return (void *)&(thePrefsStructP->RdataP->cookies);

        case eACCEPT_ALL_COOKIES:
            return (void *)&(thePrefsStructP->RdataP->accept_all_cookies);

	case eMAXPIXMAPWIDTH:
	    return (void *)&(thePrefsStructP->RdataP->maxPixmapWidth);

	case eMAXPIXMAPHEIGHT:
	    return (void *)&(thePrefsStructP->RdataP->maxPixmapHeight);

        case eUSE_COOKIE_FILE:
            return (void *)&(thePrefsStructP->RdataP->use_cookie_file);

        case eCOOKIE_FILE:
            return (void *)(thePrefsStructP->RdataP->cookie_file);

        case eIMAGEDELAY_FILE:
            return (void *)(thePrefsStructP->RdataP->imagedelay_file);

        case eBROWSERSAFECOLORS_IF_TRUECOLOR:
            return (void *)&(thePrefsStructP->RdataP->BSColors_if_Truecolor);

        case eHOTKEYS:
            return (void *)&(thePrefsStructP->RdataP->hotkeys);

        case eINVALID_COOKIE_PROMPT:
            return (void *)&(thePrefsStructP->RdataP->invalid_cookie_prompt);

        case eMAX_COOKIES:
            return (void *)&(thePrefsStructP->RdataP->max_cookies);

        case eCOOKIE_DOMAIN_LIMIT:
            return (void *)&(thePrefsStructP->RdataP->cookie_domain_limit);

        case ePERM_FILE:
            return (void *)(thePrefsStructP->RdataP->perm_file);

        case eFORM_BUTTON_BACKGROUND:
            return (void *)(thePrefsStructP->RdataP->form_button_background);

        case eVERIFY_SSL_CERTIFICATES:
            return (void *)&(thePrefsStructP->RdataP->verify_ssl_certificates);

        case eHOTLIST_MENU_HEIGHT:
            return (void *)&(thePrefsStructP->RdataP->hotlist_menu_height);

        case eHOTLIST_MENU_WIDTH:
            return (void *)&(thePrefsStructP->RdataP->hotlist_menu_width);

        case eMARKUP_MEMORY_PREALLOCATION:
            return (void *)&(thePrefsStructP->RdataP->markup_memory_preallocation);

        case eELEMENT_MEMORY_PREALLOCATION:
            return (void *)&(thePrefsStructP->RdataP->element_memory_preallocation);

        case eCOOKIETRACE:
            return (void *)&(thePrefsStructP->RdataP->cookieTrace);

        case eCLUE_HELP:
            return (void *)&(thePrefsStructP->RdataP->clue_help);

        case eCLUE_FOREGROUND:
            return (void *)(thePrefsStructP->RdataP->clueForeground);

        case eCLUE_BACKGROUND:
            return (void *)(thePrefsStructP->RdataP->clueBackground);

        case eCLUE_POPUP_DELAY:
            return (void *)&(thePrefsStructP->RdataP->clueDelay);

        case eCLUE_POPDOWN_DELAY:
            return (void *)&(thePrefsStructP->RdataP->clueDownDelay);

        case eCLUE_FONT:
            return (void *)(thePrefsStructP->RdataP->clueFont);

        case eCLUE_OVAL:
            return (void *)&(thePrefsStructP->RdataP->clue_oval);

        case eCLUE_ROUNDED:
            return (void *)&(thePrefsStructP->RdataP->clue_rounded);

        case ePRINT_DUPLEX:
            return (void *)&(thePrefsStructP->RdataP->print_duplex);

        case eMENUBAR_TEAROFF:
            return (void *)&(thePrefsStructP->RdataP->menubar_tearoff);

        case eTIFF_ERROR_MESSAGES:
            return (void *)&(thePrefsStructP->RdataP->tiff_error_messages);

        case ePNG_ERROR_MESSAGES:
            return (void *)&(thePrefsStructP->RdataP->png_error_messages);

        case eJPEG_ERROR_MESSAGES:
            return (void *)&(thePrefsStructP->RdataP->jpeg_error_messages);

        case eDETACHED_TOOLBAR:
            return (void *)&(thePrefsStructP->RdataP->detached_toolbar);

        case eDETACHED_TOOLBAR_VERTICAL:
            return (void *)&(thePrefsStructP->RdataP->detached_toolbar_vertical);

        case eTEXT_TOOLBAR:
            return (void *)&(thePrefsStructP->RdataP->text_toolbar);

        case eENCRYPTION_ICON:
            return (void *)&(thePrefsStructP->RdataP->encryption_icon);

        case ePRESENTATION_MODE_ON_RBM:
            return (void *)&(thePrefsStructP->RdataP->presentationModeOnRBM);

        case eMULTIPLE_IMAGE_LOAD:
            return (void *)&(thePrefsStructP->RdataP->multiple_image_load);

        case eMULTIPLE_IMAGE_LIMIT:
            return (void *)&(thePrefsStructP->RdataP->multiple_image_limit);

        case eJPEG2000_ERROR_MESSAGES:
            return (void *)&(thePrefsStructP->RdataP->jpeg2000_error_messages);
    }

    fprintf(stderr, "Error: tried to get nonexistant preference\n");
    return NULL;
}

/****************************************************************************
   Function: get_pref_string(int pref_id)
   Desc:     Returns a pointer to the single preference variable
                 denoted by pref_id
 ***************************************************************************/
char *get_pref_string(int pref_id)
{
    char *tmp_string = (char *)get_pref(pref_id);

    if (!tmp_string || !*tmp_string) {
        return NULL;
    } else {
        return tmp_string;
    }
}

/****************************************************************************
   Function: get_pref_int(int pref_id)
   Desc:     Returns a pointer to the single preference variable
                 denoted by pref_id
 ***************************************************************************/
int get_pref_int(int pref_id)
{
    return *(int *)get_pref(pref_id);
}

/****************************************************************************
   Function: get_pref_boolean(int pref_id)
   Desc:     Returns a pointer to the single preference variable
                 denoted by pref_id
 ***************************************************************************/
Boolean get_pref_boolean(int pref_id)
{
    return *(Boolean *)get_pref(pref_id);
}

/****************************************************************************
   Function: get_pref_float(int pref_id)
   Desc:     Returns a pointer to the single preference variable
                 denoted by pref_id
 ***************************************************************************/
float get_pref_float(int pref_id)
{
    return *(float *)get_pref(pref_id);
}

/****************************************************************************
   Function: set_pref_boolean(int pref_id, int value)
   Desc:     Convenience for boolean setting.
 ***************************************************************************/
void set_pref_boolean(int pref_id, int value)
{
    set_pref(pref_id, &value);
}

/****************************************************************************
   Function: set_pref_int(int pref_id, int value)
   Desc:     Convenience for integer setting.
 ***************************************************************************/
void set_pref_int(int pref_id, int value)
{
    set_pref(pref_id, &value);
}

/****************************************************************************
   Function: set_pref_float(int pref_id, float value)
   Desc:     Convenience for float setting.
 ***************************************************************************/
void set_pref_float(int pref_id, float value)
{
    set_pref(pref_id, &value);
}

/****************************************************************************
   Function: set_pref(int pref_id, void *incoming)
   Desc:     set the single preference variable denoted by pref_id, to 
                 whatever incoming points to.
 ***************************************************************************/
void set_pref(int pref_id, void *incoming)
{
    switch(pref_id) {
        case eUSE_PREFERENCES:
            thePrefsStructP->RdataP->use_preferences = *((Boolean *)incoming);
            break;

        case eTRACK_VISITED_ANCHORS:
            thePrefsStructP->RdataP->track_visited_anchors =
                *((Boolean *)incoming);
            break;

        case eDISPLAY_URLS_NOT_TITLES:
            thePrefsStructP->RdataP->display_urls_not_titles =
                *((Boolean *)incoming);
            break;

        case eTRACK_POINTER_MOTION:
            thePrefsStructP->RdataP->track_pointer_motion =
                *((Boolean *)incoming);
            break;

        case eTRACK_FULL_URL_NAMES:
            thePrefsStructP->RdataP->track_full_url_names =
                *((Boolean *)incoming);
            break;

        case eANNOTATIONS_ON_TOP:
            thePrefsStructP->RdataP->annotations_on_top =
		*((Boolean *)incoming);
            break;

        case eCONFIRM_DELETE_ANNOTATION:
            thePrefsStructP->RdataP->confirm_delete_annotation =
                *((Boolean *)incoming);
            break;

        case eANNOTATION_SERVER:
            thePrefsStructP->RdataP->annotation_server = (char *)incoming;
            break;

        case eRECORD_COMMAND_LOCATION:
            thePrefsStructP->RdataP->record_command_location = (char *)incoming;
            break;

        case eRECORD_COMMAND:
            thePrefsStructP->RdataP->record_command = (char *)incoming;
            break;

        case eRELOAD_PRAGMA_NO_CACHE:
            thePrefsStructP->RdataP->reload_pragma_no_cache =
                *((Boolean *)incoming);
            break;

        case eSENDMAIL_COMMAND:
            thePrefsStructP->RdataP->sendmail_command = (char *)incoming;
            break;

        case eEDIT_COMMAND:
            thePrefsStructP->RdataP->edit_command = (char *)incoming;
            break;

        case eXTERM_COMMAND:
            thePrefsStructP->RdataP->xterm_command = (char *)incoming;
            break;

        case eMAIL_FILTER_COMMAND:
            thePrefsStructP->RdataP->mail_filter_command = (char *)incoming;
            break;

        case ePRIVATE_ANNOTATION_DIRECTORY:
            thePrefsStructP->RdataP->private_annotation_directory =
                (char *)incoming;
            break;

        case eHOME_DOCUMENT:
            thePrefsStructP->RdataP->home_document = (char *)incoming;
            break;

        case eTMP_DIRECTORY:
            thePrefsStructP->RdataP->tmp_directory = (char *)incoming;
            break;

        case eDOCS_DIRECTORY:
            thePrefsStructP->RdataP->docs_directory = (char *)incoming;
            break;

        case eDEFAULT_FONT_CHOICE:
            thePrefsStructP->RdataP->default_font_choice = (char *)incoming;
            break;

        case eGLOBAL_HISTORY_FILE:
            thePrefsStructP->RdataP->global_history_file = (char *)incoming;
            break;

        case eHISTORY_FILE:
            thePrefsStructP->RdataP->history_file = (char *)incoming;
            break;

        case eUSE_GLOBAL_HISTORY:
            thePrefsStructP->RdataP->use_global_history =
		*((Boolean *)incoming);
            break;

        case eDEFAULT_HOTLIST_FILE:
            thePrefsStructP->RdataP->default_hotlist_file = (char *)incoming;
            break;

        case eDEFAULT_HOT_FILE:
            thePrefsStructP->RdataP->default_hot_file = (char *)incoming;
            break;

        case eADD_HOTLIST_ADDS_RBM:
            thePrefsStructP->RdataP->addHotlistAddsRBM = *((Boolean *)incoming);
            break;

        case eADD_RBM_ADDS_RBM:
            thePrefsStructP->RdataP->addRBMAddsRBM = *((Boolean *)incoming);
            break;

        case eDOCUMENTS_MENU_SPECFILE:
            thePrefsStructP->RdataP->documents_menu_specfile = (char *)incoming;
            break;

        case eCOLORS_PER_INLINED_IMAGE:
            thePrefsStructP->RdataP->colors_per_inlined_image =
                *((int *)incoming);
            break;

        case eIMAGE_CACHE_SIZE:
            thePrefsStructP->RdataP->image_cache_size = *((int *)incoming);
            break;

        case eRELOAD_RELOADS_IMAGES:
            thePrefsStructP->RdataP->reload_reloads_images =
                *((Boolean *)incoming);
            break;

        case eREVERSE_INLINED_BITMAP_COLORS:
            thePrefsStructP->RdataP->reverse_inlined_bitmap_colors =
                *((Boolean *)incoming);
            break;

        case eDELAY_IMAGE_LOADS:
            thePrefsStructP->RdataP->delay_image_loads = *((Boolean *)incoming);
            break;

        case eDEFAULT_AUTHOR_NAME:
            thePrefsStructP->RdataP->default_author_name = (char *)incoming;
            break;

        case eDEFAULT_AUTHOR_EMAIL:
            thePrefsStructP->RdataP->default_author_email = (char *)incoming;
            break;

        case eSIGNATURE:
            thePrefsStructP->RdataP->signature = (char *)incoming;
            break;

        case eMAIL_MODE:
            thePrefsStructP->RdataP->mail_mode = (char *)incoming;
            break;

        case ePRINT_COMMAND:
            thePrefsStructP->RdataP->print_command = (char *)incoming;
            break;

        case eUNCOMPRESS_COMMAND:
            thePrefsStructP->RdataP->uncompress_command = (char *)incoming;
            break;

        case eGUNZIP_COMMAND:
            thePrefsStructP->RdataP->gunzip_command = (char *)incoming;
            break;

        case eUSE_DEFAULT_EXTENSION_MAP:
            thePrefsStructP->RdataP->use_default_extension_map =
                *((Boolean *)incoming);
            break;

        case eUSE_DEFAULT_TYPE_MAP:
            thePrefsStructP->RdataP->use_default_type_map =
		*((Boolean *)incoming);
            break;

        case eGLOBAL_EXTENSION_MAP:
            thePrefsStructP->RdataP->global_extension_map = (char *)incoming;
            break;

        case ePERSONAL_EXTENSION_MAP:
            thePrefsStructP->RdataP->personal_extension_map = (char *)incoming;
            break;

        case eGLOBAL_TYPE_MAP:
            thePrefsStructP->RdataP->global_type_map = (char *)incoming;
            break;

        case ePERSONAL_TYPE_MAP:
            thePrefsStructP->RdataP->personal_type_map = (char *)incoming;
            break;

        case eTWEAK_GOPHER_TYPES:
            thePrefsStructP->RdataP->tweak_gopher_types =
                *((Boolean *)incoming);
            break;

        case ePRINT_MODE:
            thePrefsStructP->RdataP->print_mode = (char *)incoming;
            break;

        case eGUI_LAYOUT:
            thePrefsStructP->RdataP->gui_layout = (char *)incoming;
            break;

        case ePRINT_BANNERS:
            thePrefsStructP->RdataP->print_banners = *((Boolean *)incoming);
            break;

        case ePRINT_FOOTNOTES:
            thePrefsStructP->RdataP->print_footnotes = *((Boolean *)incoming);
            break;

        case ePRINT_PAPER_SIZE_US:
            thePrefsStructP->RdataP->print_us = *((Boolean *)incoming);
            break;

        case ePROXY_SPECFILE:
            thePrefsStructP->RdataP->proxy_specfile = (char *)incoming;
            break;

        case eNOPROXY_SPECFILE:
            thePrefsStructP->RdataP->noproxy_specfile = (char *)incoming;
            break;

        case eCCIPORT:
            thePrefsStructP->RdataP->cciPort = *((int *)incoming);
            break;

        case eMAX_NUM_OF_CCI_CONNECTIONS:
            thePrefsStructP->RdataP->max_num_of_cci_connections =
                *((int *)incoming);
            break;

        case eMAX_WAIS_RESPONSES:
            thePrefsStructP->RdataP->max_wais_responses = *((int *)incoming);
            break;

        case eKIOSK:
            thePrefsStructP->RdataP->kiosk = *((Boolean *)incoming);
            break;

        case eKIOSKPRINT:
            thePrefsStructP->RdataP->kioskPrint = *((Boolean *)incoming);
            break;

        case eKIOSKNOEXIT:
            thePrefsStructP->RdataP->kioskNoExit = *((Boolean *)incoming);
            break;

        case eKEEPALIVE:
            thePrefsStructP->RdataP->keepAlive = *((Boolean *)incoming);
            break;

        case eFTP_TIMEOUT_VAL:
            thePrefsStructP->RdataP->ftp_timeout_val = *((int *)incoming);
            break;

        case eENABLE_TABLES:
            thePrefsStructP->RdataP->enable_tables = *((Boolean *)incoming);
            break;

        case eDEFAULT_WIDTH:
            thePrefsStructP->RdataP->default_width = *((int *)incoming);
            break;

        case eDEFAULT_HEIGHT:
            thePrefsStructP->RdataP->default_height = *((int *)incoming);
            break;

        case eAUTO_PLACE_WINDOWS:
            thePrefsStructP->RdataP->auto_place_windows =
                *((Boolean *)incoming);
            break;

        case eINITIAL_WINDOW_ICONIC:
            thePrefsStructP->RdataP->initial_window_iconic =
                *((Boolean *)incoming);
            break;

        case eTITLEISWINDOWTITLE:
            thePrefsStructP->RdataP->titleIsWindowTitle =
		*((Boolean *)incoming);
            break;

        case eTWIRLING_TRANSFER_ICON:
            thePrefsStructP->RdataP->twirling_transfer_icon =
                *((Boolean *)incoming);
            break;

        case eSECURITYICON:
            thePrefsStructP->RdataP->securityIcon = *((Boolean *)incoming);
            break;

        case eTWIRL_INCREMENT:
            thePrefsStructP->RdataP->twirl_increment = *((int *)incoming);
            break;

        case eSAVE_MODE:
            thePrefsStructP->RdataP->save_mode = (char *)incoming;
            break;

        case eFULL_HOSTNAME:
            thePrefsStructP->RdataP->full_hostname = (char *)incoming;
            break;

        case eLOAD_LOCAL_FILE:
            thePrefsStructP->RdataP->load_local_file = *((int *)incoming);
            break;

        case eEDIT_COMMAND_USE_XTERM:
            thePrefsStructP->RdataP->edit_command_use_xterm =
                *((Boolean *)incoming);
            break;

        case eCONFIRM_EXIT:
            thePrefsStructP->RdataP->confirm_exit = *((Boolean *)incoming);
            break;

        case eDEFAULT_FANCY_SELECTIONS:
            thePrefsStructP->RdataP->default_fancy_selections =
                *((Boolean *)incoming);
            break;

        case eCATCH_PRIOR_AND_NEXT:
            thePrefsStructP->RdataP->catch_prior_and_next =
                *((Boolean *)incoming);
            break;

        case eSIMPLE_INTERFACE:
            thePrefsStructP->RdataP->simple_interface = *((Boolean *)incoming);
            break;

        case ePROTECT_ME_FROM_MYSELF:
            thePrefsStructP->RdataP->protect_me_from_myself =
                *((Boolean *)incoming);
            break;

        case eGETHOSTBYNAME_IS_EVIL:
            thePrefsStructP->RdataP->gethostbyname_is_evil =
                *((Boolean *)incoming);
            break;

#ifdef __sgi
        case eDEBUGGING_MALLOC:
            thePrefsStructP->RdataP->debugging_malloc = *((Boolean *)incoming);
            break;

#endif
        case eUSEAFSKLOG:
            thePrefsStructP->RdataP->useAFSKlog = *((Boolean *)incoming);
            break;

	/* New in 2.7 */
        case eSEND_REFERER:
            thePrefsStructP->RdataP->sendReferer = *((Boolean *)incoming);
            break;

        case eSEND_AGENT:
            thePrefsStructP->RdataP->sendAgent = *((Boolean *)incoming);
            break;

        case eEXPAND_URLS:
            thePrefsStructP->RdataP->expandUrls = *((Boolean *)incoming);
            break;

        case eEXPAND_URLS_WITH_NAME:
            thePrefsStructP->RdataP->expandUrlsWithName =
		*((Boolean *)incoming);
            break;

        case eDEFAULT_PROTOCOL:
            thePrefsStructP->RdataP->defaultProtocol = (char *)incoming;
            break;

        case eMETER_FOREGROUND:
            thePrefsStructP->RdataP->meterForeground = (char *)incoming;
            break;

        case eMETER_BACKGROUND:
            thePrefsStructP->RdataP->meterBackground = (char *)incoming;
            break;

        case eMETER_FONT_FOREGROUND:
            thePrefsStructP->RdataP->meterFontForeground = (char *)incoming;
            break;

        case eMETER_FONT_BACKGROUND:
            thePrefsStructP->RdataP->meterFontBackground = (char *)incoming;
            break;

        case eMETER:
            thePrefsStructP->RdataP->use_meter = *((Boolean *)incoming);
            break;

        case eBACKUP_FILES:
            thePrefsStructP->RdataP->backup_files = *((Boolean *)incoming);
            break;

        case ePIX_BASENAME:
            thePrefsStructP->RdataP->pix_basename = (char *)incoming;
            break;

        case ePIX_COUNT:
            thePrefsStructP->RdataP->pix_count = *((int *)incoming);
            break;

        case eACCEPT_LANGUAGE_STR:
            thePrefsStructP->RdataP->acceptlanguage_str = (char *)incoming;
            break;

        case eFTP_REDIAL:
            thePrefsStructP->RdataP->ftpRedial = *((int *)incoming);
            break;

        case eFTP_REDIAL_SLEEP:
            thePrefsStructP->RdataP->ftpRedialSleep = *((int *)incoming);
            break;

        case eFTP_FILENAME_LENGTH:
            thePrefsStructP->RdataP->ftpFilenameLength = *((int *)incoming);
            break;

        case eFTP_ELLIPSIS_LENGTH:
            thePrefsStructP->RdataP->ftpEllipsisLength = *((int *)incoming);
            break;

        case eFTP_ELLIPSIS_MODE:
            thePrefsStructP->RdataP->ftpEllipsisMode = *((int *)incoming);
            break;

        case eTITLE_ISWINDOW_TITLE:
            thePrefsStructP->RdataP->titleIsWindowTitle =
                *((Boolean *)incoming);
            break;

        case eUSE_SCREEN_GAMMA:
            thePrefsStructP->RdataP->useScreenGamma = *((Boolean *)incoming);
            break;

        case eSCREEN_GAMMA:
            thePrefsStructP->RdataP->screen_gamma = *((float *)incoming);
            break;

        case eDISABLEMIDDLEBUTTON:
            thePrefsStructP->RdataP->disableMiddleButton =
                *((Boolean *)incoming);
            break;

        case eHTTPTRACE:
            thePrefsStructP->RdataP->httpTrace = *((Boolean *)incoming);
            break;

        case eWWW2TRACE:
            thePrefsStructP->RdataP->www2Trace = *((Boolean *)incoming);
            break;

        case eHTMLWTRACE:
            thePrefsStructP->RdataP->htmlwTrace = *((Boolean *)incoming);
            break;

        case eCCITRACE:
            thePrefsStructP->RdataP->cciTrace = *((Boolean *)incoming);
            break;

        case eSRCTRACE:
            thePrefsStructP->RdataP->srcTrace = *((Boolean *)incoming);
            break;

        case eCACHETRACE:
            thePrefsStructP->RdataP->cacheTrace = *((Boolean *)incoming);
            break;

        case eNUTTRACE:
            thePrefsStructP->RdataP->nutTrace = *((Boolean *)incoming);
            break;

        case eTABLETRACE:
            thePrefsStructP->RdataP->tableTrace = *((Boolean *)incoming);
            break;

        case eANIMATEBUSYICON:
            thePrefsStructP->RdataP->animateBusyIcon = *((Boolean *)incoming);
            break;

        case eIMAGEVIEWINTERNAL:
            thePrefsStructP->RdataP->imageViewInternal = *((Boolean *)incoming);
            break;

        case eSPLASHSCREEN:
            thePrefsStructP->RdataP->splashScreen = *((Boolean *)incoming);
            break;

        case eINSTALL_COLORMAP:
            thePrefsStructP->RdataP->instamap = *((Boolean *)incoming);
            break;

        case eURLEXPIRED:
            thePrefsStructP->RdataP->urlExpired = *((Boolean *)incoming);
            break;

        case eFRAME_SUPPORT:
            thePrefsStructP->RdataP->frame_support = *((Boolean *)incoming);
            break;

        case ePOPUPCASCADEMAPPINGDELAY:
            thePrefsStructP->RdataP->popupCascadeMappingDelay =
                *((int *)incoming);
	    break;

	case eUSETHREADVIEW:
	    thePrefsStructP->RdataP->newsConfigView = *((int *)incoming);
	    break;
      
        case eSHOWREADGROUPS:
	    thePrefsStructP->RdataP->newsShowReadGroups = *((int *)incoming);
	    break;

        case eNOTHREADJUMPING:
	    thePrefsStructP->RdataP->newsNoThreadJumping = *((int *)incoming);
            break;

        case eSHOWALLGROUPS:
	    thePrefsStructP->RdataP->newsShowAllGroups = *((int *)incoming);
            break;
  
        case eSHOWALLARTICLES:
	    thePrefsStructP->RdataP->newsShowAllArticles = *((int *)incoming);
            break;

        case eUSEBACKGROUNDFLUSH:
	    thePrefsStructP->RdataP->newsUseBackgroundFlush =
		*((int *)incoming);
            break;

        case eBACKGROUNDFLUSHTIME:
	    thePrefsStructP->RdataP->newsBackgroundFlushTime =
		*((int *)incoming);
            break;

        case eCLIPPING:
            thePrefsStructP->RdataP->clipping = *((Boolean *)incoming);
            break;

        case eMAX_CLIP_TRANSITIONS:
            thePrefsStructP->RdataP->max_clip_transitions = *((int *)incoming);
            break;

        case eUSE_LONG_TEXT_NAMES:
            thePrefsStructP->RdataP->long_text_names = *((Boolean *)incoming);
            break;

        case eTOOLBAR_LAYOUT:
            thePrefsStructP->RdataP->toolbar_layout = (char *)incoming;
            break;

        case eNEXTISUNREAD:
	    thePrefsStructP->RdataP->newsNextIsUnread = *((int *)incoming);
            break;

        case ePREVISUNREAD:
	    thePrefsStructP->RdataP->newsPrevIsUnread = *((int *)incoming);
            break;

        case eUSENEWSRC:
	    thePrefsStructP->RdataP->newsUseNewsrc = *((int *)incoming);
            break;

        case eNEWSRCPREFIX:
	    thePrefsStructP->RdataP->newsNewsrcPrefix = (char *)incoming;
            break;

        case eNEWSSUBJECTWIDTH:
	    thePrefsStructP->RdataP->newsSubjectWidth = *((int *)incoming);
            break;

        case eNEWSAUTHORWIDTH:
	    thePrefsStructP->RdataP->newsAuthorWidth = *((int *)incoming);
            break;

        case eFOCUS_FOLLOWS_MOUSE:
            thePrefsStructP->RdataP->focusFollowsMouse = *((Boolean *)incoming);
            break;

        case eSESSION_HISTORY_ON_RBM:
            thePrefsStructP->RdataP->sessionHistoryOnRBM =
                *((Boolean *)incoming);
            break;  

        case eNUMBER_OF_ITEMS_IN_RBM_HISTORY:
	    thePrefsStructP->RdataP->numberOfItemsInRBMHistory =
		*((int *)incoming);
	    break;

        case eHOTLIST_ON_RBM:
            thePrefsStructP->RdataP->hotlistOnRBM = *((Boolean *)incoming);
            break;

        case eUSESHORTNEWSRC:
	    thePrefsStructP->RdataP->newsUseShortNewsrc = *((int *)incoming);
            break;

	/* 2.7b6 */
	case eKIOSKPROTOCOLS:
	    thePrefsStructP->RdataP->kioskProtocols = (char *)incoming;
            break;

	case eBODYCOLORS:
            thePrefsStructP->RdataP->bodyColors = *((Boolean *)incoming);
            break;

	case eBODYIMAGES:
            thePrefsStructP->RdataP->bodyImages = *((Boolean *)incoming);
            break;

	case eDEFAULTUNDERLINES:
	    thePrefsStructP->RdataP->defaultUnderlines = (char *)incoming;
            break;

	case eFTP_BINARY_MODE:
            thePrefsStructP->RdataP->ftp_binary_mode = *((Boolean *)incoming);
            break;

        case eVMS_MAIL_PREFIX:
            thePrefsStructP->RdataP->vms_mail_prefix = (char *)incoming;
            break;

        case eBACKUPFILEVERSIONS:
            thePrefsStructP->RdataP->backupFileVersions = *((int *)incoming);
            break;

	case eFONTCOLORS:
            thePrefsStructP->RdataP->fontColors = *((Boolean *)incoming);
            break;

	case ePROGRESSIVE_DISPLAY:
            thePrefsStructP->RdataP->progressive_display =
                *((Boolean *)incoming);
            break;

	case eFONTSIZES:
            thePrefsStructP->RdataP->fontSizes = *((Boolean *)incoming);
            break;

	case eFONTBASESIZE:
            thePrefsStructP->RdataP->fontBaseSize = *((int *)incoming);
            break;

        case eTRACK_TARGET_ANCHORS:
            thePrefsStructP->RdataP->track_target_anchors =
                *((Boolean *)incoming);
            break;

        case eDEBUG_MENU:
            thePrefsStructP->RdataP->debug_menu = *((Boolean *)incoming);
            break;

        case eREPORTBUGS:
            thePrefsStructP->RdataP->reportBugs = *((Boolean *)incoming);
            break;

        case eIMAGE_ANIMATION:
            thePrefsStructP->RdataP->image_animation = *((Boolean *)incoming);
            break;

	case eMIN_ANIMATION_DELAY:
            thePrefsStructP->RdataP->min_animation_delay = *((int *)incoming);
            break;

        case eREFRESHTRACE:
            thePrefsStructP->RdataP->refreshTrace = *((Boolean *)incoming);
            break;

        case eREFRESH_URL:
            thePrefsStructP->RdataP->refresh_URL = *((Boolean *)incoming);
            break;

        case eBROWSER_SAFE_COLORS:
            thePrefsStructP->RdataP->browser_safe_colors =
		*((Boolean *)incoming);
            break;

        case eBLINKING_TEXT:
            thePrefsStructP->RdataP->blinking_text = *((Boolean *)incoming);
            break;

	case eBLINK_TIME:
            thePrefsStructP->RdataP->blink_time = *((int *)incoming);
            break;

        case eCOOKIES:
            thePrefsStructP->RdataP->cookies = *((Boolean *)incoming);
            break;

        case eACCEPT_ALL_COOKIES:
            thePrefsStructP->RdataP->accept_all_cookies =
		*((Boolean *)incoming);
            break;

	case eMAXPIXMAPWIDTH:
            thePrefsStructP->RdataP->maxPixmapWidth = *((int *)incoming);
            break;

	case eMAXPIXMAPHEIGHT:
            thePrefsStructP->RdataP->maxPixmapHeight = *((int *)incoming);
            break;

        case eUSE_COOKIE_FILE:
            thePrefsStructP->RdataP->use_cookie_file = *((Boolean *)incoming);
            break;

        case eCOOKIE_FILE:
            thePrefsStructP->RdataP->cookie_file = (char *)incoming;
            break;

        case eIMAGEDELAY_FILE:
            thePrefsStructP->RdataP->imagedelay_file = (char *)incoming;
            break;

        case eBROWSERSAFECOLORS_IF_TRUECOLOR:
            thePrefsStructP->RdataP->BSColors_if_Truecolor =
		*((Boolean *)incoming);
	    break;

        case eHOTKEYS:
            thePrefsStructP->RdataP->hotkeys = *((Boolean *)incoming);
	    break;

        case eINVALID_COOKIE_PROMPT:
            thePrefsStructP->RdataP->invalid_cookie_prompt =
		*((Boolean *)incoming);
	    break;

        case eMAX_COOKIES:
            thePrefsStructP->RdataP->max_cookies = *((int *)incoming);
	    break;

        case eCOOKIE_DOMAIN_LIMIT:
            thePrefsStructP->RdataP->cookie_domain_limit = *((int *)incoming);
	    break;

        case ePERM_FILE:
            thePrefsStructP->RdataP->perm_file = (char *)incoming;
            break;

        case eFORM_BUTTON_BACKGROUND:
            thePrefsStructP->RdataP->form_button_background = (char *)incoming;
            break;

        case eVERIFY_SSL_CERTIFICATES:
            thePrefsStructP->RdataP->verify_ssl_certificates =
		*((Boolean *)incoming);
	    break;

        case eHOTLIST_MENU_HEIGHT:
            thePrefsStructP->RdataP->hotlist_menu_height = *((int *)incoming);
	    break;

        case eHOTLIST_MENU_WIDTH:
            thePrefsStructP->RdataP->hotlist_menu_width = *((int *)incoming);
	    break;

        case eMARKUP_MEMORY_PREALLOCATION:
            thePrefsStructP->RdataP->markup_memory_preallocation =
		*((int *)incoming);
	    break;

        case eELEMENT_MEMORY_PREALLOCATION:
            thePrefsStructP->RdataP->element_memory_preallocation =
		*((int *)incoming);
	    break;

        case eCOOKIETRACE:
            thePrefsStructP->RdataP->cookieTrace = *((Boolean *)incoming);
            break;

        case eCLUE_HELP:
            thePrefsStructP->RdataP->clue_help = *((Boolean *)incoming);
            break;

        case eCLUE_FOREGROUND:
            thePrefsStructP->RdataP->clueForeground = (char *)incoming;
            break;

        case eCLUE_BACKGROUND:
            thePrefsStructP->RdataP->clueBackground = (char *)incoming;
            break;

        case eCLUE_POPUP_DELAY:
            thePrefsStructP->RdataP->clueDelay = *((int *)incoming);
	    break;

        case eCLUE_POPDOWN_DELAY:
            thePrefsStructP->RdataP->clueDownDelay = *((int *)incoming);
	    break;

        case eCLUE_FONT:
            thePrefsStructP->RdataP->clueFont = (char *)incoming;
            break;

        case eCLUE_OVAL:
            thePrefsStructP->RdataP->clue_oval = *((Boolean *)incoming);
            break;

        case eCLUE_ROUNDED:
            thePrefsStructP->RdataP->clue_rounded = *((Boolean *)incoming);
            break;

        case ePRINT_DUPLEX:
            thePrefsStructP->RdataP->print_duplex = *((Boolean *)incoming);
            break;

        case eMENUBAR_TEAROFF:
            thePrefsStructP->RdataP->menubar_tearoff = *((Boolean *)incoming);
            break;

        case eTIFF_ERROR_MESSAGES:
            thePrefsStructP->RdataP->tiff_error_messages =
		*((Boolean *)incoming);
            break;

        case ePNG_ERROR_MESSAGES:
            thePrefsStructP->RdataP->png_error_messages =
		*((Boolean *)incoming);
            break;

        case eJPEG_ERROR_MESSAGES:
            thePrefsStructP->RdataP->jpeg_error_messages =
		*((Boolean *)incoming);
            break;

        case eDETACHED_TOOLBAR:
            thePrefsStructP->RdataP->detached_toolbar =	*((Boolean *)incoming);
            break;

        case eDETACHED_TOOLBAR_VERTICAL:
            thePrefsStructP->RdataP->detached_toolbar_vertical =
		*((Boolean *)incoming);
            break;

        case eTEXT_TOOLBAR:
            thePrefsStructP->RdataP->text_toolbar = *((Boolean *)incoming);
            break;

        case eENCRYPTION_ICON:
            thePrefsStructP->RdataP->encryption_icon = *((Boolean *)incoming);
            break;

        case ePRESENTATION_MODE_ON_RBM:
            thePrefsStructP->RdataP->presentationModeOnRBM =
		*((Boolean *)incoming);
            break;

        case eMULTIPLE_IMAGE_LOAD:
            thePrefsStructP->RdataP->multiple_image_load =
		*((Boolean *)incoming);
            break;

        case eMULTIPLE_IMAGE_LIMIT:
            thePrefsStructP->RdataP->multiple_image_limit = *((int *)incoming);
            break;

        case eJPEG2000_ERROR_MESSAGES:
            thePrefsStructP->RdataP->jpeg2000_error_messages =
		*((Boolean *)incoming);
            break;

        default:
            fprintf(stderr, "Error: tried to set nonexistant preference\n");
    }
}

/****************************************************************************
 ****************************************************************************
 *                         Preference Dialog functions
 *
 ****************************************************************************
 ***************************************************************************/

#ifndef VMS
/****************************************************************************
   Function: mo_preferences_dialog(mo_window *win)
   Desc:     Displays the preferences dialog
 ***************************************************************************/
void mo_preferences_dialog(mo_window *win)
{

}
#endif  /* VMS, Useless but gets in way, GEC */
