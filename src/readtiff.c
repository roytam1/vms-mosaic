/* Copyright (C) 2006, 2008 - The VMS Mosaic Project */

#include "../config.h"

#ifdef HAVE_TIFF
#include <stdio.h>

#include "mosaic.h"
#include "tiffio.h"
#include "readTIFF.h"
#include "quantize.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

extern int Quantize_Found_Alpha;

unsigned char *ReadTIFF(FILE *infile, char *filename, int *width, int *height,
			XColor *colrs, unsigned char **alpha)
{
	TIFF *tif;
	unsigned char *image = NULL;
	unsigned int *raster;
	int w, h, size;
	static int err_msgs;
	static int init = 0;

#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "ReadTIFF: starting\n");
#endif
	if (!init) {
		err_msgs = get_pref_boolean(eTIFF_ERROR_MESSAGES);
		init = 1;
	}

	/* Disable TIFF error and warning messages */
	if (!err_msgs) {
		TIFFSetErrorHandler(NULL);
		TIFFSetWarningHandler(NULL);
	}

	if (!(tif = TIFFFdOpen(fileno(infile), filename, "r")))
		return NULL;

	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	*width = w;
	*height = h;
	size = w * h;

	if (!(raster = (unsigned int *) _TIFFmalloc(size * 4)))
		goto Error;

	if (!TIFFReadRGBAImageOriented(tif, w, h, raster,
				       ORIENTATION_TOPLEFT, 0)) {
		_TIFFfree(raster);
		goto Error;
	}
#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "ReadTIFF: Got image\n");
#endif
	image = QuantizeImage((unsigned char *)raster, w, h, 256, 1, colrs, 1);

	if (Quantize_Found_Alpha) {
		unsigned char *a;
		unsigned char *ptr = (unsigned char *)raster;
		int i;

		*alpha = a = malloc(size);
		ptr += 3;
		for (i = 0; i < size; i++) {
			*a++ = *ptr;
			ptr += 4;
		}
	}
	_TIFFfree(raster);

 Error:
	TIFFCleanup(tif);	
	return image;
}
#endif
