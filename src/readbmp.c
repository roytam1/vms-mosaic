/*\
 * Modified for VMS Mosaic by GEC on 12/12/99
 *
 * $Id: bmptoppm.c,v 1.10 1992/11/24 19:38:17 dws Exp dws $
 * 
 * bmptoppm.c - Converts from a Microsoft Windows or OS/2 .BMP file to a
 * PPM file.
 * 
 * The current implementation is probably not complete, but it works for
 * all the BMP files I have.  I welcome feedback.
 * 
 * Copyright (C) 1992 by David W. Sanderson.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  This software is provided "as is"
 * without express or implied warranty.
 * 
\*/

/* Copyright (C) 2004, 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"
#include "mosaic.h"
#include <X11/Xlib.h>
#include "readbmp.h"
#include "readjpeg.h"
#include "medcut.h"
#include "quantize.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int reportBugs;
#endif

#define	MAXCOLORS 256

typedef unsigned char pixval;

typedef struct bmp_rec {
	FILE *fp;
	unsigned long pos;
	unsigned long offBits;
	unsigned long cx;
	unsigned long cy;
	int class;
	int colors;
	int comp;
	unsigned short cBitCount;
	unsigned long redmask;
	unsigned long greenmask;
	unsigned long bluemask;
	unsigned long alphamask;
} bmpInfo;

static char er_internal[] = "BMP: %s: internal error!\n";

/*
 * Classes of BMP files
 */
#define C_WIN_V3  1
#define C_OS2     2
#define C_OS2_V2  3
#define C_WIN_V4  4
#define C_WIN_V5  5

/*
 * Compression types
 */
#define BI_RGB       0
#define BI_RLE8      1
#define BI_RLE4      2
#define BI_BITFIELDS 3

/* For expanding 5-bit pixel values to 8-bit with best rounding */
static const unsigned char c5to8bits[32] = {
    0,   8,  16,  25,  33,  41,  49,  58,
   66,  74,  82,  90,  99, 107, 115, 123,
  132, 140, 148, 156, 165, 173, 181, 189,
  197, 206, 214, 222, 230, 239, 247, 255
};


static int GetShift(unsigned long mask, int *masklen)
{
	int shift = 0;
	int len = 0;

	if (mask) {
		while (!(mask & 1)) {
			mask = mask >> 1;
			shift--;
		}
		while (mask & 1) {
			mask = mask >> 1;
			len++;
		}
	}
	shift += 8 - len;
	*masklen = len;

	return shift;
}


static unsigned long BMPleninfoheader(int class)
{
	switch (class) {
	    case C_WIN_V3:
		return 40;
	    case C_OS2:
		return 12;
	    case C_OS2_V2:
		return 64;
	    case C_WIN_V4:
		return 108;
	    case C_WIN_V5:
		return 124;
	    default:
#ifndef DISABLE_TRACE
		if (reportBugs)
			fprintf(stderr, er_internal, "BMPleninfoheader");
#endif
		return 0;
	}
}


static unsigned long BMPlenrgbtable(bmpInfo *bmp)
{
	unsigned long lenrgb;

	/* No color map if more than 8 color bits */
	if (bmp->cBitCount > 8) {
		/* Bit fields take up 12 bytes in color table area except
		 * for V4 and V5 where they are part of the Bitmap Header
		 */
		if ((bmp->comp == BI_BITFIELDS) &&
		    (bmp->class != C_WIN_V4) && (bmp->class != C_WIN_V5)) {
			return 12;
		} else {
			return 0;
		}
	}

	switch (bmp->class) {
	    case C_WIN_V3:
	    case C_WIN_V4:
	    case C_WIN_V5:
	    case C_OS2_V2:
		lenrgb = 4;
		break;
	    case C_OS2:
		lenrgb = 3;
		break;
	    default:
#ifndef DISABLE_TRACE
		if (reportBugs)
			fprintf(stderr, er_internal, "BMPlenrgbtable");
#endif
		return 0;
	}

	return bmp->colors ? bmp->colors * lenrgb :
			     (1 << bmp->cBitCount) * lenrgb;
}


/*
 * Length, in bytes, of a line of the image
 * 
 * Evidently each row is padded on the right as needed to make it a
 * multiple of 4 bytes long.  This appears to be true of both
 * OS/2 and Windows BMP files.
 */
static unsigned long BMPlenline(bmpInfo *bmp)
{
	unsigned long bitsperline = bmp->cx * bmp->cBitCount;

	/*
	 * If bitsperline is not a multiple of 32, then round
	 * bitsperline up to the next multiple of 32.
	 */
	if (bitsperline % 32)
		bitsperline += 32 - (bitsperline % 32);

	/* Number of bytes per line == bitsperline / 8 */
	return bitsperline >> 3;
}


/* Return the number of bytes used to store the image bits */
static unsigned long BMPlenbits(bmpInfo *bmp)
{
	return bmp->cy * BMPlenline(bmp);
}


/* Return the offset to the BMP image bits */
static unsigned long BMPoffbits(bmpInfo *bmp)
{
	return 14 + BMPleninfoheader(bmp->class) + BMPlenrgbtable(bmp);
}

/* Return the size of the BMP file in bytes */
static unsigned long BMPlenfile(bmpInfo *bmp)
{
	return BMPoffbits(bmp) + BMPlenbits(bmp);
}


/*\
 * $Id: bitio.c,v 1.5 1992/11/24 19:36:46 dws Exp dws $
 *
 * bitio.c - bitstream I/O
 *
 * Works for (sizeof(unsigned long)-1)*8 bits.
 *
 * Copyright (C) 1992 by David W. Sanderson.
\*/

struct bitstream {
	FILE *f;		/* Bytestream */
	unsigned long bitbuf;	/* Bit buffer */
	int nbitbuf;		/* Number of bits in 'bitbuf' */
	char mode;
};

typedef struct bitstream *BITSTREAM;

#define MASK(n)		 ((1 << (n)) - 1)

#define BitPut(b, ul, n) ((b)->bitbuf = (((b)->bitbuf << (n)) |	\
					 ((ul) & MASK(n))),	\
			  (b)->nbitbuf += (n))

#define BitGet(b, n)	 (((b)->bitbuf >> ((b)->nbitbuf -= (n))) & MASK(n))

/*
 * pm_bitinit() - allocate and return a struct bitstream * for the
 * given FILE*.
 *
 * mode must be one of "r" or "w", according to whether you will be
 * reading from or writing to the struct bitstream *.
 *
 * Returns 0 on error.
 */
static struct bitstream *pm_bitinit(FILE *f, char *mode)
{
	struct bitstream *ans = (struct bitstream *) NULL;

	if (!f || !mode || !*mode || (strcmp(mode, "r") && strcmp(mode, "w")))
		return ans;

	ans = (struct bitstream *)calloc(1, sizeof(struct bitstream));
	if (ans) {
		ans->f = f;
		ans->mode = *mode;
	}
	return ans;
}

/*
 * pm_bitfini() - deallocate the given struct bitstream *.
 *
 * You must call this after you are done with the struct bitstream *.
 * 
 * It may flush some bits left in the buffer.
 *
 * Returns the number of bytes written, -1 on error.
 */
static int pm_bitfini(struct bitstream *b)
{
	int nbyte = 0;

	if (!b)
		return -1;

	/* Flush the output */
	if (b->mode == 'w') {
		/* Flush the bits */
		if (b->nbitbuf < 0 || b->nbitbuf >= 8)
			/* pm_bitwrite() didn't work */
			return -1;
		/*
		 * If we get to here, nbitbuf is 0..7
		 */
		if (b->nbitbuf)	{
			char	c;

			BitPut(b, 0, (long)8 - b->nbitbuf);
			c = (char) BitGet(b, (long)8);
			if (putc(c, b->f) == EOF)
				return -1;
			nbyte++;
		}
	}
	free(b);
	return nbyte;
}

/*
 * pm_bitread() - read the next nbits into *val from the given file.
 * 
 * The last pm_bitread() must be followed by a call to pm_bitfini().
 * 
 * Returns the number of bytes read, -1 on error.
 */
static int pm_bitread(struct bitstream *b, unsigned long nbits,
		      unsigned long *val)
{
	int nbyte = 0;
	int c;

	if (!b)
		return -1;

	while (b->nbitbuf < nbits) {
		if ((c = getc(b->f)) == EOF)
			return -1;
		nbyte++;
		BitPut(b, c, (long)8);
	}

	*val = BitGet(b, nbits);
	return nbyte;
}


static int GetByte(FILE *fp)
{
	int v;

	if ((v = fgetc(fp)) == EOF) {
#ifndef DISABLE_TRACE
		if (reportBugs)
			fprintf(stderr, "BMP: EOF in GetByte\n");
#endif
	}
	return v;
}


static short GetShort(FILE *fp)
{
	int c;
	short v;

	if ((c = fgetc(fp)) == EOF)
		goto ErrReturn;

	v = c & 0xff;

	if ((c = fgetc(fp)) == EOF)
		goto ErrReturn;

	v |= (c & 0xff) << 8;

	return v;

 ErrReturn:

#ifndef DISABLE_TRACE
	if (reportBugs)
		fprintf(stderr, "BMP: EOF in GetShort\n");
#endif
	return 0;
}


static long GetLong(FILE *fp)
{
	int c;
	long v;

	if ((c = fgetc(fp)) == EOF)
		goto ErrReturn;
	v = c & 0xff;

	if ((c = fgetc(fp)) == EOF)
		goto ErrReturn;
	v |= (c & 0xff) << 8;

	if ((c = fgetc(fp)) == EOF)
		goto ErrReturn;
	v |= (c & 0xff) << 16;

	if ((c = fgetc(fp)) == EOF)
		goto ErrReturn;
	v |= (c & 0xff) << 24;

	return v;

 ErrReturn:

#ifndef DISABLE_TRACE
	if (reportBugs)
		fprintf(stderr, "BMP: EOF in GetLong\n");
#endif
	return 0;
}


/*
 * readto - read as many bytes as necessary to position the
 * file at the desired offset.
 */
static void readto(bmpInfo *bmp, unsigned long dst)
{
	FILE *fp = bmp->fp;
	unsigned long pos = bmp->pos;

	if (!fp)
		return;

	if (pos > dst) {
#ifndef DISABLE_TRACE
		if (reportBugs)
			fprintf(stderr, "BMP: readto dest < cur pos\n");
#endif
	}

	for (; pos < dst; pos++) {
		if (fgetc(fp) == EOF) {
#ifndef DISABLE_TRACE
			if (reportBugs)
				fprintf(stderr, "BMP: EOF in readto\n");
#endif
		}
	}
	bmp->pos = pos;
}


/*
 * BMP reading routines
 */

static int BMPreadfileheader(bmpInfo *bmp)
{
	FILE *fp = bmp->fp;
	unsigned short c;

	if (GetByte(fp) != 'B') {
#ifndef DISABLE_TRACE
		if (srcTrace || reportBugs)
			fprintf(stderr, "Not a BMP file\n");
#endif
		return 0;
	}
	if ((c = GetByte(fp)) != 'M') {
#ifndef DISABLE_TRACE
		if (srcTrace || reportBugs) {
			if (c == 'A') {
				fprintf(stderr, "Unsupported BMP array file\n");
			} else {
				fprintf(stderr, "Not a BMP file\n");
			}
		}
#endif
		return 0;
	}
	GetLong(fp);			/* cbSize */
	GetShort(fp);			/* xHotSpot */
	GetShort(fp);			/* yHotSpot */

	bmp->offBits = GetLong(fp);
	bmp->pos += 14;

	return 1;
}

static int BMPreadinfoheader(bmpInfo *bmp)
{
	FILE           *fp = bmp->fp;
	unsigned long   cbFix, cx, cy, cCompress;
	unsigned short  cBitCount, cPlanes;
	int             class, colors, i;

	cbFix = GetLong(fp);

	switch (cbFix) {
	    case 12:
		class = C_OS2;

		cx = GetShort(fp);
		cy = GetShort(fp);
		cPlanes = GetShort(fp);
		cBitCount = GetShort(fp);
		colors = 0;
		cCompress = 0;

		break;

	    case 40:
		class = C_WIN_V3;

		cx = GetLong(fp);
		cy = GetLong(fp);
		cPlanes = GetShort(fp);
		cBitCount = GetShort(fp);
		cCompress = GetLong(fp);
		/*
		 * We've read 20 bytes so far, need to read 20 more
		 * for the required total of 40.
		 */
		GetLong(fp);
		GetLong(fp);
		GetLong(fp);
		colors = GetLong(fp);
		GetLong(fp);

		break;

	    case 64:
		class = C_OS2_V2;

		cx = GetLong(fp);
		cy = GetLong(fp);
		cPlanes = GetShort(fp);
		cBitCount = GetShort(fp);
		cCompress = GetLong(fp);
		/*
		 * We've read 20 bytes so far, need to read 44 more
		 * for the required total of 64.
		 */
		GetLong(fp);
		GetLong(fp);
		GetLong(fp);
		colors = GetLong(fp);
		for (i = 0; i < 7; i++)
			GetLong(fp);
		break;

	    case 108:
		class = C_WIN_V4;

		cx = GetLong(fp);
		cy = GetLong(fp);
		cPlanes = GetShort(fp);
		cBitCount = GetShort(fp);
		cCompress = GetLong(fp);
		/*
		 * We've read 20 bytes so far, need to read 88 more
		 * for the required total of 108.
		 */
		GetLong(fp);
		GetLong(fp);
		GetLong(fp);
		colors = GetLong(fp);
		GetLong(fp);
		bmp->redmask = GetLong(fp);
		bmp->greenmask = GetLong(fp);
		bmp->bluemask = GetLong(fp);
		bmp->alphamask = GetLong(fp);
		for (i = 0; i < 13; i++)
			GetLong(fp);
		break;

	    case 124:
		class = C_WIN_V5;

		cx = GetLong(fp);
		cy = GetLong(fp);
		cPlanes = GetShort(fp);
		cBitCount = GetShort(fp);
		cCompress = GetLong(fp);
		/*
		 * We've read 20 bytes so far, need to read 104 more
		 * for the required total of 124.
		 */
		GetLong(fp);
		GetLong(fp);
		GetLong(fp);
		colors = GetLong(fp);
		GetLong(fp);
		bmp->redmask = GetLong(fp);
		bmp->greenmask = GetLong(fp);
		bmp->bluemask = GetLong(fp);
		bmp->alphamask = GetLong(fp);
		for (i = 0; i < 17; i++)
			GetLong(fp);
		break;

	    default:
#ifndef DISABLE_TRACE
		if (srcTrace || reportBugs)
			fprintf(stderr, "BMP: unknown cbFix: %d\n", cbFix);
#endif
		return 0;
	}

	if (cPlanes != 1) {
#ifndef DISABLE_TRACE
		if (srcTrace || reportBugs)
			fprintf(stderr,
				"BMP: don't know how to handle cPlanes = %d\n",
				cPlanes);
#endif
		return 0;
	}

#ifndef DISABLE_TRACE
	if (srcTrace) {
		switch (class) {
		    case C_WIN_V3:
			fprintf(stderr, "Windows BMP V3");
			break;
		    case C_WIN_V4:
			fprintf(stderr, "Windows BMP V4");
			break;
		    case C_WIN_V5:
			fprintf(stderr, "Windows BMP V5");
			break;
		    case C_OS2:
			fprintf(stderr,	"OS/2 V1 or Windows V2 BMP");
			break;
		    case C_OS2_V2:
			fprintf(stderr, "OS/2 BMP V2");
			break;
		}
		fprintf(stderr, ", %dx%dx%d\n", cx, cy,	cBitCount);
		if (class != C_OS2)
			fprintf(stderr, "compression = %d colors = %d\n",
				cCompress, colors);
	}
#endif
	bmp->cx = cx;
	bmp->cy = cy;
	bmp->cBitCount = cBitCount;
	bmp->class = class;
	bmp->colors = colors;
	bmp->comp = cCompress;

	bmp->pos += cbFix;

	return 1;
}

/*
 * Returns the number of colors read.
 */
static int BMPreadrgbtable(bmpInfo *bmp, pixval *R, pixval *G, pixval *B)
{
	FILE *fp = bmp->fp;
	int i;
	int nbyte = 0;
	long ncolors = bmp->colors ? bmp->colors : (1 << bmp->cBitCount);

	for (i = 0; i < ncolors; i++) {
		if (i < MAXCOLORS) {
			B[i] = GetByte(fp);
			G[i] = GetByte(fp);
			R[i] = GetByte(fp);
		} else {
			(void) GetByte(fp);
			(void) GetByte(fp);
			(void) GetByte(fp);
		}
		nbyte += 3;

		if (bmp->class != C_OS2) {
			GetByte(fp);
			nbyte++;
		}
	}
	bmp->pos += nbyte;
	return ncolors;
}

/*
 * Returns the number of bytes read, or -1 on error.
 */
static int BMPreadrow(bmpInfo *bmp, unsigned char *row)
{
	FILE           *fp = bmp->fp;
	unsigned long   cx = bmp->cx;
	unsigned short  cBitCount = bmp->cBitCount;
	unsigned long   nbyte = 0;
	unsigned long   x;

	if (cBitCount == 32) {
		int v, y;

		for (x = 0; x < cx; x++) {
			for (y = 2; y >= 0; y--) {
				if ((v = fgetc(fp)) == EOF)
					return -1;
				row[y] = (unsigned char) v;
			}
			/* Skip the Alpha */
			if ((v = fgetc(fp)) == EOF)
				return -1;
			row += 3;
			nbyte += 4;
		}
	} else if (cBitCount == 24) {
		int v, y;

		for (x = 0; x < cx; x++) {
			for (y = 2; y >= 0; y--) {
				if ((v = fgetc(fp)) == EOF)
					return -1;
				row[y] = (unsigned char) v;
			}
			row += 3;
			nbyte += 3;
		}
	} else if (cBitCount == 16) {
		unsigned short v;

		for (x = 0; x < cx; x++) {
			v = GetShort(fp);
			row[2] = c5to8bits[v & 0x001f];
			row[1] = c5to8bits[(v >> 5) & 0x001f];
			row[0] = c5to8bits[(v >> 10) & 0x001f];
			row += 3;
			nbyte += 2;
		}
	} else if (cBitCount == 8) {
		int v;

		for (x = 0; x < cx; x++) {
			if ((v = fgetc(fp)) == EOF)
				return -1;
			nbyte++;
			*row++ = (unsigned char) v;
		}
	} else {
		BITSTREAM b;
		int rc;
		unsigned long v;

		if ((b = pm_bitinit(fp, "r")) == (BITSTREAM) 0)
			return -1;

		for (x = 0; x < cx; x++) {
			if ((rc = pm_bitread(b, cBitCount, &v)) == -1)
				return -1;
			nbyte += rc;
			*row++ = (unsigned char) v;
		}
		if (rc = pm_bitfini(b))
			return -1;
	}
	/*
	 * Make sure we read a multiple of 4 bytes.
	 */
	while (nbyte % 4) {
		GetByte(fp);
		nbyte++;
	}

	bmp->pos += nbyte;
	return nbyte;
}

static unsigned char *BMPreadbits(bmpInfo *bmp)
{
	unsigned char  *image, *ptr;
	unsigned long   cx = bmp->cx;
	unsigned long   cy = bmp->cy;
	long            y;
	int		rc, components;

	readto(bmp, bmp->offBits);

	if (bmp->cBitCount >= 16) {
		components = 3;
	} else {
		components = 1;
	}

	if (!(image = (unsigned char *)malloc(cx * cy * components)))
		return NULL;
	/*
	 * The picture is stored bottom line first, top line last
	 */
	for (y = cy - 1; y >= 0; y--) {
		ptr = image + (y * cx * components);
		rc = BMPreadrow(bmp, ptr);

		if (rc == -1) {
#ifndef DISABLE_TRACE
			if (srcTrace || reportBugs)
				fprintf(stderr,
					"BMP: couldn't read row %d\n", y);
#endif
			/* Display what we got */
			break;
		}
		if (rc % 4) {
#ifndef DISABLE_TRACE
			if (srcTrace || reportBugs)
				fprintf(stderr,
					"BMP: row had bad # of bytes: %d\n",rc);
#endif
			return NULL;
		}
	}
	return image;
}


static unsigned char *BMPreadcomp(bmpInfo *bmp, unsigned char **alpha_channel)
{
	FILE 	       *fp = bmp->fp;
	unsigned long   cx = bmp->cx;
	unsigned long   cy = bmp->cy;
	unsigned char  *image, *ptr;
	long            x, y;
	int		comp = bmp->comp;
	int             components, i, b1, b2, b3;

	if (bmp->cBitCount >= 16) {
		components = 3;
	} else {
		components = 1;
	}

	if (!(image = (unsigned char *)malloc(cx * cy * components)))
		return NULL;

	if ((comp == BI_RLE8) || (comp == BI_RLE4)) {
		y = x = 0;
		ptr = image + ((cy - 1) * cx);
		readto(bmp, bmp->offBits);

		while (y < cy) {
			if ((b1 = fgetc(fp)) == EOF)
				break;
			if (b1) {
				/* Encoded mode */
				b2 = fgetc(fp);
				for (i = 0; i < b1; i++, x++) {
					if (comp == BI_RLE8) {
					    *ptr++ = b2;
					} else {
					    *ptr++ = (i & 1) ? (b2 & 0x0f) :
						     ((b2 >> 4) & 0x0f);
					}
				}
			} else {
				/* An escape */
				b2 = fgetc(fp);
				switch (b2) {
				    case 0: {
					/* End of line */
					x = 0;
					y++;
					ptr = image + ((cy - y - 1) * cx);
					break;
				    }
				    case 1: {
					/* End of bitmap */
					return image;
				    }
				    case 2: {
					/* Delta offset */
					x += fgetc(fp);
					y += fgetc(fp);
					ptr = image + x + ((cy - y - 1) * cx);
					break;
				    }
				    default: {
					/* Absolute mode */
					for (i = 0; i < b2 ; i++, x++) {
					    if (comp == BI_RLE8) {
						*ptr++ = fgetc(fp);
					    } else {
						if ((i & 1) == 0)
						    b3 = fgetc(fp);
						*ptr++ = (i & 1) ? (b3 & 0x0f) :
							 ((b3 >> 4) & 0x0f);
					    }
					}
					if (comp == BI_RLE8) {
					    if (b2 & 1)
						/* Skip pad byte */
						fgetc(fp);
					} else {
					    if (((b2 & 0x03) == 1) ||
						((b2 & 0x03) == 2))
						fgetc(fp);
					}
				    }
				}
			}
		}
	} else if ((comp == BI_BITFIELDS) && (bmp->cBitCount == 16)) {
		unsigned short v;
		unsigned long red, green, blue;
		int redshift, greenshift, blueshift;
		int rlen, glen, blen;

		if ((bmp->class != C_WIN_V4) && (bmp->class != C_WIN_V5)) {
			/* Get RGB mask from color table field */
			red = GetLong(fp);
			green = GetLong(fp);
			blue = GetLong(fp);
			bmp->pos += 12;
		} else {
			red = bmp->redmask;
			green = bmp->greenmask;
			blue = bmp->bluemask;
		}
		redshift = GetShift(red, &rlen);
		greenshift = GetShift(green, &glen);
		blueshift = GetShift(blue, &blen);
#ifndef DISABLE_TRACE
		if (srcTrace)
		       fprintf(stderr, "BitField = %d%d%d\n", rlen, glen, blen);
#endif
		readto(bmp, bmp->offBits);

		for (y = cy - 1; y >= 0; y--) {
			ptr = image + (y * cx * components);

			for (x = 0; x < cx; x++) {
				v = GetShort(fp);
				if (blueshift >= 0) {
					ptr[2] = (unsigned char)
						 ((v & blue) << blueshift);
				} else {
					ptr[2] = (unsigned char)
						 ((v & blue) >> (-blueshift));
				}
				if (greenshift >= 0) {
					ptr[1] = (unsigned char)
						 ((v & green) << greenshift);
				} else {
					ptr[1] = (unsigned char)
						 ((v & green) >> (-greenshift));
				}
				if (redshift >= 0) {
					ptr[0] = (unsigned char)
						 ((v & red) << redshift);
				} else {
					ptr[0] = (unsigned char)
						 ((v & red) >> (-redshift));
				}
				ptr += 3;
			}
			/* Skip padding */
			x = x * 2;
			while (x++ % 4)
				GetByte(fp);
		}
	} else if ((comp == BI_BITFIELDS) && (bmp->cBitCount == 32)) {
		unsigned long v, red, green, blue, alpha;
		unsigned char *a, *aptr;
		int redshift, greenshift, blueshift, alphashift;
		int rlen, glen, blen, alen;
		int has_NZ_alpha = 0;
		int has_N255_alpha = 0;

		if ((bmp->class != C_WIN_V4) && (bmp->class != C_WIN_V5)) {
			/* Get RGB mask from color table field */
			red = GetLong(fp);
			green = GetLong(fp);
			blue = GetLong(fp);
			alpha = 0;
			bmp->pos += 12;
		} else {
			red = bmp->redmask;
			green = bmp->greenmask;
			blue = bmp->bluemask;
			alpha = bmp->alphamask;
		}

		redshift = GetShift(red, &rlen);
		greenshift = GetShift(green, &glen);
		blueshift = GetShift(blue, &blen);
		if (alpha) {
			alphashift = GetShift(alpha, &alen);
			*alpha_channel = a = malloc(bmp->cx * bmp->cy);
		}
#ifndef DISABLE_TRACE
		if (srcTrace) {
			if (alpha) {
				fprintf(stderr,	"BitField = %d%d%d%d\n",
					rlen, glen, blen, alen);
			} else {
				fprintf(stderr, "BitField = %d%d%d\n",
					rlen, glen, blen);
			}
		}
#endif
		readto(bmp, bmp->offBits);

		for (y = cy - 1; y >= 0; y--) {
			ptr = image + (y * cx * components);
			aptr = a + (y * cx);

			for (x = 0; x < cx; x++) {
				v = GetLong(fp);
				if (blueshift >= 0) {
					ptr[2] = (unsigned char)
						 ((v & blue) << blueshift);
				} else {
					ptr[2] = (unsigned char)
						 ((v & blue) >> (-blueshift));
				}
				if (greenshift >= 0) {
					ptr[1] = (unsigned char)
						 ((v & green) << greenshift);
				} else {
					ptr[1] = (unsigned char)
						 ((v & green) >> (-greenshift));
				}
				if (redshift >= 0) {
					ptr[0] = (unsigned char)
						 ((v & red) << redshift);
				} else {
					ptr[0] = (unsigned char)
						 ((v & red) >> (-redshift));
				}
				if (alpha) {
					if (alphashift >= 0) {
						*aptr = (unsigned char)
						 ((v & alpha) << alphashift);
					} else {
						*aptr = (unsigned char)
						 ((v & alpha) >> (-alphashift));
					}
					if (*aptr)
					        has_NZ_alpha = 1;
					if (*aptr++ != 255)
						has_N255_alpha = 1;
				}
				ptr += 3;
			}
			/* Skip padding */
			x = x * 4;
			while (x++ % 4)
				GetByte(fp);
		}
		/* Not an alpha if all zero or all 255 */
		if (alpha && !(has_NZ_alpha && has_N255_alpha)) {
			free(a);
			*alpha_channel = NULL;
		} else if (alpha) {
#ifndef DISABLE_TRACE
			if (srcTrace)
				fprintf(stderr,	"BMP: Got Alpha Channel\n");
#endif
		}
	} else {
#ifndef DISABLE_TRACE
		if (srcTrace || reportBugs)
			fprintf(stderr,	"BMP: Unknown compression: %d\n", comp);
#endif
		return NULL;
	}
	return image;
}


unsigned char *ReadBMP(FILE *ifp, int *width, int *height, XColor *colrs,
		       unsigned char **alpha)
{
	unsigned char *image;
	bmpInfo *bmp = (bmpInfo *)malloc(sizeof(bmpInfo));

	if (!bmp)
		return NULL;

	bmp->fp = ifp;
	bmp->pos = 0;

	if (!BMPreadfileheader(bmp) || !BMPreadinfoheader(bmp))
		goto ErrReturn;

	*width = bmp->cx;
	*height = bmp->cy;

#ifndef DISABLE_TRACE
	if ((bmp->offBits != BMPoffbits(bmp)) && (srcTrace || reportBugs))
		fprintf(stderr, "BMP: offBits is %d, expected %d\n",
			bmp->offBits, BMPoffbits(bmp));
#endif
	if (bmp->cBitCount <= 8) {
		pixval R[MAXCOLORS];	/* Reds */
		pixval G[MAXCOLORS];	/* Greens */
		pixval B[MAXCOLORS];	/* Blues */
		int ncolors, i;

		/* 16, 24 and 32 bit images do not have color maps */
		ncolors = BMPreadrgbtable(bmp, R, G, B);
		for (i = 0; i < ncolors; i++) {
			colrs[i].red = R[i] << 8;
			colrs[i].green = G[i] << 8;
			colrs[i].blue = B[i] << 8;
			colrs[i].pixel = i;
			colrs[i].flags = DoRed | DoGreen | DoBlue;
		}
	}
	if (bmp->comp == BI_RGB) {
		image = BMPreadbits(bmp);
	} else {
		image = BMPreadcomp(bmp, alpha);
	}

#ifndef DISABLE_TRACE
	if ((bmp->comp == BI_RGB) && (bmp->pos != BMPlenfile(bmp)) &&
	    (srcTrace || reportBugs))
		fprintf(stderr,	"BMP: read %d bytes, expected %d bytes\n",
			bmp->pos, BMPlenfile(bmp));
#endif
	/* Convert 3 component (RGB) data to 8 bit colormap data */
	if ((bmp->cBitCount >= 16) && image) {
		unsigned char *newimage;

		/* Use MedianCut if image has <= 256 colors. */
		newimage = MedianCut24BitTo8(image, bmp->cx, bmp->cy, colrs,
					     256, 256);
		if (newimage) {
			free(image);
			free(bmp);
			return newimage;
		}
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "BMP: Quantize 24 bit image\n");
#endif
		newimage = QuantizeImage(image, bmp->cx, bmp->cy, 256,
					 1, colrs, 0);
		if (newimage) {
			free(image);
			image = newimage;
		} else {
#ifndef DISABLE_TRACE
			if (reportBugs)
				fprintf(stderr,
					"BMP: Failed converting RGB image");
#endif
			free(image);
			goto ErrReturn;
		}
	}
	free(bmp);
	return image;

 ErrReturn:
	free(bmp);
	return NULL;
}
