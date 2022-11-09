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

/* SOCKS mods by:
 * Ying-Da Lee, <ylee@syl.dl.nec.com>
 * NEC Systems Laboratory
 * C&C Software Technology Center
 */
#include "../config.h"
#include "../libwww2/htcookie.h"
#include "mosaic.h"
#include "main.h"
#include "gui.h"
#include "pan.h"
#include "child.h"
#include "mo-www.h"
#include "newsrc.h"
#include "hotlist.h"
#include "globalhist.h"
#ifdef CCI
#include "cciBindings2.h"
#endif

#include <signal.h>
#ifndef VMS
#include <sys/utsname.h>
#else
#ifdef __GNUC__
#include MOSAIC_BUILT
#else
#include "mosaic_built"
#endif
#include <iodef.h>
#include <descrip.h>
#include <errno.h>
#include <starlet.h>
#include <lib$routines.h>

#define DVI$_DEVNAM 32
#define LNM$_STRING 2

char *built_time = BUILD_TIME;
char *ident_ver = IDENT_VER;
static int has_mbx = 0;
static short mbx_channel;
static char mbx_buf[200];
unsigned long mbx_event_flag = 23;  /* Must be flag in first cluster ( < 32) */
unsigned short mbx_iosb[4];
#endif /* VMS, BSN, GEC */

#ifndef VMS
char *userPath = NULL;
#endif

extern mo_root_hotlist *default_hotlist;

/* Normal exit.  Save cookies, etc. */
void mo_exit(void)
{
  if (default_hotlist->modified)
      mo_write_default_hotlist();
  if (get_pref_boolean(eUSE_COOKIE_FILE))
      HTStoreCookies(get_pref_string(eCOOKIE_FILE),
		     get_pref_string(ePERM_FILE));
  newsrc_kill();
  if (get_pref_boolean(eUSE_GLOBAL_HISTORY))
      mo_write_global_history();
  mo_write_pan_list();

  /* preferences_armegeddon(); */

  exit(0);
}

#ifndef VMS
MO_SIGHANDLER_RETURNTYPE ProcessExternalDirective(MO_SIGHANDLER_ARGS)
{
  char filename[64];
  char line[MO_LINE_LENGTH];
  char *status, *directive, *url;
  FILE *fp;

  signal(SIGUSR1, SIG_IGN);

  /* Construct filename from our pid. */
  sprintf(filename, "/tmp/Mosaic.%d", getpid());

  if (!(fp = fopen(filename, "r")))
      goto done;

  status = fgets(line, MO_LINE_LENGTH, fp);
  if (!status || !*line) {
      fclose(fp);
      goto done;
  }
  directive = strdup(line);

  /* We now allow URL to not exist, since some directives don't need it. */
  status = fgets(line, MO_LINE_LENGTH, fp);
  if (!status || !*line) {
      url = strdup("dummy");
  } else {
      url = strdup(line);
  }
  mo_process_external_directive(directive, url);

  free(directive);
  free(url);
  fclose(fp);

 done:
  signal(SIGUSR1, (void *)ProcessExternalDirective);
  return;
}
  
#else

void InitExternalDirective(int grp_mbx, char *mbx_name_in)
{
  char mbx_name[64], mbx_dev[64];

  if (!has_mbx) {
      int retl;
      $DESCRIPTOR(mbx_desc, NULL);
      $DESCRIPTOR(tab_desc, "LNM$PROCESS_DIRECTORY");
      $DESCRIPTOR(log_desc, "LNM$TEMPORARY_MAILBOX");    
      struct itm3 {
          short bfl, code;
          char *bufadr;
          int *retlen;
          int term;
      } itm;
      long promsk = 0xff00;

      mbx_desc.dsc$a_pointer = mbx_name;
      if (!mbx_name_in || !*mbx_name_in) {
          strcpy(mbx_name, "MOSAIC_");
          strcat(mbx_name, getenv("USER"));
      } else {
          strcpy(mbx_name, mbx_name_in);
      }
      mbx_desc.dsc$w_length = strlen(mbx_name);
      /*
       * Create a mailbox named from -mbx_name name command line option,
       * or default MOSAIC_<userid>.  Only owner and system can access it
       * unless grp_mbx is set.
       */
      if (grp_mbx) {
          promsk = 0xf000;
          itm.bfl = 9;
          itm.code = LNM$_STRING;
          itm.bufadr = (char *)&"LNM$GROUP";
          itm.retlen = &retl;
          itm.term = 0;      
          /*
           * Define/Table=LNM$PROCESS_DIRECTORY LNM$TEMPORARY_MAILBOX LNM$GROUP
           * to enter mailbox name in group name table.
           */
          if (!(sys$crelnm(0, &tab_desc, &log_desc, 0, &itm) & 1)) {
              char str[256];

              sprintf(str,
	            "Could not enter mailbox name in group table.\nError: %s",
                    strerror(errno, vaxc$errno));
              application_user_feedback(str);
          }
      }
      if (!(sys$crembx(0, &mbx_channel, 0, 0, promsk, 0, &mbx_desc, 0, 0) & 1)){
          char str[256];

          sprintf(str, "Could not open mailbox %s\nError: %s\0", mbx_name,
                  strerror(errno, vaxc$errno));
          if (grp_mbx)
	      strcat(str,
                  "\nCheck your Mosaic process privileges (GRPNAM is needed for a group mailbox).");
          application_user_feedback(str);  
          has_mbx = -1;
      } else {
          has_mbx = 1;
          itm.bfl = sizeof(mbx_dev);
          itm.code = DVI$_DEVNAM;
          itm.bufadr = mbx_dev;
          itm.retlen = &retl;
          itm.term = 0;
          if (sys$getdvi(0, mbx_channel, 0, &itm, 0, 0, 0, 0) & 1) {
              char str[256];
              char *home = getenv("HOME");
	      char *fnam;
              FILE *fp;

              sprintf(str,
                  "Mailbox for external directive processing is device %s\nName: %s\n\0",
                  mbx_dev, mbx_name);
              if (grp_mbx) {
                  strcat(str, "Name is in the group logical table.");
              } else {
                  strcat(str, "Name is in the process logical table only.");
	      }
              fnam = (char *)malloc(strlen(home) + 32);
              sprintf(fnam, "%smosaic.mbx", home);
              remove(fnam);
              if (fp = fopen(fnam, "w")) {
                  fprintf(fp, "%s %s\n", mbx_dev, mbx_name);
                  fclose(fp);
              }
              application_user_feedback(str);
              free(fnam);
          }
      }
      /*
       * Deassign LNM$TEMPORARY_MAILBOX (LNM$GROUP) from LNM$PROCESS_DIRECTORY
       */
      if (grp_mbx)
          sys$dellnm(&tab_desc, &log_desc, 0);
  }
  /*
   * Start to listen to the mailbox.
   */
  if (has_mbx == 1) {
      if (!(sys$qio(mbx_event_flag, mbx_channel, IO$_READVBLK, mbx_iosb, 0, 0,
                    mbx_buf, sizeof(mbx_buf), 0, 0, 0, 0) & 1)) {
          char str[256];

          sprintf(str, "Could not init read on mailbox %s\nError: %s",
	          mbx_name, strerror(errno, vaxc$errno));
          application_user_feedback(str);
      }
  }
}

void ProcessExternalDirective(XtPointer cd, int *s, XtInputId *id)
{
  char *status, *directive, *url;
  int free_url = 0;

  if (!(mbx_iosb[0] & 1) || (mbx_iosb[1] == 0))
      goto done;
  mbx_buf[mbx_iosb[1]] = '\0';
  directive = mbx_buf;

  /* We now allow URL to not exist, since some directives don't need it. */
  if (status = strchr(mbx_buf, '|')) {
      *status++ = '\0';
      url = status;
  } else {
      url = strdup("No URL specified.");
      free_url = 1;
  }  /* Need something in URL to prevent crashes */
  
  mo_process_external_directive(directive, url);

  if (free_url)
      free(url);

 done:
  InitExternalDirective(0, 0);
  return;
}  
#endif /* VMS, BSN */

/* Fatal exit */
static void RealFatal(void)
{
  signal(SIGBUS, 0);
  signal(SIGSEGV, 0);
  signal(SIGILL, 0);
#ifndef VMS
  abort();
#else
/*
 * Make an exit with abort fault status.
 */
  exit(44);
#endif /* VMS, BSN */
}

#ifdef __STDC__
static void FatalProblem(int sig)
#else /* not __STDC__ */
#ifdef _HPUX_SOURCE
static MO_SIGHANDLER_RETURNTYPE FatalProblem(int sig, int code,
					     struct sigcontext *scp, char *addr)
#else
static MO_SIGHANDLER_RETURNTYPE FatalProblem(int sig, int code,
					     struct sigcontext *scp, char *addr)
#endif
#endif /* not __STDC__ */
{
#ifndef VMS
  fprintf(stderr, "\nCongratulations, you have found a bug in\n");
  fprintf(stderr, "VMS Mosaic %s on %s.\n\n", MO_VERSION_STRING, 
          MO_MACHINE_TYPE);
  fprintf(stderr, "If a core file was generated in your directory,\n");
  fprintf(stderr, "please do one of the following:\n\n");
  fprintf(stderr, "  %% dbx /path/to/Mosaic /path/to/core\n");
  fprintf(stderr, "  dbx> where\n\n");
  fprintf(stderr, "OR\n\n");
  fprintf(stderr, "  %% gdb /path/to/Mosaic /path/to/core\n");
  fprintf(stderr, "  gdb> where\n\n");
  fprintf(stderr,
    "Mail the results and a description of what you were doing at the time,\n");
  fprintf(stderr,
    "(include any URLs involved!) to %s.\n\nWe thank you for your support.\n\n",
    MO_DEVELOPER_ADDRESS);
#else
#define SYI$_HW_NAME 4362
#define SYI$_VERSION 4096
#define JPI$_PAGFILCNT 1044
#define JPI$_PGFLQUOTA 1038

  int syi_hw_name = SYI$_HW_NAME;
  int syi_version = SYI$_VERSION;
  int jpi_pagfilcnt = JPI$_PAGFILCNT;
  int jpi_pgflquota = JPI$_PGFLQUOTA;
  char hardware[32], VMS_version[16];
  char *cp;
  int status, pagfilcnt, pgflquota;
  unsigned short l_hardware, l_version;

  struct dsc$descriptor_s {
    unsigned short  dsc$w_length;
    unsigned char   dsc$b_dtype;
    unsigned char   dsc$b_class;
    char            *dsc$a_pointer;
  } hardware_desc = { sizeof(hardware), 14, 1, NULL },
    VMS_version_desc = { sizeof(VMS_version), 14, 1, NULL };

  hardware_desc.dsc$a_pointer = hardware;
  VMS_version_desc.dsc$a_pointer = VMS_version;

  fprintf(stderr,
          "\nCongratulations, you may have found a bug in (your copy of)\n");
  fprintf(stderr, "VMS Mosaic %s on %s.\n", MO_VERSION_STRING, MO_MACHINE_TYPE);

  status = lib$getjpi((void *)&jpi_pagfilcnt, 0, 0, &pagfilcnt, 0, 0);
  status = lib$getjpi((void *)&jpi_pgflquota, 0, 0, &pgflquota, 0, 0);
  fprintf(stderr, "\nRemaining page file quota %d (page file quota = %d)\n",
	  pagfilcnt, pgflquota);
  if (pagfilcnt < 1000) {
      fprintf(stderr,
	"You have probably run out of page file quota.  An absolute minimum for Mosaic\n");
      fprintf(stderr,
	"is 40000 pages, but more will help.  Your systems person should increase the\n");
      fprintf(stderr,
	"Authorize parameter pgflquo for your account. But read on...\n");
  }

  fprintf(stderr,
      "\nIf you did not read the README.VMS-3_x file carefully before installing, it\n");
  fprintf(stderr,
      "might be a good time to do it now.  If there were any compilation or linking\n");
  fprintf(stderr,
      "errors, you should try to find the reason for them and correct them.\n");
  fprintf(stderr,
      "If this does not help, please take note of what happened in as much detail\n");
  fprintf(stderr, "as possible and send mail to cook@wvnet.edu.\n\n");

  status = lib$getsyi((void *)&syi_hw_name, 0, &hardware_desc, &l_hardware,
		      0, 0);
  status = lib$getsyi((void *)&syi_version, 0, &VMS_version_desc, &l_version,
		      0, 0);
  hardware[l_hardware] = '\0';
  VMS_version[l_version] = '\0';
  for (cp = &VMS_version[l_version - 1]; VMS_version; cp--) {
      if (*cp != ' ')
          break;
      *cp = '\0';
  }
  fprintf(stderr, "Your VMS version appears to be %s running on a %s.\n",
	  VMS_version, hardware);

  fprintf(stderr, "The TCP/IP software is ");
#ifdef MULTINET
  fprintf(stderr, "MultiNet.\n");
#elif WIN_TCP
  fprintf(stderr, "Pathway.\n");
#elif SOCKETSHR
  fprintf(stderr, "SOCKETSHR/NETLIB.\n");
#else
  fprintf(stderr, "TCP/IP Services (or UCX compatible).\n");
#endif

  fprintf(stderr, "Your Mosaic executable was generated using Motif ");
#ifdef MOTIF1_6
  fprintf(stderr, "1.6\n");
#else

#ifdef MOTIF1_5
  fprintf(stderr, "1.5\n");
#else

#ifdef MOTIF1_4
#ifdef MOTIF1_41
  fprintf(stderr, "1.4-1\n");
#else
  fprintf(stderr, "1.4\n");
#endif

#else

#ifdef MOTIF1_3
#ifdef MOTIF1_30
  fprintf(stderr, "1.3-0\n");
#else
#ifdef MOTIF1_31
  fprintf(stderr, "1.3-1\n");
#else
  fprintf(stderr, "1.3-x\n");
#endif
#endif

#else

#ifdef MOTIF1_26
  fprintf(stderr, "1.2-6\n");
#else
#ifdef MOTIF1_25
  fprintf(stderr, "1.2-5\n");
#else
#ifdef MOTIF1_24
  fprintf(stderr, "1.2-4\n");
#else
#ifdef MOTIF1_23
#if (MOTIF1_23 == 7)
  fprintf(stderr, "1.2-3 ECO 7\n");
#else
  fprintf(stderr, "1.2-3\n");
#endif
#else
#ifdef MOTIF1_2
  fprintf(stderr, "1.2\n");
#else
  fprintf(stderr, "1.1\n");
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

  fprintf(stderr, "and was built on %s with image Ident %s\n", built_time,
	  ident_ver);
#if defined(VAXC) && !defined(__DECC)
  fprintf(stderr, "using VAX C.\n"); 
#else
#ifdef __GNUC__
  fprintf(stderr, "using GNU C.\n");
#else
  fprintf(stderr, "using DEC C.\n"); 
#endif
#endif
#ifdef HAVE_SSL
#ifdef HAVE_HPSSL
  fprintf(stderr, "It was linked with HP SSL.\n\n"); 
#else
  fprintf(stderr, "It was linked with OpenSSL.\n\n"); 
#endif
#else
  fprintf(stderr, "\n");
#endif
  cp = getenv("SYS$LOGIN");
  if (!cp) {
      fprintf(stderr, "The logical SYS$LOGIN is undefined.\n");
  } else {
      fprintf(stderr, "The logical SYS$LOGIN points to %s\n", cp);
  }
  cp = getenv("SYS$SCRATCH");
  if (!cp) {
      fprintf(stderr, "The logical SYS$SCRATCH is undefined.\n");
  } else {
      fprintf(stderr, "The logical SYS$SCRATCH points to %s\n", cp);
  }
#endif /* VMS, BSN */
  fprintf(stderr, "...exiting VMS Mosaic now.\n\n");

  RealFatal();
}


main(int argc, char **argv, char **envp)
{
#ifdef SunOS
  struct utsname u;
#endif

#ifndef VMS
  userPath = getenv("PATH");

#ifdef SunOS
  if (!getenv("XKEYSYMDB"))
      fprintf(stderr,
	"If you have key binding problems, set the environment variable XKEYSYMDB\nto the location of the correct XKeysymDB file on your system.\n");
  if (uname(&u) < 0) {
      perror("uname");
  } else {
      if (!strcmp(u.sysname, "SunOS") && 
	  (!strcmp(u.release, "5.0") ||
	   !strcmp(u.release, "5.1") ||
	   !strcmp(u.release, "5.2") ||
	   !strcmp(u.release, "5.3") ||
	   !strcmp(u.release, "5.4") ||
	   !strcmp(u.release, "5.5"))) {
          if (!getenv("XKEYSYMDB")) {
	      FILE *fp;

              if (!(fp = fopen("/usr/openwin/lib/X11/XKeysymDB", "r"))) {
		  if (fp = fopen("/usr/openwin/lib/XKeysymDB", "r")) {
		      fclose(fp);
		      putenv("XKEYSYMDB=/usr/openwin/lib/XKeysymDB");
		  }
              } else {
	          fclose(fp);
	          putenv("XKEYSYMDB=/usr/openwin/lib/X11/XKeysymDB");
	      }
          }
      }
  }
#endif
#endif /* VMS, GEC */

#ifndef DEBUGVMS
  signal(SIGBUS, FatalProblem);
  signal(SIGSEGV, FatalProblem);
  signal(SIGILL, FatalProblem);
#endif /* DEBUGVMS, BSN */

#ifndef VMS
  /* Since we're doing lots of TCP, just ignore SIGPIPE altogether. */
  signal(SIGPIPE, SIG_IGN);
#endif /* VMS, BSN */

  InitChildProcessor();
#ifdef CCI
  MoCCIPreInitialize();
#endif

#ifndef VMS
#ifdef SVR4
  signal(SIGCLD, (void (*)())ChildTerminated);
#else
  signal(SIGCHLD, (void (*)())ChildTerminated);
#endif
#endif /* VMS, GEC */

#ifdef SOCKS
  SOCKSinit(argv[0]);
#endif

  mo_do_gui(argc, argv);
}
