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

/* Copyright (C) 2004, 2005, 2006, 2007, 2008 - The VMS Mosaic Project */

/* 
 * Created: Wed Apr 10 17:41:00 CDT 1996
 * Author: Dan Pape
 *
 */

#ifndef __GUI_H__
#define __GUI_H__

void mo_process_external_directive(char *directive, char *url);

mo_window *mo_next_window(mo_window *);
mo_window *mo_fetch_window_by_id(int);
char *mo_assemble_help_url(char *);
mo_status mo_redisplay_window(mo_window *);
void mo_set_current_cached_win(mo_window *);
mo_status mo_delete_window(mo_window *);
mo_window *mo_duplicate_window(mo_window *);
mo_window *mo_open_another_window(mo_window *, char *, char *, char *);
mo_status mo_open_initial_window(void);
void mo_gui_notify_progress(char *);
int mo_gui_check_icon(int);
void mo_gui_clear_icon(void);
void mo_gui_done_with_icon(void);
void mo_gui_update_meter(int level, char *text);
int anchor_visited_predicate(Widget, char *);
void mo_stop_it(mo_window *);
void mo_stop_animations(mo_window *win, int check_drawing);
void mo_presentation_mode(mo_window *win);

void mo_gui_check_win_security_icon(int type, mo_window *win);

void mo_do_gui(int, char **);
void mo_switch_mode(mo_window *);
void mo_flush_passwd_cache(mo_window *);
void pub_anchor_ltd(char *);
void mo_delete_frames(mo_window *);

extern void mo_tool_state(mo_toolbar *t, int state, int index);

#endif
