! MMS description file for libart
! George E. Cook, WVNET, 29-Jan-2008
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

LIBTARGET = $(WDIR)libart.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.IFDEF DECC
CQUALC = /DECC
.ELSE
.IFDEF DECCVAXC
CQUALC = /VAXC
.ELSE
CQUALC =
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

OBJECTS = Odir:art_affine.obj Odir:art_alphagamma.obj Odir:art_bpath.obj \
     Odir:art_gray_svp.obj Odir:art_misc.obj Odir:art_pixbuf.obj \
     Odir:art_rect.obj Odir:art_rect_svp.obj Odir:art_rect_uta.obj \
     Odir:art_render.obj Odir:art_render_gradient.obj Odir:art_render_mask.obj \
     Odir:art_render_svp.obj Odir:art_rgb.obj Odir:art_rgb_affine.obj \
     Odir:art_rgb_affine_private.obj Odir:art_rgb_bitmap_affine.obj \
     Odir:art_rgb_pixbuf_affine.obj Odir:art_rgb_rgba_affine.obj \
     Odir:art_rgb_a_affine.obj Odir:art_rgba.obj Odir:art_rgb_svp.obj \
     Odir:art_svp.obj Odir:art_svp_intersect.obj Odir:art_svp_ops.obj \
     Odir:art_svp_point.obj Odir:art_svp_render_aa.obj Odir:art_svp_vpath.obj \
     Odir:art_svp_vpath_stroke.obj Odir:art_svp_wind.obj Odir:art_uta.obj \
     Odir:art_uta_ops.obj Odir:art_uta_rect.obj Odir:art_uta_vpath.obj \
     Odir:art_uta_svp.obj Odir:art_vpath.obj Odir:art_vpath_bpath.obj \
     Odir:art_vpath_dash.obj Odir:art_vpath_svp.obj 

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
        @ Define/NoLog Odir $(WDIR)
.IFDEF GNUC
        @ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
	@ Write SYS$Output "Library libart.olb built."

Odir:art_affine.obj : art_affine.c config.h art_affine.h \
	art_point.h art_misc.h art_config.h
Odir:art_alphagamma.obj : art_alphagamma.c config.h \
	art_alphagamma.h art_misc.h art_config.h
Odir:art_bpath.obj : art_bpath.c config.h art_bpath.h art_misc.h \
	art_config.h art_point.h art_pathcode.h
Odir:art_gray_svp.obj : art_gray_svp.c config.h art_gray_svp.h \
	art_misc.h art_config.h art_svp.h art_rect.h art_point.h \
	art_svp_render_aa.h
Odir:art_misc.obj : art_misc.c config.h art_misc.h art_config.h
Odir:art_pixbuf.obj : art_pixbuf.c config.h art_pixbuf.h \
	art_misc.h art_config.h
Odir:art_rect.obj : art_rect.c config.h art_rect.h
Odir:art_rect_svp.obj : art_rect_svp.c config.h art_rect_svp.h \
	art_svp.h art_rect.h art_point.h art_misc.h art_config.h
Odir:art_rect_uta.obj : art_rect_uta.c config.h art_rect_uta.h \
	art_rect.h art_uta.h art_misc.h art_config.h
Odir:art_render.obj : art_render.c config.h art_render.h \
	art_alphagamma.h art_misc.h art_config.h art_rgb.h
Odir:art_render_gradient.obj : art_render_gradient.c \
	config.h art_render_gradient.h art_filterlevel.h art_render.h \
	art_alphagamma.h art_misc.h art_config.h
Odir:art_render_mask.obj : art_render_mask.c config.h \
	art_render_mask.h art_render.h art_alphagamma.h art_misc.h \
	art_config.h
Odir:art_render_svp.obj : art_render_svp.c art_render_svp.h \
	art_render.h art_alphagamma.h art_misc.h art_config.h art_svp.h \
	art_rect.h art_point.h art_svp_render_aa.h
Odir:art_rgb.obj : art_rgb.c config.h art_rgb.h art_misc.h \
	art_config.h
Odir:art_rgb_a_affine.obj : art_rgb_a_affine.c config.h \
	art_rgb_a_affine.h art_filterlevel.h art_alphagamma.h \
	art_misc.h art_config.h art_affine.h art_point.h \
	art_rgb_affine_private.h
Odir:art_rgb_affine.obj : art_rgb_affine.c config.h \
	art_rgb_affine.h art_filterlevel.h art_alphagamma.h art_misc.h \
	art_config.h art_point.h art_affine.h art_rgb_affine_private.h
Odir:art_rgb_affine_private.obj : \
	art_rgb_affine_private.c config.h art_rgb_affine_private.h \
	art_misc.h art_config.h art_point.h art_affine.h
Odir:art_rgb_bitmap_affine.obj : \
	art_rgb_bitmap_affine.c config.h art_rgb_bitmap_affine.h \
	art_filterlevel.h art_alphagamma.h art_misc.h art_config.h \
	art_point.h art_affine.h art_rgb_affine_private.h
Odir:art_rgb_pixbuf_affine.obj : \
	art_rgb_pixbuf_affine.c config.h art_rgb_pixbuf_affine.h \
	art_filterlevel.h art_alphagamma.h art_misc.h art_config.h \
	art_pixbuf.h art_point.h art_affine.h art_rgb_affine.h \
	art_rgb_rgba_affine.h
Odir:art_rgb_rgba_affine.obj : art_rgb_rgba_affine.c \
	config.h art_rgb_rgba_affine.h art_filterlevel.h \
	art_alphagamma.h art_misc.h art_config.h art_point.h \
	art_affine.h art_rgb_affine_private.h
Odir:art_rgb_svp.obj : art_rgb_svp.c config.h art_rgb_svp.h \
	art_alphagamma.h art_misc.h art_config.h art_svp.h art_rect.h \
	art_point.h art_svp_render_aa.h art_rgb.h
Odir:art_rgba.obj : art_rgba.c config.h art_rgba.h art_misc.h \
	art_config.h
Odir:art_svp.obj : art_svp.c config.h art_svp.h art_rect.h \
	art_point.h art_misc.h art_config.h
Odir:art_svp_intersect.obj : art_svp_intersect.c config.h \
	art_svp_intersect.h art_svp.h art_rect.h art_point.h art_misc.h \
	art_config.h
Odir:art_svp_ops.obj : art_svp_ops.c config.h art_svp_ops.h \
	art_svp.h art_rect.h art_point.h art_misc.h art_config.h \
	art_vpath.h art_pathcode.h art_svp_vpath.h art_svp_intersect.h \
	art_vpath_svp.h
Odir:art_svp_point.obj : art_svp_point.c config.h \
	art_svp_point.h art_svp.h art_rect.h art_point.h art_misc.h \
	art_config.h
Odir:art_svp_render_aa.obj : art_svp_render_aa.c config.h \
	art_svp_render_aa.h art_svp.h art_rect.h art_point.h art_misc.h \
	art_config.h
Odir:art_svp_vpath.obj : art_svp_vpath.c config.h \
	art_svp_vpath.h art_svp.h art_rect.h art_point.h art_vpath.h \
	art_pathcode.h art_misc.h art_config.h
Odir:art_svp_vpath_stroke.obj : art_svp_vpath_stroke.c \
	config.h art_svp_vpath_stroke.h art_svp.h art_rect.h \
	art_point.h art_vpath.h art_pathcode.h art_misc.h art_config.h \
	art_svp_intersect.h art_svp_vpath.h
Odir:art_svp_wind.obj : art_svp_wind.c config.h art_svp_wind.h \
	art_svp.h art_rect.h art_point.h art_misc.h art_config.h
Odir:art_uta.obj : art_uta.c config.h art_uta.h art_misc.h \
	art_config.h
Odir:art_uta_ops.obj : art_uta_ops.c config.h art_uta_ops.h \
	art_uta.h art_misc.h art_config.h
Odir:art_uta_rect.obj : art_uta_rect.c config.h art_uta_rect.h \
	art_rect.h art_uta.h art_misc.h art_config.h
Odir:art_uta_svp.obj : art_uta_svp.c config.h art_uta_svp.h \
	art_svp.h art_rect.h art_point.h art_uta.h art_misc.h \
	art_config.h art_vpath.h art_pathcode.h art_uta_vpath.h \
	art_vpath_svp.h
Odir:art_uta_vpath.obj : art_uta_vpath.c config.h \
	art_uta_vpath.h art_uta.h art_misc.h art_config.h art_vpath.h \
	art_rect.h art_pathcode.h
Odir:art_vpath.obj : art_vpath.c config.h art_vpath.h art_rect.h \
	art_pathcode.h art_misc.h art_config.h
Odir:art_vpath_bpath.obj : art_vpath_bpath.c config.h \
	art_vpath_bpath.h art_bpath.h art_misc.h art_config.h \
	art_point.h art_pathcode.h art_vpath.h art_rect.h
Odir:art_vpath_dash.obj : art_vpath_dash.c config.h \
	art_vpath_dash.h art_vpath.h art_rect.h art_pathcode.h \
	art_misc.h art_config.h
Odir:art_vpath_svp.obj : art_vpath_svp.c config.h \
	art_vpath_svp.h art_svp.h art_rect.h art_point.h art_vpath.h \
	art_pathcode.h art_misc.h art_config.h
Odir:testart.obj : testart.c art_misc.h art_config.h art_vpath.h art_rect.h \
	art_pathcode.h art_svp.h art_point.h art_svp_vpath.h \
	art_gray_svp.h art_rgb_svp.h art_alphagamma.h \
	art_svp_vpath_stroke.h art_svp_ops.h art_affine.h \
	art_rgb_affine.h art_filterlevel.h art_rgb_bitmap_affine.h \
	art_rgb_rgba_affine.h art_svp_point.h art_vpath_dash.h \
	art_render.h art_render_gradient.h art_render_svp.h \
	art_svp_intersect.h
Odir:testuta.obj : testuta.c art_misc.h art_config.h art_uta.h art_vpath.h \
	art_rect.h art_pathcode.h art_uta_vpath.h art_rect_uta.h \
	art_uta_rect.h libart-features.h

.c.obj :
        $(CC)$(CFLAGS)$(CFLOAT)/OBJECT=$@ $<

.obj.olb
        $(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
        Delete/Log $(WDIR)*.OBJ;*
        Delete/Log $(LIBTARGET);*
