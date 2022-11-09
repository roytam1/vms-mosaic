/* Copyright (C) 2007 - The VMS Mosaic Project */

#ifndef __READJ2K_H__
#define __READJ2K_H__

#ifdef HAVE_JPEG
unsigned char *ReadJ2K(FILE *infile, int *width, int *height, XColor *colrs,
		       int type);

#define TYPE_J2K 0
#define TYPE_JPT 1
#define TYPE_JP2 2
#endif

#endif
