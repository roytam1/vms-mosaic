/* Copyright (C) 2007, 2008 - The VMS Mosaic Project */

#include "../config.h"

/* This .h file should not be referenced in DESCRIP.MMS for this module */
/* because the SVG dummy .h file will force a recompile when needed. */
#ifdef __GNUC__
#include MOSAIC_BUILT
#else
#include "mosaic_built"
#endif

#include "mosaic.h"
#ifdef HAVE_SVG
#include "rsvg.h"
#include "librsvg-features.h"
#endif
#include "readSVG.h"
#include "quantize.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int reportBugs;
#endif

extern int Quantize_Found_NZero_Alpha;

unsigned char *ReadSVG(FILE *infile, int *width, int *height, XColor *colrs,
		       unsigned char **alpha)
{
#ifdef HAVE_SVG
    FILE *file;
    unsigned char *pixels, *image;
    register int n;
    int w, h;
    RsvgHandle *svg_info;
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    unsigned char rbuf[8192];

    rsvg_init();

    if (!(svg_info = rsvg_handle_new())) {
#ifndef DISABLE_TRACE
	if (reportBugs)
	    fprintf(stderr, "RSVG Handle Alloc failure\n");
#endif
	return NULL;
    }

    while (n = fread(rbuf, 1, 8192, infile))
        (void) rsvg_handle_write(svg_info, rbuf, n, &error);

    rsvg_handle_close(svg_info, &error);
    if (error)
        g_error_free(error);

#ifndef DISABLE_TRACE
    if (srcTrace)
	fprintf(stderr, "Read SVG image\n");
#endif

    pixbuf = rsvg_handle_get_pixbuf(svg_info);
    rsvg_handle_free(svg_info);

    if (!pixbuf) {
#ifndef DISABLE_TRACE
	if (reportBugs)
	    fprintf(stderr, "No SVG image found\n");
#endif
	return NULL;
    }
    w = *width = gdk_pixbuf_get_width(pixbuf); 
    h = *height = gdk_pixbuf_get_height(pixbuf);

    if (!(pixels = (unsigned char *)gdk_pixbuf_get_pixels(pixbuf))) {
#ifndef DISABLE_TRACE
	if (reportBugs)
	    fprintf(stderr, "No SVG pixel data found\n");
#endif
	return NULL;
    }
    image = QuantizeImage(pixels, w, h, 256, 1, colrs, 1);

    if (Quantize_Found_NZero_Alpha) {
	unsigned char *a;
	unsigned char *ptr = pixels;
	int size = w * h;
	int i;

#ifndef DISABLE_TRACE
        if (srcTrace)
	    fprintf(stderr, "Found Alpha in SVG image\n");
#endif
	*alpha = a = malloc(size);
	ptr += 3;
	for (i = 0; i < size; i++) {
		*a++ = *ptr;
		ptr += 4;
	}
    }

    /* Free pixbuf and pixels */
    g_object_unref(G_OBJECT(pixbuf));
    rsvg_term();
    return image;

#else
#ifndef DISABLE_TRACE
    if (srcTrace || reportBugs)
	fprintf(stderr, "SVG image support not included.\n");
#endif
    return NULL;
#endif
}
