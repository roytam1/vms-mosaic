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

/* Copyright (C) 2004, 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"

/*
 * Code appears to originally have been based on version 3.2g LibXpm.
 * Updated with code from 3.4k on 7-Jun-05 - GEC
 */

/*
 * The following XPM reading code was based on the libXpm code, which I
 * am free to use as long as I include the following copyright:
 */
/*
 * Copyright (C) 1989-95 GROUPE BULL
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * GROUPE BULL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of GROUPE BULL shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from GROUPE BULL.
 */

#include "mosaic.h"
#include "xpmread.h"

#include <X11/Xos.h>
#include "xpm.h"

/* For memset */
#ifndef VMS
#include <memory.h>
#else
#include <string.h>
#endif

extern int installed_colormap;
extern Colormap installed_cmap;

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int reportBugs;
#endif


/*
 * Free the computed color table
 */
static void xpmFreeColorTable(XpmColor *colorTable, int ncolors)
{
    if (colorTable) {
	int a, b;
	XpmColor *color;
	char **sptr;

	for (a = 0, color = colorTable; a < ncolors; a++, color++) {
	    for (b = 0, sptr = (char **) color; b <= NKEYS; b++, sptr++) {
		if (*sptr)
		    XpmFree(*sptr);
	    }
	}
	XpmFree(colorTable);
    }
}


/*
 * Intialize the xpmInternAttrib pointers to Null to know
 * which ones must be freed later on.
 */
static void xpmInitInternAttrib(xpmInternAttrib *attrib)
{
    attrib->ncolors = 0;
    attrib->colorTable = NULL;
    attrib->pixelindex = NULL;
}


/*
 * Free the xpmInternAttrib pointers which have been allocated
 */
static void xpmFreeInternAttrib(xpmInternAttrib *attrib)
{
    if (attrib->colorTable)
	xpmFreeColorTable(attrib->colorTable, attrib->ncolors);
    if (attrib->pixelindex)
	XpmFree(attrib->pixelindex);
}


static int ParseComment(xpmData *data)
{
    if (data->type == XPMBUFFER) {
	register char c;
	register unsigned int n = 0;
	unsigned int notend;
	char *s, *s2;

	s = data->Comment;
	*s = data->Bcmt[0];

	/* Skip the string beginning comment */
	s2 = data->Bcmt;
	do {
	    c = *data->cptr++;
	    *++s = c;
	    n++;
	    s2++;
	} while (c == *s2 && *s2 && c);

	if (*s2) {
	    /* This wasn't the beginning of a comment */
	    data->cptr -= n;
	    return 0;
	}
	/* Store comment */
	data->Comment[0] = *s;
	s = data->Comment;
	notend = 1;
	n = 0;
	while (notend) {
	    s2 = data->Ecmt;
	    while (*s != *s2 && c) {
		c = *data->cptr++;
		if (n == BUFSIZ - 1) {  /* Forget it */
		    s = data->Comment;
		    n = 0;
		}
		*++s = c;
		n++;
	    }
	    data->CommentLength = n;
	    do {
		c = *data->cptr++;
		if (n == BUFSIZ - 1) {  /* Forget it */
		    s = data->Comment;
		    n = 0;
		}
		*++s = c;
		n++;
		s2++;
	    } while (c == *s2 && *s2 && c);
	    if (!*s2) {
		/* This is the end of the comment */
		notend = 0;
		data->cptr--;
	    }
	}
	return 0;
    } else {
	FILE *file = data->stream.file;
	register int c;
	register unsigned int n = 0, a;
	unsigned int notend;
	char *s, *s2;

	s = data->Comment;
	*s = data->Bcmt[0];

	/* Skip the string beginning comment */
	s2 = data->Bcmt;
	do {
	    c = getc(file);
	    *++s = c;
	    n++;
	    s2++;
	} while (c == *s2 && *s2 && c != EOF);

	if (*s2) {
	    /* This wasn't the beginning of a comment */
	    /* Put characters back in the order that we got them */
	    for (a = n; a > 0; a--, s--)
		ungetc(*s, file);
	    return 0;
	}
	/* Store comment */
	data->Comment[0] = *s;
	s = data->Comment;
	notend = 1;
	n = 0;
	while (notend) {
	    s2 = data->Ecmt;
	    while (*s != *s2 && c != EOF) {
		c = getc(file);
		if (n == BUFSIZ - 1) {  /* Forget it */
		    s = data->Comment;
		    n = 0;
		}
		*++s = c;
		n++;
	    }
	    data->CommentLength = n;
	    do {
		c = getc(file);
		if (n == BUFSIZ - 1) {  /* Forget it */
		    s = data->Comment;
		    n = 0;
		}
		*++s = c;
		n++;
		s2++;
	    } while (c == *s2 && *s2 && c != EOF);
	    if (!*s2) {
		/* This is the end of the comment */
		notend = 0;
		ungetc(*s, file);
	    }
	}
	return 0;
    }
}


/*
 * Skip to the end of the current string and the beginning of the next one
 */
static int xpmNextString(xpmData *mdata)
{
    if (!mdata->type) {
	mdata->cptr = (mdata->stream.data)[++mdata->line];
    } else if (mdata->type == XPMBUFFER) {
	register char c;

	/* Get to the end of the current string */
	if (mdata->Eos) {
	    while ((c = *mdata->cptr++) && c != mdata->Eos)
		;
	}
	/*
	 * Then get to the beginning of the next string looking for possible
	 * comment
	 */
	if (mdata->Bos) {
	    while ((c = *mdata->cptr++) && c != mdata->Bos) {
		if (mdata->Bcmt && c == mdata->Bcmt[0])
		    ParseComment(mdata);
	    }
	} else if (mdata->Bcmt) {	/* XPM2 natural */
	    while ((c = *mdata->cptr++) == mdata->Bcmt[0])
		ParseComment(mdata);
	    mdata->cptr--;
	}
    } else {
	register int c;
	FILE *file = mdata->stream.file;

	/* Get to the end of the current string */
	if (mdata->Eos) {
	    while ((c = getc(file)) != mdata->Eos && c != EOF)
		;
	}
	/*
	 * Then get to the beginning of the next string looking for possible
	 * comment
	 */
	if (mdata->Bos) {
	    while ((c = getc(file)) != mdata->Bos && c != EOF) {
		if (mdata->Bcmt && c == mdata->Bcmt[0])
		    ParseComment(mdata);
	    }
	} else if (mdata->Bcmt) {	/* XPM2 natural */
	    while ((c = getc(file)) == mdata->Bcmt[0])
		ParseComment(mdata);
	    ungetc(c, file);
	}
    }
    return 0;
}


static unsigned int atoui(register char *p, unsigned int l,
			  unsigned int *ui_return)
{
    register int n = 0;
    register int i;

    for (i = 0; i < l; i++) {
	if (*p >= '0' && *p <= '9') {
	    n = n * 10 + *p++ - '0';
	} else {
	    break;
	}
    }
    if (i && i == l) {
	*ui_return = n;
	return 1;
    } else {
	return 0;
    }
}


/*
 * Skip whitespace and return the following word
 */
static unsigned int xpmNextWord(xpmData *mdata, char *buf)
{
    register unsigned int n = 0;
    int c;

    if (!mdata)
        return 0;

    if (!mdata->type || mdata->type == XPMBUFFER) {
	while (isspace(c = *mdata->cptr) && c != mdata->Eos)
	    mdata->cptr++;
	do {
	    c = *mdata->cptr++;
	    *buf++ = c;
	    n++;
	} while (!isspace(c) && c != mdata->Eos && (n < BUFSIZ));
	n--;
	mdata->cptr--;
    } else {
	FILE *file = mdata->stream.file;

	while ((c = getc(file)) != EOF && isspace(c) && c != mdata->Eos)
	    ;
	while (!isspace(c) && c != mdata->Eos && c != EOF && (n < BUFSIZ)) {
	    *buf++ = c;
	    n++;
	    c = getc(file);
	}
	ungetc(c, file);
    }
    return (n);
}


/*
 * Skip whitespace and compute the following unsigned int,
 * returns 1 if one is found and 0 if not
 */
static int xpmNextUI(xpmData *mdata, unsigned int *ui_return)
{
    char buf[BUFSIZ];
    int l;

    l = xpmNextWord(mdata, buf);
    return atoui(buf, l, ui_return);
}


static int ParseValues(xpmData *data, unsigned int *width, unsigned int *height,
		       unsigned int *ncolors, unsigned int *cpp,
	    	       unsigned int *x_hotspot, unsigned int *y_hotspot,
		       unsigned int *hotspot, unsigned int *extensions)
{
    unsigned int l;
    char buf[BUFSIZ];

    if (!data->format) {		/* XPM 2 or 3 */
	/*
	 * Read values: width, height, ncolors, chars_per_pixel
	 */
	if (!(xpmNextUI(data, width) && xpmNextUI(data, height) &&
	      xpmNextUI(data, ncolors) && xpmNextUI(data, cpp)))
	    return (XpmFileInvalid);
	/*
	 * Read optional information (hotspot and/or XPMEXT) if any
	 */
	l = xpmNextWord(data, buf);
	if (l) {
	    *extensions = (l == 6 && !strncmp("XPMEXT", buf, 6));
	    if (*extensions) {
		*hotspot = (xpmNextUI(data, x_hotspot) &&
			    xpmNextUI(data, y_hotspot));
	    } else {
		*hotspot = (atoui(buf, l, x_hotspot) &&
			    xpmNextUI(data, y_hotspot));
		l = xpmNextWord(data, buf);
		*extensions = (l == 6 && !strncmp("XPMEXT", buf, 6));
	    }
	}
    } else {
	/*
	 * XPM 1 file read values: width, height, ncolors, chars_per_pixel
	 */
	int i;
	char *ptr;
	Bool got_one;
	Bool saw_width = False;
	Bool saw_height = False;
	Bool saw_ncolors = False;
	Bool saw_chars_per_pixel = False;

	for (i = 0; i < 4; i++) {
	    l = xpmNextWord(data, buf);
	    if (l != 7 || strncmp("#define", buf, 7))
		return (XpmFileInvalid);
	    l = xpmNextWord(data, buf);
	    if (!l)
		return (XpmFileInvalid);
	    buf[l] = '\0';
	    ptr = buf;
	    got_one = False;
	    while (!got_one) {
		ptr = strchr(ptr, '_');
		if (!ptr)
		    return (XpmFileInvalid);
		switch (l - (ptr - buf)) {
		  case 6:
		    if (saw_width || strncmp("_width", ptr, 6) ||
			!xpmNextUI(data, width)) {
			return (XpmFileInvalid);
		    } else {
			saw_width = True;
		    }
		    got_one = True;
		    break;
		  case 7:
		    if (saw_height || strncmp("_height", ptr, 7) ||
			!xpmNextUI(data, height)) {
			return (XpmFileInvalid);
		    } else {
			saw_height = True;
		    }
		    got_one = True;
		    break;
		  case 8:
		    if (saw_ncolors || strncmp("_ncolors", ptr, 8) ||
			!xpmNextUI(data, ncolors)) {
			return (XpmFileInvalid);
		    } else {
			saw_ncolors = True;
		    }
		    got_one = True;
		    break;
		  case 16:
		    if (saw_chars_per_pixel ||
			strncmp("_chars_per_pixel", ptr, 16) ||
			!xpmNextUI(data, cpp)) {
			return (XpmFileInvalid);
		    } else {
			saw_chars_per_pixel = True;
		    }
		    got_one = True;
		    break;
		  default:
		    ptr++;
		}
	    }
	    /* Skip the end of line */
	    xpmNextString(data);
	}
	if (!saw_width || !saw_height || !saw_ncolors || !saw_chars_per_pixel)
	    return (XpmFileInvalid);

	*hotspot = 0;
	*extensions = 0;
    }
    return (XpmSuccess);
}

static int ParseColors(xpmData *data, unsigned int ncolors, unsigned int cpp,
		       XpmColor **colorTablePtr, xpmHashTable *hashtable)
{
    unsigned int key, l, a, b;
    unsigned int curkey;		/* Current color key */
    unsigned int lastwaskey;		/* Key read */
    char buf[BUFSIZ];
    char curbuf[BUFSIZ];		/* Current buffer */
    char **sptr, *s;
    XpmColor *color, *colorTable;
    char **defaults;
    int ErrorStatus;
    static char *xpmColorKeys[] = {
        "s",				/* key #1: symbol */
        "m",				/* key #2: mono visual */
        "g4",				/* key #3: 4 grays visual */
        "g",				/* key #4: gray visual */
        "c",				/* key #5: color visual */
    };

    colorTable = (XpmColor *)XpmCalloc(ncolors, sizeof(XpmColor));
    if (!colorTable)
	return (XpmNoMemory);

    if (!data->format) {		/* XPM 2 or 3 */
	for (a = 0, color = colorTable; a < ncolors; a++, color++) {
	    xpmNextString(data);	/* Skip the line */
	    /*
	     * Read pixel value
	     */
	    color->string = (char *) XpmMalloc(cpp + 1);
	    if (!color->string) {
		xpmFreeColorTable(colorTable, ncolors);
		return (XpmNoMemory);
	    }
	    for (b = 0, s = color->string; b < cpp; b++, s++)
		*s = xpmGetC(data);
	    *s = '\0';

	    /*
	     * Store the string in the hashtable with its color index number
	     */
	    if (USE_HASHTABLE) {
		ErrorStatus =
		    xpmHashIntern(hashtable, color->string, HashAtomData(a));
		if (ErrorStatus != XpmSuccess) {
		    xpmFreeColorTable(colorTable, ncolors);
		    return (ErrorStatus);
		}
	    }
	    /*
	     * Read color keys and values
	     */
	    defaults = (char **) color;
	    curkey = lastwaskey = 0;
	    *curbuf = '\0';		/* Init curbuf */
	    while (l = xpmNextWord(data, buf)) {
		if (!lastwaskey) {
		    for (key = 0, sptr = xpmColorKeys; key < NKEYS; key++,
			 sptr++) {
			if ((strlen(*sptr) == l) && !strncmp(*sptr, buf, l))
			    break;
		    }
		}
		if (!lastwaskey && key < NKEYS) {	/* Open new key */
		    if (curkey) {	/* Flush string */
			s = (char *) XpmMalloc(strlen(curbuf) + 1);
			if (!s) {
			    xpmFreeColorTable(colorTable, ncolors);
			    return(XpmNoMemory);
			}
			defaults[curkey] = s;
			strcpy(s, curbuf);
		    }
		    curkey = key + 1;	/* Set new key  */
		    *curbuf = '\0';	/* Reset curbuf */
		    lastwaskey = 1;
		} else {
		    if (!curkey) {	/* Key without value */
			xpmFreeColorTable(colorTable, ncolors);
			return (XpmFileInvalid);
		    }
		    if (!lastwaskey)
			strcat(curbuf, " ");	/* Append space */
		    buf[l] = '\0';
		    strcat(curbuf, buf);	/* Append buf */
		    lastwaskey = 0;
		}
	    }
	    if (!curkey) {		/* Key without value */
		xpmFreeColorTable(colorTable, ncolors);
		return (XpmFileInvalid);
	    }
	    s = defaults[curkey] = (char *) XpmMalloc(strlen(curbuf) + 1);
	    if (!s) {
		xpmFreeColorTable(colorTable, ncolors);
		return (XpmNoMemory);
	    }
	    strcpy(s, curbuf);
	}
    } else {				/* XPM 1 */
	/* Get to the beginning of the first string */
	data->Bos = '"';
	data->Eos = '\0';
	xpmNextString(data);
	data->Eos = '"';
	for (a = 0, color = colorTable; a < ncolors; a++, color++) {
	    /*
	     * Read pixel value
	     */
	    color->string = (char *) XpmMalloc(cpp + 1);
	    if (!color->string) {
		xpmFreeColorTable(colorTable, ncolors);
		return (XpmNoMemory);
	    }
	    for (b = 0, s = color->string; b < cpp; b++, s++)
		*s = xpmGetC(data);
	    *s = '\0';

	    /*
	     * Store the string in the hashtable with its color index number
	     */
	    if (USE_HASHTABLE) {
		ErrorStatus =
		       xpmHashIntern(hashtable, color->string, HashAtomData(a));
		if (ErrorStatus != XpmSuccess) {
		    xpmFreeColorTable(colorTable, ncolors);
		    return (ErrorStatus);
		}
	    }
	    /*
	     * Read color values
	     */
	    xpmNextString(data);	/* Get to the next string */
	    *curbuf = '\0';		/* Init curbuf */
	    while (l = xpmNextWord(data, buf)) {
		if (*curbuf)
		    strcat(curbuf, " ");	/* Append space */
		buf[l] = '\0';
		strcat(curbuf, buf);
	    }
	    s = (char *) XpmMalloc(strlen(curbuf) + 1);
	    if (!s) {
		xpmFreeColorTable(colorTable, ncolors);
		return (XpmNoMemory);
	    }
	    strcpy(s, curbuf);
	    color->c_color = s;
	    *curbuf = '\0';		/* Reset curbuf */
	    if (a < ncolors - 1)
		xpmNextString(data);	/* Get to the next string */
	}
    }
    *colorTablePtr = colorTable;
    return (XpmSuccess);

}

/* Free all allocated pointers at all exits */
#define FREE_CIDX { int f; for (f = 0; f < 256; f++) \
	if (cidx[f]) XpmFree(cidx[f]); }

static int ParsePixels(xpmData *data, unsigned int width, unsigned int height,
		       unsigned int ncolors, unsigned int cpp,
		       XpmColor *colorTable, xpmHashTable *hashtable,
		       unsigned int **pixels)
{
    unsigned int *iptr, *iptr2;
    unsigned int a, x, y;

    iptr2 = (unsigned int *) XpmMalloc(sizeof(unsigned int) * width * height);
    if (!iptr2)
	return (XpmNoMemory);

    iptr = iptr2;

    switch (cpp) {
      case (1):			/* Optimize for single character colors */
	{
	    unsigned short colidx[256];

	    memset(colidx, 0, 256 * sizeof(short));
	    for (a = 0; a < ncolors; a++)
		colidx[(unsigned char)colorTable[a].string[0]] = a + 1;

	    for (y = 0; y < height; y++) {
		xpmNextString(data);
		for (x = 0; x < width; x++, iptr++) {
		    int c = xpmGetC(data);

		    if (c > 0 && c < 256 && colidx[c]) {
			*iptr = colidx[c] - 1;
		    } else {
			XpmFree(iptr2);
			return (XpmFileInvalid);
		    }
		}
	    }
	}
	break;

      case (2):			/* Optimize for double character colors */
	{
	    /* Array of pointers malloced by need */
	    unsigned short *cidx[256];
	    int char1;

	    memset(cidx, 0, 256 * sizeof(unsigned short *));
	    for (a = 0; a < ncolors; a++) {
		char1 = colorTable[a].string[0];
		if (cidx[char1] == NULL) {  /* Get new memory */
		    cidx[char1] = (unsigned short *)
					 XpmCalloc(256, sizeof(unsigned short));
		    if (cidx[char1] == NULL) {
			FREE_CIDX;
			XpmFree(iptr2);
			return (XpmNoMemory);
		    }
		}
		cidx[char1][(unsigned char)colorTable[a].string[1]] = a + 1;
	    }

	    for (y = 0; y < height; y++) {
		xpmNextString(data);
		for (x = 0; x < width; x++, iptr++) {
		    int cc1 = xpmGetC(data);

		    if (cc1 > 0 && cc1 < 256) {
			int cc2 = xpmGetC(data);

			if (cc2 > 0 && cc2 < 256 &&
			    cidx[cc1] && cidx[cc1][cc2]) {
			    *iptr = cidx[cc1][cc2] - 1;
			} else {
			    FREE_CIDX;
			    XpmFree(iptr2);
			    return (XpmFileInvalid);
			}
		    } else {
			FREE_CIDX;
			XpmFree(iptr2);
			return (XpmFileInvalid);
		    }
		}
	    }
	    FREE_CIDX;
	}
	break;

      default:			/* Non-optimized case of long color names */
	{
	    char *s;
	    char buf[BUFSIZ];

	    buf[cpp] = '\0';
	    if (USE_HASHTABLE) {
		xpmHashAtom *slot;

		for (y = 0; y < height; y++) {
		    xpmNextString(data);
		    for (x = 0; x < width; x++, iptr++) {
			for (a = 0, s = buf; a < cpp; a++, s++)
			    *s = xpmGetC(data);
			slot = xpmHashSlot(hashtable, buf);
			if (!*slot) {	/* No color matches */
			    XpmFree(iptr2);
			    return (XpmFileInvalid);
			}
			*iptr = HashColorIndex(slot);
		    }
		}
	    } else {
		for (y = 0; y < height; y++) {
		    xpmNextString(data);
		    for (x = 0; x < width; x++, iptr++) {
			for (a = 0, s = buf; a < cpp; a++, s++)
			    *s = xpmGetC(data);
			for (a = 0; a < ncolors; a++) {
			    if (!strcmp(colorTable[a].string, buf))
				break;
			}
			if (a == ncolors) {	/* No color matches */
			    XpmFree(iptr2);
			    return (XpmFileInvalid);
			}
			*iptr = a;
		    }
		}
	    }
	}
	break;
    }
    *pixels = iptr2;
    return (XpmSuccess);
}


/*
 * parse xpm header
 */
static int xpmParseHeader(xpmData *data)
{
    char buf[BUFSIZ];
    int l;
    int n = 0;
    static xpmDataType xpmDataTypes[] = {
        "", "!", "\n", '\0', '\n', "", "", "", "",	/* Natural type */
        "C", "/*", "*/", '"', '"', ",\n", "static char *", "[] = {\n", "};\n",
        "Lisp", ";", "\n", '"', '"', "\n", "(setq ", " '(\n", "))\n",
#ifdef VMS
        NULL
#else
        NULL, NULL, NULL, 0, 0, NULL, NULL, NULL, NULL
#endif
    };

    if (data->type) {
	data->Bos = '\0';
	data->Eos = '\n';
	data->Bcmt = data->Ecmt = NULL;
	l = xpmNextWord(data, buf);
	if (l == 7 && !strncmp("#define", buf, 7)) {
	    /* This maybe an XPM 1 file */
	    char *ptr;

	    l = xpmNextWord(data, buf);
	    if (!l)
		return (XpmFileInvalid);
	    buf[l] = '\0';
	    ptr = strrchr(buf, '_');
	    if (!ptr || strncmp("_format", ptr, l - (ptr - buf)))
		return XpmFileInvalid;
	    /* This is definitely an XPM 1 file */
	    data->format = 1;
	    n = 1;			/* handle XPM1 as mainly XPM2 C */
#ifndef DISABLE_TRACE
	    if (srcTrace)
		fprintf(stderr, "Found XPM1 image.\n");
#endif
	} else {
	    /*
	     * Skip the first word, get the second one, and see if this is
	     * XPM 2 or 3
	     */
	    l = xpmNextWord(data, buf);
	    if ((l == 3 && !strncmp("XPM", buf, 3)) ||
		(l == 4 && !strncmp("XPM2", buf, 4))) {
		if (l == 3) {
		    n = 1;		/* Handle XPM as XPM2 C */
		} else {
		    /* Get the type key word */
		    l = xpmNextWord(data, buf);
		    /*
		     * Get infos about this type
		     */
		    while (xpmDataTypes[n].type &&
			   strncmp(xpmDataTypes[n].type, buf, l))
			n++;
		}
		data->format = 0;
#ifndef DISABLE_TRACE
		if (srcTrace)
		    fprintf(stderr, "Found XPM image.\n");
#endif
	    } else {
		/* Nope this is not an XPM file */
		return XpmFileInvalid;
	    }
	}
	if (xpmDataTypes[n].type) {
	    if (n == 0) {		/* Natural type */
		data->Bcmt = xpmDataTypes[n].Bcmt;
		data->Ecmt = xpmDataTypes[n].Ecmt;
		xpmNextString(data);	/* Skip the end of the headerline */
		data->Bos = xpmDataTypes[n].Bos;
		data->Eos = xpmDataTypes[n].Eos;
	    } else {
		data->Bcmt = xpmDataTypes[n].Bcmt;
		data->Ecmt = xpmDataTypes[n].Ecmt;
		if (!data->format) {	/* XPM 2 or 3 */
		    data->Bos = xpmDataTypes[n].Bos;
		    data->Eos = '\0';
		    /* Get to the beginning of the first string */
		    xpmNextString(data);
		    data->Eos = xpmDataTypes[n].Eos;
		} else {		/* XPM 1 skip end of line */
		    xpmNextString(data);
		}
	    }
	} else {
	    /* We don't know about that type of XPM file... */
	    return XpmFileInvalid;
	}
    }
    return XpmSuccess;
}


/*
 * Get the current comment line
 */
static int xpmGetCmt(xpmData *mdata, char **cmt)
{
    if (!mdata->type || !mdata->CommentLength) {
	*cmt = NULL;
    } else {
	*cmt = (char *) XpmMalloc(mdata->CommentLength + 1);
	strncpy(*cmt, mdata->Comment, mdata->CommentLength);
	(*cmt)[mdata->CommentLength] = '\0';
	mdata->CommentLength = 0;
    }
    return 0;
}


/*
 * This function parses an Xpm file or data and store the found informations
 * in an an xpmInternAttrib structure which is returned.
 */
static int xpmParseData(xpmData *data, xpmInternAttrib *attrib_return,
			XpmAttributes *attributes)
{
    /* Variables to return */
    unsigned int width, height, ncolors, cpp, cmts;
    unsigned int x_hotspot, y_hotspot;
    unsigned int hotspot = 0;
    unsigned int extensions = 0;
    XpmColor *colorTable = NULL;
    unsigned int *pixelindex = NULL;
    char *hints_cmt = NULL;
    char *colors_cmt = NULL;
    char *pixels_cmt = NULL;
    int ErrorStatus;
    xpmHashTable hashtable;

    cmts = attributes && (attributes->valuemask & XpmReturnInfos);
    /*
     * Parse the header
     */
    ErrorStatus = xpmParseHeader(data);
    if (ErrorStatus != XpmSuccess)
	return (ErrorStatus);
    /*
     * Read values
     */
    ErrorStatus = ParseValues(data, &width, &height, &ncolors, &cpp,
			      &x_hotspot, &y_hotspot, &hotspot, &extensions);
    if (ErrorStatus != XpmSuccess)
	return (ErrorStatus);
    /*
     * Store the hints comment line
     */
    if (cmts)
	xpmGetCmt(data, &hints_cmt);
    /*
     * Init the hashtable
     */
    if (USE_HASHTABLE) {
	ErrorStatus = xpmHashTableInit(&hashtable);
	if (ErrorStatus != XpmSuccess)
	    return (ErrorStatus);
    }
    /*
     * Read colors
     */
    ErrorStatus = ParseColors(data, ncolors, cpp, &colorTable, &hashtable);
    if (ErrorStatus != XpmSuccess) {
        if (USE_HASHTABLE)
	    xpmHashTableFree(&hashtable);
	goto error;
    }
    /*
     * Store the colors comment line
     */
    if (cmts)
	xpmGetCmt(data, &colors_cmt);
    /*
     * Read pixels and index them on color number
     */
    ErrorStatus = ParsePixels(data, width, height, ncolors, cpp, colorTable,
			      &hashtable, &pixelindex);
    /*
     * Free the hashtable
     */
    if (USE_HASHTABLE)
	xpmHashTableFree(&hashtable);
    if (ErrorStatus != XpmSuccess)
	goto error;
    /*
     * Store the pixels comment line
     */
    if (cmts)
	xpmGetCmt(data, &pixels_cmt);
    /*
     * Store found informations in the xpmInternAttrib structure
     */
    attrib_return->width = width;
    attrib_return->height = height;
    attrib_return->cpp = cpp;
    attrib_return->ncolors = ncolors;
    attrib_return->colorTable = colorTable;
    attrib_return->pixelindex = pixelindex;

    if (attributes) {
	if (attributes->valuemask & XpmReturnInfos) {
	    attributes->hints_cmt = hints_cmt;
	    attributes->colors_cmt = colors_cmt;
	    attributes->pixels_cmt = pixels_cmt;
	}
	if (hotspot) {
	    attributes->x_hotspot = x_hotspot;
	    attributes->y_hotspot = y_hotspot;
	    attributes->valuemask |= XpmHotspot;
	}
    }
    return (XpmSuccess);

 /* Exit point in case of error, free only locally allocated variables */
 error:
    if (colorTable)
        xpmFreeColorTable(colorTable, ncolors);
    if (pixelindex)
        XpmFree(pixelindex);
    if (hints_cmt)
        XpmFree(hints_cmt);
    if (colors_cmt)
        XpmFree(colors_cmt);
    if (pixels_cmt)
        XpmFree(pixels_cmt);

    return(ErrorStatus);
}


/*
 * Set up to read file as an xpmData which is returned.
 */
static int xpmInitFile(FILE *fp, xpmData *mdata)
{
    if (!fp) {
	return(XpmOpenFailed);
    } else {
	mdata->stream.file = fp;
    }
    mdata->type = XPMFILE;
    mdata->CommentLength = 0;
    return (XpmSuccess);
}


/*
 * Open the given array to be read or written as an xpmData which is returned
 */
static void xpmOpenArray(char **data, xpmData *mdata)
{
    mdata->type = XPMARRAY;
    mdata->stream.data = data;
    mdata->cptr = *data;
    mdata->line = 0;
    mdata->CommentLength = 0;
    mdata->Bcmt = mdata->Ecmt = NULL;
    mdata->Bos = mdata->Eos = '\0';
    mdata->format = 0;                  /* This can only be Xpm 2 or 3 */
}


/*
 * Decode XPM data from a file (or from memory if file pointer is NULL).
 */
unsigned char *ReadXpmPixmap(Widget wid, char **xpmdata, FILE *fp, int *w,
			     int *h, XColor *colrs, int *bg)
{
    xpmData mdata;
    XpmAttributes attributes;
    xpmInternAttrib attrib;
    int ErrorStatus, Colors, i;
    XColor tmpcolr;
    char *colorName;
    unsigned char *pix_data, *bptr;
    unsigned int *pixels;
    unsigned int size;
    Colormap default_cmap;

    *w = 0;
    *h = 0;

    attributes.valuemask = XpmReturnPixels;

    /* Either a file or memory buffer */
    if (fp) {
	if ((ErrorStatus = xpmInitFile(fp, &mdata)) != XpmSuccess)
	    return(NULL);
    } else {
	xpmOpenArray(xpmdata, &mdata);
    }
    xpmInitInternAttrib(&attrib);

    ErrorStatus = xpmParseData(&mdata, &attrib, &attributes);
    if (ErrorStatus != XpmSuccess) {
	xpmFreeInternAttrib(&attrib);
	return(NULL);
    }

    *w = (int)attrib.width;
    *h = (int)attrib.height;
    size = attrib.width * attrib.height;
    Colors = (int)attrib.ncolors;

    default_cmap = DefaultColormap(dsp, DefaultScreen(dsp));
    for (i = 0; i < Colors; i++) {
	colorName = attrib.colorTable[i].c_color ?
		    attrib.colorTable[i].c_color : "black";
	if (!strcmp(colorName, TRANSPARENT_COLOR)) {
	    unsigned long bg_pixel;

	    /* First, go fetch the pixel. */
	    XtVaGetValues(wid, XtNbackground, &bg_pixel, NULL);

	    /* Now, load up tmpcolr. */
	    tmpcolr.pixel = bg_pixel;

	    /* Now query for the full color info. */
	    XQueryColor(dsp, installed_colormap ? installed_cmap : default_cmap,
			&tmpcolr);
	    *bg = i;
	} else {
	    XParseColor(dsp, installed_colormap ? installed_cmap : default_cmap,
			colorName, &tmpcolr);
	}
	colrs[i].red = tmpcolr.red;
	colrs[i].green = tmpcolr.green;
	colrs[i].blue = tmpcolr.blue;
	colrs[i].pixel = i;
	colrs[i].flags = DoRed | DoGreen | DoBlue;
    }
    for (i = Colors; i < 256; i++) {
	colrs[i].red = colrs[i].green = colrs[i].blue = 0;
	colrs[i].pixel = i;
	colrs[i].flags = DoRed | DoGreen | DoBlue;
    }
    pixels = attrib.pixelindex;
    pix_data = (unsigned char *)malloc(size);
    if (!pix_data) {
#ifndef DISABLE_TRACE
	if (srcTrace || reportBugs)
	    fprintf(stderr, "Not enough memory for pixmap.\n");
#endif
	xpmFreeInternAttrib(&attrib);
        return((unsigned char *)NULL);
    }
    bptr = pix_data;
    for (i = 0; i < size; i++) {
	int pix;

	pix = (int)*pixels++;
        if (pix > 255)
            pix = 0;
	*bptr++ = (unsigned char)pix;
    }
    xpmFreeInternAttrib(&attrib);

    return(pix_data);
}
