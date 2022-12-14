#ifndef VMS_PANGDEF
#define VMS_PANGDEF

/* config.h.  Generated by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */
/* acconfig.h
   This file is in the public domain.

   Descriptive text for the C preprocessor macros that
   the distributed Autoconf macros can define.
   No software package will use all of them; autoheader copies the ones
   your configure.in uses into your configuration header file templates.

   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  Although this order
   can split up related entries, it makes it easier to check whether
   a given entry is in the file.

   Leave the following blank line there!!  Autoheader needs it.  */


/* #undef HAVE_FRIBIDI */
#define HAVE_UNISTD_H 1
/* #undef HAVE_FREETYPE_2_0_3 */
/* #undef HAVE_USP10_H */

/* #undef MINI_XFTCONFIG_DIR */
/* #undef HAVE_XFT2 */

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `flockfile' function. */
#if ( (defined(_XOPEN_SOURCE) || !defined(_ANSI_C_SOURCE)) && (__CRTL_VER >= 80200000) )
#define HAVE_FLOCKFILE 1
#endif

/* Define to 1 if you have the `FT_Get_First_Char' function. */
#define HAVE_FT_GET_FIRST_CHAR 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <stdint.h> header file. */
/* #undef HAVE_STDINT_H */

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strtok_r' function. */
/* #undef HAVE_STRTOK_R */

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

#define HAVE_CAIRO 1
#define HAVE_CAIRO_FREETYPE 1
#define HAVE_CAIRO_XLIB 1

/* Name of package */
#define PACKAGE "pango"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Whether to load modules via .la files rather than directly */
/* #undef USE_LA_MODULES */

/* Version number of package */
#define VERSION "1.10.2"
#define MODULE_VERSION "1_10_2"

/* Define to 1 if the X Window System is missing or not being used. */
/* #undef X_DISPLAY_MISSING */

#define LIBDIR
#define SYSCONFDIR

#include <types.h>

#endif
