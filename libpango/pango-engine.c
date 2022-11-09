/* Pango
 * pango-engine.c: Engines for script and language specific processing
 *
 * Copyright (C) 2003 Red Hat Software
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

#include "pango-engine.h"
#include "pango-engine-private.h"
#include "pango-impl-utils.h"

#if defined(VMS) && !defined(__STDC__)
#undef PANGO_DEFINE_TYPE_FULL
#define PANGO_DEFINE_TYPE_FULL(name, prefix,				   \
			       class_init, instance_init,		   \
			       parent_type, abstract)			   \
GType								   	   \
prefix/**/_get_type (void)				                   \
{									   \
  static GType object_type = 0;						   \
  									   \
  if (!object_type)							   \
    {									   \
      static const GTypeInfo object_info =				   \
	{								   \
	  sizeof (name/**/Class),					   \
	  (GBaseInitFunc) NULL,						   \
	  (GBaseFinalizeFunc) NULL,					   \
	  (GClassInitFunc) class_init,					   \
	  (GClassFinalizeFunc) NULL,					   \
	  NULL,           /* class_data */				   \
	  sizeof (name),						   \
	  0,             /* n_prelocs */				   \
	  (GInstanceInitFunc) instance_init				   \
	};								   \
      									   \
      object_type = g_type_register_static (parent_type,	   	   \
					    "PangoEngine",	   	   \
					    &object_info, abstract); 	   \
    }									   \
									   \
  return object_type;							   \
}
#endif
PANGO_DEFINE_TYPE_ABSTRACT (PangoEngine, pango_engine,
			    NULL, NULL,
			    G_TYPE_OBJECT)

#if defined(VMS) && !defined(__STDC__)
#undef PANGO_DEFINE_TYPE_FULL
#define PANGO_DEFINE_TYPE_FULL(name, prefix,				   \
			       class_init, instance_init,		   \
			       parent_type, abstract)			   \
GType								   	   \
prefix/**/_get_type (void)				                   \
{									   \
  static GType object_type = 0;						   \
  									   \
  if (!object_type)							   \
    {									   \
      static const GTypeInfo object_info =				   \
	{								   \
	  sizeof (name/**/Class),					   \
	  (GBaseInitFunc) NULL,						   \
	  (GBaseFinalizeFunc) NULL,					   \
	  (GClassInitFunc) class_init,					   \
	  (GClassFinalizeFunc) NULL,					   \
	  NULL,           /* class_data */				   \
	  sizeof (name),						   \
	  0,             /* n_prelocs */				   \
	  (GInstanceInitFunc) instance_init				   \
	};								   \
      									   \
      object_type = g_type_register_static (parent_type,	   	   \
					    "PangoEngineLang",	   	   \
					    &object_info, abstract); 	   \
    }									   \
									   \
  return object_type;							   \
}
#endif
PANGO_DEFINE_TYPE_ABSTRACT (PangoEngineLang, pango_engine_lang,
			    NULL, NULL,
			    PANGO_TYPE_ENGINE)

static PangoCoverageLevel
pango_engine_shape_real_covers (PangoEngineShape *engine,
				PangoFont        *font,
				PangoLanguage    *language,
				gunichar          wc)
{
  
  PangoCoverage *coverage = pango_font_get_coverage (font, language);
  PangoCoverageLevel result = pango_coverage_get (coverage, wc);

  pango_coverage_unref (coverage);

  return result;
}

void
pango_engine_shape_class_init (PangoEngineShapeClass *class)
{
  class->covers = pango_engine_shape_real_covers;
}

#if defined(VMS) && !defined(__STDC__)
#undef PANGO_DEFINE_TYPE_FULL
#define PANGO_DEFINE_TYPE_FULL(name, prefix,				   \
			       class_init, instance_init,		   \
			       parent_type, abstract)			   \
GType								   	   \
prefix/**/_get_type (void)				                   \
{									   \
  static GType object_type = 0;						   \
  									   \
  if (!object_type)							   \
    {									   \
      static const GTypeInfo object_info =				   \
	{								   \
	  sizeof (name/**/Class),					   \
	  (GBaseInitFunc) NULL,						   \
	  (GBaseFinalizeFunc) NULL,					   \
	  (GClassInitFunc) class_init,					   \
	  (GClassFinalizeFunc) NULL,					   \
	  NULL,           /* class_data */				   \
	  sizeof (name),						   \
	  0,             /* n_prelocs */				   \
	  (GInstanceInitFunc) instance_init				   \
	};								   \
      									   \
      object_type = g_type_register_static (parent_type,	   	   \
					    "PangoEngineShape",	   	   \
					    &object_info, abstract); 	   \
    }									   \
									   \
  return object_type;							   \
}
#endif
PANGO_DEFINE_TYPE_ABSTRACT (PangoEngineShape, pango_engine_shape,
			    pango_engine_shape_class_init, NULL,
			    PANGO_TYPE_ENGINE)

void
_pango_engine_shape_shape (PangoEngineShape *engine,
			   PangoFont        *font,
			   const char       *text,
			   int               length,
			   PangoAnalysis    *analysis,
			   PangoGlyphString *glyphs)
{
  g_return_if_fail (PANGO_IS_ENGINE_SHAPE (engine));
  g_return_if_fail (PANGO_IS_FONT (font));
  g_return_if_fail (text != NULL);
  g_return_if_fail (analysis != NULL);
  g_return_if_fail (glyphs != NULL);

  PANGO_ENGINE_SHAPE_GET_CLASS (engine)->script_shape (engine,
						       font,
						       text, length,
						       analysis,
						       glyphs);
}

PangoCoverageLevel
_pango_engine_shape_covers (PangoEngineShape *engine,
			    PangoFont        *font,
			    PangoLanguage    *language,
			    gunichar          wc)
{
  g_return_val_if_fail (PANGO_IS_ENGINE_SHAPE (engine), PANGO_COVERAGE_NONE);
  g_return_val_if_fail (PANGO_IS_FONT (font), PANGO_COVERAGE_NONE);

  return PANGO_ENGINE_SHAPE_GET_CLASS (engine)->covers (engine,
							font,
							language,
							wc);
}

/* No extra fields needed */
typedef PangoEngineShape PangoFallbackEngine;
typedef PangoEngineShapeClass PangoFallbackEngineClass;

static void 
fallback_engine_shape (PangoEngineShape *engine,
		       PangoFont        *font,
                       const char       *text,
                       gint              length,
                       PangoAnalysis    *analysis,
                       PangoGlyphString *glyphs)
{
  int n_chars;
  int i;
  const char *p;
  
  g_return_if_fail (font != NULL);
  g_return_if_fail (text != NULL);
  g_return_if_fail (length >= 0);
  g_return_if_fail (analysis != NULL);

  n_chars = g_utf8_strlen (text, length);
  pango_glyph_string_set_size (glyphs, n_chars);
  
  p = text;
  i = 0;
  while (i < n_chars)
    {
      glyphs->glyphs[i].glyph = 0;
      
      glyphs->glyphs[i].geometry.x_offset = 0;
      glyphs->glyphs[i].geometry.y_offset = 0;
      glyphs->glyphs[i].geometry.width = 0;

      glyphs->log_clusters[i] = p - text;
      
      ++i;
      p = g_utf8_next_char (p);
    }
}

static PangoCoverageLevel
fallback_engine_covers (PangoEngineShape *engine,
			PangoFont        *font,
			PangoLanguage    *lang,
			gunichar          wc)
{
  return PANGO_COVERAGE_NONE;
}

static void
fallback_engine_class_init (PangoEngineShapeClass *class)
{
  class->covers = fallback_engine_covers;
  class->script_shape = fallback_engine_shape;
}

#if defined(VMS) && !defined(__STDC__)
#undef PANGO_DEFINE_TYPE_FULL
#define PANGO_DEFINE_TYPE_FULL(name, prefix,				   \
			       class_init, instance_init,		   \
			       parent_type, abstract)			   \
GType								   	   \
prefix/**/_get_type (void)				                   \
{									   \
  static GType object_type = 0;						   \
  									   \
  if (!object_type)							   \
    {									   \
      static const GTypeInfo object_info =				   \
	{								   \
	  sizeof (name/**/Class),					   \
	  (GBaseInitFunc) NULL,						   \
	  (GBaseFinalizeFunc) NULL,					   \
	  (GClassInitFunc) class_init,					   \
	  (GClassFinalizeFunc) NULL,					   \
	  NULL,           /* class_data */				   \
	  sizeof (name),						   \
	  0,             /* n_prelocs */				   \
	  (GInstanceInitFunc) instance_init				   \
	};								   \
      									   \
      object_type = g_type_register_static (parent_type,	   	   \
					    "PangoFallbackEngine",   	   \
					    &object_info, abstract); 	   \
    }									   \
									   \
  return object_type;							   \
}
#endif
static PANGO_DEFINE_TYPE (PangoFallbackEngine, pango_fallback_engine,
			  fallback_engine_class_init, NULL,
			  PANGO_TYPE_ENGINE_SHAPE)

PangoEngineShape *
_pango_get_fallback_shaper (void)
{
  static PangoEngineShape *fallback_shaper = NULL;
  if (!fallback_shaper)
    fallback_shaper = g_object_new (pango_fallback_engine_get_type (), NULL);

  return fallback_shaper;
}

