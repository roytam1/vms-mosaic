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

/* Copyright (C) 1998, 1999, 2000 - The VMS Mosaic Project */

/* Author: DXP 

 A lot of this is copied from the PNGLIB file example.c

 Modified:

    August   1995 - Glenn Randers-Pehrson <glennrp@arl.mil>
                    Changed dithering to use a 6x6x6 color cube.

    March 21 1996 - DXP
                    Fixed some interlacing problems.
                  
*/

#include "../config.h"
#ifdef HAVE_PNG

#include "../libhtmlw/HTMLp.h"
#include <stdio.h>

#include "png.h"
#include "mosaic.h"
#include "readPNG.h"

#include <setjmp.h>

extern int browserSafeColors;
extern XColor BSColors[256];
extern int BSCnum;

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int reportBugs;
#endif

unsigned char *
ReadPNG(HTMLWidget hw, FILE *infile, int *width, int *height, XColor *colrs,
	int *bg)
{
    png_struct *png_ptr;
    png_info *info_ptr;
    png_color_16 my_background;
    unsigned long bg_pixel = hw->core.background_pixel;
    XColor tmpcolr;
    double screen_gamma;
    png_byte * volatile png_pixels = NULL, ** volatile row_pointers = NULL;
    int intent;
    int i;
    int num;
    static png_color std_color_cube[256];
    static int initialized = 0;
    static int num_colors;

    /* First check to see if it's a valid PNG file.  If not, return.
     * We assume that infile is a valid filepointer */
    {
        png_byte buf[8];

        if ((fread(buf, 1, 8, infile) != 8) || png_sig_cmp(buf, 0, 8))
            return(0);
    }

    /* OK, it is a valid PNG file, so let's rewind it, and start decoding it */
    rewind(infile);

    /* Allocate the structures */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        return 0;

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
	png_destroy_read_struct(&png_ptr, NULL, NULL);
        return 0;
    }

    /* Establish the setjmp return context for png_error to use. */
    if (setjmp(png_ptr->jmpbuf)) {
        
#ifndef DISABLE_TRACE
        if (srcTrace || reportBugs) {
            fprintf(stderr, "\nlibpng read error!!!\n");
        }
#endif
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

        if (png_pixels)
            free((char *)png_pixels);
        if (row_pointers)
	    free((char *)row_pointers);

        return 0;
    }

    /* Set up the input control */
    png_init_io(png_ptr, infile);
    
    /* Read the file information */
    png_read_info(png_ptr, info_ptr);
    
    /* Setup other stuff using the fields of png_info. */
    *width = (int)png_ptr->width;
    *height = (int)png_ptr->height;

#ifndef DISABLE_TRACE
    if (srcTrace) {
        fprintf(stderr, "\nBEFORE\nheight = %d\n", (int)png_ptr->width);
        fprintf(stderr, "width = %d\n", (int)png_ptr->height);
        fprintf(stderr, "bit depth = %d\n", info_ptr->bit_depth);
        fprintf(stderr, "color type = %d\n", info_ptr->color_type);
	if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
            fprintf(stderr,"num colors = %d\n", info_ptr->num_palette);
        fprintf(stderr, "rowbytes = %d\n", (int)info_ptr->rowbytes);
    }
#endif

    /* Strip pixels in 16-bit images down to 8 bits */
    if (info_ptr->bit_depth == 16)
        png_set_strip_16(png_ptr);

    /* Expand grayscale images to the full 8 bits and */
    if (((info_ptr->color_type == PNG_COLOR_TYPE_GRAY) ||
	(info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)) &&
	(info_ptr->bit_depth < 8))
	    png_set_expand(png_ptr);

    /* This handles alpha and transparency by replacing it with 
     * a background value. */
    if ((info_ptr->valid & PNG_INFO_bKGD) ||
	(info_ptr->valid & PNG_INFO_tRNS) ||
	(info_ptr->color_type & PNG_COLOR_MASK_ALPHA)) {
	    /* This should be the current background color */
	    tmpcolr.pixel = bg_pixel;
	    XQueryColor(XtDisplay(hw), hw->core.colormap, &tmpcolr);
	    my_background.index = 0;
	    if (info_ptr->valid & PNG_INFO_bKGD) {
		my_background.gray = info_ptr->background.gray;
	    } else {
		my_background.gray = 127;
	    }
	    if (info_ptr->bit_depth < 16) {
		my_background.red = tmpcolr.red / 256;
		my_background.green = tmpcolr.green / 256;
		my_background.blue = tmpcolr.blue / 256;
	    } else {
		my_background.red = tmpcolr.red;
		my_background.green = tmpcolr.green;
		my_background.blue = tmpcolr.blue;
		if (!(info_ptr->valid & PNG_INFO_bKGD))
		    my_background.gray *= 256;
	    }
	    if (!(info_ptr->color_type & PNG_COLOR_MASK_COLOR))
		my_background.red = my_background.green = my_background.blue =
		    my_background.index = my_background.gray;
	    png_set_background(png_ptr, &my_background,
			       PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
#ifndef DISABLE_TRACE
	    if (srcTrace) {
	        fprintf(stderr, "bg_index = %d\n",
		    (int)png_ptr->background.index);
	        if (!(info_ptr->color_type == PNG_COLOR_MASK_COLOR))
		    fprintf(stderr, "gray_bg = %d\n",
			(int)info_ptr->background.gray);
	    }
#endif
	    *bg = -2;
    }

    /* If it is an RGB image, then dither to browser safe colors
     * or its own palette. */
    if (info_ptr->color_type == PNG_COLOR_TYPE_RGB ||
        info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA) {

        if (!(info_ptr->valid & PNG_INFO_PLTE)) {
		/* Has no palette */
#ifndef DISABLE_TRACE
        	if (srcTrace) {
        		fprintf(stderr, "dithering (RGB->our colors)...\n");
        	}
#endif
		if (!initialized) {
		    initialized = 1;
		    if (browserSafeColors) {
			num_colors = BSCnum;
	                for (i=0; i < BSCnum; i++) {
               		    std_color_cube[i].red = BSColors[i].red >> 8;
                    	    std_color_cube[i].green = BSColors[i].green >> 8;
                	    std_color_cube[i].blue = BSColors[i].blue >> 8;
		        }
		    } else {
                	/* If there is is no valid palette, then we need
			 * to make one up */
			num_colors = 216;
                	for (i=0; i < 216; i++) {
                            /* 255.0/5 = 51 */
                    	    std_color_cube[i].red = (i%6) * 51;
                    	    std_color_cube[i].green = ((i/6)%6) * 51;
                    	    std_color_cube[i].blue = (i/36) * 51;
                	}
		    }
                }
		num = num_colors;
		/* If needed, put background color in cube */
		if (*bg == -2) {
		    std_color_cube[num].red = my_background.red;
		    std_color_cube[num].green = my_background.green;
		    std_color_cube[num].blue = my_background.blue;
		    my_background.index = num++;
#ifndef DISABLE_TRACE
		    if (srcTrace) {
		        fprintf(stderr, "RGB bg_index = %d\n",
			    (int)png_ptr->background.index);

		    }
#endif
		}
                png_set_dither(png_ptr, std_color_cube, num, num, NULL, 1);
        } else {
#ifndef DISABLE_TRACE
            if (srcTrace) {
                fprintf(stderr, "dithering (RGB->file supplied palette)...\n");
            }
#endif
            png_set_dither(png_ptr, info_ptr->palette, 
                           info_ptr->num_palette,
                           (browserSafeColors ? BSCnum :
			    get_pref_int(eCOLORS_PER_INLINED_IMAGE)), 
                           info_ptr->hist, 1);
        }
    }

    /* PNG files pack pixels of bit depths 1, 2, and 4 into bytes as
     * small as they can. This expands pixels to 1 pixel per byte, and
     * if a transparency value is supplied, an alpha channel is built. */
    if (info_ptr->bit_depth < 8)
        png_set_packing(png_ptr);

    /* Have libpng handle the gamma conversion */
    if (png_get_sRGB(png_ptr, info_ptr, &intent)) {
	png_set_sRGB(png_ptr, info_ptr, intent);
    } else {
	if (get_pref_boolean(eUSE_SCREEN_GAMMA)) {
            screen_gamma = (double)(get_pref_float(eSCREEN_GAMMA));
            
#ifndef DISABLE_TRACE
            if (srcTrace) {
                fprintf(stderr, "screen gamma = %f\n", screen_gamma);
            }
#endif
            if (info_ptr->valid & PNG_INFO_gAMA) {
#ifndef DISABLE_TRACE
                if (srcTrace) {
                    fprintf(stderr, "setting gamma = %f\n", info_ptr->gamma);
                }
#endif
                png_set_gamma(png_ptr, screen_gamma, (double)info_ptr->gamma);
            } else {
#ifndef DISABLE_TRACE
                if (srcTrace) {
                    fprintf(stderr, "setting gamma = %f\n", 0.45455);
                }
#endif
                png_set_gamma(png_ptr, screen_gamma, (double)0.45455);
            }
	}
    }
    
    png_read_update_info(png_ptr, info_ptr);
    
#ifndef DISABLE_TRACE
    if (srcTrace) {
        fprintf(stderr, "\nAFTER\nheight = %d\n", (int)png_ptr->width);
        fprintf(stderr, "width = %d\n", (int)png_ptr->height);
        fprintf(stderr, "bit depth = %d\n", info_ptr->bit_depth);
        fprintf(stderr, "color type = %d\n", info_ptr->color_type);
	if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
            fprintf(stderr, "num colors = %d\n", info_ptr->num_palette);
        fprintf(stderr, "rowbytes = %d\n", (int)info_ptr->rowbytes);
	if (*bg == -2) {
            fprintf(stderr, "bg_index = %d\n", (int)info_ptr->background.index);
	    if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
		fprintf(stderr,"gray_bg = %d\n",(int)info_ptr->background.gray);
	}
    }
#endif

    /* Allocate the pixel grid which we will need to send to 
     * png_read_image(). */
    png_pixels = (png_byte *)malloc(info_ptr->rowbytes * 
                                    (*height) * sizeof(png_byte));
    
    row_pointers = (png_byte **) malloc((*height) * sizeof(png_byte *));
    for (i=0; i < *height; i++)
        row_pointers[i] = png_pixels + (info_ptr->rowbytes * i);

    /* FINALLY - read the darn thing. */
    png_read_image(png_ptr, row_pointers);
    
    png_read_end(png_ptr, info_ptr);
    
    /* Now that we have the (transformed to 8-bit RGB) image, we have
     * to copy the resulting palette to our colormap. */
    if (info_ptr->color_type & PNG_COLOR_MASK_COLOR) {
	if (info_ptr->valid & PNG_INFO_PLTE) {
            
            for (i=0; i < info_ptr->num_palette; i++) {
                colrs[i].red = info_ptr->palette[i].red << 8;
                colrs[i].green = info_ptr->palette[i].green << 8;
                colrs[i].blue = info_ptr->palette[i].blue << 8;
                colrs[i].pixel = i;
                colrs[i].flags = DoRed|DoGreen|DoBlue;
            }
#ifndef DISABLE_TRACE
	    if (srcTrace) {
		fprintf(stderr,"using palette map\n");
	    }
#endif
	    if (*bg == -2) {
		if (info_ptr->num_trans) {
		    *bg = 0;
		} else {
		    *bg = info_ptr->background.index;
		}
	    }
            
	} else {
#ifndef DISABLE_TRACE
	    if (srcTrace) {
		fprintf(stderr, "using std_color_cube map of %d colors\n", num);
	    }
#endif
            for (i=0; i < num; i++) {
                colrs[i].red = std_color_cube[i].red << 8;
                colrs[i].green = std_color_cube[i].green << 8;
                colrs[i].blue = std_color_cube[i].blue << 8;
                colrs[i].pixel = i;
                colrs[i].flags = DoRed|DoGreen|DoBlue;
/*
    fprintf(stderr, "%d %d %d %d\n", i, std_color_cube[i].red,
		std_color_cube[i].green, std_color_cube[i].blue);
*/
            }	    
/*
    fprintf(stderr, "Start of pixels = %X\n", *png_pixels);
*/
	    if (*bg == -2) {
		if (info_ptr->num_trans) {
		    *bg = 0;
		} else {
		    *bg = info_ptr->background.index;
/*
    fprintf(stderr, "BG = %d %d %d %d\n", *bg, png_ptr->background.red,
		png_ptr->background.green, png_ptr->background.blue);
    fprintf(stderr, "BG_1 = %d %d %d\n", png_ptr->background_1.red,
		png_ptr->background_1.green, png_ptr->background_1.blue);
*/
		}
	    }
	}
    } else {
        /* Grayscale image */
        for (i=0; i < 256; i++ ) {
            colrs[i].red = i << 8;
            colrs[i].green = i << 8; 	    
            colrs[i].blue = i << 8;
            colrs[i].pixel = i;
            colrs[i].flags = DoRed|DoGreen|DoBlue;    
        }
    }
    
    free((char *)row_pointers);
    
    /* Clean up after the read, and free any memory allocated */
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    
    return png_pixels;
}
#endif
