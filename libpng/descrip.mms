! MMS description file for LIBPNG
! Mosaic version 2.6 20-Feb-1996, George Cook
! Mosaic version 2.7 10-May-1997, George Cook
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libpng.olb

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
CQUAL = /DECC/INCLUDE=[-.ZLIB]
.ELSE
.IFDEF DECCVAXC
CQUAL = /VAXC/PRECISION=SINGLE/INCLUDE=[-.ZLIB]
.ELSE
.IFDEF GNUC
CQUAL = /INCLUDE=(GCC_Include,[-.ZLIB.])
.ELSE
CQUAL = /PRECISION=SINGLE/INCLUDE=[-.ZLIB]
.ENDIF
.ENDIF
.ENDIF

.IFDEF DEBUG
CC_DEFS = $(CQUAL)/NoOpt/Debug
.ELSE
CC_DEFS = $(CQUAL)
.ENDIF


OBJS = Odir:png.obj, Odir:pngget.obj, Odir:pngrutil.obj, Odir:pngtrans.obj,\
   Odir:pngwutil.obj, Odir:pngread.obj, Odir:pngmem.obj, Odir:pngwrite.obj,\
   Odir:pngrtran.obj, Odir:pngwtran.obj, Odir:pngrio.obj, Odir:pngerror.obj,\
   Odir:pngwio.obj, Odir:pngpread.obj , Odir:pngset.obj


CFLAGS= $(CC_DEFS)

$(LIBTARGET) : $(LIBTARGET)($(OBJS))
	@ Write SYS$Output "Library libpng.olb built."

.c.obj
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	delete $(WDIR)*.obj;*,$(LIBTARGET);*


# Other dependencies.
Odir:png.obj : png.c, png.h, pngconf.h
Odir:pngpread.obj : pngpread.c, png.h, pngconf.h
Odir:pngget.obj : pngget.c, png.h, pngconf.h
Odir:pngset.obj : pngset.c, png.h, pngconf.h
Odir:pngread.obj : pngread.c, png.h, pngconf.h, [-.zlib]zlib.h
Odir:pngrtran.obj : pngrtran.c, png.h, pngconf.h
Odir:pngrutil.obj : pngrutil.c, png.h, pngconf.h
Odir:pngerror.obj : pngerror.c, png.h, pngconf.h
Odir:pngmem.obj : pngmem.c, png.h, pngconf.h
Odir:pngrio.obj : pngrio.c, png.h, pngconf.h
Odir:pngwio.obj : pngwio.c, png.h, pngconf.h
Odir:pngtrans.obj : pngtrans.c, png.h, pngconf.h
Odir:pngwrite.obj : pngwrite.c, png.h, pngconf.h
Odir:pngwtran.obj : pngwtran.c, png.h, pngconf.h
Odir:pngwutil.obj : pngwutil.c, png.h, pngconf.h

# Martin P.J. Zinser		           Email: 
# KP II					          m.zinser@gsi.de
# Gesellschaft f. Schwerionenforschung GSI        vipmzs.physik.uni-mainz.de
# Postfach 11 05 52                               mzdmza.zdv.uni-mainz.de
# D-64220 Darmstadt 		           Voice: 0049+6151/3592887
