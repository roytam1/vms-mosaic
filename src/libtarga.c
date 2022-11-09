/**************************************************************************
 ** Simplified TARGA library for Intro to Graphics Classes
 **
 ** This is a simple library for reading and writing image files in
 ** the TARGA file format (which is a simple format). 
 ** The routines are intentionally designed to be simple for use in
 ** into to graphics assignments - a more full-featured targa library
 ** also exists for other uses.
 **
 ** This library was originally written by Alex Mohr who has assigned
 ** copyright to Michael Gleicher. The code is made available under an
 ** "MIT" Open Source license.
 **/

/**
 ** Copyright (c) 2005 Michael L. Gleicher
 **
 ** Permission is hereby granted, free of charge, to any person
 ** obtaining a copy of this software and associated documentation
 ** files (the "Software"), to deal in the Software without
 ** restriction, including without limitation the rights to use, copy,
 ** modify, merge, publish, distribute, sublicense, and/or sell copies
 ** of the Software, and to permit persons to whom the Software is
 ** furnished to do so, subject to the following conditions:
 ** 
 ** The above copyright notice and this permission notice shall be
 ** included in all copies or substantial portions of the Software.
 **
 ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 ** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 ** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 ** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 ** HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 ** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 ** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 ** DEALINGS IN THE SOFTWARE.
 **/

/*
** libtarga.c -- routines for reading targa files.
*/

/*
  Modified by yu-chi because of initialization of variables at tga_load
  09-16-2005
*/

/* Modified for use with VMS Mosaic - 11-Oct-2006 */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>

#include "libtarga.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

#define TGA_IMG_NODATA           (0)
#define TGA_IMG_UNC_PALETTED     (1)
#define TGA_IMG_UNC_TRUECOLOR    (2)
#define TGA_IMG_UNC_GRAYSCALE    (3)
#define TGA_IMG_RLE_PALETTED     (9)
#define TGA_IMG_RLE_TRUECOLOR    (10)
#define TGA_IMG_RLE_GRAYSCALE    (11)

#define TGA_LOWER_LEFT           (0)
#define TGA_LOWER_RIGHT          (1)
#define TGA_UPPER_LEFT           (2)
#define TGA_UPPER_RIGHT          (3)

#define HDR_LENGTH               (18)
#define HDR_IDLEN                (0)
#define HDR_CMAP_TYPE            (1)
#define HDR_IMAGE_TYPE           (2)
#define HDR_CMAP_FIRST           (3)
#define HDR_CMAP_LENGTH          (5)
#define HDR_CMAP_ENTRY_SIZE      (7)
#define HDR_IMG_SPEC_XORIGIN     (8)
#define HDR_IMG_SPEC_YORIGIN     (10)
#define HDR_IMG_SPEC_WIDTH       (12)
#define HDR_IMG_SPEC_HEIGHT      (14)
#define HDR_IMG_SPEC_PIX_DEPTH   (16)
#define HDR_IMG_SPEC_IMG_DESC    (17)

#define TGA_ERR_NONE                    (0)
#define TGA_ERR_BAD_HEADER              (1)
#define TGA_ERR_OPEN_FAILS              (2)
#define TGA_ERR_BAD_FORMAT              (3)
#define TGA_ERR_UNEXPECTED_EOF          (4)
#define TGA_ERR_NODATA_IMAGE            (5)
#define TGA_ERR_COLORMAP_FOR_GRAY       (6)
#define TGA_ERR_BAD_COLORMAP_ENTRY_SIZE (7)
#define TGA_ERR_BAD_COLORMAP            (8)
#define TGA_ERR_READ_FAILS              (9)
#define TGA_ERR_BAD_IMAGE_TYPE          (10)
#define TGA_ERR_BAD_DIMENSIONS          (11)

/* For expanding 5-bit pixel values to 8-bit with best rounding */
static const ubyte c5to8bits[32] = {
    0,   8,  16,  25,  33,  41,  49,  58,
   66,  74,  82,  90,  99, 107, 115, 123,
  132, 140, 148, 156, 165, 173, 181, 189,
  197, 206, 214, 222, 230, 239, 247, 255
};

static uint32 TargaError;

static int16 ttohs(int16 val);
static int32 ttohl(int32 val);
static int32 htotl(int32 val);
static uint32 tga_get_pixel(FILE *tga, ubyte bytes_per_pix, 
                            ubyte *colormap, ubyte cmap_bytes_entry);
static uint32 tga_convert_color(uint32 pixel, uint32 bpp_in, ubyte alphabits,
				uint32 format_out);
static void tga_write_pixel_to_mem(ubyte *dat, ubyte img_spec, uint32 number, 
                                   uint32 w, uint32 h, uint32 pixel,
				   uint32 format);


/* Returns the last error encountered */
int tga_get_last_error()
{
    return(TargaError);
}


/* Returns a pointer to the string for an error code */
const char *tga_error_string(int error_code)
{
    switch (error_code) {
      case TGA_ERR_NONE:
        return("no error");

      case TGA_ERR_BAD_HEADER:
        return("bad image header");

      case TGA_ERR_OPEN_FAILS:
        return("cannot open file");

      case TGA_ERR_BAD_FORMAT:
        return("bad format argument");

      case TGA_ERR_UNEXPECTED_EOF:
        return("unexpected end-of-file");

      case TGA_ERR_NODATA_IMAGE:
        return("image contains no data");

      case TGA_ERR_COLORMAP_FOR_GRAY:
        return("found colormap for a grayscale image");

      case TGA_ERR_BAD_COLORMAP_ENTRY_SIZE:
        return("unsupported colormap entry size");

      case TGA_ERR_BAD_COLORMAP:
        return("bad colormap");

      case TGA_ERR_READ_FAILS:
        return("cannot read from file");

      case TGA_ERR_BAD_IMAGE_TYPE:
        return("unknown image type");

      case TGA_ERR_BAD_DIMENSIONS:
        return("image has size 0 width or height (or both)");

      default:
        return("unknown error");
    }

    /* Shut up compiler.. */
    return(NULL);
}


/* Loads and converts a targa from disk */
void *tga_load(FILE *targafile, int *width, int *height, unsigned int format)
{
    ubyte  idlen;               /* Length of the image_id string below. */
    ubyte  cmap_type;           /* Paletted image <=> cmap_type */
    ubyte  image_type;          /* Any of the IMG_TYPE constants above. */
    uint16 cmap_first;
    uint16 cmap_length;         /* How long the colormap is */
    ubyte  cmap_entry_size;     /* How big a palette entry is. */
    uint16 img_spec_xorig;      /* x origin of the image in the image data. */
    uint16 img_spec_yorig;      /* y origin of the image in the image data. */
    uint16 img_spec_width;      /* Width of the image. */
    uint16 img_spec_height;     /* Height of the image. */
    ubyte  img_spec_pix_depth;  /* The depth of a pixel in the image. */
    ubyte  img_spec_img_desc;   /* The image descriptor. */
    ubyte *tga_hdr = NULL;
    ubyte *colormap = NULL;
    /***********************************************************************
     * Add by Yu-Chi because of variable initialization.
     * Add all = 0 to all the following variables
     ***********************************************************************/
    ubyte cmap_bytes_entry = 0;
    uint32 cmap_bytes = 0;
    uint32 tmp_col = 0;
    uint32 tmp_int32 = 0;
    ubyte  tmp_byte = 0;
    ubyte alphabits = 0;
    uint32 num_pixels = 0;
    uint32 i = 0;
    uint32 j = 0;
    ubyte *image_data = 0;
    uint32 img_dat_len = 0;
    ubyte bytes_per_pix = 0;
    ubyte true_bits_per_pixel = 0;
    uint32 bytes_total = 0;
    ubyte packet_header = 0;
    ubyte repcount = 0;
    
    switch (format) {
        case TGA_TRUECOLOR_24:
        case TGA_TRUECOLOR_32:
            break;
        default:
            TargaError = TGA_ERR_BAD_FORMAT;
            goto error;
    }
    
    if (!targafile) {
        TargaError = TGA_ERR_OPEN_FAILS;
        goto error;
    }

    /* Allocate memory for the header */
    tga_hdr = (ubyte *)malloc(HDR_LENGTH);

    /* Read the header in. */
    if (fread((void *)tga_hdr, 1, HDR_LENGTH, targafile) != HDR_LENGTH) {
        free(tga_hdr);
        TargaError = TGA_ERR_BAD_HEADER;
        goto error;
    }
    
    /* Byte order is important here. */
    idlen              = (ubyte)tga_hdr[HDR_IDLEN];
    image_type         = (ubyte)tga_hdr[HDR_IMAGE_TYPE];
    
    cmap_type          = (ubyte)tga_hdr[HDR_CMAP_TYPE];
    cmap_first         = ttohs(*(uint16 *)(&tga_hdr[HDR_CMAP_FIRST]));
    cmap_length        = ttohs(*(uint16 *)(&tga_hdr[HDR_CMAP_LENGTH]));
    cmap_entry_size    = (ubyte)tga_hdr[HDR_CMAP_ENTRY_SIZE];

    img_spec_xorig     = ttohs(*(uint16 *)(&tga_hdr[HDR_IMG_SPEC_XORIGIN]));
    img_spec_yorig     = ttohs(*(uint16 *)(&tga_hdr[HDR_IMG_SPEC_YORIGIN]));
    img_spec_width     = ttohs(*(uint16 *)(&tga_hdr[HDR_IMG_SPEC_WIDTH]));
    img_spec_height    = ttohs(*(uint16 *)(&tga_hdr[HDR_IMG_SPEC_HEIGHT]));
    img_spec_pix_depth = (ubyte)tga_hdr[HDR_IMG_SPEC_PIX_DEPTH];
    img_spec_img_desc  = (ubyte)tga_hdr[HDR_IMG_SPEC_IMG_DESC];

    free(tga_hdr);

    num_pixels = img_spec_width * img_spec_height;

    if (!num_pixels) {
        TargaError = TGA_ERR_BAD_DIMENSIONS;
        goto error;
    }

    alphabits = img_spec_img_desc & 0x0F;
#ifndef DISABLE_TRACE
    if (alphabits && srcTrace)
	fprintf(stderr, "TGA: %d alpha bits.\n", alphabits);
#endif

    /* Seek past the image id, if there is one */
    if (idlen && fseek(targafile, idlen, SEEK_CUR)) {
        TargaError = TGA_ERR_UNEXPECTED_EOF;
        goto error;
    }

    /* If this is a 'nodata' image, just jump out. */
    if (image_type == TGA_IMG_NODATA) {
        TargaError = TGA_ERR_NODATA_IMAGE;
        goto error;
    }

    /* Now we're starting to get into the meat of the matter. */
    
    /* Deal with the colormap, if there is one. */
    if (cmap_type) {
        switch (image_type) {
          case TGA_IMG_UNC_PALETTED:
          case TGA_IMG_RLE_PALETTED:
#ifndef DISABLE_TRACE
	    if (srcTrace)
	        fprintf(stderr, "TGA: Paletted image.\n");
#endif
            break;
            
          case TGA_IMG_UNC_TRUECOLOR:
          case TGA_IMG_RLE_TRUECOLOR:
            /* This should really be an error, but some really old
             * crusty targas might actually be like this (created by
	     * TrueVision, no less!) so, we'll hack our way through it. */
            break;
            
          case TGA_IMG_UNC_GRAYSCALE:
          case TGA_IMG_RLE_GRAYSCALE:
            TargaError = TGA_ERR_COLORMAP_FOR_GRAY;
            goto error;
        }
        
        /* Ensure colormap entry size is something we support */
        if (!(cmap_entry_size == 15 || 
              cmap_entry_size == 16 ||
              cmap_entry_size == 24 ||
              cmap_entry_size == 32)) {
            TargaError = TGA_ERR_BAD_COLORMAP_ENTRY_SIZE;
            goto error;
        }
        
        /* Allocate memory for a colormap */
        if (cmap_entry_size & 0x07) {
            cmap_bytes_entry = ((8 - (cmap_entry_size & 0x07)) +
				cmap_entry_size) >> 3;
        } else {
            cmap_bytes_entry = cmap_entry_size >> 3;
        }
        
        cmap_bytes = cmap_bytes_entry * cmap_length;
        colormap = (ubyte *)malloc(cmap_bytes);
        
        for (i = 0; i < cmap_length; i++) {
            /* Seek ahead to first entry used */
            if (cmap_first)
                fseek(targafile, cmap_first * cmap_bytes_entry, SEEK_CUR);
            
            tmp_int32 = 0;
            for (j = 0; j < cmap_bytes_entry; j++) {
                if (!fread(&tmp_byte, 1, 1, targafile)) {
                    free(colormap);
                    TargaError = TGA_ERR_BAD_COLORMAP;
            	    goto error;
                }
                tmp_int32 += tmp_byte << (j * 8);
            }

            /* Byte order correct. */
#ifdef WORDS_BIGENDIAN
            tmp_int32 = ttohl(tmp_int32);
#endif
            for (j = 0; j < cmap_bytes_entry; j++)
                colormap[i * cmap_bytes_entry + j] =
						  (tmp_int32 >> (8 * j)) & 0xFF;
        }
    }

    /* Compute number of bytes in an image data unit
     * (either index or BGR triple)
     */
    if (img_spec_pix_depth & 0x07) {
        bytes_per_pix = ((8 - (img_spec_pix_depth & 0x07)) +
			 img_spec_pix_depth) >> 3;
    } else {
        bytes_per_pix = img_spec_pix_depth >> 3;
    }

    /* Assume that there's one byte per pixel */
    if (bytes_per_pix == 0)
        bytes_per_pix = 1;

    /* Compute how many bytes of storage we need for the image */
    bytes_total = num_pixels * format;

    image_data = (ubyte *)malloc(bytes_total);

    img_dat_len = num_pixels * bytes_per_pix;

    /* Compute the true number of bits per pixel */
    true_bits_per_pixel = cmap_type ? cmap_entry_size : img_spec_pix_depth;
#ifndef DISABLE_TRACE
    if (srcTrace)
	fprintf(stderr, "TGA: %d-bit image.\n", true_bits_per_pixel);
#endif

    switch (image_type) {
      case TGA_IMG_UNC_TRUECOLOR:
#ifndef DISABLE_TRACE
	if (srcTrace)
	    fprintf(stderr, "TGA: Truecolor image.\n");
#endif
      case TGA_IMG_UNC_GRAYSCALE:
      case TGA_IMG_UNC_PALETTED:

        /* FIXME: support grayscale */

        for (i = 0; i < num_pixels; i++) {
            /* Get the color value. */
            tmp_col = tga_get_pixel(targafile, bytes_per_pix, colormap,
				    cmap_bytes_entry);
            tmp_col = tga_convert_color(tmp_col, true_bits_per_pixel,
					alphabits, format);
            /* Now write the data out. */
            tga_write_pixel_to_mem(image_data, img_spec_img_desc, i,
				   img_spec_width, img_spec_height,
				   tmp_col, format);
        }
        break;

      case TGA_IMG_RLE_TRUECOLOR:
      case TGA_IMG_RLE_GRAYSCALE:
      case TGA_IMG_RLE_PALETTED:
#ifndef DISABLE_TRACE
	if (srcTrace)
	    fprintf(stderr, "TGA: Compressed image.\n");
#endif
        /* FIXME: handle grayscale.. */

        for (i = 0; i < num_pixels; ) {
            /* A bit of work to do to read the data.. */
            if (fread(&packet_header, 1, 1, targafile) < 1)
                /* Well, just let them fill the rest with null pixels then... */
                packet_header = 1;

            if (packet_header & 0x80) {
                /* Run length packet */
                tmp_col = tga_get_pixel(targafile, bytes_per_pix, colormap,
					cmap_bytes_entry);
                tmp_col = tga_convert_color(tmp_col, true_bits_per_pixel,
					    alphabits, format);
                repcount = (packet_header & 0x7F) + 1;
                
                /* Write all the data out */
                for (j = 0; j < repcount; j++ )
                    tga_write_pixel_to_mem(image_data, img_spec_img_desc, i + j,
					   img_spec_width, img_spec_height,
					   tmp_col, format);
                i += repcount;
            } else {
                /* Raw packet */
                repcount = (packet_header & 0x7F) + 1;
                
                for (j = 0; j < repcount; j++) {
                    tmp_col = tga_get_pixel(targafile, bytes_per_pix, colormap,
					    cmap_bytes_entry);
                    tmp_col = tga_convert_color(tmp_col, true_bits_per_pixel,
						alphabits, format);
                    tga_write_pixel_to_mem(image_data, img_spec_img_desc, i + j,
					   img_spec_width, img_spec_height,
					   tmp_col, format);
                }
                i += repcount;
            }
        }
        break;

      default:
        TargaError = TGA_ERR_BAD_IMAGE_TYPE;
        goto error;
    }

    *width  = img_spec_width;
    *height = img_spec_height;

    return((void *)image_data);

 error:
    return(NULL);
}

#ifdef WRITE_TARGA
int tga_write_raw(const char *file, int width, int height, unsigned char *dat,
		  unsigned int format) {
    FILE *tga;
    uint32 i, j, pixbuf;
    uint32 size = width * height;
    float red, green, blue, alpha;
    char id[] = "written with libtarga";
    ubyte idlen = 21;
    ubyte zeroes[5] = { 0, 0, 0, 0, 0 };
    ubyte one = 1;
    ubyte cmap_type = 0;
    ubyte img_type  = 2;  /* 2 - uncompressed truecolor  10 - RLE truecolor */
    uint16 xorigin  = 0;
    uint16 yorigin  = 0;
    ubyte pixdepth = format * 8;  /* bpp */
    ubyte img_desc;
    
    switch (format) {
      case TGA_TRUECOLOR_24:
        img_desc = 0;
        break;

      case TGA_TRUECOLOR_32:
        img_desc = 8;
        break;

      default:
        TargaError = TGA_ERR_BAD_FORMAT;
        return(0);
        break;
    }

    tga = fopen(file, "wb");
    if (!tga) {
        TargaError = TGA_ERR_OPEN_FAILS;
        return(0);
    }

    /* Write id length */
    fwrite(&idlen, 1, 1, tga);

    /* Write colormap type */
    fwrite(&cmap_type, 1, 1, tga);

    /* Write image type */
    fwrite(&img_type, 1, 1, tga);

    /* Write cmap spec. */
    fwrite(&zeroes, 5, 1, tga);

    /* Write image spec. */
    fwrite(&xorigin, 2, 1, tga);
    fwrite(&yorigin, 2, 1, tga);
    fwrite(&width, 2, 1, tga);
    fwrite(&height, 2, 1, tga);
    fwrite(&pixdepth, 1, 1, tga);
    fwrite(&img_desc, 1, 1, tga);

    /* Write image id. */
    fwrite(&id, idlen, 1, tga);

    /* Color correction -- data is in RGB, need BGR. */
    for (i = 0; i < size; i++) {
        pixbuf = 0;
        for (j = 0; j < format; j++)
            pixbuf += dat[i * format + j] << (8 * j);

        switch(format) {
          case TGA_TRUECOLOR_24:
            pixbuf = ((pixbuf & 0xFF) << 16) + 
                     (pixbuf & 0xFF00) + 
                     ((pixbuf & 0xFF0000) >> 16);
            pixbuf = htotl(pixbuf);
            
            fwrite(&pixbuf, 3, 1, tga);
            break;

          case TGA_TRUECOLOR_32:
            /* Need to un-premultiply alpha.. */
            red     = (pixbuf & 0xFF) / 255.0f;
            green   = ((pixbuf & 0xFF00) >> 8) / 255.0f;
            blue    = ((pixbuf & 0xFF0000) >> 16) / 255.0f;
            alpha   = ((pixbuf & 0xFF000000) >> 24) / 255.0f;

            if (alpha > 0.0001) {
                red /= alpha;
                green /= alpha;
                blue /= alpha;
            }

            /* Clamp to 1.0f */
            red = red > 1.0f ? 255.0f : red * 255.0f;
            green = green > 1.0f ? 255.0f : green * 255.0f;
            blue = blue > 1.0f ? 255.0f : blue * 255.0f;
            alpha = alpha > 1.0f ? 255.0f : alpha * 255.0f;

            pixbuf = (ubyte)blue + (((ubyte)green) << 8) + 
                     (((ubyte)red) << 16) + (((ubyte)alpha) << 24);
            pixbuf = htotl(pixbuf);
           
            fwrite(&pixbuf, 4, 1, tga);
            break;
        }
    }
    fclose(tga);

    return(1);
}


int tga_write_rle(const char *file, int width, int height, unsigned char *dat,
		  unsigned int format)
{
    FILE *tga;
    uint32 i, j, oc, nc, pixbuf;
    uint32 size = width * height;
    enum RLE_STATE { INIT, NONE, RLP, RAWP };
    int state = INIT;
    uint16 shortwidth = (uint16)width;
    uint16 shortheight = (uint16)height;
    ubyte repcount;
    float red, green, blue, alpha;
    int idx, row, column;
    /* Have to buffer a whole line for raw packets. */
    unsigned char *rawbuf = (unsigned char *)malloc(width * format);  
    char id[] = "written with libtarga";
    ubyte idlen = 21;
    ubyte zeroes[5] = { 0, 0, 0, 0, 0 };
    ubyte one = 1;
    ubyte cmap_type = 0;
    ubyte img_type  = 10;  /* 2 - uncompressed truecolor  10 - RLE truecolor */
    uint16 xorigin  = 0;
    uint16 yorigin  = 0;
    ubyte pixdepth = format * 8;  /* bpp */
    ubyte img_desc = format == TGA_TRUECOLOR_32 ? 8 : 0;
  
    switch (format) {
      case TGA_TRUECOLOR_24:
      case TGA_TRUECOLOR_32:
        break;

     default:
        TargaError = TGA_ERR_BAD_FORMAT;
        return(0);
    }

    tga = fopen(file, "wb");
    if (!tga) {
        TargaError = TGA_ERR_OPEN_FAILS;
        return(0);
    }

    /* Write id length */
    fwrite(&idlen, 1, 1, tga);

    /* Write colormap type */
    fwrite(&cmap_type, 1, 1, tga);

    /* Write image type */
    fwrite(&img_type, 1, 1, tga);

    /* Write cmap spec. */
    fwrite(&zeroes, 5, 1, tga);

    /* Write image spec. */
    fwrite(&xorigin, 2, 1, tga);
    fwrite(&yorigin, 2, 1, tga);
    fwrite(&shortwidth, 2, 1, tga);
    fwrite(&shortheight, 2, 1, tga);
    fwrite(&pixdepth, 1, 1, tga);
    fwrite(&img_desc, 1, 1, tga);

    /* Write image id. */
    fwrite(&id, idlen, 1, tga);

    /* Initial color values -- just to shut up the compiler. */
    nc = 0;

    /* Color correction -- data is in RGB, need BGR. */
    /* Also run-length-encoding. */
    for (i = 0; i < size; i++) {
        idx = i * format;
        row = i / width;
        column = i % width;

        /* printf("row: %d, col: %d\n", row, column); */
        pixbuf = 0;
        for (j = 0; j < format; j++)
            pixbuf += dat[idx + j] << (8 * j);

        switch (format) {
          case TGA_TRUECOLOR_24:
            pixbuf = ((pixbuf & 0xFF) << 16) + (pixbuf & 0xFF00) + 
                     ((pixbuf & 0xFF0000) >> 16);
            pixbuf = htotl(pixbuf);
            break;

          case TGA_TRUECOLOR_32:
            /* Need to un-premultiply alpha.. */
            red     = (pixbuf & 0xFF) / 255.0f;
            green   = ((pixbuf & 0xFF00) >> 8) / 255.0f;
            blue    = ((pixbuf & 0xFF0000) >> 16) / 255.0f;
            alpha   = ((pixbuf & 0xFF000000) >> 24) / 255.0f;

            if (alpha > 0.0001) {
                red /= alpha;
                green /= alpha;
                blue /= alpha;
            }

            /* Clamp to 1.0f */
            red = red > 1.0f ? 255.0f : red * 255.0f;
            green = green > 1.0f ? 255.0f : green * 255.0f;
            blue = blue > 1.0f ? 255.0f : blue * 255.0f;
            alpha = alpha > 1.0f ? 255.0f : alpha * 255.0f;

            pixbuf = (ubyte)blue + (((ubyte)green) << 8) + 
                     (((ubyte)red) << 16) + (((ubyte)alpha) << 24);
            pixbuf = htotl(pixbuf);
            break;
        }

        oc = nc;
        nc = pixbuf;

        switch (state) {
          case INIT:
            /* This is used to make sure we have 2 pixel values to consider. */
            state = NONE;
            break;

          case NONE:
            if (column == 0) {
                /* Write a 1 pixel raw packet for the old pixel,
		 * then go thru again. */
                repcount = 0;
                fwrite(&repcount, 1, 1, tga);
#ifdef WORDS_BIGENDIAN
                fwrite(&oc + 4, format, 1, tga);  /* Byte order.. */
#else
                fwrite(&oc, format, 1, tga);
#endif
                state = NONE;
                break;
            }

            repcount = 0;
            if (nc == oc) {
                state = RLP;
            } else {
                state = RAWP;
                for (j = 0; j < format; j++) {
#ifdef WORDS_BIGENDIAN
                    rawbuf[(repcount * format) + j] =
					       (ubyte)(*(&oc + format - j - 1));
#else
                    rawbuf[(repcount * format) + j] = *(((ubyte *)&oc) + j);
#endif
                }
            }
            break;

          case RLP:
            repcount++;

            if (column == 0) {
                /* Finish off rlp. */
                repcount |= 0x80;
                fwrite(&repcount, 1, 1, tga);
#ifdef WORDS_BIGENDIAN
                fwrite(&oc + 4, format, 1, tga);  /* Byte order.. */
#else
                fwrite(&oc, format, 1, tga);
#endif
                state = NONE;
                break;
            }

            if (repcount == 127) {
                /* Finish off rlp. */
                repcount |= 0x80;
                fwrite(&repcount, 1, 1, tga);
#ifdef WORDS_BIGENDIAN
                fwrite(&oc + 4, format, 1, tga);  /* Byte order.. */
#else
                fwrite(&oc, format, 1, tga);
#endif
                state = NONE;
                break;
            }

            if (nc != oc) {
                /* Finish off rlp */
                repcount |= 0x80;
                fwrite(&repcount, 1, 1, tga);
#ifdef WORDS_BIGENDIAN
                fwrite((&oc) + 4, format, 1, tga);  /* Byte order.. */
#else
                fwrite(&oc, format, 1, tga);
#endif
                state = NONE;
            }
            break;

          case RAWP:
            repcount++;

            if (column == 0) {
                /* Finish off rawp. */
                for (j = 0; j < format; j++) {
#ifdef WORDS_BIGENDIAN
                    rawbuf[(repcount * format) + j] =
					       (ubyte)(*(&oc + format - j - 1));
#else
                    rawbuf[(repcount * format) + j] = *(((ubyte *)&oc) + j);
#endif
                }
                fwrite(&repcount, 1, 1, tga);
                fwrite(rawbuf, (repcount + 1) * format, 1, tga);
                state = NONE;
                break;
            }

            if (repcount == 127) {
                /* Finish off rawp. */
                for (j = 0; j < format; j++) {
#ifdef WORDS_BIGENDIAN
                    rawbuf[(repcount * format) + j] =
					       (ubyte)(*(&oc + format - j - 1));
#else
                    rawbuf[(repcount * format) + j] = *(((ubyte *)&oc) + j);
#endif
                }
                fwrite(&repcount, 1, 1, tga);
                fwrite(rawbuf, (repcount + 1) * format, 1, tga);
                state = NONE;
                break;
            }

            if (nc == oc) {
                /* Finish off rawp */
                repcount--;
                fwrite(&repcount, 1, 1, tga);
                fwrite(rawbuf, (repcount + 1) * format, 1, tga);
                
                /* Start new rlp */
                repcount = 0;
                state = RLP;
                break;
            }

            /* Continue making rawp */
            for (j = 0; j < format; j++) {
#ifdef WORDS_BIGENDIAN
                rawbuf[(repcount * format) + j] =
					       (ubyte)(*(&oc + format - j - 1));
#else
                rawbuf[(repcount * format) + j] = *(((ubyte *)&oc) + j);
#endif
            }
            break;
        }
    }

    /* Clean up state. */
    switch (state) {
      case INIT:
        break;

      case NONE:
        /* Write the last 2 pixels in a raw packet. */
        fwrite(&one, 1, 1, tga);
#ifdef WORDS_BIGENDIAN
                fwrite(&oc + 4, format, 1, tga);  /* Byte order.. */
#else
                fwrite(&oc, format, 1, tga );
#endif
#ifdef WORDS_BIGENDIAN
                fwrite(&nc + 4, format, 1, tga);  /* Byte order.. */
#else
                fwrite(&nc, format, 1, tga);
#endif
        break;

      case RLP:
        repcount++;
        repcount |= 0x80;
        fwrite(&repcount, 1, 1, tga);
#ifdef WORDS_BIGENDIAN
                fwrite(&oc + 4, format, 1, tga);  /* Byte order.. */
#else
                fwrite(&oc, format, 1, tga);
#endif
        break;

      case RAWP:
        repcount++;
        for (j = 0; j < format; j++) {
#ifdef WORDS_BIGENDIAN
            rawbuf[(repcount * format) + j] = (ubyte)(*(&oc + format - j - 1));
#else
            rawbuf[(repcount * format) + j] = *(((ubyte *)&oc) + j);
#endif
        }
        fwrite(&repcount, 1, 1, tga);
        fwrite(rawbuf, (repcount + 1) * 3, 1, tga);
        break;
    }

    /* Close the file. */
    fclose(tga);

    free(rawbuf);

    return(1);
}
#endif  /* WRITE_TARGA */


/****************************************************************************/

static void tga_write_pixel_to_mem(ubyte *dat, ubyte img_spec, uint32 number, 
                                   uint32 w, uint32 h, uint32 pixel,
				   uint32 format)
{
    /* Write the pixel to the data regarding how the
     * header says the data is ordered top to bottom. */
    uint32 j, x, y, addy;

    switch ((img_spec & 0x30) >> 4) {
      case TGA_LOWER_LEFT:
        x = number % w;
        y = h - 1 - (number / w);
        break;
      case TGA_LOWER_RIGHT:
        x = w - 1 - (number % w);
        y = h - 1 - (number / w);
        break;
      case TGA_UPPER_LEFT:
        x = number % w;
        y = number / w;
        break;
      case TGA_UPPER_RIGHT:
        x = w - 1 - (number % w);
        y = number / h;
        break;
    }

    addy = (y * w + x) * format;
    for (j = 0; j < format; j++)
        dat[addy + j] = (ubyte)((pixel >> (j * 8)) & 0xFF);
}


static uint32 tga_get_pixel(FILE *tga, ubyte bytes_per_pix, 
                            ubyte *colormap, ubyte cmap_bytes_entry)
{
    /* Get the image data value out */
    uint32 tmp_col, j;
    uint32 tmp_int32 = 0;
    ubyte tmp_byte;

    for (j = 0; j < bytes_per_pix; j++) {
        if (fread(&tmp_byte, 1, 1, tga) < 1 ) {
            tmp_int32 = 0;
        } else {
            tmp_int32 += tmp_byte << (j * 8);
        }
    }

#ifdef WORDS_BIGENDIAN
    /* Byte-order correct the thing */
    switch (bytes_per_pix) {
      case 2:
        tmp_int32 = ttohs((uint16)tmp_int32);
        break;
        
      case 3:  /* Intentional fall-thru */
      case 4:
        tmp_int32 = ttohl(tmp_int32);
        break;
    }
#endif

    if (colormap) {
        /* Need to look up value to get real color */
        tmp_col = 0;
        for (j = 0; j < cmap_bytes_entry; j++)
            tmp_col += colormap[cmap_bytes_entry * tmp_int32 + j] << (8 * j);
    } else {
        tmp_col = tmp_int32;
    }
    
    return(tmp_col);
}


static uint32 tga_convert_color(uint32 pixel, uint32 bpp_in, ubyte alphabits,
				uint32 format_out)
{
    /* This is not only responsible for converting from different depths
     * to other depths, it also switches BGR to RGB.
     * This thing will also premultiply alpha, on a pixel by pixel basis.
     */
    ubyte r, g, b;

    switch (bpp_in) {
      case 32:
        if (alphabits > 0)
             /* 32-bit to 32-bit */
             break;
	/* Fall thru to treat as 24-bit */
        
      case 24:
        /* 24-bit to 32-bit; (force alpha to full) */
        pixel |= 0xFF000000;
        break;

      case 15:
      case 16:
	b = c5to8bits[pixel & 0x001F];
	g = c5to8bits[(pixel >> 5) & 0x001F];
	r = c5to8bits[(pixel >> 10) & 0x001F];

        /* 15-bit to 32-bit */
        if ((alphabits == 1) && !(pixel & 0x8000)) {
	    /* Transparent alpha */
            pixel = (r << 16) + (g << 8) + b;
	} else {
	    /* Alpha to full */
            pixel = 0xFF000000 + (r << 16) + (g << 8) + b;
	}
        break;
    }
    
    /* Convert the 32-bit pixel from BGR to RGB. */
    pixel = (pixel & 0xFF00FF00) + ((pixel & 0xFF) << 16) +
	    ((pixel & 0xFF0000) >> 16);

    if (alphabits) {
        ubyte a = (pixel & 0xFF000000) >> 24;

        /* Not premultiplied alpha -- multiply. */
	if (a) {
	    r = pixel & 0x000000FF;
	    g = (pixel & 0x0000FF00) >> 8;
	    b = (pixel & 0x00FF0000) >> 16;
            r = (ubyte)(((float)r / 255.0f) * ((float)a / 255.0f) * 255.0f);
            g = (ubyte)(((float)g / 255.0f) * ((float)a / 255.0f) * 255.0f);
            b = (ubyte)(((float)b / 255.0f) * ((float)a / 255.0f) * 255.0f);
            pixel = r + (g << 8) + (b << 16) + (a << 24);
	}
    }

#ifdef NEED_THIS	/* Alpha will be discarded later if TGA_TRUECOLOR_24 */
    /* Now convert from 32-bit to whatever they want. */
    switch (format_out) {
      case TGA_TRUECOLOR_32:
        /* 32 to 32 -- nop. */
        break;
        
      case TGA_TRUECOLOR_24:
        /* 32 to 24 -- discard alpha. */
        pixel &= 0x00FFFFFF;
        break;
    }
#endif
    return(pixel);
}


static int16 ttohs(int16 val)
{
#ifdef WORDS_BIGENDIAN
    return(((val & 0xFF) << 8) + (val >> 8));
#else
    return(val);
#endif 
}


static int32 ttohl(int32 val)
{
#ifdef WORDS_BIGENDIAN
    return(((val & 0x000000FF) << 24) +
           ((val & 0x0000FF00) << 8)  +
           ((val & 0x00FF0000) >> 8)  +
           ((val & 0xFF000000) >> 24));
#else
    return(val);
#endif 
}


static int32 htotl(int32 val)
{
#ifdef WORDS_BIGENDIAN
    return(((val & 0x000000FF) << 24) +
           ((val & 0x0000FF00) << 8)  +
           ((val & 0x00FF0000) >> 8)  +
           ((val & 0xFF000000) >> 24));
#else
    return(val);
#endif 
}
