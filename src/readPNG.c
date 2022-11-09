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

/* Copyright (C) 1998, 1999, 2000, 2004, 2005, 2006, 2007
 * The VMS Mosaic Project
 */

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

#include <stdio.h>

#include "png.h"
#include "mosaic.h"
#include "readPNG.h"
#include "quantize.h"

#include <setjmp.h>

extern int browserSafeColors;
extern int BSCnum;
extern int Vclass;

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int reportBugs;
#endif

/* Routines to suppress error messages */
static void PNG_error(png_structp png_ptr, png_const_charp message)
{
    longjmp(png_ptr->jmpbuf, 1);
}

static void PNG_warning(png_structp png_ptr, png_const_charp message)
{
    return;
}


unsigned char *ReadPNG(Widget hw, FILE *infile, int *width, int *height,
		       XColor *colrs, int *bg, unsigned char **alpha)
{
    png_struct *png_ptr;
    png_info *info_ptr;
    png_color_16 my_background;
    png_byte buf[8];
    png_byte *volatile pixels = NULL;
    png_byte **volatile row_pointers = NULL;
    int intent, i, j, h, w;
    int has_alpha = 0;
    int has_background = 0;
    int is_RGB = 0;
    static int init = 0;		/* Preferences */
    static int err_msgs, colors_per_image, use_screen_gamma;
    static double screen_gamma;

    /* Get preference stuff initialized */
    if (!init) {
	use_screen_gamma = get_pref_boolean(eUSE_SCREEN_GAMMA);
	if (use_screen_gamma)
	    screen_gamma = (double)get_pref_float(eSCREEN_GAMMA);

	if ((Vclass == TrueColor) || (Vclass == DirectColor)) {
	    colors_per_image = 256;
	} else {
	    colors_per_image = get_pref_int(eCOLORS_PER_INLINED_IMAGE);
	}
	err_msgs = get_pref_boolean(ePNG_ERROR_MESSAGES);
	init = 1;
    }

    /* First check to see if it's a valid PNG file.  If not, return.
     * We assume that infile is a valid filepointer */
    if ((fread(buf, 1, 8, infile) != 8) || png_sig_cmp(buf, 0, 8))
        return NULL;

    /* Rewind it and start decoding */
    rewind(infile);

    /* Allocate the structures */
    if (err_msgs) {
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
					 NULL, NULL, NULL);
    } else {
	/* No error messages */
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
					 PNG_error, PNG_warning);
    }
    if (!png_ptr)
        return NULL;

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
	png_destroy_read_struct(&png_ptr, NULL, NULL);
        return NULL;
    }

    /* Establish the setjmp return context for png_error to use. */
    if (setjmp(png_ptr->jmpbuf)) {
        
#ifndef DISABLE_TRACE
        if (srcTrace || reportBugs)
            fprintf(stderr, "libpng read error!\n");
#endif
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

        if (pixels)
            free((char *)pixels);
        if (row_pointers)
	    free((char *)row_pointers);
        return NULL;
    }

    /* Set up the input control */
    png_init_io(png_ptr, infile);
    
    /* Read the file information */
    png_read_info(png_ptr, info_ptr);
    
    /* Setup other stuff using the fields of png_info. */
    *width = w = (int)png_ptr->width;
    *height = h = (int)png_ptr->height;

#ifndef DISABLE_TRACE
    if (srcTrace) {
        fprintf(stderr, "\nBEFORE\nheight = %d\n", h);
        fprintf(stderr, "width = %d\n", w);
        fprintf(stderr, "bit depth = %d\n", info_ptr->bit_depth);
        fprintf(stderr, "color type = %d\n", info_ptr->color_type);
	if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
            fprintf(stderr, "num colors = %d\n", info_ptr->num_palette);
        fprintf(stderr, "rowbytes = %d\n", (int)info_ptr->rowbytes);
    }
#endif

    /* Strip pixels in 16-bit images down to 8 bits */
    if (info_ptr->bit_depth == 16)
        png_set_strip_16(png_ptr);

    /* Expand grayscale images to the full 8 bits */
    if ((info_ptr->color_type == PNG_COLOR_TYPE_GRAY) ||
	(info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)) {
	if (info_ptr->bit_depth < 8)
	    png_set_gray_1_2_4_to_8(png_ptr);
	/* Deal with pnglib insanity */
	png_set_strip_alpha(png_ptr);
    }
    /* Handle alpha channel later */
    if ((info_ptr->color_type & PNG_COLOR_MASK_ALPHA) ||
	(info_ptr->valid & PNG_INFO_tRNS)) {
	has_alpha = 1;

    /* Handle non-alpha transparency by setting a background value */
    } else if (info_ptr->valid & PNG_INFO_bKGD) {
	XColor tmpcolr;
	Colormap colmap;

	/* This should be the current background color */
	XtVaGetValues(hw,
		      XtNbackground, &tmpcolr.pixel,
		      XtNcolormap, &colmap,
		      NULL);
	XQueryColor(dsp, colmap, &tmpcolr);
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
	has_background = 1;
#ifndef DISABLE_TRACE
	if (srcTrace) {
	    fprintf(stderr, "bg_index = %d\n", (int)png_ptr->background.index);
	    if (!(info_ptr->color_type == PNG_COLOR_MASK_COLOR))
		fprintf(stderr, "gray_bg = %d\n",
			(int)info_ptr->background.gray);
	}
#endif
	*bg = -2;
    }

    if (info_ptr->color_type == PNG_COLOR_TYPE_RGB ||
        info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
	if (!(info_ptr->valid & PNG_INFO_PLTE)) {
	    /* Go handle RGB or RGBA without a palette */
	    is_RGB = 1;
	    goto do_RGB;
        } else {
	    /* RGB image with a palette */
	    png_uint_16p histogram = NULL;

	    png_get_hIST(png_ptr, info_ptr, &histogram);

	    if (!histogram) {
		/* If no histogram, our dither will be better */
#ifndef DISABLE_TRACE
		if (srcTrace)
		    fprintf(stderr, "PNG: RGB palette with no histogram\n");
#endif
		is_RGB = 1;
		goto do_RGB;
	    } else {
#ifndef DISABLE_TRACE
		if (srcTrace)
		    fprintf(stderr, "PNG: RGB palette with %d colors\n",
			    info_ptr->num_palette);
#endif
		png_set_dither(png_ptr, info_ptr->palette, 
                               info_ptr->num_palette,
                               browserSafeColors ? BSCnum : colors_per_image,
                               histogram, 1);
	    }
        }
    }

    /* PNG files pack pixels of bit depths 1, 2, and 4 into bytes as
     * small as they can.  This expands pixels to 1 pixel per byte. */
    if (info_ptr->bit_depth < 8)
        png_set_packing(png_ptr);

    /* Have libpng handle the gamma conversion */
    if (png_get_sRGB(png_ptr, info_ptr, &intent)) {
	png_set_sRGB(png_ptr, info_ptr, intent);
    } else if (use_screen_gamma) {
#ifndef DISABLE_TRACE
        if (srcTrace)
            fprintf(stderr, "screen gamma = %f\n", screen_gamma);
#endif
        if (info_ptr->valid & PNG_INFO_gAMA) {
#ifndef DISABLE_TRACE
            if (srcTrace)
                fprintf(stderr, "setting gamma = %f\n", info_ptr->gamma);
#endif
            png_set_gamma(png_ptr, screen_gamma, (double)info_ptr->gamma);
        } else {
#ifndef DISABLE_TRACE
            if (srcTrace)
                fprintf(stderr, "setting gamma = %f\n", 0.45455);
#endif
            png_set_gamma(png_ptr, screen_gamma, (double)0.45455);
        }
    }
    
    png_read_update_info(png_ptr, info_ptr);
    
    /* Allocate the pixel grid which we will need to send to 
     * png_read_image(). */
    pixels = (png_byte *)malloc(info_ptr->rowbytes * h * sizeof(png_byte));
    
    row_pointers = (png_byte **)malloc(h * sizeof(png_byte *));
    for (i = 0; i < h; i++)
        row_pointers[i] = pixels + (info_ptr->rowbytes * i);

    /* FINALLY - read the darn thing. */
    png_read_image(png_ptr, row_pointers);
    
    png_read_end(png_ptr, info_ptr);
    
#ifndef DISABLE_TRACE
    if (srcTrace) {
        fprintf(stderr, "\nAFTER\nheight = %d\n", (int)png_ptr->height);
        fprintf(stderr, "width = %d\n", (int)png_ptr->width);
        fprintf(stderr, "bit depth = %d\n", info_ptr->bit_depth);
        fprintf(stderr, "color type = %d\n", info_ptr->color_type);
	if (info_ptr->color_type & PNG_COLOR_MASK_COLOR)
            fprintf(stderr, "num colors = %d\n", info_ptr->num_palette);
        fprintf(stderr, "rowbytes = %d\n", (int)info_ptr->rowbytes);
	if (*bg == -2) {
            fprintf(stderr, "bg_index = %d\n", (int)info_ptr->background.index);
	    if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
		fprintf(stderr, "gray_bg = %d\n",
			(int)info_ptr->background.gray);
	}
    }
#endif
    /* Now that we have the (8-bit indexed) image, we have
     * to copy the resulting palette to our colormap. */
    if (info_ptr->color_type & PNG_COLOR_MASK_COLOR) {
        for (i = 0; i < info_ptr->num_palette; i++) {
            colrs[i].red = info_ptr->palette[i].red << 8;
            colrs[i].green = info_ptr->palette[i].green << 8;
            colrs[i].blue = info_ptr->palette[i].blue << 8;
            colrs[i].pixel = i;
            colrs[i].flags = DoRed | DoGreen | DoBlue;
        }
#ifndef DISABLE_TRACE
	if (srcTrace)
	    fprintf(stderr, "using palette map\n");
#endif
	if (*bg == -2) {
	    if (info_ptr->num_trans) {
		*bg = 0;
	    } else {
		*bg = info_ptr->background.index;
	    }
	}
    } else {
        /* Grayscale image */
        for (i = 0; i < 256; i++ ) {
            colrs[i].red = i << 8;
            colrs[i].green = i << 8; 	    
            colrs[i].blue = i << 8;
            colrs[i].pixel = i;
            colrs[i].flags = DoRed | DoGreen | DoBlue;    
        }
    }
    
    free((char *)row_pointers);

 do_RGB:

    /* Clean up after the read (or get info only) and free memory allocated */
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    /* Now handle RGB images and get the alpha channel if needed */
    if (has_alpha || is_RGB) {
	unsigned char *a;
	png_byte *volatile RGBA_pixels = NULL;

#ifndef DISABLE_TRACE
	if (srcTrace)
	    fprintf(stderr, "\nPNG: Get RGB or RGBA image\n");
#endif
	/* Null it in case setjmp activated */
	row_pointers = NULL;

	rewind(infile);

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
					 NULL, NULL, NULL);
	if (!png_ptr)
	    return NULL;

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
	    png_destroy_read_struct(&png_ptr, NULL, NULL);
	    return NULL;
	}

	if (setjmp(png_ptr->jmpbuf)) {
#ifndef DISABLE_TRACE
	    if (srcTrace || reportBugs)
		fprintf(stderr, "\nlibpng RGBA read error!\n");
#endif
	    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	    if (pixels)
		free((char *)pixels);
	    if (RGBA_pixels)
		free((char *)RGBA_pixels);
	    if (row_pointers)
		free((char *)row_pointers);
	    return NULL;
	}

	png_init_io(png_ptr, infile);
	png_read_info(png_ptr, info_ptr);

	/* Force to RGB with Alpha */
	png_set_expand(png_ptr);

	/* Force RGBA even if no Alpha */
        if (!(info_ptr->color_type & PNG_COLOR_MASK_ALPHA))
	    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

	if (info_ptr->bit_depth == 16)
	    png_set_strip_16(png_ptr);

	if (has_background)
	    png_set_background(png_ptr, &my_background,
			       PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
	if (info_ptr->bit_depth < 8)
	    png_set_packing(png_ptr);

	/* Make it all RGB */
	if ((info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA) ||
	    (info_ptr->color_type == PNG_COLOR_TYPE_GRAY))
	    png_set_gray_to_rgb(png_ptr);

	if (png_get_sRGB(png_ptr, info_ptr, &intent)) {
	    png_set_sRGB(png_ptr, info_ptr, intent);
	} else if (use_screen_gamma) {
	    if (info_ptr->valid & PNG_INFO_gAMA) {
		png_set_gamma(png_ptr, screen_gamma, (double)info_ptr->gamma);
	    } else {
		png_set_gamma(png_ptr, screen_gamma, (double)0.45455);
	    }
	}
    
	png_read_update_info(png_ptr, info_ptr);

	RGBA_pixels = (png_byte *)malloc(info_ptr->rowbytes * h *
					 sizeof(png_byte));
	row_pointers = (png_byte **)malloc(h * sizeof(png_byte *));
	for (i = 0; i < h; i++)
	    row_pointers[i] = RGBA_pixels + (info_ptr->rowbytes * i);

	png_read_image(png_ptr, row_pointers);

#ifndef DISABLE_TRACE
	if (srcTrace) {
	    fprintf(stderr, "\nAFTER RGBA \n");
	    fprintf(stderr, "bit depth = %d\n", info_ptr->bit_depth);
	    fprintf(stderr, "color type = %d\n", info_ptr->color_type);
	    if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
		fprintf(stderr, "num colors = %d\n", info_ptr->num_palette);
	    fprintf(stderr, "rowbytes = %d\n", (int)info_ptr->rowbytes);
	    if (*bg == -2) {
		fprintf(stderr, "bg_index = %d\n",
			(int)info_ptr->background.index);
		if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
		    fprintf(stderr, "gray_bg = %d\n",
			    (int)info_ptr->background.gray);
	    }
	}
#endif
	png_read_end(png_ptr, info_ptr);
	free((char *)row_pointers);

	if (is_RGB) {
	    pixels = QuantizeImage(RGBA_pixels, w, h, 256, 1, colrs, 1);
#ifndef DISABLE_TRACE
	    if (srcTrace)
	        fprintf(stderr, "Quantized 24 bit PNG\n");
#endif
	}
	if (has_alpha) {
	    int num = h * w;

	    *alpha = a = malloc(num);
	    for (i = 0, j = 3; i < num; i++, j += 4)
	        *a++ = RGBA_pixels[j];
#ifndef DISABLE_TRACE
	    if (srcTrace)
	        fprintf(stderr, "Got PNG alpha channel\n");
#endif
	}
	free((char *)RGBA_pixels);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    }
    
    return pixels;
}
#endif
