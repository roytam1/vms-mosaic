/* Pango
 * pangoft2.c: Routines for handling FreeType2 fonts
 *
 * Copyright (C) 1999 Red Hat Software
 * Copyright (C) 2000 Tor Lillqvist
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

#include "config.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "pangoft2.h"
#include "pangoft2-private.h"
#include "pangofc-fontmap.h"
#include "pangofc-private.h"

/* for compatibility with older freetype versions */
#ifndef FT_LOAD_TARGET_MONO
#define FT_LOAD_TARGET_MONO  FT_LOAD_MONOCHROME
#endif

#define PANGO_FT2_FONT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), PANGO_TYPE_FT2_FONT, PangoFT2FontClass))
#define PANGO_FT2_IS_FONT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PANGO_TYPE_FT2_FONT))
#define PANGO_FT2_FONT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), PANGO_TYPE_FT2_FONT, PangoFT2FontClass))

typedef struct _PangoFT2FontClass   PangoFT2FontClass;

struct _PangoFT2FontClass
{
  PangoFcFontClass parent_class;
};

static void pango_ft2_font_finalize   (GObject         *object);

static void                   pango_ft2_font_get_glyph_extents (PangoFont      *font,
								PangoGlyph      glyph,
								PangoRectangle *ink_rect,
								PangoRectangle *logical_rect);

static FT_Face    pango_ft2_font_real_lock_face         (PangoFcFont      *font);
static void       pango_ft2_font_real_unlock_face       (PangoFcFont      *font);
static PangoGlyph pango_ft2_font_real_get_unknown_glyph (PangoFcFont      *font,
							 gunichar          wc);

PangoFT2Font *
_pango_ft2_font_new (PangoFT2FontMap *ft2fontmap,
		     FcPattern       *pattern)
{
  PangoFontMap *fontmap = PANGO_FONT_MAP (ft2fontmap);
  PangoFT2Font *ft2font;
  double d;

  g_return_val_if_fail (fontmap != NULL, NULL);
  g_return_val_if_fail (pattern != NULL, NULL);

  ft2font = (PangoFT2Font *)g_object_new (PANGO_TYPE_FT2_FONT,
					  "pattern", pattern,
					  NULL);

  if (FcPatternGetDouble (pattern, FC_PIXEL_SIZE, 0, &d) == FcResultMatch)
    ft2font->size = d*PANGO_SCALE;

  return ft2font;
}

static void
load_fallback_face (PangoFT2Font *ft2font,
		    const char   *original_file)
{
  PangoFcFont *fcfont = PANGO_FC_FONT (ft2font);
  FcPattern *sans;
  FcPattern *matched;
  FcResult result;
  FT_Error error;
  FcChar8 *filename2 = NULL;
  gchar *name;
  int id;
  
  sans = FcPatternBuild (NULL,
			 FC_FAMILY,     FcTypeString, "sans",
			 FC_PIXEL_SIZE, FcTypeDouble, (double)ft2font->size / PANGO_SCALE,
			 NULL);
#ifdef VMS  
  if (!sans) {
    filename2 = "Unknown";
    goto bail1;
  }
#endif
  matched = FcFontMatch (NULL, sans, &result);
  
  if (FcPatternGetString (matched, FC_FILE, 0, &filename2) != FcResultMatch)
    goto bail1;
  
  if (FcPatternGetInteger (matched, FC_INDEX, 0, &id) != FcResultMatch)
    goto bail1;
  
  error = FT_New_Face (_pango_ft2_font_map_get_library (fcfont->fontmap),
		       (char *) filename2, id, &ft2font->face);
  
  
  if (error)
    {
    bail1:
      name = pango_font_description_to_string (fcfont->description);
      g_warning ("Unable to open font file %s for font %s, exiting\n", filename2, name);
      exit (1);
    }
  else
    {
      name = pango_font_description_to_string (fcfont->description);
      g_warning ("Unable to open font file %s for font %s, falling back to %s\n", original_file, name, filename2);
      g_free (name);
    }

  FcPatternDestroy (sans);
  FcPatternDestroy (matched);
}

static void
set_transform (PangoFT2Font *ft2font)
{
  PangoFcFont *fcfont = (PangoFcFont *)ft2font;
  FcMatrix *fc_matrix;

  if (FcPatternGetMatrix (fcfont->font_pattern, FC_MATRIX, 0, &fc_matrix) == FcResultMatch)
    {
      FT_Matrix ft_matrix;
      
      ft_matrix.xx = 0x10000L * fc_matrix->xx;
      ft_matrix.yy = 0x10000L * fc_matrix->yy;
      ft_matrix.xy = 0x10000L * fc_matrix->xy;
      ft_matrix.yx = 0x10000L * fc_matrix->yx;

      FT_Set_Transform (ft2font->face, &ft_matrix, NULL);
    }
}

/**
 * pango_ft2_font_get_face:
 * @font: a #PangoFont
 * 
 * Returns the native FreeType2 FT_Face structure used for this PangoFont.
 * This may be useful if you want to use FreeType2 functions directly.
 *
 * Use pango_fc_font_lock_face() instead; when you are done with a
 * face from pango_fc_font_lock_face() you must call
 * pango_fc_font_unlock_face().
 * 
 * Return value: a pointer to a #FT_Face structure, with the size set correctly
 **/
FT_Face
pango_ft2_font_get_face (PangoFont *font)
{
  PangoFT2Font *ft2font = (PangoFT2Font *)font;
  PangoFcFont *fcfont = (PangoFcFont *)font;
  FT_Error error;
  FcPattern *pattern;
  FcChar8 *filename;
  FcBool antialias, hinting, autohint;
  int id;

  pattern = fcfont->font_pattern;

  if (!ft2font->face)
    {
      ft2font->load_flags = 0;

      /* disable antialiasing if requested */
      if (FcPatternGetBool (pattern,
                            FC_ANTIALIAS, 0, &antialias) != FcResultMatch)
	antialias = FcTrue;

      if (antialias)
        ft2font->load_flags |= FT_LOAD_NO_BITMAP;
      else
	ft2font->load_flags |= FT_LOAD_TARGET_MONO;

      /* disable hinting if requested */
      if (FcPatternGetBool (pattern,
                            FC_HINTING, 0, &hinting) != FcResultMatch)
	hinting = FcTrue;

      if (!hinting)
        ft2font->load_flags |= FT_LOAD_NO_HINTING;

      /* force autohinting if requested */
      if (FcPatternGetBool (pattern,
                            FC_AUTOHINT, 0, &autohint) != FcResultMatch)
	autohint = FcFalse;

      if (autohint)
        ft2font->load_flags |= FT_LOAD_FORCE_AUTOHINT;

      if (FcPatternGetString (pattern, FC_FILE, 0, &filename) != FcResultMatch)
	      goto bail0;
      
      if (FcPatternGetInteger (pattern, FC_INDEX, 0, &id) != FcResultMatch)
	      goto bail0;

      error = FT_New_Face (_pango_ft2_font_map_get_library (fcfont->fontmap),
			   (char *) filename, id, &ft2font->face);
      if (error != FT_Err_Ok)
	{
	bail0:
	  load_fallback_face (ft2font, (const char *)filename);
	}

      g_assert (ft2font->face);

      set_transform (ft2font);

      error = FT_Set_Char_Size (ft2font->face,
				PANGO_PIXELS_26_6 (ft2font->size),
				PANGO_PIXELS_26_6 (ft2font->size),
				0, 0);
      if (error)
	g_warning ("Error in FT_Set_Char_Size: %d", error);
    }
  
  return ft2font->face;
}

#if defined(VMS) && !defined(__STDC__)
#undef G_DEFINE_TYPE_EXTENDED
#define G_DEFINE_TYPE_EXTENDED(TypeName, type_name, TYPE_PARENT, flags, CODE) \
\
static void     type_name/**/_init              (TypeName        *self); \
static void     type_name/**/_class_init        (TypeName/**/Class *klass); \
static gpointer type_name/**/_parent_class = NULL; \
static void     type_name/**/_class_intern_init (gpointer klass) \
{ \
  type_name/**/_parent_class = g_type_class_peek_parent (klass); \
  type_name/**/_class_init ((TypeName/**/Class*) klass); \
} \
\
GType \
type_name/**/_get_type (void) \
{ \
  static GType g_define_type_id = 0; \
  if (G_UNLIKELY (g_define_type_id == 0)) \
    { \
      static const GTypeInfo g_define_type_info = { \
        sizeof (TypeName/**/Class), \
        (GBaseInitFunc) NULL, \
        (GBaseFinalizeFunc) NULL, \
        (GClassInitFunc) type_name/**/_class_intern_init, \
        (GClassFinalizeFunc) NULL, \
        NULL,   /* class_data */ \
        sizeof (TypeName), \
        0,      /* n_preallocs */ \
        (GInstanceInitFunc) type_name/**/_init, \
        NULL    /* value_table */ \
      }; \
      g_define_type_id = g_type_register_static (TYPE_PARENT, "PangoFT2Font", &g_define_type_info, (GTypeFlags) flags); \
      { CODE ; } \
    } \
  return g_define_type_id; \
}
#endif

G_DEFINE_TYPE (PangoFT2Font, pango_ft2_font, PANGO_TYPE_FC_FONT)

static void 
pango_ft2_font_init (PangoFT2Font *ft2font)
{
  ft2font->face = NULL;

  ft2font->size = 0;

  ft2font->glyph_info = g_hash_table_new (NULL, NULL);
}

static void
pango_ft2_font_class_init (PangoFT2FontClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  PangoFontClass *font_class = PANGO_FONT_CLASS (class);
  PangoFcFontClass *fc_font_class = PANGO_FC_FONT_CLASS (class);
  
  object_class->finalize = pango_ft2_font_finalize;
  
  font_class->get_glyph_extents = pango_ft2_font_get_glyph_extents;
  
  fc_font_class->lock_face = pango_ft2_font_real_lock_face;
  fc_font_class->unlock_face = pango_ft2_font_real_unlock_face;
  fc_font_class->get_unknown_glyph = pango_ft2_font_real_get_unknown_glyph;
}

static PangoFT2GlyphInfo *
pango_ft2_font_get_glyph_info (PangoFont   *font,
			       PangoGlyph   glyph,
			       gboolean     create)
{
  PangoFT2Font *ft2font = (PangoFT2Font *)font;
  PangoFcFont *fcfont = (PangoFcFont *)font;
  PangoFT2GlyphInfo *info;

  info = g_hash_table_lookup (ft2font->glyph_info, GUINT_TO_POINTER (glyph));

  if ((info == NULL) && create)
    {
       info = g_new0 (PangoFT2GlyphInfo, 1);

      pango_fc_font_get_raw_extents (fcfont, ft2font->load_flags,
				     glyph,
				     &info->ink_rect,
				     &info->logical_rect);

      g_hash_table_insert (ft2font->glyph_info, GUINT_TO_POINTER(glyph), info);
    }

  return info;
}
     
static void
pango_ft2_font_get_glyph_extents (PangoFont      *font,
				  PangoGlyph      glyph,
				  PangoRectangle *ink_rect,
				  PangoRectangle *logical_rect)
{
  PangoFT2GlyphInfo *info;

  info = pango_ft2_font_get_glyph_info (font, glyph, TRUE);

  if (ink_rect)
    *ink_rect = info->ink_rect;
  if (logical_rect)
    *logical_rect = info->logical_rect;
}

/**
 * pango_ft2_font_get_kerning:
 * @font: a #PangoFont
 * @left: the left #PangoGlyph
 * @right: the right #PangoGlyph
 * 
 * Retrieves kerning information for a combination of two glyphs.
 *
 * Use pango_fc_font_kern_glyphs() instead.
 * 
 * Return value: The amount of kerning (in Pango units) to apply for 
 * the given combination of glyphs.
 **/
int
pango_ft2_font_get_kerning (PangoFont *font,
			    PangoGlyph left,
			    PangoGlyph right)
{
  PangoFcFont *fc_font = PANGO_FC_FONT (font);
  
  FT_Face face;
  FT_Error error;
  FT_Vector kerning;

  face = pango_fc_font_lock_face (fc_font);
  if (!face)
    return 0;

  if (!FT_HAS_KERNING (face))
    {
      pango_fc_font_unlock_face (fc_font);
      return 0;
    }

  error = FT_Get_Kerning (face, left, right, ft_kerning_default, &kerning);
  if (error != FT_Err_Ok)
    {
      pango_fc_font_unlock_face (fc_font);
      return 0;
    }

  pango_fc_font_unlock_face (fc_font);
  return PANGO_UNITS_26_6 (kerning.x);
}

static FT_Face
pango_ft2_font_real_lock_face (PangoFcFont *font)
{
  return pango_ft2_font_get_face ((PangoFont *)font);
}

static void
pango_ft2_font_real_unlock_face (PangoFcFont *font)
{
}

static PangoGlyph
pango_ft2_font_real_get_unknown_glyph (PangoFcFont *font,
				       gunichar     wc)
{
  return 0;
}

static gboolean
pango_ft2_free_glyph_info_callback (gpointer key, gpointer value, gpointer data)
{
  PangoFT2Font *font = PANGO_FT2_FONT (data);
  PangoFT2GlyphInfo *info = value;
  
  if (font->glyph_cache_destroy && info->cached_glyph)
    (*font->glyph_cache_destroy) (info->cached_glyph);

  g_free (value);
  return TRUE;
}

static void
pango_ft2_font_finalize (GObject *object)
{
  PangoFT2Font *ft2font = (PangoFT2Font *)object;

  if (ft2font->face)
    {
      FT_Done_Face (ft2font->face);
      ft2font->face = NULL;
    }

  g_hash_table_foreach_remove (ft2font->glyph_info,
			       pango_ft2_free_glyph_info_callback, object);
  g_hash_table_destroy (ft2font->glyph_info);
  
  G_OBJECT_CLASS (pango_ft2_font_parent_class)->finalize (object);
}

/**
 * pango_ft2_font_get_coverage:
 * @font: a #PangoFT2Font.
 * @language: a language tag.
 * @returns: a #PangoCoverage.
 * 
 * Gets the #PangoCoverage for a #PangoFT2Font. Use pango_font_get_coverage() instead.
 **/
PangoCoverage *
pango_ft2_font_get_coverage (PangoFont     *font,
			     PangoLanguage *language)
{
  return pango_font_get_coverage (font, language);
}

/* Utility functions */

/**
 * pango_ft2_get_unknown_glyph:
 * @font: a #PangoFont
 * 
 * Return the index of a glyph suitable for drawing unknown characters.
 * 
 * Use pango_fc_font_get_unknown_glyph() instead.
 * 
 * Return value: a glyph index into @font
 **/
PangoGlyph
pango_ft2_get_unknown_glyph (PangoFont *font)
{
  return 0;
}

typedef struct
{
  FT_Error     code;
  const char*  msg;
} ft_error_description;

static int
ft_error_compare (const void *pkey,
		  const void *pbase)
{
  return ((ft_error_description *) pkey)->code - ((ft_error_description *) pbase)->code;
}

const char *
_pango_ft2_ft_strerror (FT_Error error)
{
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST  {
#define FT_ERROR_END_LIST    { 0, 0 } };

  const ft_error_description ft_errors[] =
#include FT_ERRORS_H

#undef FT_ERRORDEF
#undef FT_ERROR_START_LIST
#undef FT_ERROR_END_LIST

  ft_error_description *found =
    bsearch (&error, ft_errors, G_N_ELEMENTS (ft_errors) - 1,
	     sizeof (ft_errors[0]), ft_error_compare);
  if (found != NULL)
    return found->msg;
  else
    {
      static char default_msg[100];

      g_sprintf (default_msg, "Unknown FreeType2 error %#x", error);
      return default_msg;
    }
}

void *
_pango_ft2_font_get_cache_glyph_data (PangoFont *font,
				     int        glyph_index)
{
  PangoFT2GlyphInfo *info;

  g_return_val_if_fail (PANGO_FT2_IS_FONT (font), NULL);
  
  info = pango_ft2_font_get_glyph_info (font, glyph_index, FALSE);

  if (info == NULL)
    return NULL;
  
  return info->cached_glyph;
}

void
_pango_ft2_font_set_cache_glyph_data (PangoFont     *font,
				     int            glyph_index,
				     void          *cached_glyph)
{
  PangoFT2GlyphInfo *info;

  g_return_if_fail (PANGO_FT2_IS_FONT (font));
  
  info = pango_ft2_font_get_glyph_info (font, glyph_index, TRUE);

  info->cached_glyph = cached_glyph;

  /* TODO: Implement limiting of the number of cached glyphs */
}

void
_pango_ft2_font_set_glyph_cache_destroy (PangoFont      *font,
					 GDestroyNotify  destroy_notify)
{
  g_return_if_fail (PANGO_FT2_IS_FONT (font));
  
  PANGO_FT2_FONT (font)->glyph_cache_destroy = destroy_notify;
}
