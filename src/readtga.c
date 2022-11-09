/*
 * Created for VMS Mosaic by GEC on 11-Oct-2006
 *
 * Read a Targa image from file
 */

/* Copyright (C) 2006 - The VMS Mosaic Project */

#include "../config.h"
#include "mosaic.h"
#include <X11/Xlib.h>
#include "readtga.h"
#include "libtarga.h"
#include "quantize.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

extern int Quantize_Found_NZero_Alpha;

unsigned char *ReadTGA(FILE *fp, int *width, int *height, XColor *colrs,
		       unsigned char **alpha)
{
	unsigned char *image, *imagemap;

#ifndef DISABLE_TRACE
        if (srcTrace)
                fprintf(stderr, "ReadTGA: starting\n");
#endif
	image = tga_load(fp, width, height, TGA_TRUECOLOR_32);
	if (!image) {
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "TGA error: %s\n",
				tga_error_string(tga_get_last_error()));
#endif
		return NULL;
	}
#ifndef DISABLE_TRACE
        if (srcTrace)
                fprintf(stderr, "ReadTGA: Got image\n");
#endif
	imagemap = QuantizeImage(image, *width, *height, 256, 1, colrs, 1);

	if (Quantize_Found_NZero_Alpha) {
		unsigned char *a;
		unsigned char *ptr = image;
		int size = *width * *height;
		int i;

		*alpha = a = malloc(size);
		ptr += 3;
		for (i = 0; i < size; i++) {
			*a++ = *ptr;
			ptr += 4;
		}
	}
	free(image);

	return imagemap;
}
