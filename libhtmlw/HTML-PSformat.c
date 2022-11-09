/* HTML-PSformat.c -  Module for VMS Mosaic software
 *
 * Purpose:	to parse Hypertext widget contents into appropriate PostScript
 *
 * Author:	Ameet A. Raval & Frans van Hoesel & Andrew Ford
 *		(aar@gfdl.gov & hoesel@chem.rug.nl). 
 *
 * Institution: for Ameet A. Raval:
 *			Geophysical Fluid Dynamics Laboratory,
 *			National Oceanic and Atmospheric Administration,
 *			U.S. Department of Commerce
 *			P.O. Box 308
 *			Princeton, NJ 08542
 *		for Frans van Hoesel:
 *			Xtreme graphics software
 *			Herepoortenmolendrift 36
 *			9711 DH  Groningen
 *			The Netherlands
 *		also:
 *			Andrew Ford
 *			Independent Software Consultant
 *			30 Upper Cheltenham Place,
 *			Montpelier, Bristol, BS6 5HR, GB
 *			E-mail: andrew@icarus.demon.co.uk
 *
 * Date:		1 aug 1993 
 * Modification:	8 nov 1993
 *				o added support for bold/italics courier
 *				o removed unused or no longer needed stuff
 *				o fixed the font alignment problem
 *		 	23 nov 1993
 *				o added support for horizontal ruler
 *				o on request of Ameet, added a specific
 *				  line about where to send bugreports to
 *			15 jun 1994
 *				o add headers, footers and footnotes to convey
 *				  title, page number, url of document, date (of
 *				  printing) and urls of links.
 *				o use A4 or US Letter size paper (currently
 *				  hard coded)
 *			9 may 1995 (Andrew Ford)
 *				o general overhaul
 *			8 mar 1999 (George Cook)
 *				o major rework for VMS Mosaic 3.3
 *			29 mar 2006 (George Cook)
 *				o added form content and Symbol font support
 *				  for VMS Mosaic 4.0
 *
 * Copyright:   This work is based on a product of the United States Government,
 *		and is precluded from copyright protection.  It is hereby
 *		released into the public domain.
 *
 * WE MAKE NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED
 * WARRANTY.  WE SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE
 * USERS OF THIS SOFTWARE. 
 * 
 *		Pieces of code are taken from xvps by kind
 *		permission of John Bradley.
 *
 * mMosaic changes by szukw000@mail.uni-mainz.de (april - may 2001)
 *
 */
#include "../config.h"
#include <stdarg.h>

#include <string.h>
#ifdef VMS
#include <stdlib.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#ifdef __bsdi__
#include <sys/malloc.h>
#else
#ifndef VMS
#include <malloc.h>
#endif
#endif
#include <time.h>
#include <sys/types.h>
#include "HTMLP.h"
#include "HTMLPutil.h"
#include "HTMLwidgets.h"
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

/* Fix thanks to robm. */
#if defined(__alpha) && !defined(VMS)
#include <Xm/VaSimpleP.h>
#endif

#define CR '\015'
#define LF '\012'

typedef enum {
	SET,
	RESET,
	DEFAULT,
	SET_DEFAULT
} SetType;

#ifndef DISABLE_TRACE
extern int htmlwTrace;
#endif

extern char *ident_ver;

#define USLETTER 	0

#define F_GREYSCALE	1
#define F_BWDITHER 	2
#define F_COLOR  	3

#define B_SLASH		'\\'
#define MAX_ASCII	'\177'

/* MONO returns total intensity of r,g,b components .33R + .5G + .17B */
#define MONO(rd, gn, bl) ((rd * 11 + gn * 16 + bl * 5) >> 13)

/* PSconst_out outputs to the postscript buffer an array of constant
 * strings
 */
#define PSconst_out(txt) {				\
	int n = sizeof(txt) / sizeof(txt[0]);		\
	int i; 						\
							\
	for (i = 0; i < n; i++)				\
		PSprintf("%s\n", txt[i]) ; 		\
}
	
/* For regular-font, bold-font, italic-font, fixed-font */
typedef enum { RF, BF, IF, FF, FB, FI, BI } PS_fontstyle;

static int PS_size, PS_len, PS_offset, PS_curr_page, PS_start_y, PS_hexi;
static char *PS_string;
static float Points_Pixel;
static int Pixels_Page;
static PS_fontstyle PS_oldfn = RF;
static int PS_oldfs = 0;

static XColor fg_color, bg_color;

static int footnote_space  = 8;		/* Space from main text to footnotes */
static int footnote_ptsize = 8;		/* Point size for footnote text */
static int cur_ftn_no;			/* Current footnote number */
static int n_saved_ftns;		/* Number of saved footnotes on page */
static int ftn_array_size = 0;		/* Size of allocated footnote array */
static char **footnotes = NULL;		/* Pointer array of footnote pointers */

static int colorimage_defined;		/* Have we called PScolor_image? */

typedef struct {
    double page_height;
    double page_width;
    double top_margin;
    double bot_margin;
    double left_margin;
    double right_margin;
    double text_height;
    double text_width;
} PAGE_DIMENS_T;

#define INCH	72
#define MM	INCH / 25.4

static PAGE_DIMENS_T page_dimens;
static PAGE_DIMENS_T a4_page_dimens = {
    297 * MM,
    210 * MM, 
     20 * MM,
     20 * MM,
     20 * MM,
     20 * MM
};

static PAGE_DIMENS_T us_letter_page_dimens = {
    11  * INCH,		/* page_height */
    8.5 * INCH,		/* page_width */
    0.9 * INCH,		/* top_margin */
    0.7 * INCH,
    0.9 * INCH,
    0.9 * INCH
};

/* Globals to get button value in print dialog */
int HTML_Print_Headers = 1;	/* Flag whether page headers enabled */
int HTML_Print_Footers = 1;	/* Flag whether footnote printing enabled */
int HTML_Print_Duplex  = 1;	/* Flag whether to print duplex */

/* Paper format (currently either A4 or letter).  This should be generalized. */
int HTML_Print_Paper_Size_A4 = USLETTER;

extern int installed_colormap;
extern Colormap installed_cmap;

static char *fontname[] = {
	/* In order: regular, bold, italic, bolditalic */
	"Times-Roman", "Times-Bold", "Times-Italic", "Times-BoldItalic",
	"Helvetica", "Helvetica-Bold", "Helvetica-Oblique",
	"Helvetica-BoldOblique",
	"NewCenturySchlbk-Roman", "NewCenturySchlbk-Bold",
	"NewCenturySchlbk-Italic", "NewCenturySchlbk-BoldItalic",
	/* This is a nasty trick, I have put Times in place of
	 * Lucida, because most printers don't have Lucida font
	 */
	"Times-Roman", "Times-Bold", "Times-Italic", "Times-BoldItalic",
	/* "Lucida", "Lucida-Bold", "Lucida-Italic" */
	"Symbol", "Symbol", "Symbol", "Symbol"
};

/*
 * GetDpi - return Dots-per-inch of the screen
 *
 * Calculate the pixel density in dots per inch on the current widget screen
 *
 */
static float GetDpi(HTMLWidget hw)
{
    Screen *s = XtScreen(hw);
    float dpi;

    dpi = 25.4 * WidthOfScreen(s) / WidthMMOfScreen(s);
    if (dpi < 1.0 || dpi > 10000.0)
	dpi = 72.0;
    return dpi;
}


/*
 * PSprintf - dynamic string concatenation function.
 *
 *  In successive calls, the formatted string will be appended to the global
 *  output string Sp.
 *  It assumes that on each call, the length of the text appended to Sp
 *  is less than 1024.
 *  The format string is used as in printf, so you can use additional
 *  arguments.
 *
 *  When successful, PSprintf returns the number of characters printed
 *  in this call, otherwise it returns EOF (just as printf does)
 *
 */
static int PSprintf(char *format, ...)
{
    int	len;
    char *s;
    va_list args;

    if (PS_size - PS_len < 1024) {
	PS_size += 1024;
	if (!(s = (char *) realloc(PS_string, PS_size))) {
#ifndef DISABLE_TRACE
	    if (htmlwTrace)
		fprintf(stderr, "PSprintf malloc failed\n");
#endif
	    return(EOF);
	}
	PS_string = s;
    }
    va_start(args, format);
    len = vsprintf(PS_string + PS_len, format, args);
    /* This is a hack to make it work on systems where vsprintf(s,...)
     * returns s, instead of the len.
     */
    if ((len != EOF) && len)
	PS_len += strlen(PS_string + PS_len);
    va_end(args);
    return(len);
}


/*
 * PShex - output hex byte
 *
 * Append the byte "val" to an internal string buffer in hexadecimal
 * format.  If the argument "flush" is True, or if the buffer has filled
 * up, flush the buffer to the larger postscript output buffer (using
 * PSprintf).
 *
 */
static int PShex(unsigned char val, int flush)
{
    static unsigned char hexline[80];
    static char digit[] = "0123456789abcdef";

    if (!flush) {
	hexline[PS_hexi++] = (char) digit[((unsigned) val >> (unsigned) 4) &
					  (unsigned) 0x0f];
	hexline[PS_hexi++] = (char) digit[(unsigned) val & (unsigned) 0x0f];
    }

    /* Changed from "> 78" to "> 77" on advice of
     * debra@info.win.tue.nl (Paul De Bra).
     */
    if ((flush && PS_hexi) || (PS_hexi > 77)) {
	hexline[PS_hexi] = '\0';
	PS_hexi = 0;
	return(PSprintf("%s\n", hexline));
    }
    return(0);
}


/*
 * PSfont - change font
 *
 * Change local font in buf to "font"
 * fontfamily indicates if the overall style is times, helvetica, century
 * schoolbook or lucida.
 *
 */
static void PSfont(SetType set, CurFontFamily fontfamily,
		   CurFontType type, int size)
{
    PS_fontstyle fn;
    int style, fs;
    int newfam = 0;
    static CurFontFamily oldfam = -1;
    static CurFontFamily savefam = -1;
    static CurFontFamily default_fam = TIMES;
    static CurFontType default_type = FONT;
    static int default_size = 3;
    static int savefs = 0;
    static PS_fontstyle savefn;

    /* reg, ital, bold, fix,  h1,  h2,  h3,  h4,  h5,  h6,
     * address, plain, list, fixbold, fixital, plabold, plaital, boldital
     */
    static PS_fontstyle fontstyle[18] = {
	RF, IF, BF, FF, BF, BF, BF, BF, BF,
	BF, IF, FF, FF, FB, FI, FB, FI, BI
    };

    static char fnchar[7][3] = {"RF", "BF", "IF", "FF", "FB", "FI", "BI"};

    /* Font sizes as set in HTMLFONT.C
     */
    static int fontsizes[5][7][18] = {
#ifdef VMS /* VMS doesn't have some sizes (e.g. 20 fixed (i.e., courier)) */
     /* times font sizes (no 20 italic) */
     {
       {10, 10, 10, 10, 18, 17, 14, 12, 10, 8, 10, 10, 8, 10, 10, 10, 10, 10},
       {14, 14, 14, 14, 20, 18, 14, 12, 10, 8, 14, 12, 10, 14, 14, 12, 12, 14},
       {17, 17, 17, 17, 24, 20, 17, 14, 12, 10, 17, 14, 12, 17, 17, 14, 14, 17},
       {18, 18, 18, 18, 24, 20, 18, 17, 14, 12, 18, 17, 14, 18, 18, 17, 17, 18},
       {20, 18, 20, 18, 25, 24, 20, 18, 17, 14, 18, 18, 17, 18, 18, 18, 18, 18},
       {24, 24, 24, 24, 34, 25, 24, 20, 18, 17, 24, 18, 18, 24, 24, 18, 18, 24},
       {34, 34, 34, 34, 34, 34, 25, 24, 20, 18, 34, 24, 18, 34, 34, 24, 24, 34}
     },
     /* helvetica sizes (no 20 italic) */
     {
       {10, 10, 10, 10, 18, 17, 14, 12, 10, 8, 10, 10, 8, 10, 10, 10, 10, 10},
       {14, 14, 14, 14, 20, 18, 14, 12, 10, 8, 14, 12, 10, 14, 14, 12, 12, 14},
       {17, 17, 17, 17, 24, 20, 17, 14, 12, 10, 17, 14, 12, 17, 17, 14, 14, 17},
       {18, 18, 18, 18, 24, 20, 18, 17, 14, 12, 18, 17, 14, 18, 18, 17, 17, 18},
       {20, 18, 20, 18, 25, 24, 20, 18, 17, 14, 18, 18, 17, 18, 18, 18, 18, 18},
       {24, 24, 24, 24, 34, 25, 24, 20, 18, 17, 24, 18, 18, 24, 24, 18, 18, 24},
       {34, 34, 34, 34, 34, 34, 25, 24, 20, 18, 34, 24, 18, 34, 34, 24, 24, 34}
     },
     /* new century schoolbook (no 20) */
     {
       {10, 10, 10, 10, 18, 17, 14, 12, 10, 8, 10, 10, 8, 10, 10, 10, 10, 10},
       {14, 14, 14, 14, 18, 18, 14, 12, 10, 8, 14, 12, 10, 14, 14, 12, 12, 14},
       {17, 17, 17, 17, 24, 18, 17, 14, 12, 10, 17, 14, 12, 17, 17, 14, 14, 17},
       {18, 18, 18, 18, 24, 18, 18, 17, 14, 12, 18, 17, 14, 18, 18, 17, 17, 18},
       {18, 18, 18, 18, 25, 24, 18, 18, 17, 14, 18, 18, 17, 18, 18, 18, 18, 18},
       {24, 24, 24, 24, 34, 25, 24, 18, 18, 17, 24, 18, 18, 24, 24, 18, 18, 24},
       {34, 34, 34, 34, 34, 34, 25, 24, 18, 18, 34, 24, 18, 34, 34, 24, 24, 34}
     },
     /* lucida sizes (all but 20 fixed)*/
     {
       {10, 10, 10, 10, 18, 17, 14, 12, 10, 8, 10, 10, 8, 10, 10, 10, 10, 10},
       {14, 14, 14, 14, 20, 18, 14, 12, 10, 8, 14, 12, 10, 14, 14, 12, 12, 14},
       {17, 17, 17, 17, 24, 20, 17, 14, 12, 10, 17, 14, 12, 17, 17, 14, 14, 17},
       {18, 18, 18, 18, 24, 20, 18, 17, 14, 12, 18, 17, 14, 18, 18, 17, 17, 18},
       {20, 20, 20, 18, 25, 24, 20, 18, 17, 14, 20, 18, 17, 18, 18, 18, 18, 20},
       {24, 24, 24, 24, 34, 25, 24, 20, 18, 17, 24, 18, 18, 24, 24, 18, 18, 24},
       {34, 34, 34, 34, 34, 34, 25, 24, 20, 18, 34, 24, 20, 34, 34, 24, 24, 34}
     },
#else
     /* times font sizes */
     {
       {10, 10, 10, 10, 18, 17, 14, 12, 10, 8, 10, 10, 8, 10, 10, 10, 10, 10},
       {14, 14, 14, 14, 20, 18, 14, 12, 10, 8, 14, 12, 10, 14, 14, 12, 12, 14},
       {17, 17, 17, 17, 24, 20, 17, 14, 12, 10, 17, 14, 12, 17, 17, 14, 14, 17},
       {18, 18, 18, 18, 24, 20, 18, 17, 14, 12, 18, 17, 14, 18, 18, 17, 17, 18},
       {20, 20, 20, 20, 25, 24, 20, 18, 17, 14, 20, 18, 17, 20, 20, 18, 18, 20},
       {24, 24, 24, 24, 34, 25, 24, 20, 18, 17, 24, 20, 20, 24, 24, 20, 20, 24},
       {34, 34, 34, 34, 34, 34, 25, 24, 20, 18, 34, 24, 20, 34, 34, 24, 24, 34}
     },
     /* helvetica sizes */
     {
       {10, 10, 10, 10, 18, 17, 14, 12, 10, 8, 10, 10, 8, 10, 10, 10, 10, 10},
       {14, 14, 14, 14, 20, 18, 14, 12, 10, 8, 14, 12, 10, 14, 14, 12, 12, 14},
       {17, 17, 17, 17, 24, 20, 17, 14, 12, 10, 17, 14, 12, 17, 17, 14, 14, 17},
       {18, 18, 18, 18, 24, 20, 18, 17, 14, 12, 18, 17, 14, 18, 18, 17, 17, 18},
       {20, 20, 20, 20, 25, 24, 20, 18, 17, 14, 20, 18, 17, 20, 20, 18, 18, 20},
       {24, 24, 24, 24, 34, 25, 24, 20, 18, 17, 24, 20, 20, 24, 24, 20, 20, 24},
       {34, 34, 34, 34, 34, 34, 25, 24, 20, 18, 34, 24, 20, 34, 34, 24, 24, 34}
     },
     /* new century schoolbook sizes */
     {
       {10, 10, 10, 10, 18, 17, 14, 12, 10, 8, 10, 10, 8, 10, 10, 10, 10, 10},
       {14, 14, 14, 14, 20, 18, 14, 12, 10, 8, 14, 12, 10, 14, 14, 12, 12, 14},
       {17, 17, 17, 17, 24, 20, 17, 14, 12, 10, 17, 14, 12, 17, 17, 14, 14, 17},
       {18, 18, 18, 18, 24, 20, 18, 17, 14, 12, 18, 17, 14, 18, 18, 17, 17, 18},
       {20, 20, 20, 20, 25, 24, 20, 18, 17, 14, 20, 18, 17, 20, 20, 18, 18, 20},
       {24, 24, 24, 24, 34, 25, 24, 20, 18, 17, 24, 20, 20, 24, 24, 20, 20, 24},
       {34, 34, 34, 34, 34, 34, 25, 24, 20, 18, 34, 24, 20, 34, 34, 24, 24, 34}
     },
     /* lucida sizes */
     {
       {10, 10, 10, 10, 18, 17, 14, 12, 10, 8, 10, 10, 8, 10, 10, 10, 10, 10},
       {14, 14, 14, 14, 20, 18, 14, 12, 10, 8, 14, 12, 10, 14, 14, 12, 12, 14},
       {17, 17, 17, 17, 24, 20, 17, 14, 12, 10, 17, 14, 12, 17, 17, 14, 14, 17},
       {18, 18, 18, 18, 24, 20, 18, 17, 14, 12, 18, 17, 14, 18, 18, 17, 17, 18},
       {20, 20, 20, 20, 25, 24, 20, 18, 17, 14, 20, 18, 17, 20, 20, 18, 18, 20},
       {24, 24, 24, 24, 34, 25, 24, 20, 18, 17, 24, 20, 20, 24, 24, 20, 20, 24},
       {34, 34, 34, 34, 34, 34, 25, 24, 20, 18, 34, 24, 20, 34, 34, 24, 24, 34}
     },
#endif
     /* symbol sizes */
     {
       {10, 10, 10, 10, 18, 17, 14, 12, 10, 8, 10, 10, 8, 10, 10, 10, 10, 10},
       {14, 14, 14, 14, 20, 18, 14, 12, 10, 8, 14, 12, 10, 14, 14, 12, 12, 14},
       {17, 17, 17, 17, 24, 20, 17, 14, 12, 10, 17, 14, 12, 17, 17, 14, 14, 17},
       {18, 18, 18, 18, 24, 20, 18, 17, 14, 12, 18, 17, 14, 18, 18, 17, 17, 18},
       {20, 20, 20, 20, 25, 24, 20, 18, 17, 14, 20, 18, 17, 20, 20, 18, 18, 20},
       {24, 24, 24, 24, 34, 25, 24, 20, 18, 17, 24, 20, 20, 24, 24, 20, 20, 24},
       {34, 34, 34, 34, 34, 34, 25, 24, 20, 18, 34, 24, 20, 34, 34, 24, 24, 34}
     }
    };

    switch (set) {
        case SET_DEFAULT:
	    /* Just save the default and return */
	    default_fam = fontfamily;
	    default_type = type;
	    default_size = size;
	    return;
        case DEFAULT:
	    /* Set current font to the default */
	    fontfamily = default_fam;
	    type = default_type;
	    size = default_size;
	    break;
        case RESET:
            /* Reset to previous font */
	    if (savefs) {
	        PSprintf("%2s %d SF\n", fnchar[savefn], savefs);
		PS_oldfn = savefn;
		PS_oldfs = savefs;
	    }
	    if (savefam != -1) {
	        PSprintf("/RF {/%s} D\n", fontname[savefam * 4]);
	        PSprintf("/BF {/%s} D\n", fontname[savefam * 4 + 1]);
	        PSprintf("/IF {/%s} D\n", fontname[savefam * 4 + 2]);
	        PSprintf("/BI {/%s} D\n", fontname[savefam * 4 + 3]);
		oldfam = savefam;
	    }
	    return;
	case SET:
	    break;
    }

    /* Set definitions to correct font family */
    if (oldfam != fontfamily) {
	PSprintf("/RF {/%s} D\n", fontname[fontfamily * 4]);
	PSprintf("/BF {/%s} D\n", fontname[fontfamily * 4 + 1]);
	PSprintf("/IF {/%s} D\n", fontname[fontfamily * 4 + 2]);
	PSprintf("/BI {/%s} D\n", fontname[fontfamily * 4 + 3]);
	savefam = oldfam;
	oldfam = fontfamily;
	newfam = 1;
    }

    /* Added the next line in case xmosaic version 199.4 has more fonts */
    style = 0;

    switch (type) {
        case FONT:
	    style = 0;
	    break;
        case ITALIC_FONT:
	    style = 1;
	    break;
        case BOLD_FONT:
	    style = 2;
	    break;
        case FIXED_FONT:
	    style = 3;
	    break;
        case HEADER1_FONT:
	    style = 4;
	    break;
        case HEADER2_FONT:
	    style = 5;
	    break;
        case HEADER3_FONT:
	    style = 6;
	    break;
        case HEADER4_FONT:
	    style = 7;
	    break;
        case HEADER5_FONT:
	    style = 8;
	    break;
        case HEADER6_FONT:
	    style = 9;
	    break;
        case ADDRESS_FONT:
	    style = 10;
	    break;
        case PLAIN_FONT:
	    style = 11;
	    break;
        case LISTING_FONT:
	    style = 12;
	    break;
        case FIXEDBOLD_FONT:
	    style = 13;
	    break;
        case FIXEDITALIC_FONT:
	    style = 14;
	    break;
        case PLAINBOLD_FONT:
	    style = 15;
	    break;
        case PLAINITALIC_FONT:
	    style = 16;
	    break;
        case BOLDITALIC_FONT:
	    style = 17;
    }

    if (--size < 0) {
	size = 0;
    } else if (size > 6) {
	size = 6;
    }

    fn = fontstyle[style];
    fs = fontsizes[fontfamily][size][style];

    if (newfam || (fn != PS_oldfn) || (fs != PS_oldfs)) {
	PSprintf("%2s %d SF\n", fnchar[fn], fs);
	savefn = PS_oldfn;
	savefs = PS_oldfs;
	PS_oldfn = fn;
	PS_oldfs = fs;
    }
}


/*
 * PSshowpage - end of page function
 *
 * Show the current page and restore any changes to the printer state.
 * Any accumulated footnotes are output and the outstanding footnote count
 * reset to zero.  Footnotes are preceded by a footnote rule and each footnote
 * is consists of a raised mark and the footnote text (i.e., the url).  The
 * mark is in a smaller font than the text.  The ideas are filched from LaTeX. 
 */
static void PSshowpage(void)
{
    PSprintf("restore\n");
    if (n_saved_ftns > 0) {
	int i;

	PSprintf(
	        "gsave 0.2 setlinewidth newpath %.2f %.2f M %.2f 0 RL stroke\n",
		page_dimens.left_margin,
		page_dimens.bot_margin + (footnote_ptsize * n_saved_ftns) + 4,
		page_dimens.text_width * 0.4);
	for (i = 0; n_saved_ftns; n_saved_ftns--, i++) {
	    PSprintf("newpath %.2f %.2f M RF %.2f SF (%d) S 3 -2 R RF %d SF\n",
	    	     page_dimens.left_margin,
	    	     page_dimens.bot_margin + 5 + (n_saved_ftns - 1) *
			footnote_ptsize,
		     0.7 * footnote_ptsize, cur_ftn_no - n_saved_ftns,
	    	     footnote_ptsize);
	    PSprintf("(%.120s) S\n", footnotes[i]);
	}
	PSprintf("grestore\n");
    }
    PSprintf("showpage\n");
}


/*
 * PSnewpage - begin a fresh page
 *
 * Increment the page count and handle the structured comment
 * conventions.
 *
 */
static void PSnewpage(void) 
{
    PS_curr_page++;
    
    /* The PostScript reference Manual states that the Page: Tag
     * should have a label and an ordinal; otherwise programs like
     * psutils fail    -gustaf
     */
    PSprintf("%%%%Page: %d %d\nsave\n", PS_curr_page, PS_curr_page);
    if (HTML_Print_Headers)
	PSprintf("%d ", PS_curr_page);
    PSprintf("NP\n");
    PSfont(DEFAULT, TIMES, FONT, 3);	  /* Reset to default */
}


/*
 * PSinit_latin1 - handle ISO encoding
 *
 * Print out initializing PostScript text for ISO Latin1 font encoding.
 * This code is copied from the Idraw program (from Stanford's InterViews 
 * package), courtesy of Steinar Kjaernsr|d, steinar@ifi.uio.no
 *
 */
static void PSinit_latin1(void)
{
    static char *txt[] = {
	"/reencodeISO {",
	"dup dup findfont dup length dict begin",
	"{ 1 index /FID ne { def }{ pop pop } ifelse } forall",
	"/Encoding ISOLatin1Encoding D",
	"currentdict end definefont",
	"} D",
	"/ISOLatin1Encoding [",
	"/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef",
	"/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef",
	"/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef",
	"/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef",
	"/space/exclam/quotedbl/numbersign/dollar/percent/ampersand/quoteright",
	"/parenleft/parenright/asterisk/plus/comma/minus/period/slash",
	"/zero/one/two/three/four/five/six/seven/eight/nine/colon/semicolon",
	"/less/equal/greater/question/at/A/B/C/D/E/F/G/H/I/J/K/L/M/N",
	"/O/P/Q/R/S/T/U/V/W/X/Y/Z/bracketleft/backslash/bracketright",
	"/asciicircum/underscore/quoteleft/a/b/c/d/e/f/g/h/i/j/k/l/m",
	"/n/o/p/q/r/s/t/u/v/w/x/y/z/braceleft/bar/braceright/asciitilde",
	"/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef",
	"/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef",
	"/.notdef/dotlessi/grave/acute/circumflex/tilde/macron/breve",
	"/dotaccent/dieresis/.notdef/ring/cedilla/.notdef/hungarumlaut",
	"/ogonek/caron/space/exclamdown/cent/sterling/currency/yen/brokenbar",
	"/section/dieresis/copyright/ordfeminine/guillemotleft/logicalnot",
	"/hyphen/registered/macron/degree/plusminus/twosuperior/threesuperior",
	"/acute/mu/paragraph/periodcentered/cedilla/onesuperior/ordmasculine",
	"/guillemotright/onequarter/onehalf/threequarters/questiondown",
	"/Agrave/Aacute/Acircumflex/Atilde/Adieresis/Aring/AE/Ccedilla",
	"/Egrave/Eacute/Ecircumflex/Edieresis/Igrave/Iacute/Icircumflex",
	"/Idieresis/Eth/Ntilde/Ograve/Oacute/Ocircumflex/Otilde/Odieresis",
	"/multiply/Oslash/Ugrave/Uacute/Ucircumflex/Udieresis/Yacute",
	"/Thorn/germandbls/agrave/aacute/acircumflex/atilde/adieresis",
	"/aring/ae/ccedilla/egrave/eacute/ecircumflex/edieresis/igrave",
	"/iacute/icircumflex/idieresis/eth/ntilde/ograve/oacute/ocircumflex",
	"/otilde/odieresis/divide/oslash/ugrave/uacute/ucircumflex/udieresis",
	"/yacute/thorn/ydieresis",
	"] D",
	"[RF BF IF BI RF2 BF2 IF2 BI2 RF3 BF3 IF3 BI3 RF4 BF4 IF4 BI4",
	" FF FB FI] {reencodeISO D} forall"
    };

    PSconst_out(txt);
}


/*
 * PSinit - initialize Postscript output
 *
 * Does the initialization per html document
 *
 */
static void PSinit(void) 
{
    PS_size = PS_len = PS_offset = PS_hexi = 0;
    PS_start_y = 0;
    PS_string = (char *) malloc(1);
    PS_oldfs = 0;
    PS_oldfn = RF;
    PS_curr_page = 0;
    n_saved_ftns = 0;
    cur_ftn_no = 1;
    colorimage_defined = 0;
}


/*
 * PSheader - initialize Postscript output
 *
 * Prints out the prolog.  The following PostScript macros are defined
 *	D	def - define a macro
 *	E	exch - exhange parameters
 *	M	moveto
 *	R	rmoveto
 *	L	lineto
 *	RL	rlineto
 *	SQ	draw a unit square
 *	U	underline a string
 *	DB	draw a disc bullet
 *	CB	draw a circle bullet
 *	SB	draw a square bullet
 *	BB	draw a block bullet
 *	HR	draw a horizontal rule
 *	SF	set font
 *	RF	roman font (dependent on font family)
 *	BF	bold font (dependent on font family)
 *	IF	italic font (dependent on font family)
 *	BI	bold italic font (dependent on font family)
 *	FF	fixed font (courier)
 *	FB	fixed bold font (courier bold)
 *	FI	fixed italic font (courier oblique)
 *	HBAR    draw a horizontal scrollbar
 *	VBAR    draw a vertical scrollbar
 *	nstr	buffer for creating page number string
 *	pageno	literal "Page "
 *	url	URL of document
 *	title	title of document
 *	date	date modified/printed
 */
static void PSheader(char *title, int font, char *url, char *time_str) 
{
    int free_title = 0;
    char time_buf[40];
    time_t clock = time(NULL);
    static char *txt[] = {
	"/M {moveto} D",
	"/S {show} D",
	"/R {rmoveto} D",
	"/L {lineto} D",
	"/RL {rlineto} D",
	"/SQ {newpath 0 0 M 0 1 L 1 1 L 1 0 L closepath} D",
	"/U {gsave currentpoint currentfont /FontInfo get",
	" /UnderlinePosition get",
	" 0 E currentfont /FontMatrix get dtransform E pop add newpath moveto",
	" dup stringwidth rlineto stroke grestore S } D",
	"/DB {/r E D gsave -13 0 R currentpoint",
	" newpath r 0 360 arc closepath fill grestore } D",
	"/CB {/r E D gsave -13 0 R currentpoint",
	" newpath r 0 360 arc closepath stroke grestore } D",
	"/SB {/r E D gsave -15 3 R 6 0 RL 0 -6 RL -6 0 RL 0 6 RL closepath",
	" stroke grestore} D",
	"/BB {/r E D gsave -14 3 R 5 0 RL 0 -5 RL -5 0 RL 0 5 RL closepath",
	" fill grestore} D",
	"/SBS {/r E D gsave -15 2 R 5 0 RL 0 -5 RL -5 0 RL 0 5 RL closepath",
	" stroke grestore} D",
	"/BBS {/r E D gsave -14 2 R 4 0 RL 0 -4 RL -4 0 RL 0 4 RL closepath",
	" fill grestore} D",
	"/HR {/myH E D /l E D gsave l 0 RL currentpoint stroke M 0 myH neg R",
	" l neg 0 RL 0.6 setgray stroke grestore } D",
	"/HR {/myH E D /l E D gsave 0 myH RL l 0 RL currentpoint 0.4 setgray",
	" stroke M 0 myH neg RL l neg 0 RL 0.6 setgray stroke grestore } D",
	"/HRF {/myH E D /l E D gsave 0 myH RL l 0 RL 0 myH neg RL l neg 0 RL",
	" closepath 0.6 setgray fill grestore } D",
	"/SF {E findfont E scalefont setfont } D",
	"/FF {/Courier } D",
	"/FB {/Courier-Bold } D",
	"/FI {/Courier-Oblique } D",
        "/HBAR {/Lng E D 6 6 RL 0 -2 RL Lng 0 RL 0 2 RL 6 -6 RL -6 -6 RL 0 2 RL\n",
        "Lng neg 0 RL 0 -2 RL -6 6 RL closepath gsave 0.5 setgray fill grestore} D",
        "/VBAR {/Lng E D 6 -6 RL -2 0 RL 0 Lng neg RL 2 0 RL -6 -6 RL -6 6 RL\n",
        "2 0 RL 0 Lng RL -2 0 RL 6 6 RL closepath gsave 0.5 setgray fill grestore} D"
	};

#if !defined(VMS) || defined (__alpha)
    strftime(time_buf, sizeof(time_buf), 
	     "Printed %a %b %e %T %Y", localtime(&clock));
#else
    sprintf(time_buf, "Printed %s", asctime(localtime(&clock)));
#endif

    /* Always show the print date */
    time_str = time_buf;

    PSprintf("%%!PS-Adobe-1.0\n");
    PSprintf(
        "%%%%Creator: VMS Mosaic V%s, Postscript by Ameet Raval,\n", ident_ver);
    PSprintf("%%%%         Frans van Hoesel, Andrew Ford and George Cook\n");

    if (!title) {
	title = strdup("Untitled");
	free_title = 1;
    }

    {
	char *tmp;

	for (tmp = title; *tmp; tmp++) {
	    if (*tmp == CR || *tmp == LF) {
                *tmp = ' ';
	    /* Prevent unbalanced () due to length truncation */
	    } else if (*tmp == '(') {
		*tmp = '[';
	    } else if (*tmp == ')') {
		*tmp = ']';
	    }
	}
    }

    PSprintf("%%%%Title: %s\n", title);
    PSprintf("%%%%CreationDate: %s\n", time_buf + 8);
    PSprintf("%%%%Pages: (atend)\n%%%%PageOrder: Ascend\n");
    PSprintf("%%%%BoundingBox: %d %d %d %d\n",
	     (int)page_dimens.left_margin,
	     (int)(page_dimens.bot_margin - 12),
	     (int)(page_dimens.left_margin + page_dimens.text_width + 0.5),
	     (int)(page_dimens.bot_margin + page_dimens.text_height + 12.5));
    PSprintf(
	"%%%%DocumentFonts: %s %s %s %s Courier Courier-Bold Courier-Oblique\n",
	fontname[font * 4], fontname[font * 4 + 1], fontname[font * 4 + 2],
	fontname[font * 4 + 3]);
    PSprintf("%%%%EndComments\nsave /D {def} def /E {exch} D\n");
    PSprintf("/RF {/%s} D\n", fontname[0]);
    PSprintf("/BF {/%s} D\n", fontname[1]);
    PSprintf("/IF {/%s} D\n", fontname[2]);
    PSprintf("/BI {/%s} D\n", fontname[2]);
    PSprintf("/RF2 {/%s} D\n", fontname[4]);
    PSprintf("/BF2 {/%s} D\n", fontname[5]);
    PSprintf("/IF2 {/%s} D\n", fontname[6]);
    PSprintf("/BI2 {/%s} D\n", fontname[7]);
    PSprintf("/RF3 {/%s} D\n", fontname[8]);
    PSprintf("/BF3 {/%s} D\n", fontname[9]);
    PSprintf("/IF3 {/%s} D\n", fontname[10]);
    PSprintf("/BI3 {/%s} D\n", fontname[11]);
    PSprintf("/RF4 {/%s} D\n", fontname[12]);
    PSprintf("/BF4 {/%s} D\n", fontname[13]);
    PSprintf("/IF4 {/%s} D\n", fontname[14]);
    PSprintf("/BI4 {/%s} D\n", fontname[15]);
    PSprintf("/RF5 {/%s} D\n", fontname[16]);
    PSprintf("/BF5 {/%s} D\n", fontname[17]);
    PSprintf("/IF5 {/%s} D\n", fontname[18]);
    PSprintf("/BI5 {/%s} D\n", fontname[19]);
    PSprintf("/nstr 6 string D /pgno (Page ) D\n");
    PSprintf("/url (%.64s) D\n", url);
    PSprintf("/title (%.64s) D\n", title);
    PSprintf("/date (%s) D\n", time_str);
    if (HTML_Print_Duplex)
        PSprintf("<</Duplex true>> setpagedevice");
    PSconst_out(txt);
    
    /* Output the newpage definition. */
    
    PSprintf("/NP {");
    if (HTML_Print_Headers) {
	PSprintf("gsave 0.4 setlinewidth\n");
	PSprintf("  newpath %.2f %.2f M %.2f 0 RL stroke",
		 page_dimens.left_margin,
		 page_dimens.bot_margin + page_dimens.text_height,
		 page_dimens.text_width);
	PSprintf("  newpath %.2f %.2f M %.2f 0 RL stroke\n",
		 page_dimens.left_margin, page_dimens.bot_margin,
		 page_dimens.text_width);
	PSprintf("  BF 12 SF %.2f %.2f M (%.64s) S\n",
		 page_dimens.left_margin,
		 page_dimens.bot_margin + page_dimens.text_height + 6, title);
	PSprintf("  nstr cvs dup stringwidth pop pgno stringwidth pop add\n");
	PSprintf("  %.2f E sub %.2f M pgno S S\n",
		 page_dimens.left_margin + page_dimens.text_width, 
		 page_dimens.bot_margin + page_dimens.text_height + 6);
	PSprintf("  BF 10 SF %.2f %.2f M (%.64s) S\n",
		 page_dimens.left_margin, page_dimens.bot_margin - 12, url);
	PSprintf("  (%s) dup stringwidth pop %.2f E sub %.2f M S grestore\n",
		 time_str, page_dimens.left_margin + page_dimens.text_width,
		 page_dimens.bot_margin - 12);
    }
    PSprintf("  %.2f %.2f translate %.5f %.5f scale } D\n",
	     page_dimens.left_margin, 
	     page_dimens.bot_margin + page_dimens.text_height,
	     Points_Pixel, Points_Pixel);
    PSinit_latin1();
    
    PSprintf("%%%%EndProlog\n");

    if (free_title)
	free(title);
}


/*
 * PStrailer - write postscript trailer
 *
 */
static void PStrailer(void)
{
    PSprintf("%%%%Trailer\nrestore\n%%%%Pages: %d\n", PS_curr_page);
    if (footnotes) {
	free(footnotes);
	footnotes = NULL;
	ftn_array_size = 0;
    }
}


/*
 * PSmoveto - move to new x,y location
 *
 * If the Y value does not fit on the current page, begin a new page
 * (I think in the current implementation, this never happens)
 *
 */
static void PSmoveto(int x, int y) 
{
    if (y > (PS_start_y + Pixels_Page)) {
	PS_start_y = y;
	PSshowpage();
	PSnewpage();
    }
    PS_offset = 0;
    PSprintf("%d %d M\n", x, -(y - PS_start_y));
}


/*
 * PSmove_offset - set Y-offset
 *
 * Do a relative vertical move, whenever the offset changes
 *
 */
static void PSmove_offset(int offset)
{
    if (offset != PS_offset) {
	PSprintf("0 %d R\n", PS_offset - offset);
	PS_offset = offset;
    }
}


/*
 * Return an indication of whether or not the current element has a footnote.
 *
 * An element has a footnote if it is text or an image and its anchorHRef is
 * not null.  If the element is a textual element with an anchorHRef, that has
 * been split across lines then it should be followed by a linefeed element
 * and a text element with the same anchorHRef.  In this case say that the
 * element doesn't have a footnote so as to avoid duplicate footnotes.
 */
static int has_footnote(struct ele_rec *el)
{
    int	rc = 0;
    char *anchorHRef;

    if (!el)
	return 0;

    anchorHRef = el->anchor_tag_ptr->anc_href;

    /* Ignore internal anchors */
    if (anchorHRef && (*anchorHRef != '#')) {
	struct ele_rec *next;

	switch (el->type) {
	    case E_TEXT:
	    case E_IMAGE:
	        for (next = el->next; el; el = next, next = el->next) {
		    if (!next || !next->anchor_tag_ptr->anc_href ||
		        strcmp(next->anchor_tag_ptr->anc_href, anchorHRef)) {
		        rc = 1;
			break;
		    } else if ((next->type == E_TEXT) ||
			       (next->type == E_IMAGE)) {
			break;
		    }
	        }
	    default:
	        break;
	}
    }
    return rc;
}


/*
 * PSfootnote - output a footnote mark and store the footnote
 *
 * The footnote mark is placed at the current point, enclosed in a
 * gsave/grestore pair so that the position of the following output
 * is not affected.
 * The reference is stored in a malloced array (which may need to be
 * expanded), to be output at the end of the page.
 */
static void PSfootnote(char *href, double height)
{
    PSprintf("gsave 0 %.2f R RF %d SF (%d) S grestore\n", 
	     height, footnote_ptsize, cur_ftn_no++);

    if (n_saved_ftns == ftn_array_size) {
	ftn_array_size += 16;
	if (!footnotes) {
	    footnotes = (char **)calloc(ftn_array_size, sizeof(char *));
	} else {
	    footnotes = (char **)realloc((void *)footnotes,
				         ftn_array_size * sizeof(char *));
	}
	if (!footnotes) {
#ifndef DISABLE_TRACE
	    if (htmlwTrace)
		fprintf(stderr, "PSfootnote realloc failed\n");
#endif
	    return;
	}
    }
    footnotes[n_saved_ftns++] = href;
}


/*
 * PStext - output text
 *
 * Show text "t" and protect special characters if needed.
 * If Underline is non-zero, the text is underlined.
 *
 */
static void PStext(HTMLWidget hw, struct ele_rec *eptr, String s)
{
    String s2, stmp;
    unsigned char ch;
    int underline = eptr->underline_number;
			  
    /* Set font */
    PSfont(SET, eptr->font_family, eptr->font_type, eptr->font_size);
    PSmove_offset(eptr->baseline);

    /* Allocate a string long enough to hold the original string with
     * every character stored as an octal escape (worst case scenario). */
    s2 = (String) malloc(strlen(s) * 4 + 1);
    if (!s2) {
#ifndef DISABLE_TRACE
	if (htmlwTrace)
	    fprintf(stderr, "PStext malloc failed\n");
#endif
	return;
    }

    /*  For each char in s, if it is a special char, insert "\"
     *  into the new string s2, then insert the actual char
     */
    for (stmp = s2; ch = *s++; ) {
	if ((ch == '(') || (ch == ')') || (ch == B_SLASH)) {
	    *stmp++ = B_SLASH;
	    *stmp++ = ch;
	} else if (ch > (unsigned char) MAX_ASCII) {
	    /* Convert to octal */
	    *stmp++ = B_SLASH;
	    *stmp++ = ((ch >> 6) & 007) + '0';
	    *stmp++ = ((ch >> 3) & 007) + '0';
	    *stmp++ = (ch & 007) + '0';
	} else {
	    *stmp++ = ch;
	}
    }
    *stmp = '\0';
    PSprintf("(%s)%c\n", s2, underline ? 'U' : 'S');
    if (HTML_Print_Footers && has_footnote(eptr))
	PSfootnote(eptr->anchor_tag_ptr->anc_href, 0.7 * eptr->font->ascent);
    free(s2);
}


/*
 * PSbullet - output a bullet
 *
 * The bullet is normally filled, except for a bullet with an indent level
 * of two.  The size of the higher level bullets is just somewhat smaller
 *
 */
static void PSbullet(HTMLWidget hw, struct ele_rec *eptr)
{
    int	width = eptr->font->max_bounds.lbearing +
	        eptr->font->max_bounds.rbearing;
    int	offset = eptr->baseline;
    int	level = eptr->indent_level;
    double size = eptr->height / 5.5;
    char *type;

    if (size < 1.1)
	size = 1.1;
    if (level > 4) 
	size /= 1.33333;

    /* The next line is a hack to get a good position of the
     * bullet in most practical cases, otherwise the
     * bullet may appear just a bit too low (for large fonts).
     * What it does is to compare the lineheight with
     * the lineheight of the next element, to correct
     * for the possibly too large y offset.
     */
    if (eptr->next && ((eptr->next->type == E_TEXT) ||
		       (eptr->next->type == E_IMAGE)))
	offset += eptr->height - eptr->next->height;

    if (level && (level % 2)) {
	if (level == 1 || (level % 4) == 1) {
	    type = "DB";
	} else {
	    type = "CB";
	}
    } else if (level == 0 || (level % 4) == 2) {
	if (level > 4) {
	    type = "BBS";
	} else {
	    type = "BB";
	}
    } else {
	if (level > 4) {
	    type = "SBS";
	} else {
	    type = "SB";
	}
    }

    PSfont(SET, eptr->font_family, eptr->font_type, eptr->font_size);
    PSmove_offset(offset - width / 4);
    PSprintf(" %f %s\n", size, type);
}


/*
 * PShrule - draw a horizontal line with the given width
 *
 */
static void PShrule(struct ele_rec *eptr)
{
    int width = eptr->width + (eptr->width * 0.025);

    if (eptr->bwidth) {
	/* Shaded */
        PSprintf("%d %d HR\n", width, eptr->height);
    } else {
        PSprintf("%d %d HRF\n", width, eptr->height);
    }
}


/*
 * PStable - draw a table
 *
 * Currently does nothing
 *
 */
static void PStable(struct ele_rec *eptr)
{
#if 0
    struct _TableRec *tptr = eptr->table_data;
    int width  = tptr->width;
    int height = tptr->height;

    PSmove_offset(eptr->baseline);
    PSprintf("gsave currentpoint %d sub translate ", height);
    PSprintf("%d %d scale\n", width, height);
    PSprintf("SQ stroke\n");
    PSprintf("grestore\n");
#endif
}


/*
 * PSwidget - draw a widget (form field)
 *
 * Currently just draw a grey box of the dimensions of the field.
 * This is nowhere near complete but is a first step.
 * The widget record type field gives the type of field:
 *	0	textfield
 *	1	checkbox
 *	2	radiobox
 *	3	pushbutton
 *	4	password
 *	5	option menu
 */
static void PSwidget(struct ele_rec *eptr)
{
    char **Info = NULL;
    char *Txt;
    int w, i, Type;
    int NrInfo = 0;
    int Center = 0;
    int OptionMenu = 0;
    int Hbar = 0;
    int Vbar = 0;
    float Width, Height, Xpos, Ypos, Leng, Ascent;
    float Extra = 0;
    struct wid_rec *wptr = eptr->widget_data;
    CurFontType style = FONT;

    Width = (float)wptr->width - 2.0;
    Height = (float)wptr->height - 2.0;
    Type = wptr->type;
    Ascent = eptr->font->ascent;

    switch(Type) {
	case W_CHECKBOX:
	case W_RADIOBOX:
	    if (XmToggleButtonGetState(wptr->w)) {
		Info = (char **)malloc(sizeof(char *));
		Info[0] = "x";
		NrInfo = 1;
		Center = 1;
		style = BOLD_FONT;
		Extra = 3.0;
	    }
	    break;
	case W_PUSHBUTTON:
	    if (wptr->value) {
		Info = (char**)malloc(sizeof(char *));
		Info[0] = wptr->value;
		NrInfo = 1;
		Center = 1;
	    }
	    break;
	case W_TEXTFIELD:
	    Txt = XmTextFieldGetString(wptr->w);
	    if (Txt && *Txt) {
		Info = (char **)malloc(sizeof(char *));
		Info[0] = Txt;
		NrInfo = 1;
		Center = 1;
		style = FIXED_FONT;
	    } else if (Txt) {
		XtFree(Txt);
	    }
	    break;
	case W_TEXTAREA:
	    if (wptr->child && XmIsText(wptr->child)) {
		int NrRows = 0;
		int NrCols = 0;
		int Col;
		char *Start, *End;

		Txt = NULL;
		XtVaGetValues(wptr->child,
			      XmNcolumns, &NrCols,
			      XmNrows, &NrRows,
			      XmNvalue, &Txt,
			      NULL);
		if (Txt && *Txt) {
		    Info = (char **)calloc(NrRows, sizeof(char *));
		    i = -1; 
		    Start = Txt;

		    while (++i < NrRows) {
			End = Start;
			Col = 0;
			while (*End && *End != '\n') { 
			    End++;
			    if (++Col == NrCols)
				break; 
			}
			if (Col == 0)
			    break;
			Info[i] = (char *)calloc(Col + 1, sizeof(char));
			strncpy(Info[i], Start, Col);
			NrInfo++;
			Start = End;
			if (*Start == '\n')
			    Start++;
	 	    }
		    style = FIXED_FONT;
		}
		if (Txt)
		    XtFree(Txt);
	    }
	    Hbar = Vbar = 1;
	    break;
	case W_LIST:
	    if (wptr->child && XmIsList(wptr->child)) {
		XmStringTable Items;
		int NrItems = 0;
		int NrVisible = 0;
		int NrSelected = 0;

		XtVaGetValues(wptr->child,
			      XmNitemCount, &NrItems,
			      XmNvisibleItemCount, &NrVisible,
			      XmNselectedItemCount, &NrSelected,
        		      XmNitems, &Items,
			      NULL);
		NrInfo = NrVisible;
		if (NrInfo == 0)
		    NrInfo = 1;
		Info = (char **)calloc(NrInfo, sizeof(char *));
		i = -1;
		while (++i < NrInfo)
		    XmStringGetLtoR(Items[i], XmSTRING_DEFAULT_CHARSET,
				    Info + i);
		style = FIXED_FONT;
	    }
	    Hbar = Vbar = 1;
	    break;
	case W_OPTIONMENU:
	    {
	    	XmString Label = NULL;
            	Widget Child;

	    	Child = XmOptionButtonGadget(wptr->w);
            	XtVaGetValues(Child,
			      XmNlabelString, &Label,
			      NULL);
		Info = (char **)calloc(1, sizeof(char *));
	    	XmStringGetLtoR(Label, XmSTRING_DEFAULT_CHARSET, Info);
		XmStringFree(Label);
	    	NrInfo = 1;
	    	Center = 1;
		style = ITALIC_FONT;
	    }
	    break;
	default:
	   break;
    }

    /* If has space before it, move to the right */
    if (wptr->extra_before)
	PSprintf("%d 0 R\n", wptr->extra_before);

    PSmove_offset(0);
    PSprintf("gsave 1 setlinewidth gsave currentpoint newpath M\n");
    if (Type != W_RADIOBOX) {
        PSprintf("%.2f 0 RL 0 %.2f neg RL %.2f neg 0 RL 0 %.2f RL closepath\n",
	         Width, Height, Width, Height);
    } else {
	float Hw =  Width / 2.0;
	float Hh =  Height / 2.0;

	PSprintf("%.2f 0 R %.2f %.2f neg RL %.2f neg %.2f neg RL\n",
		 Hw, Hw, Hh, Hw, Hh, Hw, Hh);
        PSprintf("%.2f neg %.2f RL %.2f %.2f RL closepath\n",
	         Hw, Hh, Hw, Hh);
    }
    PSprintf("gsave 0.95 setgray fill grestore stroke grestore\n");

    if (Type == W_OPTIONMENU) {
	Xpos = Width - 10.0;
	Ypos = (Height / 2.0) - 2.0;
	PSprintf("gsave currentpoint newpath M %.2f %.2f neg R\n", Xpos, Ypos);
    	PSprintf("6 0 RL 0 -3 RL -6 0 RL 0 3 RL closepath stroke grestore\n");
    }

    if (Hbar) {
	Xpos = 3.0;
	Ypos = Height - 10.0;
	Leng = Width - 30.0;
	if (Leng < 0)
	    Leng = 0.0;
	PSprintf("gsave currentpoint newpath M %.2f %.2f neg R\n", Xpos, Ypos);
	PSprintf("%.2f HBAR grestore\n", Leng);
    }
    if (Vbar) {
	Xpos = Width - 10.0;
	Ypos = 3.0;
	Leng = Height - 30.0;
	if (Leng < 0)
	    Leng = 0.0;
	PSprintf("gsave currentpoint newpath M %.2f %.2f neg R\n", Xpos, Ypos);
	PSprintf("%.2f VBAR grestore\n", Leng);
    }
    if (NrInfo) {
	Ascent = eptr->font->ascent;
	Width -= 4;
	PSfont(SET, HELVETICA, style, 2);
	i = -1;

	while (++i < NrInfo) {
	    Txt = Info[i];
	    if (NrInfo == 1 && Center) {
	  	Ypos = (Height + Ascent - Extra) / 2.0;
		Xpos = 2.0 + Extra;
	    } else {
		Ypos += Ascent;
		Xpos = 0;
	    }
	    PSprintf("gsave currentpoint M %.2f %.2f neg R\n", Xpos, Ypos);
    	    PSprintf("save (%s) dup stringwidth pop /myW exch def\n", Txt);
    	    PSprintf("%.2f myW lt {%.2f myW div 1 scale} if S restore\n",
    		     Width, Width);
	    Ypos += eptr->font->descent;
	    PSprintf("grestore\n");
	}
	PSfont(RESET, HELVETICA, style, 2);
    }
    if (Info) {
	if (Type == W_LIST || Type == W_OPTIONMENU || Type == W_TEXTFIELD) {
	    i = -1;
	    while (++i < NrInfo)
		XtFree(Info[i]);
	} else if (Type == W_TEXTAREA) {
	    i = -1;
	    while (++i < NrInfo)
		free(Info[i]);
	}
	free(Info);
    }

    w = wptr->width;
    /* Add a little space after in most cases */
    if ((wptr->type != W_CHECKBOX) && (wptr->type != W_RADIOBOX))
	w += IMAGE_DEFAULT_BORDER;

    /*  Move currentpoint to right of widget */
    PSprintf("grestore %d 0 R\n", w); 
}


/*
 * PSrle_encode - perform run length encoding
 *
 * Does the run-length encoding.  This is done to reduce the file size and
 * therefore the time to send the file to the printer.  You get longer
 * processing time instead.
 * 
 * rle is encoded as such:
 *  <count> <value>			# 'run' of count+1 equal pixels
 *  <count | 0x80> <count+1 data bytes>	# count+1 non-equal pixels
 * count can range between 0 and 127
 *
 * Returns length of the rleline vector
 *
*/ 
static int PSrle_encode(unsigned char *scanline, unsigned char *rleline,
			int wide) 
{
    int i, j;
    int blocklen = 0;
    int isrun = 0;
    int rlen = 0;
    unsigned char block[256];
    unsigned char pix;

    for (i = 0; i < wide; i++) {
	/* There are 5 possible states:
	 *   0: block empty.
	 *   1: block is a run, current pix == previous pix
	 *   2: block is a run, current pix != previous pix
	 *   3: block not a run, current pix == previous pix
	 *   4: block not a run, current pix != previous pix
	 */
	pix = scanline[i];

	if (!blocklen) {
	    /* case 0:  empty */
	    block[blocklen++] = pix;
	    isrun = 1;
	} else if (isrun) {
	    if (pix == block[blocklen - 1]) { 
		/*  case 1:  isrun, prev == cur */
		block[blocklen++] = pix;
	    } else {
		/*  case 2:  isrun, prev != cur */
		if (blocklen > 1) {
		    /* We have a run block to flush */
		    rleline[rlen++] = blocklen - 1;
		    rleline[rlen++] = block[0];
		    /* Start new run block with pix */
		    block[0] = pix;
		    blocklen = 1;
		} else {
		    /* blocklen <= 1, turn into non-run */
		    isrun = 0;
		    block[blocklen++] = pix;
		}
	    }
	} else { 
	    /* Not a run */
	    if (pix == block[blocklen - 1]) {
		/* case 3: non-run, prev == cur */
		if (blocklen > 1) {
		    /* Have a non-run block to flush */
		    rleline[rlen++] = (blocklen - 1) | 0x80;
		    for (j = 0; j < blocklen; j++)
			rleline[rlen++] = block[j];
		    /* Start new run block with pix */
		    block[0] = pix;
		    blocklen = isrun = 1;
		} else {
		    /* blocklen <= 1 turn into a run */
		    isrun = 1;
		    block[blocklen++] = pix;
		}
	    } else {
		/* case 4:  non-run, prev != cur */
		block[blocklen++] = pix;
	    }
	}
	
	/* Max block length.  Flush */
	if (blocklen == 128) {
	    if (isrun) {
		rleline[rlen++] = blocklen - 1;
		rleline[rlen++] = block[0];
	    } else {
		rleline[rlen++] = (blocklen - 1) | 0x80;
		for (j = 0; j < blocklen; j++)
		    rleline[rlen++] = block[j];
	    }
	    blocklen = 0;
	}
    }

    /* Flush last block */
    if (blocklen) {
	if (isrun) {
	    rleline[rlen++] = blocklen - 1;
	    rleline[rlen++] = block[0];
	} else {
	    rleline[rlen++] = (blocklen - 1) | 0x80;
	    for (j = 0; j < blocklen; j++)
		rleline[rlen++] = block[j];
	}
    }
    return rlen;
}


/*
 * PScolor_image - created postscript colorimage operator 
 *
 * Spits out code that checks if the PostScript device in question
 * knows about the 'colorimage' operator.  If it doesn't, it defines
 * 'colorimage' in terms of image (i.e., generates a greyscale image
 * from RGB data)
 *
 */
static void PScolor_image(void) 
{
    static char *txt[] = {
	"% define 'colorimage' if it isn't defined",
	"%   ('colortogray' and 'mergeprocs' come from xwd2ps",
	"%	 via xgrab)",
	"/colorimage where   % do we know about 'colorimage'?",
	"  { pop }		   % yes: pop off the 'dict' returned",
	"  {				 % no:  define one",
	"	/colortogray {  % define an RGB->I function",
	"	  /rgbdata exch store	% call input 'rgbdata'",
	"	  rgbdata length 3 idiv",
	"	  /npixls exch store",
	"	  /rgbindx 0 store",
	"	  /grays npixls string store  % str to hold the result",
	"	  0 1 npixls 1 sub {",
	"		grays exch",
	"		rgbdata rgbindx	   get 20 mul	% Red",
	"		rgbdata rgbindx 1 add get 32 mul	% Green",
	"		rgbdata rgbindx 2 add get 12 mul	% Blue",
	"		add add 64 idiv	  % I = .5G + .31R + .18B",
	"		put",
	"		/rgbindx rgbindx 3 add store",
	"	  } for",
	"	  grays",
	"	} bind def\n",
	/* Utility procedure for colorimage operator.
	 * This procedure takes two procedures off the
	 * stack and merges them into a single procedure
	 */
	"	/mergeprocs { % def",
	"	  dup length",
	"	  3 -1 roll dup length dup 5 1 roll",
	"	  3 -1 roll add array cvx dup",
	"	  3 -1 roll 0 exch putinterval",
	"	  dup 4 2 roll putinterval",
	"	} bind def\n",
	"	/colorimage { % def",
	/* Remove 'false 3' operands */
	"	  pop pop",
	"	  {colortogray} mergeprocs",
	"	  image",
	"	} bind def",
	/* End of 'false' case */
	"  } ifelse"
    };

    if (!colorimage_defined) {
	PSconst_out(txt);
	colorimage_defined = 1;
    }
}
 

/*
 * PScolormap - write colormap
 *
 * Spits out code for the colormap of the following image.
 * If !color, it spits out a mono-ized graymap.
 * 
 */
static void PScolormap(int color, int nc, XColor *cmap)
{
    int i;

    /* Define the colormap */
    PSprintf("/cmap %d string def\n\n\n", nc * (color ? 3 : 1));

    /* Load up the colormap */
    PSprintf("currentfile cmap readhexstring\n");

    for (i = 0; i < nc; i++) {
	if (color) {
	    PSprintf("%02x%02x%02x ", cmap[i].red >> 8,
		     cmap[i].green >> 8, cmap[i].blue >> 8);
	} else {
	    PSprintf("%02x ", MONO(cmap[i].red, cmap[i].green, cmap[i].blue));
	}
	if ((i % 10) == 9)
	    PSprintf("\n");
    }
    PSprintf("\npop pop\n");     /* Lose return values from readhexstring */
}


/*
 * PSrle_cmapimage - define rlecmapimage operator
 * 
 */
static void PSrle_cmapimage(int color) 
{
    static char *txt[] = {
	/* rlecmapimage expects to have 'w h bits matrix' on stack */
	"/rlecmapimage {",
	"  /buffer 1 string def",
	"  /rgbval 3 string def",
	"  /block  384 string def",
	"  { currentfile buffer readhexstring pop",
	"	/bcount exch 0 get store",
	"	bcount 128 ge",
	"	{ ",
	"	  0 1 bcount 128 sub",
	"	{ currentfile buffer readhexstring pop pop"
    };
    static char *txt_color[] = {
	"		/rgbval cmap buffer 0 get 3 mul 3 getinterval store",
	"		block exch 3 mul rgbval putinterval",
	"	  } for",
	"	  block  0  bcount 127 sub 3 mul  getinterval",
	"	}",
	"	{ ",
	"	  currentfile buffer readhexstring pop pop",
	"	  /rgbval cmap buffer 0 get 3 mul 3 getinterval store",
	"	  0 1 bcount { block exch 3 mul rgbval putinterval } for",
	"	  block 0 bcount 1 add 3 mul getinterval",
	"	} ifelse",
	"  }",
	"  false 3 colorimage",
	"} bind def"
    };
    static char *txt_gray[] = {
	"		/rgbval cmap buffer 0 get 1 getinterval store",
	"		block exch rgbval putinterval",
	"	  } for",
	"	  block  0  bcount 127 sub  getinterval",
	"	}",
	"	{ ",
	"	  currentfile buffer readhexstring pop pop",
	"	  /rgbval cmap buffer 0 get 1 getinterval store",
	"	  0 1 bcount { block exch rgbval putinterval } for",
	"	  block 0 bcount 1 add getinterval",
	"	} ifelse",
	"  }",
	"  image",
	"} bind def"
    };

    PSconst_out(txt);
    if (color) { 
	PSconst_out(txt_color);
    } else {
	PSconst_out(txt_gray);
    }
}


/*
 * PSwrite_bw - write B&W image
 *
 * Write the given image array 'pic' (B/W stippled, 1 byte per pixel,
 * 0=blk, 1=wht) out as hexadecimal, max of 72 hex chars per line.  If
 * 'flipbw', then 0=white, 1=black.  Returns '0' if everythings fine,
 * 'EOF' if writing failed.
 *
*/
static int PSwrite_bw(unsigned char *pic, int w, int h, int flipbw)
{
    int	i, j;
    int	err = 0;
    unsigned char outbyte = 0;
    unsigned char bitnum = 0;
    unsigned char bit;
    
    for (i = 0; (i < h) && (err != EOF); i++) {
	for (j = 0; (j < w) && (err != EOF); j++) {
	    bit = *pic++;
	    outbyte = (outbyte << 1) | ((bit)&0x01);
	    if (++bitnum == 8) {
		if (flipbw)
		    outbyte = ~outbyte & 0xff;
		err = PShex(outbyte, False);
		outbyte = bitnum = 0;
	    }
	}
	if (bitnum) {	/* Few bits left over in this row */
	    outbyte <<= 8-bitnum;
	    if (flipbw)
		outbyte = ~outbyte & 0xff;
	    err = PShex(outbyte, False);
	    outbyte = bitnum = 0;
	}
    }
    err = PShex('\0', True);	/*  Flush the hex buffer if needed */
    
    return err;
}


/* Isgray returns true if the nth color index is a gray value */
#define Isgray(colr, n) \
   (colr[n].red == colr[n].green && colr[n].red == colr[n].blue)

/* Is_bg returns true if the nth color index is the screen background */
#define Is_bg(colr, n)  (colr[n].red == bg_color.red &&	\
   colr[n].green == bg_color.green && colr[n].blue == bg_color.blue)

/* Is_fg returns true if the nth color index is the screen foreground */
#define Is_fg(colr, n)  (colr[n].red == fg_color.red &&	\
   colr[n].green == fg_color.green && colr[n].blue == fg_color.blue)

/*
 * PSimage - generate image Postscript code
 *
 * Draw the image, unless there was no image, in which case an empty grey
 * rectangle is shown.
 * If anchor is set, a black border is shown around the image.
 * Positioning is not exactly that of Mosaic's screen, but close enough.
 * 
*/
static void PSimage(struct ele_rec *eptr)
{
    ImageInfo *img = eptr->pic_data;
    unsigned char *imgp = img->image_data;
    XColor *colrs;
    int anchor = 0;
    int ncolors;
    int w = img->width;
    int h = img->height;
    int err = 0;
    int extra = 0;
    int i, j, slen, colorps, colortype, bits;

    PSmove_offset(0);

    if ((eptr->anchor_tag_ptr->anc_href || img->has_border) &&
	eptr->bwidth && (!img->internal || (img->internal == 2))) {
	anchor = 1;
	/*
	 *  Draw an outline by drawing a slightly larger black square
	 *  below the actual image
	 */
	PSprintf("gsave currentpoint %d sub translate ", h);
	PSprintf("0 -2 translate %d %d scale\n", w + 4, h + 4);
	PSprintf("SQ fill\ngrestore\n");
	extra = 4;
    }
	
    if (!imgp) {
	/*  Image was not available... do something instead
	 *  Draw an empty square for example
	 */
	PSprintf("gsave currentpoint %d sub translate", h);
	if (anchor) {
	    PSprintf(" 2 0 translate");
	} else {
	    PSprintf(" 0 2 translate");
	}
	PSprintf(" %d %d scale\n", w, h);
	PSprintf("0.9 setgray SQ fill\ngrestore\n");
	/*  Move currentpoint just right of image */
	PSprintf("%d 0 R\n", w + extra);
	return;
    }
    if (img->ori_colrs) {
	colrs = img->ori_colrs;
	ncolors = img->ori_num_colors;
    } else {
	colrs = img->colrs;
	ncolors = img->num_colors;
    }

    /*  This is a hack to see if the image is Black & White, 
     *  Greyscale or 8 bit color.
     *  Assume it's bw if it has only the foreground and background colors.
     *  Assume it's greyscale if all the colors are grey.
     */
    colorps = 0;
    if (img->internal ||
	((ncolors == 2) &&
	 ((Is_bg(colrs, 0) && Is_fg(colrs, 1)) ||
	  (Is_fg(colrs, 0) && Is_bg(colrs, 1)))) ||
	((ncolors == 1) && (Is_bg(colrs, 0) || Is_fg(colrs, 0)))) {
	colortype = F_BWDITHER;
	slen = (w + 7) / 8;
	bits = 1;
    } else {
	colortype = F_GREYSCALE;
	slen = w;
	bits = 8;
	for (i = 0; i < ncolors; i++) {
	    if (!Isgray(colrs, i)) {
		colortype = F_COLOR;
		slen = w * 3;
		colorps = 1;
		break;
	    }
	}
    }
	
    /*  If we're using color, make sure 'colorimage' is defined.
     *  'colorimage' is a Postscript LanguageLevel 2 operator. */
    if (colorps)
	PScolor_image();
	
    /*  Build a temporary dictionary */
    PSprintf("20 dict begin\n\n");

    /*  Define string to hold a scanline's worth of data */
    PSprintf("/pix %d string def\n\n", slen);
    
    /*  Position and scaling */
    PSprintf("gsave currentpoint %d sub translate", h);
    if (anchor || eptr->pic_data->has_border) {
	PSprintf(" 2 0 translate");
    } else {
	PSprintf(" 0 2 translate");
    }
    PSprintf(" %d %d scale\n", w, h);
    
    if (colortype == F_BWDITHER) {
	/*  1-bit dither code uses 'image' */
	int flipbw = 0;
	
	/*  Set if color #0 is 'white' */
	if (!img->internal && ((ncolors == 2 &&
	     MONO(colrs[0].red, colrs[0].green, colrs[0].blue) >
	     MONO(colrs[1].red, colrs[1].green, colrs[1].blue)) ||
	    (ncolors == 1 && 
	     MONO(colrs[0].red, colrs[0].green, colrs[0].blue) >
	     MONO(127, 127, 127))))
	    flipbw = 1; 
	
	/*  Dimensions of data */
	PSprintf("%d %d %d\n", w, h, bits);
	
	/*  Mapping matrix */
	PSprintf("[%d 0 0 %d 0 %d]\n\n", w, -h, h);

	PSprintf("{currentfile pix readhexstring pop}\nimage\n");

	/*  Write the actual image data */
	if (img->internal) {
	    /* Must first convert the image data if it is an internal image */
	    /* Code adapted from XPM stuff in PICREAD.C */
	    int value, i, j;
	    int size = w * h;
	    int limit = ((w + 7) / 8) * 8;	/* # of bits per line */
	    int cnt = 0;
	    unsigned char *dataP = malloc(size * sizeof(char));
	    unsigned char *ptr = dataP;
	    static char Mask[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

	    if (!dataP)
		return;
	    for (i = 0; ptr < (dataP + size); i++) {
		value = (int)img->image_data[i];
		for (j = 0; j < 8; j++) {
		    if (ptr == (dataP + size))
			break;
		    if (cnt < w) {
			if (value & Mask[j]) {
			    *ptr++ = fg_color.pixel;
			} else {
			    *ptr++ = bg_color.pixel;
			}
		    }
		    if (++cnt >= limit)
			cnt = 0;
		}
	    }
	    err = PSwrite_bw(dataP, w, h, flipbw);
	    free(dataP);
	} else {
	    err = PSwrite_bw(imgp, w, h, flipbw);
	}
    } else {
	/*  All other formats */
	unsigned char *rleline = (unsigned char *) NULL;
	int rlen;
	
	PScolormap(colorps, ncolors, colrs);
	PSrle_cmapimage(colorps);

	/*  Dimensions of data */
	PSprintf("%d %d %d\n", w, h, bits);
	/*  Mapping matrix */
	PSprintf("[%d 0 0 %d 0 %d]\nrlecmapimage\n", w, -h, h);
	
	if (!(rleline = (unsigned char *) malloc(w * 2))) {
#ifndef DISABLE_TRACE
	    if (htmlwTrace)
		fprintf(stderr, "failed to malloc space for rleline\n");
#endif
	    return;
	}

	for (i = 0; (i < h) && (err != EOF); i++) {
	    rlen = PSrle_encode(imgp, rleline, w);
	    imgp += w;
	    for (j = 0; (j < rlen) && (err != EOF); j++)
		err = PShex(rleline[j], False);
	    err = PShex('\0', True);	/*  Flush the hex buffer */
	}
	free(rleline);
    }
	
    /*  Stop using temporary dictionary */
    PSprintf("end\ngrestore\n");
	
    /*  Move currentpoint just right of image */
    PSprintf("%d 0 R\n", w + extra); 
    if (HTML_Print_Footers && has_footnote(eptr)) {
	PSmove_offset(0);
	PSfootnote(eptr->anchor_tag_ptr->anc_href, 2.0);
    }

    /* Forget about the macro's */
#undef Isgray
#undef Is_fg
#undef Is_bg
}


/*
 * ParseTextToPSString - entry point for postscript output
 *
 * Parse all the formatted text elements from start to end
 * into an ascii text string, and return it.
 * Very like ParseTextToString() except the text is prettied up
 * into Postscript to show headers and the like.
 * space_width and lmargin tell us how many spaces
 * to indent lines.
 * Because this routine is only used to print whole documents,
 * some parameters are not needed at all!
 * Also it assumes that you are indeed printing the whole document, and
 * not just a selected portion of it.  It therefore can assume that
 * only for the first page the initialization is needed, and only
 * the last page has the trailers.  You cannot use ParseTextToPSString()
 * as you can use ParseTextToString() because of this initialization code.
 *
 * The fontfamily parameter is new
 * The font is encoded as:
 *	0: times
 *	1: helvetica
 *	2: new century schoolbook
 *	3: lucida
 */
String ParseTextToPSString(HTMLWidget	  hw, 
			   struct ele_rec *elist,
			   struct ele_rec *startp,
			   struct ele_rec *endp,
			   int		  start_pos, 
			   int		  end_pos,
			   int		  space_width,
			   int		  lmargin,
			   int		  fontfamily,
			   char   	  *url,
			   char	   	  *time_str)
{
    double pagewidth;
    struct ele_rec *eptr, *ptr, *tmpptr;
    struct ele_rec *prev, *start, *end;
    struct ele_rec *next_line;
    unsigned long fg_pixel, bg_pixel;    
    int	xpos, ypos, epos, height;
    int	prev_y, skipped_cr;
    int	footnotes_this_line, reserved_space;
    int	footnotes_this_page = 0;
    int	newline = 1;
    int	newpage = 1;
    CurFontType	pftype;
    Display *dsp = hw->html.dsp;
 	
    if (!startp || !hw->html.first_formatted_line)
	return(NULL);
    /*
     * Get the foreground and background colors so we can check later
     * for black&white documents
     */
    XtVaGetValues(hw->html.view, 
#ifdef MOTIF
		  XtNforeground, &fg_pixel,
#endif
		  XtNbackground, &bg_pixel, 
		  NULL);
#ifndef MOTIF
    XtVaGetValues((Widget)hw, 
		  XtNforeground, &fg_pixel,
		  NULL);
#endif	
    fg_color.pixel = fg_pixel;
    bg_color.pixel = bg_pixel;
    XQueryColor(dsp, installed_colormap ? installed_cmap :
		     DefaultColormap(dsp, DefaultScreen(dsp)),
		&fg_color);
    XQueryColor(dsp, installed_colormap ? installed_cmap :
		     DefaultColormap(dsp, DefaultScreen(dsp)),
		&bg_color);
#if 0
    /*  This piece of code is needed if the user selects a portion
     *  of the document with the mouse.
     *  I think it will never be used, but I left it in anyway. F.
     */
    if (SwapElements(startp, endp, start_pos, end_pos)) {
	start = endp;
	end = startp;
	epos = start_pos;
	start_pos = end_pos;
	end_pos = epos;
    } else {
	start = startp;
	end = endp;
    }
#endif
	
    /* Setup page size according to user preference. */

    if (HTML_Print_Paper_Size_A4) {
	page_dimens = a4_page_dimens;
    } else {
	page_dimens = us_letter_page_dimens;
    }
    page_dimens.text_height = page_dimens.page_height -
			      page_dimens.top_margin - page_dimens.bot_margin;
    page_dimens.text_width = page_dimens.page_width - page_dimens.left_margin -
			     page_dimens.right_margin;

    /* Calculate the number of Postscript points per pixel of current
     * screen, and the height of the page in pixels (used in figuring
     * when we've hit the bottom of the page).
     */
    Points_Pixel = 72.0 / GetDpi(hw);
    /* gustaf fix */
    pagewidth = hw->html.view_width;

    /* Reduce the scaling if the width used for formatting is greater
     * than 8 * 72 pixels (8 inch).
     * In theory, this is not what you want for A4 paper (only 8.27 inch
     * wide), but I guess that the hw->html.doc_width includes some
     * left and right margins, so it seems to work in practice.
     */
    if (pagewidth * Points_Pixel > page_dimens.text_width) 
	Points_Pixel = page_dimens.text_width / pagewidth;
    Pixels_Page = (int) (page_dimens.text_height / Points_Pixel);		
	
    PSinit();
    PSheader(hw->html.title, fontfamily, url, time_str);
    /* Must set it initially so resets will work */
    PSfont(SET_DEFAULT, fontfamily, FONT, 3);
    PSfont(DEFAULT, fontfamily, FONT, 3);
    PSnewpage();

    /* Sort the elements by (y + height) */
    eptr = hw->html.first_formatted_line;
    next_line = eptr->line_next;
    if (next_line && (next_line->y < eptr->y)) {
	hw->html.first_formatted_line = next_line;
	eptr->line_next = next_line->line_next;
	next_line->line_next = eptr;
    }
    eptr = next_line;
    if (eptr) {
	next_line = eptr->line_next;
    } else {
	next_line = NULL;
    }
    while (next_line) {
	if (next_line->y < eptr->y) {
	    /* Remove it from current place and put where belongs */
	    eptr->line_next = next_line->line_next;
	    ptr = hw->html.first_formatted_line;
	    if (next_line->y < ptr->y) {
		next_line->line_next = ptr;
		hw->html.first_formatted_line = next_line;
	    } else {
		prev = ptr;
		ptr = ptr->line_next;
		while (ptr) {
		    if (next_line->y < ptr->y) {
			next_line->line_next = ptr;
			prev->line_next = next_line;
			break;
		    }
		    prev = ptr;
		    ptr = ptr->line_next;
		}
	    }
	} else {
	    eptr = next_line;
	}
        next_line = eptr->line_next;
    }

    next_line = hw->html.first_formatted_line;
    if (!next_line->prev)
	next_line = next_line->line_next;

    if (next_line) {
        eptr = next_line->prev;
    } else {
	eptr = NULL;
    }
    while (eptr) {
	if (newline) {
	    height = next_line->height;
	    ypos = next_line->y;
	    pftype = FONT;
	    footnotes_this_line = 0;
	    tmpptr = eptr;
	    while (tmpptr->type != E_LINEFEED) {
		if (HTML_Print_Footers && has_footnote(tmpptr))
		    footnotes_this_line++;
		/* Check for left aligned image */
		if ((tmpptr->type == E_IMAGE) &&
		    (tmpptr->pic_data->aligned == 2) &&
		    (tmpptr->height > height))
		    height = tmpptr->height;
		if (tmpptr->prev) {
		    tmpptr = tmpptr->prev;
		} else {
		    break;
		}
	    }
	    /* Start at beginning of line */
	    if (tmpptr->type != E_LINEFEED) {
		eptr = tmpptr;
	    } else {
		eptr = tmpptr->next;
	    }
	    if (!eptr)
		break;

	    /* Keep so we know when it changes */
	    prev_y = eptr->y;

	    /* Check if line fits completely on page */
	    reserved_space = 0;
	    if (footnotes_this_page || footnotes_this_line) {
		reserved_space = (footnote_space +
				  (footnote_ptsize *
				   (footnotes_this_page + footnotes_this_line)))
				 / Points_Pixel;
	    }
	    /* Don't do if first line on page */
	    if (!newpage && ((ypos + height + reserved_space) >
			     (PS_start_y + Pixels_Page))) {
		PS_start_y = ypos;
		PSshowpage();
		PSnewpage();
		footnotes_this_page = 0;
		newpage = 1;
	    } else {
		newpage = 0;
	    }
	    footnotes_this_page += footnotes_this_line;
	}

	skipped_cr = 0;
	/* Skip CRs */
	/* Skip E_TABLEs to prevent premature new page */
	while (eptr && ((eptr->type == E_CR) ||
	       (eptr->type == E_TABLE) || (eptr->type == E_CELL_TABLE))) {
	    if (eptr->type == E_CR)
		skipped_cr = 1;
	    eptr = eptr->next;
	}
	if (!eptr)
	    break;

	xpos = eptr->x - lmargin;
	if (xpos < 0)
	    xpos = 0;

	/* A hack to correct spacing */
	switch (eptr->font_size) {
	    case 1:
		if (eptr->font_family == TIMES) {
			if (pftype == BOLD_FONT) {
				xpos += (.01 * xpos);
			} else {
				xpos -= (.02 * xpos);
			}
		} else if (eptr->font_family == HELVETICA) {
			if (pftype == BOLD_FONT) {
				xpos -= (.05 * xpos);
			} else {
				xpos -= (.03 * xpos);
			}
		} else if ((pftype != ITALIC_FONT) &&
			   (pftype != ADDRESS_FONT) &&
			   (pftype != BOLD_FONT)) { 
			xpos -= (.05 * xpos);
		}
		break;
	    case 2:
		if (eptr->font_family == TIMES) {
			if ((pftype == ITALIC_FONT) ||
			    (pftype == ADDRESS_FONT)) {
				xpos -= (.03 * xpos);
			} else if (pftype != BOLD_FONT) {
				xpos -= (.01 * xpos);
			}
		} else if (eptr->font_family == HELVETICA) {
			if ((pftype == ITALIC_FONT) ||
			    (pftype == ADDRESS_FONT)) {
				xpos -= (.04 * xpos);
			} else if (pftype == BOLD_FONT) {
				xpos -= (.02 * xpos);
			} else {
				xpos -= (.01 * xpos);
			}
		} else if ((pftype == ITALIC_FONT) ||
			   (pftype == ADDRESS_FONT)) {
				xpos -= (.02 * xpos);
		} else {
				xpos -= (.01 * xpos);
		}
		break;
	    case 3:
		if (eptr->font_family == TIMES) {
			if (pftype != BOLD_FONT) {
				xpos += (.04 * xpos);
			} else {
				xpos += (.01 * xpos);
			}
		} else if (eptr->font_family == HELVETICA) {
			if ((pftype == ADDRESS_FONT) ||
			    (pftype == ITALIC_FONT)) {
				xpos += (.02 * xpos);
			} else if (pftype == BOLD_FONT) {
				xpos += (.03 * xpos);
			} else {
				xpos += (.04 * xpos);
			}
		} else if ((pftype != ITALIC_FONT) && (pftype != ADDRESS_FONT)){
			xpos += (.05 * xpos);
		} else {
			xpos += (.04 * xpos);
		}
		break;
	    case 4:
		if (eptr->font_family == TIMES) {
			if ((pftype != ITALIC_FONT) &&
			    (pftype != ADDRESS_FONT)) {
				xpos -= (.01 * xpos);
			} else {
				xpos -= (.03 * xpos);
			}
		} else if (eptr->font_family == HELVETICA) {
			if (pftype != BOLD_FONT)
				xpos -= (.01 * xpos);
		} else if ((pftype == ITALIC_FONT) ||
			   (pftype == ADDRESS_FONT)) {
			xpos -= (.02 * xpos);
		} else if (pftype == BOLD_FONT) {
			xpos -= (.01 * xpos);
		} else {
			xpos += (.01 * xpos);
		}
		break;
	    case 5:
		if (eptr->font_family == TIMES) {
			if ((pftype != ITALIC_FONT) &&
			    (pftype != ADDRESS_FONT)) {
				if (pftype == BOLD_FONT) {
					xpos += (.07 * xpos);
				} else {
					xpos += (.04 * xpos);
				}
			} else {
				xpos -= (.03 * xpos);
			}
		} else if (eptr->font_family == HELVETICA) {
			if ((pftype == ITALIC_FONT) ||
			    (pftype == ADDRESS_FONT)) {
				xpos -= (.01 * xpos);
			} else if (pftype == BOLD_FONT) {
				xpos += (.06 * xpos);
			} else {
				xpos += (.05 * xpos);
			}
		} else if ((pftype == ITALIC_FONT) || (pftype == ADDRESS_FONT)){
			xpos -= (.02 * xpos);
		} else if (pftype == BOLD_FONT) {
			xpos -= (.01 * xpos);
		} else {
			xpos += (.01 * xpos);
		}
		break;
	    case 6:
		if (eptr->font_family == TIMES) {
			if ((pftype != ITALIC_FONT) &&
			    (pftype != ADDRESS_FONT) && (pftype != BOLD_FONT)) {
				xpos -= (.01 * xpos);
			} else {
				xpos -= (.03 * xpos);
			}
		} else if (eptr->font_family == HELVETICA) {
			if ((pftype == ITALIC_FONT) ||
			    (pftype == ADDRESS_FONT)) {
				xpos -= (.01 * xpos);
			} else if (pftype == BOLD_FONT) {
				xpos -= (.02 * xpos);
			}
		} else if (eptr->font_family == CENTURY) {
			xpos -= (.02 * xpos);
		}
		break;
	    case 7:
		if (eptr->font_family == TIMES) {
			xpos += (.04 * xpos);
		} else if (eptr->font_family == HELVETICA) {
			if ((pftype != ITALIC_FONT) &&
			    (pftype != ADDRESS_FONT)) {
				if (pftype == BOLD_FONT) {
					xpos += (.04 * xpos);
				} else {
					xpos += (.06 * xpos);
				}
			} else {
				xpos += (.03 * xpos);
			}
		} else if ((pftype != ITALIC_FONT) &&
			   (pftype != ADDRESS_FONT)) {
			xpos += (.06 * xpos);
		} else {
			xpos += (.08 * xpos);
		}
	}
	/* Keep the font type for use by above hack */
	pftype = eptr->font_type;

	/* Only PSmoveto when newline, y changes or skipped a CR */
	if (newline || (prev_y != eptr->y) || skipped_cr ||
	    ((eptr->type == E_IMAGE) && (eptr->pic_data->aligned == 1))) {
	    PSmoveto(xpos, eptr->y);
	    prev_y = eptr->y;
	}
	newline = 0;

	switch (eptr->type) {
	case E_TEXT:
/*	    PStext(hw, eptr, (String)((eptr == start) ?
		   (eptr->edata + start_pos) : eptr->edata));
*/
	    PStext(hw, eptr, (String)eptr->edata);
	    break;

	case E_BULLET: 
	    PSbullet(hw, eptr);
	    break;

	case E_IMAGE:
	    /* Ignore background image */
	    if (eptr->pic_data->is_bg_image)
		break;

	    /* Handle borders */
	    if (eptr->anchor_tag_ptr->anc_href || eptr->pic_data->has_border)
		PSmoveto(xpos + 2, eptr->y + 2);

	    PSimage(eptr);
	    break;

	case E_LINEFEED: 
	    if (next_line->line_next) {
		newline = 1;
		next_line = next_line->line_next;
		eptr = next_line->prev;
	    } else {
		eptr = NULL;
	    }
	    break;

	case E_HRULE:
	    PShrule(eptr);
	    break;

	case E_TABLE:
	    PStable(eptr);
	    break;

	case E_WIDGET:
	    PSwidget(eptr);
	    break;
	}

	if (eptr && !newline)
	    eptr = eptr->next;
    }
    PSshowpage();
    PStrailer();

    return(PS_string);
}
