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

#include "../config.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "HTMLparse.h"
#include "HTMLP.h"
#include "HTMLPutil.h"
#include "HTMLfont.h"

#include "../src/mosaic.h"
#include "../src/img.h"
#include "../src/fsdither.h"
#include "../src/mo-www.h"

#include <X11/Intrinsic.h>
#ifndef MOTIF1_2
#undef XtIsRealized	     /* Motif 1.1 definition causes build failure */
#endif

#include "NoImage.xbm"
#include "DelayedImage.xbm"
#include "AnchoredDelayedImage.xbm"

#include "../src/bitmaps/gopher_image.xbm"
#include "../src/bitmaps/gopher_movie.xbm"
#include "../src/bitmaps/gopher_menu.xbm"
#include "../src/bitmaps/gopher_text.xbm"
#include "../src/bitmaps/gopher_sound.xbm"
#include "../src/bitmaps/gopher_index.xbm"
#include "../src/bitmaps/gopher_telnet.xbm"
#include "../src/bitmaps/gopher_binary.xbm"
#include "../src/bitmaps/gopher_unknown.xbm"

#ifndef DISABLE_TRACE
extern int htmlwTrace;
extern int reportBugs;
#endif

static ImageInfo *gopher_image = NULL;
static ImageInfo *gopher_movie = NULL;
static ImageInfo *gopher_menu = NULL;
static ImageInfo *gopher_text = NULL;
static ImageInfo *gopher_sound = NULL;
static ImageInfo *gopher_index = NULL;
static ImageInfo *gopher_telnet = NULL;
static ImageInfo *gopher_binary = NULL;
static ImageInfo *gopher_unknown = NULL;

extern Pixmap internalCookie;

extern GC maskGC;   /* PGE, transparent on solid background speedup */

extern int Select_GC_Reset;

/* For selective image loading */
extern Boolean currently_delaying_images;

/* Visual and class of display, from GUI.C */
extern int Vclass;
extern Visual *theVisual;

/* Display dimension limit stuff from GUI.C */
extern int LimDim;
extern int LimDimX;
extern int LimDimY;
 
/* From COLORS.C */
extern XColor BSColors[256];
extern int BSCnum;

extern int installed_colormap;
extern Colormap installed_cmap;

static void ImageAnimate(XtPointer cld, XtIntervalId *id);

/* Static as long as we only support one display at a time */
static XtAppContext app_con = NULL;

int progressiveDisplayEnabled;

#if defined(__STDC__) || defined(__sgi)
#define IMGINFO_FROM_BITMAP(x) \
{ \
	if (!x) { \
		x = GetImageRec(); \
		x->image = XCreatePixmapFromBitmapData(dsp, XtWindow(hw), \
						   (char *) x##_bits,  \
						   x##_width, x##_height, \
						   fg_pixel, bg_pixel, depth); \
	} \
	picd->width = x##_width; \
	picd->height = x##_height; \
	picd->req_width = -1; \
	picd->req_height = -1; \
	picd->image_data = (unsigned char *) x##_bits; \
	picd->internal = 1; \
	picd->delayed = 0; \
	picd->num_colors = 2; \
	picd->image = x->image; \
}
#else
#define IMGINFO_FROM_BITMAP(x) \
{ \
	if (!x) { \
		x = GetImageRec(); \
		x->image = XCreatePixmapFromBitmapData(dsp, XtWindow(hw), \
						   (char *) x/**/_bits,  \
						   x/**/_width, x/**/_height, \
						   fg_pixel, bg_pixel, depth); \
	} \
	picd->width = x/**/_width; \
	picd->height = x/**/_height; \
	picd->req_width = -1; \
	picd->req_height = -1; \
	picd->image_data = (unsigned char *) x/**/_bits; \
	picd->internal = 1; \
	picd->delayed = 0; \
	picd->num_colors = 2; \
	picd->image = x->image; \
}
#endif

/*
 * Free all the widget's colors in the default colormap that we have
 * allocated so far.
 */
void FreeColors(HTMLWidget hw, Colormap colormap)
{
	int i, j;
	unsigned long pix;
	Display *d = hw->html.dsp;

	for (i = 0; i < 256; i++) {
		if (hw->html.allocation_index[i]) {
			pix = (unsigned long)i;
			/* Because X is stupid, we have to Free the color
			 * once for each time we've allocated it.
			 */
			for (j = 0; j < hw->html.allocation_index[i]; j++)
				XFreeColors(d, colormap, &pix, 1, 0L);
			hw->html.allocation_index[i] = 0;
		}
	}
}

/*
 * Find closest color by allocating it, or picking an already allocated color
 */
int FindColor(HTMLWidget hw, Colormap colormap, XColor *colr)
{
	int match, ret;
	XColor tempcolr;
	static XColor def_colrs[256];
	static int have_colors = 0;
	Display *d = hw->html.dsp;

        tempcolr.red = colr->red;      
        tempcolr.green = colr->green;  
        tempcolr.blue = colr->blue;    
        /* XAllocColor doesn't use flags */

	ret = match = XAllocColor(d, colormap, colr);
	if (!match) {
		int rd, gd, bd, dist, mindist;
		int NumCells, cindx, i;

                colr->red = tempcolr.red;
                colr->green = tempcolr.green;
                colr->blue = tempcolr.blue;  

		NumCells = DisplayCells(d, DefaultScreen(d));
		if (!have_colors) {
			for (i = 0; i < NumCells; i++)
				def_colrs[i].pixel = i;
			XQueryColors(d, colormap, def_colrs, NumCells);
			have_colors = 1;
		}
		mindist = 196608;		/* 256 * 256 * 3 */
		cindx = -1;
		for (i = 0; i < NumCells; i++) {
			rd = (int)(def_colrs[i].red >> 8) -
			     (int)(colr->red >> 8);
			gd = (int)(def_colrs[i].green >> 8) -
			     (int)(colr->green >> 8);
			bd = (int)(def_colrs[i].blue >> 8) -
			     (int)(colr->blue >> 8);
			dist = (rd * rd) + (gd * gd) + (bd * bd);
			if (dist < mindist) {
				mindist = dist;
				cindx = def_colrs[i].pixel;
				if (dist == 0)
					break;
			}
		}
                if (cindx == (-1)) {   
                        colr->pixel = BlackPixel(d, DefaultScreen(d));
                        colr->red = colr->green = colr->blue = 0;
                } else {               
                        colr->pixel = cindx;
                        colr->red = def_colrs[cindx].red;
                        colr->green = def_colrs[cindx].green;
                        colr->blue = def_colrs[cindx].blue;
                } 
	} else {
		/* Keep a count of how many times we have allocated the
		 * same color, so we can properly free them later.
		 */
		hw->html.allocation_index[colr->pixel]++;
		/* If this is a new color, we've actually changed default
		 * colormap, and may have to re-query it later.
		 */
		if (hw->html.allocation_index[colr->pixel] == 1)
			have_colors = 0;
	}
	return ret;
}

/* Returns position of highest set bit in 'ul' as an integer (0-31),
 * or -1 if none.
 */
static int highbit(unsigned long ul)
{
	int i;

	for (i = 31; !(ul & 0x80000000) && i >= 0; i--, ul <<= 1)
	    ;
	return i;
}


/*
 * Rescale an image GD 24 Apr 97
 * From the XV Software 3.10a.  See the copyright notice of xv-3.10a 
 */
static void RescalePic(ImageInfo *picd, int nw, int nh)
{ 
	/* Change image_data width and height in picd */
	int cy = 0;
	int ex, ey;
	int *cxarr;
	int *cxarrp = NULL;
	unsigned char *elptr, *epptr, *epic;
	unsigned char *aptr, *apptr;
	unsigned char *alptr = NULL;
	unsigned char *alpha = NULL;
	unsigned char *clptr = NULL;

	/* Create a new pic of the appropriate size */
	elptr = epic = (unsigned char *) malloc((size_t) (nw * nh));
	cxarr = (int *) malloc(nw * sizeof(int));

	if (picd->alpha)
		alptr = alpha = (unsigned char *) malloc((size_t) (nw * nh));

	if (!epic || !cxarr || (picd->alpha && !alpha)) {
		if (epic)
			free(epic);
		if (cxarr)
			free(cxarr);
		if (alpha)
			free(alpha);
		fprintf(stderr, "Rescale image memory overflow\n");
		return;
	}

	/* The scaling routine.  Not really all that scary after all...
	 * OPTIMIZATON:  Malloc an nw array of ints which will hold the
	 * values of the equation px = (pWIDE * ex) / nw.  Faster than doing
	 * a mul and a div for every point in picture. */
	for (ex = 0; ex < nw; ex++)        
		cxarr[ex] = (picd->width * ex) / nw;

	for (ey = 0; ey < nh; ey++, elptr += nw) {
		cy = (picd->height * ey) / nh;      
		epptr = elptr;
		clptr = picd->image_data + (cy * picd->width);
		if (alpha) {
			apptr = alptr;
			alptr += nw;
			aptr = picd->alpha + (cy * picd->width); 
		}
		for (ex = 0, cxarrp = cxarr; ex < nw; ex++, epptr++) {
			*epptr = clptr[*cxarrp];
			if (alpha)
				*apptr++ = aptr[*cxarrp];
			cxarrp++;
		}
	}                                 
	free(cxarr);

	picd->image_data = epic;
	picd->alpha = alpha;
	picd->width = nw;
	picd->height = nh;
}

/*
 * Make an image of appropriate depth for display from image data.
 *
 * NOTE: as of 2.7b5 it appears that this is never called with "clip"
 * nonzero.  The 2.7b5 code ignored "clip" in most code paths, so I'm
 * just going to ignore it in all paths.
 */
static XImage *MakeImage(Display *dsp, unsigned char *data, int depth,
		         ImageInfo *img_info, XColor *cmap, int clip)
{
    XImage *newimage;
    unsigned char *bit_data, *bitp, *endofline;
    unsigned char *datap = data;
    unsigned long c;
    int shiftnum, shiftstart, shiftstop, shiftinc;
    int rshift, gshift, bshift, rmask, gmask, bmask;
    int bpp, temp, w, h;
    int width = img_info->width;
    int height = img_info->height;

    /* We create the XImage first so that we can rely on Xlib to choose
     * the correct bits_per_pixel and scanline width for the image.
     * It's OK to pass a NULL data pointer to XCreateImage.
     * Note we use a worst-case assumption of bitmap_pad = 32.
     */
    newimage = XCreateImage(dsp, theVisual, depth, ZPixmap, 0,
			    (char *) NULL, width, height, 32, 0);
    if (!newimage) {
	application_user_feedback("Image too large:  XCreateImage failure");
	return NULL;
    } 

    /* Allocate data space using scanline width from XCreateImage. */
    bitp = bit_data = (unsigned char *) malloc(newimage->bytes_per_line *
					       height);
    newimage->data = (char *) bit_data;
    if (!bit_data) {
	XDestroyImage(newimage);
	application_user_feedback("Image data too large:  Not enough memory");
	return NULL;
    }

    /* Fill in the image data. */
    bpp = newimage->bits_per_pixel;  /* Not always the same as depth! */

    switch (bpp) {
       case 1:
       case 2:
       case 4:
	  /* FIXME: this code assumes byte_order == bitmap_bit_order */
	  if (newimage->bitmap_bit_order == LSBFirst) {
	    shiftstart = 0;
	    shiftstop = 8;
	    shiftinc = bpp;
	  } else {
	    shiftstart = 8 - bpp;
	    shiftstop = (-bpp);
	    shiftinc = (-bpp);
	  }
	  for (h = 0; h < height; h++) {
	    endofline = bitp + newimage->bytes_per_line;
	    temp = 0;
	    shiftnum = shiftstart;
	    for (w = 0; w < width; w++) {
	      temp |= (*datap++) << shiftnum;
	      shiftnum += shiftinc;
	      if (shiftnum == shiftstop) {
		*bitp++ = (unsigned char) temp;
		temp = 0;
		shiftnum = shiftstart;
	      }
	    }
	    if (bitp != endofline) {
	      /* Dump out last partial byte */
	      *bitp++ = (unsigned char) temp;
	      /* Zero-pad; probably not really necessary */
	      while (bitp != endofline)
		*bitp++ = '\0';
	    }
	  }
	  break;

       case 8:
	  if (newimage->bytes_per_line == width) {
	    /* Easy if no padding needed */
	    memcpy(bitp, datap, width * height);
	  } else {
	    /* Copy a scanline at a time; don't bother to fill pad bytes */
	    for (h = 0; h < height; h++) {
	      memcpy(bitp, datap, width);
	      datap += width;
	      bitp += newimage->bytes_per_line;
	    }
	  }
	  break;

	  /*
	   * Donated by - nosmo@ximage.com
	   */
       case 16:
	  rmask = theVisual->red_mask;
	  gmask = theVisual->green_mask;
	  bmask = theVisual->blue_mask;
	  rshift = 15 - highbit(rmask);
	  gshift = 15 - highbit(gmask);
	  bshift = 15 - highbit(bmask);

	  if (newimage->byte_order == MSBFirst) {
	    for (h = 0; h < height; h++) {
	      endofline = bitp + newimage->bytes_per_line;
	      for (w = width; w > 0; w--) {
		temp = (int) *datap++;
		temp = ((cmap[temp].red >> rshift) & rmask) |
		       ((cmap[temp].green >> gshift) & gmask) |
		       ((cmap[temp].blue >> bshift) & bmask);
		*bitp++ = (temp >> 8) & 0xff;
		*bitp++ = temp & 0xff;
	      }
	      bitp = endofline;
	    }
	  } else {
	    for (h = 0; h < height; h++) {
	      endofline = bitp + newimage->bytes_per_line;
	      for (w = width; w > 0; w--) {
		temp = (int) *datap++;
		temp = ((cmap[temp].red >> rshift) & rmask) |
		       ((cmap[temp].green >> gshift) & gmask) |
		       ((cmap[temp].blue >> bshift) & bmask);
		*bitp++ = temp & 0xff;
		*bitp++ = (temp >> 8) & 0xff;
	      }
	      bitp = endofline;
	    }
	  }
	  break;

       case 32:
	  /* Bletcherous code ... assumes masks are 8 bits wide. */
	  rshift = highbit(theVisual->red_mask) - 7;
	  gshift = highbit(theVisual->green_mask) - 7;
	  bshift = highbit(theVisual->blue_mask) - 7;

	  for (h = 0; h < height; h++) {
	    endofline = bitp + newimage->bytes_per_line;
	    for (w = width; w > 0; w--) {
	      temp = (int) *datap++;
	      c = (((cmap[temp].red >> 8) & 0xff) << rshift) |
		  (((cmap[temp].green >> 8) & 0xff) << gshift) |
		  (((cmap[temp].blue >> 8) & 0xff) << bshift);

	      if (newimage->byte_order == MSBFirst) {
		*bitp++ = (unsigned char)((c >> 24) & 0xff);
		*bitp++ = (unsigned char)((c >> 16) & 0xff);
		*bitp++ = (unsigned char)((c >> 8) & 0xff);
		*bitp++ = (unsigned char)(c & 0xff);
	      } else {
		*bitp++ = (unsigned char)(c & 0xff);
		*bitp++ = (unsigned char)((c >> 8) & 0xff);
		*bitp++ = (unsigned char)((c >> 16) & 0xff);
		*bitp++ = (unsigned char)((c >> 24) & 0xff);
	      }
	    }
	    bitp = endofline;
	  }
	  break;

       default:
#ifndef DISABLE_TRACE
	  if (reportBugs)
	    fprintf(stderr, "Don't know how to format pixmap of depth %d\n",
		    bpp);
#endif
	  XDestroyImage(newimage);
	  return(NULL);
    }

    return(newimage);
}

typedef struct img_list {
	Pixmap img;
	Pixel fg_pixel;
	Pixel bg_pixel;
	struct img_list *next;
} ImgList;

static ImageInfo *DelayedImageData(HTMLWidget hw)
{
	static Pixel fg_pixel, bg_pixel;
	static ImgList *images;
	static ImageInfo *delayed_image = NULL;
	ImgList *next;

        if (!delayed_image) {
		/* First time */
		images = (ImgList *)malloc(sizeof(ImgList));
		images->next = NULL;
		delayed_image = GetImageRec();
                                 
		delayed_image->internal = 2;
		delayed_image->delayed = 1;
		delayed_image->width = DelayedImage_width;
		delayed_image->height = DelayedImage_height;
		delayed_image->num_colors = 2;
		delayed_image->image_data = (unsigned char *)DelayedImage_bits;
		images->fg_pixel = fg_pixel = hw->manager.foreground;
		images->bg_pixel = bg_pixel = hw->core.background_pixel;
                delayed_image->image = XCreatePixmapFromBitmapData(hw->html.dsp,
				       XtWindow(hw), (char *) DelayedImage_bits,
                        	       DelayedImage_width, DelayedImage_height,
                        	       fg_pixel, bg_pixel,
                        	       DefaultDepthOfScreen(XtScreen(hw)));
		images->img = delayed_image->image;
	} else if ((bg_pixel != hw->core.background_pixel) ||
	           (fg_pixel != hw->manager.foreground)) {
		/* Check for match in cached images */
		next = images;
		while (next) {
		    if ((next->bg_pixel == hw->core.background_pixel) &&
			(next->fg_pixel == hw->manager.foreground))
			break;
		    next = next->next;
		}
		if (next) {
		    fg_pixel = next->fg_pixel;
		    bg_pixel = next->bg_pixel;
		    delayed_image->image = next->img;
		} else {
		    /* Didn't find a match, so create a new one */
		    next = images;
		    images = (ImgList *)malloc(sizeof(ImgList));
		    images->next = next;
		    images->fg_pixel = fg_pixel = hw->manager.foreground;
		    images->bg_pixel = bg_pixel = hw->core.background_pixel;
		    delayed_image->image = XCreatePixmapFromBitmapData(
				       hw->html.dsp, XtWindow(hw),
				       (char *) DelayedImage_bits,
                        	       DelayedImage_width, DelayedImage_height,
                        	       fg_pixel, bg_pixel,
                        	       DefaultDepthOfScreen(XtScreen(hw)));
		    images->img = delayed_image->image;
		}
	}
	return(delayed_image);
}

static ImageInfo *AnchoredImageData(HTMLWidget hw)
{
	static Pixel fg_pixel, bg_pixel;
	static ImgList *images;
	static ImageInfo *anchored_image = NULL;
	ImgList *next;

        if (!anchored_image) {
		/* First time */
		images = (ImgList *)malloc(sizeof(ImgList));
		images->next = NULL;
		anchored_image = GetImageRec();

		anchored_image->internal = 2;
		anchored_image->delayed = 1;
		anchored_image->width = AnchoredImage_width;
		anchored_image->height = AnchoredImage_height;
		anchored_image->num_colors = 2;
		anchored_image->image_data =(unsigned char *)AnchoredImage_bits;
		images->fg_pixel = fg_pixel = hw->manager.foreground;
		images->bg_pixel = bg_pixel = hw->core.background_pixel;
                anchored_image->image = XCreatePixmapFromBitmapData(
			 hw->html.dsp, XtWindow(hw), (char *)AnchoredImage_bits,
                         AnchoredImage_width, AnchoredImage_height,
                         fg_pixel, bg_pixel,
                         DefaultDepthOfScreen(XtScreen(hw)));
		images->img = anchored_image->image;
	} else if ((bg_pixel != hw->core.background_pixel) ||
	           (fg_pixel != hw->manager.foreground)) {
		/* Check for match in cached images */
		next = images;
		while (next) {
		    if ((next->bg_pixel == hw->core.background_pixel) &&
			(next->fg_pixel == hw->manager.foreground))
			break;
		    next = next->next;
		}
		if (next) {
		    fg_pixel = next->fg_pixel;
		    bg_pixel = next->bg_pixel;
		    anchored_image->image = next->img;
		} else {
		    /* Didn't find a match, so create a new one */
		    next = images;
		    images = (ImgList *)malloc(sizeof(ImgList));
		    images->next = next;
		    images->fg_pixel = fg_pixel = hw->manager.foreground;
		    images->bg_pixel = bg_pixel = hw->core.background_pixel;
		    anchored_image->image = XCreatePixmapFromBitmapData(
				      hw->html.dsp, XtWindow(hw),
				      (char *)AnchoredImage_bits,
				      AnchoredImage_width, AnchoredImage_height,
				      fg_pixel, bg_pixel,
				      DefaultDepthOfScreen(XtScreen(hw)));
		    images->img = anchored_image->image;
		}
	}
	return(anchored_image);
}

static ImageInfo *NoImageData(HTMLWidget hw)
{
	static Pixel fg_pixel, bg_pixel;
	static ImgList *images;
	static ImageInfo *no_image = NULL;
	ImgList *next;

	if (!no_image) {
		/* First time */
		images = (ImgList *)malloc(sizeof(ImgList));
		images->next = NULL;
		no_image = GetImageRec();

		no_image->internal = 2;
		no_image->delayed = 0;
		no_image->width = NoImage_width;
		no_image->height = NoImage_height;
		no_image->num_colors = 2;
		no_image->image_data = (unsigned char *)NoImage_bits;
		images->fg_pixel = fg_pixel = hw->manager.foreground;
		images->bg_pixel = bg_pixel = hw->core.background_pixel;
		images->img = no_image->image = XCreatePixmapFromBitmapData(
					    hw->html.dsp, XtWindow(hw),
					    (char *) NoImage_bits,
					    NoImage_width, NoImage_height,
                        		    fg_pixel, bg_pixel,
					    DefaultDepthOfScreen(XtScreen(hw)));
	} else if ((bg_pixel != hw->core.background_pixel) ||
	           (fg_pixel != hw->manager.foreground)) {
		/* Check for match in cached images */
		next = images;
		while (next) {
		    if ((next->bg_pixel == hw->core.background_pixel) &&
			(next->fg_pixel == hw->manager.foreground))
			break;
		    next = next->next;
		}
		if (next) {
		    fg_pixel = next->fg_pixel;
		    bg_pixel = next->bg_pixel;
		    no_image->image = next->img;
		} else {
		    /* Didn't find a match, so create a new one */
		    next = images;
		    images = (ImgList *)malloc(sizeof(ImgList));
		    images->next = next;
		    images->fg_pixel = fg_pixel = hw->manager.foreground;
		    images->bg_pixel = bg_pixel = hw->core.background_pixel;
		    images->img = no_image->image = XCreatePixmapFromBitmapData(
					    hw->html.dsp, XtWindow(hw),
					    (char *) NoImage_bits,
                        		    NoImage_width, NoImage_height,
                        		    fg_pixel, bg_pixel,
                        		    DefaultDepthOfScreen(XtScreen(hw)));
		}
	}
	return(no_image);
}


/* Apply alpha channel to image data */ 
static unsigned char *ProcessImageAlpha(HTMLWidget hw, ImageInfo *pic,
					ElemInfo *eptr)
{
	XColor *cmap = pic->colrs;
	XColor bg;
	unsigned char *alpha = pic->alpha;
	unsigned char *p1 = pic->image_data;
	unsigned char *p2;
	unsigned char cnum = pic->num_colors;
	unsigned char A;
	int i, j, R, G, B, RB, GB, BB;
	int h = pic->height;
	int w = pic->width;

	if (!alpha)
		return(NULL);

	if (!(p2 = (unsigned char *)malloc(h * w * 3))) {
		application_user_feedback(
				   "Alpha image too large:  Not enough memory");
		return(NULL);
	}
	pic->rgb = p2;

	if (hw->html.bgmap_SAVE && eptr) {
		unsigned char *p3, *bg_line_end;
		unsigned char *bg_data = hw->html.bg_pic_data->image_data;
		XColor *bg_cmap = hw->html.bg_pic_data->colrs;
		int x, y, w_start, h_start, bg_cur_h;
		int w_bg = hw->html.bg_pic_data->width;
		int h_bg = hw->html.bg_pic_data->height;
		int extra = 0;

		/* Fixup transparent background image */
		if (hw->html.bg_pic_data->transparent) {
			bg.pixel = hw->core.background_pixel;
			XQueryColor(hw->html.dsp, hw->core.colormap, &bg);
			bg_cmap[hw->html.bg_pic_data->bg_index].red = bg.red;
			bg_cmap[hw->html.bg_pic_data->bg_index].green =bg.green;
			bg_cmap[hw->html.bg_pic_data->bg_index].blue = bg.blue;
			bg_cmap[hw->html.bg_pic_data->bg_index].pixel =bg.pixel;
		} else if (hw->html.bg_pic_data->alpha) {
			bg_data = hw->html.bg_pic_data->alpha_image_data;
		}
		if (pic->has_border && (!pic->internal || (pic->internal == 2)))
			extra = eptr->bwidth;
		x = eptr->x - hw->html.scroll_x + extra;
		y = eptr->y - hw->html.scroll_y + extra;
		w_start = (x + hw->html.scroll_x) % w_bg;
		h_start = (y + hw->html.scroll_y) % h_bg;

		bg_cur_h = h_start;
		for (i = 0; i < h; i++) {
			p3 = bg_data + (bg_cur_h * w_bg) + w_start;
			bg_line_end = p3 + (w_bg - w_start);

			for (j = 0; j < w; j++) {
				/* Get unsigned shorts in signed ints */ 
				R = cmap[*p1].red;
				G = cmap[*p1].green;
				B = cmap[*p1++].blue;
				RB = bg_cmap[*p3].red;
				GB = bg_cmap[*p3].green;
				BB = bg_cmap[*p3++].blue;
				A = *alpha++;

				if (A == 0) {
				    /* Fully transparent, return background */
				    *p2++ = RB >> 8;
				    *p2++ = GB >> 8;
				    *p2++ = BB >> 8;
				} else if (A == 255) {
				    /* Fully opaque, return image pixel */ 
				    *p2++ = R >> 8;
				    *p2++ = G >> 8;
				    *p2++ = B >> 8;
				} else {
				    *p2++ = ((((R - RB) * A) >> 8) + RB) >> 8;
				    *p2++ = ((((G - GB) * A) >> 8) + GB) >> 8;
				    *p2++ = ((((B - BB) * A) >> 8) + BB) >> 8;
				}

				if (p3 == bg_line_end) {
				    p3 = bg_data + (bg_cur_h * w_bg);
				    bg_line_end = p3 + w_bg;
				}
			}
			if (++bg_cur_h == h_bg)
				bg_cur_h = 0;
		}
		/* Keep in case the element moves due to alignment */
		pic->saved_x = eptr->x;
		pic->saved_y = eptr->y;
	} else {
		/* Get background color */
		bg.pixel = hw->core.background_pixel;
		XQueryColor(hw->html.dsp, hw->core.colormap, &bg);
		RB = bg.red;
		GB = bg.green;
		BB = bg.blue;

		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				/* Get unsigned shorts in signed ints */ 
				R = cmap[*p1].red;
				G = cmap[*p1].green;
				B = cmap[*p1++].blue;
				A = *alpha++;

				if (A == 0) {
				    /* Fully transparent, return background */
				    *p2++ = RB >> 8;
				    *p2++ = GB >> 8;
				    *p2++ = BB >> 8;
				} else if (A == 255) {
				    /* Fully opaque, return image pixel */ 
				    *p2++ = R >> 8;
				    *p2++ = G >> 8;
				    *p2++ = B >> 8;
				} else {
				    *p2++ = ((((R - RB) * A) >> 8) + RB) >> 8;
				    *p2++ = ((((G - GB) * A) >> 8) + GB) >> 8;
				    *p2++ = ((((B - BB) * A) >> 8) + BB) >> 8;
				}
			}
		}
	}

	/* Keep for printing and element realignment */
	if (pic->ori_colrs)
		free(pic->ori_colrs);
	pic->ori_colrs = malloc(pic->num_colors * sizeof(XColor));
	memcpy(pic->ori_colrs, pic->colrs, pic->num_colors * sizeof(XColor));
	pic->ori_num_colors = pic->num_colors;

	/* Go reduce it to 8 bit */
	XtCallCallbackList((Widget)hw, hw->html.quantize_callback,
			   (XtPointer)pic);
#ifndef DISABLE_TRACE
	if (htmlwTrace)
	       fprintf(stderr, "Quantized alpha image on background\n");
#endif
	return(pic->rgb);
}

/* Currently never called with clip non-zero */
Pixmap InfoToImage(HTMLWidget hw, ImageInfo *img_info, int clip, ElemInfo *eptr)
{
	int i, size;
	Pixmap Img = (Pixmap)NULL;
	XImage *tmpimage = NULL;
	XColor tmpcolr;
	XColor *cmap = img_info->colrs;
	int *Mapping;
	unsigned char *alpha_data = NULL;
	unsigned char *ptr, *ptr2, *tmpdata;
	Boolean need_to_dither_BW;
	unsigned long black_pixel, white_pixel;
	int depth = DefaultDepthOfScreen(XtScreen(hw));
	int need_dither_color = 0;
	Display *dsp = hw->html.dsp;
	static int init = 0;
	static int MaxPixmapWidth, MaxPixmapHeight;

	if (!clip && img_info->alpha)
		alpha_data = ProcessImageAlpha(hw, img_info, eptr);

	if (!clip && (depth == 1)) {
		need_to_dither_BW = True;
		black_pixel = BlackPixel(dsp, DefaultScreen(dsp));
		white_pixel = WhitePixel(dsp, DefaultScreen(dsp));
	} else {
		need_to_dither_BW = False;
	}
	Mapping = (int *)malloc(img_info->num_colors * sizeof(int));

	if (!clip) {
		for (i = 0; i < img_info->num_colors; i++) {
			if ((Vclass == TrueColor) || (Vclass == DirectColor)) {
				Mapping[i] = i;
			} else if (need_to_dither_BW) {
       				Mapping[i] = ((cmap[i].red >> 5) * 11 +
					      (cmap[i].green >> 5) * 16 +
					      (cmap[i].blue >> 5 ) * 5) /
					      (65504 / 64);
			} else {
				tmpcolr.red = cmap[i].red;
				tmpcolr.green = cmap[i].green;
				tmpcolr.blue = cmap[i].blue;
				/* Don't need to set flags */
				if (!FindColor(hw, hw->core.colormap, &tmpcolr))
					/* Didn't get actual color */
					need_dither_color = 1;
				Mapping[i] = tmpcolr.pixel;
			}
		}
	}
	/*
	 * Special case:  For 2 color non-black&white images, instead
	 * of 2 dither patterns, we will always drop them to be
	 * black on white.
	 */
	if (need_to_dither_BW && (img_info->num_colors == 2)) {
		if (Mapping[0] < Mapping[1]) {
			Mapping[0] = 0;
			Mapping[1] = 64;
		} else {
			Mapping[0] = 64;
			Mapping[1] = 0;
		}
	}

	size = img_info->width * img_info->height;
	if (!(tmpdata = (unsigned char *)malloc(size))) {
		LimDim = 2;
		application_user_feedback("Image too large:  Out of memory");
		if (alpha_data)
			free(alpha_data);
		goto abort;
	}
	if (clip) {
		ptr = img_info->clip_data;
	} else {
		if (img_info->alpha) {
			ptr = alpha_data;
		} else {
			ptr = img_info->image_data;
		}
	}
	ptr2 = tmpdata;
	if (need_to_dither_BW) {
		int cx, cy, delta, not_right_col, not_last_row;

		for (; ptr2 < (tmpdata + size - 1); ptr2++, ptr++)
			*ptr2 = Mapping[(int)*ptr];

		ptr2 = tmpdata;
		for (cy = 0; cy < img_info->height; cy++) {
			for (cx = 0; cx < img_info->width; cx++) {
			    /*
			     * Assume high numbers are really negative
			     */
			    if (*ptr2 > 128) {
				*ptr2 = 0;
			    } else if (*ptr2 > 64) {
				*ptr2 = 64;
			    }
			    /*
			     * Traditional Floyd-Steinberg
			     */
			    if (*ptr2 < 32) {
				delta = *ptr2;
				*ptr2 = black_pixel;
			    } else {
				delta = *ptr2 - 64;
				*ptr2 = white_pixel;
			    }
			    if (not_right_col = (cx < (img_info->width - 1)))
				*(ptr2 + 1) += delta * 7 >> 4;
			    if (not_last_row = (cy < (img_info->height - 1)))
				*(ptr2 + img_info->width) += delta * 5 >> 4;
			    if (not_right_col && not_last_row)
				*(ptr2 + img_info->width + 1) += delta >> 4;
			    if (cx && not_last_row)
				*(ptr2 + img_info->width - 1) += delta * 3 >> 4;
			    ptr2++;
			}
		}
	} else if (need_dither_color && BSCnum) {
#ifndef DISABLE_TRACE
		if (htmlwTrace)
			fprintf(stderr, "Dithering color or grayscale image\n");
#endif
		FS_Dither(img_info, tmpdata, BSColors, BSCnum, ptr);
		cmap = BSColors;
	} else {
		for (i = 0; i < size; i++) {
			if (clip) { 
				*ptr2++ = *ptr++;
			} else {
				*ptr2++ = (unsigned char)Mapping[(int)*ptr++];
			}
		}
	}
	tmpimage = MakeImage(dsp, tmpdata, depth, img_info, cmap, clip);
        free(tmpdata);
	if (alpha_data) {
		if (img_info->is_bg_image) {
			img_info->alpha_image_data = alpha_data;
		} else {
			free(alpha_data);
		}
	}

	/* Check if over dimension limits */
	if ((img_info->width > LimDimX) || (img_info->height > LimDimY)) {
		LimDim = -1;
	} else {
		LimDim = 0;
	}
	/* Instead of calling the get_pref_int() function all the time,
	 * store values in static variables. */
	if (!init) {
		MaxPixmapHeight = get_pref_int(eMAXPIXMAPHEIGHT);
		MaxPixmapWidth  = get_pref_int(eMAXPIXMAPWIDTH);
		init = 1;
	}
	if ((MaxPixmapWidth && (img_info->width > MaxPixmapWidth)) ||
	    (MaxPixmapHeight && (img_info->height > MaxPixmapHeight))) {
		Img = (Pixmap)NULL;
		LimDim = 2;	/* So error message prints */
		application_user_feedback(
			       "Image too large:  Exceeds preference settings");
	} else {
		Img = XCreatePixmap(dsp, XtWindow(hw->html.view),
				    img_info->width, img_info->height, depth);
		if (LimDim == -1)
			/* Wait for completion, so LimDim can get set by trap */
			XSync(dsp, False);
		if (LimDim == 1)
			application_user_feedback(
				 "Image too large:  Pixmap allocation failure");
	}
 abort:
	if (LimDim > 0)
		fprintf(stderr, "Image too large (%d x %d)\n",
			img_info->width, img_info->height);
	if (!tmpimage || !Img || (LimDim == 1)) {
		ImageInfo *noim = NoImageData(hw);

		if (tmpimage)
			XDestroyImage(tmpimage);
		if (Img)
			/* Causes error if LimDim = 1 which we trap */
			XFreePixmap(dsp, Img);
		if (img_info->fetched && !img_info->cached) {
			free(img_info->image_data);
			if (img_info->clip_data)
				free(img_info->clip_data);
		}
		if (img_info->fetched && img_info->ori_colrs)
			free(img_info->ori_colrs);
		img_info->width = noim->width;
		img_info->height = noim->height;
		img_info->internal = 2;
		img_info->fetched = 0;
		img_info->transparent = 0;
		Img = noim->image;
	} else {
		XPutImage(dsp, Img, hw->html.drawGC, tmpimage, 0, 0, 0, 0,
			  img_info->width, img_info->height);
		XDestroyImage(tmpimage);
	}
        free((char *)Mapping);
	return(Img);
}


void HtmlGetImage(HTMLWidget hw, ImageInfo *picd, PhotoComposeContext *pcc,
		  int force_load)
{
	ImageInfo icbs;
	int width, height;
	static int init = 0;
	static int MaxPixmapWidth, MaxPixmapHeight;

	if (!init) {
		MaxPixmapHeight = get_pref_int(eMAXPIXMAPHEIGHT);
		MaxPixmapWidth  = get_pref_int(eMAXPIXMAPWIDTH);
		init = 1;
	}
	if (!strncmp(picd->src, "internal-", 9)) {   /* Internal images */
		Pixel fg_pixel = hw->manager.foreground;
		Pixel bg_pixel = hw->core.background_pixel;
		Display *dsp = hw->html.dsp;
		int depth = DefaultDepthOfScreen(XtScreen(hw));

		if (!strcmp(picd->src, "internal-cookie-image")) {
			picd->image = internalCookie;
			picd->width = 33;
			picd->height = 32;
			picd->req_width = -1;
			picd->req_height = -1;
			picd->image_data = NULL;
			picd->internal = 1;
			picd->delayed = 0;
			picd->num_colors = 8;
		} else if (!strcmp(picd->src, "internal-gopher-image")) {
			IMGINFO_FROM_BITMAP(gopher_image)
		} else if (!strcmp(picd->src, "internal-gopher-movie")) {
			IMGINFO_FROM_BITMAP(gopher_movie)
		} else if (!strcmp(picd->src, "internal-gopher-menu")) {
			IMGINFO_FROM_BITMAP(gopher_menu)
		} else if (!strcmp(picd->src, "internal-gopher-text")) {
			IMGINFO_FROM_BITMAP(gopher_text)
		} else if (!strcmp(picd->src, "internal-gopher-sound")) {
			IMGINFO_FROM_BITMAP(gopher_sound)
		} else if (!strcmp(picd->src, "internal-gopher-index")) {
			IMGINFO_FROM_BITMAP(gopher_index)
		} else if (!strcmp(picd->src, "internal-gopher-telnet")) {
			IMGINFO_FROM_BITMAP(gopher_telnet)
		} else if (!strcmp(picd->src, "internal-gopher-binary")) {
			IMGINFO_FROM_BITMAP(gopher_binary)
		} else {
			/* "internal-gopher-unknown" or no match */
			IMGINFO_FROM_BITMAP(gopher_unknown)
		}
		return;
	}

	icbs = *picd;
	/* Not internal, maybe delayed or in cache? */
	if ((hw->html.delay_image_loads || (currently_delaying_images == 1) ||
	     icbs.urldelayed) && !force_load) {
		/* Only looks in cache */
		XtCallCallbackList((Widget)hw, hw->html.image_callback,
				   (XtPointer) &icbs);
		/* The image is not cached or not blanked out. */
		if (!icbs.fetched && (icbs.internal != 3)) {
			ImageInfo *dlim;

			/* Update picd from the correct delayed image */
			if (!pcc->in_anchor) {
				dlim = DelayedImageData(hw);
			} else {
				dlim = AnchoredImageData(hw);
			}
			*picd = icbs;
			if (currently_delaying_images) {
				picd->urldelayed = 1;
			} else {
				picd->urldelayed = 0;
			}
			picd->height = dlim->height;
			picd->width = dlim->width;
			picd->internal = 2;
			picd->delayed = 1;
			picd->fetched = 0;
			picd->image_data = dlim->image_data;
			picd->image = dlim->image; 
                	return;
		}
	} else {	/* Load image from the cache or net */
		XtCallCallbackList((Widget)hw, hw->html.image_callback,
                                   (XtPointer) &icbs);
		/* Can't find image.  Put NoImage in picd unless blanked out */
		if (!icbs.fetched && (icbs.internal != 3)) {
			ImageInfo *img;

			*picd = icbs;
			if ((icbs.req_height == 1) || (icbs.req_width == 1)) {
				/* Use blank for tiny requested images */
				img  = BlankImageData(hw);
				picd->internal = 3;
			} else {
				img = NoImageData(hw);
				picd->internal = 2;
			}
			picd->height = img->height;
			picd->width = img->width;
			picd->delayed = 0;
			picd->fetched = 0;
			picd->image_data = img->image_data;
			picd->image = img->image;
	              	return;
		}
	}
	/* A hack because I put blank image in img.c instead of above, GEC */
	if (icbs.internal) {
		*picd = icbs;
		return;
	}
	/* A sanity check */
	if (!(icbs.width * icbs.height) || !icbs.image_data) {
		ImageInfo *noim = NoImageData(hw);

		picd->height = noim->height;
		picd->width = noim->width;
		picd->internal = 2;
		picd->delayed = 0;
		picd->fetched = 0;
		picd->image_data = noim->image_data;
		picd->image = noim->image; 
                return;
	}

        /* Here we have an image from the cache or the net */
        /* data is in icbs */

        /* Fields set to const. are untouched 				*/
        /*   src alt_text align req_height req_width border hspace	*/
        /*   vspace usemap map ismap fptr				*/
        /*   internal = 0						*/
        /*   delayed = 0 						*/
        /*   cw_only	 						*/
        /*								*/
        /* After the callback here is the modified field 		*/
        /*   height = original height of image				*/
        /*   width = original width of image				*/
        /*   fetched = 1	if we are here				*/
        /*   cached = 1							*/
        /*   colrs (no more than 256 colors)				*/
        /*   bg_index							*/
        /*   image_data							*/
        /*   num_colors							*/
        /*   clip_data							*/
        /*   transparent						*/

	/* Rescale image here */
	if (icbs.anim_info) {
		width = icbs.awidth;
		height = icbs.aheight;
	} else {
		width = icbs.width;
		height = icbs.height;
	}
	if ((icbs.req_width > 0) && (icbs.req_height < 1) &&
	    (icbs.req_width != width)) {
		int change = (icbs.req_width * 100) / width;

		icbs.req_height = (height * change) / 100;
	}
	if (icbs.req_width < 1)
		icbs.req_width = width;
	if (icbs.req_height < 1)
		icbs.req_height = height;
	if (icbs.percent_width &&
	    (((icbs.percent_width * pcc->cur_line_width) / 100) > 0)) {
		icbs.req_width =
			       (icbs.percent_width * pcc->cur_line_width) / 100;
	}
	/* Rescale if too large for display card */
	if (MaxPixmapWidth && (icbs.req_width > MaxPixmapWidth)) {
		int change = (MaxPixmapWidth * 100) / icbs.req_width;

		icbs.req_width = MaxPixmapWidth;
		icbs.req_height = (icbs.req_height * change) / 100;
	}
	if (MaxPixmapHeight && (icbs.req_height > MaxPixmapHeight)) {
		int change = (MaxPixmapHeight * 100) / icbs.req_height;

		icbs.req_height = MaxPixmapHeight;
		icbs.req_width = (icbs.req_width * change) / 100;
	}
	if ((icbs.req_width != width) || (icbs.req_height != height)) {
		/* Rescale */
		XColor colrs[256];
		int wchange = (icbs.req_width * 100) / width;
		int hchange = (icbs.req_height * 100) / height;
		int new_width = (icbs.width * wchange) / 100;
		int new_height = (icbs.height * hchange) / 100;

		if (!icbs.cw_only) {
			int i;

			RescalePic(&icbs, new_width ? new_width : 1,
				   new_height ? new_height : 1);
			for (i = 0; i < icbs.num_colors; i++)
				colrs[i] = icbs.colrs[i];
			ProcessImageData((Widget)hw, &icbs, colrs);
			icbs.cached = 0;
		} else {
			icbs.width = new_width ? new_width : 1;
			icbs.height = new_height ? new_height : 1;
		}
		if (icbs.x)
			icbs.x = (icbs.x * wchange) / 100;
		if (icbs.y)
			icbs.y = (icbs.y * hchange) / 100;
		if (icbs.anim_info) {
			ImageInfo *tmp = icbs.next;

			while (tmp && !icbs.cw_only) {
				new_width = (tmp->width * wchange) / 100;
				new_height = (tmp->height * hchange) / 100;
				RescalePic(tmp, new_width ? new_width : 1,
					   new_height ? new_height : 1);
				memcpy(colrs, tmp->colrs,
				       tmp->num_colors * sizeof(XColor));
				ProcessImageData((Widget)hw, tmp, colrs);
				/* Adjust image offset, if any */
				if (tmp->x)
					tmp->x = (tmp->x * wchange) / 100;
				if (tmp->y)
					tmp->y = (tmp->y * hchange) / 100;
				tmp = tmp->next;
			}
			/* Adjust overall animation size */
			icbs.awidth = (icbs.awidth * wchange) / 100;
			icbs.aheight = (icbs.aheight * hchange) / 100;
			icbs.awidth = icbs.awidth ? icbs.awidth : 1;
			icbs.aheight = icbs.aheight ? icbs.aheight : 1;
	 	}
	}
	*picd = icbs;

	/* Fixup animation struct */
	if (picd->anim_info)
		picd->anim_info->start = picd;
	return;
}

/* Place an image and create an element record for it. */
void ImagePlace(HTMLWidget hw, MarkInfo *mptr, PhotoComposeContext *pcc)
{
	char *srcPtr, *altPtr, *alignPtr, *valignPtr, *heightPtr, *widthPtr;
	char *borderPtr, *hspacePtr, *vspacePtr, *usemapPtr, *ismapPtr, *tptr;
	ElemInfo *eptr;
	ImageInfo *picd;
	ImageInfo lpicd;
	XCharStruct all;
	XFontStruct *alt_font;
	int orig_cur_baseline, orig_cur_line_height, orig_x;
	int baseline, dir, ascent, descent;
	int extra = 0;
	int absmiddle = 0;
	int absbottom = 0;
	int texttop = 0;
	int width = 0;
	int height = 0;
	FloatRec *tmp_float;

	/* Do progressive display of elements up to this point */
	if (!pcc->cw_only && progressiveDisplayEnabled &&
            hw->html.last_formatted_elem && ((hw->html.last_formatted_elem->y <
	    (hw->html.scroll_y + hw->html.view_height)) ||
	    (pcc->last_progressive_ele && pcc->last_progressive_ele->next &&
	     (pcc->last_progressive_ele->next->y <
	      (hw->html.scroll_y + hw->html.view_height))))) {
		eptr = hw->html.last_formatted_elem;
		if (pcc->last_progressive_ele) {
			if (pcc->last_progressive_ele->next)
				eptr = pcc->last_progressive_ele->next;
			ProgressiveDisplay(hw, eptr, pcc);
		} else {
			ProgressiveDisplay(hw, hw->html.formatted_elements,
					   pcc);
		}
		pcc->last_progressive_ele = hw->html.last_formatted_elem;
	}

	if (tptr = ParseMarkTag(mptr->start, MT_INPUT, "STYLE")) {
		if (!strcmp(tptr, "display:none")) {
			free(tptr);
			return;
		}
	}
	if (!(srcPtr = ParseMarkTag(mptr->start, MT_IMAGE, "SRC")))
		return;
	if (!*srcPtr) {
		free(srcPtr);
		return;
	}
	altPtr = ParseMarkTag(mptr->start, MT_IMAGE, "ALT");
	alignPtr = ParseMarkTag(mptr->start, MT_IMAGE, "ALIGN");
	valignPtr = ParseMarkTag(mptr->start, MT_IMAGE, "VALIGN");
	heightPtr = ParseMarkTag(mptr->start, MT_IMAGE, "HEIGHT");
	widthPtr = ParseMarkTag(mptr->start, MT_IMAGE, "WIDTH");
	borderPtr = ParseMarkTag(mptr->start, MT_IMAGE, "BORDER");
	hspacePtr = ParseMarkTag(mptr->start, MT_IMAGE, "HSPACE");
	vspacePtr = ParseMarkTag(mptr->start, MT_IMAGE, "VSPACE");
	usemapPtr = ParseMarkTag(mptr->start, MT_IMAGE, "USEMAP");
	ismapPtr = ParseMarkTag(mptr->start, MT_IMAGE, "ISMAP");

	memset(&lpicd, 0, sizeof(ImageInfo));
	lpicd.src = srcPtr;
	lpicd.alt_text = altPtr;
	lpicd.align = ALIGN_NONE;
	lpicd.valign = VALIGN_BOTTOM;
	lpicd.req_height = -1;	/* No req_height */
	lpicd.req_width = -1;	/* No req_width */
	lpicd.hspace = DEF_IMAGE_HSPACE;
	lpicd.vspace = DEF_IMAGE_VSPACE;
	lpicd.usemap = usemapPtr;
	lpicd.delayed = hw->html.delay_image_loads;
	lpicd.cw_only = pcc->cw_only;
	if (mptr->s_picd)
		lpicd.urldelayed = mptr->s_picd->urldelayed;

	/** memset sets them
	lpicd.border = 0;
	lpicd.height = 0;
	lpicd.width = 0;
	lpicd.percent_width = 0;
	lpicd.has_border = 0;
	lpicd.map = NULL;
	lpicd.ismap = 0;
	lpicd.fptr = NULL; 
	lpicd.internal = 0;
	lpicd.fetched = 0;
	lpicd.cached = 0;
	lpicd.num_colors = 0;
	lpicd.bg_index = 0;
	lpicd.image_data = NULL;
	lpicd.clip_data = NULL;
	lpicd.transparent = 0; 
	lpicd.image = None;
	lpicd.clip = None;
	lpicd.is_bg_image = 0;
	lpicd.aligned = 0;
	lpicd.text = NULL;
	lpicd.anim_info = NULL;
	lpicd.anim_image = None;
	lpicd.has_anim_image = 0;
	lpicd.disposal = 0;
	lpicd.alpha = NULL;
	lpicd.ori_colrs = NULL;
	lpicd.ori_num_colors = 0;
	**/

        if (valignPtr) {
	        if (caseless_equal(valignPtr, "TOP")) {
 			lpicd.valign = VALIGN_TOP;
		} else if (caseless_equal(valignPtr, "MIDDLE")) {
  			lpicd.valign = VALIGN_MIDDLE;
		} else if (caseless_equal(valignPtr, "BOTTOM")) {
			lpicd.valign = VALIGN_BOTTOM;
		}
	        free(valignPtr);
	}
        if (alignPtr) {
		if (caseless_equal(alignPtr, "LEFT")) {
			lpicd.align = HALIGN_LEFT;
		} else if (caseless_equal(alignPtr, "RIGHT")) {
			lpicd.align = HALIGN_RIGHT;
	        } else if (caseless_equal(alignPtr, "TOP")) {
 			lpicd.valign = VALIGN_TOP;
	        } else if (caseless_equal(alignPtr, "TEXTTOP")) {
 			lpicd.valign = VALIGN_TOP;
			texttop = 1;
		} else if (caseless_equal(alignPtr, "MIDDLE")) {
  			lpicd.valign = VALIGN_MIDDLE;
		} else if (caseless_equal(alignPtr, "ABSMIDDLE")) {
  			lpicd.valign = VALIGN_MIDDLE;
			absmiddle = 1;
		} else if (caseless_equal(alignPtr, "BOTTOM") ||
			   caseless_equal(alignPtr, "BASELINE")) {
			lpicd.valign = VALIGN_BOTTOM;
		} else if (caseless_equal(alignPtr, "ABSBOTTOM")) {
			lpicd.valign = VALIGN_BOTTOM;
			absbottom = 1;
		}
	        free(alignPtr);
	}
	if (heightPtr) {
		lpicd.req_height = atoi(heightPtr);
		if (lpicd.req_height < 1) 	/* Too small ... */
			lpicd.req_height = -1;
		free(heightPtr);
	}
	if (widthPtr) {
		lpicd.req_width = atoi(widthPtr);
		if (strchr(widthPtr, '%')) {
			lpicd.percent_width = lpicd.req_width;
			lpicd.req_width = -1;
		} else if (lpicd.req_width < 1) {
			lpicd.req_width = -1;
		}
		free(widthPtr);
	}

	/* Use default border if in anchor */
	if (pcc->in_anchor) {
		lpicd.border = IMAGE_DEFAULT_BORDER; 
		lpicd.has_border = 1;
	}
	/* However, specified border overrides the default action */
	if (borderPtr) {
		lpicd.border = atoi(borderPtr);
		if (lpicd.border < 0) {
			if (pcc->in_anchor) {
				lpicd.border = IMAGE_DEFAULT_BORDER;
			} else {
				lpicd.border = 0;
			}
		} else if (lpicd.border > 0) {
			lpicd.has_border = 1;
		} else {
			lpicd.has_border = 0;
		}
		free(borderPtr);
	}
	if (hspacePtr){
		lpicd.hspace = atoi(hspacePtr);
		if (lpicd.hspace < 0)
			lpicd.hspace = 0;
		free(hspacePtr);
	}
	if (vspacePtr) {
		lpicd.vspace = atoi(vspacePtr);
		if (lpicd.vspace < 0)
			lpicd.vspace = 0;
		free(vspacePtr);
	}
	if (ismapPtr) {
		if (*ismapPtr == '2') {
			/* Reset Button */
			lpicd.ismap = 2;
		} else {
			lpicd.ismap = 1;
		}
		free(ismapPtr);
	}

	/* Now initialize the image part */
	picd = GetImageRec();
	*picd = lpicd;		/* For work */

	/* Skip load if possible so progressive loads work better */
	if (pcc->cw_only && !hw->html.delay_image_loads &&
	    !currently_delaying_images && !lpicd.urldelayed) {
		if (lpicd.req_width > 0)
			picd->width = lpicd.req_width;
		if (lpicd.req_height > 0)
			picd->height = lpicd.req_height;
		if (lpicd.percent_width)
			picd->width = (lpicd.percent_width *
				       pcc->cur_line_width) / 100;
		if (!picd->width || !picd->height)
			HtmlGetImage(hw, picd, pcc, False);
	} else {
		/* Get an image in picd */
		HtmlGetImage(hw, picd, pcc, False);  /* Don't force load */
	}

	/* Now we have an image.  It is:
	 *	- an internal-gopher	(internal=1, fetched=0, delayed=0)
	 *	- a delayed image	(internal=2, fetched=0, delayed=1)
	 *	- a no (bad) image	(internal=2, fetched=0, delayed=0)
	 *	- a blank image		(internal=3, fetched=0, delayed=0)
	 *	- the requested image	(internal=0, fetched=1, delayed=0)
	 *	- didn't do it		(internal=0, fetched=0, delayed=X)
	 */

	/* Save the work */
	if (!pcc->cw_only)
		mptr->s_picd = picd;

	if (!pcc->in_anchor && picd->ismap && pcc->cur_form && picd->fetched)
		/* SUPER SPECIAL CASE!
 		 * If you have an ISMAP image inside a form, and that form
		 * doesn't already have an HREF by being inside an anchor,
		 * (Being a DelayedHRef is considered no href) clicking in
		 * that image will submit the form, adding the x,y coordinates
		 * of the click as part of the list of name/value pairs.
		 */
		picd->fptr = pcc->cur_form;

	if (picd->has_border)
		extra = 2 * picd->border;

	/* Use animation size for formatting */
	if (picd->anim_info) {
		height = picd->aheight;
		width = picd->awidth;
	} else {
		height = picd->height;
		width = picd->width;
	}

	/* Do ALT text if delayed or bad image */
	if (picd->internal == 2) {
		int twidth;

		PushFont(hw, pcc);
		pcc->cur_font_size = 3;
		pcc->cur_font_type = FONT;
		SetFontSize(hw, pcc, 0);
		XTextExtents(pcc->cur_font, " ", 1, &dir, &ascent,
			     &descent, &all);
		if (!picd->alt_text) {
			char *sptr;
			char *tptr = ParseMarkTag(mptr->start, MT_IMAGE, "SRC");

			if (sptr = strrchr(tptr, '/')) {
				picd->alt_text = strdup(sptr + 1);
				free(tptr);
			} else {
				picd->alt_text = tptr;
			}
		}
		twidth = XTextWidth(pcc->cur_font, picd->alt_text,
				    strlen(picd->alt_text)) + all.width + 2;
		/* Don't do text for bad images in tables if not enough room */
		if (pcc->in_table && !picd->delayed && !pcc->cw_only &&
		    ((width + twidth) > pcc->cur_line_width)) {
			twidth = 0;
			free(picd->alt_text);
			picd->alt_text = NULL;
		}
		width += twidth;
		alt_font = pcc->cur_font;
		pcc->cur_font = PopFont(hw, pcc);
		SetFontSize(hw, pcc, 0);
		/* Always have border */
		if (!extra) {
			picd->border = IMAGE_DEFAULT_BORDER;
			extra = 2 * IMAGE_DEFAULT_BORDER;
			picd->has_border = 1;
		}
	} else {
		alt_font = pcc->cur_font;
	}

	height += extra + picd->vspace;
	width += extra;

	if ((picd->align == HALIGN_LEFT) &&
	    (height < (pcc->cur_font->ascent + pcc->cur_font->descent)))
		height = pcc->cur_font->ascent + pcc->cur_font->descent;

	height += picd->vspace;

	/* How is it aligned? */
	if (picd->valign == VALIGN_TOP) {
		if (!texttop) {
			if (!pcc->cur_line_height) {
				baseline = pcc->cur_font->ascent + picd->vspace;
			} else {
				baseline = pcc->cur_baseline;
			}
		} else {
			baseline = -3;
		}
	} else if (picd->valign == VALIGN_MIDDLE) {
		if (!absmiddle) {
			baseline = height / 2;
		} else {
			baseline = -1;
		}
	} else if (absbottom) {
		baseline = -2;
	} else {
		baseline = height;
	}

	/*
	 * If we are starting an image in formatted text, and it
	 * needs a preceeding space, add that space now.
	 */
	XTextExtents(pcc->cur_font, " ", 1, &dir, &ascent, &descent, &all);
	extra = all.width;
	if (!pcc->preformat && pcc->have_space_after && !pcc->is_bol)
		pcc->x += extra;

	/* Left aligned images go at beginning of line */
	if ((picd->align == HALIGN_LEFT) && !pcc->is_bol)
		ConditionalLineFeed(hw, 1, pcc);

	width += picd->hspace;

	/* Now look if the image is too wide, if so insert a linefeed. */
	if (!pcc->preformat && !pcc->cw_only) {
		int space = 0;

		/* Ignore space in front of right aligned image */
		if (pcc->float_right && (pcc->float_right->type == 1))
			space = pcc->float_right->image_extra;

		if ((!pcc->is_bol || pcc->float_right) &&
		    ((pcc->x + width + picd->hspace - space) > (pcc->eoffsetx +
		      pcc->left_margin + pcc->cur_line_width))) {
			ConditionalLineFeed(hw, 1, pcc);
			/* If still no room, then have a previous right
			 * aligned image.  Force a linefeed past it. */
			if (pcc->float_right &&
			    ((pcc->x + width + picd->hspace - space) >
			     (pcc->eoffsetx + pcc->left_margin +
			      pcc->cur_line_width))) {
				if (pcc->cur_line_height <
				    (pcc->float_right->y - pcc->y + 1)) {
					pcc->cur_line_height =
					       pcc->float_right->y - pcc->y + 1;
					LinefeedPlace(hw, pcc);
				}
			}
		}
	}
	pcc->x += picd->hspace;

	if (picd->align == HALIGN_RIGHT) {
		orig_x = pcc->x - picd->hspace;
		if (((pcc->eoffsetx + pcc->left_margin + pcc->cur_line_width -
		     (width + extra)) >= pcc->x) && ((width + picd->hspace) <=
		    (pcc->eoffsetx + pcc->left_margin + pcc->cur_line_width))) {
			pcc->x = pcc->eoffsetx + pcc->left_margin +
				 pcc->cur_line_width - width - picd->hspace;
		} else {
			ConditionalLineFeed(hw, 1, pcc);
			orig_x = pcc->eoffsetx + pcc->left_margin;
			if ((width + picd->hspace + extra) < (pcc->eoffsetx +
			    pcc->left_margin + pcc->cur_line_width)) {
				pcc->x = pcc->eoffsetx + pcc->left_margin +
					 pcc->cur_line_width - width -
					 picd->hspace;
			} else {
				/* No room to do any right alignment */
				pcc->x += picd->hspace;
				picd->align = ALIGN_NONE;
			}
		}
	}

	if (pcc->cw_only) {
	    /* If percentage width, then could be made as short as 1 */
	    if (picd->percent_width) {
	        if ((pcc->eoffsetx + pcc->left_margin + 1 +
		     (2 * picd->hspace)) > pcc->computed_min_x)
		    pcc->computed_min_x = pcc->eoffsetx + pcc->left_margin +
					  1 + (2 * picd->hspace);
	    } else {
	        if ((pcc->eoffsetx + pcc->left_margin + width + picd->hspace) >
		    pcc->computed_min_x)
		    pcc->computed_min_x = pcc->eoffsetx + pcc->left_margin +
					  picd->hspace + width;
		if (pcc->nobr && (pcc->computed_min_x <
				  (pcc->nobr_x + picd->hspace + width)))
		    pcc->computed_min_x = pcc->nobr_x + picd->hspace + width;
	    }
	    if ((pcc->x + width) > pcc->computed_max_x)
                pcc->computed_max_x = pcc->x + width;
	}

	/* By default, a short image not in a table has space above it */
	if (!pcc->in_table && !pcc->cur_line_height &&
	    (height < pcc->cur_font->ascent) &&
	    (lpicd.align == ALIGN_NONE) && (lpicd.valign == VALIGN_BOTTOM))
		pcc->cur_line_height = pcc->cur_baseline =
				       pcc->cur_font->ascent;

	orig_cur_baseline = pcc->cur_baseline;
	orig_cur_line_height = pcc->cur_line_height;
	if (!pcc->cw_only) {
		/* Do implicit label */
		if (picd->fptr && pcc->in_label && pcc->label_id) {
			mptr->anc_name = pcc->label_id;
			pcc->label_id = NULL;
			CreateAnchorElement(hw, mptr, pcc);
			pcc->in_label = 0;
		}
		eptr = CreateElement(hw, E_IMAGE, alt_font,
				     pcc->x, pcc->y + picd->vspace,
				     width, height, baseline, pcc);
		eptr->underline_number = 0;  /* Images can't be underlined! */
		eptr->anchor_tag_ptr = pcc->anchor_tag_ptr;
		eptr->pic_data = picd;
		if ((picd->align != HALIGN_RIGHT) &&
		    (picd->align != HALIGN_LEFT))
			AdjustBaseLine(eptr, pcc);
		eptr->bwidth = picd->border;
		if (picd->fptr)  /* Special ISMAP in a form */
			eptr->is_in_form = 1;
	} else {
		if (pcc->cur_line_height < height)
			pcc->cur_line_height = height;
		FreeImageInfo(picd, hw);
	}

	/* Do floating images if room on line */
	picd->hspace += extra;
	if (picd->align == HALIGN_LEFT) {
		if (((pcc->x + width + extra) <
		    (pcc->eoffsetx + pcc->left_margin + pcc->cur_line_width))) {
			pcc->left_margin += width + picd->hspace;
			pcc->cur_line_width -= width + picd->hspace;
			tmp_float = (FloatRec *)malloc(sizeof(FloatRec));
			tmp_float->next = pcc->float_left;
			pcc->float_left = tmp_float;
			tmp_float->type = 1;
			tmp_float->marg = width + picd->hspace;
			pcc->cur_baseline = orig_cur_baseline;
			pcc->cur_line_height = orig_cur_line_height;
			pcc->is_bol = 1;
			pcc->pf_lf_state = 1;
			pcc->x += width + extra;
			if (pcc->cw_only) {
				tmp_float->y = pcc->y + height;
			} else {
				tmp_float->y = eptr->y + height;
				/* Keep Adjustbaseline from messing it up */
				picd->aligned = 2;
				eptr = CreateElement(hw, E_CR, pcc->cur_font,
						     pcc->x, pcc->y, 0,
						     pcc->cur_line_height,
						     pcc->cur_baseline, pcc);
			}
			if (pcc->in_paragraph)
				pcc->in_paragraph++;
		} else {
			if (!pcc->cw_only)
				AdjustBaseLine(eptr, pcc);
			pcc->is_bol = 0;
			pcc->pf_lf_state = 0;
			pcc->x += width;
		}
	} else if (picd->align == HALIGN_RIGHT) {
		if (((orig_x + width + picd->hspace) <
		    (pcc->eoffsetx + pcc->left_margin + pcc->cur_line_width))) {
			pcc->right_margin += width + picd->hspace;
			pcc->cur_line_width -= width + picd->hspace;
			tmp_float = (FloatRec *)malloc(sizeof(FloatRec));
			tmp_float->next = pcc->float_right;
			pcc->float_right = tmp_float;
			tmp_float->type = 1;
			tmp_float->marg = width + picd->hspace;
			tmp_float->image_extra = picd->hspace;
			if (pcc->cw_only) {
				tmp_float->y = pcc->y + height;
			} else {
				tmp_float->y = eptr->y + height;
				/* Keep it from being messed with */
				picd->aligned = 1;
			}
			pcc->cur_baseline = orig_cur_baseline;
			pcc->cur_line_height = orig_cur_line_height;
			pcc->x = orig_x;
			if (pcc->in_paragraph)
				pcc->in_paragraph++;
		} else {
			if (!pcc->cw_only)
				AdjustBaseLine(eptr, pcc);
			pcc->is_bol = 0;
			pcc->pf_lf_state = 0;
			pcc->x += width;
		}
	} else {
		pcc->is_bol = 0;
		pcc->pf_lf_state = 0;
		pcc->x += width;
		if (pcc->cw_only && pcc->nobr)
			pcc->nobr_x += width;
	}
	pcc->have_space_after = 0;
}

/*
 * Redraw a formatted image element.
 * The image border color reflects whether it is an active anchor or not.
 * If it hasn't been already created, make the Pixmap now.
 * If iptr != NULL, then was called by animation timer routine.
 */
void ImageRefresh(HTMLWidget hw, ElemInfo *eptr, ImageInfo *iptr)
{
	ImageInfo *pic_data, *pic;
	ImageInfo *epic_data = eptr->pic_data;
	unsigned long valuemask;
	XGCValues values;
	int ax, ay, axe, aye, height, width;
	int x = eptr->x - hw->html.scroll_x;
	int y = eptr->y - hw->html.scroll_y;
	int extra = 0;
	int timer_refresh = 0;
	GC gc = hw->html.drawGC;
	Display *dsp = hw->html.dsp;
	Window win = XtWindow(hw->html.view);

	/* Don't refresh background image */
	if (epic_data->is_bg_image)
		return;

	if (iptr) {
		timer_refresh = 1;
		pic_data = iptr;
	} else {
		pic_data = epic_data;
		/* If done running, refresh last image of animation */
		if (epic_data->fetched && epic_data->anim_info &&
		    (epic_data->running == 2))
			pic_data = epic_data->last;
	}
	if (epic_data->has_border &&
	    (!epic_data->internal || (epic_data->internal == 2)))
		extra = eptr->bwidth;

	ax = x;
	ay = y;
	axe = ax + extra;
	aye = ay + extra;

	/* Compute offset of animation image, if any */
	if (epic_data->fetched && epic_data->anim_info) {
		x += pic_data->x;
		y += pic_data->y;
		height = epic_data->aheight;
		width = epic_data->awidth;
	} else {
		height = pic_data->height;
		width = pic_data->width;
	}
	if (hw->html.cur_fg != eptr->fg) {
		XSetForeground(dsp, gc, eptr->fg);
		hw->html.cur_fg = eptr->fg;
	}
	if (hw->html.cur_bg != eptr->bg) {
		XSetBackground(dsp, gc, eptr->bg);
		hw->html.cur_bg = eptr->bg;
	}
	/* Do ALT text if delayed or bad image */
	if ((epic_data->internal == 2) && epic_data->alt_text &&
	    *epic_data->alt_text) {
		if (eptr->font != hw->html.cur_font) {
			XSetFont(dsp, gc, eptr->font->fid);
			hw->html.cur_font = eptr->font;
		}
		XDrawString(dsp, win, gc, axe + epic_data->width + 1,
			    aye + (height / 2) +
				        (eptr->font->max_bounds.ascent / 2) - 2,
			    epic_data->alt_text, strlen(epic_data->alt_text));
		/* Compute new width of text inside border */
		width = eptr->width - epic_data->hspace - (2 * extra);
	}
	/* Draw border */
	if (extra && !timer_refresh) {
		XFillRectangle(dsp, win, gc, axe, ay, width, extra);
		XFillRectangle(dsp, win, gc, axe, aye + height, width, extra);
		XFillRectangle(dsp, win, gc, ax, ay, extra,
			       height + (2 * extra));
		XFillRectangle(dsp, win, gc, axe + width, ay, extra,
			       height + (2 * extra));
	}
	/* Don't refresh an animation while running unless it has
	 * a saved animation image.
	 */
	if (!timer_refresh && epic_data->fetched && epic_data->anim_info &&
	    epic_data->running) {
		if (epic_data->has_anim_image == 1) {
			/* Don't display if incomplete background image */
			if ((epic_data->running == 1) && epic_data->bg_image &&
			    !epic_data->bg_visible)
				return;
			XCopyArea(dsp, epic_data->anim_image, win, gc, 0, 0,
		  		  epic_data->awidth, epic_data->aheight,
		  		  axe, aye);
			return;
		} else if (epic_data->running == 1) {
			return;
		}
	}
	/* If alpha channel image moved over background image, redo Pixmap */ 
	if (pic_data->image && pic_data->alpha && hw->html.bgmap_SAVE &&
	    ((eptr->x != pic_data->saved_x) || (eptr->y != pic_data->saved_y))){
		int i;

		XFreePixmap(dsp, pic_data->image);
		pic_data->image = (Pixmap)NULL;
		/* Put original colors back */
		pic_data->num_colors = pic_data->ori_num_colors;
		for (i = 0; i < pic_data->ori_num_colors; i++)
			pic_data->colrs[i] = pic_data->ori_colrs[i];
	}
	if (!pic_data->image) {
		pic_data->image = InfoToImage(hw, pic_data, 0, eptr);
		if (pic_data->image) {
			if (pic_data->transparent && !pic_data->clip) {
				pic_data->clip = XCreatePixmapFromBitmapData(
						    dsp, win,
						    (char *)pic_data->clip_data,
						    pic_data->width,
						    pic_data->height,
						    1, 0, 1);
			} else if (!pic_data->transparent) {
				pic_data->clip = None;
			}
		}
		/* Force background overwrite the first time. */
		pic_data->background_pixel = (Pixmap) 0xFFFFFFFF;
	}
	if (pic_data->image) {
 		/*
		** If (image is transparent and within max clip size)
		** and (there is a background image) then use clip mask.
		** Also clip if transparent animation image.
 		*/
		if ((pic_data->transparent == 1) &&
		    (hw->html.bgmap_SAVE || epic_data->has_anim_image)) {
			values.clip_mask = pic_data->clip;
			if (epic_data->has_anim_image) {
			    values.clip_x_origin = pic_data->x;
			    values.clip_y_origin = pic_data->y;
			    /* Get background in animation image */
			    if (epic_data->has_anim_image != 1) {
				if (hw->html.bgmap_SAVE) {
				    if (!epic_data->bg_image)
					epic_data->bg_image =
					     XCreatePixmap(dsp, win,
							   epic_data->awidth,
							   epic_data->aheight,
							   DefaultDepthOfScreen(
							         XtScreen(hw)));
				    /* Save the background, but must */
				    /* be fully visible to get it all */
				    if (!epic_data->bg_visible &&
					((aye + epic_data->aheight) > 0) &&
					(aye < hw->html.view_height) &&
					((axe + epic_data->awidth) > 0) &&
					(axe < hw->html.view_width)) {
					HTMLDrawBackgroundImage((Widget)hw,
			 		 		    axe, aye,
					  		    epic_data->awidth,
					  		    epic_data->aheight);
					XCopyArea(dsp, win, epic_data->bg_image,
						  gc, axe, aye,
						  epic_data->awidth,
						  epic_data->aheight, 0, 0);
					/* Is it fully visible? */
					if ((aye >= 0) &&
			    		    ((aye + epic_data->aheight) <=
					     hw->html.view_height) &&
					    (axe >= 0) &&
					    ((axe + epic_data->awidth) <= 
					     hw->html.view_width))
					    epic_data->bg_visible = 1;
				    }
				    if (epic_data == pic_data) {
					/* Redisplay all for first image */
					XCopyArea(dsp, epic_data->bg_image,
						  epic_data->anim_image, gc,
						  0, 0, epic_data->awidth,
						  epic_data->aheight, 0, 0);
				    } else {
					XCopyArea(dsp, epic_data->bg_image,
						  epic_data->anim_image, gc,
						  pic_data->x, pic_data->y,
						  pic_data->width,
						  pic_data->height,
						  pic_data->x, pic_data->y);
					/* Clear previous image's area */
					if (epic_data->prev) {
					    pic = epic_data->prev;
					    XCopyArea(dsp, epic_data->bg_image,
						      epic_data->anim_image,
						      gc, pic->x, pic->y,
						      pic->width, pic->height,
						      pic->x, pic->y);
					}
				    }
				} else {
				    /* Get background color */
				    if (hw->html.cur_fg != eptr->bg) {
					XSetForeground(dsp, gc, eptr->bg);
					hw->html.cur_fg = eptr->bg;
				    }
				    if (epic_data == pic_data) {
					XFillRectangle(dsp,
						       epic_data->anim_image,
						       gc, 0, 0,
						       epic_data->awidth,
						       epic_data->aheight);
				    } else {
					XFillRectangle(dsp,
						       epic_data->anim_image,
						       gc, pic_data->x,
						       pic_data->y,
						       pic_data->width,
						       pic_data->height);
					/* Redo previous area */
					if (epic_data->prev) {
					    pic = epic_data->prev;
					    XFillRectangle(dsp,
							  epic_data->anim_image,
							  gc, pic->x, pic->y,
							  pic->width,
							  pic->height);
					}
				    }
				}
			    }
			} else {
			    values.clip_x_origin = x + extra;
			    values.clip_y_origin = y + extra;
			}

			valuemask = GCClipMask | GCClipXOrigin | GCClipYOrigin;
			XChangeGC(dsp, gc, valuemask, &values);

			if (epic_data->has_anim_image) {
				XCopyArea(dsp, pic_data->image,
					  epic_data->anim_image, gc, 0, 0,
					  pic_data->width, pic_data->height,
					  pic_data->x, pic_data->y);
				epic_data->has_anim_image = 1;
			} else {
				XCopyArea(dsp, pic_data->image, win, gc, 0, 0,
					  pic_data->width, pic_data->height,
					  x + extra, y + extra);
			}
			values.clip_mask = None;
			values.clip_x_origin = values.clip_y_origin = 0;
			XChangeGC(dsp, gc, valuemask, &values);

			/* Now display the animation image if visible */
			if (epic_data->has_anim_image &&
			    (((aye + epic_data->aheight) > 0) &&
			    (aye < hw->html.view_height)))
				XCopyArea(dsp, epic_data->anim_image, win, gc,
			  		  0, 0,
			  		  epic_data->awidth, epic_data->aheight,
			  		  axe, aye);
 		/*
		** If (image is transparent) and (background is solid
		** and color is different from transparent color used
		** in image) then change image to match background color.
 		*/
		} else {
			if (pic_data->transparent &&
			    (pic_data->background_pixel !=
			     hw->core.background_pixel)) {
				/* Create a GC if not already done. */
				if (!maskGC) {
					maskGC = XCreateGC(dsp, win, 0, 0);
					XCopyGC(dsp, gc, 0xFFFFFFFF, maskGC);
				}
				/* Clear the background pixels to 0 */
				values.foreground = 0xFFFFFFFF;
				values.background = 0;
				values.function = GXand;
				valuemask = GCForeground | GCBackground |
					    GCFunction;
				XChangeGC(dsp, maskGC, valuemask, &values);
				XCopyPlane(dsp, pic_data->clip,
					   pic_data->image, maskGC, 0, 0,
					   pic_data->width, pic_data->height,
					   0, 0, 1);
				/* Set background pixels to background color */
				values.foreground = 0;
				values.background = hw->core.background_pixel;
				values.function = GXor;
				XChangeGC(dsp, maskGC, valuemask, &values);
				XCopyPlane(dsp, pic_data->clip,
					   pic_data->image, maskGC, 0, 0,
					   pic_data->width, pic_data->height,
					   0, 0, 1);
				pic_data->background_pixel =
						      hw->core.background_pixel;
			}

			/* If has animation image, then need to save */
			if (epic_data->has_anim_image) {
				/* If image is empty, put background in */
				if (epic_data->has_anim_image != 1) {
				    if (hw->html.bgmap_SAVE) {
					XCopyArea(dsp, win,
						  epic_data->anim_image, gc,
						  axe, aye,
						  epic_data->awidth,
						  epic_data->aheight, 0, 0);
				    } else {
					if (hw->html.cur_fg != eptr->bg) {
					    XSetForeground(dsp, gc, eptr->bg);
					    hw->html.cur_fg = eptr->bg;
					}
					XFillRectangle(dsp,
						       epic_data->anim_image,
						       gc, 0, 0,
						       epic_data->awidth,
						       epic_data->aheight);
				    }
				}
				XCopyArea(dsp, pic_data->image,
					  epic_data->anim_image, gc, 0, 0,
			 	 	  pic_data->width, pic_data->height,
					  pic_data->x, pic_data->y);
				epic_data->has_anim_image = 1;
			}
			XCopyArea(dsp, pic_data->image, win, gc, 0, 0,
				  pic_data->width, pic_data->height,
				  x + extra, y + extra);
		}
	}
	/* Is it the first image in an animation? */
	if (!timer_refresh && epic_data->fetched && epic_data->anim_info &&
	    !epic_data->running) {
		int delay = pic_data->delay;

		if (!app_con)
			app_con = XtWidgetToApplicationContext((Widget) hw);
		if (!delay)
			delay = 10;
		epic_data->anim_info->current = pic_data;
		epic_data->anim_info->eptr = eptr;
		/* Because of refresh sequence, need to start over */
		epic_data->anim_info->next = epic_data->anim_info->start;
		epic_data->timer = XtAppAddTimeOut(app_con, delay,
					      (XtTimerCallbackProc)ImageAnimate,
					      (XtPointer)epic_data->anim_info);
		/* We don't want to start it again until a reformat */
		epic_data->running = 1;
	}
}

/* Timer routine to display next GIF animation image.  It frees the
 * animation data record when done.
 */
static void ImageAnimate(XtPointer cld, XtIntervalId *id)
{
	AnimInfo *anim_info = (AnimInfo *)cld;
	ImageInfo *pic_data = anim_info->next;
	ImageInfo *current, *epic;
	int delay;
	HTMLWidget hw = anim_info->hw;
	ElemInfo *eptr = anim_info->eptr;

	/* Check if window still exists */
	if (!*anim_info->window || !XtIsRealized((Widget) hw)) {
		free(anim_info);
		return;
	}
	eptr->pic_data->timer = 0;

	/* Check if window has been refreshed or animation stopped */
	if (anim_info->drawing != hw->html.draw_count) {
		free(anim_info);
		return;
	}

	/* Tell ExtendAdjust on other windows that we may screw up the GC */
        Select_GC_Reset = 1;

	if (!pic_data) {	/* At end of images */
		/* Zero is infinite */
		if (anim_info->count) {
			/* Interations finished */
			if (!--anim_info->count) {
				anim_info->start->running = 2;
				anim_info->start->last = anim_info->current;
				free(anim_info);
				return;
			}
		}
		pic_data = anim_info->start;
		/* Reset the animation image */
		if (pic_data->has_anim_image)
			pic_data->has_anim_image = -1;
	}
	epic = eptr->pic_data;
	epic->prev = NULL;
	current = anim_info->current;
	/* Replace background covered by previous image, if requested */
	if (current->disposal == 2) {
		if (epic->has_anim_image) {
			epic->has_anim_image = -1;
			if (pic_data->transparent == 1) {
				if ((current->x != pic_data->x) ||
				    (current->y != pic_data->y) ||
				    (current->width > pic_data->width) ||
				    (current->height > pic_data->height))
					/* Previous one was a different size */
					epic->prev = current;
				/* Animation image will handle the background */
				goto skipped_it;
			}
		}
		/* Skip if next image completely covers previous one */
		if ((current->x != pic_data->x) ||
		    (current->y != pic_data->y) ||
		    (current->width > pic_data->width) ||
		    (current->height > pic_data->height)) {
			int width, height;
			int sx = eptr->x;
			int sy = eptr->y;
			int extra = 0;

			/* Get border width, if any */
			if (epic->has_border &&
			    (!epic->internal || (epic->internal == 2)))
				extra = eptr->bwidth;

			sx += current->x + extra - hw->html.scroll_x;
			sy += current->y + extra - hw->html.scroll_y;
			if ((current->x + current->width) > epic->awidth) {
				width = epic->awidth -
						  (current->x + current->width);
			} else {
				width = current->width;
			}
			if ((current->y + current->height) > epic->aheight) {
				height = epic->aheight -
						 (current->y + current->height);
			} else {
				height = current->height;
			}
			if ((width > 0) && (height > 0)) {
				if (hw->html.bgmap_SAVE) {
					/* There is a background image */
					HTMLDrawBackgroundImage((Widget)hw,
						         sx, sy, width, height);
				} else {
					/* Get correct background color */
					XSetForeground(hw->html.dsp,
						       hw->html.drawGC,
						       eptr->bg);
					/* Must set every time due to drawGC
					 * being shared between windows */ 
					hw->html.cur_fg = eptr->bg;
					XFillRectangle(hw->html.dsp,
						       XtWindow(hw->html.view),
						       hw->html.drawGC, sx, sy,
						       width, height);
				}
			}
		}
	}
 skipped_it:

	anim_info->current = pic_data;
	anim_info->next = pic_data->next;

	/* Only refresh if visible or has animation image */
	if (epic->has_anim_image ||
	    (((eptr->y + eptr->height) > hw->html.scroll_y) &&
	     (eptr->y < (hw->html.scroll_y + hw->html.view_height)))) {
		unsigned long save_bg = hw->core.background_pixel;

		/* Get correct background in case in table cell */
		hw->core.background_pixel = eptr->bg;

		ImageRefresh(hw, eptr, pic_data);

		hw->core.background_pixel = save_bg;
	}
	if (hw->html.drawing && (pic_data->delay < 500)) {
		/* If still formatting window, them slow down animations */
		delay = 500;
	} else if (pic_data->delay) {
		delay = pic_data->delay * 10;
	} else {
		delay = 10;
	}
	epic->timer = XtAppAddTimeOut(app_con, delay,
				      (XtTimerCallbackProc)ImageAnimate,
				      (XtPointer)anim_info);
}
