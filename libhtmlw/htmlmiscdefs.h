/* Copyright (C) 1997 - G.Dauphin
 * See the file "license.mMosaic" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

/* Copyright (C) 2007 - The VMS Mosaic Project */

#ifndef LIBHTMLW_HTML_MISC_DEFS_H
#define LIBHTMLW_HTML_MISC_DEFS_H

#define CHECK_OUT_OF_MEM(x) { if (!x) {\
				fprintf(stderr, "Out of memory\n");\
				exit(1);\
			    } }

#endif /* LIBHTMLW_HTML_MISC_DEFS_H */
