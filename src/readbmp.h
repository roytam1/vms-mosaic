/*
 * For VMS Mosaic by GEC on 3/29/00
 */

#ifndef _READBMP_H_
#define _READBMP_H_

unsigned char *ReadBMP(FILE *ifp, int *width, int *height, XColor *colrs,
		       unsigned char **alpha);

#endif /* _READBMP_H_ */
