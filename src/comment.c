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

/* Copyright (C) 2003, 2004, 2005, 2006, 2007 - The VMS Mosaic Project */

/*
 * Written By:  Scott Powers
 * Date:        April 26, 1995
 * Purpose:     Creates a comment card for Mosaic.  Submittal is via form
 *		and works in conjunction with a "C" cgi-bin program.
 */

#include "../config.h"
#include "mosaic.h"
#include "gui.h"
#define _COMMENT_H
#include "comment.h"
#undef _COMMENT_H
#include <stdio.h>

#ifndef VMS
#include <pwd.h>
#include <sys/utsname.h>
#else
#include "vms_pwd.h"
#include <lib$routines.h>

/* This .h file should not be referenced in DESCRIP.MMS for this module */
/* because the SSL dummy .h file will force a recompile when needed. */
#ifdef __GNUC__
#include MOSAIC_BUILT
#else
#include "mosaic_built"
#endif
#endif

extern char *built_time;
extern char *ident_ver;

int do_comment = 0;

#ifndef VMS
extern struct utsname mo_uname;
#endif

static int DumpHtml(char *htmlname);


static char *MakeFilename()
{
	char *hptr, *fname;
	char home[256];
#ifndef VMS
	struct passwd *pwdent;
#endif

	/*
	 * Try the HOME environment variable, then the password file, and
	 *   finally give up.
	 */
	if (!(hptr = getenv("HOME"))) {
#ifndef VMS
		if (!(pwdent = getpwuid(getuid()))) {
			return(NULL);
		} else {
			strcpy(home, pwdent->pw_dir);
		}
#else
		return(NULL);
#endif
	} else {
		strcpy(home, hptr);
	}
 	fname = (char *)malloc(strlen(home) + strlen(COMMENT_CARD_FILENAME) +
			       strlen(MO_VERSION_STRING) + 5);
#ifndef VMS
	sprintf(fname, "%s/%s%s", home, COMMENT_CARD_FILENAME,
		MO_VERSION_STRING);
#else
	sprintf(fname, "%s%s%s", home, COMMENT_CARD_FILENAME,
		MO_VERSION_STRING2);
#endif
	return(fname);
}


static void InitCard(char *fname)
{
	FILE *fp;
	long num[10];

#ifdef VMS
	/*
	 * Make sure we start a new file like UNIX
	 */
        remove(fname);
#endif
	if (!(fp = fopen(fname, "w")))
		return;
	num[0] = 1;
	fwrite(num, sizeof(long), 2, fp);
                               
	fclose(fp);
	return;
}


static void PutCardCount(long *num, char *fname)
{
	FILE *fp;

#ifndef VMS
	if (!(fp = fopen(fname, "w")))
#else
	if (!(fp = fopen(fname, "r+")))
#endif
		return;

	fwrite(num, sizeof(long), 2, fp);

	fclose(fp);
	return;
}


void CommentCard(mo_window *win)
{
	long num[10];
	int n;
	char *fname;

	if (!win && !(win = mo_next_window(NULL)))
		return;

	for (n = 0; n < 10; n++)
		num[n] = 0;

	if (!do_comment) {
		if (!(fname = MakeFilename()))
			return;
 		num[0] = GetCardCount(fname);
		num[0]++;
	}

#ifndef PRERELEASE
	if ((num[0] == COMMENT_TIME) || do_comment) {
		char *htmlname = (char *)malloc(sizeof(char) * (L_tmpnam + 5));
		char *htmlurl;

		if (!tmpnam(htmlname)) {
			if (!do_comment)
				free(fname);
			free(htmlname);
			return;
		}
		strcat(htmlname, ".html");
		if (!DumpHtml(htmlname)) {
			if (!do_comment)
				free(fname);
			free(htmlname);
			return;
		}
		htmlurl = (char *)malloc(strlen(htmlname) +
					 strlen("file://localhost") + 10);
#ifndef VMS
		sprintf(htmlurl, "file://localhost%s", htmlname);
#else
		sprintf(htmlurl, "file://localhost/%s", htmlname);
#endif
		mo_open_another_window(win, htmlurl, NULL, NULL);
#ifdef VMS
		remove(htmlname);
#endif
		free(htmlurl);
		free(htmlname);
	}
#endif
	if (!do_comment) {
		PutCardCount(num, fname);
		free(fname);
	}
	return;
}


static int DumpHtml(char *htmlname)
{
	FILE *fp;
#ifdef VMS
#define SYI$_HW_NAME 4362
#define SYI$_VERSION 4096
	int syi_hw_name = SYI$_HW_NAME;
	int syi_version = SYI$_VERSION;
	char hardware[32], VMS_version[16];
	char *cp;
	int status;
	unsigned short l_hardware, l_version;

	struct  dsc$descriptor_s {
	  unsigned short  dsc$w_length;
	  unsigned char   dsc$b_dtype;
	  unsigned char   dsc$b_class;
	  char            *dsc$a_pointer;
	} hardware_desc = { sizeof(hardware), 14, 1, NULL },
	  VMS_version_desc = { sizeof(VMS_version), 14, 1, NULL };

	hardware_desc.dsc$a_pointer = hardware;
	VMS_version_desc.dsc$a_pointer = VMS_version;
#endif

	if (!(fp = fopen(htmlname, "w")))
		return(0);

	fprintf(fp, "%s\n", comment_card_html_top);
#ifndef VMS
	fprintf(fp, " Mosaic Compiled OS: %s<br>\n", MO_COMMENT_OS);
	fprintf(fp, " <input type=\"hidden\" name=\"os\" value=\"%s\">\n",
		MO_COMMENT_OS);
	fprintf(fp, " Sysname: %s<br>\n", mo_uname.sysname);
	fprintf(fp, " <input type=\"hidden\" name=\"sysname\" value=\"%s\">\n",
		mo_uname.sysname);
	fprintf(fp, " Release: %s<br>\n", mo_uname.release);
	fprintf(fp, " <input type=\"hidden\" name=\"release\" value=\"%s\">\n",
		mo_uname.release);
#else
	status = lib$getsyi((void *)&syi_hw_name, 0, &hardware_desc,
			    &l_hardware, 0, 0);
        status = lib$getsyi((void *)&syi_version, 0, &VMS_version_desc,
			    &l_version, 0, 0);
	hardware[l_hardware] = '\0';
        VMS_version[l_version] = '\0';
        for (cp = &VMS_version[l_version - 1]; VMS_version; cp--) {
        	if (*cp != ' ')
			break;
        	*cp = '\0';
        }
	fprintf(fp, "System version %s running on a %s.<br>\n", VMS_version,
		hardware);
	fprintf(fp,
		"<input type=\"hidden\" name=\"release\" value=\"%s %s\">\n",
		MO_MACHINE_TYPE, VMS_version);
	fprintf(fp, "<input type=\"hidden\" name=\"hardware\" value=\"%s\">\n",
		hardware);
	fprintf(fp, "<input type=\"hidden\" name=\"TCP/IP\" value=\"");
#ifdef MULTINET
	fprintf(fp, "MultiNet\">TCP/IP: MultiNet<br>\n");
#elif WIN_TCP
	fprintf(fp, "Pathway\">TCP/IP: Pathway<br>\n");
#elif SOCKETSHR
	fprintf(fp, "SOCKETSHR/NETLIB\">TCP/IP: SOCKETSHR/NETLIB<br>\n");
#else
	fprintf(fp, "UCX (or UCX compatible)\">\n");
	fprintf(fp, "TCP/IP: TCP/IP Services or UCX compatible<br>\n");
#endif /* TCP/IP flavour */

	fprintf(fp, "Your Mosaic executable was generated using Motif \n");
	fprintf(fp, "<input type=\"hidden\" name=\"motif\" value=\"");
#ifdef MOTIF1_6
	fprintf(fp, "1.6\">1.6<br>\n");
#else

#ifdef MOTIF1_5
	fprintf(fp, "1.5\">1.5<br>\n");
#else

#ifdef MOTIF1_4
#ifdef MOTIF1_41
	fprintf(fp, "1.4-1\">1.4-1<br>\n");
#else
	fprintf(fp, "1.4\">1.4<br>\n");
#endif

#else

#ifdef MOTIF1_3
#ifdef MOTIF1_30
	fprintf(fp, "1.3-0\">1.3-0<br>\n");
#else
#ifdef MOTIF1_31
	fprintf(fp, "1.3-1\">1.3-1<br>\n");
#else
	fprintf(fp, "1.3-x\">1.3-x<br>\n");
#endif
#endif

#else

#ifdef MOTIF1_26
	fprintf(fp, "1.2-6\">1.2-6<br>\n");
#else
#ifdef MOTIF1_25
	fprintf(fp, "1.2-5\">1.2-5<br>\n");
#else
#ifdef MOTIF1_24
	fprintf(fp, "1.2-4\">1.2-4<br>\n");
#else
#ifdef MOTIF1_23
#if (MOTIF1_23 == 7)
	fprintf(fp, "1.2-3 ECO 7\">1.2-3 ECO 7<br>\n");
#else
	fprintf(fp, "1.2-3\">1.2-3<br>\n");
#endif
#else
#ifdef MOTIF1_2
	fprintf(fp, "1.2\">1.2<br>\n");
#else
	fprintf(fp, "1.1\">1.1<br>\n");
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
        fprintf(fp, "and was built on %s with image Ident %s<br>\n",
		built_time, ident_ver);
	fprintf(fp, "<input type=\"hidden\" name=\"built\" value=\"%s\">\n",
		built_time);
	fprintf(fp, "<input type=\"hidden\" name=\"ident\" value=\"%s\">\n",
		ident_ver);
#if defined(VAXC) && !defined(__DECC)
        fprintf(fp, "using VAX C. \n");
	fprintf(fp, "<input type=\"hidden\" name=\"C\" value=\"VAX C\">\n");
#else
#ifdef __GNUC__
        fprintf(fp, "using GNU C. \n");
	fprintf(fp, "<input type=\"hidden\" name=\"C\" value=\"GNU C\">\n");
#else
        fprintf(fp, "using DEC C. \n");
	fprintf(fp, "<input type=\"hidden\" name=\"C\" value=\"DEC C\">\n");
#endif
#endif
#ifdef HAVE_SSL
#ifdef HAVE_HPSSL
	fprintf(fp, "It was linked with HP SSL.<br>\n");
	fprintf(fp, "<input type=\"hidden\" name=\"OpenSSL\" value=\"HP\">\n");
#else
	fprintf(fp, "It was linked with OpenSSL.<br>\n");
	fprintf(fp, "<input type=\"hidden\" name=\"OpenSSL\" value=\"Yes\">\n");
#endif
#else
        fprintf(fp, "It was not linked with OpenSSL.<br>\n");
	fprintf(fp, "<input type=\"hidden\" name=\"OpenSSL\" value=\"No\">\n");
#endif
#endif /* VMS, GEC */
	fprintf(fp, "%s\n", comment_card_html_bot);
	fclose(fp);

	return(1);
}


long GetCardCount(char *fname)
{
	FILE *fp;
	long num[10];
	int freeit = 0;

	if (!fname) {
	    fname = MakeFilename();
	    freeit = 1;
	}
	if (!(fp = fopen(fname, "r"))) {
		InitCard(fname);
		if (freeit)
			free(fname);
		return((long)0);
	}
	if (freeit)
		free(fname);
	fseek(fp, 0L, SEEK_SET);
	fread(num, sizeof(long), 2, fp);

	fclose(fp);
	return(num[0]);
}
