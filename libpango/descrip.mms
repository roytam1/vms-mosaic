! MMS description file for libpango
! George E. Cook, WVNET, 22-Aug-2008
! Mosaic 4.3
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!
#*****************************************************************************
#                                                                            *
# Make file for VMS                                                          *
# Author : J.Jansen (joukj@hrem.nano.tudelft.nl)                             *
# Date : 23 April 2006                                                       *
#                                                                            *
#*****************************************************************************

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libpango.olb

.IFDEF GNUC
CC = GCC
CINCLUDE =
.ELSE
CC = CC
CINCLUDE = /INCLUDE=([],[-.GLIB],[-.LIBFREETYPE])
.ENDIF

CDEFINES = /DEFINE=("PANGO_ENABLE_BACKEND=1","PANGO_ENABLE_ENGINE=1",\
"PANGO_MODULE_PREFIX="_pan_b_ft2"")

.IFDEF DECC
CQUALC = /DECC/WARN=(DISABLE=PTRMISMATCH1)
.ELSE
.IFDEF DECCVAXC
CQUALC = /VAXC
.ELSE
.IFDEF GNUC
CQUALC = /INCLUDE=(GCC_Include)
.ELSE
CQUALC =
.ENDIF
.ENDIF
.ENDIF

.IFDEF ALPHA
CFLOAT = /FLOAT=IEEE/IEEE=DENORM
.ELSE
.IFDEF VAX
CFLOAT = /G_FLOAT
.ELSE
CFLOAT =
.ENDIF
.ENDIF

.IFDEF DEBUG
CFLAGS = $(CQUALC)/NoOpt/Debug
.ELSE
CFLAGS = $(CQUALC)
.ENDIF
 
OBJECTS = Odir:break.obj Odir:fonts.obj Odir:glyphstring.obj Odir:mapping.obj \
	  Odir:modules.obj Odir:pango-attributes.obj Odir:pango-color.obj \
	  Odir:pango-context.obj Odir:pango-coverage.obj \
	  Odir:pango-fontmap.obj Odir:pango-fontset.obj \
	  Odir:pango-glyph-item.obj Odir:pango-item.obj Odir:pango-layout.obj \
	  Odir:pango-markup.obj Odir:pango-tabs.obj Odir:pango-utils.obj \
	  Odir:reorder-items.obj Odir:shape.obj Odir:pango-enum-types.obj \
	  Odir:pango-engine.obj Odir:pango-script.obj Odir:ellipsize.obj \
	  Odir:pango-renderer.obj Odir:pangofc-font.obj \
	  Odir:pangofc-fontmap.obj Odir:module-defs-fc.obj \
	  Odir:pangofc-decoder.obj Odir:pangoft2.obj Odir:pangoft2-fontmap.obj \
	  Odir:module-defs-ft2.obj Odir:pangoft2-render.obj Odir:basic-fc.obj\
	  Odir:fribidi_char_type.obj Odir:fribidi.obj Odir:fribidi_types.obj

.FIRST
	@ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
	@ Topdir = F$Environment("Default")-"LIBPANGO]"
	@ Define/Nolog PANGO 'Topdir'LIBPANGO]
        @ Define/NoLog FREETYPE 'Topdir'LIBFREETYPE]
	@ Define/Nolog FONTCONFIG 'Topdir'LIBFONTCONFIG]
	@ Define/Nolog GLIB 'Topdir'GLIB]
	@ Define/Nolog GOBJECT 'Topdir'GLIB.GOBJECT]
.IFDEF GNUC
        @ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
	@ Write SYS$Output "Library libpango.olb built.

Odir:break.obj : break.c pango-break.h pango-item.h pango-types.h \
	pango-modules.h pango-engine.h pango-font.h pango-coverage.h \
	pango-glyph.h
Odir:fonts.obj : fonts.c pango-types.h pango-font.h pango-coverage.h \
	pango-types.h pango-fontmap.h pango-font.h pango-fontset.h \
	pango-utils.h
Odir:glyphstring.obj : glyphstring.c pango-glyph.h pango-types.h \
	pango-item.h pango-font.h pango-coverage.h
Odir:mapping.obj : mapping.c pango-glyph.h pango-types.h pango-item.h
Odir:module-defs-x.obj : module-defs-x.c module-defs.h \
	modules.h pango-engine.h pango-types.h pango-item.h \
	pango-font.h pango-coverage.h pango-glyph.h pango-modules.h
Odir:module-defs-fc.obj : module-defs-fc.c module-defs.h \
	modules.h pango-engine.h pango-types.h pango-item.h \
	pango-font.h pango-coverage.h pango-glyph.h pango-modules.h
Odir:modules.obj : modules.c config.h pango-modules.h \
	pango-engine.h pango-types.h pango-item.h pango-font.h \
	pango-coverage.h pango-glyph.h pango-utils.h pango-enum-types.h
Odir:pango-attributes.obj : pango-attributes.c pango-attributes.h \
	pango-font.h pango-coverage.h pango-types.h pango-utils.h
Odir:pango-color.obj : pango-color.c pango-attributes.h \
	pango-font.h pango-coverage.h pango-types.h
Odir:pango-context.obj : pango-context.c pango-context.h \
	pango-font.h pango-coverage.h pango-types.h pango-fontmap.h \
	pango-fontset.h pango-attributes.h pango-utils.h \
	pango-modules.h pango-engine.h pango-item.h pango-glyph.h
Odir:pango-coverage.obj : pango-coverage.c pango-coverage.h
Odir:pango-enum-types.obj : pango-enum-types.c pango.h \
	pango-attributes.h pango-font.h pango-coverage.h pango-types.h \
	pango-break.h pango-item.h pango-context.h pango-fontmap.h \
	pango-fontset.h pango-engine.h pango-glyph.h pango-enum-types.h \
	pango-layout.h pango-glyph-item.h pango-tabs.h
Odir:pango-fontmap.obj : pango-fontmap.c pango-fontmap.h \
	pango-font.h pango-coverage.h pango-types.h pango-fontset.h \
	pango-utils.h
Odir:pango-fontset.obj : pango-fontset.c pango-types.h \
	pango-font.h pango-coverage.h pango-types.h pango-fontset.h \
	pango-utils.h pango-font.h
Odir:pango-glyph-item.obj : pango-glyph-item.c \
	pango-glyph-item.h pango-attributes.h pango-font.h \
	pango-coverage.h pango-types.h pango-item.h pango-glyph.h
Odir:pango-item.obj : pango-item.c pango-attributes.h \
	pango-font.h pango-coverage.h pango-types.h pango-item.h
Odir:pango-layout.obj : pango-layout.c pango-glyph.h \
	pango-types.h pango-item.h pango-layout.h pango-attributes.h \
	pango-font.h pango-coverage.h pango-context.h pango-fontmap.h \
	pango-fontset.h pango-glyph-item.h pango-tabs.h pango-break.h \
	pango-engine.h
Odir:pango-markup.obj : pango-markup.c pango-attributes.h \
	pango-font.h pango-coverage.h pango-types.h pango-utils.h
Odir:pango-tabs.obj : pango-tabs.c pango-tabs.h pango-types.h
Odir:pango-utils.obj pango-utils.o : pango-utils.c config.h pango-font.h \
	pango-coverage.h pango-types.h pango-utils.h pango-font.h
Odir:pangofc-fontmap.obj : pangofc-fontmap.c
Odir:pangoft2-fontmap.obj : pangoft2-fontmap.c config.h \
	pango-fontmap.h pango-font.h pango-coverage.h pango-types.h \
	pango-fontset.h pango-utils.h pangoft2-private.h \
	pango-modules.h pango-engine.h pango-item.h pango-glyph.h \
	pangoft2.h pango-layout.h pango-attributes.h pango-context.h \
	pango-fontmap.h pango-glyph-item.h pango-tabs.h modules.h \
	pango-modules.h
Odir:pangoft2.obj : pangoft2.c config.h pango-utils.h \
	pango-font.h pango-coverage.h pango-types.h pangoft2.h \
	pango-layout.h pango-attributes.h pango-context.h \
	pango-fontmap.h pango-fontset.h pango-glyph-item.h pango-item.h \
	pango-glyph.h pango-tabs.h pangoft2-private.h pango-modules.h \
	pango-engine.h
Odir:pangox-fontcache.obj : pangox-fontcache.c pangox.h \
	pango-layout.h pango-attributes.h pango-font.h pango-coverage.h \
	pango-types.h pango-context.h pango-fontmap.h pango-fontset.h \
	pango-glyph-item.h pango-item.h pango-glyph.h pango-tabs.h
Odir:pangox-fontmap.obj : pangox-fontmap.c pango-fontmap.h \
	pango-font.h pango-coverage.h pango-types.h pango-fontset.h \
	pango-utils.h pangox-private.h pango-modules.h pango-engine.h \
	pango-item.h pango-glyph.h pangox.h pango-layout.h \
	pango-attributes.h pango-context.h pango-fontmap.h \
	pango-glyph-item.h pango-tabs.h
Odir:pangox.obj : pangox.c pangox.h pango-layout.h pango-attributes.h \
	pango-font.h pango-coverage.h pango-types.h pango-context.h \
	pango-fontmap.h pango-fontset.h pango-glyph-item.h pango-item.h \
	pango-glyph.h pango-tabs.h pango-utils.h pangox-private.h \
	pango-modules.h pango-engine.h modules.h \
	pango-modules.h config.h
Odir:pangofc-font.obj : pangofc-font.c
Odir:querymodules.obj : querymodules.c config.h pango-break.h pango-item.h \
	pango-types.h pango-context.h pango-font.h pango-coverage.h \
	pango-fontmap.h pango-fontset.h pango-attributes.h \
	pango-utils.h pango-engine.h pango-glyph.h
Odir:reorder-items.obj : reorder-items.c pango-glyph.h \
	pango-types.h pango-item.h
Odir:shape.obj : shape.c pango-glyph.h pango-types.h pango-item.h \
	pango-engine.h pango-font.h pango-coverage.h
Odir:pango-engine.obj : pango-engine.c
Odir:pango-script.obj : pango-script.c
Odir:pango-renderer.obj : pango-renderer.c pango-renderer.h
Odir:pangoft2-render.obj : pangoft2-render.c
Odir:ellipsize.obj : ellipsize.c
Odir:pangofc-decoder.obj : pangofc-decoder.c
Odir:pango-ot-buffer.obj : pango-ot-buffer.c
Odir:pango-ot-info.obj : pango-ot-info.c
Odir:pango-ot-ruleset.obj : pango-ot-ruleset.c
Odir:module-defs-ft2.obj : module-defs-ft2.c
Odir:basic-fc.obj : basic-fc.c  pango-context.h pango-font.h \
        pango-coverage.h pango-types.h pango-fontmap.h pango-fontset.h \
        pango-attributes.h pango-ot.h pango-glyph.h pango-item.h \
        pango-engine.h pango-utils.h basic-common.h
Odir:fribidi.obj : fribidi.c pango-utils.h pango-font.h pango-coverage.h \
        pango-types.h fribidi_types.h
Odir:fribidi_char_type.obj : fribidi_char_type.c pango-utils.h pango-font.h \
        pango-coverage.h pango-types.h fribidi_types.h fribidi_tab_char_type_2.i
Odir:fribidi_types.obj : fribidi_types.c pango-utils.h pango-font.h \
        pango-coverage.h pango-types.h fribidi_types.h

.c.obj :
        $(CC)$(CFLAGS)$(CFLOAT)$(CINCLUDE)$(CDEFINES)/OBJECT=$@ $<

.obj.olb
        $(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
        Delete/Log $(WDIR)*.OBJ;*
        Delete/Log $(LIBTARGET);*
