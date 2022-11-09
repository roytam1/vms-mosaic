/* Copyright (C) 2004, 2005, 2006 - The VMS Mosaic Project */

#include "../config.h"
#ifdef HAVE_JPEG

#include "mosaic.h"
#include "jpeglib.h"
#include "readJPEG.h"
#include <setjmp.h>

#ifndef DISABLE_TRACE
extern int srcTrace;
extern int reportBugs;
#endif

/* Browser safe color map filled in by colors.c */
extern JSAMPARRAY jcolormap;

/* From LIBWWW2 htfwriter.c */
extern char *image_file_fnam;

extern int Vclass;
extern int browserSafeColors;
extern int BSCnum;

struct my_error_mgr {
    struct jpeg_error_mgr pub;	/* "public" fields */
    jmp_buf setjmp_buffer;	/* For return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

/* Fatal error routine */
static void my_read_error_exit(j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

#ifndef DISABLE_TRACE
	if (srcTrace || get_pref_boolean(eJPEG_ERROR_MESSAGES)) {
#else
	if (get_pref_boolean(eJPEG_ERROR_MESSAGES)) {
#endif
		fprintf(stderr, "Error reading JPEG image: ");
		(*cinfo->err->output_message) (cinfo);
	}
	longjmp(myerr->setjmp_buffer, 1);
}

/* Warning message routine */
static void my_read_emit_message(j_common_ptr cinfo, int msg_level)
{
	struct jpeg_error_mgr *err = cinfo->err;	

	/* First warning only */
	if ((msg_level < 0) && (err->num_warnings == 0)) {
		/* Count warnings */
		err->num_warnings++;
#ifndef DISABLE_TRACE
		if (srcTrace || get_pref_boolean(eJPEG_ERROR_MESSAGES)) {
#else
		if (get_pref_boolean(eJPEG_ERROR_MESSAGES)) {
#endif
			fprintf(stderr, "JPEG image warning: ");
			(*cinfo->err->output_message) (cinfo);
		}
	}
}


#ifdef WRITE_JPEG
static void my_write_error_exit(j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

#ifndef DISABLE_TRACE
	if (srcTrace) {
		fprintf(stderr, "Error writing JPEG image: ");
		(*cinfo->err->output_message) (cinfo);
	}
#endif
	longjmp(myerr->setjmp_buffer, 1);
}
#endif


unsigned char *ReadJPEG(FILE *infile, int *width, int *height, XColor *colrs)
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	unsigned char *retBuffer = NULL;  /* Output image buffer */
	unsigned char *r;
	JSAMPROW buffer[1];	/* Row pointer array for read_scanlines */
	int row_stride;		/* Physical row width in output buffer */
	int i;

#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "ReadJPEG: starting\n");
#endif
	/* We set up the normal JPEG error routines, 
	 * then override error_exit and emit_message. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	/* Fatal errors */
	jerr.pub.error_exit = my_read_error_exit;
	/* Warnings */
	jerr.pub.emit_message = my_read_emit_message;

	/* Establish the setjmp return context for my_read_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error. */
    		jpeg_destroy_decompress(&cinfo);
		fclose(infile);

		if (retBuffer)
			free(retBuffer);
		return NULL;
	}

	jpeg_create_decompress(&cinfo);

	jpeg_stdio_src(&cinfo, infile);

	(void) jpeg_read_header(&cinfo, TRUE);

	/* We can ignore the return value from jpeg_read_header since
	 *   (a) suspension is not possible with the stdio data source, and
	 *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	 * See libjpeg.doc for more info.
	 */

  	cinfo.quantize_colors = TRUE; 
	cinfo.two_pass_quantize = TRUE;

	if (browserSafeColors) {
		cinfo.desired_number_of_colors = BSCnum;
		/* Only use our map if it is color */
		if (cinfo.out_color_components == 3) {
			cinfo.actual_number_of_colors = BSCnum;
			cinfo.colormap = jcolormap;
		}
	} else {
		if ((Vclass == TrueColor) || (Vclass == DirectColor)) {
			cinfo.desired_number_of_colors = 256;
		} else {
			/* 8 bit display */
			static int init = 0;
			static int ncolors;

			if (!init) {
			      ncolors = get_pref_int(eCOLORS_PER_INLINED_IMAGE);
			      init = 1;
			}
			/* Use more colors if internal image viewer */
			if (image_file_fnam && (ncolors < 144)) {
			      cinfo.desired_number_of_colors = 144;
			} else {
			      cinfo.desired_number_of_colors = ncolors;
			}
		}
	}

	jpeg_start_decompress(&cinfo);

	row_stride = cinfo.output_width * cinfo.output_components;
	if (!(retBuffer = (unsigned char *) malloc(row_stride *
			  			   cinfo.output_height))) {
		jpeg_destroy_decompress(&cinfo);
#ifndef DISABLE_TRACE
		if (srcTrace || reportBugs)
		       fprintf(stderr, "Couldn't create space for JPEG read\n");
#endif
		return NULL;
	}
#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr,
		        "buffer size is width=%d x height=%d x depth=%d\n",
		        cinfo.output_width, cinfo.output_height, 
		        cinfo.output_components);
#endif

	r = retBuffer;
	while (cinfo.output_scanline < cinfo.output_height) {
		buffer[0] = r;
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		r += row_stride;
	}

	*width = cinfo.output_width;
	*height = cinfo.output_height;

	/* Set up X colormap */
	if (cinfo.out_color_components == 3) {
		for (i = 0; i < cinfo.actual_number_of_colors; i++) {
			colrs[i].red = cinfo.colormap[0][i] << 8;
			colrs[i].green = cinfo.colormap[1][i] << 8;
			colrs[i].blue = cinfo.colormap[2][i] << 8;
			colrs[i].pixel = i;
			colrs[i].flags = DoRed | DoGreen | DoBlue;
		}
	} else {
		for (i = 0; i < cinfo.actual_number_of_colors; i++) {
			colrs[i].red = colrs[i].green = 
				colrs[i].blue = cinfo.colormap[0][i] << 8;
			colrs[i].pixel = i;
			colrs[i].flags = DoRed | DoGreen | DoBlue;
		}
	}
#ifndef DISABLE_TRACE
	if (srcTrace) {
		fprintf(stderr, "cinfo.actual_number_of_colors = %d\n",
			cinfo.actual_number_of_colors);
		fprintf(stderr, "cinfo.out_color_components = %d\n",
			cinfo.out_color_components);
	}
#endif
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

 	return retBuffer;
}

#ifdef WRITE_JPEG
int WriteJPEG(FILE *outfile, unsigned char *image, int width, int height)
{
	struct jpeg_compress_struct cinfo;
	struct my_error_mgr jerr;
	JSAMPROW row_pointer[1];     /* Row pointer array for write_scanlines */
	int row_stride;		     /* Physical row width in output buffer */

#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr, "WriteJPEG: starting\n");
#endif
	/* We set up the normal JPEG error routines, 
	 * then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_write_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error. */
    		jpeg_destroy_compress(&cinfo);
		return 0;
	}

	jpeg_create_compress(&cinfo);

	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = width;       /* Image width and height, in pixels */
	cinfo.image_height = height;
	cinfo.input_components = 3;      /* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB;  /* Colorspace of input image */

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 85, TRUE);    /* Default is 75; 95 is "best" */
	cinfo.comp_info[0].h_samp_factor = 1;  /* Turn off downsampling */
	cinfo.comp_info[0].v_samp_factor = 1;

#ifndef DISABLE_TRACE
	if (srcTrace)
		fprintf(stderr,
		        "buffer size is width=%d x height=%d x depth=%d\n",
		        cinfo.image_width, cinfo.image_height, 
		        cinfo.input_components);
#endif
	jpeg_start_compress(&cinfo, TRUE);

	row_stride = cinfo.image_width * cinfo.input_components;
	while (cinfo.next_scanline < cinfo.image_height) {
	        row_pointer[0] = &image[cinfo.next_scanline * row_stride];
                jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

	(void) jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	return 1;
}
#endif
#endif
