/* Pango
 * pango-engine.h: Engines for script and language specific processing
 *
 * Copyright (C) 2000,2003 Red Hat Software
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

#ifndef __PANGO_ENGINE_H__
#define __PANGO_ENGINE_H__

#include <pango/pango-types.h>
#include <pango/pango-item.h>
#include <pango/pango-font.h>
#include <pango/pango-glyph.h>
#include <pango/pango-script.h>

G_BEGIN_DECLS

#ifdef PANGO_ENABLE_ENGINE

/* Module API */

#define PANGO_RENDER_TYPE_NONE "PangoRenderNone"

#define PANGO_TYPE_ENGINE              (pango_engine_get_type ())
#define PANGO_ENGINE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), PANGO_TYPE_ENGINE, PangoEngine))
#define PANGO_IS_ENGINE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), PANGO_TYPE_ENGINE))
#define PANGO_ENGINE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), PANGO_TYPE_ENGINE, PangoEngineClass))
#define PANGO_IS_ENGINE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PANGO_TYPE_ENGINE))
#define PANGO_ENGINE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), PANGO_TYPE_ENGINE, PangoEngineClass))

typedef struct _PangoEngine PangoEngine;
typedef struct _PangoEngineClass PangoEngineClass;

/**
 * PangoEngine:
 *
 * #PangoEngine is the base class for all types of language and
 * script specific engines. It has no functionality by itself.
 **/
struct _PangoEngine
{
  /*< private >*/
  GObject parent_instance;
};

/**
 * PangoEngineClass:
 *
 * Class structure for #PangoEngine
 **/
struct _PangoEngineClass
{
  /*< private >*/
  GObjectClass parent_class;
};

GType pango_engine_get_type (void) G_GNUC_CONST;

#define PANGO_ENGINE_TYPE_LANG "PangoEngineLang"

#define PANGO_TYPE_ENGINE_LANG              (pango_engine_lang_get_type ())
#define PANGO_ENGINE_LANG(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), PANGO_TYPE_ENGINE_LANG, PangoEngineLang))
#define PANGO_IS_ENGINE_LANG(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), PANGO_TYPE_ENGINE_LANG))
#define PANGO_ENGINE_LANG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), PANGO_TYPE_ENGINE_LANG, PangoEngineLangClass))
#define PANGO_IS_ENGINE_LANG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PANGO_TYPE_ENGINE_LANG))
#define PANGO_ENGINE_LANG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), PANGO_TYPE_ENGINE_LANG, PangoEngineLangClass))

typedef struct _PangoEngineLangClass PangoEngineLangClass;

/**
 * PangoEngineLang:
 *
 * The #PangoEngineLang class is implemented by engines that
 * customize the rendering-system independent part of the
 * Pango pipeline for a particular script or language. For
 * instance, a custom #PangoEngineLang could be provided for
 * Thai to implement the dictionary-based word boundary
 * lookups needed for that language.
 **/
struct _PangoEngineLang
{
  /*< private >*/
  PangoEngine parent_instance;
};

/**
 * PangoEngineLangClass:
 * @script_break: Provides a custom implementation of pango_break().
 *  if this is %NULL, pango_default_break() will be used.
 *
 * Class structure for #PangoEngineLang
 **/
struct _PangoEngineLangClass
{
  /*< private >*/
  PangoEngineClass parent_class;
  
  /*< public >*/
  void (*script_break) (PangoEngineLang *engine,
			const char    *text,
			int            len,
			PangoAnalysis *analysis,
			PangoLogAttr  *attrs,
                        int            attrs_len);
};

GType pango_engine_lang_get_type (void) G_GNUC_CONST;

#define PANGO_ENGINE_TYPE_SHAPE "PangoEngineShape"

#define PANGO_TYPE_ENGINE_SHAPE              (pango_engine_shape_get_type ())
#define PANGO_ENGINE_SHAPE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), PANGO_TYPE_ENGINE_SHAPE, PangoEngineShape))
#define PANGO_IS_ENGINE_SHAPE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), PANGO_TYPE_ENGINE_SHAPE))
#define PANGO_ENGINE_SHAPE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), PANGO_TYPE_ENGINE_SHAPE, PangoEngine_ShapeClass))
#define PANGO_IS_ENGINE_SHAPE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PANGO_TYPE_ENGINE_SHAPE))
#define PANGO_ENGINE_SHAPE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), PANGO_TYPE_ENGINE_SHAPE, PangoEngineShapeClass))

typedef struct _PangoEngineShapeClass PangoEngineShapeClass;

/**
 * PangoEngineShape
 *
 * The #PangoEngineShape class is implemented by engines that
 * customize the rendering-system dependent part of the
 * Pango pipeline for a particular script or language.
 * A #PangoEngineShape implementation is then specific to both
 * a particular rendering system or group of rendering systems
 * and to a particular script. For instance, there is one
 * #PangoEngineShape implementation to handling shaping Arabic
 * for Fontconfig-based backends.
 **/
struct _PangoEngineShape
{
  PangoEngine parent_instance;
};

/**
 * PangoEngineShapeClass:
 * @script_shape: Given a font, a piece of text, and a #PangoAnalysis
 *   structure, converts characters to glyphs and positions the
 *   resulting glyphs. The results are stored in the #PangoGlyphString
 *   that is passed in. (The implementation should resize it
 *   appropriately using pango_glyph_string_set_size()). All fields
 *   of the @log_clusters and @glyphs array must be filled in, with
 *   the exception that Pango will automatically generate
 *   <literal>glyphs->glyphs[i].attr.is_cluster_start</literal>
 *   using the @log_clusters array. Each input character must occur in one
 *   of the output logical clusters;
 *   if no rendering is desired for a character, this may involve
 *   inserting glyphs with the #PangoGlyph ID 0, which is guaranteed never
 *   to render.
 * @covers: Returns the characters that this engine can cover
 *   with a given font for a given language. If not overridden, the default
 *   implementation simply returns the coverage information for the
 *   font itself unmodified.
 *
 * Class structure for #PangoEngineShape
 **/
struct _PangoEngineShapeClass
{
  /*< private >*/
  PangoEngineClass parent_class;
  
  /*< public >*/
  void (*script_shape) (PangoEngineShape *engine,
			PangoFont        *font,
			const char       *text,
			int               length,
			PangoAnalysis    *analysis,
			PangoGlyphString *glyphs);
  PangoCoverageLevel (*covers)   (PangoEngineShape *engine,
				  PangoFont        *font,
				  PangoLanguage    *language,
				  gunichar          wc);
};

GType pango_engine_shape_get_type (void) G_GNUC_CONST;

typedef struct _PangoEngineInfo PangoEngineInfo;
typedef struct _PangoEngineScriptInfo PangoEngineScriptInfo;

struct _PangoEngineScriptInfo 
{
  PangoScript script;
  gchar *langs;
};

struct _PangoEngineInfo
{
  gchar *id;
  gchar *engine_type;
  gchar *render_type;
  PangoEngineScriptInfo *scripts;
  gint n_scripts;
};

/**
 * script_engine_list:
 * @engines: location to store a pointer to an array of engines.
 * @n_engines: location to store the number of elements in @engines.
 * 
 * Function to be provided by a module to list the engines that the
 * module supplies. The function stores a pointer to an array
 * of #PangoEngineInfo structures and the length of that array in
 * the given location.
 *
 * Note that script_engine_init() will not be called before this
 * function.
 **/
void script_engine_list (PangoEngineInfo **engines,
			 int              *n_engines);

/**
 * script_engine_init:
 * @module: a #GTypeModule structure used to associate any
 *  GObject types created in this module with the module.
 * 
 * Function to be provided by a module to register any
 * GObject types in the module.
 **/
void script_engine_init (GTypeModule *module);


/**
 * script_engine_exit:
 * 
 * Function to be provided by the module that is called
 * when the module is unloading. Frequently does nothing.
 **/
void script_engine_exit (void);

/**
 * script_engine_create:
 * @id: the ID of an engine as reported by script_engine_list.
 * 
 * Function to be provided by the module to create an instance
 * of one of the engines implemented by the module.
 * 
 * Return value: a newly created #PangoEngine of the specified
 *  type, or %NULL if an error occurred. (In normal operation,
 *  a module should not return %NULL. A %NULL return is only
 *  acceptable in the case where system misconfiguration or
 *  bugs in the driver routine are encountered.)
 **/
PangoEngine *script_engine_create (const char *id);

/* Utility macro used by PANGO_ENGINE_LANG_DEFINE_TYPE and
 * PANGO_ENGINE_LANG_DEFINE_TYPE
 */
#define PANGO_ENGINE_DEFINE_TYPE(name, prefix, class_init, instance_init, parent_type) \
static GType prefix ## _type;						  \
static void								  \
prefix ## _register_type (GTypeModule *module)				  \
{									  \
  static const GTypeInfo object_info =					  \
    {									  \
      sizeof (name ## Class),						  \
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
  prefix ## _type =  g_type_module_register_type (module, parent_type,	  \
					          # name,		  \
						  &object_info, 0);	  \
}

/**
 * PANGO_ENGINE_LANG_DEFINE_TYPE:
 * @name: Name of the the type to register (for example:, <literal>ArabicEngineFc</literal>
 * @prefix: Prefix for symbols that will be defined (for example:, <literal>arabic_engine_fc</literal>
 * @class_init: Class initialization function for the new type, or %NULL
 * @instance_init: Instance initialization function for the new type, or %NULL
 *
 * Outputs the necessary code for GObject type registration for a
 * #PangoEngineLang class defined in a module. Two static symbols
 * are defined.
 *
 * <programlisting>
 *  static GType <replaceable>prefix</replaceable>_type;
 *  static void <replaceable>prefix</replaceable>_register_type (GTypeModule module);
 * </programlisting>
 *
 * The <function><replaceable>prefix</replaceable>_register_type()</function>
 * function should be called in your script_engine_init() function for
 * each type that your module implements, and then your script_engine_create()
 * function can create instances of the object as follows:
 *
 * <informalexample><programlisting>
 *  PangoEngine *engine = g_object_new (<replaceable>prefix</replaceable>_type, NULL);
 * </programlisting></informalexample>
 **/
#define PANGO_ENGINE_LANG_DEFINE_TYPE(name, prefix, class_init, instance_init)	\
  PANGO_ENGINE_DEFINE_TYPE (name, prefix,				\
			    class_init, instance_init,			\
                            PANGO_TYPE_ENGINE_LANG)

/**
 * PANGO_ENGINE_SHAPE_DEFINE_TYPE:
 * @name: Name of the the type to register (for example:, <literal>ArabicEngineFc</literal>
 * @prefix: Prefix for symbols that will be defined (for example:, <literal>arabic_engine_fc</literal>
 * @class_init: Class initialization function for the new type, or %NULL
 * @instance_init: Instance initialization function for the new type, or %NULL
 *
 * Outputs the necessary code for GObject type registration for a
 * #PangoEngineShape class defined in a module. Two static symbols
 * are defined.
 *
 * <programlisting>
 *  static GType <replaceable>prefix</replaceable>_type;
 *  static void <replaceable>prefix</replaceable>_register_type (GTypeModule module);
 * </programlisting>
 *
 * The <function><replaceable>prefix</replaceable>_register_type()</function>
 * function should be called in your script_engine_init() function for
 * each type that your module implements, and then your script_engine_create()
 * function can create instances of the object as follows:
 *
 * <informalexample><programlisting>
 *  PangoEngine *engine = g_object_new (<replaceable>prefix</replaceable>_type, NULL);
 * </programlisting></informalexample>
 **/
#define PANGO_ENGINE_SHAPE_DEFINE_TYPE(name, prefix, class_init, instance_init)	\
  PANGO_ENGINE_DEFINE_TYPE (name, prefix,				\
			    class_init, instance_init,			\
                            PANGO_TYPE_ENGINE_SHAPE)

/* Macro used for possibly builtin Pango modules. Not useful
 * for externally build modules. If we are compiling a module standaline,
 * then we name the entry points script_engine_list, etc. But if we
 * are compiling it for inclusion directly in Pango, then we need them to
 * to have distinct names for this module, so we prepend a prefix.
 *
 * The two intermediate macros are to deal with details of the C
 * preprocessor; token pasting tokens must be function arguments,
 * and macro substitution isn't used on function arguments that
 * are used for token pasting.
 */
#ifdef PANGO_MODULE_PREFIX
#if defined(VMS) && !defined(__STDC__)
#define PANGO_MODULE_ENTRY(func) _pan_b_ft2_script_engine_/**/func
#else
#define PANGO_MODULE_ENTRY(func) _PANGO_MODULE_ENTRY2(PANGO_MODULE_PREFIX,func)
#define _PANGO_MODULE_ENTRY2(prefix,func) _PANGO_MODULE_ENTRY3(prefix,func)
#define _PANGO_MODULE_ENTRY3(prefix,func) prefix##_script_engine_##func
#endif
#else
#if defined(VMS) && !defined(__STDC__)
#define PANGO_MODULE_ENTRY(func) script_engine_/**/func
#else
#define PANGO_MODULE_ENTRY(func) script_engine_##func
#endif
#endif

#endif /* PANGO_ENABLE_ENGINE */

G_END_DECLS

#endif /* __PANGO_ENGINE_H__ */
