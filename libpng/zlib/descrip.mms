! descrip.mms: MMS description file for building zlib on VMS
! written by Martin P.J. Zinser <m.zinser@gsi.de>
! Mosaic version 2.6 20-Feb-1996, George Cook
! zlib version 1.2.1 11-JAN-2004, George Cook
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libz.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.IFDEF DECC
CQUAL = /DECC
.ELSE
.IFDEF DECCVAXC
CQUAL = /VAXC
.ELSE
CQUAL =
.ENDIF
.ENDIF

.IFDEF DEBUG
CC_DEFS = $(CQUAL)/NoOpt/Debug
.ELSE
CC_DEFS = $(CQUAL)
.ENDIF

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
	@ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")

OBJS = Odir:adler32.obj, Odir:compress.obj, Odir:crc32.obj, Odir:deflate.obj,\
       Odir:gzio.obj, Odir:infback.obj, Odir:inffast.obj, Odir:inflate.obj,\
       Odir:inftrees.obj, Odir:trees.obj, Odir:uncompr.obj, Odir:zutil.obj

CFLAGS= $(CC_DEFS)

$(LIBTARGET) : $(LIBTARGET)($(OBJS))
	@ Write SYS$Output "Library libz.olb built."

.c.obj
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean : 
	delete $(WDIR)*.obj;*,$(LIBTARGET);*


# Other dependencies.
Odir:adler32.obj : adler32.c zlib.h zconf.h
Odir:compress.obj : compress.c zlib.h zconf.h
Odir:crc32.obj : crc32.c crc32.h zlib.h zconf.h
Odir:deflate.obj : deflate.c deflate.h zutil.h zlib.h zconf.h
Odir:gzio.obj : gzio.c zutil.h zlib.h zconf.h
Odir:inffast.obj : inffast.c zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h
Odir:inflate.obj : inflate.c zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h
Odir:infback.obj : infback.c zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h
Odir:inftrees.obj : inftrees.c zutil.h zlib.h zconf.h inftrees.h
Odir:trees.obj : trees.c deflate.h zutil.h zlib.h zconf.h trees.h
Odir:uncompr.obj : uncompr.c zlib.h zconf.h
Odir:zutil.obj : zutil.c zutil.h zlib.h zconf.h
