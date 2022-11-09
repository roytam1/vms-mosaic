/* This file is Copyright (C) 1998, 2004, 2005, 2006 - The VMS Mosaic Project */

#include "../config.h"
#include "mosaic.h"
#include <X11/Xlib.h>
#include "colors.h"

extern void FS_Dither();
extern int installed_colormap;
extern Colormap installed_cmap;
XColor BSColors[256];
int BSCnum;

#define MUST_HAVE 187

#ifndef DISABLE_TRACE
extern int srcTrace;
#endif

#ifdef HAVE_JPEG
#include "jpeglib.h"
#include "readJPEG.h"
JSAMPARRAY jcolormap;
#endif

static char *colors_icons[] = {
"78706B",
"5A5357",
"717FCA",
"929ED3",
"6F76A0",
"9F8762",
"51518C",
"4B51B1",
"A3B1D7",
"CDE1F9",
"B5C0D1",
"404040",
"808080",
"BFBFBF",
"E7E7E7"
};

static char *colors_216[] = {
"ffffff",
"ffffcc",
"ffff99",
"ffff66",
"ffff33",
"ffff00",
"ffccff",
"ffcccc",
"ffcc99",
"ffcc66",
"ffcc33",
"ffcc00",
"ff99ff",
"ff99cc",
"ff9999",
"ff9966",
"ff9933",
"ff9900",
"ff66ff",
"ff66cc",
"ff6699",
"ff6666",
"ff6633",
"ff6600",
"ff33ff",
"ff33cc",
"ff3399",
"ff3366",
"ff3333",
"ff3300",
"ff00ff",
"ff00cc",
"ff0099",
"ff0066",
"ff0033",
"ff0000",
"ccffff",
"ccffcc",
"ccff99",
"ccff66",
"ccff33",
"ccff00",
"ccccff",
"cccccc",
"cccc99",
"cccc66",
"cccc33",
"cccc00",
"cc99ff",
"cc99cc",
"cc9999",
"cc9966",
"cc9933",
"cc9900",
"cc66ff",
"cc66cc",
"cc6699",
"cc6666",
"cc6633",
"cc6600",
"cc33ff",
"cc33cc",
"cc3399",
"cc3366",
"cc3333",
"cc3300",
"cc00ff",
"cc00cc",
"cc0099",
"cc0066",
"cc0033",
"cc0000",
"99ffff",
"99ffcc",
"99ff99",
"99ff66",
"99ff33",
"99ff00",
"99ccff",
"99cccc",
"99cc99",
"99cc66",
"99cc33",
"99cc00",
"9999ff",
"9999cc",
"999999",
"999966",
"999933",
"999900",
"9966ff",
"9966cc",
"996699",
"996666",
"996633",
"996600",
"9933ff",
"9933cc",
"993399",
"993366",
"993333",
"993300",
"9900ff",
"9900cc",
"990099",
"990066",
"990033",
"990000",
"66ffff",
"66ffcc",
"66ff99",
"66ff66",
"66ff33",
"66ff00",
"66ccff",
"66cccc",
"66cc99",
"66cc66",
"66cc33",
"66cc00",
"6699ff",
"6699cc",
"669999",
"669966",
"669933",
"669900",
"6666ff",
"6666cc",
"666699",
"666666",
"666633",
"666600",
"6633ff",
"6633cc",
"663399",
"663366",
"663333",
"663300",
"6600ff",
"6600cc",
"660099",
"660066",
"660033",
"660000",
"333399",
"333333",
"333300",
"330099",
"330066",
"330033",
"330000",
"00ffff",
"00ffcc",
"00ff99",
"00ff66",
"00ff33",
"00ff00",
"00ccff",
"00cccc",
"00cc99",
"00cc66",
"00cc33",
"00cc00",
"0099ff",
"0099cc",
"009999",
"009966",
"009933",
"009900",
"0066ff",
"0066cc",
"006699",
"006666",
"006633",
"006600",
"0033ff",
"0033cc",
"003399",
"003366",
"003333",
"003300",
"0000ff",
"0000cc",
"000099",
"000066",
"000033",
"000000",
/* End of MUST_HAVE colors */
"3300ff",
"3333ff",
"336666",
"336633",
"336600",
"333366",
"33ffff",
"33ffcc",
"33ff99",
"33ff66",
"33ff33",
"33ff00",
"33ccff",
"33cccc",
"33cc99",
"33cc66",
"33cc33",
"33cc00",
"3399ff",
"3399cc",
"339999",
"339966",
"339933",
"339900",
"3366ff",
"3366cc",
"336699",
"3333cc",
"3300cc"
};

int get_safe_colors(Widget wid)
{
	unsigned int r, g, b;
	int i, j, num;
	int failed = 0;
	char *t;
	XColor col;
	Colormap cmap;
	int gotit[216];
	static int safe_colors_done = 0;
#ifdef HAVE_JPEG
	JSAMPROW workspace;
#endif

	/* Already been here */
	if (safe_colors_done)
		return safe_colors_done;

	/* Can't do if less than eight planes */ 
	if (DefaultDepthOfScreen(XtScreen(wid)) < 8) {
		BSCnum = 0;
		return 0;
	}

	cmap = installed_colormap ? installed_cmap :
	       DefaultColormapOfScreen(XtScreen(wid));

#ifdef HAVE_JPEG
	jcolormap = (JSAMPARRAY)malloc(3 * sizeof(JSAMPROW));
	workspace = (JSAMPROW)malloc(3 * 256 * sizeof(JSAMPLE));
	for (i = 0; i < 3; i++) {
		jcolormap[i] = workspace;
		workspace += 256;
	}
#endif
	for (i = 0; i < MUST_HAVE; i++) {
		t = colors_216[i];
		sscanf(t, "%2x%2x%2x", &r, &g, &b);
#ifdef HAVE_JPEG
		jcolormap[0][i] = (JSAMPLE)r;
		jcolormap[1][i] = (JSAMPLE)g;
		jcolormap[2][i] = (JSAMPLE)b;
#endif

		col.red = r << 8;
		col.green = g << 8;
		col.blue = b << 8;
		col.flags = DoRed | DoGreen | DoBlue;
		if (!XAllocColor(dsp, cmap, &col)) {
			failed++;
			gotit[i] = 0;
		} else {
			gotit[i] = col.pixel;
		}
		BSColors[i] = col;
	}
	if (failed) {
		unsigned long pix;

		fprintf(stderr,
			"Unable to allocate enough browser safe colors\n");
		fprintf(stderr, "Could only get %d of %d colors\n",
			MUST_HAVE - failed, MUST_HAVE);
		fprintf(stderr, "Disabling browser safe color support\n");
		/* Free the colors we did get */
		for (i = 0; i < MUST_HAVE; i++) {
			if (gotit[i]) {
				pix = (unsigned long) gotit[i];
				XFreeColors(dsp, cmap,  &pix, 1, 0L);
			}
		}
		BSCnum = 0;	/* Disable dithering */
		return 0;
	}

	BSCnum = MUST_HAVE;
	safe_colors_done = 1;

	/* Now try to get remaining browser safe colors */
	for (i = MUST_HAVE; i < 216; i++) {
		t = colors_216[i];
		sscanf(t, "%2x%2x%2x", &r, &g, &b);
#ifdef HAVE_JPEG
		jcolormap[0][i] = (JSAMPLE)r;
		jcolormap[1][i] = (JSAMPLE)g;
		jcolormap[2][i] = (JSAMPLE)b;
#endif
		col.red = r << 8;
		col.green = g << 8;
		col.blue = b << 8;
		col.flags = DoRed | DoGreen | DoBlue;
		if (!XAllocColor(dsp, cmap, &col)) {
#ifndef DISABLE_TRACE
	                if (srcTrace)
				fprintf(stderr,
					"Only got %d browser safe colors\n",
					BSCnum);
#endif
			break;
		} else {
			BSCnum++;
		}
		BSColors[i] = col;
	}

	/* Get the colors used by the icons */
	for (i = 0; i < 15; i++) {
		t = colors_icons[i];
		sscanf(t, "%2x%2x%2x", &r, &g, &b);
#ifdef HAVE_JPEG
		jcolormap[0][i + 216] = (JSAMPLE)r;
		jcolormap[1][i + 216] = (JSAMPLE)g;
		jcolormap[2][i + 216] = (JSAMPLE)b;
#endif
		col.red = r << 8;
		col.green = g << 8;
		col.blue = b << 8;
		col.flags = DoRed | DoGreen | DoBlue;
		/* Should be already allocated, but need pixel value */
		XAllocColor(dsp, cmap, &col);
		BSColors[BSCnum++] = col;
	}

	safe_colors_done = 2;

	num = BSCnum;
	/* Try to get colors allocated by Motif and other applications */
	for (i = 0; i < (256 - num); i++) {
		failed = 0;
		for (j = 0; j < num; j++) {
			if (BSColors[j].pixel == i) {
				failed++;
				break;
			}
		}
		if (!failed) {
			col.pixel = i;
			XQueryColor(dsp, cmap, &col);
			BSColors[BSCnum] = col;
#ifdef HAVE_JPEG
			jcolormap[0][BSCnum] = (JSAMPLE)(col.red >> 8);
			jcolormap[1][BSCnum] = (JSAMPLE)(col.green >> 8);
			jcolormap[2][BSCnum] = (JSAMPLE)(col.blue >> 8);
#endif
			BSCnum++;
#ifndef DISABLE_TRACE
	                if (srcTrace)
				fprintf(stderr,
				       "COLORS: got pixel %d, RGB = %d %d %d\n",
				       i, col.red, col.green, col.blue);
#endif
		}
	}
#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "COLORS: BSCnum = %d\n", BSCnum);
#endif
	return safe_colors_done;
}

/* Dummy routine to force linker to find FS_Dither for use by LIBHTMLW */
void silly_linker()
{
	FS_Dither();
}
