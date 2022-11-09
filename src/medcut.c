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
#include "mosaic.h"
#include "medcut.h"
#include "main.h"

#define RED     0
#define GREEN   1
#define BLUE    2

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int reportBugs;
#endif

#define FindHash(red, green, blue, h_ptr) \
  if (hash_scale) { \
      h_ptr = Hash[((((red * 306) + (green * 601) + (blue * 117)) >> 10) * \
	            NCells) >> 16]; \
  } else { \
      h_ptr = Hash[(((red * 306) + (green * 601) + (blue * 117)) >> 10) * \
	            NCells >> 8]; \
  } \
  while(h_ptr) { \
      if ((h_ptr->pixel[RED] == red) && \
	  (h_ptr->pixel[GREEN] == green) && \
	  (h_ptr->pixel[BLUE] == blue)) \
	  break; \
      h_ptr = h_ptr->hash_next; \
  }

/* 0 = hash 8 bit values, 1 = hash 16 bit values */
int hash_scale;

HashInfo *Hash[256];

static struct c_box_rec {
	int min_pix[3];
	int max_pix[3];
	int count;
	HashInfo *c_data;
} C_boxes[256];

static int BoxCount;
static HashInfo *hash_ptr;
HashInfo *free_hash = (HashInfo *)NULL;
static HashInfo *tptr;
static int Size;
static int ColorCnt;
static int NCells;


static void InitMinMax(int boxnum)
{
	C_boxes[boxnum].min_pix[RED] = 65536;
	C_boxes[boxnum].max_pix[RED] = 0;
	C_boxes[boxnum].min_pix[GREEN] = 65536;
	C_boxes[boxnum].max_pix[GREEN] = 0;
	C_boxes[boxnum].min_pix[BLUE] = 65536;
	C_boxes[boxnum].max_pix[BLUE] = 0;
}


void FreeHash()
{
	int i;

	for (i = 0; i < 256; i++) {
		hash_ptr = Hash[i];
		while (hash_ptr) {
			tptr = hash_ptr;
			hash_ptr = hash_ptr->hash_next;
			tptr->hash_next = free_hash;
			free_hash = tptr;
		}
	}
}


HashInfo *PixAddHash(int red, int green, int blue)
{
	int lum;

	if (hash_scale) {
		lum = ((((red * 306) + (green * 601) + (blue * 117)) >> 10) *
		       NCells) >> 16;
	} else {
		lum = ((((red * 306) + (green * 601) + (blue * 117)) >> 10) *
		       NCells) >> 8;
	}
	if (free_hash) {
		hash_ptr = free_hash;
		free_hash = free_hash->hash_next;
	} else {
		hash_ptr = (HashInfo *)	XtMalloc(sizeof(HashInfo));
	}
	if (!hash_ptr) {
		fprintf(stderr, "MedianCut: Cannot malloc color\n");
		mo_exit();
	}
	hash_ptr->pixel[RED] = red;
	hash_ptr->pixel[GREEN] = green;
	hash_ptr->pixel[BLUE] = blue;
	hash_ptr->box_num = 0;
	hash_ptr->next = NULL;
	hash_ptr->hash_next = Hash[lum];
	Hash[lum] = hash_ptr;
	return(hash_ptr);
}


static void AddColor(HashInfo *cptr, int boxnum)
{
	HashInfo *ptr;

	while (cptr) {
		ptr = cptr;
		cptr = cptr->next;
		ptr->box_num = boxnum;
		ptr->next = C_boxes[boxnum].c_data;
		C_boxes[boxnum].c_data = ptr;
		if (ptr->pixel[RED] < C_boxes[boxnum].min_pix[RED])
			C_boxes[boxnum].min_pix[RED] = ptr->pixel[RED];
		if (ptr->pixel[RED] > C_boxes[boxnum].max_pix[RED])
			C_boxes[boxnum].max_pix[RED] = ptr->pixel[RED];
		if (ptr->pixel[GREEN] < C_boxes[boxnum].min_pix[GREEN])
			C_boxes[boxnum].min_pix[GREEN] = ptr->pixel[GREEN];
		if (ptr->pixel[GREEN] > C_boxes[boxnum].max_pix[GREEN])
			C_boxes[boxnum].max_pix[GREEN] = ptr->pixel[GREEN];
		if (ptr->pixel[BLUE] < C_boxes[boxnum].min_pix[BLUE])
			C_boxes[boxnum].min_pix[BLUE] = ptr->pixel[BLUE];
		if (ptr->pixel[BLUE] > C_boxes[boxnum].max_pix[BLUE])
			C_boxes[boxnum].max_pix[BLUE] = ptr->pixel[BLUE];
	}
}


static void CountColors(unsigned char *data, XColor *colrs, int *color_used)
{
	unsigned char *dptr = data;
	register int i;
	int red, green, blue;
	register HashInfo *tptr;

	InitMinMax(0);
	C_boxes[0].c_data = NULL;
	ColorCnt = 0;

	memset(color_used, 0, 256 * sizeof(int *));

	for (i = Size; i > 0; i--)
		color_used[(int)*dptr++] = 1;

	for (i = 0; i < 256; i++) {
		if (!color_used[i])
			continue;
		red = colrs[i].red;
		green = colrs[i].green;
		blue = colrs[i].blue;
		FindHash(red, green, blue, tptr);
		if (!tptr) {
			tptr = PixAddHash(red, green, blue);
			AddColor(tptr, 0);
			ColorCnt++;
		}
	}
}


static void CountColors32x32x32(unsigned char *data, int MaxColors)
{
	unsigned char *dptr = data;
	register int i;
	unsigned int idx, x;
	int red, green, blue;
	register HashInfo *tptr;
	char color_used[32768];

	InitMinMax(0);
	C_boxes[0].c_data = NULL;
	ColorCnt = 0;

	memset(color_used, 0, 32768);

	for (i = Size; i > 0; i--) {
		/* Reduce 24 bit color to 15 bit (32x32x32 cube) */
		idx = (unsigned int)(*dptr & 0xF8) << 7;
		/* Round it up if needed */
		if ((idx < 31744) && ((*dptr & 0x07) > 3))
			idx += 1024;
		idx += x = (unsigned int)(*++dptr & 0xF8) << 2;
		if ((x < 992) && ((*dptr & 0x07) > 3))
			idx += 32;
		idx += x = (*++dptr & 0xF8) >> 3;
		if ((x < 31) && ((*dptr & 0x07) > 3))
			idx++;
		dptr++;
		if (!color_used[idx]) {
			if (++ColorCnt > MaxColors)
				return;
			color_used[idx] = 1;
		}
	}
	for (i = 0; i < 32768; i++) {
		if (!color_used[i])
			continue;
		red = (i & 0x7C00) >> 7;
		green = (i & 0x03E0) >> 2;
		blue = (i & 0x001F) << 3;
		FindHash(red, green, blue, tptr);
		if (!tptr) {
			tptr = PixAddHash(red, green, blue);
			AddColor(tptr, 0);
		}
	}
#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "Found %d colors in 32x32x32 cube\n", ColorCnt);
#endif
}


static int FindTarget(int *tptr)
{
	int i, indx, rr, gr, br;
	int range = 0;

	for (i = 0; i < BoxCount; i++) {
		rr = C_boxes[i].max_pix[RED] - C_boxes[i].min_pix[RED];
		gr = C_boxes[i].max_pix[GREEN] - C_boxes[i].min_pix[GREEN];
		br = C_boxes[i].max_pix[BLUE] - C_boxes[i].min_pix[BLUE];
		if (rr > range) {
			range = rr;
			*tptr = i;
			indx = RED;
		}
		if (gr > range)	{
			range = gr;
			*tptr = i;
			indx = GREEN;
		}
		if (br > range)	{
			range = br;
			*tptr = i;
			indx = BLUE;
		}
	}
	return(indx);
}


static void SplitBox(int boxnum, int color_indx)
{
	HashInfo *low, *high, *data;
	int med_cnt, split_val, low_cnt, high_cnt;
	int Low_cnt = 0, High_cnt = 0;
	int Greater = BoxCount++;
	int Lesser = boxnum;

	InitMinMax(Lesser);
	InitMinMax(Greater);
	data = C_boxes[boxnum].c_data;
	med_cnt = C_boxes[boxnum].count / 2;
	C_boxes[Lesser].c_data = NULL;
	C_boxes[Greater].c_data = NULL;
	while (med_cnt > 0) {
		if (data->pixel[color_indx] < data->next->pixel[color_indx]) {
			low = data;
			high = data->next;
			data = high->next;
		} else {
			high = data;
			low = data->next;
			data = low->next;
		}
		low->next = NULL;
		high->next = NULL;
		high_cnt = low_cnt = 1;
		split_val = low->pixel[color_indx];
		while (data) {
			tptr = data;
			data = data->next;
			if (tptr->pixel[color_indx] > split_val) {
				tptr->next = high;
				high = tptr;
				high_cnt++;
			} else {
				tptr->next = low;
				low = tptr;
				low_cnt++;
			}
		}
		if (low_cnt <= med_cnt) {
			AddColor(low, Lesser);
			Low_cnt += low_cnt;
			med_cnt -= low_cnt;
			if (med_cnt == 0) {
				AddColor(high, Greater);
				High_cnt += high_cnt;
			}
			data = high;
		} else {
			AddColor(high, Greater);
			High_cnt += high_cnt;
			data = low;
		}
	}
	C_boxes[Lesser].count = Low_cnt;
	C_boxes[Greater].count = High_cnt;
}


static void SplitColors(int e_cnt)
{
	if (ColorCnt < e_cnt) {
		int i;

		tptr = C_boxes[0].c_data;
		for (i = 0; i < ColorCnt; i++) {
			hash_ptr = tptr;
			tptr = tptr->next;
			C_boxes[i].c_data = hash_ptr;
			C_boxes[i].count = 1;
			hash_ptr->box_num = i;
			hash_ptr->next = NULL;
		}
		BoxCount = ColorCnt;
	} else {
		int target, color_indx;

		BoxCount = 1;
		while (BoxCount < e_cnt) {
			target = 0;
			color_indx = FindTarget(&target);
			SplitBox(target, color_indx);
		}
	}
}


static void ConvertData(unsigned char *data, XColor *colrs, int *colors_used)
{
	unsigned char *dptr = data;
	register HashInfo *hash_ptr;
	register int i;
	int red, green, blue;
	int pixel_map[256];

	/*
	 * Generate translation map.
	 */
	for (i = 0; i < 256; i++) {
		if (!colors_used[i])
			continue;
		red = colrs[i].red;
		green = colrs[i].green;
		blue = colrs[i].blue;
		FindHash(red, green, blue, hash_ptr);
		if (!hash_ptr) {
#ifndef DISABLE_TRACE
			if (reportBugs)
				fprintf(stderr, "Unknown color (%d,%d,%d)\n",
					red, green, blue);
#endif
			hash_ptr = Hash[0];
		}
		pixel_map[i] = hash_ptr->box_num;
	}

	for (i = Size; i > 0; i--) {
		*dptr = (unsigned char)pixel_map[(int)*dptr];
		dptr++;
	}
}


static unsigned char *Convert24BitData(unsigned char *data)
{
	unsigned char *dptr = data;
        unsigned char *iptr, *newdata;
	register int i;
	int red, green, blue;
	register HashInfo *hash_ptr;

	newdata = iptr = malloc(Size);
	for (i = Size; i > 0; i--) {
		red = *dptr & 0xF8;
		if ((red < 248) && ((*dptr & 0x07) > 3))
			red += 8;
		green = *++dptr & 0xF8;
		if ((green < 248) && ((*dptr & 0x07) > 3))
			green += 8;
		blue = *++dptr & 0xF8;
		if ((blue < 248) && ((*dptr & 0x07) > 3))
			blue += 8;
		dptr++;
		FindHash(red, green, blue, hash_ptr);
		if (!hash_ptr) {
#ifndef DISABLE_TRACE
			if (reportBugs)
				fprintf(stderr, "Unknown color (%d,%d,%d)\n",
					red, green, blue);
#endif
			hash_ptr = Hash[0];
		}
		*iptr++ = hash_ptr->box_num;
	}
	return newdata;
}


static void PrintColormap(int e_cnt, XColor *colrs)
{
	unsigned int Tred, Tgreen, Tblue;
	int i;

	for (i = 0; i < BoxCount; i++) {
		int c_cnt = 0;

		Tred = Tgreen = Tblue = 0;
		tptr = C_boxes[i].c_data;
		while (tptr) {
			Tred += tptr->pixel[RED];
			Tgreen += tptr->pixel[GREEN];
			Tblue += tptr->pixel[BLUE];
			c_cnt++;
			tptr = tptr->next;
		}
	        if (hash_scale) {
			colrs[i].red = Tred / c_cnt;
			colrs[i].green = Tgreen / c_cnt;
			colrs[i].blue = Tblue / c_cnt;
		} else {
			unsigned short x;

			/* Colors must be 16 bits instead of 8 */
			x = Tred / c_cnt;
			colrs[i].red = (x << 8) | x;
			x = Tgreen / c_cnt;
			colrs[i].green = (x << 8) | x;
			x = Tblue / c_cnt;
			colrs[i].blue = (x << 8) | x;
		}
	}
	for (i = BoxCount; i < e_cnt; i++)
		colrs[i].red = colrs[i].green = colrs[i].blue = 0;
#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "Using %d colors\n", BoxCount);
#endif
}


void MedianCut(unsigned char *data, int w, int h, XColor *colrs,
	       int start_cnt, int end_cnt)
{
	int colors_used[256];

	Size = h * w;  /* Globals */
	NCells = start_cnt;
	BoxCount = 0;
	ColorCnt = 0;
	hash_scale = 1;

	memset(C_boxes, 0, sizeof(C_boxes));
	memset(Hash, 0, sizeof(Hash));

	CountColors(data, colrs, colors_used);
	C_boxes[0].count = ColorCnt;
	SplitColors(end_cnt);
	ConvertData(data, colrs, colors_used);
	PrintColormap(end_cnt, colrs);
	FreeHash();
}


unsigned char *MedianCut24BitTo8(unsigned char *data, int w, int h,
				 XColor *colrs, int end_cnt, int max_cnt)
{
	unsigned char *idata;

	Size = h * w;  /* Globals */
	NCells = 256;
	BoxCount = 0;
	ColorCnt = 0;
	hash_scale = 0;

	memset(C_boxes, 0, sizeof(C_boxes));
	memset(Hash, 0, sizeof(Hash));

	CountColors32x32x32(data, max_cnt);
	if (ColorCnt > max_cnt) {	/* Too many colors? */
		FreeHash();
		return NULL;
	}
	C_boxes[0].count = ColorCnt;
	SplitColors(end_cnt);
	idata = Convert24BitData(data);
	PrintColormap(end_cnt, colrs);
	FreeHash();

	return idata;
}
