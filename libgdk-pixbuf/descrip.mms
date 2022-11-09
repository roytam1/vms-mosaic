! MMS description file for libgdk-pixbuf
! George E. Cook, WVNET, 17-Aug-2008
! Mosaic 4.3
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libgdk-pixbuf.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.IFDEF DECC
CQUALC = /DECC/INCLUDE=([],[-.GLIB],[-.LIBINTL],[-.ZLIB])
.ELSE
.IFDEF DECCVAXC
CQUALC = /VAXC/INCLUDE=([],[-.GLIB],[-.LIBINTL],[-.ZLIB])
.ELSE
.IFDEF GNUC
#CQUALC = /INCLUDE=(GCC_Include)
CQUALC = 
.ELSE
CQUALC = /INCLUDE=([],[-.GLIB],[-.LIBINTL],[-.ZLIB])
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

OBJECTS = Odir:gdk-pixbuf-animation.obj Odir:gdk-pixbuf-data.obj \
	  Odir:gdk-pixbuf-enum-types.obj Odir:gdk-pixbuf-io.obj \
	  Odir:gdk-pixbuf-loader.obj Odir:gdk-pixbuf-scale.obj \
	  Odir:gdk-pixbuf-util.obj Odir:gdk-pixbuf.obj Odir:gdk-pixdata.obj \
	  Odir:pixops.obj Odir:io-jpeg.obj Odir:io-png.obj

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
        @ Define/NoLog Odir $(WDIR)
	@ Topdir = F$Environment("Default")-"LIBGDK-PIXBUF]"
        @ Define/NoLog GDK-PIXBUF []
	@ Define/NoLog GLIB 'Topdir'GLIB]
	@ Define/Nolog GOBJECT 'Topdir'GLIB.GOBJECT]
.IFDEF GNUC
        @ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
        @- Define/NoLog Sys GNU_CC_Include
.ELSE
        @- Define/NoLog Sys SYS$Library
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
	@ Write SYS$Output "Library libgdk-pixbuf.olb built."

Odir:gdk-pixbuf-animation.obj : gdk-pixbuf-animation.c  gdk-pixbuf-animation.h \
	gdk-pixbuf_config.h
Odir:gdk-pixbuf-data.obj : gdk-pixbuf-data.c gdk-pixbuf-private.h \
	gdk-pixbuf_config.h
Odir:gdk-pixbuf-enum-types.obj : gdk-pixbuf-enum-types.c gdk-pixbuf-private.h \
	gdk-pixbuf_config.h
Odir:gdk-pixbuf-io.obj : gdk-pixbuf-io.c gdk-pixbuf-io.h gdk-pixbuf-private.h \
	gdk-pixbuf_config.h
Odir:gdk-pixbuf-loader.obj : gdk-pixbuf-loader.c gdk-pixbuf-loader.h \
	 gdk-pixbuf-private.h gdk-pixbuf_config.h
Odir:gdk-pixbuf-scale.obj : gdk-pixbuf-scale.c gdk-pixbuf-private.h \
	gdk-pixbuf_config.h
Odir:gdk-pixbuf-util.obj : gdk-pixbuf-util.c gdk-pixbuf-private.h \
	gdk-pixbuf_config.h
Odir:gdk-pixbuf.obj : gdk-pixbuf.c gdk-pixbuf.h gdk-pixbuf-private.h \
	gdk-pixbuf-i18n.h gdk-pixbuf_config.h
Odir:gdk-pixdata.obj : gdk-pixdata.c gdk-pixdata.h gdk-pixbuf-private.h \
	gdk-pixbuf_config.h
Odir:pixops.obj : pixops.c pixops.h pixops-internal.h
Odir:io-jpeg.obj : io-jpeg.c gdk-pixbuf-private.h gdk-pixbuf_config.h
Odir:io-png.obj : io-png.c gdk-pixbuf-private.h gdk-pixbuf_config.h

.c.obj :
        $(CC)$(CFLAGS)$(CFLOAT)/OBJECT=$@ $<

.obj.olb
        $(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
        Delete/Log $(WDIR)*.OBJ;*
        Delete/Log $(LIBTARGET);*
