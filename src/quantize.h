/* This file is Copyright (C) 2005 - The VMS Mosaic Project */

#ifndef __QUANTIZE_COLORS_H__
#define __QUANTIZE_COLORS_H__

extern unsigned char *QuantizeImage(unsigned char *image, unsigned int w,
                                    unsigned int h, unsigned int number_colors,
                                    unsigned int dither, XColor *colrs,
				    int alpha);

extern void ImageQuantize(Widget w, XtPointer clid, XtPointer calld);
#endif
