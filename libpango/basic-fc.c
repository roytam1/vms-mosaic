/* Pango
 * basic-fc.c: Basic shaper for FreeType-based backends
 *
 * Copyright (C) 2000 Red Hat Software
 * Author: Owen Taylor <otaylor@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>

#include <glib/gprintf.h>
#include "pango-engine.h"
#include "pango-utils.h"
#include "pangofc-font.h"

#include "basic-common.h"

/* No extra fields needed */
typedef PangoEngineShape      BasicEngineFc;
typedef PangoEngineShapeClass BasicEngineFcClass;

#define SCRIPT_ENGINE_NAME "BasicScriptEngineFc"
#define RENDER_TYPE PANGO_RENDER_TYPE_FC

static PangoEngineScriptInfo basic_scripts[] = {
  { PANGO_SCRIPT_ARMENIAN, "*" },
  { PANGO_SCRIPT_BOPOMOFO, "*" },
  { PANGO_SCRIPT_CHEROKEE, "*" },
  { PANGO_SCRIPT_COPTIC,   "*" },
  { PANGO_SCRIPT_CYRILLIC, "*" },
  { PANGO_SCRIPT_DESERET,  "*" },
  { PANGO_SCRIPT_ETHIOPIC, "*" },
  { PANGO_SCRIPT_GEORGIAN, "*" },
  { PANGO_SCRIPT_GOTHIC,   "*" },
  { PANGO_SCRIPT_GREEK,    "*" },
  { PANGO_SCRIPT_HAN,      "*" },
  { PANGO_SCRIPT_HIRAGANA, "*" },
  { PANGO_SCRIPT_KATAKANA, "*" },
  { PANGO_SCRIPT_LATIN,    "*" },
  { PANGO_SCRIPT_OGHAM,    "*" },
  { PANGO_SCRIPT_OLD_ITALIC, "*" },
  { PANGO_SCRIPT_RUNIC,     "*" },
  { PANGO_SCRIPT_CANADIAN_ABORIGINAL, "*" },
  { PANGO_SCRIPT_YI,       "*" },
  { PANGO_SCRIPT_BRAILLE,  "*" },
  { PANGO_SCRIPT_CYPRIOT,  "*" },
  { PANGO_SCRIPT_LIMBU,    "*" },
  { PANGO_SCRIPT_OSMANYA,  "*" },
  { PANGO_SCRIPT_SHAVIAN,  "*" },
  { PANGO_SCRIPT_LINEAR_B, "*" },
  { PANGO_SCRIPT_UGARITIC, "*" },
    
  { PANGO_SCRIPT_COMMON,   "" }
};

static PangoEngineInfo script_engines[] = {
  {
    SCRIPT_ENGINE_NAME,
    PANGO_ENGINE_TYPE_SHAPE,
    RENDER_TYPE,
    basic_scripts, G_N_ELEMENTS(basic_scripts)
  }
};

static void
swap_range (PangoGlyphString *glyphs,
	    int               start,
	    int               end)
{
  int i, j;
  
  for (i = start, j = end - 1; i < j; i++, j--)
    {
      PangoGlyphInfo glyph_info;
      gint log_cluster;
      
      glyph_info = glyphs->glyphs[i];
      glyphs->glyphs[i] = glyphs->glyphs[j];
      glyphs->glyphs[j] = glyph_info;
      
      log_cluster = glyphs->log_clusters[i];
      glyphs->log_clusters[i] = glyphs->log_clusters[j];
      glyphs->log_clusters[j] = log_cluster;
    }
}

static void
set_glyph (PangoFont        *font,
	   PangoGlyphString *glyphs,
	   int               i,
	   int               offset,
	   PangoGlyph        glyph)
{
  PangoRectangle logical_rect;

  glyphs->glyphs[i].glyph = glyph;
  
  glyphs->glyphs[i].geometry.x_offset = 0;
  glyphs->glyphs[i].geometry.y_offset = 0;

  glyphs->log_clusters[i] = offset;

  pango_font_get_glyph_extents (font, glyphs->glyphs[i].glyph, NULL, &logical_rect);
  glyphs->glyphs[i].geometry.width = logical_rect.width;
}

static void 
basic_engine_shape (PangoEngineShape *engine,
		    PangoFont        *font,
		    const char       *text,
		    gint              length,
		    PangoAnalysis    *analysis,
		    PangoGlyphString *glyphs)
{
  PangoFcFont *fc_font = PANGO_FC_FONT (font);
  int n_chars;
  int i;
  const char *p;

  g_return_if_fail (font != NULL);
  g_return_if_fail (text != NULL);
  g_return_if_fail (length >= 0);
  g_return_if_fail (analysis != NULL);

  n_chars = g_utf8_strlen (text, length);
  pango_glyph_string_set_size (glyphs, n_chars);

  pango_fc_font_lock_face (fc_font);

  p = text;
  for (i = 0; i < n_chars; i++)
    {
      gunichar wc;
      gunichar mirrored_ch;
      PangoGlyph index;

      wc = g_utf8_get_char (p);

      if (analysis->level % 2)
	if (pango_get_mirror_char (wc, &mirrored_ch))
	  wc = mirrored_ch;

      if (wc == 0xa0)	/* non-break-space */
	wc = 0x20;
		
      if (pango_is_zero_width (wc))
	{
	  set_glyph (font, glyphs, i, p - text, 0);
	}
      else
	{
	  index = pango_fc_font_get_glyph (fc_font, wc);

	  if (!index)
	    {
	      set_glyph (font, glyphs, i, p - text,
			 pango_fc_font_get_unknown_glyph (fc_font, wc));
	    }
	  else
	    {
	      set_glyph (font, glyphs, i, p - text, index);
	      
	      if (g_unichar_type (wc) == G_UNICODE_NON_SPACING_MARK)
		{
		  if (i > 0)
		    {
		      PangoRectangle logical_rect, ink_rect;
		      
		      glyphs->glyphs[i].geometry.width = MAX (glyphs->glyphs[i-1].geometry.width,
							      glyphs->glyphs[i].geometry.width);
		      glyphs->glyphs[i-1].geometry.width = 0;
		      glyphs->log_clusters[i] = glyphs->log_clusters[i-1];
		      
		      /* Some heuristics to try to guess how overstrike glyphs are
		       * done and compensate
		       */
		      pango_font_get_glyph_extents (font, glyphs->glyphs[i].glyph, &ink_rect, &logical_rect);
		      if (logical_rect.width == 0 && ink_rect.x == 0)
			glyphs->glyphs[i].geometry.x_offset = (glyphs->glyphs[i].geometry.width - ink_rect.width) / 2;
		    }
		}
	    }
	}
      
      p = g_utf8_next_char (p);
    }

  /* Simple bidi support; most bidi languages (Arabic, Hebrew, Syriac)
   * are in fact handled in other modules.
   */
  if (analysis->level % 2)
    {
      int start, end;

      /* Swap all glyphs */
      swap_range (glyphs, 0, n_chars);
      
      /* Now reorder glyphs within each cluster back to LTR */
      for (start = 0; start < n_chars;)
	{
	  end = start;
	  while (end < n_chars &&
		 glyphs->log_clusters[end] == glyphs->log_clusters[start])
	    end++;
	  
	  swap_range (glyphs, start, end);
	  start = end;
	}
    }

  pango_fc_font_kern_glyphs (fc_font, glyphs);

  pango_fc_font_unlock_face (fc_font);
}

static void
basic_engine_fc_class_init (PangoEngineShapeClass *class)
{
  class->script_shape = basic_engine_shape;
}

#if defined(VMS) && !defined(__STDC__)
#undef PANGO_ENGINE_DEFINE_TYPE
#define PANGO_ENGINE_DEFINE_TYPE(name, prefix, class_init, instance_init, parent_type) \
static GType prefix/**/_type;						  \
static void								  \
prefix/**/_register_type (GTypeModule *module)				  \
{									  \
  static const GTypeInfo object_info =					  \
    {									  \
      sizeof (name/**/Class),						  \
      (GBaseInitFunc) NULL,						  \
      (GBaseFinalizeFunc) NULL,						  \
      (GClassInitFunc) class_init,					  \
      (GClassFinalizeFunc) NULL,					  \
      NULL,           /* class_data */					  \
      sizeof (name),					  		  \
      0,             /* n_prelocs */					  \
      (GInstanceInitFunc) instance_init,				  \
    };									  \
									  \
  prefix/**/_type =  g_type_module_register_type (module, parent_type,	  \
					          "BasicEngineFc",	  \
						  &object_info, 0);	  \
}
#endif
PANGO_ENGINE_SHAPE_DEFINE_TYPE (BasicEngineFc, basic_engine_fc,
				basic_engine_fc_class_init, NULL)

void 
PANGO_MODULE_ENTRY(init) (GTypeModule *module)
{
  basic_engine_fc_register_type (module);
}

void 
PANGO_MODULE_ENTRY(exit) (void)
{
}

void 
PANGO_MODULE_ENTRY(list) (PangoEngineInfo **engines,
			  int              *n_engines)
{
  *engines = script_engines;
  *n_engines = G_N_ELEMENTS (script_engines);
}

PangoEngine *
PANGO_MODULE_ENTRY(create) (const char *id)
{
  if (!strcmp (id, SCRIPT_ENGINE_NAME))
    return g_object_new (basic_engine_fc_type, NULL);
  else
    return NULL;
}
