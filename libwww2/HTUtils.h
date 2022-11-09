/* Utility macros for the W3 code library
 *                                MACROS FOR GENERAL USE
 *
 *  See also: the system dependent file "tcp.h"
 */

#ifndef HTUTILS_H
#define HTUTILS_H

#ifndef STDIO_H
#include <stdio.h>
#define STDIO_H
#endif

/*
 * Tracing now works as a boolean from a resource.  No, there are no
 *   more if's than before...
 *
 * SWP -- 02/08/96
 *
 * Removed all old tracing stuff.
 *
 * GEC -- 07/03/05
 */

/*
 * Standard C library for malloc(), etc.
 *
 */
#ifdef vax
#ifdef unix
#define ultrix  /* Assume vax + unix = ultrix */
#endif
#endif

#ifndef VMS
#ifndef ultrix
#ifdef NeXT
#include <libc.h>       /* NeXT */
#endif
#ifndef MACH /* Vincent.Cate@furmint.nectar.cs.cmu.edu */
#include <stdlib.h>     /* ANSI */
#endif
#else /* ultrix */
#include <malloc.h>
#include <memory.h>
#endif

#else   /* VMS */
#include <stdlib.h>
#include <ctype.h>
#include <lib$routines.h>
#ifndef __GNUC__
#include "../libnut/str-tools.h"
#include "../libnut/ellipsis.h"
#include "tcp.h"	/* Must come before mosaic.h if it is needed, VAX C */
#include "../src/mosaic.h"
#include "../src/mo-www.h"
#include "../src/gui.h"
#include "../src/mailto.h"
#else
#include "str-tools.h"
#include "ellipsis.h"
#include "tcp.h"
#include "mosaic.h"
#include "mo-www.h"
#include "gui.h"
#include "mailto.h"
#endif
#endif

#ifdef __sgi
#include <malloc.h>
#endif

/*
 * Macros for declarations
 */
#define PUBLIC                  /* Accessible outside this module     */
#define PRIVATE static          /* Accessible only within this module */

#if 0
#define WWW_CONST const             /* "const" only exists in STDC */
#endif
#define WWW_CONST

#ifndef NULL
#define NULL ((void *)0)
#endif

/*
 * Booleans
 */
/* Note: GOOD and BAD are already defined (differently) on RS6000 aix */
/* #define GOOD(status) ((status)38;1)   VMS style status: test bit 0       */
/* #define BAD(status)  (!GOOD(status))  Bit 0 set if OK, otherwise clear   */

#ifndef BOOLEAN_DEFINED
typedef char    BOOLEAN;                /* Logical value */

#ifndef TRUE
#define TRUE    (BOOLEAN)1
#define FALSE   (BOOLEAN)0
#endif
#define BOOLEAN_DEFINED
#endif

#ifndef BOOL
#define BOOL BOOLEAN
#endif
#ifndef YES
#define YES (BOOLEAN)1
#define NO (BOOLEAN)0
#endif

#ifndef min
#define min(a, b) ((a) <= (b) ? (a) : (b))
#endif

#define TCP_PORT 80     /* Allocated to http by Jon Postel/ISI 24-Jan-92 */

/*      Inline Function WHITE: Is character c white space? */
/*      For speed, include all control characters */
#define WHITE(c) (((unsigned char)(c)) <= 32)

/*
 * Success (>=0) and failure (<0) codes
 *
 */
#define HT_REDIRECTING 29998
#define HT_LOADED 29999                 /* Instead of a socket */
#define HT_NOT_LOADED -29999
#define HT_INTERRUPTED -29998
#define HT_NOT_SENT -29997
#define HT_OK           0               /* Generic success */

#define HT_FAILED       -1              /* Generic failure */
#define HT_NO_ACCESS    -10             /* Access not available */
#define HT_FORBIDDEN    -11             /* Access forbidden */
#define HT_INTERNAL     -12             /* Weird -- should never happen. */
#define HT_BAD_EOF      -12             /* Premature EOF */

#ifndef HTSTRING_H
#include "HTString.h"   /* String utilities */
#endif

#if defined(__STDC__) && !defined(sun)
#include <stdarg.h>
#else
#include <varargs.h>
#endif

/*
 * Out Of Memory checking for malloc() return:
 *
 */
#ifndef __FILE__
#define __FILE__ ""
#define __LINE__ ""
#endif

#ifndef VMS
#define outofmem(file, func) \
 { fprintf(stderr, "%s %s: out of memory.\nProgram aborted.\n", file, func); \
  exit(1);}
#else
extern void outofmem (WWW_CONST char *fname, WWW_CONST char *func);
#endif /* VMS, BSN, GEC */

/*
 * Upper- and Lowercase macros
 *
 *  The problem here is that toupper(x) is not defined officially unless
 *  isupper(x) is.  These macros are CERTAINLY needed on #if defined(pyr) ||
 *  define(mips) or BDSI platforms.  For safefy, we make them mandatory.
 */
#ifndef __CTYPE_LOADED
#include <ctype.h>
#endif

#ifndef TOLOWER
  /* Pyramid and Mips can't uppercase non-alpha */
#define TOLOWER(c) (isupper(c) ? tolower(c) : (c))
#define TOUPPER(c) (islower(c) ? toupper(c) : (c))
#endif

#define CR '\015'	/* Must be converted to ^M for transmission */
#define LF '\012'	/* Must be converted to ^J for transmission */

#endif  /* HTUTILS_H */
