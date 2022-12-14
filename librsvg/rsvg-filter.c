/* vim: set sw=4: -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* 
   rsvg-filter.c: Provides filters
 
   Copyright (C) 2004 Caleb Moore
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
  
   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Author: Caleb Moore <c.moore@student.unsw.edu.au>
*/

#include "config.h"

#include "rsvg-private.h"
#include "rsvg-filter.h"
#include "rsvg-styles.h"
#include "rsvg-image.h"
#include "rsvg-css.h"
#include <string.h>

#include <math.h>

#include "rsvg-art-render.h"
#include "rsvg-art-composite.h"

#define PERFECTBLUR 0

/*************************************************************/
/*************************************************************/

typedef struct
{
	gint x1, y1, x2, y2;
} FPBox;

typedef struct _RsvgFilterPrimitiveOutput RsvgFilterPrimitiveOutput;

struct _RsvgFilterPrimitiveOutput
{
	GdkPixbuf *result;
	FPBox bounds;
	gboolean Rused;
	gboolean Gused;
	gboolean Bused;
	gboolean Aused;
};

typedef struct _RsvgFilterContext RsvgFilterContext;

struct _RsvgFilterContext
{
	gint width, height;
	RsvgFilter *filter;
	GHashTable *results;
	GdkPixbuf *source;
	GdkPixbuf *bg;
	RsvgFilterPrimitiveOutput lastresult;
	double affine[6];
	double paffine[6];
	RsvgDrawingCtx * ctx;
};

typedef struct _RsvgFilterPrimitive RsvgFilterPrimitive;

struct _RsvgFilterPrimitive
{
	RsvgNode super;
	double x, y, width, height;
	GString *in;
	GString *result;
	gboolean sizedefaults;
   
	void (*render) (RsvgFilterPrimitive * self, RsvgFilterContext * ctx);
};

/*************************************************************/
/*************************************************************/

static void
rsvg_filter_primitive_render (RsvgFilterPrimitive * self,
			      RsvgFilterContext * ctx)
{
	self->render (self, ctx);
}

static FPBox
rsvg_filter_primitive_get_bounds (RsvgFilterPrimitive * self,
				  RsvgFilterContext * ctx)
{
	FPBox output;
	int skip;
	skip = 0;	

	if (self == NULL)
		skip = 1;
	else if (self->sizedefaults)
		skip = 1;

	if (skip)
		{
			output.x1 = ctx->affine[0] * ctx->filter->x + ctx->affine[4];
			output.y1 = ctx->affine[3] * ctx->filter->y + ctx->affine[5];
			output.x2 =
				ctx->affine[0] * (ctx->filter->x + ctx->filter->width) +
				ctx->affine[4];
			output.y2 =
				ctx->affine[3] * (ctx->filter->y + ctx->filter->height) +
				ctx->affine[5];

			if (output.x1 < 0)
				output.x1 = 0;
			if (output.x2 > ctx->width)
				output.x2 = ctx->width;
			if (output.y1 < 0)
				output.y1 = 0;
			if (output.y2 > ctx->height)
				output.y2 = ctx->height;		

			return output;
		}
	
	output.x1 = ctx->paffine[0] * self->x + ctx->paffine[4];
	output.y1 = ctx->paffine[3] * self->y + ctx->paffine[5];
	output.x2 = ctx->paffine[0] * (self->x + self->width) + ctx->paffine[4];
	output.y2 = ctx->paffine[3] * (self->y + self->height) + ctx->paffine[5];
	
	if (output.x1 < ctx->affine[0] * ctx->filter->x + ctx->affine[4])
		output.x1 = ctx->affine[0] * ctx->filter->x + ctx->affine[4];
	if (output.x2 >
		ctx->affine[0] * (ctx->filter->x + ctx->filter->width) + ctx->affine[4])
		output.x2 =
			ctx->affine[0] * (ctx->filter->x + ctx->filter->width) + ctx->affine[4];
	if (output.y1 < ctx->affine[3] * ctx->filter->y + ctx->affine[5])
		output.y1 = ctx->affine[3] * ctx->filter->y + ctx->affine[5];
	if (output.y2 > ctx->affine[3] * (ctx->filter->y + ctx->filter->height) +
		ctx->affine[5])
		output.y2 = ctx->affine[3] * (ctx->filter->y + ctx->filter->height) +
			ctx->affine[5];
	
	if (output.x1 < 0)
		output.x1 = 0;
	if (output.x2 > ctx->width)
		output.x2 = ctx->width;
	if (output.y1 < 0)
		output.y1 = 0;
	if (output.y2 > ctx->height)
		output.y2 = ctx->height;
	
	return output;
}

GdkPixbuf *
_rsvg_pixbuf_new_cleared (GdkColorspace colorspace, gboolean has_alpha, int bits_per_sample,
						  int width, int height)
{
	GdkPixbuf *pb;
	guchar *data;

	pb = gdk_pixbuf_new (colorspace, has_alpha, bits_per_sample, width, height);
	data = gdk_pixbuf_get_pixels (pb);
	memset(data, 0, width * height * 4);

	return pb;
}

static guchar
gdk_pixbuf_get_interp_pixel(guchar * src, gdouble ox, gdouble oy, guchar ch, FPBox boundarys, guint rowstride)
{
	double xmod, ymod;
	double dist1, dist2, dist3, dist4;
	double c, c1, c2, c3, c4;

	xmod = fmod(ox, 1.0);
	ymod = fmod(oy, 1.0);

	dist1 = (1 - xmod) * (1 - ymod);
	dist2 = (xmod) * (1 - ymod);
	dist3 = (xmod) * (ymod);
	dist4 = (1 - xmod) * (ymod);

	if (floor(ox) <= boundarys.x1 || floor(ox) >= boundarys.x2 || 
		floor(oy) <= boundarys.y1 || floor(oy) >= boundarys.y2)
		c1 = 0;
	else
		c1 = src[(guint)floor(oy) * rowstride + (guint)floor(ox) * 4 + ch];

	if (ceil(ox) <= boundarys.x1 || ceil(ox) >= boundarys.x2 || 
		floor(oy) <= boundarys.y1 || floor(oy) >= boundarys.y2)
		c2 = 0;
	else
		c2 = src[(guint)floor(oy) * rowstride + (guint)ceil(ox) * 4 + ch];

	if (ceil(ox) <= boundarys.x1 || ceil(ox) >= boundarys.x2 || 
		ceil(oy) <= boundarys.y1 || ceil(oy) >= boundarys.y2)
		c3 = 0;
	else
		c3 = src[(guint)ceil(oy) * rowstride + (guint)ceil(ox) * 4 + ch];
	
	if (floor(ox) <= boundarys.x1 || floor(ox) >= boundarys.x2 || 
		ceil(oy) <= boundarys.y1 || ceil(oy) >= boundarys.y2)
		c4 = 0;
	else
		c4 = src[(guint)ceil(oy) * rowstride + (guint)floor(ox) * 4 + ch];

	c = (c1 * dist1 + c2 * dist2 + c3 * dist3 + c4 * dist4) / (dist1 + dist2 + dist3 + dist4);

	return (guchar)c;
}

static void
rsvg_filter_fix_coordinate_system (RsvgFilterContext * ctx, RsvgState * state)
{
	int x, y, height, width;
	int i;
	guchar *pixels;
	int stride;
	ArtIRect bbox = ((RsvgArtRender *)ctx->ctx->render)->bbox;

	/* First for object bounding box coordinates we need to know how much of the 
	   source has been drawn on */
	pixels = gdk_pixbuf_get_pixels (ctx->source);
	stride = gdk_pixbuf_get_rowstride (ctx->source);

	x = bbox.x0;
	y = bbox.y0;
	width = bbox.x1 - bbox.x0;
	height = bbox.y1 -bbox.y0;

	ctx->width = gdk_pixbuf_get_width (ctx->source);
	ctx->height = gdk_pixbuf_get_height (ctx->source);
	
	if (ctx->filter->filterunits == userSpaceOnUse)
		{
			for (i = 0; i < 6; i++)
				ctx->affine[i] = state->affine[i];
		}
	else
		{
			ctx->affine[0] = width;
			ctx->affine[1] = 0.;
			ctx->affine[2] = 0.;
			ctx->affine[3] = height;
			ctx->affine[4] = x;
			ctx->affine[5] = y;
		}
	
	if (ctx->filter->primitiveunits == userSpaceOnUse)
		{
			for (i = 0; i < 6; i++)
				ctx->paffine[i] = state->affine[i];
		}
	else
		{
			ctx->paffine[0] = width;
			ctx->paffine[1] = 0.;
			ctx->paffine[2] = 0.;
			ctx->paffine[3] = height;
			ctx->paffine[4] = x;
			ctx->paffine[5] = y;
		}
}

static void
rsvg_filter_free_pair (gpointer value)
{
	RsvgFilterPrimitiveOutput * output;

	output = (RsvgFilterPrimitiveOutput *)value;
	g_object_unref (G_OBJECT (output->result));
	g_free (output);
}

/**
 * rsvg_filter_render: Copy the source to the bg using a filter.
 * @self: a pointer to the filter to use
 * @source: a pointer to the source pixbuf
 * @bg: the background pixbuf
 * @context: the context
 *
 * This function will create a context for itself, set up the coordinate systems
 * execute all its little primatives and then clean up its own mess
 **/
void
rsvg_filter_render (RsvgFilter * self, GdkPixbuf * source, GdkPixbuf * output, 
					GdkPixbuf * bg, RsvgDrawingCtx * context)
{
	RsvgFilterContext *ctx;
	RsvgArtRender * render = ((RsvgArtRender *)context->render);
	RsvgFilterPrimitive *current;
	guint i;
	FPBox bounds;
	
	ctx = g_new (RsvgFilterContext, 1);
	ctx->filter = self;
	ctx->source = source;
	ctx->bg = bg;
	ctx->results = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, rsvg_filter_free_pair);
	ctx->ctx = context;	

	g_object_ref (G_OBJECT (source));

	rsvg_filter_fix_coordinate_system (ctx, rsvg_state_current (context));

	ctx->lastresult.result = source;
	ctx->lastresult.Rused = 1;
	ctx->lastresult.Gused = 1;
	ctx->lastresult.Bused = 1;
	ctx->lastresult.Aused = 1;
	ctx->lastresult.bounds = rsvg_filter_primitive_get_bounds (NULL, ctx);	

	for (i = 0; i < self->super.children->len; i++)
		{
			current = g_ptr_array_index (self->super.children, i);
			if (current->super.type == RSVG_NODE_FILTER_PRIMITIVE)
				rsvg_filter_primitive_render (current, ctx);
		}

	g_hash_table_destroy (ctx->results);

	bounds = rsvg_filter_primitive_get_bounds (NULL, ctx);	

	if (render->clippath)
		rsvg_art_clip_image(ctx->lastresult.result, render->clippath);

	rsvg_art_alpha_blt (ctx->lastresult.result, bounds.x1, bounds.y1, bounds.x2 - bounds.x1,
					bounds.y2 - bounds.y1, output, bounds.x1, bounds.y1);
	render->bbox.x0 = bounds.x1;
	render->bbox.y0 = bounds.y1;
	render->bbox.x1 = bounds.x2;
	render->bbox.y1 = bounds.y2;
	g_object_unref (G_OBJECT (ctx->lastresult.result));
	g_free(ctx);
}

/**
 * rsvg_filter_store_result: Files a result into a context.
 * @name: The name of the result
 * @result: The pointer to the result
 * @ctx: the context that this was called in
 *
 * Puts the new result into the hash for easy finding later, also
 * Stores it as the last result
 **/
static void
rsvg_filter_store_output(GString * name, RsvgFilterPrimitiveOutput result,
						  RsvgFilterContext * ctx)
{
	RsvgFilterPrimitiveOutput * store;

	g_object_unref (G_OBJECT (ctx->lastresult.result));
	
	store = g_new(RsvgFilterPrimitiveOutput, 1);
	*store = result;

	if (strcmp (name->str, ""))
		{
			g_object_ref (G_OBJECT (result.result));	/* increments the references for the table */
			g_hash_table_insert (ctx->results, g_strdup (name->str), store);
		}
	
	g_object_ref (G_OBJECT (result.result));	/* increments the references for the last result */
	ctx->lastresult = result;
}

static void
rsvg_filter_store_result(GString * name, GdkPixbuf * result,
						  RsvgFilterContext * ctx)
{
	RsvgFilterPrimitiveOutput output;
	output.Rused = 1;
	output.Gused = 1;
	output.Bused = 1;
	output.Aused = 1;
	output.bounds.x1 = 0;
	output.bounds.y1 = 0;
	output.bounds.x2 = ctx->width;
	output.bounds.y2 = ctx->height;
	output.result = result;

	rsvg_filter_store_output(name, output, ctx);
}

static GdkPixbuf *
pixbuf_get_alpha (GdkPixbuf * pb)
{
	guchar *data;
	guchar *pbdata;
	GdkPixbuf *output;
	
	gsize i, pbsize;

	pbsize = gdk_pixbuf_get_width (pb) * gdk_pixbuf_get_height (pb);

	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8,
									   gdk_pixbuf_get_width (pb),
									   gdk_pixbuf_get_height (pb));
	
	data = gdk_pixbuf_get_pixels (output);
	pbdata = gdk_pixbuf_get_pixels (pb);
	
	for (i = 0; i < pbsize; i++)
		data[i * 4 + 3] = pbdata[i * 4 + 3];
	
	return output;
}

/**
 * rsvg_filter_get_in: Gets a pixbuf for a primative.
 * @name: The name of the pixbuf
 * @ctx: the context that this was called in
 *
 * Returns a pointer to the result that the name refers to, a special
 * Pixbuf if the name is a special keyword or NULL if nothing was found
 **/
static RsvgFilterPrimitiveOutput
rsvg_filter_get_result (GString * name, RsvgFilterContext * ctx)
{
	RsvgFilterPrimitiveOutput output;
	RsvgFilterPrimitiveOutput * outputpointer;

	if (!strcmp (name->str, "SourceGraphic"))
		{
			g_object_ref (G_OBJECT (ctx->source));
			output.result = ctx->source;
			output.Rused = output.Gused = output.Bused = output.Aused = 1;
			return output;
		}
	else if (!strcmp (name->str, "BackgroundImage"))
		{
			g_object_ref (G_OBJECT (ctx->bg));
			output.result = ctx->bg;
			output.Rused = output.Gused = output.Bused = output.Aused = 1;
			return output;
		}
	else if (!strcmp (name->str, "") || !strcmp (name->str, "none"))
		{
			g_object_ref (G_OBJECT (ctx->lastresult.result));
			output = ctx->lastresult;
			return output;
		}
	else if (!strcmp (name->str, "SourceAlpha"))
		{
			output.Rused = output.Gused = output.Bused = 0;
			output.Aused = 1;
		    output.result = pixbuf_get_alpha (ctx->source);
			return output;
		}
	else if (!strcmp (name->str, "BackgroundAlpha"))
		{
			output.Rused = output.Gused = output.Bused = 0;
			output.Aused = 1;
			output.result = pixbuf_get_alpha (ctx->bg);
			return output;
		}	

	outputpointer = (RsvgFilterPrimitiveOutput*)(g_hash_table_lookup (ctx->results, name->str));

	if (outputpointer != NULL)
		{
			output = *outputpointer;		
			g_object_ref (G_OBJECT (output.result));
			return output;
		}

	g_warning(_("%s not found\n"),name->str);
	
	output = ctx->lastresult;
	g_object_ref (G_OBJECT (ctx->lastresult.result));
	return output;
}


static GdkPixbuf *
rsvg_filter_get_in (GString * name, RsvgFilterContext * ctx)
{
	return rsvg_filter_get_result (name, ctx).result;
}

/**
 * rsvg_filter_parse: Looks up an allready created filter.
 * @defs: a pointer to the hash of definitions
 * @str: a string with the name of the filter to be looked up
 *
 * Returns a pointer to the filter that the name refers to, or NULL
 * if none was found
 **/
RsvgFilter *
rsvg_filter_parse (const RsvgDefs * defs, const char *str)
{
	if (!strncmp (str, "url(", 4))
		{
			const char *p = str + 4;
			int ix;
			char *name;
			RsvgNode *val;
			
			while (g_ascii_isspace (*p))
				p++;

			for (ix = 0; p[ix]; ix++)
				if (p[ix] == ')')
					break;
			
			if (p[ix] == ')')
				{
					name = g_strndup (p, ix);
					val = rsvg_defs_lookup (defs, name);
					g_free (name);
					
					if (val && val->type == RSVG_NODE_FILTER)
						return (RsvgFilter *) val;
				}
		}
	
	return NULL;
}

static void
rsvg_filter_set_args (RsvgNode * self, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	const char *value;
	RsvgFilter *filter;
	double font_size;
	
	font_size = rsvg_state_current_font_size (ctx);
	filter = (RsvgFilter *)self;
	
	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "filterUnits")))
				{
					if (!strcmp (value, "userSpaceOnUse"))
						filter->filterunits = userSpaceOnUse;
					else
						filter->filterunits = objectBoundingBox;
				}
			if ((value = rsvg_property_bag_lookup (atts, "primitiveUnits")))
				{
					if (!strcmp (value, "objectBoundingBox"))
						filter->primitiveunits = objectBoundingBox;
					else
						filter->primitiveunits = userSpaceOnUse;
				}
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				filter->x =
					rsvg_css_parse_normalized_length (value,
													  ctx->dpi_x,
													  1,
													  font_size);
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				filter->y =
					rsvg_css_parse_normalized_length (value,
													  ctx->dpi_y,
													  1,
													  font_size);
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				filter->width =
					rsvg_css_parse_normalized_length (value,
													  ctx->dpi_x,
													  1,
													  font_size);
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				filter->height =
					rsvg_css_parse_normalized_length (value,
													  ctx->dpi_y,
													  1,
													  font_size);					
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super);
		}
}

/**
 * rsvg_new_filter: Creates a black filter
 *
 * Creates a blank filter and assigns default values to everything
 **/
RsvgNode *
rsvg_new_filter (void)
{
	RsvgFilter *filter;

	filter = g_new (RsvgFilter, 1);
	_rsvg_node_init(&filter->super);
	filter->filterunits = objectBoundingBox;
	filter->primitiveunits = userSpaceOnUse;
	filter->x = -0.1;
	filter->y = -0.1;
	filter->width = 1.2;
	filter->height = 1.2;
	filter->super.children = g_ptr_array_new ();
	filter->super.type = RSVG_NODE_FILTER;
	filter->super.set_atts = rsvg_filter_set_args;
	return &filter->super;
}

/*************************************************************/
/*************************************************************/

typedef enum
{
	normal, multiply, screen, darken, lighten, softlight, 
	hardlight, colordodge, colorburn, overlay, exclusion,
	difference
}
RsvgFilterPrimitiveBlendMode;

typedef struct _RsvgFilterPrimitiveBlend RsvgFilterPrimitiveBlend;
struct _RsvgFilterPrimitiveBlend
{
	RsvgFilterPrimitive super;
	RsvgFilterPrimitiveBlendMode mode;
	GString *in2;
};

static void rsvg_filter_blend(RsvgFilterPrimitiveBlendMode mode, GdkPixbuf *in, GdkPixbuf *in2, GdkPixbuf *output, FPBox boundarys)
{
	guchar i;
	gint x, y;
	gint rowstride, rowstride2, rowstrideo, height, width;
	guchar *in_pixels;
	guchar *in2_pixels;
	guchar *output_pixels;
	height = gdk_pixbuf_get_height (in);
	width = gdk_pixbuf_get_width (in);
	rowstride = gdk_pixbuf_get_rowstride (in);
	rowstride2 = gdk_pixbuf_get_rowstride (in2);
	rowstrideo = gdk_pixbuf_get_rowstride (output);

	output_pixels = gdk_pixbuf_get_pixels (output);
	in_pixels = gdk_pixbuf_get_pixels (in);
	in2_pixels = gdk_pixbuf_get_pixels (in2);

	if (boundarys.x1 < 0)
		boundarys.x1 = 0;
	if (boundarys.y1 < 0)
		boundarys.y2 = 0;
	if (boundarys.x2 >= width)
		boundarys.x2 = width;
	if (boundarys.y2 >= height)
		boundarys.y2 = height;
		
	for (y = boundarys.y1; y < boundarys.y2; y++)
		for (x = boundarys.x1; x < boundarys.x2; x++)
			{
				double qr, cr, qa, qb, ca, cb, bca, bcb;
				
				qa = (double) in_pixels[4 * x + y * rowstride + 3] / 255.0;
				qb = (double) in2_pixels[4 * x + y * rowstride2 + 3] / 255.0;
				qr = 1 - (1 - qa) * (1 - qb);
				cr = 0;
				for (i = 0; i < 3; i++)
					{
						ca = (double) in_pixels[4 * x + y * rowstride + i] * qa / 255.0;
						cb = (double) in2_pixels[4 * x + y * rowstride2 + i] * qb / 255.0;
						/*these are the ca and cb that are used in the non-standard blend functions*/
						bcb = (1 - qa) * cb + ca;
						bca = (1 - qb) * ca + cb;
						switch (mode)
							{
							case normal:
								cr = (1 - qa) * cb + ca;
								break;
							case multiply:
								cr = (1 - qa) * cb + (1 - qb) * ca + ca * cb;
								break;
							case screen:
								cr = cb + ca - ca * cb;
								break;
							case darken:
								cr = MIN ((1 - qa) * cb + ca, (1 - qb) * ca + cb);
								break;
							case lighten:
								cr = MAX ((1 - qa) * cb + ca, (1 - qb) * ca + cb);
								break;
							case softlight:
								if (bcb < 0.5)
									cr = 2 * bca * bcb + bca * bca * (1 - 2 * bcb);
								else
									cr = sqrt(bca)*(2*bcb-1)+(2*bca)*(1-bcb);
								break;
							case hardlight:
								if (cb < 0.5)
									cr = 2 * bca * bcb;
								else
									cr = 1 - 2 * (1 - bca) * (1 - bcb);
								break;
							case colordodge:
								if (bcb == 1)
									cr = 1;
								else
									cr = MIN(bca / (1 - bcb), 1);
								break;
							case colorburn:
								if (bcb == 0)
									cr = 0;
								else
									cr = MAX(1 - (1 - bca) / bcb, 0);
								break;
							case overlay:
								if (bca < 0.5)
									cr = 2 * bca * bcb;
								else
									cr = 1 - 2 * (1 - bca) * (1 - bcb);
								break;
							case exclusion:
								cr = bca + bcb - 2 * bca * bcb;
								break;
							case difference:
								cr = abs(bca - bcb);
								break;
							}
						cr *= 255.0 / qr;
						if (cr > 255)
							cr = 255;
						if (cr < 0)
							cr = 0;
						output_pixels[4 * x + y * rowstrideo + i] = (guchar) cr;
						
					}
				output_pixels[4 * x + y * rowstrideo + 3] = qr * 255.0;
			}
}


static void
rsvg_filter_primitive_blend_render (RsvgFilterPrimitive * self,
									RsvgFilterContext * ctx)
{
	FPBox boundarys;
	
	RsvgFilterPrimitiveBlend *upself;
	
	GdkPixbuf *output;
	GdkPixbuf *in;
	GdkPixbuf *in2;
	
	upself = (RsvgFilterPrimitiveBlend *) self;
	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	in = rsvg_filter_get_in (self->in, ctx);
	in2 = rsvg_filter_get_in (upself->in2, ctx);
	
	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, gdk_pixbuf_get_width (in), gdk_pixbuf_get_height (in));
	
	rsvg_filter_blend(upself->mode, in, in2, output, boundarys);

	rsvg_filter_store_result (self->result, output, ctx);
	
	g_object_unref (G_OBJECT (in));
	g_object_unref (G_OBJECT (in2));
	g_object_unref (G_OBJECT (output));
}

void rsvg_filter_adobe_blend(gint modenum, GdkPixbuf *in, GdkPixbuf *bg, GdkPixbuf *output, RsvgDrawingCtx * ctx)
{
	FPBox boundarys;
	RsvgFilterPrimitiveBlendMode mode;

	boundarys.x1 = ((RsvgArtRender *)ctx->render)->bbox.x0;
	boundarys.y1 = ((RsvgArtRender *)ctx->render)->bbox.y0;
	boundarys.x2 = ((RsvgArtRender *)ctx->render)->bbox.x1;
	boundarys.y2 = ((RsvgArtRender *)ctx->render)->bbox.y1;

	mode = normal;

	switch(modenum)
		{
		case 0:
			mode = normal;
			break;
		case 1:
			mode = multiply; 
			break;
		case 2:
			mode = screen;
			break;
		case 3:
			mode = darken; 
			break;
		case 4:
			mode = lighten;
			break;
		case 5:
			mode = softlight; 
			break;
		case 6:
			mode = hardlight;
			break;
		case 7:
			mode = colordodge; 
			break;
		case 8:
			mode = colorburn;
			break;
		case 9:
			mode = overlay; 
			break;
		case 10:
			mode = exclusion;
			break;
		case 11:
			mode = difference; 
			break;
		}

	rsvg_filter_blend(mode, in, bg, output, boundarys);
}

static void
rsvg_filter_primitive_blend_free (RsvgNode * self)
{
	RsvgFilterPrimitiveBlend *upself;	
	upself = (RsvgFilterPrimitiveBlend *) self;
	g_string_free (upself->super.result, TRUE);
	g_string_free (upself->super.in, TRUE);
	g_string_free (upself->in2, TRUE);
	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_blend_set_atts (RsvgNode * node, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	const char *value;
	double font_size;
	RsvgFilterPrimitiveBlend *filter;
	
	font_size = rsvg_state_current_font_size (ctx);

	filter = (RsvgFilterPrimitiveBlend *)node;
	
	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "mode")))
			{
				if (!strcmp (value, "multiply"))
					filter->mode = multiply;
				else if (!strcmp (value, "screen"))
					filter->mode = screen;
				else if (!strcmp (value, "darken"))
					filter->mode = darken;
				else if (!strcmp (value, "lighten"))
					filter->mode = lighten;
				else
					filter->mode = normal;
			}
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (filter->super.in, value);					
			if ((value = rsvg_property_bag_lookup (atts, "in2")))
				g_string_assign (filter->in2, value);					
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);					
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
			{
				filter->super.height =
					rsvg_css_parse_normalized_length (value,
													  ctx->dpi_y,
													  1,
													  font_size);
				filter->super.sizedefaults = 0;
			}
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}
}

RsvgNode *
rsvg_new_filter_primitive_blend (void)
{
	RsvgFilterPrimitiveBlend *filter;
	filter = g_new (RsvgFilterPrimitiveBlend, 1);
	_rsvg_node_init(&filter->super.super);
	filter->mode = normal;
	filter->super.in = g_string_new ("none");
	filter->in2 = g_string_new ("none");
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;
	filter->super.render = &rsvg_filter_primitive_blend_render;
	filter->super.super.free = &rsvg_filter_primitive_blend_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_blend_set_atts;
	return (RsvgNode *)filter;
}

/*************************************************************/
/*************************************************************/

typedef struct _RsvgFilterPrimitiveConvolveMatrix RsvgFilterPrimitiveConvolveMatrix;

struct _RsvgFilterPrimitiveConvolveMatrix
{
	RsvgFilterPrimitive super;
	double *KernelMatrix;
	double divisor;
	gint orderx, ordery;
	double dx, dy;
	double bias;
	gint targetx, targety;
	gboolean preservealpha;
	gint edgemode;
};

static void
rsvg_filter_primitive_convolve_matrix_render (RsvgFilterPrimitive * self,
											  RsvgFilterContext * ctx)
{
	guchar ch;
	gint x, y;
	gint i, j;
	gint rowstride, height, width;
	FPBox boundarys;
	
	guchar *in_pixels;
	guchar *output_pixels;
	
	RsvgFilterPrimitiveConvolveMatrix *upself;
	
	GdkPixbuf *output;
	GdkPixbuf *in;
	
	gint sx, sy, kx, ky;
	guchar sval;
	double kval, sum, dx, dy, targetx, targety;
	
	gint tempresult;
	
	upself = (RsvgFilterPrimitiveConvolveMatrix *) self;
	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	in = rsvg_filter_get_in (self->in, ctx);
	in_pixels = gdk_pixbuf_get_pixels (in);
	
	height = gdk_pixbuf_get_height (in);
	width = gdk_pixbuf_get_width (in);
	
	targetx = upself->targetx * ctx->paffine[0];
	targety = upself->targety * ctx->paffine[3];

	if (upself->dx != 0 || upself->dy != 0)
		{
			dx = upself->dx * ctx->paffine[0];
			dy = upself->dy * ctx->paffine[3];
		}
	else
		dx = dy = 1;

	rowstride = gdk_pixbuf_get_rowstride (in);
	
	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, width, height);
	output_pixels = gdk_pixbuf_get_pixels (output);
	
	for (y = boundarys.y1; y < boundarys.y2; y++)
		for (x = boundarys.x1; x < boundarys.x2; x++)
			{
				for (ch = 0; ch < 3 + !upself->preservealpha; ch++)
					{
						sum = 0;
						for (i = 0; i < upself->ordery; i++)
							for (j = 0; j < upself->orderx; j++)
								{
									sx = x - targetx + j * dx;
									sy = y - targety + i * dy;
									if (upself->edgemode == 0)
										{
											if (sx < boundarys.x1)
												sx = boundarys.x1;
											if (sx >= boundarys.x2)
												sx = boundarys.x2 - 1;
											if (sy < boundarys.y1)
												sy = boundarys.y1;
											if (sy >= boundarys.y2)
												sy = boundarys.y2 - 1;
										}
									else if (upself->edgemode == 1)
										{
											if (sx < boundarys.x1 || (sx >= boundarys.x2))
												sx = boundarys.x1 + (sx - boundarys.x1) %
													(boundarys.x2 - boundarys.x1);
											if (sy < boundarys.y1 || (sy >= boundarys.y2))
												sy = boundarys.y1 + (sy - boundarys.y1) %
													(boundarys.y2 - boundarys.y1);
										}
									else if (upself->edgemode == 2)
										if (sx < boundarys.x1 || (sx >= boundarys.x2) || 
											sy < boundarys.y1 || (sy >= boundarys.y2))
										continue;

									kx = upself->orderx - j - 1;
									ky = upself->ordery - i - 1;
									sval = in_pixels[4 * sx + sy * rowstride + ch];
									kval = upself->KernelMatrix[kx + ky * upself->orderx];
									sum += (double) sval *kval;
								}
						tempresult = sum / upself->divisor + upself->bias;

						if (tempresult > 255)
							tempresult = 255;
						if (tempresult < 0)
							tempresult = 0;
						
						output_pixels[4 * x + y * rowstride + ch] = tempresult;
					}
				if (upself->preservealpha)
					output_pixels[4 * x + y * rowstride + 3] =
						in_pixels[4 * x + y * rowstride + 3];
			}
	rsvg_filter_store_result (self->result, output, ctx);
	
	g_object_unref (G_OBJECT (in));
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_convolve_matrix_free (RsvgNode * self)
{
	RsvgFilterPrimitiveConvolveMatrix *upself;

	upself = (RsvgFilterPrimitiveConvolveMatrix *) self;
	g_string_free (upself->super.result, TRUE);
	g_string_free (upself->super.in, TRUE);
	g_free (upself->KernelMatrix);
	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_convolve_matrix_set_atts (RsvgNode * self,
												RsvgHandle * ctx,
												RsvgPropertyBag * atts)
{
	gint i, j;
	guint listlen;
	double font_size;
	const char *value;
	gboolean has_target_x, has_target_y;
	RsvgFilterPrimitiveConvolveMatrix *filter;
	
	font_size = rsvg_state_current_font_size (ctx);
	filter = (RsvgFilterPrimitiveConvolveMatrix*)self;
	has_target_x = 0;
	has_target_y = 0;

	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (filter->super.in, value);
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "targetX")))
				{
					has_target_x = 1;
					filter->targetx = atoi (value);
				}
			if ((value = rsvg_property_bag_lookup (atts, "targetY")))
				{
					has_target_y = 1;
					filter->targety = atoi (value);
				}
			if ((value = rsvg_property_bag_lookup (atts, "bias")))
				filter->bias = atof (value);
			if ((value = rsvg_property_bag_lookup (atts, "preserveAlpha")))
				{
					if (!strcmp (value, "true"))
						filter->preservealpha = TRUE;
					else
						filter->preservealpha = FALSE;
				}
			if ((value = rsvg_property_bag_lookup (atts, "divisor")))
				filter->divisor = atof (value);					
			if ((value = rsvg_property_bag_lookup (atts, "order")))
				{
					double tempx, tempy;
					rsvg_css_parse_number_optional_number (value,
														   &tempx, &tempy);
					filter->orderx = tempx;
					filter->ordery = tempy;
					
				}
			if ((value = rsvg_property_bag_lookup (atts, "kernelUnitLength")))
				rsvg_css_parse_number_optional_number (value,
													   &filter->dx, &filter->dy);
							
			if ((value = rsvg_property_bag_lookup (atts, "kernelMatrix")))
				filter->KernelMatrix =
					rsvg_css_parse_number_list (value, &listlen);
			
			if ((value = rsvg_property_bag_lookup (atts, "edgeMode"))) 
				{
					if (!strcmp (value, "wrap"))
						filter->edgemode = 1;
					else if (!strcmp (value, "none"))
						filter->edgemode = 2;
					else
						filter->edgemode = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}

	if (filter->divisor == 0)
		{
			for (j = 0; j < filter->orderx; j++)
				for (i = 0; i < filter->ordery; i++)
					filter->divisor += filter->KernelMatrix[j + i * filter->orderx];
		}

	if (filter->divisor == 0)
		filter->divisor = 1;
		
	if ((gint)listlen < filter->orderx * filter->ordery)
		filter->orderx = filter->ordery = 0;

	if (!has_target_x)
		{
			filter->targetx = floor(filter->orderx / 2);
		}
	if (!has_target_y)
		{
			filter->targety = floor(filter->ordery / 2);
		}
}

RsvgNode *
rsvg_new_filter_primitive_convolve_matrix(void)
{
	RsvgFilterPrimitiveConvolveMatrix *filter;
	filter = g_new (RsvgFilterPrimitiveConvolveMatrix, 1);	
	_rsvg_node_init(&filter->super.super);
	filter->super.in = g_string_new ("none");
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;		
	filter->divisor = 0;
	filter->bias = 0;
	filter->dx = 0;
	filter->dy = 0;
	filter->preservealpha = FALSE;	
	filter->edgemode = 0;
	filter->super.render = &rsvg_filter_primitive_convolve_matrix_render;
	filter->super.super.free = &rsvg_filter_primitive_convolve_matrix_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_convolve_matrix_set_atts;
	return (RsvgNode *)filter;
}

/*************************************************************/
/*************************************************************/

typedef struct _RsvgFilterPrimitiveGaussianBlur
RsvgFilterPrimitiveGaussianBlur;

struct _RsvgFilterPrimitiveGaussianBlur
{
	RsvgFilterPrimitive super;
	double sdx, sdy;
};

static void
box_blur (GdkPixbuf *in, GdkPixbuf *output, GdkPixbuf *intermediate, gint kw, 
		  gint kh, FPBox boundarys, RsvgFilterPrimitiveOutput op)
{
	gint ch;
	gint x, y;
	gint rowstride, height, width;
	
	guchar *in_pixels;
	guchar *output_pixels;

	gint sum;	

	gint divisor;

	
	height = gdk_pixbuf_get_height (in);
	width = gdk_pixbuf_get_width (in);

	in_pixels = gdk_pixbuf_get_pixels (in);
	output_pixels = gdk_pixbuf_get_pixels (intermediate);
	
	rowstride = gdk_pixbuf_get_rowstride (in);

	if (kw > boundarys.x2 - boundarys.x1)
		kw = boundarys.x2 - boundarys.x1;

	if (kh > boundarys.y2 - boundarys.y1)
		kh = boundarys.y2 - boundarys.y1;


	if (kw >= 1)	
		{
			for (ch = 0; ch < 4; ch++)
				{
					switch (ch)
						{	
						case 0:
							if (!op.Rused)
								continue;
						case 1:
							if (!op.Gused)
								continue;						
						case 2:
							if (!op.Bused)
								continue;
						case 3:
							if (!op.Aused)
								continue;
						}
					for (y = boundarys.y1; y < boundarys.y2; y++)
						{
							sum = 0;
							divisor = 0;
							for (x = boundarys.x1; x < boundarys.x1 + kw; x++)
								{
									if (ch != 3)
										{
											divisor += in_pixels[4 * x + y * rowstride + 3];
											sum += in_pixels[4 * x + y * rowstride + ch] * in_pixels[4 * x + y * rowstride + 3];
										}
									else
										{
											divisor++;
											sum += in_pixels[4 * x + y * rowstride + ch];
										}

									if (x - kw / 2 >= 0 && x - kw / 2 < boundarys.x2)
										{
											if (divisor > 0)
												output_pixels[4 * (x - kw / 2) + y * rowstride + ch] = sum / divisor;
										}
								}
							for (x = boundarys.x1 + kw; x < boundarys.x2; x++)
								{
									if (ch != 3)
										{
											divisor += in_pixels[4 * x + y * rowstride + 3];
											divisor -= in_pixels[4 * (x - kw) + y * rowstride + 3];
											sum -= in_pixels[4 * (x - kw) + y * rowstride + ch] * 
												in_pixels[4 * (x - kw) + y * rowstride + 3];
											sum += in_pixels[4 * x + y * rowstride + ch] * 
												in_pixels[4 * x + y * rowstride + 3];
										}
									else
										{
											sum -= in_pixels[4 * (x - kw) + y * rowstride + ch];
											sum += in_pixels[4 * x + y * rowstride + ch];
										}
									if (divisor > 0)
										output_pixels[4 * (x - kw / 2) + y * rowstride + ch] = sum / divisor;
								}
							for (x = boundarys.x2; x < boundarys.x2 + kw; x++)
								{
									if (ch != 3)
										{
											divisor -= in_pixels[4 * (x - kw) + y * rowstride + 3];
											sum -= in_pixels[4 * (x - kw) + y * rowstride + ch]* 
												in_pixels[4 * (x - kw) + y * rowstride + 3];
										}									
									else
										{
											divisor--;
											sum -= in_pixels[4 * (x - kw) + y * rowstride + ch];
										}
									if (x - kw / 2 >= 0 && x - kw / 2 < boundarys.x2)
										{
											if (divisor > 0)
												output_pixels[4 * (x - kw / 2) + y * rowstride + ch] = sum / divisor;
										}
								}
						}
				}
		}
	else
		intermediate = in;
	
	in_pixels = gdk_pixbuf_get_pixels (intermediate);
	output_pixels = gdk_pixbuf_get_pixels (output);

	if (kh >= 1)
		{
			for (ch = 0; ch < 4; ch++)
				{
					switch (ch)
						{	
						case 0:
							if (!op.Rused)
								continue;
						case 1:
							if (!op.Gused)
								continue;						
						case 2:
							if (!op.Bused)
								continue;
						case 3:
							if (!op.Aused)
								continue;
						}
				

					for (x = boundarys.x1; x < boundarys.x2; x++)
						{
							sum = 0;
							divisor = 0;
							
							for (y = boundarys.y1; y < boundarys.y1 + kh; y++)
								{
									if (ch != 3)
										{
											divisor += in_pixels[4 * x + y * rowstride + 3];
											sum += in_pixels[4 * x + y * rowstride + ch] *
												in_pixels[4 * x + y * rowstride + 3];
										}
									else
										{
											divisor++;
											sum += in_pixels[4 * x + y * rowstride + ch];
										}
									if (y - kh / 2 >= 0 && y - kh / 2 < boundarys.y2)
										{
											if (divisor > 0)
												output_pixels[4 * x + (y - kh / 2) * rowstride + ch] = sum / divisor;
										}
								}
							for (; y < boundarys.y2; y++)
								{
									if (ch != 3)
										{
											divisor += in_pixels[4 * x + y * rowstride + 3];
											divisor -= in_pixels[4 * x + (y - kh) * rowstride + 3];
											sum -= in_pixels[4 * x + (y - kh) * rowstride + ch] *
												in_pixels[4 * x + (y - kh) * rowstride + 3];
											sum += in_pixels[4 * x + y * rowstride + ch] *
												in_pixels[4 * x + y * rowstride + 3];
										}
									else
										{
											sum -= in_pixels[4 * x + (y - kh) * rowstride + ch];
											sum += in_pixels[4 * x + y * rowstride + ch];
										}											
									if (divisor > 0)
										output_pixels[4 * x + (y - kh / 2) * rowstride + ch] = sum / divisor;
								}
							for (; y < boundarys.y2 + kh; y++)
								{
									if (ch != 3)
										{								
											divisor -= in_pixels[4 * x + (y - kh) * rowstride + 3];
											sum -= in_pixels[4 * x + (y - kh) * rowstride + ch] *
												in_pixels[4 * x + (y - kh) * rowstride + 3];
										}
									else
										{
											divisor--;
											sum -= in_pixels[4 * x + (y - kh) * rowstride + ch];
										}
									if (y - kh / 2 >= 0 && y - kh / 2 < boundarys.y2)
										{
											if (divisor > 0)
												output_pixels[4 * x + (y - kh / 2) * rowstride + ch] = sum / divisor;
										}
								}
						}
				}
			
		}
	else
		{
			gdk_pixbuf_copy_area(intermediate, 0,0, width, height, 
								 output, 0, 0);
		}

}

static void
fast_blur (GdkPixbuf *in, GdkPixbuf *output, gfloat sx, 
		   gfloat sy, FPBox boundarys, RsvgFilterPrimitiveOutput op)
{
	GdkPixbuf *intermediate1;
	GdkPixbuf *intermediate2;
	gint kx, ky;

	kx = floor(sx * 3*sqrt(2*M_PI)/4 + 0.5);
	ky = floor(sy * 3*sqrt(2*M_PI)/4 + 0.5);

	intermediate1 = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, 
											gdk_pixbuf_get_width (in),
											gdk_pixbuf_get_height (in));
	intermediate2 = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, 
											gdk_pixbuf_get_width (in),
											gdk_pixbuf_get_height (in));

	box_blur (in, intermediate2, intermediate1, kx, 
			  ky, boundarys, op);
	box_blur (intermediate2, intermediate2, intermediate1, kx, 
			  ky, boundarys, op);
	box_blur (intermediate2, output, intermediate1, kx, 
			  ky, boundarys, op);

	g_object_unref (G_OBJECT (intermediate1));
	g_object_unref (G_OBJECT (intermediate2));
}

static void
rsvg_filter_primitive_gaussian_blur_render (RsvgFilterPrimitive * self,
											RsvgFilterContext * ctx)
{
	RsvgFilterPrimitiveGaussianBlur *upself;
	
	GdkPixbuf *output;
	GdkPixbuf *in;
	FPBox boundarys;
	gfloat sdx, sdy;
	RsvgFilterPrimitiveOutput op;
	
	upself = (RsvgFilterPrimitiveGaussianBlur *) self;
	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	op = rsvg_filter_get_result (self->in, ctx);
	in = op.result;
	
	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, 
									 gdk_pixbuf_get_width (in),
									 gdk_pixbuf_get_height (in));
	
	/* scale the SD values */
	sdx = upself->sdx * ctx->paffine[0];
	sdy = upself->sdy * ctx->paffine[3];
	
	fast_blur (in, output, sdx, 
				   sdy, boundarys, op);

	op.result = output;
	op.bounds = boundarys;
	rsvg_filter_store_output (self->result, op, ctx);
	
	g_object_unref (G_OBJECT (in));
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_gaussian_blur_free (RsvgNode * self)
{
	RsvgFilterPrimitiveGaussianBlur *upself;
	
	upself = (RsvgFilterPrimitiveGaussianBlur *) self;
	g_string_free (upself->super.result, TRUE);
	g_string_free (upself->super.in, TRUE);
	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_gaussian_blur_set_atts (RsvgNode * self,
											  RsvgHandle * ctx,
											  RsvgPropertyBag * atts)
{
	const char *value;
	double font_size;
	RsvgFilterPrimitiveGaussianBlur *filter;

	font_size = rsvg_state_current_font_size (ctx);
	
	filter = (RsvgFilterPrimitiveGaussianBlur *)self;
	
	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (filter->super.in, value);					
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "stdDeviation")))
				rsvg_css_parse_number_optional_number (value,
													   &filter->sdx,
													   &filter->sdy);
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}
}

RsvgNode *
rsvg_new_filter_primitive_gaussian_blur(void)
{
	RsvgFilterPrimitiveGaussianBlur *filter;
	filter = g_new (RsvgFilterPrimitiveGaussianBlur, 1);
	_rsvg_node_init(&filter->super.super);
	filter->super.in = g_string_new ("none");
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;
	filter->sdx = 0;
	filter->sdy = 0;
	filter->super.render = &rsvg_filter_primitive_gaussian_blur_render;
	filter->super.super.free = &rsvg_filter_primitive_gaussian_blur_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_gaussian_blur_set_atts;
	return (RsvgNode *)filter;
}

/*************************************************************/
/*************************************************************/

typedef struct _RsvgFilterPrimitiveOffset RsvgFilterPrimitiveOffset;

struct _RsvgFilterPrimitiveOffset
{
	RsvgFilterPrimitive super;
	gint dx, dy;
};

static void
rsvg_filter_primitive_offset_render (RsvgFilterPrimitive * self,
									 RsvgFilterContext * ctx)
{
	guchar ch;
	gint x, y;
	gint rowstride, height, width;
	FPBox boundarys;
	
	guchar *in_pixels;
	guchar *output_pixels;
	
	RsvgFilterPrimitiveOutput out;
	RsvgFilterPrimitiveOffset *upself;
	
	GdkPixbuf *output;
	GdkPixbuf *in;
	
	int ox, oy;
	
	upself = (RsvgFilterPrimitiveOffset *) self;
	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	in = rsvg_filter_get_in (self->in, ctx);
	in_pixels = gdk_pixbuf_get_pixels (in);
	
	height = gdk_pixbuf_get_height (in);
	width = gdk_pixbuf_get_width (in);
	
	rowstride = gdk_pixbuf_get_rowstride (in);
	
	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, width, height);
	
	output_pixels = gdk_pixbuf_get_pixels (output);
	
	ox = ctx->paffine[0] * upself->dx + ctx->paffine[2] * upself->dy;
	oy = ctx->paffine[1] * upself->dx + ctx->paffine[3] * upself->dy;
	
	for (y = boundarys.y1; y < boundarys.y2; y++)
		for (x = boundarys.x1; x < boundarys.x2; x++)
			{
				if (x - ox < boundarys.x1 || x - ox >= boundarys.x2)
					continue;
				if (y - oy < boundarys.y1 || y - oy >= boundarys.y2)
					continue;
		
				for (ch = 0; ch < 4; ch++)
					{
						output_pixels[y * rowstride + x * 4 + ch] =
							in_pixels[(y - oy) * rowstride + (x - ox) * 4 + ch];
					}
			}

	out.result = output;
	out.Rused = 1;
	out.Gused = 1;
	out.Bused = 1;
	out.Aused = 1;
	out.bounds = boundarys;

	rsvg_filter_store_output (self->result, out, ctx);
	
	g_object_unref (G_OBJECT (in));
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_offset_free (RsvgNode * self)
{
	RsvgFilterPrimitiveOffset *upself;
	
	upself = (RsvgFilterPrimitiveOffset *) self;
	g_string_free (upself->super.result, TRUE);
	g_string_free (upself->super.in, TRUE);
	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_offset_set_atts (RsvgNode * self, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	const char *value;
	double font_size;
	RsvgFilterPrimitiveOffset *filter;
	
	font_size = rsvg_state_current_font_size (ctx);
	
	filter = (RsvgFilterPrimitiveOffset *)self;
	
	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (filter->super.in, value);
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "dx")))
				filter->dx =
					rsvg_css_parse_normalized_length (value,
													  ctx->dpi_x,
													  1,
													  font_size);
			if ((value = rsvg_property_bag_lookup (atts, "dy")))
				filter->dy =
					rsvg_css_parse_normalized_length (value,
													  ctx->dpi_y,
															  1,
													  font_size);
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}
}

RsvgNode *
rsvg_new_filter_primitive_offset (void)
{
	RsvgFilterPrimitiveOffset *filter;
	filter = g_new (RsvgFilterPrimitiveOffset, 1);	
	_rsvg_node_init(&filter->super.super);
	filter->super.in = g_string_new ("none");
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;
	filter->dy = 0;
	filter->dx = 0;
	filter->super.render = &rsvg_filter_primitive_offset_render;
	filter->super.super.free = &rsvg_filter_primitive_offset_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_offset_set_atts;
	return (RsvgNode *)filter;
}

/*************************************************************/
/*************************************************************/

typedef struct _RsvgFilterPrimitiveMerge RsvgFilterPrimitiveMerge;

struct _RsvgFilterPrimitiveMerge
{
	RsvgFilterPrimitive super;
};

static void
rsvg_filter_primitive_merge_render (RsvgFilterPrimitive * self,
									RsvgFilterContext * ctx)
{
	guint i;
	FPBox boundarys;
	
	RsvgFilterPrimitiveMerge *upself;
	
	GdkPixbuf *output;
	GdkPixbuf *in;
	
	upself = (RsvgFilterPrimitiveMerge *) self;
	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, ctx->width, ctx->height);
	
	for (i = 0; i < upself->super.super.children->len; i++)
		{
			RsvgFilterPrimitive * mn;
			mn = g_ptr_array_index (upself->super.super.children, i);
			if (mn->super.type != RSVG_NODE_FILTER_PRIMITIVE_MERGE_NODE)
				continue;
			in = rsvg_filter_get_in (mn->in, ctx);
			rsvg_art_alpha_blt (in, boundarys.x1, boundarys.y1, boundarys.x2 - boundarys.x1,
								boundarys.y2 - boundarys.y1, output, boundarys.x1,
								boundarys.y1);
			g_object_unref (G_OBJECT (in));
		}
	
	rsvg_filter_store_result (self->result, output, ctx);
	
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_merge_free (RsvgNode * self)
{
	RsvgFilterPrimitiveMerge *upself;
	
	upself = (RsvgFilterPrimitiveMerge *) self;
	g_string_free (upself->super.result, TRUE);
	
	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_merge_set_atts (RsvgNode * self, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	const char *value;
	double font_size;
	RsvgFilterPrimitiveMerge *filter;
	
	font_size = rsvg_state_current_font_size (ctx);

	filter = (RsvgFilterPrimitiveMerge *)self;

	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}
}

RsvgNode *
rsvg_new_filter_primitive_merge (void)
{
	RsvgFilterPrimitiveMerge *filter;
	filter = g_new (RsvgFilterPrimitiveMerge, 1);	
	_rsvg_node_init(&filter->super.super);
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;
	filter->super.render = &rsvg_filter_primitive_merge_render;
	filter->super.super.free = &rsvg_filter_primitive_merge_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_merge_set_atts;
	filter->super.super.children = g_ptr_array_new();
	return (RsvgNode *)filter;
}

static void
rsvg_filter_primitive_merge_node_set_atts (RsvgNode * self,
										   RsvgHandle * ctx,
										   RsvgPropertyBag * atts)
{
	const char * value;
	if (rsvg_property_bag_size (atts))
		{
			/* see bug 145149 - sodipodi generates bad SVG... */
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (((RsvgFilterPrimitive *)self)->in, value);
		}
}

static void
rsvg_filter_primitive_merge_node_free (RsvgNode * self)
{
	RsvgFilterPrimitive *upself;
	upself = (RsvgFilterPrimitive *) self;
	g_string_free (upself->in, TRUE);
	_rsvg_node_free(self);
}

RsvgNode *
rsvg_new_filter_primitive_merge_node (void)
{
	RsvgFilterPrimitive *filter;
	filter = g_new (RsvgFilterPrimitive, 1);
	_rsvg_node_init(&filter->super);
	filter->in = g_string_new ("none");
	filter->super.free = rsvg_filter_primitive_merge_node_free;
	filter->super.set_atts = rsvg_filter_primitive_merge_node_set_atts;
	filter->super.type = RSVG_NODE_FILTER_PRIMITIVE_MERGE_NODE;
	return &filter->super;
}

/*************************************************************/
/*************************************************************/

typedef struct _RsvgFilterPrimitiveColourMatrix
RsvgFilterPrimitiveColourMatrix;

struct _RsvgFilterPrimitiveColourMatrix
{
	RsvgFilterPrimitive super;
	double *KernelMatrix;
	double divisor;
	gint orderx, ordery;
	double bias;
	gint targetx, targety;
	gboolean preservealpha;
};

static void
rsvg_filter_primitive_colour_matrix_render (RsvgFilterPrimitive * self,
											RsvgFilterContext * ctx)
{
	guchar ch;
	gint x, y;
	gint i;
	gint rowstride, height, width;
	FPBox boundarys;
	
	guchar *in_pixels;
	guchar *output_pixels;
	
	RsvgFilterPrimitiveColourMatrix *upself;
	
	GdkPixbuf *output;
	GdkPixbuf *in;
	
	double sum;
	
	gint tempresult;

	upself = (RsvgFilterPrimitiveColourMatrix *) self;
	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	in = rsvg_filter_get_in (self->in, ctx);
	in_pixels = gdk_pixbuf_get_pixels (in);
	
	height = gdk_pixbuf_get_height (in);
	width = gdk_pixbuf_get_width (in);
	
	rowstride = gdk_pixbuf_get_rowstride (in);
	
	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, width, height);	
	output_pixels = gdk_pixbuf_get_pixels (output);   
	
	for (y = boundarys.y1; y < boundarys.y2; y++)
		for (x = boundarys.x1; x < boundarys.x2; x++)
			{
				for (ch = 0; ch < 4; ch++)
					{
						sum = 0;
						for (i = 0; i < 4; i++)
							{
								sum += upself->KernelMatrix[ch * 5 + i] *
									in_pixels[4 * x + y * rowstride + i];
							}
						sum += upself->KernelMatrix[ch * 5 + 4] * 255;
						
						tempresult = sum;
						if (tempresult > 255)
							tempresult = 255;
						if (tempresult < 0)
							tempresult = 0;
						output_pixels[4 * x + y * rowstride + ch] = tempresult;
					}
			}

	rsvg_filter_store_result (self->result, output, ctx);
	
	g_object_unref (G_OBJECT (in));
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_colour_matrix_free (RsvgNode * self)
{
	RsvgFilterPrimitiveColourMatrix *upself;

	upself = (RsvgFilterPrimitiveColourMatrix *) self;
	g_string_free (upself->super.result, TRUE);
	g_string_free (upself->super.in, TRUE);
	if (upself->KernelMatrix)
		g_free (upself->KernelMatrix);
	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_colour_matrix_set_atts (RsvgNode * self, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	gint type;
	guint listlen;
	double font_size;
	const char *value;
	RsvgFilterPrimitiveColourMatrix *filter;
	
	font_size = rsvg_state_current_font_size (ctx);
	
	filter = (RsvgFilterPrimitiveColourMatrix *)self;
	
	type = 0;
	
	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (filter->super.in, value);					
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "values")))
				{
					filter->KernelMatrix =
						rsvg_css_parse_number_list (value, &listlen);
				}
			if ((value = rsvg_property_bag_lookup (atts, "type")))
				{
					if (!strcmp (value, "matrix"))
						type = 0;
					else if (!strcmp (value, "saturate"))
						type = 1;
					else if (!strcmp (value, "hueRotate"))
						type = 2;
					else if (!strcmp (value, "luminanceToAlpha"))
						type = 3;
					else
						type = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}			

	if (type == 0)
		{
			if (listlen != 20)
				{
					if (filter->KernelMatrix != NULL)
						g_free (filter->KernelMatrix);
					filter->KernelMatrix = g_new0 (double, 20);
				}
		}
	else if (type == 1)
		{
			float s;
			if (listlen != 0)
				{
					s = filter->KernelMatrix[0];
					g_free (filter->KernelMatrix);
				}
			else
				s = 1;
			filter->KernelMatrix = g_new0 (double, 20);

			filter->KernelMatrix[0] = 0.213 + 0.787 * s;
			filter->KernelMatrix[1] = 0.715 - 0.715 * s;
			filter->KernelMatrix[2] = 0.072 - 0.072 * s;
			filter->KernelMatrix[5] = 0.213 - 0.213 * s;
			filter->KernelMatrix[6] = 0.715 + 0.285 * s;
			filter->KernelMatrix[7] = 0.072 - 0.072 * s;
			filter->KernelMatrix[10] = 0.213 - 0.213 * s;
			filter->KernelMatrix[11] = 0.715 - 0.715 * s;
			filter->KernelMatrix[12] = 0.072 + 0.928 * s;
			filter->KernelMatrix[18] = 1;
		}
	else if (type == 2)
		{
			double cosval, sinval, arg;

			if (listlen != 0)
				{
					arg = filter->KernelMatrix[0];
					g_free (filter->KernelMatrix);
				}
			else
				arg = 0;

			cosval = cos (arg);
			sinval = sin (arg);

			filter->KernelMatrix = g_new0 (double, 20);
			
			filter->KernelMatrix[0] = 0.213 + cosval * 0.787 + sinval * -0.213;
			filter->KernelMatrix[1] = 0.715 + cosval * -0.715 + sinval * -0.715;
			filter->KernelMatrix[2] = 0.072 + cosval * -0.072 + sinval * 0.928;
			filter->KernelMatrix[5] = 0.213 + cosval * -0.213 + sinval * 0.143;
			filter->KernelMatrix[6] = 0.715 + cosval * 0.285 + sinval * 0.140;
			filter->KernelMatrix[7] = 0.072 + cosval * -0.072 + sinval * -0.283;
			filter->KernelMatrix[10] = 0.213 + cosval * -0.213 + sinval * -0.787;
			filter->KernelMatrix[11] = 0.715 + cosval * -0.715 + sinval * 0.715;
			filter->KernelMatrix[12] = 0.072 + cosval * 0.928 + sinval * 0.072;
			filter->KernelMatrix[18] = 1;
		}
	else if (type == 3)
		{
			if (filter->KernelMatrix != NULL)
				g_free (filter->KernelMatrix);

			filter->KernelMatrix = g_new0 (double, 20);

			filter->KernelMatrix[15] = 0.2125;
			filter->KernelMatrix[16] = 0.7154;
			filter->KernelMatrix[17] = 0.0721;
		}
	else 
		{
			g_assert_not_reached();
		}
}

RsvgNode *
rsvg_new_filter_primitive_colour_matrix (void)
{
	RsvgFilterPrimitiveColourMatrix *filter;
	filter = g_new (RsvgFilterPrimitiveColourMatrix, 1);	
	_rsvg_node_init(&filter->super.super);
	filter->super.in = g_string_new ("none");
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;	
	filter->KernelMatrix = NULL;
	filter->super.render = &rsvg_filter_primitive_colour_matrix_render;
	filter->super.super.free = &rsvg_filter_primitive_colour_matrix_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_colour_matrix_set_atts;
	return (RsvgNode *)filter;
}


/*************************************************************/
/*************************************************************/

typedef struct _RsvgNodeComponentTransferFunc RsvgNodeComponentTransferFunc;

typedef gdouble (*ComponentTransferFunc) (gdouble C,
										  RsvgNodeComponentTransferFunc*
										  user_data);

typedef struct _RsvgFilterPrimitiveComponentTransfer
RsvgFilterPrimitiveComponentTransfer;

struct _RsvgNodeComponentTransferFunc
{
	RsvgNode super;
	ComponentTransferFunc function;
	gchar channel;
	gdouble *tableValues;
	guint nbTableValues;
	gdouble slope;
	gdouble intercept;
	gdouble amplitude;
	gdouble exponent;
	gdouble offset;
};

struct _RsvgFilterPrimitiveComponentTransfer
{
	RsvgFilterPrimitive super;
};

static gint
get_component_transfer_table_value (gdouble C, RsvgNodeComponentTransferFunc *user_data)
{
	gdouble N;
	gint k;
	N = user_data->nbTableValues;	

	k = floor(C * N);
	k -= 1;
	if (k < 0)
		k = 0;
	return k;
}

static gdouble
identity_component_transfer_func (gdouble C, RsvgNodeComponentTransferFunc *user_data)
{
	return C;
}

static gdouble
table_component_transfer_func (gdouble C, RsvgNodeComponentTransferFunc *user_data)
{
	guint k;
	gdouble vk, vk1;
	gfloat distancefromlast;
	
	if (!user_data->nbTableValues)
		return C;
	
	k = get_component_transfer_table_value (C, user_data);

	if (k == user_data->nbTableValues - 1)
		return user_data->tableValues[k - 1];

	vk = user_data->tableValues[k];
	vk1 = user_data->tableValues[k + 1];
	
	distancefromlast = (C - ((double)k + 1) / (double)user_data->nbTableValues) * (double)user_data->nbTableValues; 

	return (vk + distancefromlast * (vk1 - vk));
}

static gdouble
discrete_component_transfer_func (gdouble C, RsvgNodeComponentTransferFunc *user_data)
{
	gint k;
	
	if (!user_data->nbTableValues)
		return C;
	
	k = get_component_transfer_table_value (C, user_data);
	
	return user_data->tableValues[k];
}

static gdouble
linear_component_transfer_func (gdouble C, RsvgNodeComponentTransferFunc *user_data)
{
	return (user_data->slope * C) + user_data->intercept;
}

static gdouble
gamma_component_transfer_func (gdouble C, RsvgNodeComponentTransferFunc *user_data)
{
	return user_data->amplitude * pow (C,
									   user_data->exponent) + user_data->offset;
}

static void 
rsvg_filter_primitive_component_transfer_render (RsvgFilterPrimitive *
												self,
												RsvgFilterContext * ctx)
{
	gint x, y, c;
	guint i;
	gint temp;
	gint rowstride, height, width;
	FPBox boundarys;
	RsvgNodeComponentTransferFunc * channels[4];
	ComponentTransferFunc functions[4];
	
	guchar *in_pixels;
	guchar *output_pixels;
	
	RsvgFilterPrimitiveComponentTransfer *upself;
	
	GdkPixbuf *output;
	GdkPixbuf *in;
	upself = (RsvgFilterPrimitiveComponentTransfer *) self;
	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	for (c = 0; c < 4; c++)
		{
			char channel = "rgba"[c];
			for (i = 0; i < self->super.children->len; i++)
				{
					RsvgNodeComponentTransferFunc * temp;
					temp = (RsvgNodeComponentTransferFunc *)
						g_ptr_array_index(self->super.children, i);
					if (temp->channel == channel)
						{
							functions[c] = temp->function;
							channels[c] = temp;
							break;
						}
				}
			if (i == self->super.children->len)
				functions[c] = identity_component_transfer_func;
			
		}

	in = rsvg_filter_get_in (self->in, ctx);
	in_pixels = gdk_pixbuf_get_pixels (in);
	
	height = gdk_pixbuf_get_height (in);
	width = gdk_pixbuf_get_width (in);
	
	rowstride = gdk_pixbuf_get_rowstride (in);
	
	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, width, height);
	
	output_pixels = gdk_pixbuf_get_pixels (output);

	for (y = boundarys.y1; y < boundarys.y2; y++)
		for (x = boundarys.x1; x < boundarys.x2; x++)
			for (c = 0; c < 4; c++)
				{
					temp = functions[c]((double)in_pixels[y * rowstride + x * 4 + c] / 255.0, channels[c]) * 255.0;
					if (temp > 255)
						temp = 255;
					else if (temp < 0)
						temp = 0;
					output_pixels[y * rowstride + x * 4 + c] = temp;		
				}
	rsvg_filter_store_result (self->result, output, ctx);
	
	g_object_unref (G_OBJECT (in));
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_component_transfer_set_atts (RsvgNode * self, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	double font_size;
	const char *value;
	RsvgFilterPrimitiveComponentTransfer *filter;
	
	font_size = rsvg_state_current_font_size (ctx);
	filter = (RsvgFilterPrimitiveComponentTransfer *)self;

	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (filter->super.in, value);
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}
}

RsvgNode *
rsvg_new_filter_primitive_component_transfer (void)
{
	RsvgFilterPrimitiveComponentTransfer *filter;

	filter = g_new (RsvgFilterPrimitiveComponentTransfer, 1);
	_rsvg_node_init(&filter->super.super);
	filter->super.result = g_string_new ("none");
	filter->super.in = g_string_new ("none");
	filter->super.sizedefaults = 1;
	filter->super.render = &rsvg_filter_primitive_component_transfer_render;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_component_transfer_set_atts;

	filter->super.super.children = g_ptr_array_new ();
	return (RsvgNode *)filter;
}

static void 
rsvg_node_component_transfer_function_set_atts (RsvgNode * self,
												RsvgHandle * ctx,
												RsvgPropertyBag * atts)
{
	const char *value;
	RsvgNodeComponentTransferFunc * data = (RsvgNodeComponentTransferFunc *)self;

	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "type")))
				{
					if (!strcmp (value, "identity"))
						data->function = identity_component_transfer_func;
					else if (!strcmp (value, "table"))
						data->function = table_component_transfer_func;
					else if (!strcmp (value, "discrete"))
						data->function = discrete_component_transfer_func;
					else if (!strcmp (value, "linear"))
						data->function = linear_component_transfer_func;
					else if (!strcmp (value, "gamma"))
						data->function = gamma_component_transfer_func;
				}
			if ((value = rsvg_property_bag_lookup (atts, "tableValues")))
				{
					data->tableValues = 
						rsvg_css_parse_number_list (value, 
													&data->nbTableValues);
				}
			if ((value = rsvg_property_bag_lookup (atts, "slope")))
				{
					data->slope = g_ascii_strtod(value, NULL); 
				}
			if ((value = rsvg_property_bag_lookup (atts, "intercept")))
				{
					data->intercept = g_ascii_strtod(value, NULL); 
				}
			if ((value = rsvg_property_bag_lookup (atts, "amplitude")))
				{
					data->amplitude = g_ascii_strtod(value, NULL); 
				}
			if ((value = rsvg_property_bag_lookup (atts, "exponent")))
				{
					data->exponent = g_ascii_strtod(value, NULL); 
				}
			if ((value = rsvg_property_bag_lookup (atts, "offset")))
				{
					data->offset = g_ascii_strtod(value, NULL); 
				}
		}
}

static void
rsvg_component_transfer_function_free(RsvgNode * self)
	 
{
	RsvgNodeComponentTransferFunc *filter = (RsvgNodeComponentTransferFunc *)self;
	if (filter->nbTableValues)
		g_free(filter->tableValues);
	_rsvg_node_free(self);
}

RsvgNode *
rsvg_new_node_component_transfer_function (char channel)
{
	RsvgNodeComponentTransferFunc *filter;

	filter = g_new (RsvgNodeComponentTransferFunc, 1);
	_rsvg_node_init(&filter->super);
	filter->super.free = rsvg_component_transfer_function_free;
	filter->super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.set_atts = rsvg_node_component_transfer_function_set_atts;
	filter->channel = channel;
	filter->function = identity_component_transfer_func;
	filter->nbTableValues = 0;
	return (RsvgNode *)filter;
}

/*************************************************************/
/*************************************************************/

typedef struct _RsvgFilterPrimitiveErode
RsvgFilterPrimitiveErode;

struct _RsvgFilterPrimitiveErode
{
	RsvgFilterPrimitive super;
	double rx, ry;
	int mode;
};

static void
rsvg_filter_primitive_erode_render (RsvgFilterPrimitive * self,
									RsvgFilterContext * ctx)
{
	guchar ch, extreme;
	gint x, y;
	gint i, j;
	gint rowstride, height, width;
	FPBox boundarys;
	
	guchar *in_pixels;
	guchar *output_pixels;
	
	RsvgFilterPrimitiveErode *upself;
	
	GdkPixbuf *output;
	GdkPixbuf *in;
	
	gint kx, ky;
	guchar val;
	
	upself = (RsvgFilterPrimitiveErode *) self;
	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	in = rsvg_filter_get_in (self->in, ctx);
	in_pixels = gdk_pixbuf_get_pixels (in);
	
	height = gdk_pixbuf_get_height (in);
	width = gdk_pixbuf_get_width (in);
	
	rowstride = gdk_pixbuf_get_rowstride (in);
	
	/* scale the radius values */
	kx = upself->rx * ctx->paffine[0];
	ky = upself->ry * ctx->paffine[3];

	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, width, height);

	output_pixels = gdk_pixbuf_get_pixels (output);
	
	for (y = boundarys.y1; y < boundarys.y2; y++)
		for (x = boundarys.x1; x < boundarys.x2; x++)
			for (ch = 0; ch < 4; ch++)
				{
					if (upself->mode == 0)
						extreme = 255;
					else
						extreme = 0;
					for (i = -ky; i < ky + 1; i++)
						for (j = -kx; j < kx + 1; j++)
							{
								if (y + i >= height || y + i < 0 || 
									x + j >= width || x + j < 0)
									continue;
								
								val = in_pixels[(y + i) * rowstride 
												+ (x + j) * 4 + ch];
							   

								if (upself->mode == 0)
									{	
										if (extreme > val)
											extreme = val;
									}
								else
									{
										if (extreme < val)
											extreme = val;
									}
								
							}
					output_pixels[y * rowstride + x * 4 + ch] = extreme;
				}
	rsvg_filter_store_result (self->result, output, ctx);
	
	g_object_unref (G_OBJECT (in));
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_erode_free (RsvgNode * self)
{
	RsvgFilterPrimitiveErode *upself;
	
	upself = (RsvgFilterPrimitiveErode *) self;
	g_string_free (upself->super.result, TRUE);
	g_string_free (upself->super.in, TRUE);
	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_erode_set_atts (RsvgNode * self, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	const char *value;	

	double font_size;
	RsvgFilterPrimitiveErode *filter;

	font_size = rsvg_state_current_font_size (ctx);
	
	filter = (RsvgFilterPrimitiveErode *)self;
	
	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (filter->super.in, value);					
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "radius")))
				{
					rsvg_css_parse_number_optional_number (value,
														   &filter->rx,
														   &filter->ry);
				}
			if ((value = rsvg_property_bag_lookup (atts, "operator")))
				{
					if (!strcmp (value, "erode"))
						filter->mode = 0;
					else if (!strcmp (value, "dilate"))
						filter->mode = 1;
				}
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}
}

RsvgNode *
rsvg_new_filter_primitive_erode (void)
{
	RsvgFilterPrimitiveErode *filter;
	filter = g_new (RsvgFilterPrimitiveErode, 1);
	_rsvg_node_init(&filter->super.super);
	filter->super.in = g_string_new ("none");
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;
	filter->rx = 0;
	filter->ry = 0;
	filter->mode = 0;
	filter->super.render = &rsvg_filter_primitive_erode_render;
	filter->super.super.free = &rsvg_filter_primitive_erode_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_erode_set_atts;
	return (RsvgNode *)filter;
}

/*************************************************************/
/*************************************************************/

typedef enum
{
	COMPOSITE_MODE_OVER, COMPOSITE_MODE_IN, COMPOSITE_MODE_OUT, 
	COMPOSITE_MODE_ATOP, COMPOSITE_MODE_XOR, COMPOSITE_MODE_ARITHMETIC
}
RsvgFilterPrimitiveCompositeMode;

typedef struct _RsvgFilterPrimitiveComposite RsvgFilterPrimitiveComposite;
struct _RsvgFilterPrimitiveComposite
{
	RsvgFilterPrimitive super;
	RsvgFilterPrimitiveCompositeMode mode;
	GString *in2;

	gdouble k1, k2, k3, k4;
};

static void
rsvg_filter_primitive_composite_render (RsvgFilterPrimitive * self,
									RsvgFilterContext * ctx)
{
	guchar i;
	gint x, y;
	gint rowstride, height, width;
	FPBox boundarys;
	
	guchar *in_pixels;
	guchar *in2_pixels;
	guchar *output_pixels;
	
	RsvgFilterPrimitiveComposite *upself;
	
	GdkPixbuf *output;
	GdkPixbuf *in;
	GdkPixbuf *in2;
	
	upself = (RsvgFilterPrimitiveComposite *) self;
	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	in = rsvg_filter_get_in (self->in, ctx);
	in_pixels = gdk_pixbuf_get_pixels (in);
	in2 = rsvg_filter_get_in (upself->in2, ctx);
	in2_pixels = gdk_pixbuf_get_pixels (in2);
	
	height = gdk_pixbuf_get_height (in);
	width = gdk_pixbuf_get_width (in);
	
	rowstride = gdk_pixbuf_get_rowstride (in);
	
	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, width, height);
	output_pixels = gdk_pixbuf_get_pixels (output);

	if (upself->mode == COMPOSITE_MODE_ARITHMETIC)
		for (y = boundarys.y1; y < boundarys.y2; y++)
			for (x = boundarys.x1; x < boundarys.x2; x++)
				{
					double qr, cr, qa, qb, ca, cb;
					
					qa = (double) in_pixels[4 * x + y * rowstride + 3] / 255.0;
					qb = (double) in2_pixels[4 * x + y * rowstride + 3] / 255.0;
					cr = 0;
					
					qr = upself->k2 * qa + upself->k3 * qb + upself->k1 * qa * qb;
					
					for (i = 0; i < 3; i++)
						{
							ca = (double) in_pixels[4 * x + y * rowstride + i] / 255.0 * qa;
							cb = (double) in2_pixels[4 * x + y * rowstride + i] / 255.0 * qb;
							
							cr = (ca * upself->k2 + cb * upself->k3 + 
								  ca * cb * upself->k1 + upself->k4);
							if (cr > 1)
								cr = 1;
							if (cr < 0)
								cr = 0;
							output_pixels[4 * x + y * rowstride + i] = (guchar)(cr * 255.0);
							
						}
					if (qr > 1)
					qr = 1;
					if (qr < 0)
					qr = 0;
					output_pixels[4 * x + y * rowstride + 3] = (guchar)(qr * 255.0);
				}
	
	else
		for (y = boundarys.y1; y < boundarys.y2; y++)
			for (x = boundarys.x1; x < boundarys.x2; x++)
				{
					double qr, cr, qa, qb, ca, cb, Fa, Fb, Fab, Fo;
					
					qa = (double) in_pixels[4 * x + y * rowstride + 3] / 255.0;
					qb = (double) in2_pixels[4 * x + y * rowstride + 3] / 255.0;
					cr = 0;
					Fa = Fb = Fab = Fo = 0;
					switch (upself->mode)
						{
						case COMPOSITE_MODE_OVER:
							Fa = 1;
							Fb = 1 - qa;
							break;
						case COMPOSITE_MODE_IN:
							Fa = qb;
							Fb = 0;
							break;
						case COMPOSITE_MODE_OUT:
							Fa = 1 - qb;
							Fb = 0;
							break;
						case COMPOSITE_MODE_ATOP:
							Fa = qb;
							Fb = 1 - qa;
							break;
						case COMPOSITE_MODE_XOR:
							Fa = 1 - qb;
							Fb = 1 - qa;
							break;
						default:
							break;
						}
				
					qr = Fa * qa + Fb * qb + Fab * qa * qb;

					for (i = 0; i < 3; i++)
						{
							ca = (double) in_pixels[4 * x + y * rowstride + i] / 255.0 * qa;
							cb = (double) in2_pixels[4 * x + y * rowstride + i] / 255.0 * qb;
							
							/* VMS bug fix */
							if (qr) {
							    cr = (ca * Fa + cb * Fb + ca * cb * Fab + Fo) / qr;
							} else {
							    cr = 0;
							}
							if (cr > 1)
								cr = 1;
							if (cr < 0)
							cr = 0;
							output_pixels[4 * x + y * rowstride + i] = (guchar)(cr * 255.0);
							
						}
					if (qr > 1)
						qr = 1;
					if (qr < 0)
						qr = 0;
					output_pixels[4 * x + y * rowstride + 3] = (guchar)(qr * 255.0);
				}
	
	rsvg_filter_store_result (self->result, output, ctx);
	
	g_object_unref (G_OBJECT (in));
	g_object_unref (G_OBJECT (in2));
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_composite_free (RsvgNode * self)
{
	RsvgFilterPrimitiveComposite *upself;
	
	upself = (RsvgFilterPrimitiveComposite *) self;
	g_string_free (upself->super.result, TRUE);
	g_string_free (upself->super.in, TRUE);
	g_string_free (upself->in2, TRUE);
	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_composite_set_atts (RsvgNode * self, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	double font_size;
	const char *value;
	RsvgFilterPrimitiveComposite *filter;
	
	font_size = rsvg_state_current_font_size (ctx);

	filter = (RsvgFilterPrimitiveComposite *)self;
	
	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "operator"))) 
				{
					if (!strcmp (value, "in"))
						filter->mode = COMPOSITE_MODE_IN;
					else if (!strcmp (value, "out"))
						filter->mode = COMPOSITE_MODE_OUT;
					else if (!strcmp (value, "atop"))
						filter->mode = COMPOSITE_MODE_ATOP;
					else if (!strcmp (value, "xor"))
						filter->mode = COMPOSITE_MODE_XOR;
					else if (!strcmp (value, 
									  "arithmetic"))
						filter->mode = COMPOSITE_MODE_ARITHMETIC;
					else
						filter->mode = COMPOSITE_MODE_OVER;
				}
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (filter->super.in, value);					
			if ((value = rsvg_property_bag_lookup (atts, "in2")))
				g_string_assign (filter->in2, value);					
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);					
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "k1")))
				{
					filter->k1 = g_ascii_strtod(value, NULL); 
				}
			if ((value = rsvg_property_bag_lookup (atts, "k2")))
				{
					filter->k2 = g_ascii_strtod(value, NULL); 
				}
			if ((value = rsvg_property_bag_lookup (atts, "k3")))
				{
					filter->k3 = g_ascii_strtod(value, NULL); 
				}
			if ((value = rsvg_property_bag_lookup (atts, "k4")))
				{
					filter->k4 = g_ascii_strtod(value, NULL); 
				}
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}
}

RsvgNode *
rsvg_new_filter_primitive_composite (void)
{
	RsvgFilterPrimitiveComposite *filter;
	filter = g_new (RsvgFilterPrimitiveComposite, 1);
	_rsvg_node_init(&filter->super.super);
	filter->mode = COMPOSITE_MODE_OVER;
	filter->super.in = g_string_new ("none");
	filter->in2 = g_string_new ("none");
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;
	filter->k1 = 0;
	filter->k2 = 0;
	filter->k3 = 0;
	filter->k4 = 0;
	filter->super.render = &rsvg_filter_primitive_composite_render;
	filter->super.super.free = &rsvg_filter_primitive_composite_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_composite_set_atts;
	return (RsvgNode *)filter;
}

/*************************************************************/
/*************************************************************/

typedef struct _RsvgFilterPrimitiveFlood
RsvgFilterPrimitiveFlood;

struct _RsvgFilterPrimitiveFlood
{
	RsvgFilterPrimitive super;
	guint32 colour;
	guint opacity;
};

static void
rsvg_filter_primitive_flood_render (RsvgFilterPrimitive * self,
									RsvgFilterContext * ctx)
{
	guchar i;
	gint x, y;
	gint rowstride, height, width;
	FPBox boundarys;
	
	guchar *output_pixels;
	
	RsvgFilterPrimitiveFlood *upself;
	
	GdkPixbuf *output;
	
	RsvgFilterPrimitiveOutput out;

	upself = (RsvgFilterPrimitiveFlood *) self;
	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	height = ctx->height;
	width = ctx->width;
	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, width, height);
	rowstride = gdk_pixbuf_get_rowstride (output);
	
	output_pixels = gdk_pixbuf_get_pixels (output);

	for (y = boundarys.y1; y < boundarys.y2; y++)
		for (x = boundarys.x1; x < boundarys.x2; x++)
			{
				for (i = 0; i < 3; i++)
					{
						output_pixels[4 * x + y * rowstride + i] = ((char *)
							(&upself->colour))[2 - i];
					}
				output_pixels[4 * x + y * rowstride + 3] = upself->opacity;
			}

	out.result = output;
	out.Rused = 1;
	out.Gused = 1;
	out.Bused = 1;
	out.Aused = 1;
	out.bounds = boundarys;

	rsvg_filter_store_output (self->result, out, ctx);
	
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_flood_free (RsvgNode * self)
{
	RsvgFilterPrimitiveFlood *upself;
	
	upself = (RsvgFilterPrimitiveFlood *) self;
	g_string_free (upself->super.result, TRUE);
	g_string_free (upself->super.in, TRUE);
	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_flood_set_atts (RsvgNode * self, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	const char *value;
	double font_size;
	RsvgFilterPrimitiveFlood *filter;

	font_size = rsvg_state_current_font_size (ctx);
	
	filter = (RsvgFilterPrimitiveFlood *)self;

	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "flood-color")))
				{
							filter->colour = rsvg_css_parse_color (value, 0);						
				}
			if ((value = rsvg_property_bag_lookup (atts, "flood-opacity")))
				{
					filter->opacity = rsvg_css_parse_opacity (value);
				}
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}
}

RsvgNode *
rsvg_new_filter_primitive_flood (void)
{
	RsvgFilterPrimitiveFlood *filter;	
	filter = g_new (RsvgFilterPrimitiveFlood, 1);
	_rsvg_node_init(&filter->super.super);
	filter->super.in = g_string_new ("none");
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;
	filter->opacity = 255;
	filter->colour = 0;
	filter->super.render = &rsvg_filter_primitive_flood_render;
	filter->super.super.free = &rsvg_filter_primitive_flood_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_flood_set_atts;
	return (RsvgNode *)filter;
}

/*************************************************************/
/*************************************************************/

typedef struct _RsvgFilterPrimitiveDisplacementMap RsvgFilterPrimitiveDisplacementMap;

struct _RsvgFilterPrimitiveDisplacementMap
{
	RsvgFilterPrimitive super;
	gint dx, dy;
	char xChannelSelector, yChannelSelector;
	GString *in2;
	double scale;
};

static void
rsvg_filter_primitive_displacement_map_render (RsvgFilterPrimitive * self,
											   RsvgFilterContext * ctx)
{
	guchar ch, xch, ych;
	gint x, y;
	gint rowstride, height, width;
	FPBox boundarys;
	
	guchar *in_pixels;
	guchar *in2_pixels;
	guchar *output_pixels;
	
	RsvgFilterPrimitiveDisplacementMap *upself;
	
	GdkPixbuf *output;
	GdkPixbuf *in;
	GdkPixbuf *in2;
	
	double ox, oy;
	
	upself = (RsvgFilterPrimitiveDisplacementMap *) self;
	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	in = rsvg_filter_get_in (self->in, ctx);
	in_pixels = gdk_pixbuf_get_pixels (in);

	in2 = rsvg_filter_get_in (upself->in2, ctx);
	in2_pixels = gdk_pixbuf_get_pixels (in2);
	
	height = gdk_pixbuf_get_height (in);
	width = gdk_pixbuf_get_width (in);
	
	rowstride = gdk_pixbuf_get_rowstride (in);
	
	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, width, height);
	
	output_pixels = gdk_pixbuf_get_pixels (output);
	
	switch (upself->xChannelSelector)
		{
		case 'R':
			xch = 0;
			break;
		case 'G':
			xch = 1;
			break;
		case 'B':
			xch = 2;
			break;
		case 'A':
			xch = 3;
			break;
		default:
			xch = 4;
		};

	switch (upself->yChannelSelector)
		{
		case 'R':
			ych = 0;
			break;
		case 'G':
			ych = 1;
			break;
		case 'B':
			ych = 2;
			break;
		case 'A':
			ych = 3;
			break;
		default:
			ych = 4;
		};

	for (y = boundarys.y1; y < boundarys.y2; y++)
		for (x = boundarys.x1; x < boundarys.x2; x++)
			{
				if (xch != 4)
					ox = x + upself->scale * ctx->paffine[0] * 
						((double)in2_pixels[y * rowstride + x * 4 + xch] / 255.0 - 0.5);
				else
					ox = x;

				if (ych != 4)
					oy = y + upself->scale * ctx->paffine[3] * 
						((double)in2_pixels[y * rowstride + x * 4 + ych] / 255.0 - 0.5);
				else
					oy = y;

				for (ch = 0; ch < 4; ch++)
					{
						output_pixels[y * rowstride + x * 4 + ch] =
							gdk_pixbuf_get_interp_pixel(in_pixels, ox, oy, ch, boundarys, 
														rowstride);
					}
			}

	rsvg_filter_store_result (self->result, output, ctx);
	
	g_object_unref (G_OBJECT (in));
	g_object_unref (G_OBJECT (in2));
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_displacement_map_free (RsvgNode * self)
{
	RsvgFilterPrimitiveDisplacementMap *upself;
	
	upself = (RsvgFilterPrimitiveDisplacementMap *) self;
	g_string_free (upself->super.result, TRUE);
	g_string_free (upself->super.in, TRUE);
	g_string_free (upself->in2, TRUE);
	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_displacement_map_set_atts (RsvgNode * self, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	const char *value;
	double font_size;
	RsvgFilterPrimitiveDisplacementMap *filter;
	
	font_size = rsvg_state_current_font_size (ctx);
	
	filter = (RsvgFilterPrimitiveDisplacementMap *)self;
	
	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (filter->super.in, value);
			if ((value = rsvg_property_bag_lookup (atts, "in2")))
				g_string_assign (filter->in2, value);
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "xChannelSelector")))
				filter->xChannelSelector = (value)[0];
			if ((value = rsvg_property_bag_lookup (atts, "yChannelSelector")))
				filter->yChannelSelector = (value)[0];
			if ((value = rsvg_property_bag_lookup (atts, "scale")))
				filter->scale = g_ascii_strtod(value, NULL);
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}
}

RsvgNode *
rsvg_new_filter_primitive_displacement_map ()
{
	RsvgFilterPrimitiveDisplacementMap *filter;	
	filter = g_new (RsvgFilterPrimitiveDisplacementMap, 1);	
	_rsvg_node_init(&filter->super.super);
	filter->super.in = g_string_new ("none");
	filter->in2 = g_string_new ("none");
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;
	filter->xChannelSelector = ' ';
	filter->yChannelSelector = ' ';
	filter->scale = 0;
	filter->super.render = &rsvg_filter_primitive_displacement_map_render;
	filter->super.super.free = &rsvg_filter_primitive_displacement_map_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_displacement_map_set_atts;
	return (RsvgNode *)filter;
}

/*************************************************************/
/*************************************************************/

/* Produces results in the range [1, 2**31 - 2].
   Algorithm is: r = (a * r) mod m
   where a = 16807 and m = 2**31 - 1 = 2147483647
   See [Park & Miller], CACM vol. 31 no. 10 p. 1195, Oct. 1988
   To test: the algorithm should produce the result 1043618065
   as the 10,000th generated number if the original seed is 1.
*/
#define feTurbulence_RAND_m 2147483647 /* 2**31 - 1 */
#define feTurbulence_RAND_a 16807 /* 7**5; primitive root of m */
#define feTurbulence_RAND_q 127773 /* m / a */
#define feTurbulence_RAND_r 2836 /* m % a */
#define feTurbulence_BSize 0x100
#define feTurbulence_BM 0xff
#define feTurbulence_PerlinN 0x1000
#define feTurbulence_NP 12 /* 2^PerlinN */
#define feTurbulence_NM 0xfff

typedef struct _RsvgFilterPrimitiveTurbulence RsvgFilterPrimitiveTurbulence;
struct _RsvgFilterPrimitiveTurbulence
{
	RsvgFilterPrimitive super;

	int uLatticeSelector[feTurbulence_BSize + feTurbulence_BSize + 2];
	double fGradient[4][feTurbulence_BSize + feTurbulence_BSize + 2][2];

	int seed;

	double fBaseFreqX;
	double fBaseFreqY;

	int nNumOctaves;
	gboolean bFractalSum;
	gboolean bDoStitching;
};

struct feTurbulence_StitchInfo
{
	int nWidth; /* How much to subtract to wrap for stitching. */
	int nHeight;
	int nWrapX; /* Minimum value to wrap. */
	int nWrapY;
};

static long feTurbulence_setup_seed(int lSeed)
{
	if (lSeed <= 0) 
		lSeed = -(lSeed % (feTurbulence_RAND_m - 1)) + 1;
	if (lSeed > feTurbulence_RAND_m - 1) 
		lSeed = feTurbulence_RAND_m - 1;
	return lSeed;
}

static long feTurbulence_random(int lSeed)
{
  long result;

  result = feTurbulence_RAND_a * (lSeed % feTurbulence_RAND_q) - feTurbulence_RAND_r * (lSeed / feTurbulence_RAND_q);
  if (result <= 0) 
	  result += feTurbulence_RAND_m;
  return result;
}

static void feTurbulence_init(RsvgFilterPrimitiveTurbulence *filter)
{
	double s;
	int i, j, k, lSeed;
	
	lSeed = feTurbulence_setup_seed(filter->seed);
	for(k = 0; k < 4; k++)
		{
			for(i = 0; i < feTurbulence_BSize; i++)
				{
					filter->uLatticeSelector[i] = i;
					for (j = 0; j < 2; j++)
						filter->fGradient[k][i][j] = (double)(((lSeed = feTurbulence_random(lSeed)) % (feTurbulence_BSize + feTurbulence_BSize)) - feTurbulence_BSize) / feTurbulence_BSize;
					s = (double)(sqrt(filter->fGradient[k][i][0] * filter->fGradient[k][i][0] + filter->fGradient[k][i][1] * filter->fGradient[k][i][1]));
					filter->fGradient[k][i][0] /= s;
					filter->fGradient[k][i][1] /= s;
				}
		}
	
	while(--i)
		{
			k = filter->uLatticeSelector[i];
			filter->uLatticeSelector[i] = filter->uLatticeSelector[j = (lSeed = feTurbulence_random(lSeed)) % feTurbulence_BSize];
			filter->uLatticeSelector[j] = k;
		}
	
	for(i = 0; i < feTurbulence_BSize + 2; i++)
		{
			filter->uLatticeSelector[feTurbulence_BSize + i] = filter->uLatticeSelector[i];
			for(k = 0; k < 4; k++)
				for(j = 0; j < 2; j++)
					filter->fGradient[k][feTurbulence_BSize + i][j] = filter->fGradient[k][i][j];
		}
}

#define feTurbulence_s_curve(t) ( t * t * (3. - 2. * t) )
#define feTurbulence_lerp(t, a, b) ( a + t * (b - a) )

static double feTurbulence_noise2(RsvgFilterPrimitiveTurbulence *filter,
								  int nColorChannel, double vec[2], 
								  struct feTurbulence_StitchInfo *pStitchInfo)
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	double rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	register int i, j;

	t = vec[0] + feTurbulence_PerlinN;
	bx0 = (int)t;
	bx1 = bx0+1;
	rx0 = t - (int)t;
	rx1 = rx0 - 1.0f;
	t = vec[1] + feTurbulence_PerlinN;
	by0 = (int)t;
	by1 = by0+1;
	ry0 = t - (int)t;
	ry1 = ry0 - 1.0f;

	/* If stitching, adjust lattice points accordingly. */
	if(pStitchInfo != NULL)
		{
			if(bx0 >= pStitchInfo->nWrapX)
				bx0 -= pStitchInfo->nWidth;
			if(bx1 >= pStitchInfo->nWrapX)
				bx1 -= pStitchInfo->nWidth;
			if(by0 >= pStitchInfo->nWrapY)
				by0 -= pStitchInfo->nHeight;
			if(by1 >= pStitchInfo->nWrapY)
				by1 -= pStitchInfo->nHeight;
		}

	bx0 &= feTurbulence_BM;
	bx1 &= feTurbulence_BM;
	by0 &= feTurbulence_BM;
	by1 &= feTurbulence_BM;
	i = filter->uLatticeSelector[bx0];
	j = filter->uLatticeSelector[bx1];
	b00 = filter->uLatticeSelector[i + by0];
	b10 = filter->uLatticeSelector[j + by0];
	b01 = filter->uLatticeSelector[i + by1];
	b11 = filter->uLatticeSelector[j + by1];
	sx = (double)(feTurbulence_s_curve(rx0));
	sy = (double)(feTurbulence_s_curve(ry0));
	q = filter->fGradient[nColorChannel][b00]; u = rx0 * q[0] + ry0 * q[1];
	q = filter->fGradient[nColorChannel][b10]; v = rx1 * q[0] + ry0 * q[1];
	a = feTurbulence_lerp(sx, u, v);
	q = filter->fGradient[nColorChannel][b01]; u = rx0 * q[0] + ry1 * q[1];
	q = filter->fGradient[nColorChannel][b11]; v = rx1 * q[0] + ry1 * q[1];
	b = feTurbulence_lerp(sx, u, v);

	return feTurbulence_lerp(sy, a, b);
}

static double feTurbulence_turbulence(RsvgFilterPrimitiveTurbulence *filter,
									  int nColorChannel, double *point, 
									  double fTileX, double fTileY, double fTileWidth, double fTileHeight)
{
	struct feTurbulence_StitchInfo stitch;
	struct feTurbulence_StitchInfo *pStitchInfo = NULL; /* Not stitching when NULL. */

	double fSum = 0.0f, vec[2], ratio = 1.;
	int nOctave;

	/* Adjust the base frequencies if necessary for stitching. */
	if(filter->bDoStitching)
		{
			/* When stitching tiled turbulence, the frequencies must be adjusted
			   so that the tile borders will be continuous. */
			if(filter->fBaseFreqX != 0.0)
				{
					double fLoFreq = (double)(floor(fTileWidth * filter->fBaseFreqX)) / fTileWidth;
					double fHiFreq = (double)(ceil(fTileWidth * filter->fBaseFreqX)) / fTileWidth;
					if(filter->fBaseFreqX / fLoFreq < fHiFreq / filter->fBaseFreqX)
						filter->fBaseFreqX = fLoFreq;
					else
						filter->fBaseFreqX = fHiFreq;
				}

			if(filter->fBaseFreqY != 0.0)
				{
					double fLoFreq = (double)(floor(fTileHeight * filter->fBaseFreqY)) / fTileHeight;
					double fHiFreq = (double)(ceil(fTileHeight * filter->fBaseFreqY)) / fTileHeight;
					if(filter->fBaseFreqY / fLoFreq < fHiFreq / filter->fBaseFreqY)
						filter->fBaseFreqY = fLoFreq;
					else
						filter->fBaseFreqY = fHiFreq;
				}

			/* Set up initial stitch values. */
			pStitchInfo = &stitch;
			stitch.nWidth = (int)(fTileWidth * filter->fBaseFreqX + 0.5f);
			stitch.nWrapX = fTileX * filter->fBaseFreqX + feTurbulence_PerlinN + stitch.nWidth;
			stitch.nHeight = (int)(fTileHeight * filter->fBaseFreqY + 0.5f);
			stitch.nWrapY = fTileY * filter->fBaseFreqY + feTurbulence_PerlinN + stitch.nHeight;
		}

	vec[0] = point[0] * filter->fBaseFreqX;
	vec[1] = point[1] * filter->fBaseFreqY;

	for(nOctave = 0; nOctave < filter->nNumOctaves; nOctave++)
		{
			if(filter->bFractalSum)
				fSum += (double)(feTurbulence_noise2(filter, nColorChannel, vec, pStitchInfo) / ratio);
			else
				fSum += (double)(fabs(feTurbulence_noise2(filter, nColorChannel, vec, pStitchInfo)) / ratio);

			vec[0] *= 2;
			vec[1] *= 2;
			ratio *= 2;

			if(pStitchInfo != NULL)
				{
					/* Update stitch values. Subtracting PerlinN before the multiplication and
					   adding it afterward simplifies to subtracting it once. */
					stitch.nWidth *= 2;
					stitch.nWrapX = 2 * stitch.nWrapX - feTurbulence_PerlinN;
					stitch.nHeight *= 2;
					stitch.nWrapY = 2 * stitch.nWrapY - feTurbulence_PerlinN;
				}
		}

	return fSum;
}

static void
rsvg_filter_primitive_turbulence_render (RsvgFilterPrimitive * self,
										 RsvgFilterContext * ctx)
{
	RsvgFilterPrimitiveTurbulence *upself;
	gint x, y, tileWidth, tileHeight, rowstride, width, height;
	FPBox boundarys;
	guchar *output_pixels;
	GdkPixbuf *output;
	gdouble affine[6];
	GdkPixbuf *in;
	
	in = rsvg_filter_get_in (self->in, ctx);
	height = gdk_pixbuf_get_height (in);
	width = gdk_pixbuf_get_width (in);
	rowstride = gdk_pixbuf_get_rowstride (in);

	upself = (RsvgFilterPrimitiveTurbulence *) self;
	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	tileWidth = (boundarys.x2 - boundarys.x1);
	tileHeight = (boundarys.y2 - boundarys.y1);

	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, width, height);
	output_pixels = gdk_pixbuf_get_pixels (output);

	_rsvg_affine_invert(affine, ctx->paffine);

	for (y = 0; y < tileHeight; y++)
		{
			for (x = 0; x < tileWidth; x++)
				{
					gint i;
					double point[2];
					point[0] = affine[0] * (x+boundarys.x1) +
						affine[2] * (y+boundarys.y1) + affine[4];
					point[1] = affine[1] * (x+boundarys.x1) +
						affine[3] * (y+boundarys.y1) + affine[5];
					
					for (i = 0; i < 4; i++)
						{
							double cr;
							
							cr = feTurbulence_turbulence(upself, i, point, (double)x, (double)y, (double)tileWidth, (double)tileHeight);
							
							if(upself->bFractalSum)
								cr = ((cr * 255.) + 255.) / 2.;
							else
								cr = (cr * 255.);

							cr = CLAMP(cr, 0., 255.);

							output_pixels[(4 * (x+boundarys.x1)) + ((y+boundarys.y1) * rowstride) + i] = (guchar)(cr);
						}
				}
		}

	rsvg_filter_store_result (self->result, output, ctx);
	
	g_object_unref (G_OBJECT (in));
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_turbulence_free (RsvgNode * self)
{
	RsvgFilterPrimitiveTurbulence *upself;
	
	upself = (RsvgFilterPrimitiveTurbulence *) self;
	g_string_free (upself->super.result, TRUE);
	g_string_free (upself->super.in, TRUE);
	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_turbulence_set_atts (RsvgNode * self, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	const char *value;
	double font_size;
	RsvgFilterPrimitiveTurbulence *filter;
	
	font_size = rsvg_state_current_font_size (ctx);
	
	filter = (RsvgFilterPrimitiveTurbulence *)self;

	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (filter->super.in, value);
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "baseFrequency")))
				rsvg_css_parse_number_optional_number(value, &filter->fBaseFreqX, &filter->fBaseFreqY);
			if ((value = rsvg_property_bag_lookup (atts, "numOctaves")))
				filter->nNumOctaves = atoi(value);
			if ((value = rsvg_property_bag_lookup (atts, "seed")))
				filter->seed = atoi(value);
			if ((value = rsvg_property_bag_lookup (atts, "stitchTiles")))
				filter->bDoStitching = (!strcmp(value, "stitch"));
			if ((value = rsvg_property_bag_lookup (atts, "type")))
				filter->bFractalSum = (!strcmp(value, "fractalNoise"));
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}
}

RsvgNode *
rsvg_new_filter_primitive_turbulence (void)
{
	RsvgFilterPrimitiveTurbulence *filter;
	filter = g_new (RsvgFilterPrimitiveTurbulence, 1);	
	_rsvg_node_init(&filter->super.super);
	filter->super.in = g_string_new ("none");
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;
	filter->fBaseFreqX = 0;
	filter->fBaseFreqY = 0;
	filter->nNumOctaves = 1;
	filter->seed = 0;
	filter->bDoStitching = 0;
	filter->bFractalSum = 0;
	feTurbulence_init(filter);
	filter->super.render = &rsvg_filter_primitive_turbulence_render;
	filter->super.super.free = &rsvg_filter_primitive_turbulence_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_turbulence_set_atts;
	return (RsvgNode *)filter;
}


/*************************************************************/
/*************************************************************/

typedef struct _RsvgFilterPrimitiveImage RsvgFilterPrimitiveImage;

struct _RsvgFilterPrimitiveImage
{
	RsvgFilterPrimitive super;
	RsvgHandle *ctx;
	GString *href;
};

static GdkPixbuf *
rsvg_filter_primitive_image_render_in (RsvgFilterPrimitive * self,
									   RsvgFilterContext * context)
{
	FPBox boundarys;
	RsvgDrawingCtx * ctx;
	RsvgFilterPrimitiveImage *upself;
	int i;
	RsvgNode * parent;
	GdkPixbuf *img, *save;
	RsvgNode *drawable;	

	ctx = context->ctx;
	upself = (RsvgFilterPrimitiveImage *) self;

	if(!upself->href)
		return NULL;

	parent = rsvg_defs_lookup (ctx->defs, upself->href->str);
	if (!parent)
		return NULL;

	drawable = (RsvgNode*)parent;

	boundarys = rsvg_filter_primitive_get_bounds (self, context);
	
	img = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, context->width, context->height);
	
	save = ((RsvgArtRender *)ctx->render)->pixbuf;
	((RsvgArtRender *)ctx->render)->pixbuf = img;

	for (i = 0; i < 6; i++)
		rsvg_state_current(ctx)->affine[i] = context->paffine[i];

	rsvg_state_push(ctx);
	
	rsvg_node_draw (drawable, ctx, 0);
	
	rsvg_state_pop(ctx);
		
	((RsvgArtRender *)ctx->render)->pixbuf = save;
	return img;
}

static GdkPixbuf *
rsvg_filter_primitive_image_render_ext (RsvgFilterPrimitive * self,
										RsvgFilterContext * ctx)
{
	FPBox boundarys;
	RsvgFilterPrimitiveImage *upself;
	GdkPixbuf * img;

	GdkPixbuf * intermediate;

	upself = (RsvgFilterPrimitiveImage *) self;

	if(!upself->href)
		return NULL;

	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);

	img = rsvg_pixbuf_new_from_href(upself->href->str,
									rsvg_handle_get_base_uri (upself->ctx), NULL);

	if(!img)
		return NULL;

	intermediate = gdk_pixbuf_new (GDK_COLORSPACE_RGB, 1, 8, boundarys.x2 - boundarys.x1, 
								   boundarys.y2 - boundarys.y1);

	rsvg_art_affine_image(img, intermediate, 
						  ctx->paffine, 
						  (boundarys.x2 - boundarys.x1) / ctx->paffine[0], 
						  (boundarys.y2 - boundarys.y1) / ctx->paffine[3]);
	
	if (!intermediate)
		{
			g_object_unref (G_OBJECT (img));
			return NULL;
		}


	g_object_unref (G_OBJECT (img));
	return intermediate;

}

static void
rsvg_filter_primitive_image_render (RsvgFilterPrimitive * self,
									RsvgFilterContext * ctx)
{
	FPBox boundarys;
	RsvgFilterPrimitiveImage *upself;
	RsvgFilterPrimitiveOutput op;
	
	GdkPixbuf *output, *img;
	
	upself = (RsvgFilterPrimitiveImage *) self;

	if(!upself->href)
		return;

	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, ctx->width, ctx->height);

	img = rsvg_filter_primitive_image_render_in (self, ctx);
	if (img == NULL)
		{
			img = rsvg_filter_primitive_image_render_ext (self, ctx);
			if (img)
				{
					gdk_pixbuf_copy_area (img, 0, 0, 
										  boundarys.x2 - boundarys.x1, 
										  boundarys.y2 - boundarys.y1,
										  output, boundarys.x1, boundarys.y1);
					g_object_unref (G_OBJECT (img));
				}
		}		
	else
		{
			gdk_pixbuf_copy_area (img, boundarys.x1, boundarys.y1, boundarys.x2 - boundarys.x1, boundarys.y2 - boundarys.y1,
								  output, boundarys.x1, boundarys.y1);
			g_object_unref (G_OBJECT (img));
		}

	op.result = output;
	op.bounds = boundarys;
	op.Rused = 1;
	op.Gused = 1;
	op.Bused = 1;
	op.Aused = 1;

	rsvg_filter_store_output (self->result, op, ctx);
	
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_image_free (RsvgNode * self)
{
	RsvgFilterPrimitiveImage *upself;
	
	upself = (RsvgFilterPrimitiveImage *) self;
	g_string_free (upself->super.result, TRUE);
	g_string_free (upself->super.in, TRUE);

	if(upself->href)
		g_string_free (upself->href, TRUE);

	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_image_set_atts (RsvgNode * self, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	const char *value;
	double font_size;
	RsvgFilterPrimitiveImage *filter;
	
	font_size = rsvg_state_current_font_size (ctx);
	
	filter = (RsvgFilterPrimitiveImage*)self;
	filter->ctx = ctx;	

	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (filter->super.in, value);
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);
			if ((value = rsvg_property_bag_lookup (atts, "xlink:href")))
				{
					filter->href = g_string_new (NULL);
					g_string_assign (filter->href, value);
				}
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}
}

RsvgNode *
rsvg_new_filter_primitive_image (void)
{
	RsvgFilterPrimitiveImage *filter;
	filter = g_new (RsvgFilterPrimitiveImage, 1);
	_rsvg_node_init(&filter->super.super);
	filter->super.in = g_string_new ("none");
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;
	filter->super.render = &rsvg_filter_primitive_image_render;
	filter->super.super.free = &rsvg_filter_primitive_image_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_image_set_atts;
	return (RsvgNode *)filter;
}

/*************************************************************/
/*************************************************************/


typedef struct _FactorAndMatrix FactorAndMatrix;

struct _FactorAndMatrix
{
	gint matrix[9];
	gdouble factor;
};

typedef struct _vector3 vector3;

struct _vector3
{
	gdouble x;
	gdouble y;
	gdouble z;
};

static gdouble
norm (vector3 A)
{
	return sqrt(A.x*A.x + A.y*A.y + A.z*A.z);
}

static gdouble
dotproduct (vector3 A, vector3 B)
{
	return A.x*B.x + A.y*B.y + A.z*B.z;
}

static vector3
normalise (vector3 A)
{
	double divisor;
	divisor = norm(A);

	A.x /= divisor;
	A.y /= divisor;
	A.z /= divisor;

	return A;
}

static FactorAndMatrix
get_light_normal_matrix_x (gint n)
{
	const static FactorAndMatrix matrix_list [] =
		{
			{
				{0,  0,  0,
				 0, -2,  2,
				 0, -1,  1},
				2.0/3.0
			},
			{
				{0,  0,  0,
				 -2,  0,  2,
				 -1,  0,  1},
				1.0/3.0
			},
			{
				{0,  0,  0,
				 -2,  2,  0,
				 -1,  1,  0},
				2.0/3.0
			},
			{
				{0, -1,  1,
				 0, -2,  2,
				 0, -1,  1},
				1.0/2.0
			},
			{
				{-1,  0,  1,
				 -2,  0,  2,
				 -1,  0,  1},
				1.0/4.0
			},
			{
				{-1,  1,  0,
				 -2,  2,  0,
				 -1,  1,  0},
				1.0/2.0
			},
			{
				{0, -1,  1,
				 0, -2,  2,
				 0,  0,  0},
				2.0/3.0
			},
			{
				{-1,  0,  1,
				 -2,  0,  2,
				 0,  0,  0},
				1.0/3.0
			},
			{
				{-1,  1,  0,
				 -2,  2,  0,
				 0,  0,  0},
				2.0/3.0
			}
		};

	return matrix_list[n];
}

static FactorAndMatrix
get_light_normal_matrix_y (gint n)
{
	const static FactorAndMatrix matrix_list [] =
		{
			{
				{0,  0,  0,
				 0, -2, -1,
				 0,  2,  1},
				2.0/3.0
			},
			{
				{0,  0,  0,
				 -1, -2, -1,
				 1,  2,  1},
				1.0/3.0
			},
			{
				{0,  0,  0,
				 -1, -2,  0,
				 1,  2,  0},
				2.0/3.0
			},
			{
				
				{0, -2, -1,
				 0,  0,  0,
				 0,  2,  1},
				1.0/2.0
			},
			{
				{-1, -2, -1,
				 0,  0,  0,
				 1,  2,  1},
				1.0/4.0
			},
			{
				{-1, -2,  0,
				 0,  0,  0,
				 1,  2,  0},
				1.0/2.0
			},
			{
				
				{0, -2, -1,
				 0,  2,  1,
				 0,  0,  0},
				2.0/3.0
			},
			{
				{0, -2, -1,
				 1,  2,  1,
				 0,  0,  0},
				1.0/3.0
			},
			{
				{-1, -2,  0,
				 1,  2,  0,
				 0,  0,  0},
				2.0/3.0
				
			}		
		};

	return matrix_list[n];
}

static vector3
get_surface_normal (guchar * I, FPBox boundarys, gint x, gint y, 
					gdouble dx, gdouble dy, gdouble rawdx, gdouble rawdy, gdouble surfaceScale, gint rowstride)
{
	gint mrow, mcol;
	FactorAndMatrix fnmx, fnmy;
	gint *Kx, *Ky;
	gdouble factorx, factory;
	gdouble Nx, Ny;
	vector3 output;

	if (x + dx >= boundarys.x2 - 1)
		mcol = 2;
	else if (x - dx < boundarys.x1 + 1)
		mcol = 0;
	else
		mcol = 1;

	if (y + dy >= boundarys.y2 - 1)
		mrow = 2;
	else if (y - dy < boundarys.y1 + 1)
		mrow = 0;
	else
		mrow = 1;

	fnmx = get_light_normal_matrix_x(mrow * 3 + mcol);
	factorx = fnmx.factor / rawdx;
	Kx = fnmx.matrix;

	fnmy = get_light_normal_matrix_y(mrow * 3 + mcol);
	factory = fnmy.factor / rawdy;
	Ky = fnmy.matrix;	

    Nx = -surfaceScale * factorx * ((gdouble)
		(Kx[0]*gdk_pixbuf_get_interp_pixel(I,x-dx,y-dy, 3, boundarys, rowstride) +
		 Kx[1]*gdk_pixbuf_get_interp_pixel(I,x   ,y-dy, 3, boundarys, rowstride) + 
		 Kx[2]*gdk_pixbuf_get_interp_pixel(I,x+dx,y-dy, 3, boundarys, rowstride) + 
		 Kx[3]*gdk_pixbuf_get_interp_pixel(I,x-dx,y   , 3, boundarys, rowstride) + 
		 Kx[4]*gdk_pixbuf_get_interp_pixel(I,x   ,y   , 3, boundarys, rowstride) + 
		 Kx[5]*gdk_pixbuf_get_interp_pixel(I,x+dx,y   , 3, boundarys, rowstride) + 
		 Kx[6]*gdk_pixbuf_get_interp_pixel(I,x-dx,y+dy, 3, boundarys, rowstride) + 
		 Kx[7]*gdk_pixbuf_get_interp_pixel(I,x   ,y+dy, 3, boundarys, rowstride) + 
		 Kx[8]*gdk_pixbuf_get_interp_pixel(I,x+dx,y+dy, 3, boundarys, rowstride))) / 255.0;
	
    Ny = -surfaceScale * factory * ((gdouble)
		(Ky[0]*gdk_pixbuf_get_interp_pixel(I,x-dx,y-dy, 3, boundarys, rowstride) +
		 Ky[1]*gdk_pixbuf_get_interp_pixel(I,x   ,y-dy, 3, boundarys, rowstride) + 
		 Ky[2]*gdk_pixbuf_get_interp_pixel(I,x+dx,y-dy, 3, boundarys, rowstride) + 
		 Ky[3]*gdk_pixbuf_get_interp_pixel(I,x-dx,y   , 3, boundarys, rowstride) + 
		 Ky[4]*gdk_pixbuf_get_interp_pixel(I,x   ,y   , 3, boundarys, rowstride) + 
		 Ky[5]*gdk_pixbuf_get_interp_pixel(I,x+dx,y   , 3, boundarys, rowstride) + 
		 Ky[6]*gdk_pixbuf_get_interp_pixel(I,x-dx,y+dy, 3, boundarys, rowstride) + 
		 Ky[7]*gdk_pixbuf_get_interp_pixel(I,x   ,y+dy, 3, boundarys, rowstride) + 
		 Ky[8]*gdk_pixbuf_get_interp_pixel(I,x+dx,y+dy, 3, boundarys, rowstride))) / 255.0;

	output.x = Nx;
	output.y = Ny;

	output.z = 1;
	output = normalise(output);
	return output;
}

typedef enum {
	DISTANTLIGHT, POINTLIGHT, SPOTLIGHT
} lightType;

typedef struct _RsvgNodeLightSource RsvgNodeLightSource;

struct _RsvgNodeLightSource
{
	RsvgNode super;
	lightType type;
	gdouble x; /*doubles as azimuth*/
	gdouble y; /*dounles as elevation*/
	gdouble z;
	gdouble pointsAtX;
	gdouble pointsAtY;
	gdouble pointsAtZ;
	gdouble specularExponent;
	gdouble limitingconeAngle;
};

static vector3
get_light_direction (RsvgNodeLightSource * source, gdouble x1, gdouble y1, gdouble z, gdouble * affine)
{
	vector3 output;

	double x, y;

	x = affine[0] * x1 + affine[2] * y1 + affine[4];
	y = affine[1] * x1 + affine[3] * y1 + affine[5];

	switch (source->type)
		{
		case DISTANTLIGHT:
			output.x = cos(source->x)*cos(source->y);
			output.y = sin(source->x)*cos(source->y);
			output.z = sin(source->y);
			break;
		default:
			output.x = source->x - x;
			output.y = source->y - y;
			output.z = source->z - z;
			output = normalise(output);
			break;
		}
	return output;
}

static vector3
get_light_colour(RsvgNodeLightSource * source, vector3 colour, 
				 gdouble x1, gdouble y1, gdouble z, gdouble * affine)
{
	double base, angle, x, y;
	vector3 s;
	vector3 L;
	vector3 output;

	if (source->type != SPOTLIGHT)
		return colour;
	
	x = affine[0] * x1 + affine[2] * y1 + affine[4];
	y = affine[1] * x1 + affine[3] * y1 + affine[5];

	L.x = source->x - x;
	L.y = source->y - y;
	L.z = source->z - z;
	L = normalise(L);

	s.x = source->pointsAtX - source->x;
	s.y = source->pointsAtY - source->y;
	s.z = source->pointsAtZ - source->z;
	s = normalise(s);

	base = -dotproduct(L, s);

	angle = acos(base) * 180.0 / M_PI;

	if (base < 0 || angle > source->limitingconeAngle)
		{
			output.x = 0;
			output.y = 0;
			output.z = 0;
			return output;
		}
	
	output.x = colour.x*pow(base, source->specularExponent);
	output.y = colour.y*pow(base, source->specularExponent);
	output.z = colour.z*pow(base, source->specularExponent);

	return output;
}


static void 
rsvg_filter_primitive_light_source_set_atts (RsvgNode * self,
											 RsvgHandle * ctx,
											 RsvgPropertyBag * atts)
{
	RsvgNodeLightSource * data;
	const char *value;
	double font_size;
	font_size = rsvg_state_current_font_size (ctx);

	data = (RsvgNodeLightSource *)self;

	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "azimuth")))
				{
					data->x = rsvg_css_parse_angle(value) / 180.0 * M_PI; 
				}
			if ((value = rsvg_property_bag_lookup (atts, "elevation")))
				{
					data->y = rsvg_css_parse_angle(value) / 180.0 * M_PI;
				}
			if ((value = rsvg_property_bag_lookup (atts, "limitingConeAngle")))
				{
					data->limitingconeAngle = rsvg_css_parse_angle(value);
				}
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					data->x = rsvg_css_parse_normalized_length(value, ctx->dpi_x,
															   1, font_size); 
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					data->y = rsvg_css_parse_normalized_length(value, ctx->dpi_y,
															   1, font_size); 
				}
			if ((value = rsvg_property_bag_lookup (atts, "z")))
				{
					data->z = rsvg_css_parse_normalized_length(value, rsvg_dpi_percentage (ctx),
															   1, font_size); 
				}
			if ((value = rsvg_property_bag_lookup (atts, "pointsAtX")))
				{
					data->pointsAtX = rsvg_css_parse_normalized_length(value, ctx->dpi_x,
																	   1, font_size); 
				}
			if ((value = rsvg_property_bag_lookup (atts, "pointsAtY")))
				{
					data->pointsAtY = rsvg_css_parse_normalized_length(value, ctx->dpi_y,
																	   1, font_size); 
				}
			if ((value = rsvg_property_bag_lookup (atts, "pointsAtZ")))
				{
					data->pointsAtZ = rsvg_css_parse_normalized_length(value, rsvg_dpi_percentage (ctx),
																	   1, font_size);
				}  
			if ((value = rsvg_property_bag_lookup (atts, "specularExponent")))
				data->specularExponent = g_ascii_strtod(value, NULL);
		}
}

static void
rsvg_filter_primitive_light_source_free(RsvgNode * tofree)
{
	g_free(tofree);
}

RsvgNode *
rsvg_new_filter_primitive_light_source(char type)
{
	RsvgNodeLightSource * data;
	data = g_new(RsvgNodeLightSource, 1);
	_rsvg_node_init(&data->super);
	data->super.free = rsvg_filter_primitive_light_source_free;
	data->super.set_atts = rsvg_filter_primitive_light_source_set_atts;
	data->specularExponent = 1;
	data->super.type = RSVG_NODE_FILTER_PRIMITIVE_MERGE_NODE;
	if (type == 's')
		data->type = SPOTLIGHT;
	else if (type == 'd')
		data->type = DISTANTLIGHT;
	else 
		data->type = POINTLIGHT;
	data->limitingconeAngle = 180;
	return &data->super;
}

/*************************************************************/
/*************************************************************/

typedef struct _RsvgFilterPrimitiveDiffuseLighting RsvgFilterPrimitiveDiffuseLighting;

struct _RsvgFilterPrimitiveDiffuseLighting
{
	RsvgFilterPrimitive super;
	gdouble dx, dy;
	double diffuseConstant;
	double surfaceScale;
	guint32 lightingcolour;
};

static void
rsvg_filter_primitive_diffuse_lighting_render (RsvgFilterPrimitive * self,
											   RsvgFilterContext * ctx)
{
	gint x, y;
	float dy, dx, rawdy, rawdx;
	gdouble z;
	gint rowstride, height, width;
	gdouble factor, surfaceScale;
	vector3 lightcolour, L, N;
	vector3 colour;
	gdouble iaffine[6];
	RsvgNodeLightSource * source = NULL;
	FPBox boundarys;
	
	guchar *in_pixels;
	guchar *output_pixels;
	
	RsvgFilterPrimitiveDiffuseLighting *upself;
	
	GdkPixbuf *output;
	GdkPixbuf *in;
	unsigned int i;	

	for (i = 0; i < self->super.children->len; i++)
		{
			RsvgNode * temp;
			temp = g_ptr_array_index(self->super.children, i);
			if (temp->type == RSVG_NODE_FILTER_PRIMITIVE_MERGE_NODE)
				source = (RsvgNodeLightSource *)temp;
		}
	if (source == NULL)
		return;

	upself = (RsvgFilterPrimitiveDiffuseLighting *) self;
	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	in = rsvg_filter_get_in (self->in, ctx);
	in_pixels = gdk_pixbuf_get_pixels (in);
	
	height = gdk_pixbuf_get_height (in);
	width = gdk_pixbuf_get_width (in);
	
	rowstride = gdk_pixbuf_get_rowstride (in);
	
	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, width, height);
	
	output_pixels = gdk_pixbuf_get_pixels (output);
	
	colour.x = ((guchar *)(&upself->lightingcolour))[2] / 255.0;
	colour.y = ((guchar *)(&upself->lightingcolour))[1] / 255.0;
	colour.z = ((guchar *)(&upself->lightingcolour))[0] / 255.0;

	surfaceScale =  upself->surfaceScale / 255.0;

	if (upself->dy < 0 || upself->dx < 0)
		{
			dx = 1;
			dy = 1;
			rawdx = 1;
			rawdy = 1;
		}
	else 
		{
			dx = upself->dx * ctx->paffine[0];
			dy = upself->dy * ctx->paffine[3];
			rawdx = upself->dx;
			rawdy = upself->dy;
		}

	_rsvg_affine_invert(iaffine, ctx->paffine);

	for (y = boundarys.y1; y < boundarys.y2; y++)
		for (x = boundarys.x1; x < boundarys.x2; x++)
			{
				z = surfaceScale * (double)in_pixels[y * rowstride + x * 4 + 3];
				L = get_light_direction(source, x, y, z, iaffine);
				N = get_surface_normal(in_pixels, boundarys, x, y, 
									   dx, dy, rawdx, rawdy, upself->surfaceScale, 
									   rowstride);
				lightcolour = get_light_colour(source, colour, x, y, z,
											   iaffine);
				factor = dotproduct(N, L);

				output_pixels[y * rowstride + x * 4    ] = MAX(0,MIN(255, upself->diffuseConstant * factor * 
					lightcolour.x * 255.0));
				output_pixels[y * rowstride + x * 4 + 1] = MAX(0,MIN(255, upself->diffuseConstant * factor * 
					lightcolour.y * 255.0));
				output_pixels[y * rowstride + x * 4 + 2] = MAX(0,MIN(255, upself->diffuseConstant * factor * 
					lightcolour.z * 255.0));
				output_pixels[y * rowstride + x * 4 + 3] = 255;
			}
	
	rsvg_filter_store_result (self->result, output, ctx);
	
	g_object_unref (G_OBJECT (in));
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_diffuse_lighting_free (RsvgNode * self)
{
	RsvgFilterPrimitiveDiffuseLighting *upself;
	
	upself = (RsvgFilterPrimitiveDiffuseLighting *) self;
	g_string_free (upself->super.result, TRUE);
	g_string_free (upself->super.in, TRUE);
	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_diffuse_lighting_set_atts (RsvgNode * self, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	const char *value;
	double font_size;
	RsvgFilterPrimitiveDiffuseLighting *filter;
	
	font_size = rsvg_state_current_font_size (ctx);
	
	filter = (RsvgFilterPrimitiveDiffuseLighting *)self;
	
	
	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (filter->super.in, value);
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "kernelUnitLength")))
				rsvg_css_parse_number_optional_number (value,
													   &filter->dx, &filter->dy);
			if ((value = rsvg_property_bag_lookup (atts, "lighting-color")))
				filter->lightingcolour = rsvg_css_parse_color (value, 0);
			if ((value = rsvg_property_bag_lookup (atts, "diffuseConstant")))
				filter->diffuseConstant = 
					g_ascii_strtod(value, NULL);
			if ((value = rsvg_property_bag_lookup (atts, "surfaceScale")))
				filter->surfaceScale = 
					g_ascii_strtod(value, NULL);
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}
}

RsvgNode *
rsvg_new_filter_primitive_diffuse_lighting (void)
{
	RsvgFilterPrimitiveDiffuseLighting *filter;
	filter = g_new (RsvgFilterPrimitiveDiffuseLighting, 1);
	_rsvg_node_init(&filter->super.super);
	filter->super.in = g_string_new ("none");
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;
	filter->surfaceScale = 1;
	filter->diffuseConstant = 1;
	filter->dx = 1;
	filter->dy = 1;
	filter->lightingcolour = 0xFFFFFFFF;
	filter->super.render = &rsvg_filter_primitive_diffuse_lighting_render;
	filter->super.super.free = &rsvg_filter_primitive_diffuse_lighting_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_diffuse_lighting_set_atts;
	filter->super.super.children = g_ptr_array_new();
	return (RsvgNode *)filter;
}

/*************************************************************/
/*************************************************************/

typedef struct _RsvgFilterPrimitiveSpecularLighting RsvgFilterPrimitiveSpecularLighting;

struct _RsvgFilterPrimitiveSpecularLighting
{
	RsvgFilterPrimitive super;
	double specularConstant;
	double specularExponent;
	double surfaceScale;
	guint32 lightingcolour;
};

static void
rsvg_filter_primitive_specular_lighting_render (RsvgFilterPrimitive * self,
											   RsvgFilterContext * ctx)
{
	gint x, y, temp;
	gdouble z, surfaceScale;
	gint rowstride, height, width;
	gdouble factor, max, base;
	vector3 lightcolour;
	vector3 colour;
	vector3 L;
	gdouble iaffine[6];
	FPBox boundarys;
	RsvgNodeLightSource *source = NULL;	

	guchar *in_pixels;
	guchar *output_pixels;
	
	RsvgFilterPrimitiveSpecularLighting *upself;
	
	GdkPixbuf *output;
	GdkPixbuf *in;

	unsigned int i;	

	for (i = 0; i < self->super.children->len; i++)
		{
			RsvgNode * temp;
			temp = g_ptr_array_index(self->super.children, i);
			if (temp->type == RSVG_NODE_FILTER_PRIMITIVE_MERGE_NODE)
				source = (RsvgNodeLightSource *)temp;
		}
	if (source == NULL)
		return;
	
	upself = (RsvgFilterPrimitiveSpecularLighting *) self;
	boundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	in = rsvg_filter_get_in (self->in, ctx);
	in_pixels = gdk_pixbuf_get_pixels (in);
	
	height = gdk_pixbuf_get_height (in);
	width = gdk_pixbuf_get_width (in);
	
	rowstride = gdk_pixbuf_get_rowstride (in);
	
	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, width, height);
	
	output_pixels = gdk_pixbuf_get_pixels (output);
	
	colour.x = ((guchar *)(&upself->lightingcolour))[2] / 255.0;
	colour.y = ((guchar *)(&upself->lightingcolour))[1] / 255.0;
	colour.z = ((guchar *)(&upself->lightingcolour))[0] / 255.0;

	surfaceScale = upself->surfaceScale / 255.0; 

	_rsvg_affine_invert(iaffine, ctx->paffine);

	for (y = boundarys.y1; y < boundarys.y2; y++)
		for (x = boundarys.x1; x < boundarys.x2; x++)
			{
				z = in_pixels[y * rowstride + x * 4 + 3] * surfaceScale;
				L = get_light_direction(source, x, y, z, iaffine);	
				L.z += 1;
				L = normalise(L);

				lightcolour = get_light_colour(source, colour, x, y, z, 
											   iaffine);
				base = dotproduct(get_surface_normal(in_pixels, boundarys, x, y, 
													 1, 1, 1.0 /  ctx->paffine[0], 1.0 / ctx->paffine[3], 
													 upself->surfaceScale, 
													 rowstride), L);
				
				factor = pow(base, upself->specularExponent);

				max = 0;
				temp = upself->specularConstant * factor* lightcolour.x * 255.0;		
				if (temp < 0)
					temp = 0;				
				if (temp > 255)
					temp = 255;
				max = MAX(temp, max);
				output_pixels[y * rowstride + x * 4    ] = temp;
				temp = upself->specularConstant * factor * lightcolour.y * 255.0;
				if (temp < 0)
					temp = 0;				
				if (temp > 255)
					temp = 255;
				max = MAX(temp, max);
				output_pixels[y * rowstride + x * 4 + 1] = temp;
				temp = upself->specularConstant * factor * lightcolour.z * 255.0;
				if (temp < 0)
					temp = 0;				
				if (temp > 255)
					temp = 255;
				max = MAX(temp, max);		
				output_pixels[y * rowstride + x * 4 + 2] = temp;

				output_pixels[y * rowstride + x * 4 + 3] = max;
			}
	
	rsvg_filter_store_result (self->result, output, ctx);

	g_object_unref (G_OBJECT (in));
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_specular_lighting_free (RsvgNode * self)
{
	RsvgFilterPrimitiveSpecularLighting *upself;
	
	upself = (RsvgFilterPrimitiveSpecularLighting *) self;
	g_string_free (upself->super.result, TRUE);
	g_string_free (upself->super.in, TRUE);
	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_specular_lighting_set_atts (RsvgNode * self, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	const char *value;
	double font_size;
	RsvgFilterPrimitiveSpecularLighting *filter;
	
	font_size = rsvg_state_current_font_size (ctx);
	
	filter = (RsvgFilterPrimitiveSpecularLighting *)self;
	
	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (filter->super.in, value);
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "lighting-color")))
				filter->lightingcolour = rsvg_css_parse_color (value, 0);
			if ((value = rsvg_property_bag_lookup (atts, "specularConstant")))
				filter->specularConstant = 
					g_ascii_strtod(value, NULL);
			if ((value = rsvg_property_bag_lookup (atts, "specularExponent")))
				filter->specularExponent = 
					g_ascii_strtod(value, NULL);
			if ((value = rsvg_property_bag_lookup (atts, "surfaceScale")))
				filter->surfaceScale = 
					g_ascii_strtod(value, NULL);
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}
}


RsvgNode *
rsvg_new_filter_primitive_specular_lighting (void)
{
	RsvgFilterPrimitiveSpecularLighting *filter;
	filter = g_new (RsvgFilterPrimitiveSpecularLighting, 1);	
	_rsvg_node_init(&filter->super.super);
	filter->super.in = g_string_new ("none");
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;
	filter->surfaceScale = 1;
	filter->specularConstant = 1;
	filter->specularExponent = 1;
	filter->lightingcolour = 0xFFFFFFFF;
	filter->super.render = &rsvg_filter_primitive_specular_lighting_render;
	filter->super.super.free = &rsvg_filter_primitive_specular_lighting_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_specular_lighting_set_atts;
	filter->super.super.children = g_ptr_array_new();
	return (RsvgNode *)filter;
}

/*************************************************************/
/*************************************************************/

typedef struct _RsvgFilterPrimitiveTile
RsvgFilterPrimitiveTile;

struct _RsvgFilterPrimitiveTile
{
	RsvgFilterPrimitive super;
};

static int
mod(int a, int b)
{
	while (a < 0)
		a += b;
	return a % b;
}

static void
rsvg_filter_primitive_tile_render (RsvgFilterPrimitive * self,
									RsvgFilterContext * ctx)
{
	guchar i;
	gint x, y, rowstride;
	FPBox boundarys, oboundarys;

	RsvgFilterPrimitiveOutput input;

	guchar *in_pixels;
	guchar *output_pixels;
	
	GdkPixbuf *output;
	GdkPixbuf *in;
		
	RsvgFilterPrimitiveTile *upself;
	
	upself = (RsvgFilterPrimitiveTile *) self;
	oboundarys = rsvg_filter_primitive_get_bounds (self, ctx);
	
	input = rsvg_filter_get_result (self->in, ctx);
	in = input.result;
	boundarys = input.bounds;
   

	in_pixels = gdk_pixbuf_get_pixels (in);

	output = _rsvg_pixbuf_new_cleared (GDK_COLORSPACE_RGB, 1, 8, ctx->width, ctx->height);
	rowstride = gdk_pixbuf_get_rowstride (output);
	
	output_pixels = gdk_pixbuf_get_pixels (output);

	for (y = oboundarys.y1; y < oboundarys.y2; y++)
		for (x = oboundarys.x1; x < oboundarys.x2; x++)
			for (i = 0; i < 4; i++)
				{
					output_pixels[4 * x + y * rowstride + i] = 
						in_pixels[(mod((x - boundarys.x1), (boundarys.x2 - boundarys.x1)) + 
								   boundarys.x1) * 4 + 
								  (mod((y - boundarys.y1), (boundarys.y2 - boundarys.y1)) + 
								   boundarys.y1) * rowstride + i];
				}
	
	rsvg_filter_store_result (self->result, output, ctx);
	
	g_object_unref (G_OBJECT (output));
}

static void
rsvg_filter_primitive_tile_free (RsvgNode * self)
{
	RsvgFilterPrimitiveTile *upself;
	
	upself = (RsvgFilterPrimitiveTile *) self;
	g_string_free (upself->super.result, TRUE);
	g_string_free (upself->super.in, TRUE);
	_rsvg_node_free(self);
}

static void
rsvg_filter_primitive_tile_set_atts (RsvgNode * self, RsvgHandle * ctx, RsvgPropertyBag * atts)
{
	const char *value;
	double font_size;
	RsvgFilterPrimitiveTile *filter;

	font_size = rsvg_state_current_font_size (ctx);
	
	filter = (RsvgFilterPrimitiveTile*)self;

	if (rsvg_property_bag_size (atts))
		{
			if ((value = rsvg_property_bag_lookup (atts, "in")))
				g_string_assign (filter->super.in, value);
			if ((value = rsvg_property_bag_lookup (atts, "result")))
				g_string_assign (filter->super.result, value);
			if ((value = rsvg_property_bag_lookup (atts, "x")))
				{
					filter->super.x =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "y")))
				{
					filter->super.y =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "width")))
				{
					filter->super.width =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_x,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "height")))
				{
					filter->super.height =
						rsvg_css_parse_normalized_length (value,
														  ctx->dpi_y,
														  1,
														  font_size);
					filter->super.sizedefaults = 0;
				}
			if ((value = rsvg_property_bag_lookup (atts, "id")))
				rsvg_defs_register_name (ctx->defs, value, &filter->super.super);
		}
}

RsvgNode *
rsvg_new_filter_primitive_tile (void)
{
	RsvgFilterPrimitiveTile *filter;
	filter = g_new (RsvgFilterPrimitiveTile, 1);
	_rsvg_node_init(&filter->super.super);
	filter->super.in = g_string_new ("none");
	filter->super.result = g_string_new ("none");
	filter->super.sizedefaults = 1;
	filter->super.render = &rsvg_filter_primitive_tile_render;
	filter->super.super.free = &rsvg_filter_primitive_tile_free;
	filter->super.super.type = RSVG_NODE_FILTER_PRIMITIVE;
	filter->super.super.set_atts = rsvg_filter_primitive_tile_set_atts;
	return (RsvgNode *)filter;
}
