/* +-------------------------------------------------------------------+ */
/* | Copyright 1990 - 1994, David Koblas. (koblas@netcom.com)          | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.           | */
/* +-------------------------------------------------------------------+ */
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

/* Copyright (C) 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include "gifread.h"

#define	MAXCOLORMAPSIZE	256

#define	TRUE	1
#define	FALSE	0

#define	MAX_LWZ_BITS	12

#define INTERLACE	0x40
#define LOCALCOLORMAP	0x80

#define BitSet(byte, bit)  ((byte & bit) == bit)

#define	ReadOK(file, buffer, len)  (fread(buffer, len, 1, file) != 0)

#define LM_to_uint(a, b)  ((b << 8) | a)

static struct {
	unsigned int	Width;
	unsigned int	Height;
	unsigned char	ColorMap[MAXCOLORMAPSIZE][3];
	unsigned int	BitPixel;
	unsigned int	ColorResolution;
	unsigned int	Background;
	unsigned int	AspectRatio;
	/*
	**
	*/
	int             xGrayScale;
} GifScreen;

static struct {
	int transparent;
	int delayTime;
	int inputFlag;
	int disposal;
	int animated;
} Gif89;

static int ReadColorMap(FILE *, int, unsigned char [MAXCOLORMAPSIZE][3]);
static void DoExtension(FILE *, int);
static int GetDataBlock(FILE *, unsigned char *);
static unsigned char *ReadImage(FILE *, int, int, XColor *, int,
                		unsigned char [MAXCOLORMAPSIZE][3], int);

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int reportBugs;
#endif

unsigned char *ReadGIF(FILE *fd, int *w, int *h, XColor *colrs, int *bg,
		       int imageNumber,	int *animated, int *delay,
		       int *x, int *y, int *disposal) 
{
    unsigned char c;
    unsigned char buf[16];
    unsigned char localColorMap[MAXCOLORMAPSIZE][3];
    unsigned char *image = NULL;
    int	useGlobalColormap;

    if (imageNumber == 1) {
	char *version;

	/*
	 * Initialize GIF89 extensions
	 */
	Gif89.transparent = -1;
	Gif89.delayTime = -1;
	Gif89.inputFlag = -1;
	Gif89.disposal = 0;
	Gif89.animated = -1;

	if (!ReadOK(fd, buf, 6)) {
#ifndef DISABLE_TRACE
	    if (srcTrace)
		fprintf(stderr, "Bogus image file\n");
#endif
	    return(NULL);
	}
	if (strncmp((char *)buf, "GIF", 3)) {
#ifndef DISABLE_TRACE
	    if (srcTrace)
		fprintf(stderr, "GIF: Not a GIF file\n");
#endif
	    return(NULL);
	}

	version = (char *)buf + 3;

	if (strncmp(version, "87a", 3) && strncmp(version, "89a", 3)) {
#ifndef DISABLE_TRACE
	    if (srcTrace)
		fprintf(stderr, "GIF: Version not '87a' or '89a'\n");
#endif
	    return(NULL);
	}

	/* Get screen descriptor */
	if (!ReadOK(fd, buf, 7))
	    return(NULL);

	GifScreen.Width           = LM_to_uint(buf[0], buf[1]);
	GifScreen.Height          = LM_to_uint(buf[2], buf[3]);
	GifScreen.BitPixel        = 2 << (buf[4] & 0x07);
	/* GifScreen.ColorResolution = ((buf[4] & 0x70) >> 3) + 1; */
	GifScreen.Background      = buf[5];
	GifScreen.AspectRatio     = buf[6];

	if (BitSet(buf[4], LOCALCOLORMAP)) {	/* Global Colormap */
	    static int scale = 65536 / MAXCOLORMAPSIZE;
	    int i;

	    if (ReadColorMap(fd, GifScreen.BitPixel, GifScreen.ColorMap)) {
#ifndef DISABLE_TRACE
		if (srcTrace)
		    fprintf(stderr, "GIF: Error reading global colormap\n");
#endif
		return(NULL);
	    }
	    for (i = 0; i < GifScreen.BitPixel; i++) {
		colrs[i].red = GifScreen.ColorMap[i][0] * scale;
		colrs[i].green = GifScreen.ColorMap[i][1] * scale;
		colrs[i].blue = GifScreen.ColorMap[1][2] * scale;
		colrs[i].pixel = i;
		colrs[i].flags = DoRed | DoGreen | DoBlue;
	    }
	    for (i = GifScreen.BitPixel; i < MAXCOLORMAPSIZE; i++) {
		colrs[i].red = colrs[i].green = colrs[i].blue = 0;
		colrs[i].pixel = i;
		colrs[i].flags = DoRed | DoGreen | DoBlue;
	    }
	}

	if (GifScreen.AspectRatio && GifScreen.AspectRatio != 49) {
	    /* float r = ((float) GifScreen.AspectRatio + 15.0) / 64.0; */

#ifndef DISABLE_TRACE
	    if (srcTrace)
		fprintf(stderr, "Warning:  non-square pixels!\n");
#endif
	}
    }  /* End of first image only stuff */

    while (!image) {
	if (!ReadOK(fd, &c, 1))	{
#ifndef DISABLE_TRACE
	    if (srcTrace)
		fprintf(stderr,	"GIF: EOF / read error on image data\n");
#endif
	    return(NULL);
	}

	if (c == ';') {		/* GIF terminator */
	    if (!image) {
#ifndef DISABLE_TRACE
		if (srcTrace && (imageNumber == 1))
		    fprintf(stderr, "No GIF image in file\n");
#endif
		return(NULL);
	    }
	    break;
	}

	if (c == '!') { 	/* Extension */
	    if (!ReadOK(fd, &c, 1))
		return(NULL);
	    DoExtension(fd, c);
	    continue;
	}

	if (c != ',') {		/* Not a valid start character */
#ifndef DISABLE_TRACE
	    if (srcTrace)
		fprintf(stderr,	"Bogus character 0x%02x, ignoring\n", (int)c);
#endif
	    continue;
	}

	if (!ReadOK(fd, buf, 9))
	    return(NULL);

	useGlobalColormap = !BitSet(buf[8], LOCALCOLORMAP);

	*x = LM_to_uint(buf[0], buf[1]);
	*y = LM_to_uint(buf[2], buf[3]);
	*w = LM_to_uint(buf[4], buf[5]);
	*h = LM_to_uint(buf[6], buf[7]);

	if (!useGlobalColormap) {
	    int bitPixel = 1 << ((buf[8] & 0x07) + 1);

	    if (ReadColorMap(fd, bitPixel, localColorMap)) {
#ifndef DISABLE_TRACE
		if (srcTrace)
		    fprintf(stderr, "GIF: Error reading local colormap\n");
#endif
		return(NULL);
	    }
	    image = ReadImage(fd, *w, *h, colrs, bitPixel, localColorMap,
			      BitSet(buf[8], INTERLACE));
	} else {
	    image = ReadImage(fd, *w, *h, colrs,
			      GifScreen.BitPixel, GifScreen.ColorMap,
			      BitSet(buf[8], INTERLACE));
	}
    }
#ifndef DISABLE_TRACE
    if (srcTrace)
	fprintf(stderr, "with x = %d, y = %d\n", *x, *y);
#endif
    *bg = Gif89.transparent;
    *animated = Gif89.animated;
    *delay = Gif89.delayTime;
    *disposal = Gif89.disposal;

    return(image);
}

static int ReadColorMap(FILE *fd, int number,
			unsigned char buffer[MAXCOLORMAPSIZE][3])
{
	if (!ReadOK(fd, buffer[0], 3 * number)) {
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "Bad colormap\n");
#endif
		return(TRUE);
	}
	return FALSE;
}

static void DoExtension(FILE *fd, int label)
{
	static unsigned char buf[256];
	int count;
	char *str;

	switch (label) {
	    case 0x01:		/* Plain Text Extension */
		str = "Plain Text";
#ifdef notdef
		if (GetDataBlock(fd, buf) <= 0)
			;

		lpos   = LM_to_uint(buf[0], buf[1]);
		tpos   = LM_to_uint(buf[2], buf[3]);
		width  = LM_to_uint(buf[4], buf[5]);
		height = LM_to_uint(buf[6], buf[7]);
		cellw  = buf[8];
		cellh  = buf[9];
		foreground = buf[10];
		background = buf[11];

		while (GetDataBlock(fd, buf) > 0) {
			PPM_ASSIGN(image[ypos][xpos],
				   cmap[v][0],
				   cmap[v][1],
				   cmap[v][2]);
			index++;
		}
		return;
#else
		break;
#endif
	    case 0xff:		/* Application Extension */
		str = "Application";
		if ((count = GetDataBlock(fd, buf)) && (count == 11) &&
		    !strncmp((char *)buf, "NETSCAPE2.0", 11) &&
		    (count = GetDataBlock(fd, buf)) && (count == 3) &&
		    (buf[0] == 1))
			Gif89.animated = LM_to_uint(buf[1], buf[2]);
				
		break;
	    case 0xfe:		/* Comment Extension */
		str = "Comment";
		while ((count = GetDataBlock(fd, buf)) && (count > 0)) {
#ifndef DISABLE_TRACE
			if (srcTrace) {
				buf[count] = '\0';
				fprintf(stderr, "GIF comment: %s\n", buf);
			}
#endif
		}
		return;
	    case 0xf9:		/* Graphic Control Extension */
		str = "Graphic Control";
		(void) GetDataBlock(fd, buf);
		Gif89.disposal  = (buf[0] >> 2) & 0x7;
		Gif89.inputFlag = (buf[0] >> 1) & 0x1;
		Gif89.delayTime = LM_to_uint(buf[1], buf[2]);
		if (buf[0] & 0x1) {
			Gif89.transparent = (int)(buf[3]);
		} else {
			Gif89.transparent = -1;
		}
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "GIF bg: %d disposal: %d delay: %d\n",
				Gif89.transparent, Gif89.disposal,
				Gif89.delayTime);
#endif
		break;
	    default:
		str = "UNKNOWN";
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "UNKNOWN (0x%02x) extension\n", label);
#endif
		break;
	}
#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "Got a '%s' extension\n", str);
#endif
	while (GetDataBlock(fd, buf) > 0)
		;
	return;
}

static int ZeroDataBlock = FALSE;

static int GetDataBlock(FILE *fd, unsigned char *buf)
{
	unsigned char count = 0;

	if (!ReadOK(fd, &count, 1)) {
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "Error in getting DataBlock size\n");
#endif
		return -1;
	}
	ZeroDataBlock = (count == 0);

	if (count && (!ReadOK(fd, buf, count))) {
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "Error in reading DataBlock\n");
#endif
		return -1;
	}
	return((int)count);
}


/*
**  Pulled out of nextCode
*/
static int curbit, lastbit, get_done, last_byte;
static int return_clear;
/*
**  Out of nextLWZ
*/
static int stack[(1 << (MAX_LWZ_BITS)) * 2];
static int *sp;
static int code_size, set_code_size;
static int max_code, max_code_size;
static int clear_code, end_code;

static void initLWZ(int input_code_size)
{
	set_code_size = input_code_size;
	code_size = set_code_size + 1;
	clear_code = 1 << set_code_size;
	end_code = clear_code + 1;
	max_code_size = 2 * clear_code;
	max_code = clear_code + 2;

	curbit = lastbit = 0;
	last_byte = 2;
	get_done = FALSE;
	return_clear = TRUE;

	sp = stack;
}

static int nextCode(FILE *fd, int code_size)
{
	int i, j, ret, end;
	static unsigned char buf[280];
	static int maskTbl[16] = {
		0x0000, 0x0001, 0x0003, 0x0007,
		0x000f, 0x001f, 0x003f, 0x007f,
		0x00ff, 0x01ff, 0x03ff, 0x07ff,
		0x0fff, 0x1fff, 0x3fff, 0x7fff,
	};

	if (return_clear) {
		return_clear = FALSE;
		return clear_code;
	}

	end = curbit + code_size;

	if (end >= lastbit) {
		int count;

		if (get_done)
			return -1;
		buf[0] = buf[last_byte - 2];
		buf[1] = buf[last_byte - 1];

		if (!(count = GetDataBlock(fd, &buf[2])))
			get_done = TRUE;

		last_byte = 2 + count;
		curbit = (curbit - lastbit) + 16;
		lastbit = (2 + count) * 8;
		end = curbit + code_size;
	}

	j = end / 8;
	i = curbit / 8;

	if (i == j) {
		ret = (int)buf[i];
	} else if (i + 1 == j) {
		ret = (int)buf[i] | ((int)buf[i + 1] << 8);
	} else {
		ret = (int)buf[i] | ((int)buf[i + 1] << 8) |
		      ((int)buf[i + 2] << 16);
	}
	ret = (ret >> (curbit % 8)) & maskTbl[code_size];

	curbit += code_size;

	return ret;
}

#define readLWZ(fd) ((sp > stack) ? *--sp : nextLWZ(fd))

static int nextLWZ(FILE *fd)
{
	static int table[2][(1 << MAX_LWZ_BITS)];
	static int firstcode, oldcode;
	int code, incode;
	register int i;

	while ((code = nextCode(fd, code_size)) >= 0) {
	       if (code == clear_code) {
			/* Corrupt GIFs can make this happen */
			if (clear_code >= (1 << MAX_LWZ_BITS))
				return -2;

		        for (i = 0; i < clear_code; ++i) {
			       table[0][i] = 0;
			       table[1][i] = i;
		        }
		        for (; i < (1 << MAX_LWZ_BITS); ++i)
			       table[0][i] = table[1][i] = 0;
		        code_size = set_code_size + 1;
		        max_code_size = 2 * clear_code;
		        max_code = clear_code + 2;
		        sp = stack;
			do {
			       firstcode = oldcode = nextCode(fd, code_size);
			} while (firstcode == clear_code);

			return firstcode;
	       }
	       if (code == end_code) {
		        int count;
		        unsigned char buf[260];

			if (ZeroDataBlock)
				return -2;

		        while ((count = GetDataBlock(fd, buf)) > 0)
			       ;
#ifndef DISABLE_TRACE
		        if (count && srcTrace)
				fprintf(stderr,
				      "Missing EOD in data (common occurence)");
#endif
			return -2;
	       }

	       incode = code;

	       if (code >= max_code) {
		       *sp++ = firstcode;
		       code = oldcode;
	       }
	       while (code >= clear_code) {
		       *sp++ = table[1][code];
		       if (code == table[0][code])
			       return(code);
		       if ((int)sp >= ((int)stack + sizeof(stack)))
			       return(code);
		       code = table[0][code];
	       }

	       *sp++ = firstcode = table[1][code];

	       if ((code = max_code) < (1 << MAX_LWZ_BITS)) {
		       table[0][code] = oldcode;
		       table[1][code] = firstcode;
		       if ((++max_code >= max_code_size) &&
			   (max_code_size < (1 << MAX_LWZ_BITS))) {
			       max_code_size *= 2;
			       code_size++;
		       }
	       }
	       oldcode = incode;

	       if (sp > stack)
		       return *--sp;
	}
	return code;
}


static unsigned char *ReadImage(FILE *fd, int len, int height, XColor *colrs,
				int cmapSize,
				unsigned char cmap[MAXCOLORMAPSIZE][3],
				int interlace)
{
	unsigned char c;	
	int i, v;
	unsigned char *dp, *image;
	int size = len * height;

	/*
	**  Initialize the Compression routines
	*/
	if (!ReadOK(fd, &c, 1)) {
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr,
			       "EOF / read error on image data in ReadImage\n");
#endif
		return(NULL);
	}

	initLWZ(c);

	image = (unsigned char *)calloc(size, sizeof(char));
	if (!image) {
#ifndef DISABLE_TRACE
		if (srcTrace || reportBugs)
			fprintf(stderr,
				"Cannot allocate space for GIF image data\n");
#endif
		return(NULL);
	}

	for (v = 0; v < cmapSize; v++) {
		colrs[v].red   = cmap[v][0] * 0x101;
		colrs[v].green = cmap[v][1] * 0x101;
		colrs[v].blue  = cmap[v][2] * 0x101;
		colrs[v].pixel = v;
		colrs[v].flags = DoRed | DoGreen | DoBlue;
	}
	for (v = cmapSize; v < MAXCOLORMAPSIZE; v++) {
		colrs[v].red = colrs[v].green = colrs[v].blue = 0;
		colrs[v].pixel = v;
		colrs[v].flags = DoRed | DoGreen | DoBlue;
	}

#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "Reading %d by %d%s GIF image\n",
			len, height, interlace ? " interlaced" : "");
#endif
	if (interlace) {
		int pass = 0;
		int step = 8;
		int xpos = 0;
		int ypos = 0;

		for (i = 0; i < height; i++) {
			if (ypos < height) {
				dp = &image[len * ypos];
				for (xpos = 0; xpos < len; xpos++) {
					if ((v = readLWZ(fd)) < 0)
						goto fini;
					*dp++ = v;
				}
			}
			if ((ypos += step) >= height) {
				if (pass++ > 0)
					step /= 2;
				ypos = step / 2;
			}
		}
	} else {
		dp = image;
		for (i = 0; i < size; i++) {
			if ((v = readLWZ(fd)) < 0)
				goto fini;
			*dp++ = v;
		}
	}

 fini:
	if (readLWZ(fd) >= 0) {
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "Too much GIF data, ignoring extra\n");
#endif
	}
	return(image);
}

void Get_GIF_ScreenSize(int *w, int *h)
{
	*w = GifScreen.Width;
	*h = GifScreen.Height;
#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "GIF Screen width = %d, height = %d\n",
			GifScreen.Width, GifScreen.Height);
#endif
}
