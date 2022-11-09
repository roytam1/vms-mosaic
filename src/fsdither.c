/*
 * Taken and heavily modified from xanim_utils.c by GEC for VMS Mosaic, 1999.
 *
 * Copyright (C) 1991,1992,1993,1994 by Mark Podlipec. 
 * All rights reserved.
 *
 * This software may be freely copied, modified and redistributed
 * without fee provided that this copyright notice is preserved 
 * intact on all copies and modified copies.
 * 
 * There is no warranty or other guarantee of fitness of this software.
 * It is provided solely "as is". The author(s) disclaim(s) all
 * responsibility and liability with respect to this software's usage
 * or its effect upon hardware or computer systems.
 *
 */

/* Copyright (C) 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"

#include <stdio.h>
#include <stdlib.h>
#include "../libhtmlw/HTML.h"
#include <X11/Xlib.h>

#define CMAP_ABS(x)  ((x < 0) ? -(x) : x)

/* 5 is 32K, 6 is 256K, 7 is 2M and 8 is 16M */
#define CMAP_CACHE_BITS 6

static unsigned short *cmap_cache = NULL;
static unsigned int cmap_cache_size;
static unsigned int cmap_cache_rmask;
static unsigned int cmap_cache_gmask;
static unsigned int cmap_cache_bmask;
static ImageInfo *cmap_cache_image;

typedef struct {
  unsigned short red, green, blue;
} ColorReg;


static void CMAP_Cache_Init()
{
  cmap_cache_size = 0x01 << (CMAP_CACHE_BITS * 3);
  cmap_cache = (unsigned short *)malloc(cmap_cache_size *
				        sizeof(unsigned short));
  if (!cmap_cache)
    fprintf(stderr, "CMAP_CACHE: malloc failure\n");
  cmap_cache_bmask = ((0x01 << CMAP_CACHE_BITS) - 1) << 8;
  cmap_cache_gmask = cmap_cache_bmask << CMAP_CACHE_BITS;
  cmap_cache_rmask = cmap_cache_gmask << CMAP_CACHE_BITS;
  cmap_cache_image = NULL;
}


static int CMAP_Find_Closest(XColor *t_cmap, unsigned int csize,
			     int r, int g, int b)
{
    static ColorReg find_cmap[256];
    static XColor *cur_find_cmap = NULL;
    static unsigned int find_red[256], find_green[256], find_blue[256];
    register unsigned int i, min_diff;
    register int cmap_entry;

    if (cur_find_cmap != t_cmap) {
      if (!cur_find_cmap) {
        for (i = 0; i < 256; i++) {
	  find_red[i]   = 11 * i * i; 
	  find_green[i] = 16 * i * i;
	  find_blue[i]  =  5 * i * i;
        }
      }
      for (i = 0; i < csize; i++) {  
	find_cmap[i].red   = t_cmap[i].red   >> 8;
	find_cmap[i].green = t_cmap[i].green >> 8;
	find_cmap[i].blue  = t_cmap[i].blue  >> 8;
      }  
      cur_find_cmap = t_cmap;
    }
    r *= 257;
    g *= 257;
    b *= 257;
    r >>= 8;
    g >>= 8;
    b >>= 8;
    cmap_entry = 0;
    for (i = 0; i < csize; i++) {
      register unsigned int diff;

      diff  = find_red[  CMAP_ABS(r - (int)(find_cmap[i].red))];
      diff += find_green[CMAP_ABS(g - (int)(find_cmap[i].green))];
      diff += find_blue[ CMAP_ABS(b - (int)(find_cmap[i].blue))];
      if (i == 0)
	min_diff = diff;
  
      if (diff == 0)
	return(i);
      if (diff < min_diff) {
	min_diff = diff;
	cmap_entry = i;
      }
    }
    return(cmap_entry);
}


/*
 * Floyd-Steinberg dither an image
 */
void FS_Dither(ImageInfo *image, unsigned char *image_out,
	       XColor *new_colors, int num_colors, unsigned char *alpha_in)
{
  unsigned int width = image->width;
  unsigned int height = image->height;
  unsigned int flag = 0;
  ColorReg *cmap_in;
  register unsigned int x, y;
  short *err_buff0, *err_buff1, *e_ptr, *ne_ptr;
  short r_err, g_err, b_err;
  unsigned char *in, *o_ptr;
  static unsigned int shift_r = 3 * CMAP_CACHE_BITS;
  static unsigned int shift_g = 2 * CMAP_CACHE_BITS;
  static unsigned int shift_b = CMAP_CACHE_BITS;

  if (!cmap_cache)
    CMAP_Cache_Init();
  if (image != cmap_cache_image) {
    /*
     * Set CMAP cache to all 0xffff's.  Since CMAP's are currently
     * limited to 256 in size, this is a non-valid value.
     */
    memset(cmap_cache, 255, cmap_cache_size * sizeof(unsigned short));
    cmap_cache_image = image;
  }
  
  /* Allocate error buffer and set up pointers */
  e_ptr = err_buff1 = err_buff0 = (short *)malloc(6 * width * sizeof(short));
  if (!err_buff0) {
    fprintf(stderr, "FSDither: malloc failure\n");
    return;
  }
  err_buff1 += 3 * width;

  if (alpha_in) {
    in = alpha_in;
  } else {
    in = image->image_data;
  }

  {
    register unsigned int i;
    register unsigned int msize = image->num_colors;

    cmap_in = (ColorReg *)malloc(msize * sizeof(ColorReg));
    if (!cmap_in) {
      fprintf(stderr, "FSDITHER: cmap malloc failure\n");
      return;
    }
    for (i = 0; i < msize; i++) {
      cmap_in[i].red   = image->colrs[i].red >> 8;
      cmap_in[i].green = image->colrs[i].green >> 8;
      cmap_in[i].blue  = image->colrs[i].blue >> 8;
    }
  }

  {
    register unsigned char *i_ptr = (unsigned char *)(in + (width *
							    (height - 1)));

    x = width;
    while (x--) {
      register unsigned int p = *i_ptr++;

      *e_ptr++ = (unsigned short)cmap_in[p].red   << 4;
      *e_ptr++ = (unsigned short)cmap_in[p].green << 4;
      *e_ptr++ = (unsigned short)cmap_in[p].blue  << 4;
    }
  }

  y = height;
  while (y--) {
    o_ptr = (unsigned char *)(image_out + (width * y));
    if (flag) {
      e_ptr = err_buff1;
      ne_ptr = err_buff0;
      flag = 0;
    } else {
      e_ptr = err_buff0;
      ne_ptr = err_buff1;
      flag = 1;
    }
    if (y) {
      register short *tptr = ne_ptr;
      register unsigned char *i_ptr = (unsigned char *)(in + (width * (y - 1)));

      x = width;
      while (x--) {
	register unsigned int p = *i_ptr++;

	*tptr++ = (unsigned short)cmap_in[p].red   << 4;
	*tptr++ = (unsigned short)cmap_in[p].green << 4;
	*tptr++ = (unsigned short)cmap_in[p].blue  << 4;
      }
    }

    x = width;
    while (x--) {
      unsigned int color_out, cache_i;
      register short r, g, b;

      r = *e_ptr++ / 16;
      if (r < 0) {
	r = 0;
      } else if (r > 255) {
	r = 255;
      }
      g = *e_ptr++ / 16;
      if (g < 0) {
	g = 0;
      } else if (g > 255) {
	g = 255;
      }
      b = *e_ptr++ / 16;
      if (b < 0) {
	b = 0;
      } else if (b > 255) {
	b = 255;
      }

      cache_i = (((r << shift_r) & cmap_cache_rmask) |
                 ((g << shift_g) & cmap_cache_gmask) |
                 ((b << shift_b) & cmap_cache_bmask)) >> 8;
      if (cmap_cache[cache_i] == 0xffff) {
	color_out = CMAP_Find_Closest(new_colors, num_colors, r, g, b);
	cmap_cache[cache_i] = (short)color_out;
      } else {
	color_out = (unsigned int)cmap_cache[cache_i];
      }
      r_err = r - (short)(new_colors[color_out].red >> 8);
      g_err = g - (short)(new_colors[color_out].green >> 8);
      b_err = b - (short)(new_colors[color_out].blue >> 8);

      *o_ptr++ = (unsigned char)new_colors[color_out].pixel;

      if (x) {
	e_ptr[0] += 7 * r_err;
	e_ptr[1] += 7 * g_err;
	e_ptr[2] += 7 * b_err;
      }
      if (y) {
        if (x < (width - 1)) {		/* Not 1st of line */
	  *ne_ptr++ += 3 * r_err;
	  *ne_ptr++ += 3 * g_err;
	  *ne_ptr++ += 3 * b_err;
	}
        ne_ptr[0] += 5 * r_err;
        ne_ptr[1] += 5 * g_err;
        ne_ptr[2] += 5 * b_err;
        if (x) {
	  ne_ptr[3] += r_err;
	  ne_ptr[4] += g_err;
	  ne_ptr[5] += b_err;
	}
      }
    }  /* End of x */
  }  /* End of y */

  if (err_buff0)
    free(err_buff0);
  if (cmap_in)
    free(cmap_in);

  return;
}
