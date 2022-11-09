/* sunraster.c:
 *
 * sun rasterfile image type
 *
 * jim frost 09.27.89
 *
 * Copyright 1989, 1991 Jim Frost.
*/
/*
 * Copyright 1989, 1990, 1991 Jim Frost
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  The author makes no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* Modified for use with VMS Mosaic by GEC - 20-Oct-2006 */

#include "../config.h"
#include "mosaic.h"
#include <X11/Xlib.h>
#include "readsun.h"
#include "quantize.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int reportBugs;
#endif

typedef unsigned char byte;

#define memToVal(PTR, LEN) (                                   		       \
(LEN) == 1 ? (unsigned long)(                 *( (byte *)(PTR))         ) :    \
(LEN) == 2 ? (unsigned long)(((unsigned long)(*( (byte *)(PTR))   ))<< 8)      \
                          + (                 *(((byte *)(PTR))+1)      ) :    \
(LEN) == 3 ? (unsigned long)(((unsigned long)(*( (byte *)(PTR))   ))<<16)      \
                          + (((unsigned long)(*(((byte *)(PTR))+1)))<< 8)      \
			  + (		      *(((byte *)(PTR))+2)	) :    \
             (unsigned long)(((unsigned long)(*( (byte *)(PTR))   ))<<24)      \
			  + (((unsigned long)(*(((byte *)(PTR))+1)))<<16)      \
			  + (((unsigned long)(*(((byte *)(PTR))+2)))<< 8)      \
			  + (                 *(((byte *)(PTR))+3)      ) )

struct rheader {
  unsigned char magic[4];    /* Magic number */
  unsigned char width[4];    /* Width of image in pixels */
  unsigned char height[4];   /* Height of image in pixels */
  unsigned char depth[4];    /* Depth of each pixel */
  unsigned char length[4];   /* Length of the image in bytes */
  unsigned char type[4];     /* Format of file */
  unsigned char maptype[4];  /* Type of colormap */
  unsigned char maplen[4];   /* Length of colormap in bytes */
};

/* Following the header is the colormap (unless maplen is zero) then
 * the image.  Each row of the image is rounded to 2 bytes.
 */

#define RMAGICNUMBER 0x59a66a95  /* Magic number of this file type */

/* These are the possible file formats
 */
#define ROLD       0  /* Old format, see /usr/include/rasterfile.h */
#define RSTANDARD  1  /* Standard format */
#define RRLENCODED 2  /* Run length encoding to compress the image */
#define RRGB       3  /* RGB-format instead of BGR in 24 or 32-bit mode */
#define RTIFF      4  /* TIFF <-> rasterfile */
#define RIFF       5  /* IFF (TAAC) <-> rasterfile */

/* These are the possible colormap types.  If it's in RGB format,
 * the map is made up of three byte arrays (red, green, then blue)
 * that are each 1/3 of the colormap length.
 */
#define RNOMAP  0  /* No colormap follows the header */
#define RRGBMAP 1  /* rgb colormap */
#define RRAWMAP 2  /* raw colormap; good luck */

#define RESC 128   /* run-length encoding escape character */


static void babble(struct rheader *header)
{
  fprintf(stderr, "sunLoad: Image is");
  switch (memToVal(header->type, 4)) {
    case ROLD:
      fprintf(stderr, " old-style");
      break;
    case RSTANDARD:
      fprintf(stderr, " standard");
      break;
    case RRLENCODED:
      fprintf(stderr, " run-length encoded");
      break;
    case RRGB:
      fprintf(stderr, " RGB");  /* RGB format instead of BGR */
      break;
    case RTIFF:
      fprintf(stderr, " TIFF");
      break;
    case RIFF:
      fprintf(stderr, " RIFF");
      break;
    default:
      fprintf(stderr, " unknown-type");
  }
  fprintf(stderr, " %dx%d", memToVal(header->width, 4),
	  memToVal(header->height, 4));

  switch (memToVal(header->depth, 4)) {
    case 1:
      fprintf(stderr, " monochrome");
      break;
    case 8:
      fprintf(stderr, " 8 plane %s",
	     memToVal(header->maplen, 4) > 0 ? "color" : "greyscale");
      break;
    case 24:
      fprintf(stderr, " 24 plane color");
      break;

    case 32:
      /* Isn't it nice how the sunraster.h file doesn't bother to mention
       * that 32-bit depths are allowed?
       */
      fprintf(stderr, " 32 plane color");
      break;
  }
  fprintf(stderr, " Sun rasterfile\n");
}

/* Read either rl-encoded or normal image data
 * Return TRUE if read was ok
 */
static Boolean sunread(FILE *fp, byte *buf, unsigned int len, unsigned int enc)
{
  static char repchar;
  static char remaining = '\0';

  /* rl-encoded read */
  if (enc) {
    while (len--)
      if (remaining) {
	remaining--;
	*buf++ = (unsigned char)repchar;
      } else {
	if ((repchar = fgetc(fp)) == EOF)
	  goto error;
	if ((unsigned char)repchar == RESC) {
	  if ((remaining = fgetc(fp)) == EOF)
	    goto error;
	  if (remaining == '\0') {
	    *buf++= RESC;
	  } else {
	    if ((repchar = fgetc(fp)) == EOF)
	      goto error;
	    *buf++= (unsigned char)repchar;
	  }
	} else {
	  *buf++= (unsigned char)repchar;
	}
      }
  } else {
    /* Normal read */
    if (fread((void *)buf, 1, len, fp) < len) {
#ifndef DISABLE_TRACE
      if (srcTrace || reportBugs)
        fprintf(stderr, "sunLoad: Bad read on standard image data\n");
#endif
      return FALSE;
    }
  }
  return TRUE;

 error:
#ifndef DISABLE_TRACE
  if (srcTrace || reportBugs)
    fprintf(stderr, "sunLoad: Bad read on encoded image data\n");
#endif
  return FALSE;
}

static byte *sunLoad(FILE *fp, int *w, int *h, Boolean *cmap, XColor *colrs)
{
  struct rheader  header;
  unsigned int    mapsize, depth, enc, pixlen;
  unsigned int    linelen;   /* Length of raster line in bytes */
  unsigned int    fill;      /* # of fill bytes per raster line */
  unsigned int    x, y, width, height;
  int		  z;
  byte            fillchar;
  byte           *map, *mapred, *mapgreen, *mapblue;
  byte           *image, *lineptr;

  switch (fread((void *)&header, 1, sizeof(struct rheader), fp)) {
    case sizeof(struct rheader):
      if (memToVal(header.magic, 4) != RMAGICNUMBER)
         return(NULL);
#ifndef DISABLE_TRACE
      if (srcTrace)
         babble(&header);
#endif
      break;
    case -1:
      perror("sunLoad");
    default:
      return(NULL);
  }

  *w = width = memToVal(header.width, 4);
  *h = height = memToVal(header.height, 4);
  depth = memToVal(header.depth, 4);

  switch(depth) {
    case 1:
    case 8:
      pixlen = 1;
      break;
    case 24:
    case 32:
      pixlen = 3;
      break;
    default:
#ifndef DISABLE_TRACE
      if (srcTrace || reportBugs)
        fprintf(stderr, "sunLoad: Bad depth %d (1, 8, 24 and 32 are valid)\n",
	        depth);
#endif
      return NULL;
  }

  image = malloc(width * height * pixlen);

  /*
   *  Handle color map...
   */
  if (mapsize = memToVal(header.maplen, 4)) {
    map = malloc(mapsize);
    if (fread((void *)map, 1, mapsize, fp) < mapsize) {
#ifndef DISABLE_TRACE
      if (srcTrace || reportBugs)
        fprintf(stderr, "sunLoad: Bad read on colormap\n");
#endif
      free(map);
      free(image);
      return NULL;
    }
    mapsize /= 3;
    if (depth > 8) {
#ifndef DISABLE_TRACE
      if (srcTrace || reportBugs)
        fprintf(stderr, "sunLoad: True color image colormap being ignored\n");
#endif
      *cmap = 0;
    } else if (depth == 8) {	/* 8-bit indexed */
      if (mapsize > 256) {
#ifndef DISABLE_TRACE
        if (srcTrace || reportBugs)
          fprintf(stderr, "sunLoad: Colormap is too big for depth\n");
#endif
        mapsize = 256;
      }
      mapred = map;
      mapgreen = mapred + mapsize;
      mapblue = mapgreen + mapsize;
      for (y = 0; y < mapsize; y++) {
        colrs[y].red  = *mapred++ << 8;
        colrs[y].green = *mapgreen++ << 8;
        colrs[y].blue = *mapblue++ << 8;
      }
      *cmap = 1;
    } else {
      /* Monochrome */
      colrs[0].red = colrs[0].green = colrs[0].blue = 65535;
      colrs[1].red = colrs[1].green = colrs[1].blue = 0;
      *cmap = 1;
    }
    free(map);

  /*
   *  Handle 8-bit greyscale via a simple ramp function...
   */
  } else if (depth == 8) {
    for (mapsize = 256, y = 0; y < mapsize; y++)
      colrs[y].red = colrs[y].green = colrs[y].blue = y << 8;
    *cmap = 1;

  /* Monochrome */
  } else if (depth == 1) {
    colrs[0].red = colrs[0].green = colrs[0].blue = 65535;
    colrs[1].red = colrs[1].green = colrs[1].blue = 0;
    *cmap = 1;

  /*
   * 24-bit and 32-bit handle themselves.  Currently we don't support
   * a colormap for them.
   */
  } else {
    *cmap = 0;
  }

  enc = (memToVal(header.type, 4) == RRLENCODED);
  lineptr = image;

  /* If it's a 32-bit image, we read the line and then strip off the
   * top byte of each pixel to get truecolor format
   */
  if (depth >= 24) {
    byte *buf, *bp;

    linelen = width * (depth == 24 ? 3 : 4);
    fill = linelen % 2 ? 1 : 0;
    buf = malloc(linelen);
    for (y = 0; y < height; y++) {
      if (!sunread(fp, buf, linelen, enc)) {
        free(buf);
        return(image);
      }
      bp = buf;
      if (memToVal(header.type, 4) != RRGB) {
        if (depth == 24) {
          for (x = 0; x < width; x++) {
            *lineptr++ = *(bp + 2);  /* Red */
            *lineptr++ = *(bp + 1);  /* Green */
            *lineptr++ = *bp;        /* Blue */
            bp += 3;
          }
        } else {
          /* 32-bit */
          for (x = 0; x < width; x++) {
            *lineptr++ = *(bp + 3);  /* Red */
            *lineptr++ = *(bp + 2);  /* Green */
            *lineptr++ = *(bp + 1);  /* Blue */
            bp += 4;
          }
        }
      } else {	/* RGB */
        if (depth == 24) {
          for (x = 0; x < width; x++) {
            *lineptr++ = *bp++;      /* Red */
            *lineptr++ = *bp++;      /* Green */
            *lineptr++ = *bp++;      /* Blue */
          }
        } else {
          /* 32-bit */
          for (x = 0; x < width; x++) {
            *lineptr++ = *bp++;      /* Red */
            *lineptr++ = *bp++;      /* Green */
            *lineptr++ = *bp++;      /* Blue */
            bp++;
          }
        }
      }
      if (fill && !sunread(fp, &fillchar, fill, enc)) {
        free(buf);
        return(image);
      }
    }
    free(buf);
  } else if (depth == 8) {
    linelen = width;
    fill = linelen % 2 ? 1 : 0;
    for (y = 0; y < height; y++) {
      if (!sunread(fp, lineptr, linelen, enc))
        return(image);
      lineptr += linelen;
      if (fill && !sunread(fp, &fillchar, fill, enc))
        return(image);
    }
  } else {
    byte *bp, *buf;

    /* Monochrome */
    linelen = (width / 8) + (width % 8 ? 1 : 0);
    fill = linelen % 2 ? 1 : 0;
    buf = malloc(linelen);
    for (y = 0; y < height; y++) {
      if (!sunread(fp, buf, linelen, enc)) {
	free(buf);
        return(image);
      }
      bp = buf;
      for (x = 0; x < linelen; x++) {
	for (z = 7; z >= 0; z--)
	  *lineptr++ = (*bp >> z) & 1;  
        bp++;
      }
      if (fill && !sunread(fp, &fillchar, fill, enc)) {
	free(buf);
        return(image);
      }
    }
    free(buf);
  }
  return(image);
}

unsigned char *ReadSUN(FILE *fp, int *width, int *height, XColor *colrs)
{
  unsigned char *image, *newimage;
  Boolean cmap;

  image = sunLoad(fp, width, height, &cmap, colrs);

  if (!cmap) {
     newimage = QuantizeImage(image, *width, *height, 256, 1, colrs, 0);
     free(image);
     return(newimage);
  }

  /* Already 8-bit with color map */
  return(image);
}
