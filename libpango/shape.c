/* Pango
 * shape.c: Convert characters into glyphs.
 *
 * Copyright (C) 1999 Red Hat Software
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

#include <pango/pango-glyph.h>
#include <pango/pango-engine-private.h>

/**
 * pango_shape:
 * @text:      the text to process
 * @length:    the length (in bytes) of @text
 * @analysis:  #PangoAnalysis structure from pango_itemize()
 * @glyphs:    glyph string in which to store results
 *
 * Given a segment of text and the corresponding 
 * #PangoAnalysis structure returned from pango_itemize(),
 * convert the characters into glyphs. You may also pass
 * in only a substring of the item from pango_itemize().
 */
void
pango_shape (const gchar      *text, 
             gint              length, 
             PangoAnalysis    *analysis,
             PangoGlyphString *glyphs)
{
  int i;
  int last_cluster = -1;
  
  if (analysis->shape_engine)
    _pango_engine_shape_shape (analysis->shape_engine, analysis->font,
			       text, length, analysis, glyphs);
  else
    {
      pango_glyph_string_set_size (glyphs, 1);

      glyphs->glyphs[0].glyph = 0;
      
      glyphs->glyphs[0].geometry.x_offset = 0;
      glyphs->glyphs[0].geometry.y_offset = 0;
      glyphs->glyphs[0].geometry.width = 0;
      
      glyphs->log_clusters[0] = 0;
    }

  /* Set glyphs[i].attr.is_cluster_start based on log_clusters[]
   */
  for (i = 0; i < glyphs->num_glyphs; i++)
    {
      if (glyphs->log_clusters[i] != last_cluster)
	{
	  glyphs->glyphs[i].attr.is_cluster_start = TRUE;
	  last_cluster = glyphs->log_clusters[i];
	}
      else
	glyphs->glyphs[i].attr.is_cluster_start = FALSE;
    }
	    
  g_assert (glyphs->num_glyphs > 0);
}
