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

#include "../config.h"

#include "mosaic.h"
#include "mo-www.h"
#include "globalhist.h"
#include "../libhtmlw/HTMLp.h"
#include "picread.h"
#ifdef CCI
#include "cci.h"
#include "ccibindings2.h"
extern int cci_event;
#endif
#include "gifread.h"
#include "medcut.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int cacheTrace;
#endif

/* Defined in gui.c */
extern char *cached_url;
extern int browserSafeColors;
extern int BSCnum;
         
/* Defined in gui-documents.c */
extern int interrupted;

extern int Vclass;

/* Used in libwww2 */
int force_image_load = 0;
char *image_file_fnam = NULL;
extern char *HTReferer;

/* For selective image loading */
#define BlankImage_width 8
#define BlankImage_height 8
static unsigned char BlankImage_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
static ImageInfo *blank_image = NULL;

extern char **imagekill_sites;

ImageInfo *BlankImageData(HTMLWidget w)
{
	static Pixel fg_pixel, bg_pixel;

        if (blank_image == NULL) {
		blank_image = (ImageInfo *)malloc(sizeof(ImageInfo));
		blank_image->usemap = NULL;
		blank_image->map = NULL;
		blank_image->ismap = 0;
		blank_image->fptr = NULL;
		blank_image->internal = 3;
		blank_image->delayed = 0;
		blank_image->fetched = 0;
		blank_image->cached = 0;
		blank_image->width = BlankImage_width;
		blank_image->height = BlankImage_height;
		blank_image->num_colors = 2;
		blank_image->bg_index = 0;
		blank_image->image_data = (unsigned char*)BlankImage_bits;
		blank_image->clip_data = NULL;
		blank_image->transparent = 0;
		blank_image->clip = None;
		blank_image->alt_text = NULL;
		blank_image->src = NULL;
		blank_image->is_bg_image = 0;
		blank_image->aligned = 0;
		fg_pixel = w->manager.foreground;
		bg_pixel = w->core.background_pixel;
                blank_image->image = XCreatePixmapFromBitmapData(XtDisplay(w),
                        XtWindow(w), (char*) BlankImage_bits,
                        BlankImage_width, BlankImage_height,
                        fg_pixel,
                        bg_pixel,
                        DefaultDepthOfScreen(XtScreen(w)));
	}
	if ((fg_pixel != w->manager.foreground) ||
	    (bg_pixel != w->core.background_pixel)) {
		fg_pixel = w->manager.foreground;
		bg_pixel = w->core.background_pixel;
                blank_image->image = XCreatePixmapFromBitmapData(XtDisplay(w),
                        XtWindow(w), (char*) BlankImage_bits,
                        BlankImage_width, BlankImage_height,
                        fg_pixel,
                        bg_pixel,
                        DefaultDepthOfScreen(XtScreen(w)));
	}

	return(blank_image);
}

static unsigned char nums[] = { 1, 2, 4, 8, 16, 32, 64, 128 };

void ProcessImageData (HTMLWidget hw, ImageInfo *img_info, XColor *colrs)
{
	int bg = img_info->bg_index;
	int i, j, cnt, bcnt, pnum;
	int Used[256];
	int transition_count;
	int widthbyheight;
	unsigned short bg_red, bg_green, bg_blue;
	unsigned char *bg_map;
	unsigned char *bgptr;
	unsigned char *cptr;
	unsigned char *ptr;
	static int clipping;
	static int max_clip;
	static int max_colors;
	static int init = 0;

	if (!init) {
		clipping = get_pref_boolean(eCLIPPING);
		max_clip = get_pref_int(eMAX_CLIP_TRANSITIONS);
		max_colors = get_pref_int(eCOLORS_PER_INLINED_IMAGE);
		init = 1;
	}
	/* If we have a transparent background, prepare for it */
	bg_map = NULL;

	if (bg >= 0) {
		unsigned long bg_pixel;
		XColor tmpcolr;

/* This code copied from xpmread.c.  I could almost delete the code from there,
 * but I suppose an XPM file could pathologically have multiple transparent
 * colour indicies. -- GWP
 */
		bg_pixel = hw->core.background_pixel;
		tmpcolr.pixel = bg_pixel;
		XQueryColor(XtDisplay(hw), hw->core.colormap, &tmpcolr);
		bg_red = colrs[bg].red = tmpcolr.red;
		bg_green = colrs[bg].green = tmpcolr.green;
		bg_blue = colrs[bg].blue = tmpcolr.blue;
		colrs[bg].flags = DoRed|DoGreen|DoBlue;
		bg_map = (unsigned char *)malloc(img_info->width *
			img_info->height);
	}

	if ((bg >= 0 ) && clipping) {
		img_info->transparent = 1;
		img_info->clip_data = (unsigned char *)malloc(
			((img_info->width + 7)/8) * img_info->height);
		memset(img_info->clip_data, 0, ((img_info->width + 7)/8) *
			img_info->height);
	} else {
		img_info->transparent = 0;
		img_info->clip_data = NULL;
		bg = img_info->bg_index = (-1);
	}

	widthbyheight = img_info->width * img_info->height;
	for (i=0; i < 256; i++)          /* Fill out used color array. */
		Used[i] = 0;
	cnt = 1;
	bgptr = bg_map;
	cptr = img_info->clip_data;
	ptr = img_info->image_data;
	transition_count = 0;
 
	/* This sets the bg map, creates bitmap data for the clip mask
         * when there is a bg image and counts the colors
	 */
	for (i = 0; i < img_info->height; i++) {
		for (j = 0, bcnt = 0; j < img_info->width; j++) {
			if (Used[*ptr] == 0) {
				Used[*ptr] = cnt;
				cnt++;
			}
			if (bg >= 0) {
				if (*ptr == bg) {
					*bgptr = 1;
					if (transition_count & 1)
						transition_count++;
				} else {
					*bgptr = 0;
					if (!(transition_count & 1))
						transition_count++;
					*cptr += nums[bcnt % 8];
				}
				if ((bcnt % 8) == 7 || j == (img_info->width-1))
					cptr++;
				bgptr++;
				bcnt++;
			}
			ptr++;
		}
	}
	cnt--;

#ifndef DISABLE_TRACE
	if (srcTrace && (transition_count > 0))
		fprintf(stderr, "[IMG] transparency transition count = %d\n",
			 transition_count);
#endif

	/* The background color was never used */
	if (transition_count == 1) {
		img_info->transparent = 0;
		free(img_info->clip_data);
		img_info->clip_data = NULL;
		bg = img_info->bg_index = (-1);
	}

	if (img_info->transparent && (max_clip >= 0) &&
	    (transition_count > max_clip)) {
		img_info->transparent = 2;  /* Transparent but no clipmask */
	}
 
	/* If the image has too many colors, apply a median cut algorithm to
	 * reduce the color usage, and then reprocess it.
	 * Don't cut colors for direct mapped visuals like TrueColor.
	 * Also, cut colors less for internal image viewer files.
	 * If using browser safe colors, then cut to number of safe colors.
	 */
	pnum = max_colors;
	if (image_file_fnam && (pnum < 144))
		/* Internal image viewer file */
		pnum = 144;
	if (browserSafeColors && (pnum < BSCnum))
		pnum = BSCnum;
	if ((cnt > pnum) && (Vclass != TrueColor) && (Vclass != DirectColor)) {
		MedianCut(img_info->image_data, img_info->width,
			  img_info->height, colrs, 256, pnum);
		for (i=0; i < 256; i++)
			Used[i] = 0;
		cnt = 1;
		ptr = img_info->image_data;
		for (i=0; i < widthbyheight; i++) {
			if (Used[(int)*ptr] == 0) {
				Used[*ptr] = cnt;
				cnt++;
			}
			ptr++;
		}
		cnt--;

		/* If had a transparent bg, MedianCut used it.  Get a new one */
		if (bg >= 0) {
			cnt++;
			bg = 256;
		}
	}
	img_info->num_colors = cnt;

	/* bg is not set in here if it gets munged by MedCut */
	for (i=0; i < 256; i++) {
		int indx;

		if (Used[i] != 0) {
			indx = Used[i] - 1;
			img_info->colrs[indx] = colrs[i];
			/* squeegee in the background color */
			if ((bg >= 0) && (i == bg)) {
				img_info->colrs[indx].red = bg_red;
				img_info->colrs[indx].green = bg_green;
				img_info->colrs[indx].blue = bg_blue;
				img_info->bg_index = indx;
			}
		}
	}

	/* If MedianCut ate our background, add the new one now. */
	if (bg == 256) {
		img_info->colrs[cnt - 1].red = bg_red;
		img_info->colrs[cnt - 1].green = bg_green;
		img_info->colrs[cnt - 1].blue = bg_blue;
		img_info->bg_index = (cnt-1); 
	}                              
	bgptr = bg_map;
	ptr = img_info->image_data;
	for (i=0; i < widthbyheight; i++) {
		*ptr = (unsigned char)(Used[*ptr] - 1);
		/* If MedianCut ate the background, enforce it here */
		if (bg == 256) {
			if (*bgptr)
				*ptr = (unsigned char) img_info->bg_index;
			bgptr++;
		}
		ptr++;
	}
	if (bg_map)      /* Free the background map if we have one */
		free (bg_map);

}

/* Image resolution function. */
void ImageResolve(Widget w, XtPointer clid, XtPointer calld)
{
	ImageInfo *img_info = (ImageInfo *) calld;
	mo_window *win = (mo_window *) clid;
	mo_window *load_win;
	HTMLWidget hw = (HTMLWidget) w;
	ImageInfo *cache_info;
	ImageInfo *ainfo, *previous, *tmp;
	AnimInfo *anim_info;
	char *src = NULL;
	int width, height, aw, ah;
	int x, y, ax, ay;
	XColor colrs[256];
	XColor acolrs[256];
	char *filename;
	char *url;
	char *ptr;
	int rc;
	int bg, abg;
	unsigned char *bit_data;
	unsigned char *adata;
	int animated;
	int delay;
	int disposal, adisposal;
	int i, count;
	FILE *gif_fp = NULL;
	static char *referer = NULL;

        img_info->fetched = 0;
        img_info->internal = 0;
        img_info->cached = 0;
        img_info->width = 0;
        img_info->height = 0;
        img_info->num_colors = 0;
        img_info->image_data = NULL;
        img_info->clip_data = NULL;
	img_info->transparent = 0;
	img_info->bg_index = -1;
	img_info->image = (Pixmap)NULL;
	img_info->anim_image = (Pixmap)NULL;
	img_info->text = NULL;
	img_info->is_bg_image = 0;
	img_info->aligned = 0;
	img_info->anim_info = NULL;
	img_info->prev = NULL;

	if (!w) {
		fprintf(stderr, "[IMG] No Widget!\n");
		return;
	}

	if (img_info->src)
		src = strdup(img_info->src);
	if (!src)
		return;

	/* Now strip off leading blanks and convert newlines */
	src = mo_convert_newlines_to_spaces(src);

	/* Now strip out any spaces and CRs */
	if (strchr(src, ' ') || strchr(src, '\r')) {
		filename = src;
		for (ptr = src; *ptr; ptr++, filename++) {
  		      	while (*ptr && ((*ptr == ' ') || (*ptr == '\r')))
				ptr++;
			*filename = *ptr;
		}
		*filename = '\0';
	}
#ifndef DISABLE_TRACE
        if (srcTrace)
            fprintf(stderr, "[IMG] Processing src = '%s'\n", src);
#endif

	load_win = win;
	/* Get top level mo_window */
	while (win->is_frame) {
		win = win->parent;
	}
	if (!load_win->cached_url || !cached_url ||
	    strcmp(load_win->cached_url, cached_url)) {
		if (cached_url)
			free(cached_url);
		cached_url = strdup(load_win->cached_url ?
				    load_win->cached_url : "lose");
		if (load_win->cached_url)
			free(load_win->cached_url);
		load_win->cached_url = strdup(cached_url);
	}

	url = mo_url_canonicalize(src, load_win->cached_url);

	free(img_info->src);
	img_info->src = strdup(url);
	free(src);

	/* First, should we just kill it? */
        if (imagekill_sites) {
            for (i = 0; imagekill_sites[i]; i++) {
                if (strstr(url, imagekill_sites[i])) {
		    ImageInfo *bim;

		    bim = BlankImageData(hw);
		    img_info->width = bim->width;
		    img_info->height = bim->height;
		    img_info->internal = 3;
		    img_info->delayed = 0;
		    img_info->fetched = 0;
		    img_info->image_data = bim->image_data;
		    img_info->image = bim->image;
#ifdef CCI
		    if (cci_event) 
			MoCCISendEventOutput(IMAGE_LOADED);
#endif
#ifndef DISABLE_TRACE
	            if (srcTrace)
			fprintf(stderr, "[IMG] Killed it\n");
#endif
		    free(url);
		    return;
                }
            }
        }

	/* Second, look in the cache */
        cache_info = mo_fetch_cached_image_data(url);
        if (cache_info && cache_info->image_data) {
        	img_info->internal = 0;
        	img_info->delayed = 0;
        	img_info->fetched = 1;
		img_info->cached = 1;
                img_info->image_data = cache_info->image_data;
                img_info->clip_data = cache_info->clip_data;
		img_info->awidth = cache_info->awidth;
		img_info->aheight = cache_info->aheight;
		if (img_info->cw_only) {
			if (win->image_animation)
				img_info->anim_info = cache_info->anim_info;
			else
				img_info->anim_info = NULL;
			img_info->next = NULL;
		} else if (cache_info->anim_info && win->image_animation) {
			img_info->anim_info =
				(AnimInfo *)malloc(sizeof(AnimInfo));
			img_info->anim_info->hw = hw;
			img_info->anim_info->drawing =
				img_info->anim_info->hw->html.draw_count;
			img_info->anim_info->count = cache_info->iterations;
			img_info->anim_info->start = img_info;
			img_info->anim_info->next = NULL;
			img_info->anim_info->window = (int *)(&win->base);
			img_info->running = 0;
			img_info->has_anim_image = cache_info->has_anim_image;
			img_info->timer = 0;
			if (img_info->has_anim_image) {
				img_info->anim_image = XCreatePixmap(
					XtDisplay(hw),
					XtWindow(hw),
					img_info->awidth, img_info->aheight,
	                        	DefaultDepthOfScreen(XtScreen(hw)));
				img_info->bg_image = (Pixmap)NULL;
				img_info->bg_visible = 0;
			} else {
				img_info->anim_image = (Pixmap)NULL;
			}
			tmp = cache_info->next;
			previous = img_info;
			while (tmp) {
				ainfo = (ImageInfo *)malloc(sizeof(ImageInfo));
				previous->next = ainfo;
				previous = ainfo;
				ainfo->next = NULL;
		  	     	ainfo->transparent = tmp->transparent;        
		  	     	ainfo->bg_index = tmp->bg_index;        
				ainfo->width = tmp->width;
				ainfo->height = tmp->height;
				ainfo->x = tmp->x;
				ainfo->y = tmp->y;
				ainfo->disposal = tmp->disposal;
				ainfo->image_data = tmp->image_data;
		                ainfo->clip_data = tmp->clip_data;
				ainfo->delay = tmp->delay;
				ainfo->image = (Pixmap)NULL;
				ainfo->clip = None;
				ainfo->internal = 0;
				ainfo->anim_info = NULL;
				ainfo->num_colors = tmp->num_colors;
				for (i = 0; i < ainfo->num_colors; i++)
					ainfo->colrs[i] = tmp->colrs[i];
				tmp = tmp->next;
			}
		} else {
			img_info->next = NULL;
			img_info->anim_info = NULL;
			img_info->has_anim_image = 0;
		}
		img_info->delay = cache_info->delay;
		img_info->disposal = cache_info->disposal;
		img_info->iterations = cache_info->iterations;
                img_info->transparent = cache_info->transparent;
		img_info->bg_index = cache_info->bg_index;
		img_info->width = cache_info->width;
		img_info->height = cache_info->height;
		img_info->x = cache_info->x;
		img_info->y = cache_info->y;
		img_info->num_colors = cache_info->num_colors;
		for (i = 0; i < img_info->num_colors; i++)
			img_info->colrs[i] = cache_info->colrs[i];
#ifdef CCI
		if (cci_event) 
			MoCCISendEventOutput(IMAGE_LOADED);
#endif
		free(url);
#ifndef DISABLE_TRACE
	        if (srcTrace || cacheTrace)
			fprintf(stderr, "[IMG] Found it in cache.\n");
#endif
		return;
	}

	/* If we don't have the image cached, return if interrupted is high. */
	if (interrupted) {
		free (url);
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "[IMG] Returning Null interrupted %d\n",
				interrupted);
#endif
		return;
	}

	/* Is it delayed? */
	if (!force_image_load && (win->delay_image_loads ||
	    img_info->urldelayed)) {
		free(url);
		return;
	} else {
		force_image_load = 0;
	}

	/* Free previous referer, if any */
	if (referer) {
		free(referer);
	}
	referer = strdup(load_win->cached_url);
	/* Pass the value to libwww */
	HTReferer = referer;

	filename = mo_tmpnam(url);
        rc = mo_pull_er_over_virgin(url, filename);

	if (!rc) {
#ifndef DISABLE_TRACE
                if (srcTrace)
		    fprintf(stderr, "mo_pull_er_over_virgin returned %d\n", rc);
#endif
		free(filename);
		free(url);
		return;
	}

#ifdef CCI
	/* Send it through CCI if need be */
	MoCCISendBrowserViewFile(url, "unknown", filename);
#endif
	bit_data = ReadBitmap(w, filename, &width, &height, &x, &y, colrs, &bg,
		&animated, &delay, &disposal, &gif_fp);

	if (!bit_data) {
#ifndef DISABLE_TRACE
		if (srcTrace)
		    fprintf(stderr, "[IMG] data == NULL; punting...\n");
#endif
		remove(filename);
		free(filename);
		free(url);
		return;
	}

	if (gif_fp && win->image_animation) {
		count = 2;
		if (win->min_animation_delay > delay) {
			img_info->delay = win->min_animation_delay;
		} else {
			img_info->delay = delay;
		}
		Get_GIF_ScreenSize(&img_info->awidth, &img_info->aheight);
		if (img_info->awidth < width)
			img_info->awidth = width;
		if (img_info->aheight < height)
			img_info->aheight = height;
		img_info->disposal = disposal;
		/* Use animation image if disposal is 1 */
		if (disposal == 1)
			adisposal = 1;
		else
			adisposal = 0;
		img_info->running = 0;
		anim_info = (AnimInfo *)malloc(sizeof(AnimInfo));
		anim_info->hw = (HTMLWidget) w;
		anim_info->drawing = anim_info->hw->html.draw_count;
		anim_info->start = img_info;
		anim_info->next = NULL;
		anim_info->window = (int *)(&win->base);
		previous = img_info;
		previous->next = NULL;
		while (adata = ReadGIF(gif_fp, &aw, &ah, acolrs, &abg, count,
		       &i, &delay, &ax, &ay, &disposal, NULL)) {
			ainfo = (ImageInfo *)malloc(sizeof(ImageInfo));
			previous->next = ainfo;
			previous = ainfo;
			ainfo->next = NULL;
		       	ainfo->bg_index = abg;        
			ainfo->width = aw;
			ainfo->height = ah;
			if (img_info->awidth < aw)
				img_info->awidth = aw;
			if (img_info->aheight < ah)
				img_info->aheight = ah;
			/* If mixed sizes, then use animation image */
			if (!adisposal && ((aw != width) || (ah != height)))
				adisposal = 1;
			ainfo->x = ax;
			ainfo->y = ay;
			ainfo->disposal = disposal;
			if (disposal == 1)
				adisposal = 1;
			ainfo->image_data = adata;
			if (win->min_animation_delay > delay) {
				ainfo->delay = win->min_animation_delay;
			} else {
				ainfo->delay = delay;
			}
			ainfo->image = (Pixmap)NULL;
			ainfo->clip = None;
			ainfo->internal = 0;
			ainfo->anim_info = NULL;
			ProcessImageData((HTMLWidget) w, ainfo, acolrs);
			/* Force animation image if transparent */
			if (!adisposal && ainfo->transparent && (disposal == 2))
				adisposal = 1;
			count++;
		}
		if (animated >= 0) {
			anim_info->count = img_info->iterations = animated;
		} else {
			anim_info->count = img_info->iterations = 1;
		}
		if (count == 2) { /* Not animated */
			img_info->iterations = -1;
			img_info->next = NULL;
			if (anim_info)
				free(anim_info);
			anim_info = NULL;
		} else if (adisposal) {
			/* Has at least one disposal == 1 or has mixed
			 * image sizes or has transparent image with
			 * disposal == 2 */
			if (!img_info->cw_only)
				img_info->anim_image = XCreatePixmap(
					XtDisplay(hw),
					XtWindow(hw),
					img_info->awidth, img_info->aheight,
					DefaultDepthOfScreen(XtScreen(hw)));
			else
				img_info->anim_image = (Pixmap)NULL;
			img_info->has_anim_image = -1;
			img_info->bg_image = (Pixmap)NULL;
			img_info->bg_visible = 0;
		} else {
			img_info->anim_image = (Pixmap)NULL;
			img_info->has_anim_image = 0;
		}
	} else {
		img_info->iterations = animated;
		img_info->next = NULL;
		anim_info = NULL;
	}

	if (gif_fp && (gif_fp != stdin))
		fclose(gif_fp);

#ifndef DISABLE_TRACE
	if (srcTrace) {
		fprintf(stderr, "GIF animation iterations = %d\n",
			img_info->iterations);
		fprintf(stderr, "bg = %d\n", bg);
	}
#endif
	/* Now delete the file. */
	remove(filename);
	free(filename);

       	img_info->bg_index = bg;        
       	img_info->internal = 0;
       	img_info->delayed = 0;
	img_info->fetched = 1;
	img_info->cached = 1;
	img_info->width = width;
	img_info->height = height;
	img_info->x = x;
	img_info->y = y;
	img_info->image_data = bit_data;
	img_info->anim_info = anim_info;
	img_info->timer = 0;

	ProcessImageData((HTMLWidget) w, img_info, colrs);

	/* Save internal image viewer file for later deletion and
	 * delete any previous one for this node */
	if (image_file_fnam) {
		if (win->current_node) {
			if (win->current_node->image_file) {
				remove(win->current_node->image_file);
				free(win->current_node->image_file);
			}
			win->current_node->image_file = image_file_fnam;
		} else {
			/* Only happens if window started up with it */
			win->image_file = image_file_fnam;
		}
		image_file_fnam = NULL;
	}

	cache_info = (ImageInfo *)malloc(sizeof(ImageInfo));
        cache_info->bg_index = img_info->bg_index;        
       	cache_info->internal = 0;
       	cache_info->delayed = 0;
	cache_info->width = width;
	cache_info->height = height;
	cache_info->awidth = img_info->awidth;
	cache_info->aheight = img_info->aheight;
	cache_info->x = x;
	cache_info->y = y;
        cache_info->image_data = img_info->image_data;
        cache_info->iterations = img_info->iterations;
        cache_info->delay = img_info->delay;
        cache_info->disposal = img_info->disposal;
	cache_info->has_anim_image = img_info->has_anim_image;
	cache_info->fetched = 1;
	cache_info->cached = 1;
        cache_info->num_colors = img_info->num_colors;
        cache_info->clip_data = img_info->clip_data;
	cache_info->transparent = img_info->transparent;
	for (i = 0; i < img_info->num_colors; i++)
		cache_info->colrs[i] = img_info->colrs[i];
	cache_info->anim_info = anim_info;
	if (img_info->cw_only) {
		if (anim_info) {
			cache_info->next = img_info->next;
			img_info->next = NULL;
		} else {
			cache_info->next = NULL;
		}
	} else if (anim_info) {
		tmp = img_info->next;
		previous = cache_info;
		while (tmp) {
			ainfo = (ImageInfo *)malloc(sizeof(ImageInfo));
			previous->next = ainfo;
			previous = ainfo;
			ainfo->next = NULL;
	  	     	ainfo->transparent = tmp->transparent;        
	  	     	ainfo->bg_index = tmp->bg_index;        
			ainfo->width = tmp->width;
			ainfo->height = tmp->height;
			ainfo->x = tmp->x;
			ainfo->y = tmp->y;
			ainfo->disposal = tmp->disposal;
			ainfo->image_data = tmp->image_data;
	                ainfo->clip_data = tmp->clip_data;
			ainfo->delay = tmp->delay;
			ainfo->image = (Pixmap)NULL;
			ainfo->clip = None;
			ainfo->internal = 0;
			ainfo->anim_info = NULL;
			ainfo->num_colors = tmp->num_colors;
			for (i = 0; i < ainfo->num_colors; i++)
				ainfo->colrs[i] = tmp->colrs[i];
			tmp = tmp->next;
		}
	} else {
		cache_info->next = NULL;
	}
#ifndef DISABLE_TRACE
	if (srcTrace || cacheTrace)
		fprintf(stderr, "[IMG] Doing mo_cache_data on '%s', 0x%08x\n",
			url, cache_info);
#endif
	mo_cache_data(url, (void *)cache_info, 0);

	free(url);

#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "[IMG] Leaving...\n");
#endif
#ifdef CCI
	if (cci_event) MoCCISendEventOutput(IMAGE_LOADED);
#endif
	return;
}

/* Used only by globalhist.c */
mo_status mo_free_image_data (void *ptr)
{
	ImageInfo *img = (ImageInfo *)ptr;
	ImageInfo *next = img->next;
	ImageInfo *tmp;

#ifndef DISABLE_TRACE
	if (srcTrace || cacheTrace)
		fprintf(stderr, "[mo_free_image_info] Freeing 0x%08x\n", img);
#endif

	if (!img)
		return mo_fail;

	if (img->image_data)
		free(img->image_data);
	if (img->clip_data)
		free(img->clip_data);
	/* Free any animation */
	while (next) {
		tmp = next;
		next = tmp->next;
		if (tmp->image_data)
			free(tmp->image_data);
		if (tmp->clip_data)
			free(tmp->clip_data);
		free(tmp);
	}

	free(img);  /* Free the struc also */

	return mo_succeed;
}
