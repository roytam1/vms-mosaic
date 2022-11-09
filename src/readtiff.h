/* Copyright (C) 2006 - The VMS Mosaic Project */

#ifndef __READTIFF_H__
#define __READTIFF_H__

#ifdef HAVE_TIFF
unsigned char *ReadTIFF(FILE *infile, char *filename, int *width, int *height,
                        XColor *colrs, unsigned char **alpha);
#endif

#endif
