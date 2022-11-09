/* vim: set sw=4: -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
   rsvg-filter.h : Provides filters

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

   Author: Caleb Moore <calebmm@tpg.com.au>
*/

#ifndef RSVG_FILTER_H
#define RSVG_FILTER_H

#include "rsvg.h"
#include "rsvg-defs.h"
#include <libxml/SAX.h>

G_BEGIN_DECLS

typedef RsvgCoordUnits RsvgFilterUnits;

struct _RsvgFilter {
	RsvgNode super;
	int refcnt;
	double x, y, width, height; 
	RsvgFilterUnits filterunits;
	RsvgFilterUnits primitiveunits;
};

void 
rsvg_filter_render (RsvgFilter *self, GdkPixbuf *source, GdkPixbuf *output, GdkPixbuf *bg, RsvgDrawingCtx *context);

RsvgNode *
rsvg_new_filter (void);

RsvgFilter *
rsvg_filter_parse (const RsvgDefs *defs, const char *str);

RsvgNode * 
rsvg_new_filter_primitive_blend (void);

#define rsvg_new_filter_primitive_convolve_matrix rsvg_new_fil_prim_conv_matrix
RsvgNode * 
rsvg_new_filter_primitive_convolve_matrix (void);

#define rsvg_new_filter_primitive_gaussian_blur rsvg_new_fil_prim_gauss_blur
RsvgNode * 
rsvg_new_filter_primitive_gaussian_blur (void);

#define rsvg_new_filter_primitive_offset rsvg_new_fil_prim_offset
RsvgNode * 
rsvg_new_filter_primitive_offset (void);

RsvgNode * 
rsvg_new_filter_primitive_merge (void);

#define rsvg_new_filter_primitive_merge_node rsvg_new_fil_prim_merge_node
RsvgNode * 
rsvg_new_filter_primitive_merge_node (void);

#define rsvg_new_filter_primitive_colour_matrix rsvg_new_fil_prim_color_matrix
RsvgNode *
rsvg_new_filter_primitive_colour_matrix (void);

#define rsvg_new_filter_primitive_component_transfer rsvg_new_fil_prim_com_trans
RsvgNode *
rsvg_new_filter_primitive_component_transfer (void);

#define rsvg_new_node_component_transfer_function rsvg_new_node_comp_trans_func
RsvgNode *
rsvg_new_node_component_transfer_function (char channel);

RsvgNode *
rsvg_new_filter_primitive_erode (void);

#define rsvg_new_filter_primitive_composite rsvg_new_fil_prim_composite
RsvgNode *
rsvg_new_filter_primitive_composite (void);

RsvgNode *
rsvg_new_filter_primitive_flood (void);

#define rsvg_new_filter_primitive_displacement_map rsvg_new_fil_prime_displac_map
RsvgNode *
rsvg_new_filter_primitive_displacement_map (void);

#define rsvg_new_filter_primitive_turbulence rsvg_new_fil_prim_turbul
RsvgNode *
rsvg_new_filter_primitive_turbulence (void);

RsvgNode *
rsvg_new_filter_primitive_image (void);

#define rsvg_new_filter_primitive_diffuse_lighting rsvg_new_fil_prim_diff_light
RsvgNode *
rsvg_new_filter_primitive_diffuse_lighting (void);

#define rsvg_new_filter_primitive_light_source rsvg_new_filt_prim_light_src
RsvgNode *
rsvg_new_filter_primitive_light_source (char type);

#define rsvg_new_filter_primitive_specular_lighting rsvg_new_fil_prim_spec_light
RsvgNode *
rsvg_new_filter_primitive_specular_lighting (void);

RsvgNode *
rsvg_new_filter_primitive_tile (void);

void
rsvg_filter_adobe_blend(gint modenum, GdkPixbuf *in, GdkPixbuf *bg, GdkPixbuf *output,
						RsvgDrawingCtx * ctx);

G_END_DECLS

#endif
