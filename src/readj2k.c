/*
 * Copyright (c) 2002-2007, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2007, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux and Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Modified for use with VMS Mosaic by GEC - 29-May-2007 */

#include "../config.h"
#ifdef HAVE_JPEG

#include "mosaic.h"
#include "readj2k.h"
#include "quantize.h"
#include "openjpeg.h"

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int reportBugs;
#endif

/* -------------------------------------------------------------------------- */

/*
 * Error callback
 */
void error_callback(const char *msg, void *client_data) {
#ifndef DISABLE_TRACE
	if (srcTrace || reportBugs ||
	    get_pref_boolean(eJPEG2000_ERROR_MESSAGES))
#else
	if (get_pref_boolean(eJPEG2000_ERROR_MESSAGES))
#endif
		fprintf(stderr, "Error reading JPEG 2000 image: %s", msg);
	return;
}

/*
 * Warning callback
 */
void warning_callback(const char *msg, void *client_data) {
#ifndef DISABLE_TRACE
	if (srcTrace || reportBugs ||
	    get_pref_boolean(eJPEG2000_ERROR_MESSAGES))
#else
	if (get_pref_boolean(eJPEG2000_ERROR_MESSAGES))
#endif
		fprintf(stderr, "Warning reading JPEG 2000 image: %s", msg);
	return;
}

/*
 * Debug callback
 */
void info_callback(const char *msg, void *client_data) {
#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "Information reading JPEG 2000 image: %s", msg);
#endif
	return;
}

static int int_ceildivpow2(int a, int b) {
	return (a + (1 << b) - 1) >> b;
}

/* -------------------------------------------------------------------------- */

unsigned char *ReadJ2K(FILE *fp, int *width, int *height, XColor *colrs,
		       int type)
{
	opj_dparameters_t parameters;		/* Decompression parameters */
	opj_event_mgr_t event_mgr;		/* Event manager */
	opj_image_t *image = NULL;
	opj_dinfo_t *dinfo = NULL;		/* Handle to a decompressor */
	opj_cio_t *cio = NULL;
	unsigned char *src = NULL;
	unsigned char *data = NULL;
	unsigned char *ptr, *tmp;
	int file_length, w, wr, h, hr, i;

	/* Configure the event callbacks */
	memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
	event_mgr.error_handler = error_callback;
	event_mgr.warning_handler = warning_callback;
	event_mgr.info_handler = info_callback;

	/* Set decoding parameters to default values */
	opj_set_default_decoder_parameters(&parameters);

	parameters.decod_format = type;

	/* Read the input file and put it in memory */
	/* ---------------------------------------- */

	fseek(fp, 0, SEEK_END);
	file_length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	src = (unsigned char *) malloc(file_length);
	fread(src, 1, file_length, fp);

	/* Decode the code-stream */
	/* ---------------------- */

        switch(parameters.decod_format) {
	    case CODEC_J2K:
		/* JPEG-2000 codestream */

		/* Get a decoder handle */
		dinfo = opj_create_decompress(CODEC_J2K);

		/* Catch events using our callbacks and give a local context */
		opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);

		/* Setup the decoder decoding params using user parameters */
		opj_setup_decoder(dinfo, &parameters);

		/* Open a byte stream */
		cio = opj_cio_open((opj_common_ptr)dinfo, src, file_length);

		/* Decode the stream and fill the image structure */
		image = opj_decode(dinfo, cio);
		if (!image) {
#ifndef DISABLE_TRACE
			if (srcTrace || reportBugs)
				fprintf(stderr,
					"ReadJ2K Error: J2K decode failure!\n");
#endif
			goto Failure;
		}
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "Decoded JPEG 2000 J2K image\n");
#endif
		break;

	    case CODEC_JP2:
		/* JPEG 2000 compressed image file */

		dinfo = opj_create_decompress(CODEC_JP2);
		opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);

		/* Setup the decoder decoding params using the current
		 * image and user parameters */
		opj_setup_decoder(dinfo, &parameters);

		cio = opj_cio_open((opj_common_ptr)dinfo, src, file_length);

		image = opj_decode(dinfo, cio);
		if (!image) {
#ifndef DISABLE_TRACE
			if (srcTrace || reportBugs)
				fprintf(stderr,
					"ReadJ2K Error: JP2 decode failure!\n");
#endif
			goto Failure;
		}
#ifndef DISABLE_TRACE
		if (srcTrace)
			fprintf(stderr, "Decoded JPEG 2000 JP2 image\n");
#endif
		break;

	    case CODEC_JPT:
		/* JPEG 2000, JPIP */

		dinfo = opj_create_decompress(CODEC_JPT);
		opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, stderr);

		/* Setup the decoder decoding params using user parameters */
		opj_setup_decoder(dinfo, &parameters);

		cio = opj_cio_open((opj_common_ptr)dinfo, src, file_length);

		image = opj_decode(dinfo, cio);
		if (!image) {
#ifndef DISABLE_TRACE
			if (srcTrace || reportBugs)
				fprintf(stderr,
					"ReadJ2K Error: JPT decode failure!\n");
#endif
			goto Failure;
		}
	}

	/* Close the byte stream */
	opj_cio_close(cio);

	/* Free the memory containing the code-stream */
	free(src);

	/* Create bitmap */
	/* ------------- */

	w = image->comps[0].w;
	wr = int_ceildivpow2(image->comps[0].w, image->comps[0].factor);
	h = image->comps[0].h;
	hr = int_ceildivpow2(image->comps[0].h, image->comps[0].factor);

	*width = wr;
	*height = hr;

	if (!(data = (unsigned char *)malloc(wr * hr * image->numcomps))) {
#ifndef DISABLE_TRACE
		if (srcTrace || reportBugs)
			fprintf(stderr, "ReadJ2K Error: malloc failure!\n");
#endif
		goto Failure;
	}
	ptr = data;
	if (image->numcomps == 3) {
		/* RGB */
		if (w != wr) {
			for (i = 0; i < wr * hr; i++) {
			    *ptr++ = image->comps[0].data[i / wr * w + i % wr];
			    *ptr++ = image->comps[1].data[i / wr * w + i % wr];
			    *ptr++ = image->comps[2].data[i / wr * w + i % wr];
			}
		} else {
			for (i = 0; i < wr * hr; i++) {
			    *ptr++ = image->comps[0].data[i];
			    *ptr++ = image->comps[1].data[i];
			    *ptr++ = image->comps[2].data[i];
			}
		}
		if (tmp = QuantizeImage(data, wr, hr, 256, 1, colrs, 0)) {
			free(data);
			data = tmp;
		} else {
			free(data);
			data = NULL;
		}
	} else if (image->numcomps == 1) {
		/* Greyscale */
		if (w != wr) {
			for (i = 0; i < wr * hr; i++)
			    *ptr++ = image->comps[0].data[i / wr * w + i % wr];
		} else {
			for (i = 0; i < wr * hr; i++)
			    *ptr++ = image->comps[0].data[i];
		}
		for (i = 0; i < 256; i++) {
			colrs[i].red = colrs[i].green = colrs[i].blue = i << 8;
			colrs[i].pixel = i;
			colrs[i].flags = DoRed | DoGreen | DoBlue;
		}
	}

 Failure:
	/* Free remaining structures */
	opj_destroy_decompress(dinfo);

	/* Free image data structure */
	opj_image_destroy(image);

	return data;
}
#endif
