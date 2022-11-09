! MMS description file for libfreetype
! George E. Cook, WVNET, 19-Aug-2008
! Mosaic 4.3
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!
#
# FreeType 2 build system -- top-level Makefile for OpenVMS
#


# Copyright 2001 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libfreetype.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.IFDEF DECC
CQUALC = /DECC/DEFINE=(FT2_BUILD_LIBRARY)/INCLUDE=([])
.ELSE
.IFDEF DECCVAXC
CQUALC = /VAXC/DEFINE=(FT2_BUILD_LIBRARY)/INCLUDE=([])
.ELSE
.IFDEF GNUC
CQUALC = /INCLUDE=([],GCC_Include)/DEFINE=(FT2_BUILD_LIBRARY)
.ELSE
CQUALC = /DEFINE=(FT2_BUILD_LIBRARY)/INCLUDE=([])
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

OBJECTS = Odir:ftsystem.obj Odir:autofit.obj Odir:ftbase.obj Odir:ftinit.obj \
	  Odir:ftdebug.obj Odir:ftbdf.obj Odir:ftmm.obj Odir:fttype1.obj \
	  Odir:ftxf86.obj Odir:ftpfr.obj Odir:ftstroke.obj Odir:ftwinfnt.obj \
	  Odir:ftbbox.obj Odir:ftbitmap.obj Odir:ftsynth.obj Odir:ftlcdfil.obj \
	  Odir:bdf.obj Odir:ftcache.obj Odir:cff.obj Odir:type1cid.obj \
	  Odir:ftgzip.obj Odir:ftlzw.obj Odir:otvalid.obj Odir:pcf.obj \
	  Odir:pfr.obj Odir:psaux.obj Odir:pshinter.obj Odir:psnames.obj \
	  Odir:raster.obj Odir:sfnt.obj Odir:smooth.obj Odir:truetype.obj \
	  Odir:type1.obj Odir:type42.obj Odir:winfnt.obj Odir:ftglyph.obj

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
        @ Define/NoLog Odir $(WDIR)
	@ Topdir = F$Environment("Default")-"LIBFREETYPE]"
        @ Define/NoLog FREETYPE 'Topdir'LIBFREETYPE]
        @ Define/NoLog ZLIB 'Topdir'ZLIB]
.IFDEF GNUC
        @ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
        @ Write SYS$Output "Library libfreetype.olb built."
 
Odir:ftsystem.obj : ftsystem.c ftconfig.h freetype.h
Odir:autofit.obj : autofit.c afmodule.c ftconfig.h freetype.h
Odir:ftbase.obj : ftbase.c ftrfork.c ftconfig.h ftoption.h freetype.h
Odir:ftinit.obj : ftinit.c ftmodule.h ftconfig.h freetype.h
Odir:ftglyph.obj : ftglyph.c ftconfig.h freetype.h
Odir:ftdebug.obj : ftdebug.c ftconfig.h freetype.h
Odir:ftbdf.obj : ftbdf.c ftconfig.h freetype.h
Odir:ftmm.obj : ftmm.c ftconfig.h freetype.h
Odir:fttype1.obj : fttype1.c ftconfig.h freetype.h
Odir:ftxf86.obj : ftxf86.c ftconfig.h freetype.h
Odir:ftpfr.obj : ftpfr.c ftconfig.h freetype.h
Odir:ftstroke.obj : ftstroke.c ftconfig.h freetype.h
Odir:ftwinfnt.obj : ftwinfnt.c ftconfig.h freetype.h
Odir:ftbbox.obj : ftbbox.c ftconfig.h freetype.h
Odir:ftbitmap.obj : ftbitmap.c ftconfig.h freetype.h
Odir:ftsynth.obj : ftsynth.c ftconfig.h freetype.h
Odir:ftlcdfil.obj : ftlcdfil.c ftconfig.h freetype.h
Odir:bdf.obj : bdf.c bdflib.c bdfdrivr.c ftconfig.h freetype.h
Odir:ftcache.obj : ftcache.c ftcbasic.c ftccache.c ftccmap.c ftcglyph.c \
	ftcimage.c ftcmanag.obj ftcmru.c ftcsbits.c ftconfig.h freetype.h
Odir:cff.obj : cff.c cffdrivr.c cfftoken.h ftconfig.h
Odir:type1cid.obj : type1cid.c cidriver.c ftconfig.h
Odir:ftgzip.obj : ftgzip.c ftconfig.h
Odir:ftlzw.obj : ftlzw.c ftconfig.h
Odir:otvalid.obj : otvalid.c ftconfig.h
Odir:pcf.obj : pcf.c pcfdrivr.c ftconfig.h
Odir:pfr.obj : pfr.c pfrdrivr.c ftconfig.h
Odir:psaux.obj : psaux.c psauxmod.c ftconfig.h
Odir:pshinter.obj : pshinter.c pshmod.c ftconfig.h
Odir:psnames.obj : psnames.c psmodule.c ftconfig.h
Odir:raster.obj : raster.c ftrend1.c ftconfig.h
Odir:sfnt.obj : sfnt.c sfdriver.c ftconfig.h
Odir:smooth.obj : smooth.c ftsmooth.c ftconfig.h
Odir:truetype.obj : truetype.c ttdriver.c ftconfig.h
Odir:type1.obj : type1.c t1parse.c t1load.c t1objs.c t1driver.c t1gload.c \
	t1afm.c ftconfig.h
Odir:type42.obj : type42.c t42drivr.c ftconfig.h
Odir:winfnt.obj : winfnt.c ftconfig.h

.c.obj :
        $(CC)$(CFLAGS)$(CFLOAT)/OBJECT=$@ $<

.obj.olb
        $(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
        Delete/Log $(WDIR)*.OBJ;*
        Delete/Log $(LIBTARGET);*
