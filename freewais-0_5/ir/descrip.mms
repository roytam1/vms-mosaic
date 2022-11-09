! MMS description file for LIBWAIS
! Bjorn S. Nilsson, Aleph, CERN, 20-Aug-1994
! Mosaic version 2.4 20-Aug-1994
! Only routines needed for Mosaic 2.4 are included in this compilation
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libwais.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.IFDEF DECC
.IFDEF MULTINET
CQUALC=/DECC/Prefix=ANSI
.ELSE
CQUALC=/DECC/Prefix=ALL
.ENDIF
.ELSE
.IFDEF DECCVAXC
CQUALC=/VAXC
.ELSE
CQUALC=
.ENDIF
.ENDIF

.IFDEF MULTINET
CDEFS = /Define=MULTINET
.ENDIF

.IFDEF SOCKETSHR ! BGT
CDEFS = /Define=SOCKETSHR ! BGT
.ENDIF ! BGT

.IFDEF DEBUG
CFLAGS = $(CQUALC)$(CDEFS)/NoOpt/Debug
.ELSE
CFLAGS = $(CQUALC)$(CDEFS)
.ENDIF

OBJECTS = Odir:cutil.obj Odir:futil.obj Odir:panic.obj Odir:ui.obj \
          Odir:wmessage.obj Odir:wprot.obj Odir:wutil.obj Odir:zprot.obj \
          Odir:ztype1.obj Odir:zutil.obj

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

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
	@ Write SYS$Output "Library libwais.olb built."

Odir:cutil.obj    : cutil.c cutil.h cdialect.h panic.h
Odir:futil.obj    : futil.c futil.h cdialect.h cutil.h panic.h
Odir:panic.obj    : panic.c panic.h cdialect.h futil.h cutil.h
Odir:ui.obj       : ui.c ui.h cdialect.h zprot.h zutil.h cutil.h wprot.h \
                    ztype1.h transprt.h wmessage.h panic.h version.h server.h \
                    wutil.h ustubs.h futil.h
Odir:wmessage.obj : wmessage.c wmessage.h cdialect.h ustubs.h cutil.h
Odir:wprot.obj    : wprot.c wprot.h cdialect.h zprot.h zutil.h cutil.h \
                    ztype1.h panic.h
Odir:wutil.obj    : wutil.c futil.h cdialect.h cutil.h zprot.h zutil.h wprot.h \
                    ztype1.h wutil.h
Odir:zprot.obj    : zprot.c zprot.h cdialect.h zutil.h cutil.h
Odir:ztype1.obj   : ztype1.c ztype1.h cdialect.h zutil.h cutil.h panic.h
Odir:zutil.obj    : zutil.c zutil.h cdialect.h cutil.h

.c.obj
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	Delete/Log $(WDIR)*.OBJ;*
	Delete/Log $(LIBTARGET);*

