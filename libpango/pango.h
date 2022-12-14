/* Pango
 * pango.h:
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

#ifndef __PANGO_H__
#define __PANGO_H__

#ifndef __GNUC__
#include <pango/pango-attributes.h>
#include <pango/pango-break.h>
#include <pango/pango-context.h>
#include <pango/pango-coverage.h>
#include <pango/pango-engine.h>
#include <pango/pango-enum-types.h>
#include <pango/pango-font.h>
#include <pango/pango-fontmap.h>
#include <pango/pango-glyph.h>
#include <pango/pango-item.h>
#include <pango/pango-layout.h>
#include <pango/pango-renderer.h>
#include <pango/pango-script.h>
#include <pango/pango-types.h>
#else
#include <pango-attributes.h>
#include <pango-break.h>
#include <pango-context.h>
#include <pango-coverage.h>
#include <pango-engine.h>
#include <pango-enum-types.h>
#include <pango-font.h>
#include <pango-fontmap.h>
#include <pango-glyph.h>
#include <pango-item.h>
#include <pango-layout.h>
#include <pango-renderer.h>
#include <pango-script.h>
#include <pango-types.h>
#endif

#endif /* __PANGO_H__ */
