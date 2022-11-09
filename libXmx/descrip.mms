! MMS description file for LIBXMX
! Bjorn S. Nilsson, Aleph, CERN, 20-Nov-1993
! (Mosaic version 2.0)
! Motif 1.2 support added on 3-Jun-1994
! Mosaic 2.4 20-Aug-1994
!
! Copyright (C) 2006 - The VMS Mosaic Project
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libxmx.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.IFDEF DECC
.IFDEF PATHWAY
.INCLUDE [-.TWG]DECC_PREFIX_RULES.MMS
CQUALC=/DECC $(CC_PREFIX_NO_SIN)
.ELSE
.IFDEF MULTINET
CQUALC=/DECC/Prefix=ANSI
.ELSE
CQUALC=/DECC/Prefix=ALL
.ENDIF
.ENDIF
.ELSE ! Not DEC C
.IFDEF DECCVAXC
CQUALC=/VAXC
.ELSE
CQUALC=
.ENDIF
.ENDIF

.IFDEF DEBUG
CFLAGS = $(CQUALC)/NoOpt/Debug
.ELSE
CFLAGS = $(CQUALC)
.ENDIF

OBJECTS = Odir:xmx.obj Odir:xmx2.obj

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
	@ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.IFDEF PATHWAY
	@ @[-.TWG]def
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
	@ Write SYS$Output "Library libXmx.olb built."

Odir:xmx.obj  : xmx.c [-]config.h [-]config_$(WORK).h xmx.h xmxp.h\
		[-.libliteclue]liteclue.h
Odir:xmx2.obj : xmx2.c [-]config.h [-]config_$(WORK).h xmx.h xmxp.h

.c.obj :
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	Delete/Log $(WDIR)*.OBJ;*
	Delete/Log $(LIBTARGET);*

