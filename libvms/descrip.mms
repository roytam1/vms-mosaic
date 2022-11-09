! MMS description file for libvms (VMS Library for Mosaic)
! George E. Cook, WVNET, 12-May-1996
! Mosaic 2.7-4
!
! Copyright (C) 2007 - The VMS Mosaic Project
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libvms.olb

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

OBJECTS =  Odir:cmdline.obj Odir:mosaic_cld.obj

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
	@ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.IFDEF PATHWAY
	@ @[-.TWG]def
.ENDIF
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

.IFDEF CMU
.IFDEF DECC
.ELSE
default :       $(LIBTARGET) ucx$ipc.olb [-]mosaic.hlp
	@	! Do nothing

ucx$ipc.olb :	ucx$ipc.olb(ucx$crtlibxfr.obj)
	@ Write SYS$Output "Library ucx$ipc.olb built."

ucx$crtlibxfr.obj : ucx$crtlibxfr.mar
        @ If F$Search("UCX$IPC.OLB") .EQS. "" Then Library/Create UCX$IPC.OLB
	MACRO/NOLIST/OBJECT=UCX$CRTLIBXFR.OBJ UCX$CRTLIBXFR.MAR
.ENDIF
.ELSE
default :       $(LIBTARGET) [-]mosaic.hlp
	@	! Do nothing
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
	@ Write SYS$Output "Library libvms.olb built."

Odir:cmdline.obj    : cmdline.c cmdline.h [-.src]mosaic.h [-]config.h \
		      [-]config_$(WORK).h [-]ssl_$(WORK).h
Odir:mosaic_cld.obj : mosaic_cld.cld

mosaic.rnh          : mosaic.help
        EDIT/TPU/NOSECTION/NODISPLAY/COMMAND=CVTHELP.TPU MOSAIC.HELP
[-]mosaic.hlp       : mosaic.rnh
	$(RUNOFF) $(RFLAGS) $(MMS$SOURCE)
	@ Write SYS$Output "Mosaic help file created."

.c.obj
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	Delete/Log $(WDIR)*.OBJ;*
	Delete/Log $(LIBTARGET);*

