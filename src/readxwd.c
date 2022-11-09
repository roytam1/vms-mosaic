/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                   DDDD   EEEEE   CCCC   OOO   DDDD   EEEEE                  %
%                   D   D  E      C      O   O  D   D  E                      %
%                   D   D  EEE    C      O   O  D   D  EEE                    %
%                   D   D  E      C      O   O  D   D  E                      %
%                   DDDD   EEEEE   CCCC   OOO   DDDD   EEEEE                  %
%                                                                             %
%                                                                             %
%                    Utility Routines to Read Image Formats                   %
%                                                                             %
%                                                                             %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                              January 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1994 E. I. du Pont de Nemours & Company                          %
%                                                                             %
%  Permission to use, copy, modify, distribute, and sell this software and    %
%  its documentation for any purpose is hereby granted without fee,           %
%  provided that the above Copyright notice appear in all copies and that     %
%  both that Copyright notice and this permission notice appear in            %
%  supporting documentation, and that the name of E. I. du Pont de Nemours    %
%  & Company not be used in advertising or publicity pertaining to            %
%  distribution of the software without specific, written prior               %
%  permission.  E. I. du Pont de Nemours & Company makes no representations   %
%  about the suitability of this software for any purpose.  It is provided    %
%  "as is" without express or implied warranty.                               %
%                                                                             %
%  E. I. du Pont de Nemours & Company disclaims all warranties with regard    %
%  to this software, including all implied warranties of merchantability      %
%  and fitness, in no event shall E. I. du Pont de Nemours & Company be       %
%  liable for any special, indirect or consequential damages or any           %
%  damages whatsoever resulting from loss of use, data or profits, whether    %
%  in an action of contract, negligence or other tortuous action, arising     %
%  out of or in connection with the use or performance of this software.      %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/* Modified for use with VMS Mosaic by GEC - 23-Oct-2006 */

#include "../config.h"
#define XLIB_ILLEGAL_ACCESS  1		/* Get access to Display struct */
#include "mosaic.h"
#include "XWDFile.h"
#include "readxwd.h"
#include "quantize.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int reportBugs;
#endif

#define DirectClass  1
#define PseudoClass  2

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  M S B F i r s t O r d e r L o n g                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function MSBFirstOrderLong converts a least-significant byte first buffer
%  of integers to most-significant byte first.
%
%  The format of the MSBFirstOrderLong routine is:
%
%       MSBFirstOrderLong(p,length);
%
%  A description of each parameter follows.
%
%   o  p:  Specifies a pointer to a buffer of integers.
%
%   o  length:  Specifies the length of the buffer.
%
%
*/
static void MSBFirstOrderLong(register char *p, register unsigned int length)
{
  register char c;
  register char *q, *sp;

  q = p + length;
  while (p < q) {
    sp = p + 3;
    c = *sp;
    *sp = *p;
    *p++ = c;
    sp = p + 1;
    c = *sp;
    *sp = *p;
    *p = c;
    p += 3;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  M S B F i r s t O r d e r S h o r t                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function MSBFirstOrderShort converts a least-significant byte first buffer
%  of integers to most-significant byte first.
%
%  The format of the MSBFirstOrderShort routine is:
%
%       MSBFirstOrderLongShort(p,length);
%
%  A description of each parameter follows.
%
%   o  p:  Specifies a pointer to a buffer of integers.
%
%   o  length:  Specifies the length of the buffer.
%
%
*/
static void MSBFirstOrderShort(register char *p, register unsigned int length)
{
  register char c;
  register char *q;

  q = p + length;
  while (p < q) {
    c = *p;
    *p = *(p + 1);
    p++;
    *p++ = c;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d X W D I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadXWDImage reads an X Window System window dump image file and
%  returns it.  It allocates the memory necessary for the new image data
%  and returns a pointer to the new image.
%
*/
static char *ReadXWDImage(FILE *fp, int *w, int *h, Boolean *cmap,
			  XColor *colors)
{
  char *window_name;
  Display display;
  char *image = NULL;
  char *q;
  int class, x, y, width, height;
  register int i;
  register unsigned long pixel;
  unsigned long lsb_first, packets;
  ScreenFormat screen_format;
  XImage *ximage;
  XWDFileHeader header;

  /*
     Read in header information.
  */
  if (fread((void *)&header, 1, sizeof(header), fp) < sizeof(header)) {
#ifndef DISABLE_TRACE
      if (srcTrace)
          fprintf(stderr, "Unable to read XWD header\n");
#endif
      return(NULL);
  }
  /*
    Ensure the header byte-order is most-significant byte first.
  */
  lsb_first = 1;
  if (*(char *)&lsb_first)
      MSBFirstOrderLong((char *) &header, sizeof(header));
  /*
    Check to see if the dump file is in the proper format.
  */
  if (header.file_version != XWD_FILE_VERSION) {
#ifndef DISABLE_TRACE
      if (srcTrace || reportBugs)
          fprintf(stderr, "XWD file format version mismatch\n");
#endif
      return(NULL);
  }
  if (header.header_size < sizeof(header)) {
#ifndef DISABLE_TRACE
      if (srcTrace)
          fprintf(stderr, "XWD header size is too small\n");
#endif
      return(NULL);
  }
  packets = header.header_size - sizeof(header);
  window_name = (char *) malloc((unsigned int) packets * sizeof(char));
  if (!window_name) {
#ifndef DISABLE_TRACE
      if (srcTrace || reportBugs)
          fprintf(stderr, "XWD: Unable to allocate memory\n");
#endif
      return(NULL);
  }
  if (fread((void *) window_name, 1, packets, fp) < packets) {
#ifndef DISABLE_TRACE
      if (srcTrace)
          fprintf(stderr, "Unable to read window name from XWD file\n");
#endif
      return(NULL);
  }
  free(window_name);

  /*
    Initialize the X image.
  */
  display.byte_order = header.byte_order;
  display.bitmap_unit = header.bitmap_unit;
  display.bitmap_bit_order = header.bitmap_bit_order;
  display.pixmap_format = &screen_format;
  display.nformats = 1;
  screen_format.depth = header.pixmap_depth;
  screen_format.bits_per_pixel = (int) header.bits_per_pixel;
  ximage = XCreateImage(&display, (Visual *) NULL,
                 (unsigned int) header.pixmap_depth, (int) header.pixmap_format,
                 (int) header.xoffset, NULL, (unsigned int) header.pixmap_width,
                 (unsigned int) header.pixmap_height, (int) header.bitmap_pad,
                 (int) header.bytes_per_line);
  ximage->red_mask = header.red_mask;
  ximage->green_mask = header.green_mask;
  ximage->blue_mask = header.blue_mask;

  /*
    Read colormap.
  */
  if (header.ncolors) {
#ifndef DISABLE_TRACE
      if (srcTrace)
          fprintf(stderr, "XWD: image has color map with %d colors\n",
	          header.ncolors);
#endif
      if (header.ncolors <= 256) {
          if (fread((void *) colors, sizeof(XColor), header.ncolors, fp) <
	      header.ncolors) {
#ifndef DISABLE_TRACE
              if (srcTrace)
                  fprintf(stderr, "Unable to read color map from XWD file\n");
#endif
              goto error;
          }
          /*
            Ensure the header byte-order is most-significant byte first.
          */
          lsb_first = 1;
          if (*(char *)&lsb_first) {
              for (i = 0; i < header.ncolors; i++) {
                  MSBFirstOrderLong((char *) &colors[i].pixel,
				    sizeof(unsigned long));
                  MSBFirstOrderShort((char *) &colors[i].red,
			             3 * sizeof(unsigned short));
              }
          }
      } else {
          header.ncolors = 0;
      }
  }

  /*
    Allocate the pixel buffer.
  */
  if (ximage->format == ZPixmap) {
#ifndef DISABLE_TRACE
      if (srcTrace)
          fprintf(stderr, "XWD: image is format ZPixmap\n");
#endif
      packets = ximage->bytes_per_line * ximage->height;
  } else {
      packets = ximage->bytes_per_line * ximage->height * ximage->depth;
  }
  ximage->data = (char *) malloc(packets * sizeof(unsigned char));
  if (!ximage->data) {
#ifndef DISABLE_TRACE
      if (srcTrace || reportBugs)
          fprintf(stderr, "XWD: Unable to allocate Ximage memory\n");
#endif
      goto error;
  }
  if (fread(ximage->data, 1, packets, fp) < packets) {
#ifndef DISABLE_TRACE
      if (srcTrace)
          fprintf(stderr, "Unable to read XWD pixmap\n");
#endif
      goto error;
  }

  /*
    Get image
  */
  *w = width = ximage->width;
  *h = height = ximage->height;

  if ((ximage->red_mask > 0) || (ximage->green_mask > 0) ||
      (ximage->blue_mask > 0)) {
      class = DirectClass;
      *cmap = 0;
#ifndef DISABLE_TRACE
      if (srcTrace)
          fprintf(stderr, "XWD: Image is 24-bit\n");
#endif
  } else {
      class = PseudoClass;
      *cmap = 1;
#ifndef DISABLE_TRACE
      if (srcTrace)
          fprintf(stderr, "XWD: Image is 8-bit\n");
#endif
  }
  switch (class) {
    case DirectClass: {
      unsigned long blue_mask, red_mask, green_mask;
      unsigned long green_shift = 0, blue_shift = 0, red_shift = 0;

      q = image = malloc(width * height * 3);
      if (!image) {
#ifndef DISABLE_TRACE
          if (srcTrace || reportBugs)
              fprintf(stderr, "XWD: Unable to allocate image memory\n");
#endif
          goto error;
      }
      /*
        Determine shift and mask for red, green, and blue.
      */
      red_mask = ximage->red_mask;
      while ((red_mask & 0x01) == 0) {
          red_mask >>= 1;
          red_shift++;
      }
      green_mask = ximage->green_mask;
      while ((green_mask & 0x01) == 0) {
          green_mask >>= 1;
          green_shift++;
      }
      blue_mask = ximage->blue_mask;
      while ((blue_mask & 0x01) == 0) {
          blue_mask >>= 1;
          blue_shift++;
      }
      /*
        Convert X image to DirectClass packets.
      */
      if (header.ncolors) {
	  register unsigned short index;

          for (y = 0; y < height; y++) {
              for (x = 0; x < width; x++) {
                  pixel = XGetPixel(ximage, x, y);
                  index = (unsigned short) ((pixel >> red_shift) & red_mask);
                  *q++ = (unsigned char) (colors[index].red >> 8);
                  index = (unsigned short) ((pixel >> green_shift) &green_mask);
                  *q++ = (unsigned char) (colors[index].green >> 8);
                  index = (unsigned short) ((pixel >> blue_shift) & blue_mask);
                  *q++ = (unsigned char) (colors[index].blue >> 8);
              }
          }
      } else {
	  register unsigned long color;

          for (y = 0; y < height; y++) {
              for (x = 0; x < width; x++) {
                  pixel = XGetPixel(ximage, x, y);
                  color = (pixel >> red_shift) & red_mask;
                  *q++ = (unsigned char)
                          ((((unsigned long) color * 65535) / red_mask) >> 8);
                  color = (pixel >> green_shift) & green_mask;
                  *q++ = (unsigned char)
                          ((((unsigned long) color * 65535) / green_mask) >> 8);
                  color = (pixel >> blue_shift) & blue_mask;
                  *q++ = (unsigned char)
                          ((((unsigned long) color * 65535) / blue_mask) >> 8);
              }
	  }
      }
      break;
    }
    case PseudoClass: {
      /*
        Convert X image to PseudoClass packets.
      */
      if (!header.ncolors) {
#ifndef DISABLE_TRACE
          if (srcTrace)
              fprintf(stderr, "Unsupported XWD image\n");
#endif
          goto error;
      }
      q = image = malloc(width * height);
      if (!image) {
#ifndef DISABLE_TRACE
          if (srcTrace || reportBugs)
              fprintf(stderr, "XWD: Unable to allocate 8-bit image memory\n");
#endif
          goto error;
      }
      for (y = 0; y < height; y++) {
          for (x = 0; x < width; x++)
              *q++ = (unsigned char) XGetPixel(ximage, x, y);
      }
      break;
    }
  }

 error:
  /*
    Free X image.
  */
  XDestroyImage(ximage);

  return(image);
}


unsigned char *ReadXWD(FILE *fp, int *width, int *height, XColor *colrs)
{
  unsigned char *image;
  Boolean cmap;

#ifndef DISABLE_TRACE
  if (srcTrace)
      fprintf(stderr, "ReadXWD: starting\n");
#endif

  image = (unsigned char *)ReadXWDImage(fp, width, height, &cmap, colrs);

  if (!cmap) {
      unsigned char *newimage;

      newimage = QuantizeImage(image, *width, *height, 256, 1, colrs, 0);
      free(image);
      return(newimage);
  }

  /* Already 8-bit with color map */
  return(image);
}
