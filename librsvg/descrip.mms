! MMS description file for librsvg
! George E. Cook, WVNET, 4-Aug-2008
! Mosaic 4.3
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!

# Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)librsvg.olb

.IFDEF GNUC
CC = GCC
CINCLUDE =
.ELSE
CC = CC
CINCLUDE = /INCLUDE=([-.LIBXML2],[-.GLIB],[-.LIBFREETYPE],[-.LIBGDK-PIXBUF])
.ENDIF

.IFDEF DECC
CQUALC = /DECC/WARNING=(DISABLE=(PTRMISMATCH1,PTRMISMATCH))
.ELSE
.IFDEF DECCVAXC
CQUALC = /VAXC
.ELSE
.IFDEF GNUC
#CQUALC = /INCLUDE=(GCC_Include)
CQUALC = 
.ELSE
CQUALC = 
.ENDIF
.ENDIF
.ENDIF

.IFDEF ALPHA
CFLOAT = /FLOAT=IEEE
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

OBJECTS = Odir:librsvg-features.obj Odir:rsvg-affine.obj \
     Odir:rsvg-art-composite.obj Odir:rsvg-art-draw.obj \
     Odir:rsvg-art-mask.obj Odir:rsvg-art-paint-server.obj \
     Odir:rsvg-art-render.obj Odir:rsvg-bpath-util.obj Odir:rsvg-cond.obj \
     Odir:rsvg-css.obj Odir:rsvg-defs.obj Odir:rsvg-file-util.obj \
     Odir:rsvg-filter.obj Odir:rsvg-image.obj Odir:rsvg-marker.obj \
     Odir:rsvg-mask.obj Odir:rsvg-paint-server.obj Odir:rsvg-path.obj \
     Odir:rsvg-shapes.obj Odir:rsvg-structure.obj Odir:rsvg-styles.obj \
     Odir:rsvg-text.obj Odir:rsvg.obj

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
        @ Define/NoLog Odir $(WDIR)
	@ Topdir = F$Environment("Default")-"LIBRSVG]"
	@ Define/NoLog freetype 'Topdir'libfreetype]
	@ Define/NoLog fontconfig 'Topdir'libfontconfig]
	@ Define/NoLog glib 'Topdir'glib]
	@ Define/NoLog gobject 'Topdir'glib.gobject]
	@ Define/NoLog libart_lgpl 'Topdir'libart]
	@ Define/NoLog libxml 'Topdir'libxml2]
	@ Define/NoLog libcroco 'Topdir'libcroco]
	@ Define/NoLog pango 'Topdir'libpango]
.IFDEF GNUC
        @ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
	@ Write SYS$Output "Library librsvg.olb built."

Odir:librsvg-features.obj : librsvg-features.c config.h librsvg-features.h
Odir:rsvg-affine.obj : rsvg-affine.c config.h rsvg-private.h
Odir:rsvg-art-composite.obj : rsvg-art-composite.c config.h \
	rsvg-art-composite.h rsvg-private.h
Odir:rsvg-art-draw.obj : rsvg-art-draw.c config.h rsvg-art-draw.h rsvg-private.h
Odir:rsvg-art-mask.obj : rsvg-art-mask.c config.h rsvg-art-mask.h rsvg-private.h
Odir:rsvg-art-paint-server.obj : rsvg-art-paint-server.c config.h \
	rsvg-art-paint-server.h rsvg-private.h
Odir:rsvg-art-render.obj : rsvg-art-render.c config.h rsvg-art-render.h \
	rsvg-private.h
Odir:rsvg-bpath-util.obj : rsvg-bpath-util.c config.h rsvg-bpath-util.h \
	rsvg-private.h
Odir:rsvg-cond.obj : rsvg-cond.c config.h rsvg-private.h
Odir:rsvg-css.obj : rsvg-css.c config.h rsvg-css.h rsvg-private.h
Odir:rsvg-defs.obj : rsvg-defs.c config.h rsvg-defs.h rsvg-private.h
Odir:rsvg-file-util.obj : rsvg-file-util.c config.h rsvg-private.h
Odir:rsvg-filter.obj : rsvg-filter.c config.h rsvg-filter.h rsvg-private.h
Odir:rsvg-image.obj : rsvg-image.c config.h rsvg-image.h rsvg-private.h
Odir:rsvg-marker.obj : rsvg-marker.c config.h rsvg-marker.h rsvg-private.h
Odir:rsvg-mask.obj : rsvg-mask.c config.h rsvg-mask.h rsvg-private.h
Odir:rsvg-paint-server.obj : rsvg-paint-server.c config.h rsvg-paint-server.h \
	rsvg-private.h
Odir:rsvg-path.obj : rsvg-path.c config.h rsvg-path.h rsvg-private.h
Odir:rsvg-shapes.obj : rsvg-shapes.c config.h rsvg-shapes.h rsvg-private.h
Odir:rsvg-structure.obj : rsvg-structure.c config.h rsvg-structure.h \
	rsvg-private.h
Odir:rsvg-styles.obj : rsvg-styles.c config.h rsvg-styles.h rsvg-private.h
Odir:rsvg-text.obj : rsvg-text.c config.h rsvg-text.h rsvg-private.h
Odir:rsvg.obj : rsvg.c config.h rsvg.h rsvg-private.h [-.libxml2]xmlmemory.h

.c.obj :
        $(CC)$(CFLAGS)$(CFLOAT)$(CINCLUDE)/OBJECT=$@ $<

.obj.olb
        $(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
        Delete/Log $(WDIR)*.OBJ;*
        Delete/Log $(LIBTARGET);*
