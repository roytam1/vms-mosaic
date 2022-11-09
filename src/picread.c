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

/* Copyright (C) 1998, 1999, 2000, 2003, 2004, 2005, 2006
 * The VMS Mosaic Project
 */

#include "../config.h"
#include "mosaic.h"
#include "picread.h"
#include "gifread.h"
#include "xpmread.h"
#include "readBMP.h"

#ifdef HAVE_JPEG
#include "readJPEG.h"
#endif

#ifdef HAVE_PNG
#include "readPNG.h"
#endif

#include <X11/Xos.h>

#define	MAX_LINE	81

extern int installed_colormap;
extern Colormap installed_cmap;

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int reportBugs;
#endif

static char nibMask[8] = {
	1, 2, 4, 8, 16, 32, 64, 128
};


static unsigned char *ReadXbmBitmap(FILE *fp, int *w, int *h, XColor *colrs)
{
	char line[MAX_LINE], name_and_type[MAX_LINE];
	char *t;
	unsigned char *ptr, *dataP;
	int bytes_per_line, version10p, raster_length, padding;
	int i, bytes, temp, value;
        static unsigned long fg_pixel, bg_pixel;
        static int done_fetch_colors = 0;
	static XColor fg_color, bg_color;
	static int reverse;
        extern Widget view;
        extern int Vclass;
	int blackbit;
	int whitebit;

        if (!done_fetch_colors) {
            /* First, go fetch the pixels. */
            XtVaGetValues(view, XtNforeground, &fg_pixel,
                          XtNbackground, &bg_pixel, NULL);
            
            /* Now, load up fg_color and bg_color. */
            fg_color.pixel = fg_pixel;
            bg_color.pixel = bg_pixel;
            
            /* Now query for the full color info. */
            XQueryColor(XtDisplay(view), 
                        (installed_colormap ?
		         installed_cmap :
		         DefaultColormap(XtDisplay(view),
                          DefaultScreen(XtDisplay(view)))),
                        &fg_color);
            XQueryColor(XtDisplay(view), 
                        (installed_colormap ?
		         installed_cmap :
		         DefaultColormap(XtDisplay(view),
                          DefaultScreen(XtDisplay(view)))),
                        &bg_color);

            done_fetch_colors = 1;

	    /*
	     * For a TrueColor visual, we can't use the pixel value as
	     * the color index because it is > 255.  Arbitrarily assign
	     * 0 to foreground, and 1 to background.
	     */
	    if ((Vclass == TrueColor) || (Vclass == DirectColor)) {
		fg_color.pixel = 0;
		bg_color.pixel = 1;
	    }

	    reverse = get_pref_boolean(eREVERSE_INLINED_BITMAP_COLORS);
        }

        if (reverse) {
            blackbit = bg_color.pixel;
            whitebit = fg_color.pixel;
        } else {
            blackbit = fg_color.pixel;
            whitebit = bg_color.pixel;
        }
	/*
	 * Error out here on visuals we can't handle so we won't core dump
	 * later.
	 */
	if (((blackbit > 255) || (whitebit > 255)) && (Vclass != TrueColor)) {
	    fprintf(stderr,
		    "Error:  cannot handle default colormap that is deeper than 8, and not TrueColor\n");
            fprintf(stderr,
		    "        If you actually have such a system, please notify mosaic@wvnvms.wvnet.edu.\n");
            fprintf(stderr, "        We thank you for your support.\n");
	    exit(1);
	}

        if (reverse) {
            colrs[blackbit].red = bg_color.red;
            colrs[blackbit].green = bg_color.green;
            colrs[blackbit].blue = bg_color.blue;
            colrs[blackbit].pixel = bg_color.pixel;
            colrs[blackbit].flags = DoRed|DoGreen|DoBlue;
            
            colrs[whitebit].red = fg_color.red;
            colrs[whitebit].green = fg_color.green;
            colrs[whitebit].blue = fg_color.blue;
            colrs[whitebit].pixel = fg_color.pixel;
            colrs[whitebit].flags = DoRed|DoGreen|DoBlue;
        } else {
            colrs[blackbit].red = fg_color.red;
            colrs[blackbit].green = fg_color.green;
            colrs[blackbit].blue = fg_color.blue;
            colrs[blackbit].pixel = fg_color.pixel;
            colrs[blackbit].flags = DoRed|DoGreen|DoBlue;
            
            colrs[whitebit].red = bg_color.red;
            colrs[whitebit].green = bg_color.green;
            colrs[whitebit].blue = bg_color.blue;
            colrs[whitebit].pixel = bg_color.pixel;
            colrs[whitebit].flags = DoRed|DoGreen|DoBlue;
        }

	*w = 0;
	*h = 0;
	for ( ; ; ) {
		if (!(fgets(line, MAX_LINE, fp)))
			break;
		if (strlen(line) == (MAX_LINE - 1))
			return((unsigned char *)NULL);

		if (sscanf(line, "#define %s %d", name_and_type, &value) == 2) {
			if (!(t = strrchr(name_and_type, '_'))) {
				t = name_and_type;
			} else {
				t++;
			}
			if (!strcmp("width", t))
				*w = value;
			if (!strcmp("height", t))
				*h = value;
			continue;
		}
		if (sscanf(line, "static short %s = {", name_and_type) == 1) {
			version10p = 1;
			break;
		} else if (sscanf(line,
				  "static char * %s = {", name_and_type) == 1) {
			/* Let LibXpm code handle XPM1 images */
			return((unsigned char *)NULL);
		} else if (sscanf(line,
				  "static char %s = {", name_and_type) == 1) {
			version10p = 0;
			break;
		} else if (sscanf(line, "static unsigned char %s = {",
				  name_and_type) == 1) {
			version10p = 0;
			break;
		} else {
			continue;
		}
	}
	if ((*w == 0) || (*h == 0)) {
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "XBM: Can't read image.\n");
#endif
		return((unsigned char *)NULL);
	}
	padding = 0;
	if (((*w % 16) >= 1) && ((*w % 16) <= 8) && version10p)
		padding = 1;

	bytes_per_line = ((*w + 7) / 8) + padding;
	raster_length =  bytes_per_line * *h;
	dataP = (unsigned char *)malloc((*w) * (*h));
	if (!dataP) {
#ifndef DISABLE_TRACE
		if (srcTrace || reportBugs)
			fprintf(stderr, "Not enough memory.\n");
#endif
		return((unsigned char *)NULL);
	}
	ptr = dataP;
	if (version10p)	{
		int cnt = 0;
		int lim = (bytes_per_line - padding) * 8;

		for (bytes = 0; bytes < raster_length; bytes += 2) {
			if (fscanf(fp, " 0x%x%*[,}]%*[ \r\n]", &value) != 1) {
#ifndef DISABLE_TRACE
				if (srcTrace)
					fprintf(stderr,
						"Error scanning bits item.\n");
#endif
				return((unsigned char *)NULL);
			}
			temp = value;
			value = temp & 0xff;
			for (i = 0; i < 8; i++)	{
				if (cnt < (*w))	{
					if (value & nibMask[i]) {
						*ptr++ = blackbit;
					} else {
						*ptr++ = whitebit;
					}
				}
				if (++cnt >= lim)
					cnt = 0;
			}
			if (!padding || ((bytes + 2) % bytes_per_line)) {
				value = temp >> 8;
				for (i = 0; i < 8; i++)	{
					if (cnt < (*w))	{
						if (value & nibMask[i]) {
							*ptr++ = blackbit;
						} else {
							*ptr++ = whitebit;
						}
					}
					if (++cnt >= lim)
						cnt = 0;
				}
			}
		}
	} else {
		int cnt = 0;
		int lim = bytes_per_line * 8;

		for (bytes = 0; bytes < raster_length; bytes++)	{
			if (fscanf(fp, " 0x%x%*[,}]%*[ \r\n]", &value) != 1) {
#ifndef DISABLE_TRACE
				if (srcTrace)
					fprintf(stderr,
						"Error scanning bits item.\n");
#endif
				return((unsigned char *)NULL);
			}
			for (i = 0; i < 8; i++)	{
				if (cnt < (*w))	{
					if (value & nibMask[i]) {
						*ptr++ = blackbit;
					} else {
						*ptr++ = whitebit;
					}
				}
				if (++cnt >= lim)
					cnt = 0;
			}
		}
	}
	return(dataP);
}


unsigned char *ReadBitmap(Widget wid, char *file, int *w, int *h, int *x,
			  int *y, XColor *colrs, int *bg, int *animated,
			  int *delay, int *disposal, FILE **gif_fp,
			  unsigned char **alpha)
{
    unsigned char *bit_data;
    char rbuf[7];
    FILE *fp;

    *bg = -1;
    *animated = -1;
    *x = 0;
    *y = 0;
    *gif_fp = NULL;
    *alpha = NULL;
    rbuf[6] = '\0';
    
    if (!file || !*file) {
	fp = NULL;
    } else {
#ifndef VMS
	fp = fopen(file, "r");
#else
	fp = fopen(file, "r", "ctx=stm", "rop=RAH");
#endif
    }
    
    if (fp) {
	bit_data = ReadGIF(fp, w, h, colrs, bg, 1, animated, delay, x, y,
			   disposal, rbuf);
	if (bit_data) {
	    *gif_fp = fp;
	    return(bit_data);
	}
	rewind(fp);

	/* The rbuf check is a hack to avoid I/O and routine calls.
	 * Otherwise each routine would be called to check the file.
	 *
	 * Both XBM and XPM1 files can start with '#defin'
	 */
	if (strstr(rbuf, "#defin") || strstr(rbuf, "static")) {
	    bit_data = ReadXbmBitmap(fp, w, h, colrs);
	    if (bit_data) {
		if (fp != stdin)
		    fclose(fp);
		return(bit_data);
 	    }
	    rewind(fp);
        }
	if (strstr(rbuf, "XPM") || strstr(rbuf, "#defin")) {
	    /* XPM 1, 2 or 3 image */
	    bit_data = ReadXpmPixmap(wid, NULL, fp, w, h, colrs, bg);
	    if (bit_data) {
		if (fp != stdin)
		    fclose(fp);
		return(bit_data);
	    }
#ifdef HAVE_PNG
	} else if (strstr(rbuf, "PNG")) {
	    bit_data = ReadPNG(wid, fp, w, h, colrs, bg, alpha);
	    if (bit_data) {
		if (fp != stdin)
		    fclose(fp);
		return(bit_data);
	    }
#endif
	} else if (strstr(rbuf, "BM")) {
	    bit_data = ReadBMP(fp, w, h, colrs, alpha);
	    if (bit_data) {
		if (fp != stdin)
		    fclose(fp);
		return(bit_data);
	    }
#ifdef HAVE_JPEG
	} else if (!strstr(rbuf, "ERROR")) {
	    /* If not checking rbuf, then ReadJPEG must be last because it 
	     * screws up the file pointer by closing it if there is an error.
	     * Check for JPEG by default because don't know how to identify
	     * one in first six bytes.
	     */
	    bit_data = ReadJPEG(fp, w, h, colrs);
	    if (bit_data) {
		if (fp != stdin)
		    fclose(fp);
		return(bit_data);
	    }
#endif
	}
    }

    /* Not a supported image */
    if (fp && (fp != stdin))
	fclose(fp);
    return((unsigned char *)NULL);
}
