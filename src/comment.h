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

#define COMMENT_CARD_FILENAME ".mosaic-cc-"
#define COMMENT_TIME 5

#ifndef VMS
#define MO_COMMENT_OS "Not Supported"  /* Not used on VMS */
#endif

extern void CommentCard(mo_window *win);
extern long GetCardCount(char *fname);

#ifdef _COMMENT_H

char *comment_card_html_top = "<title>Comment Card for VMS Mosaic 4.2</title>\n\
<h1 align=center>\n\
Please Help Keep VMS Mosaic Alive!\n\
</h1><hr><h2>\n\
Thank you for using VMS Mosaic!  I would appreciate\n\
your taking the time to answer these few questions.\n\
<p align=right>\n\
George Cook&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n\
</h2>\n\
<hr>\n\
<form method=\"POST\" action=\"http://wvnvms.wvnet.edu/htbin/cgi-mailto-mosaic/mosaic/Mosaic_4.2_Comment_Card\">\n\
<h3>\n\
<ul>\n\
<li>\n\
If you do not like surveys or you have already\n\
completed this survey, please press this button,\n\
<input type=\"submit\" value=\"Just Count Me\" name=\"countme\">,\n\
to be counted. Pushing the button will send the following information about\n\
your system to be used in my statistics:\n\
<p>\n";
 
char *comment_card_html_bot = "</p>\n\
</li>\n\
<br>\n\
<li>\n\
If you do not want to complete the survey,\n\
just close this window.\n\
</li>\n\
<br>\n\
<li>\n\
Otherwise, please proceed!\n\
</li>\n\
</ul>\n\
</h3>\n\
<hr>\n\
<p>\n\
How long have you been using VMS Mosaic?\n\
<br>\n\
<select name=\"usage\">\n\
	<option value=\"no comment\" selected>\n\
		No Comment\n\
	<option value=\"never\">\n\
		Never\n\
	<option value=\"lt 1 mon\">\n\
		Less Than 1 Month\n\
	<option value=\"1-6 mon\">\n\
		1 - 6 Months\n\
	<option value=\"6 mon-1 yr\">\n\
		6 Months to a Year\n\
	<option value=\"1-2 yrs\">\n\
		1 - 2 Years\n\
	<option value=\"gt 2 yrs\">\n\
		More Than 2 Years\n\
</select>\n\
<p>\n\
Do you also use Mozilla on VMS platforms?\n\
<br>\n\
<select name=\"Mozilla\">\n\
	<option value=\"no comment\" selected>\n\
		No Comment\n\
	<option value=\"yes\">\n\
		Yes, I Do\n\
	<option value=\"no\">\n\
		No, I Do Not\n\
</select>\n\
<input type=hidden name=\"mosaic_list\" value=\"no comment\">\n\
<input type=hidden name=\"useful\" value=\"no comment\">\n\
<input type=hidden name=\"useful_feedback\" value=\"no comment\">\n\
<p>\n\
Please enter the improvement\n\
you would most like to see in VMS Mosaic:\n\
<br>\n\
<textarea name=\"improvement\" rows=3 cols=60>\n\
</textarea>\n\
<p>\n\
Please enter your email address (optional):\n\
<br>\n\
<textarea name=\"email address\" rows=1 cols=60>\n\
</textarea>\n\
<p>\n\
Other comments and/or suggestions are welcomed:\n\
<br>\n\
<textarea name=\"comments_feedback\" rows=5 cols=60>\n\
</textarea>\n\
<p>\n\
When you are done, please press this button:\n\
<br>\n\
<input type=\"submit\" value=\"Submit Comment Card for VMS Mosaic\" \
name=\"submitme\">\n\
</form>\n\n";

#endif
