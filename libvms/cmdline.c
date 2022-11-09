/*
**
**  Facility:	Mosaic
**
**  Module:	VMS_MOSAIC_CMDLINE
**
**  Abstract:	Routines to handle a VMS CLI interface for Mosaic.  The CLI
**		command line is parsed and a new argc/argv are built and
**		returned to Mosaic.  This code was adapted from Hunter
**		Goatley's <goathunter@wkuvx1.wku.edu> CLI interface for
**		VMS Unzip.
**
**	02-000		Hunter Goatley		12-JUL-1994 00:00
**		Original UnZip version (v5.11).
**
*/

/* Copyright (C) 2003, 2004, 2006 - The VMS Mosaic Project */

#include "../config.h"
#include "../src/mosaic.h"
#include <ssdef.h>
#include <descrip.h>
#include <climsgdef.h>
#include <clidef.h>
#include <lib$routines.h>
#include <str$routines.h>

extern char *built_time;
extern char *ident_ver;

/* This .h file should not be referenced in DESCRIP.MMS for this module */
/* because the SSL dummy .h file will force a recompile when needed. */
#ifdef __GNUC__
#include MOSAIC_BUILT
#else
#include "mosaic_built"
#endif

/*
**  "Macro" to initialize a dynamic string descriptor.
*/
#define init_dyndesc(dsc) { \
	dsc.dsc$w_length = 0; \
	dsc.dsc$b_dtype = DSC$K_DTYPE_T; \
	dsc.dsc$b_class = DSC$K_CLASS_D; \
	dsc.dsc$a_pointer = 0; }

/*
**  Define descriptors for all of the CLI parameters and qualifiers.
*/
$DESCRIPTOR(cli_color,		"COLOR");		/* -color */
$DESCRIPTOR(cli_mono,		"MONO");		/* -mono */
$DESCRIPTOR(cli_defaults,	"DEFAULTS");		/* -nd */
$DESCRIPTOR(cli_display,	"DISPLAY");		/* -display */
$DESCRIPTOR(cli_geometry,	"GEOMETRY");		/* -geometry */
$DESCRIPTOR(cli_install,	"INSTALL_COLORMAP");	/* -install */
$DESCRIPTOR(cli_iconic,		"ICONIC");		/* -iconic */
$DESCRIPTOR(cli_remote,		"REMOTE");		/* -mbx */
$DESCRIPTOR(cli_mbx_name,	"MAILBOX_NAME");	/* -mbx_name */
$DESCRIPTOR(cli_group,		"GROUP");		/* -mbx_grp */
$DESCRIPTOR(cli_home,		"HOME");		/* -home */
$DESCRIPTOR(cli_version,	"VERSION");
$DESCRIPTOR(cli_delayimageloads,"DELAY_IMAGE_LOADS");	/* -dil */
$DESCRIPTOR(cli_globalhistory,	"GLOBAL_HISTORY");	/* -ngh */
$DESCRIPTOR(cli_imagecachesize,	"IMAGE_CACHE_SIZE");	/* -ics */
$DESCRIPTOR(cli_kiosk,		"KIOSK");		/* -kiosk */
$DESCRIPTOR(cli_kioskNoExit,	"KIOSK.NOEXIT");	/* -kioskNoExit */
$DESCRIPTOR(cli_kioskPrint,	"KIOSK.PRINT");		/* -kioskPrint */
$DESCRIPTOR(cli_tempdirectory,	"TEMP_DIRECTORY");	/* -tmpdir */
$DESCRIPTOR(cli_background,	"BACKGROUND");		/* -background */
$DESCRIPTOR(cli_foreground,	"FOREGROUND");		/* -foreground */
$DESCRIPTOR(cli_synchronous,	"SYNCHRONOUS");		/* -sync */
$DESCRIPTOR(cli_nopreferences,	"NOPREFERENCES");	/* -nopref */
$DESCRIPTOR(cli_starturl,	"STARTURL");
$DESCRIPTOR(mosaic_command,	"Mosaic ");

#ifdef __DECC
extern void *mosaic_cld;
#else
#if defined(__GNUC__) && defined(vax)
#include <gnu_hacks.h>
GLOBALREF(void *, mosaic_cld);
#else
globalref void *mosaic_cld;
#endif
#endif

extern unsigned long cli$dcl_parse();
extern unsigned long cli$present();
extern unsigned long cli$get_value();

#ifdef TEST
unsigned long vms_mosaic_cmdline(int *, char ***);

unsigned long main(int argc, char **argv)
{
    return (vms_mosaic_cmdline(&argc, &argv));
}
#endif

/*
**  Routine:	vms_mosaic_cmdline
**
**  Function:
**
**	Parse the DCL command line and create a fake argv array to be
**	handed off to Mosaic.
**                               
**	NOTE: the argv[] is built as we go, so all the parameters are
**	checked in the appropriate order!!
**
**  Formal parameters:
**
**	argc_p		- Address of int to receive the new argc.
**			  Int contains original argc count at entry.
**	argv_p		- Address of char ** to receive the argv address
**			  The char ** contains original argv address at entry.
**
**  Calling sequence:
**
**	status = vms_mosaic_cmdline(&argc, &argv);
**
**  Returns:
**
**	SS$_NORMAL	- Success.
**	SS$_INSFMEM	- A malloc() or realloc() failed
**
*/
unsigned long vms_mosaic_cmdline(int *argc_p, char ***argv_p)
{
    register int status;
    char options[256];
    char *the_cmd_line;
    char *ptr;
    int x, len;
    int new_argc;
    char **new_argv;
    char **ori_argv;
#ifndef DISABLE_TRACE
    char *Trace;
#endif

    struct dsc$descriptor_d work_str;
    struct dsc$descriptor_d foreign_cmdline;
    struct dsc$descriptor_d home_document;
    struct dsc$descriptor_d geometry;
    struct dsc$descriptor_d display;
    struct dsc$descriptor_d mailbox_name;
    struct dsc$descriptor_d imagecachesize;
    struct dsc$descriptor_d tempdirectory;
    struct dsc$descriptor_d background;
    struct dsc$descriptor_d foreground;

    init_dyndesc(work_str);
    init_dyndesc(foreign_cmdline);
    init_dyndesc(home_document);
    init_dyndesc(geometry);
    init_dyndesc(display);
    init_dyndesc(mailbox_name);
    init_dyndesc(imagecachesize);
    init_dyndesc(tempdirectory);
    init_dyndesc(background);
    init_dyndesc(foreground);

    lib$get_foreign(&foreign_cmdline);

#ifndef DISABLE_TRACE
    Trace = getenv("MOSAIC_TRACE");

    if (Trace && foreign_cmdline.dsc$w_length) {
	char *tmp = malloc(foreign_cmdline.dsc$w_length + 1);

	strncpy(tmp, foreign_cmdline.dsc$a_pointer,
		foreign_cmdline.dsc$w_length);
	tmp[foreign_cmdline.dsc$w_length] = '\0';
	fprintf(stderr, "Foreign command line: %s\n", tmp);
	free(tmp);
    }
#endif 
    /*
    **  If nothing was returned or the first character is a "-" or not a "/",
    **  then assume it's a UNIX-style command and return.
    */
    if ((foreign_cmdline.dsc$w_length == 0) ||
	(*(foreign_cmdline.dsc$a_pointer) == '-') ||
	(*(foreign_cmdline.dsc$a_pointer) != '/'))
		return(SS$_NORMAL);

    str$concat(&work_str, &mosaic_command, &foreign_cmdline);
    status = cli$dcl_parse(&work_str, &mosaic_cld, lib$get_input,
			   lib$get_input, 0);
    if (!(status & 1))
	return(status);

    /*
    ** Check if version requested.  If so, display info and exit.
    */
    status = cli$present(&cli_version);
    if (status == CLI$_PRESENT) {
	printf("%s Mosaic version %s\n", MO_MACHINE_TYPE, MO_VERSION_STRING);
	printf("The executable was built with image Ident %s\n", ident_ver);
	printf("on %s for use with ", built_time);
#ifdef MULTINET
	printf("MultiNet TCP/IP\n");
#elif WIN_TCP
	printf("Pathway TCP/IP\n");
#elif SOCKETSHR
	printf("SOCKETSHR/NETLIB\n");
#else
	printf("UCX (or UCX compatible) TCP/IP\n");
#endif
	printf("and was generated using Motif ");

#ifdef MOTIF1_5
	printf("1.5 and");
#else

#ifdef MOTIF1_4
#ifdef MOTIF1_41
	printf("1.4-1 and");
#else
	printf("1.4 and");
#endif

#else

#ifdef MOTIF1_3
#ifdef MOTIF1_30
	printf("1.3-0 and");
#else
#ifdef MOTIF1_31
	printf("1.3-1 and");
#else
	printf("1.3-x and");
#endif
#endif

#else

#ifdef MOTIF1_26
	printf("1.2-6 and");
#else
#ifdef MOTIF1_25
	printf("1.2-5 and");
#else
#ifdef MOTIF1_24
	printf("1.2-4 and");
#else
#ifdef MOTIF1_23
#if (MOTIF1_23 == 7)
	printf("1.2-3 ECO 7 and");
#else
	printf("1.2-3 and");
#endif
#else
#ifdef MOTIF1_2
	printf("1.2 and");
#else
	printf("1.1 and");
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

#ifdef VAXC
	printf(" VAX C.\n");
#else
#ifdef __GNUC__
	printf(" GNU C.\n");
#else
	printf(" DEC C.\n");
#endif
#endif

#if defined(HAVE_JPEG) || defined(HAVE_PNG)
	printf("Support for inline ");
#if defined(HAVE_JPEG) && defined(HAVE_PNG)
	printf("JPEG and PNG");
#else
#ifdef HAVE_JPEG
	printf("JPEG");
#else
	printf("PNG");
#endif
#endif
	printf(" images is included.\n");
#endif
#ifdef HAVE_SSL
#ifdef HAVE_HPSSL
	printf("Support for secure connections using HP SSL is included.\n");
#else
	printf("Support for secure connections using OpenSSL is included.\n");
#endif
#else
	printf("Support for secure connections is not included.\n");
#endif
	exit(0);
    }

    /*
    **  There's always going to be a new_argv[] because of the image name.
    */
    if ((the_cmd_line = (char *) malloc(sizeof("mosaic") + 1)) == NULL)
	return(SS$_INSFMEM);

    strcpy(the_cmd_line, "mosaic");

    /*
    **  First, check to see if any of the regular options were specified.
    */

    ptr = &options[0];		/* Point to temporary buffer */

    /*
    **  Set color or monochrome resources
    */
    status = cli$present(&cli_color);
    if (status != CLI$_ABSENT) {
	strncpy(ptr, " -color", 7);
	ptr = ptr + 7;
    }
    status = cli$present(&cli_mono);
    if (status != CLI$_ABSENT) {
	strncpy(ptr, " -mono", 6);
	ptr = ptr + 6;
    }

    /*
    ** Do not use any defaults
    */
    status = cli$present(&cli_defaults);
    if (status == CLI$_NEGATED) {
	strncpy(ptr, " -nd", 4);
	ptr = ptr + 4;
    }

    /*
    **  Set display
    */
    status = cli$present(&cli_display);
    if (status == CLI$_PRESENT)
	status = cli$get_value(&cli_display, &display);

    /*
    **  Use user geometry
    */
    status = cli$present(&cli_geometry);
    if (status == CLI$_PRESENT)
	status = cli$get_value(&cli_geometry, &geometry);
                                
    /*
    **  Install color map
    */
    status = cli$present(&cli_install);
    if (status == CLI$_PRESENT) {
	strncpy(ptr, " -install", 9);
	ptr = ptr + 9;
    }

    /*
    **  Start as icon
    */
    status = cli$present(&cli_iconic);
    if (status == CLI$_PRESENT) {
	strncpy(ptr, " -iconic", 8);
	ptr = ptr + 8;
    }

    /*
    **  Set for remote control
    */
    status = cli$present(&cli_remote);
    if (status == CLI$_PRESENT) {
	strncpy(ptr, " -mbx", 5);
	ptr = ptr + 5;
    }

    /*
    **  Use group mailbox
    */
    status = cli$present(&cli_group);
    if (status == CLI$_PRESENT) {
	strncpy(ptr, " -mbx_grp", 9);
	ptr = ptr + 9;
    }

    /*
    **  Set mailbox name for remote control
    */
    status = cli$present(&cli_mbx_name);
    if (status == CLI$_PRESENT)
	status = cli$get_value(&cli_mbx_name, &mailbox_name);

    /*
    **  Set home document
    */
    status = cli$present (&cli_home);
    if (status == CLI$_PRESENT)
	status = cli$get_value(&cli_home, &home_document);

    /*
    **  Delay image loading
    */
    status = cli$present(&cli_delayimageloads);
    if (status == CLI$_PRESENT) {
	strncpy(ptr, " -dil", 5);
	ptr = ptr + 5;
    }

    /*
    ** Do not use global history file
    */
    status = cli$present(&cli_globalhistory);
    if (status == CLI$_NEGATED) {
	strncpy(ptr, " -ngh", 5);
	ptr = ptr + 5;
    }

    /*
    **  Image cache size in Kilobytes
    */
    status = cli$present(&cli_imagecachesize);
    if (status == CLI$_PRESENT)
	status = cli$get_value(&cli_imagecachesize, &imagecachesize);

    /*
    **  Set kiosk mode
    */
    status = cli$present(&cli_kiosk);
    if (status == CLI$_PRESENT) {
	if (cli$present(&cli_kioskNoExit) != CLI$_ABSENT) {
	    strncpy(ptr, " -kioskNoExit", 13);
	    ptr = ptr + 13;
	} else {
	    strncpy(ptr, " -kiosk", 7);
	    ptr = ptr + 7;
	}
	if (cli$present(&cli_kioskPrint) != CLI$_ABSENT) {
	    strncpy(ptr, " -kioskPrint", 12);
	    ptr = ptr + 12;
	}
    }

    /*
    **  Get Temp directory
    */
    status = cli$present(&cli_tempdirectory);
    if (status == CLI$_PRESENT)
	status = cli$get_value(&cli_tempdirectory, &tempdirectory);

    /*
    **  Set background color
    */
    status = cli$present (&cli_background);
    if (status == CLI$_PRESENT)
	status = cli$get_value(&cli_background, &background);

    /*
    **  Set foreground color
    */
    status = cli$present(&cli_foreground);
    if (status == CLI$_PRESENT)
	status = cli$get_value(&cli_foreground, &foreground);

    /*
    **  Set synchronous mode
    */
    status = cli$present(&cli_synchronous);
    if (status == CLI$_PRESENT) {
	strncpy(ptr, " -sync", 6);
	ptr = ptr + 6;
    }

    /*
    **  Disable preferences
    */
    status = cli$present(&cli_nopreferences);
    if (status != CLI$_ABSENT) {
	strncpy(ptr, " -nopref", 8);
	ptr = ptr + 8;
    }

    /*
    **  Now copy the final options string to the_cmd_line.
    */
    x = ptr - &options[0];
    if (x > 1) {
	options[x] = '\0';
      	len = strlen(the_cmd_line) + x + 1;
	if ((the_cmd_line = (char *) realloc(the_cmd_line, len)) == NULL)
	    return(SS$_INSFMEM);
	strcat(the_cmd_line, options);
    }

    /*
    **  Now get the specified startup URL.
    */
    status = cli$present(&cli_starturl);
    if (status & 1) {
	status = cli$get_value(&cli_starturl, &work_str);
	len = strlen(the_cmd_line) + work_str.dsc$w_length + 2;
	if ((the_cmd_line = (char *) realloc(the_cmd_line, len)) == NULL)
	    return(SS$_INSFMEM);
	strcat(the_cmd_line, " ");
	x = strlen(the_cmd_line);
	ori_argv = *argv_p;
	strncpy(&the_cmd_line[x], ori_argv[*argc_p - 1],
		work_str.dsc$w_length);
	the_cmd_line[len] = '\0';
    }

    /*
    **  Get the display
    **/
    if (display.dsc$w_length) {
	len = strlen(the_cmd_line) + display.dsc$w_length + 11;
	if ((the_cmd_line = (char *) realloc(the_cmd_line, len)) == NULL)
	    return(SS$_INSFMEM);
	strcat(the_cmd_line, " -display ");
	x = strlen(the_cmd_line);
	strncpy(&the_cmd_line[x], display.dsc$a_pointer, display.dsc$w_length);
	the_cmd_line[len] = '\0';
    }

    /*
    **  Get the geometry
    **/
    if (geometry.dsc$w_length) {
	len = strlen(the_cmd_line) + geometry.dsc$w_length + 12;
	if ((the_cmd_line = (char *) realloc(the_cmd_line, len)) == NULL)
	    return(SS$_INSFMEM);
	strcat(the_cmd_line, " -geometry ");
	x = strlen(the_cmd_line);
	strncpy(&the_cmd_line[x], geometry.dsc$a_pointer,
		geometry.dsc$w_length);
	the_cmd_line[len] = '\0';
    }

    /*
    **  Get the mailbox name
    **/
    if (mailbox_name.dsc$w_length) {
	len = strlen(the_cmd_line) + mailbox_name.dsc$w_length + 12;
	if ((the_cmd_line = (char *) realloc(the_cmd_line, len)) == NULL)
	    return(SS$_INSFMEM);
	strcat(the_cmd_line, " -mbx_name ");
	x = strlen(the_cmd_line);
	strncpy(&the_cmd_line[x], mailbox_name.dsc$a_pointer,
		mailbox_name.dsc$w_length);
	the_cmd_line[len] = '\0';
    }

    /*
    **  Get the home document
    **/
    if (home_document.dsc$w_length) {
	len = strlen(the_cmd_line) + home_document.dsc$w_length + 8;
	if ((the_cmd_line = (char *) realloc(the_cmd_line, len)) == NULL)
	    return(SS$_INSFMEM);
	strcat(the_cmd_line, " -home ");
	x = strlen(the_cmd_line);
	strncpy(&the_cmd_line[x], home_document.dsc$a_pointer,
		home_document.dsc$w_length);
	the_cmd_line[len] = '\0';
    }

    /*
    **  Get the image cache size
    **/
    if (imagecachesize.dsc$w_length) {
	len = strlen(the_cmd_line) + imagecachesize.dsc$w_length + 7;
	if ((the_cmd_line = (char *) realloc(the_cmd_line, len)) == NULL)
	    return(SS$_INSFMEM);
	strcat(the_cmd_line, " -ics ");
	x = strlen(the_cmd_line);
	strncpy(&the_cmd_line[x], imagecachesize.dsc$a_pointer,
		imagecachesize.dsc$w_length);
	the_cmd_line[len] = '\0';
    }

    /*
    **  Get the temp directory
    **/
    if (tempdirectory.dsc$w_length) {
	len = strlen(the_cmd_line) + tempdirectory.dsc$w_length + 10;
	if ((the_cmd_line = (char *) realloc(the_cmd_line, len)) == NULL)
	    return(SS$_INSFMEM);
	strcat(the_cmd_line, " -tmpdir ");
	x = strlen(the_cmd_line);
	strncpy(&the_cmd_line[x], tempdirectory.dsc$a_pointer,
		tempdirectory.dsc$w_length);
	the_cmd_line[len] = '\0';
    }

    /*
    **  Get the background color
    **/
    if (background.dsc$w_length) {
	len = strlen(the_cmd_line) + background.dsc$w_length + 14;
	if ((the_cmd_line = (char *) realloc(the_cmd_line, len)) == NULL)
	    return(SS$_INSFMEM);
	strcat(the_cmd_line, " -background ");
	x = strlen(the_cmd_line);
	strncpy(&the_cmd_line[x], background.dsc$a_pointer,
		background.dsc$w_length);
	the_cmd_line[len] = '\0';
    }

    /*
    **  Get the foreground color
    **/
    if (foreground.dsc$w_length) {
	len = strlen(the_cmd_line) + foreground.dsc$w_length + 14;
	if ((the_cmd_line = (char *) realloc(the_cmd_line, len)) == NULL)
	    return(SS$_INSFMEM);
	strcat(the_cmd_line, " -foreground ");
	x = strlen(the_cmd_line);
	strncpy(&the_cmd_line[x], foreground.dsc$a_pointer,
		foreground.dsc$w_length);
	the_cmd_line[len] = '\0';
    }

    /*
    **  Now that we've built our new UNIX-like command line, count the
    **  number of args and build an argv array.
    */

#if defined(TEST)
    printf("%s\n", the_cmd_line);
#endif

#ifndef DISABLE_TRACE
    if (Trace)
	fprintf(stderr, "Rebuilt command line: %s\n", the_cmd_line);
#endif 

    new_argc = 1;
    for (ptr = the_cmd_line; (ptr = strchr(ptr,' ')); ptr++, new_argc++)
	;

    /*
    **  Allocate memory for the new argv[].  The last element of argv[]
    **  is supposed to be 0, so allocate enough for new_argc + 1.
    */
    if ((new_argv = (char **) calloc(new_argc + 1, sizeof(char *))) == NULL)
	return(SS$_INSFMEM);

    /*
    **  For each option, store the address in new_argv[] and convert the
    **  separating blanks to nulls so each argv[] string is terminated.
    */
    for (ptr = the_cmd_line, x = 0; x < new_argc; x++) {
	new_argv[x] = ptr;
	if (ptr = strchr (ptr, ' '))
	    *ptr++ = '\0';
    }
    new_argv[new_argc] = 0;

#if defined(TEST)
    printf("new_argc    = %d\n", new_argc);
    for (x = 0; x < new_argc; x++)
	printf("new_argv[%d] = %s\n", x, new_argv[x]);
#endif

    /*
    **  All finished.  Return the new argc and argv[] addresses to Mosaic.
    */
    *argc_p = new_argc;
    *argv_p = new_argv;

    return(SS$_NORMAL);
}