! MMS description file for LIBTIFF
! Mosaic version 4.1 5-Jul-2006, George Cook
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libtiff.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
	@ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.IFDEF DECC
.IFDEF ALPHA
	@ If F$TRNLNM("ALPHA$LIBRARY") .NES. "" Then Define/NoLog Sys Alpha$Library
.ELSE
	@ If F$TRNLNM("DECC$LIBRARY_INCLUDE") .NES. "" Then Define/NoLog Sys DECC$Library_Include
.ENDIF
.ELSE
.IFDEF GNUC
	@ Define/NoLog Sys GNU_CC_Include
.ELSE
	@ Define/NoLog Sys SYS$Library
.ENDIF
.ENDIF

.IFDEF DECC
CQUAL = /DECC/INCLUDE=([-.ZLIB],[-.LIBJPEG])
.ELSE
.IFDEF DECCVAXC
CQUAL = /VAXC/INCLUDE=([-.ZLIB],[-.LIBJPEG])
.ELSE
.IFDEF GNUC
CQUAL = /INCLUDE=(GCC_Include,[-.ZLIB.],[-.LIBJPEG])
.ELSE
/INCLUDE=([-.ZLIB],[-.LIBJPEG])
.ENDIF
.ENDIF
.ENDIF

.IFDEF DEBUG
CC_DEFS = $(CQUAL)/NoOpt/Debug
.ELSE
CC_DEFS = $(CQUAL)
.ENDIF

OBJS =  Odir:tif_aux.obj, \
        Odir:tif_close.obj, \
        Odir:tif_codec.obj, \
        Odir:tif_color.obj, \
        Odir:tif_compress.obj, \
        Odir:tif_dir.obj, \
        Odir:tif_dirinfo.obj, \
        Odir:tif_dirread.obj, \
        Odir:tif_dirwrite.obj, \
        Odir:tif_dumpmode.obj, \
        Odir:tif_error.obj, \
        Odir:tif_extension.obj, \
        Odir:tif_fax3.obj, \
        Odir:tif_fax3sm.obj, \
        Odir:tif_flush.obj, \
        Odir:tif_getimage.obj, \
        Odir:tif_jpeg.obj, \
        Odir:tif_ojpeg.obj, \
        Odir:tif_luv.obj, \
        Odir:tif_lzw.obj, \
        Odir:tif_next.obj, \
        Odir:tif_open.obj, \
        Odir:tif_packbits.obj, \
        Odir:tif_pixarlog.obj, \
        Odir:tif_predict.obj, \
        Odir:tif_print.obj, \
        Odir:tif_read.obj, \
        Odir:tif_swab.obj, \
        Odir:tif_strip.obj, \
        Odir:tif_thunder.obj, \
        Odir:tif_tile.obj, \
        Odir:tif_version.obj, \
        Odir:tif_warning.obj, \
        Odir:tif_write.obj, \
        Odir:tif_zip.obj, \
        Odir:tif_vms.obj, \
        Odir:lfind.obj

CFLAGS= $(CC_DEFS)

$(LIBTARGET) : $(LIBTARGET)($(OBJS))
	@ Write SYS$Output "Library libtiff.olb built."

.c.obj
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	delete $(WDIR)*.obj;*,$(LIBTARGET);*


# Other dependencies.
Odir:tif_aux.obj : 	 tif_aux.c, tiffconf.h
Odir:tif_close.obj : 	 tif_close.c, tiffconf.h
Odir:tif_codec.obj : 	 tif_codec.c, tiffconf.h
Odir:tif_color.obj : 	 tif_color.c, tiffconf.h
Odir:tif_compress.obj :	 tif_compress.c, tiffconf.h
Odir:tif_dir.obj : 	 tif_dir.c, tiffconf.h
Odir:tif_dirinfo.obj : 	 tif_dirinfo.c, tiffconf.h
Odir:tif_dirread.obj : 	 tif_dirread.c, tiffconf.h
Odir:tif_dirwrite.obj :	 tif_dirwrite.c, tiffconf.h
Odir:tif_dumpmode.obj :	 tif_dumpmode.c, tiffconf.h
Odir:tif_error.obj : 	 tif_error.c, tiffconf.h
Odir:tif_extension.obj : tif_extension.c, tiffconf.h
Odir:tif_fax3.obj : 	 tif_fax3.c, tiffconf.h
Odir:tif_fax3sm.obj : 	 tif_fax3sm.c, tiffconf.h
Odir:tif_flush.obj : 	 tif_flush.c, tiffconf.h
Odir:tif_getimage.obj :	 tif_getimage.c, tiffconf.h
Odir:tif_jpeg.obj : 	 tif_jpeg.c, tiffconf.h, [-.libjpeg]jpeglib.h
Odir:tif_ojpeg.obj : 	 tif_ojpeg.c, tiffconf.h, [-.libjpeg]jpeglib.h
Odir:tif_luv.obj : 	 tif_luv.c, tiffconf.h
Odir:tif_lzw.obj : 	 tif_lzw.c, tiffconf.h
Odir:tif_next.obj : 	 tif_next.c, tiffconf.h
Odir:tif_open.obj : 	 tif_open.c, tiffconf.h
Odir:tif_packbits.obj :	 tif_packbits.c, tiffconf.h
Odir:tif_pixarlog.obj :  tif_pixarlog.c, tiffconf.h, [-.zlib]zlib.h
Odir:tif_predict.obj : 	 tif_predict.c, tiffconf.h
Odir:tif_print.obj : 	 tif_print.c, tiffconf.h
Odir:tif_read.obj : 	 tif_read.c, tiffconf.h
Odir:tif_swab.obj : 	 tif_swab.c, tiffconf.h
Odir:tif_strip.obj : 	 tif_strip.c, tiffconf.h
Odir:tif_thunder.obj : 	 tif_thunder.c, tiffconf.h
Odir:tif_tile.obj : 	 tif_tile.c, tiffconf.h
Odir:tif_version.obj : 	 tif_version.c, tiffconf.h
Odir:tif_warning.obj : 	 tif_warning.c, tiffconf.h
Odir:tif_write.obj : 	 tif_write.c, tiffconf.h
Odir:tif_zip.obj :	 tif_zip.c, tiffconf.h, [-.zlib]zlib.h
Odir:tif_vms.obj : 	 tif_vms.c, tiffconf.h
Odir:lfind.obj : 	 lfind.c
