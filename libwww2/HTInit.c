/* Copyright (C) 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"
#include "HTInit.h"

#include "HTML.h"
#include "HTPlain.h"
#include "HTMosaicHTML.h"
#include "HTMLGen.h"
#include "HTFile.h"
#include "HTFormat.h"
#include "HTMIME.h"
#include "HTWSRC.h"
#include "HTFWriter.h"

#include "tcp.h"
#include "HTUtils.h"

typedef struct suffix_rec {
        char *suffix;
        char *rep;
        char *encoding;
	float quality;
} Suffix;

static Suffix Suffixes[] = {
      {".saveme", "application/octet-stream", "binary", 1.0},  /* xtra */
      {".dump",   "application/octet-stream", "binary", 1.0},  /* xtra */
      {".hqx",    "application/octet-stream", "binary", 1.0},
      {".arc",    "application/octet-stream", "binary", 1.0},
      {".o",      "application/octet-stream", "binary", 1.0},
      {".a",      "application/octet-stream", "binary", 1.0},
      {".bin",    "application/octet-stream", "binary", 1.0},

      {".alpha_exe", "application/x-Executable", "binary", 1.0},
      {".exe",    "application/x-Executable", "binary", 1.0},

      {".Z",	  "application/UNIX Compressed", "binary", 1.0},
      {".gz",     "application/GNU Compressed", "binary", 1.0},
      {".tgz",    "application/GNU Compressed", "binary", 1.0},

      {".oda",    "application/oda", "binary", 1.0},

      {".pdf",    "application/pdf", "binary", 1.0},

      {".eps",    "application/postscript", "binary", 1.0},
      {".ai",     "application/postscript", "binary", 1.0},
      {".ps",     "application/postscript", "binary", 1.0},
      
      {".rtf",    "application/rtf", "binary", 1.0},

      {".dvi",    "application/x-DVI", "binary", 1.0},

      {".hdf",    "application/x-hdf", "binary", 1.0},
      
      {".latex",  "application/x-Latex", "binary", 1.0},

      {".cdf",    "application/x-netcdf", "binary", 1.0},
      {".nc",     "application/x-netcdf", "binary", 1.0},

      {".tex",    "application/x-Tex", "binary", 1.0},
      {".texinfo", "application/x-Texinfo", "binary", 1.0},
      {".texi",   "application/x-Texinfo", "binary", 1.0},

      {".t",      "application/x-Troff", "binary", 1.0},
      {".tr",     "application/x-Troff", "binary", 1.0},
      {".roff",   "application/x-Troff", "binary", 1.0},
      {".man",    "application/x-Troff-man", "binary", 1.0},
      {".me",     "application/x-Troff-me", "binary", 1.0},
      {".ms",     "application/x-Troff-ms", "binary", 1.0},

      {".src",    "application/x-wais-source", "binary", 1.0},
      {".wsrc",   "application/x-wais-source", "binary", 1.0},  /* xtra */

      {".zip",    "application/x-Zip File", "binary", 1.0},
      {".zoo",    "application/x-Zoo File", "binary", 1.0},
      {".uu",     "application/x-UUencoded", "binary", 1.0},

      {".bck",    "application/x-VMS BAK File", "binary", 1.0},
      {".hlb",    "application/x-VMS Help Libr.", "binary", 1.0},
      {".olb",    "application/x-VMS Obj. Libr.", "binary", 1.0},
      {".tlb",    "application/x-VMS Text Libr.", "binary", 1.0},
      {".obj",    "application/x-VMS Prog. Obj.", "binary", 1.0},
      {".decw$book", "application/x-DEC BookReader", "binary", 1.0},
      {".mem",    "application/x-RUNOFF-MANUAL", "binary", 1.0},

      {".bcpio",  "application/x-bcpio", "binary", 1.0},
      {".cpio",   "application/x-cpio", "binary", 1.0},
      {".gtar",   "application/x-gtar", "binary", 1.0},
      {".shar",   "application/x-shar", "binary", 1.0},
      {".sh",     "application/x-shar", "binary", 1.0},  /* xtra */
      {".sv4cpio", "application/x-sv4cpio", "binary", 1.0},
      {".sv4crc", "application/x-sv4crc", "binary", 1.0},
      {".tar",    "application/x-tar", "binary", 1.0},
      {".ustar",  "application/x-ustar", "binary", 1.0},

      {".snd",    "audio/basic", "binary", 1.0},
      {".au",     "audio/basic", "binary", 1.0},
      {".aud",    "audio/basic", "binary", 1.0},
      {".aifc",   "audio/x-aiff", "binary", 1.0},
      {".aif",    "audio/x-aiff", "binary", 1.0},
      {".aiff",   "audio/x-aiff", "binary", 1.0},
      {".wav",    "audio/x-wav", "binary", 1.0},
      {".midi",   "audio/midi", "binary", 1.0},
      
      {".bmp",    "image/bmp", "binary", 1.0},
      {".gif",    "image/gif", "binary", 1.0},
      {".png",    "image/png", "binary", 1.0},
      {".ief",    "image/ief", "binary", 1.0},
      {".jfif",   "image/jpeg", "binary", 1.0},     /* xtra */
      {".jfif-tbnl", "image/jpeg", "binary", 1.0},  /* xtra */
      {".jpe",    "image/jpeg", "binary", 1.0},
      {".jpg",    "image/jpeg", "binary", 1.0},
      {".jpeg",   "image/jpeg", "binary", 1.0},
      {".jp2",    "image/jp2", "binary", 1.0},
      {".j2c",    "image/jp2", "binary", 1.0},
      {".j2k",    "image/jp2", "binary", 1.0},
      {".tga",    "image/tga", "binary", 1.0},
      {".tif",    "image/tiff", "binary", 1.0},
      {".tiff",   "image/tiff", "binary", 1.0},
      {".ham",    "image/ham", "binary", 1.0},
      {".ras",    "image/x-sun-raster", "binary", 1.0},
      {".pnm",    "image/x-portable-anymap", "binary", 1.0},
      {".pbm",    "image/x-portable-bitmap", "binary", 1.0},
      {".pgm",    "image/x-portable-graymap", "binary", 1.0},
      {".ppm",    "image/x-portable-pixmap", "binary", 1.0},
      {".rgb",    "image/x-rgb", "binary", 1.0},
      {".xbm",    "image/x-xbitmap", "binary", 1.0},
      {".xpm",    "image/x-xpixmap", "binary", 1.0},
      {".xwd",    "image/x-xwindowdump", "binary", 1.0},

      {".php",    "text/html", "binary", 1.0},
      {".php3",   "text/html", "binary", 1.0},
      {".phtml",  "text/html", "binary", 1.0},
      {".shtml",  "text/html", "binary", 1.0},
      {".sht",    "text/html", "binary", 1.0},
      {".htm",    "text/html", "binary", 1.0},
      {".html",   "text/html", "binary", 1.0},
      {".htmlx",  "text/html", "binary", 1.0},

      /* ASCII text */
      {".asc",    "text/plain", "binary", 1.0},
      {".bas",    "text/plain", "binary", 1.0},
      {".c",	  "text/plain", "binary", 1.0},
      {".cc",     "text/plain", "binary", 1.0},
      /* Certificate */
      {".crt",    "text/plain", "binary", 1.0},
      /* Cascading style sheet */
      {".css",    "text/plain", "binary", 1.0},
      {".cxx",    "text/plain", "binary", 1.0},
      {".c++",    "text/plain", "binary", 1.0},
      {".for",    "text/plain", "binary", 1.0},
      {".h",	  "text/plain", "binary", 1.0},
      {".java",   "text/plain", "binary", 1.0},
      {".js",     "text/plain", "binary", 1.0},
      {".log",    "text/plain", "binary", 1.0},
      {".pl",     "text/plain", "binary", 1.0},
      {".text",   "text/plain", "binary", 1.0},
      {".txt",    "text/plain", "binary", 1.0},

      {".rtx",    "text/richtext", "binary", 1.0},  /* MIME richtext */
      {".tsv",    "text/tab-separated-values", "binary", 1.0},
      {".etx",    "text/x-setext", "binary", 1.0},

      {".mpg",    "video/mpeg", "binary", 1.0},
      {".mpe",    "video/mpeg", "binary", 1.0},
      {".mpeg",   "video/mpeg", "binary", 1.0},

      {".mov",    "video/quicktime", "binary", 1.0},
      {".qt",     "video/quicktime", "binary", 1.0},

      {".avi",    "video/x-msvideo", "binary", 1.0},

      {".movie",  "video/x-sgi-movie", "binary", 1.0},
      {".mv",     "video/x-sgi-movie", "binary", 1.0},

      {".mime",   "message/rfc822", "binary", 1.0},
      {NULL, NULL, NULL, 0.0}
};

extern int use_default_type_map;
extern char *global_type_map;
extern char *personal_type_map;
extern int use_default_extension_map;
extern char *global_extension_map;
extern char *personal_extension_map;

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/* Reread config files. */
PUBLIC void HTReInit (void)
{
  if (HTPresentations) {
      HTList_delete(HTPresentations);
      HTPresentations = NULL;
  }
  HTFormatInit();

  if (HTSuffixes) {
      HTList_delete(HTSuffixes);
      HTSuffixes = NULL;
  }
  HTFileInit();

  return;
}

PUBLIC void HTFormatInit (void)
{
  /* Note: secs_per_byte value not used by Mosaic, so we use it instead
   * to indicate whether the type is included in HTTP accept headers.
   * Zero means include it.
   */

  /* Conversions aren't customizable. */
  
  /* Add conversions in reverse because last added is at top of list. */
  HTSetConversion("application/x-wais-source", "*", HTWSRCConvert,
		  1.0, 0.0, 0.1);
  HTSetConversion("application/x-html", "www/present", HTMosaicHTMLPresent,
		  1.0, 0.0, 0.1);
  HTSetConversion("application/xhtml+xml", "www/present", HTMosaicHTMLPresent,
		  1.0, 0.0, 0.1);
  HTSetConversion("application/xml", "www/present", HTMosaicHTMLPresent,
		  1.0, 0.0, 0.1);
  HTSetConversion("application/html", "www/present", HTMosaicHTMLPresent,
		  1.0, 0.0, 0.1);
  HTSetConversion("text/x-html", "www/present", HTMosaicHTMLPresent,
		  1.0, 0.0, 0.1);
  HTSetConversion("text/xml", "www/present", HTMosaicHTMLPresent,
		  1.0, 0.0, 0.0);
  HTSetConversion("text/plain", "www/present", HTPlainPresent,	1.0, 0.0, 0.0);
  HTSetConversion("text/html", "www/present", HTMosaicHTMLPresent,
		  1.0, 0.0, 0.0);
  HTSetConversion("www/mime", "*", HTMIMEConvert, 1.0, 0.0, 0.0);

  if (use_default_type_map) {
#if defined(__sgi)
      HTSetPresentation("audio/basic", "sfplay %s", 1.0, 3.0, 0.0);
      HTSetPresentation("audio/x-aiff", "sfplay %s", 1.0, 3.0, 0.0);
#else
#ifdef VMS
      HTSetPresentation("audio/basic", "mcr decsound -file %s", 1.0, 3.0, 0.0);
      HTSetPresentation("audio/x-aiff", "mcr decsound -file %s", 1.0, 3.0, 0.0);
#else
#if defined(ultrix) || defined(__alpha)
      HTSetPresentation("audio/basic", "aplay %s", 1.0, 3.0, 0.0);
      HTSetPresentation("audio/x-aiff", "aplay %s", 1.0, 3.0, 0.0);
#else
      HTSetPresentation("audio/basic", "showaudio %s", 1.0, 3.0, 0.0);
      HTSetPresentation("audio/x-aiff", "showaudio %s", 1.0, 3.0, 0.0);
#endif
#endif  /* VMS, BSN */
#endif  /* __sgi */

      HTSetPresentation("image/bmp", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-bmp", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-ms-bmp", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/gif", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/jpeg", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/jp2", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-jpeg2000", "xv %s", 1.0, 3.0, 0.1);
      HTSetPresentation("image/pjpeg", "xv %s", 1.0, 3.0, 0.1);
      HTSetPresentation("image/png", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-png", "xv %s", 1.0, 3.0, 0.1);
      HTSetPresentation("image/sun-raster", "xv %s", 1.0, 3.0, 0.1);
      HTSetPresentation("image/x-sun-raster", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/targa", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/tga", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-targa", "xv %s", 1.0, 3.0, 0.1);
      HTSetPresentation("image/x-tga", "xv %s", 1.0, 3.0, 0.1);
      HTSetPresentation("image/tiff", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/tif", "xv %s", 1.0, 3.0, 0.1);
      HTSetPresentation("image/x-tiff", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-tif", "xv %s", 1.0, 3.0, 0.1);
      HTSetPresentation("image/x-portable-anymap", "xv %s", 1.0, 3.0, 0.1);
      HTSetPresentation("image/x-portable-bitmap", "xv %s", 1.0, 3.0, 0.1);
      HTSetPresentation("image/x-portable-graymap", "xv %s", 1.0, 3.0, 0.1);
      HTSetPresentation("image/x-portable-pixmap", "xv %s", 1.0, 3.0, 0.1);
      HTSetPresentation("image/x-rgb", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/rgb", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-xpm", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-xbm", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-xbitmap", "xv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("image/x-xpixmap", "xv %s", 1.0, 3.0, 0.0);  /* ?? */

      HTSetPresentation("image/xwd", "xwud -in %s", 1.0, 3.0, 0.1);
      HTSetPresentation("image/x-xwd", "xwud -in %s", 1.0, 3.0, 0.1);
      HTSetPresentation("image/x-xwindowdump", "xwud -in %s", 1.0, 3.0, 0.0);

      HTSetPresentation("video/mpeg", "mpeg_play %s", 1.0, 3.0, 0.0);
      HTSetPresentation("video/x-mpeg", "mpeg_play %s", 1.0, 3.0, 0.1);
#ifndef __sgi
      HTSetPresentation("video/quicktime", "xanim -f %s", 1.0, 3.0, 0.0);
#else
      HTSetPresentation("video/quicktime", "movieplayer -f %s", 1.0, 3.0, 0.0);
      HTSetPresentation("video/x-sgi-movie", "movieplayer -f %s", 1.0, 3.0,0.0);
#endif

#ifndef VMS
      HTSetPresentation("application/postscript", "ghostview %s", 1.0, 3.0,0.0);
#else
      HTSetPresentation("application/postscript", DEFAULT_PS_VIEWER,
			1.0, 3.0, 0.0);
#endif /* VMS, BSN, GEC */
      HTSetPresentation("application/pdf", "gv %s", 1.0, 3.0, 0.0);
      HTSetPresentation("application/x-dvi", "xdvi %s", 1.0, 3.0, 0.0);

      HTSetPresentation("message/rfc822", "xterm -e metamail %s",
			1.0, 3.0, 0.0);

      HTSetPresentation("application/x-latex", "mosaic-internal-present",
			1.0, 3.0, 0.1);
      HTSetPresentation("application/x-tex", "mosaic-internal-present",
			1.0, 3.0, 0.1);
      HTSetPresentation("application/x-texinfo", "mosaic-internal-present",
			1.0, 3.0, 0.1);
      HTSetPresentation("application/x-troff", "mosaic-internal-present",
			1.0, 3.0, 0.1);
      HTSetPresentation("application/x-troff-man", "mosaic-internal-present",
			1.0, 3.0, 0.1);
      HTSetPresentation("application/x-troff-me", "mosaic-internal-present",
			1.0, 3.0, 0.1);
      HTSetPresentation("application/x-troff-ms", "mosaic-internal-present",
			1.0, 3.0, 0.1);
      HTSetPresentation("application/x-x509-ca-cert", "mosaic-internal-present",
			1.0, 3.0, 0.1);
      HTSetPresentation("text/css", "mosaic-internal-present",
			1.0, 3.0, 0.1);
      HTSetPresentation("text/richtext", "mosaic-internal-present",
			1.0, 3.0, 0.1);
      HTSetPresentation("text/tab-separated-values", "mosaic-internal-present",
			1.0, 3.0, 0.1);
      HTSetPresentation("text/x-setext", "mosaic-internal-present",
			1.0, 3.0, 0.1);
  }

  /* Fall through clause. */
  HTSetPresentation ("*/*", "mosaic-internal-dump", 1.0, 3.0, 0.0);

  /* Does nothing except set variable default_presentation which is not used
  HTSetPresentation ("*", "mosaic-internal-dump", 1.0, 3.0, 0.0);
  */

  /* These should override the default types as necessary. */
  HTLoadTypesConfigFile(global_type_map);

  /* These should override everything else. */
  HTLoadTypesConfigFile(personal_type_map);
}


/* Some of the following is taken from: */

/*
Copyright (c) 1991 Bell Communications Research, Inc. (Bellcore)

Permission to use, copy, modify, and distribute this material 
for any purpose and without fee is hereby granted, provided 
that the above copyright notice and this permission notice 
appear in all copies, and that the name of Bellcore not be 
used in advertising or publicity pertaining to this 
material without the specific, prior written permission 
of an authorized representative of Bellcore.  BELLCORE 
MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY 
OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS", 
WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
*/
/****************************************************** 
    Metamail -- A tool to help diverse mail readers 
                cope with diverse multimedia mail formats.

    Author:  Nathaniel S. Borenstein, Bellcore

 ******************************************************* */

#define LINE_BUF_SIZE       2000

static char *GetCommand(char *s, char **t)
{
    char *s2;
    int quoted = 0;

    /* marca -- added + 1 for error case -- oct 24, 1993. */
    s2 = malloc(strlen(s) * 2 + 1);  /* Absolute max, if all % signs */
    *t = s2;
    while (s && *s) {
	if (quoted) {
            if (*s == '%')
		*s2++ = '%';  /* Quote through next level, ugh! */
            *s2++ = *s++;
	    quoted = 0;
	} else {
	    if (*s == ';') {
                *s2 = '\0';
		return(++s);
	    }
	    if (*s == '\\') {
		quoted = 1;
		++s;
	    } else {
		*s2++ = *s++;
	    }
	}
    }
    *s2 = '\0';
    return(NULL);
}	

static int ProcessMailcapEntry(FILE *fp)
{
    HTPresentation *pres;
    int rawentryalloc = 2000;
    int i, j, len;
    char LineBuf[LINE_BUF_SIZE];
    char *rawentry, *s, *t, *contenttype, *command;

    rawentry = malloc(1 + rawentryalloc);
    *rawentry = '\0';
    while (fgets(LineBuf, LINE_BUF_SIZE, fp)) {
	if (*LineBuf == '#')
	    continue;
	len = strlen(LineBuf);
        if (LineBuf[len - 1] == '\n')
	    LineBuf[--len] = '\0';
	if ((len + strlen(rawentry)) > rawentryalloc) {
	    rawentryalloc += 2000;
	    rawentry = realloc(rawentry, rawentryalloc + 1);
	}
	if ((len > 0) && (LineBuf[len - 1] == '\\')) {
            LineBuf[len - 1] = '\0';
	    strcat(rawentry, LineBuf);
	} else {
	    strcat(rawentry, LineBuf);
	    break;
	}
    }
    for (s = rawentry; *s && isspace((unsigned char) *s); ++s)
	;
    if (!*s) {
	/* Totally blank entry -- quietly ignore */
	free(rawentry);
	return(0);
    }
    s = strchr(rawentry, ';');
    if (!s) {
#ifndef DISABLE_TRACE
        if (www2Trace)
	    fprintf(stderr, "Ignoring invalid mailcap entry: %s\n", rawentry);
#endif
        free(rawentry);
        return(0);
    }
    *s++ = '\0';

    /* Remove blanks and make lower case */
    for (i = j = 0; rawentry[i]; i++) {
        if (!isspace((unsigned char)(rawentry[i])))
	    rawentry[j++] = TOLOWER(rawentry[i]);
    }
    rawentry[j] = '\0';

    contenttype = strdup(rawentry);

    t = GetCommand(s, &command);

    /* Update current if already defined (no duplicates allowed) */
    i = 0;
    do {
	pres = HTList_objectAt(HTPresentations, i);
	if (pres && !strcmp(pres->rep->name, contenttype)) {
	    if (pres->command)
		free(pres->command);
	    pres->command = command;
	    pres->rep_out = WWW_PRESENT;
	    pres->converter = HTSaveAndExecute;
	    pres->quality = 1.0;
	    pres->secs = 3.0;
	    /* Force it into Accept: headers */
	    pres->secs_per_byte = 0.0;
#ifndef DISABLE_TRACE
	    if (www2Trace)
	        fprintf(stderr, "Mailcap overrode: %s\n", contenttype);
#endif
	    break;
	} else if (!pres) {
	    /* Not found */
	    HTSetPresentation(contenttype, command, 1.0, 3.0, 0.0);
	    free(command);
	    break;
	}
    } while (++i);

    free(contenttype);
    free(rawentry);
    return(1);
}


int HTLoadTypesConfigFile (char *fn)
{
    FILE *fp;

#ifndef DISABLE_TRACE
    if (www2Trace)
        fprintf(stderr, "Loading types config file '%s'\n", fn);
#endif

    if (fp = fopen(fn, "r")) {
        while (!feof(fp))
            ProcessMailcapEntry(fp);
	fclose(fp);
    }
    return(-1);
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

/*	Define a basic set of suffixes
**	------------------------------
**
**	The LAST suffix for a type is that used for temporary files
**	of that type.
**	The quality is an apriori bias as to whether the file should be
**	used.  Note that different suffixes can be used to represent files
**	which are of the same format but are originals or regenerated,
**	with different values.
*/
PUBLIC void HTFileInit (void)
{
  if (use_default_extension_map) {
      Suffix *suffixes = Suffixes;

      while (suffixes->suffix) {
	  HTSetSuffix(suffixes->suffix, suffixes->rep, suffixes->encoding,
		      suffixes->quality);
	  suffixes++;
      }
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "Using default extension map\n");
#endif
  }

  /* These should override the default extensions as necessary. */
  HTLoadExtensionsConfigFile(global_extension_map);
  
  /* These should override everything else. */
  HTLoadExtensionsConfigFile(personal_extension_map);
}


/* -------------------- Extension config file reading --------------------- */

/* The following is lifted from NCSA httpd 1.0a1, by Rob McCool;
 * NCSA httpd is in the public domain, as is this code. */

#define MAX_STRING_LEN 256

static int getline(char *s, int n, FILE *f) 
{
  register int i = 0;
  
  while (1) {
      s[i] = (char)fgetc(f);
      
      if (s[i] == CR)
          s[i] = fgetc(f);
    
      if ((s[i] == EOF) || (s[i] == LF) || (i == (n - 1))) {
          s[i] = '\0';
          return (feof(f) ? 1 : 0);
      }
      ++i;
  }
}

static void getword(char *word, char *line, char stop, char stop2) 
{
  int x;
  int y = 0;

  for (x = 0; line[x] && (line[x] != stop) && (line[x] != stop2); x++)
      word[x] = line[x];
  
  word[x] = '\0';
  if (line[x]) 
      ++x;

  while (line[y++] = line[x++])
      ;

  return;
}

int HTLoadExtensionsConfigFile(char *fn)
{
  char l[MAX_STRING_LEN], w[MAX_STRING_LEN];
  char *ct, *ptr;
  FILE *f;
  int x;
  int count = 0;

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "Loading extensions config file '%s'\n", fn);
#endif
  
  if (!(f = fopen(fn, "r"))) {
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "Could not open extensions config file '%s'\n", fn);
#endif
      return -1;
  }

  while (!getline(l, MAX_STRING_LEN, f)) {
      /* Always get rid of leading white space for "line" */
      for (ptr = l; *ptr && isspace(*ptr); ptr++)
	  ;
      getword(w, ptr, ' ', '\t');
      if (!*ptr || *w == '#')
          continue;
      ct = strdup(w);
      while (*ptr) {
          getword(w, ptr, ' ', '\t');
          if (*w && (*w != ' ')) {
              char *ext = (char *)malloc(sizeof(char) * (strlen(w) + 1 + 1));

              for (x = 0; w[x]; x++)
                  ext[x + 1] = TOLOWER(w[x]);
              *ext = '.';
              ext[strlen(w) + 1] = '\0';
#ifndef DISABLE_TRACE
              if (www2Trace)
                  fprintf(stderr, "Setting suffix '%s' to '%s'\n", ext, ct);
#endif
              HTSetSuffix(ext, ct, "binary", 1.0);
              count++;
              free(ext);
          }
      }
      free(ct);
  }
  fclose(f);

  return count;
}
