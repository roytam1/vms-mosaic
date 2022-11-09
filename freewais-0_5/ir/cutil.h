/* WIDE AREA INFORMATION SERVER SOFTWARE
   No guarantees or restrictions.  See the readme file for the full standard
   disclaimer.  
  
   3.26.90	Harry Morris, morris@think.com
   4.11.90  HWM - generalized conditional includes (see c-dialect.h)
 *
 * $Log: cutil.h,v $
 * Revision 1.1  1993/02/16  15:05:35  freewais
 * Initial revision
 *
 * Revision 1.19  92/03/07  19:44:24  jonathan
 * Added some IBM defines. mycroft@hal.gnu.ai.mit.edu.
 * 
 * Revision 1.18  92/02/21  11:01:07  jonathan
 * Added wais_log_level
 * 
 * Revision 1.17  92/02/16  21:24:25  jonathan
 * Removed macro for waislog under BSD, since vprintf is now part of cutil.c
 * 
 *
 */

/* Copyright (c) CNIDR (see ../COPYRIGHT) */


#ifndef _H_C_util_
#define _H_C_util_

#include "cdialect.h"

#if defined(ANSI_LIKE) || defined(PROTO_ANSI)
#include <stdarg.h>
#else /* ndef ANSI_LIKE */
#include <varargs.h>
#endif /* ndef ANSI_LIKE */

#include <stdio.h>   /* this used to be wrapped in an ifndef NULL, 
			but messed up on some gcc's */
#if defined(THINK_C) || defined(_IBMR2) || (A_UX)
#include <time.h>
#endif
#if !defined(THINK_C)
#include <sys/time.h>
#endif

#define MAX_FILENAME_LEN 255
#define MAX_DELIMITERS 256

#ifdef ANSI_LIKE
#ifndef EXIT_SUCCESS /* only include it if not already included */
#include <stdlib.h> /* this is a shame */
#endif /* ndef EXIT_SUCCESS */
#else	
#include "ustubs.h"
#endif /* else */

/*----------------------------------------------------------------------*/
/* types and constants */

#ifndef boolean
#define boolean	unsigned long
#endif /* ndef boolean */ 

#ifndef THINK_C
#ifndef Boolean
#define Boolean	boolean
#endif /* ndef Boolean */ 
#endif /* ndef THINK_C */

#ifndef true
#define true 	(boolean)1L
#endif /* ndef true */

#ifndef false
#define false 	(boolean)0L   /* used to be (!true), but broke 
				 some compilers */
#endif /* ndef false */

#ifndef TRUE
#define TRUE	true
#endif /* ndef TRUE */

#ifndef FALSE
#define FALSE	false
#endif /* ndef FALSE */

#ifndef NULL
#define NULL	0L
#endif /* ndef NULL */

/*----------------------------------------------------------------------*/
/* Fast string macros - warning don't pass NULL to these! */

#define STREQ(s1,s2) ((*(s1)==*(s2)) && !strcmp(s1,s2))
#define STRNCMP(s1,s2,n) \
    ((*(s1)==*(s2)) ? strncmp(s1,s2,n) : (*(s1) - *(s2)))

/*----------------------------------------------------------------------*/
/* convenience */

#define NL() printf("\n")

/*----------------------------------------------------------------------*/
/* functions */

#ifdef __cplusplus
/* declare these as C style functions */
extern "C"
	{
#endif /* def __cplusplus */

/* enhanced memory handling functions - don't call them directly, use the
   macros below */
void	fs_checkPtr _AP((void* ptr));
void*	fs_malloc _AP((size_t size));
void*	fs_realloc _AP((void* ptr,size_t size));
void	fs_free _AP((void* ptr));
char* 	fs_strncat _AP((char* dst,char* src,size_t maxToAdd,size_t maxTotal));
char* 	fs_strncpy _AP((char* s1,char* s2, long n));

/* macros for memory functions.  call these in your program.  */
#define s_checkPtr(ptr) 	fs_checkPtr(ptr)
#define s_malloc(size)	      	fs_malloc(size)
#define s_realloc(ptr,size)	fs_realloc((ptr),(size))
#define s_free(ptr)		{ fs_free((char*)ptr); ptr = NULL; }
#define s_strncat(dst,src,maxToAdd,maxTotal)	fs_strncat((dst),(src),(maxToAdd),(maxTotal))
#define s_strncpy(s1,s2,n) fs_strncpy((s1), (s2), (n))

char* 	s_strdup _AP((char* s));

boolean wordbreak_isiso _AP(( long ch)); /* dgg */
boolean wordbreak_notalnum _AP(( long ch));  /* dgg */
boolean wordbreak_notgraph _AP(( long ch));	 /* dgg */
boolean wordbreak_user _AP(( long ch));	 /* dgg, uses gDelimiters */

char*	strtokf _AP((char* s1,boolean (*isDelimiter)(long c))); 
char* strtokf_isalnum _AP((char* s1));

#define IS_DELIMITER	true
#define	NOT_DELIMITER	false

#ifdef ANSI_LIKE	/* use ansi */
long		cprintf _AP((boolean print,char* format,...));
#else /* use K & R */
long		cprintf _AP(());
#endif

#ifdef ANSI_LIKE	/* use ansi */
void		waislog _AP((long priority, long message, char* format,...));
void		vwaislog _AP((long priority, long message, char *format, va_list));
#else /* use K & R */
void		waislog _AP(());
void		vwaislog _AP(());
#endif /* ANSI_LIKE */

/* waislog priorities and messages */
/* this is backwards because of how wais_log_level works. */
#define WLOG_HIGH	1
#define WLOG_MEDIUM 	5
#define WLOG_LOW	9

#define WLOG_CONNECT	1
#define WLOG_CLOSE	2
#define WLOG_SEARCH	3
#define WLOG_RESULTS	4
#define WLOG_RETRIEVE	5
#define WLOG_INDEX	6
#define WLOG_INFO	100
#define WLOG_ERROR	-1
#define WLOG_WARNING	-2

void 	warn _AP((char* message));

boolean substrcmp _AP((char *string1, char *string2));
#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif
#define ABS(x) (((x) < 0) ? (-(x)) : (x))

char *printable_time _AP((void));

char char_downcase _AP((unsigned long ch));
char *string_downcase _AP((char* word));


char *next_arg _AP((int *argc, char ***argv));
char *peek_arg _AP((int *argc, char ***argv));

void		beFriendly _AP((void));

#ifdef _C_C_util_
long wais_pid = 0;
long log_line = 0;
long wais_log_level = 10;
char gDelimiters[MAX_DELIMITERS];  /* dgg */
char *log_file_name = NULL;
FILE *logfile = NULL;
#else
extern long wais_pid;
extern long log_line;
extern wais_log_level;
extern char gDelimiters[];  /* dgg */
extern char *log_file_name;
extern FILE *logfile;
#endif /* _C_C_util_ */

#ifdef __cplusplus
	}
#endif /* def __cplusplus */

#ifdef USE_SYSLOG
#define LOG_WAIS LOG_LOCAL5
#endif

/*----------------------------------------------------------------------*/

#ifdef SOLARIS
#define LOG_USER "LOGNAME"
#else
#define LOG_USER "USER"
#endif


#ifdef SOLARIS
#define bzero(str,size) memset(str,0,size)
#define bcopy(s,d,c) memcpy(d,s,c)
#define index(str,c)  strchr(str,c)
#define rindex(str,c) strrchr(str,c)
#endif
/*
#ifdef LINUX
#include <string.h>
#define index(str,c)  strchr(str,c)
#define rindex(str,c) strrchr(str,c)
#endif
*/
#ifdef VMS
#define readBitMap readBitMapW
#endif /* VMS case-insensitivity clashes with Mosaic, BSN */
#endif /* ndef _H_C_util_ */
