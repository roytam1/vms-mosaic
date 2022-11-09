#ifndef VMS_FCONF
#define VMS_FCONF

/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated manually by keithp.  */

/* Path library uses when config file is broken */
#define FC_DEFAULT_FONTS "fontconfig$fonts"

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Use expat library for xml parsing */
#define HAVE_EXPAT 1

/* Use libxml2 instead of Expat */
#undef ENABLE_LIBXML2

/* expat uses old xmlparse.h include */
/* #undef HAVE_XMLPARSE_H */

/* Define to 1 if you have the <fcntl.h> header file. */
/* #undef HAVE_FCNTL_H */

/* whether system has freetype2 library */
#define HAVE_FREETYPE 1

/* Define to 1 if you have the <ft2build.h> header file. */
/* #undef HAVE_FT2BUILD_H */

/* Define to 1 if you have the `FT_Init_FreeType' function. */
#define HAVE_FT_INIT_FREETYPE 1

/* Define to 1 if you have the `getopt' function. */
#define HAVE_GETOPT 1

/* Define to 1 if you have the `getopt_long' function. */
/* #define HAVE_GETOPT_LONG */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if your system has a working `malloc' function. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
/* #undef HAVE_STAT_EMPTY_STRING_BUG */

/* Define to 1 if you have the <stdint.h> header file. */
/* #undef HAVE_STDINT_H */

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define to 1 if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H */

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strrchr' function. */
#define HAVE_STRRCHR 1

/* Define to 1 if you have the `strtol' function. */
#define HAVE_STRTOL 1

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

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if you have the `XML_ParserCreate' function. */
#define HAVE_XML_PARSERCREATE 1

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
/* #undef LSTAT_FOLLOWS_SLASHED_SYMLINK */

/* Define to the address where bug reports for this package should be sent. */
/* #undef PACKAGE_BUGREPORT */

/* Library major version */
#define PACKAGE_MAJOR 1

/* Library minor version */
#define PACKAGE_MINOR 0

/* Define to the full name of this package. */
/* #undef PACKAGE_NAME */

/* Library revision */
#define PACKAGE_REVISION 1

/* Define to the full name and version of this package. */
/* #undef PACKAGE_STRING */

/* Define to the one symbol short name of this package. */
/* #undef PACKAGE_TARNAME */

/* Define to the version of this package. */
/* #undef PACKAGE_VERSION */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

#define FONTCONFIG_PATH "/mosaic_dir"

#define PKGCACHEDIR "/fontconfig$cache"

#define HAVE_FT_GET_BDF_PROPERTY 1

#define HAVE_FT_GET_PS_FONT_INFO 1

#define HAVE_FT_GET_NEXT_CHAR 1

#ifndef VMS
#define __inline__ __inline
#else
#define __inline__
#endif

#endif
