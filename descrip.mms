! Descrip.MMS to build all of Mosaic on VMS
!
! Copyright (C) - The VMS Mosaic Project
!
! Björn S. Nilsson, Aleph, CERN, 22-Nov-1993
! Support for Motif 1.2 added on 3-Jun-1994
! Mosaic 2.4 on 21-Aug-1994
! Mosaic 2.6 on 1-Nov-1995, George Cook
! Libvms added on 12-May-1996, George Cook
! Libliteclue added on 22-Feb-2006, George Cook
! Libtiff added on 5-Jul-2006, George Cook
!
! Usage:
!        $ @MAKE_MOSAIC
!
! This DESCRIP.MMS is not intended to be invoked directly.  It should
! only be invoked by running MAKE_MOSAIC.COM.
!
!
!  MULTINET=1              Use MULTINET (default is UCX)
!  DEBUG=1                 Make a debug executable
!  DECC=1                  DEC C compilation.
!  GNUC=1                  GNU C compilation.
!  VAXC=1                  VAX C compilation.
!  PATHWAY=1               Use PathWay for TCP. (PathWay is Wollongong)
!  MOTIF1_2                Use Motif 1.2 libraries.
!  NOTIFF=1                Compile without tiff image support
!  NOWAIS=1                Compile without direct WAIS support
!  USE_MMK=1               Build using MMK instead of MMS
!  SOCKETSHR=1             Use SOCKETSHR and NETLIB for TCP interface - BGT
!

WDIR = [.$(WORK)]

.IFDEF NOTIFF
.IFDEF NOWAIS
LIBS = libwww2 libXmx libhtmlw libjpeg libopenjpeg libnut libpng zlib \
       libliteclue libvms src
.ELSE
LIBS = libWAIS libwww2 libXmx libhtmlw libjpeg libopenjpeg libnut libpng zlib \
       libliteclue libvms src
.ENDIF

.ELSE

.IFDEF NOWAIS
LIBS = libwww2 libXmx libhtmlw libjpeg libopenjpeg libnut libpng zlib \
       libtiff libliteclue libvms src
.ELSE
LIBS = libWAIS libwww2 libXmx libhtmlw libjpeg libopenjpeg libnut libpng zlib \
       libtiff libliteclue libvms src
.ENDIF
.ENDIF

default : $(LIBS)

.IFDEF NOLINK
	@ Write SYS$Output "Skipping link"
.ELSE
	@ Write SYS$Output "The executable is in [.src]Mosaic.exe_$(WORK)"
	@ Write SYS$Output "Please complete the Comment Card on the Help menu"
.ENDIF

libWAIS :
	@ Write SYS$Output "--- Building libWAIS"
	@ Set Default [.freeWAIS-0_5.ir]
        @ If F$Search("$(WORK).dir") .EQS. "" Then Create/Dir $(WDIR)
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("$(WDIR)*.olb") .NES. "") Then Delete $(WDIR)*.olb;*
	@ If (F$Search("$(WDIR)work.mark") .EQS. "") .AND. (F$Search("$(WDIR)work.done") .EQS. "") Then Copy Descrip.mms $(WDIR)work.mark
.IFDEF USE_MMK
	@ MMK$(MMSQUALIFIERS)
.ELSE
	@ $(MMS)$(MMSQUALIFIERS)
.ENDIF
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
        @ If F$Search("$(WDIR)*.obj") .NES. "" Then Delete $(WDIR)*.obj;*
	@ Set Default [-.-]

libXmx :
	@ Write SYS$Output "--- Building libXmx"
	@ Set Default [.libxmx]
        @ If F$Search("$(WORK).dir") .EQS. "" Then Create/Dir $(WDIR)
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("$(WDIR)*.olb") .NES. "") Then Delete $(WDIR)*.olb;*
	@ If (F$Search("$(WDIR)work.mark") .EQS. "") .AND. (F$Search("$(WDIR)work.done") .EQS. "") Then Copy Descrip.mms $(WDIR)work.mark
.IFDEF USE_MMK
	@ MMK$(MMSQUALIFIERS)
.ELSE
	@ $(MMS)$(MMSQUALIFIERS)
.ENDIF
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
        @ If F$Search("$(WDIR)*.obj") .NES. "" Then Delete $(WDIR)*.obj;*
	@ Set Default [-]

libhtmlw :
	@ Write SYS$Output "--- Building libhtmlw"
	@ Set Default [.libhtmlw]
        @ If F$Search("$(WORK).dir") .EQS. "" Then Create/Dir $(WDIR)
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("$(WDIR)*.olb") .NES. "") Then Delete $(WDIR)*.olb;*
	@ If (F$Search("$(WDIR)work.mark") .EQS. "") .AND. (F$Search("$(WDIR)work.done") .EQS. "") Then Copy Descrip.mms $(WDIR)work.mark
.IFDEF USE_MMK
	@ MMK$(MMSQUALIFIERS)
.ELSE
	@ $(MMS)$(MMSQUALIFIERS)
.ENDIF
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
        @ If F$Search("$(WDIR)*.obj") .NES. "" Then Delete $(WDIR)*.obj;*
	@ Set Default [-]

libwww2 :
	@ Write SYS$Output "--- Building libwww2"
	@ Set Default [.libwww2]
        @ If F$Search("$(WORK).dir") .EQS. "" Then Create/Dir $(WDIR)
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("$(WDIR)*.olb") .NES. "") Then Delete $(WDIR)*.olb;*
	@ If (F$Search("$(WDIR)work.mark") .EQS. "") .AND. (F$Search("$(WDIR)work.done") .EQS. "") Then Copy Descrip.mms $(WDIR)work.mark
.IFDEF USE_MMK
	@ MMK$(MMSQUALIFIERS)
.ELSE
	@ $(MMS)$(MMSQUALIFIERS)
.ENDIF
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
        @ If F$Search("$(WDIR)*.obj") .NES. "" Then Delete $(WDIR)*.obj;*
	@ Set Default [-]

libjpeg :
	@ Write SYS$Output "--- Building libjpeg"
	@ Set Default [.libjpeg]
        @ If F$Search("$(WORK).dir") .EQS. "" Then Create/Dir $(WDIR)
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("$(WDIR)*.olb") .NES. "") Then Delete $(WDIR)*.olb;*
	@ If (F$Search("$(WDIR)work.mark") .EQS. "") .AND. (F$Search("$(WDIR)work.done") .EQS. "") Then Copy Descrip.mms $(WDIR)work.mark
.IFDEF USE_MMK
	@ MMK$(MMSQUALIFIERS)
.ELSE
	@ $(MMS)$(MMSQUALIFIERS)
.ENDIF
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
        @ If F$Search("$(WDIR)*.obj") .NES. "" Then Delete $(WDIR)*.obj;*
	@ Set Default [-]

libopenjpeg :
	@ Write SYS$Output "--- Building libopenjpeg"
	@ Set Default [.libopenjpeg]
        @ If F$Search("$(WORK).dir") .EQS. "" Then Create/Dir $(WDIR)
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("$(WDIR)*.olb") .NES. "") Then Delete $(WDIR)*.olb;*
	@ If (F$Search("$(WDIR)work.mark") .EQS. "") .AND. (F$Search("$(WDIR)work.done") .EQS. "") Then Copy Descrip.mms $(WDIR)work.mark
.IFDEF USE_MMK
	@ MMK$(MMSQUALIFIERS)
.ELSE
	@ $(MMS)$(MMSQUALIFIERS)
.ENDIF
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
        @ If F$Search("$(WDIR)*.obj") .NES. "" Then Delete $(WDIR)*.obj;*
	@ Set Default [-]

libnut :
	@ Write SYS$Output "--- Building libnut"
	@ Set Default [.libnut]
        @ If F$Search("$(WORK).dir") .EQS. "" Then Create/Dir $(WDIR)
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("$(WDIR)*.olb") .NES. "") Then Delete $(WDIR)*.olb;*
	@ If (F$Search("$(WDIR)work.mark") .EQS. "") .AND. (F$Search("$(WDIR)work.done") .EQS. "") Then Copy Descrip.mms $(WDIR)work.mark
.IFDEF USE_MMK
	@ MMK$(MMSQUALIFIERS)
.ELSE
	@ $(MMS)$(MMSQUALIFIERS)
.ENDIF
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
        @ If F$Search("$(WDIR)*.obj") .NES. "" Then Delete $(WDIR)*.obj;*
	@ Set Default [-]

libpng :
	@ Write SYS$Output "--- Building libpng"
	@ Set Default [.libpng]
        @ If F$Search("$(WORK).dir") .EQS. "" Then Create/Dir $(WDIR)
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("$(WDIR)*.olb") .NES. "") Then Delete $(WDIR)*.olb;*
	@ If (F$Search("$(WDIR)work.mark") .EQS. "") .AND. (F$Search("$(WDIR)work.done") .EQS. "") Then Copy Descrip.mms $(WDIR)work.mark
.IFDEF USE_MMK
	@ MMK$(MMSQUALIFIERS)
.ELSE
	@ $(MMS)$(MMSQUALIFIERS)
.ENDIF
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
        @ If F$Search("$(WDIR)*.obj") .NES. "" Then Delete $(WDIR)*.obj;*
	@ Set Default [-]

zlib :
	@ Write SYS$Output "--- Building zlib"
	@ Set Default [.zlib]
        @ If F$Search("$(WORK).dir") .EQS. "" Then Create/Dir $(WDIR)
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("$(WDIR)*.olb") .NES. "") Then Delete $(WDIR)*.olb;*
	@ If (F$Search("$(WDIR)work.mark") .EQS. "") .AND. (F$Search("$(WDIR)work.done") .EQS. "") Then Copy Descrip.mms $(WDIR)work.mark
.IFDEF USE_MMK
	@ MMK$(MMSQUALIFIERS)
.ELSE
	@ $(MMS)$(MMSQUALIFIERS)
.ENDIF
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
        @ If F$Search("$(WDIR)*.obj") .NES. "" Then Delete $(WDIR)*.obj;*
	@ Set Default [-]

libtiff :
	@ Write SYS$Output "--- Building libtiff"
	@ Set Default [.libtiff]
        @ If F$Search("$(WORK).dir") .EQS. "" Then Create/Dir $(WDIR)
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("$(WDIR)*.olb") .NES. "") Then Delete $(WDIR)*.olb;*
	@ If (F$Search("$(WDIR)work.mark") .EQS. "") .AND. (F$Search("$(WDIR)work.done") .EQS. "") Then Copy Descrip.mms $(WDIR)work.mark
.IFDEF USE_MMK
	@ MMK$(MMSQUALIFIERS)
.ELSE
	@ $(MMS)$(MMSQUALIFIERS)
.ENDIF
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
        @ If F$Search("$(WDIR)*.obj") .NES. "" Then Delete $(WDIR)*.obj;*
	@ Set Default [-]

libliteclue :
	@ Write SYS$Output "--- Building libliteclue"
	@ Set Default [.libliteclue]
        @ If F$Search("$(WORK).dir") .EQS. "" Then Create/Dir $(WDIR)
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("$(WDIR)*.olb") .NES. "") Then Delete $(WDIR)*.olb;*
	@ If (F$Search("$(WDIR)work.mark") .EQS. "") .AND. (F$Search("$(WDIR)work.done") .EQS. "") Then Copy Descrip.mms $(WDIR)work.mark
.IFDEF USE_MMK
	@ MMK$(MMSQUALIFIERS)
.ELSE
	@ $(MMS)$(MMSQUALIFIERS)
.ENDIF
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
        @ If F$Search("$(WDIR)*.obj") .NES. "" Then Delete $(WDIR)*.obj;*
	@ Set Default [-]

libvms :
	@ Write SYS$Output "--- Building libvms"
	@ Set Default [.libvms]
        @ If F$Search("$(WORK).dir") .EQS. "" Then Create/Dir $(WDIR)
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("$(WDIR)*.olb") .NES. "") Then Delete $(WDIR)*.olb;*
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("ucx$ipc.olb") .NES. "") Then Delete ucx$ipc.olb;*
	@ If (F$Search("$(WDIR)work.mark") .EQS. "") .AND. (F$Search("$(WDIR)work.done") .EQS. "") Then Copy Descrip.mms $(WDIR)work.mark
.IFDEF USE_MMK
	@ MMK$(MMSQUALIFIERS)
.ELSE
	@ $(MMS)$(MMSQUALIFIERS)
.ENDIF
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
        @ If F$Search("$(WDIR)*.obj") .NES. "" Then Delete $(WDIR)*.obj;*
        @ If F$Search("ucx$crtlibxfr.obj") .NES. "" Then Delete ucx$crtlibxfr.obj;*
        @ If F$Search("mosaic.rnh") .NES. "" Then Delete mosaic.rnh;*
	@ Set Default [-]

src :
	@ Write SYS$Output "--- Building src"
	@ Set Default [.src]
        @ If F$Search("$(WORK).dir") .EQS. "" Then Create/Dir $(WDIR)
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("$(WDIR)*.olb") .NES. "") Then Delete $(WDIR)*.olb;*
	@ If (F$Search("$(WDIR)work.mark") .EQS. "") .AND. (F$Search("$(WDIR)work.done") .EQS. "") Then Copy Descrip.mms $(WDIR)work.mark
.IFDEF USE_MMK
	@ MMK$(MMSQUALIFIERS)
.ELSE
	@ $(MMS)$(MMSQUALIFIERS)
.ENDIF
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
        @ If F$Search("$(WDIR)*.obj") .NES. "" Then Delete $(WDIR)*.obj;*
        @ If F$Search("$(WDIR)*.opt") .NES. "" Then Delete $(WDIR)*.opt;*
	@ Set Default [-]
	@ Purge *.h

clean :
	@ Write SYS$Output "--- Cleaning tree"
	@ Set Default [.freeWAIS-0_5.ir.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [---]
!
	@ Set Default [.libxmx.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
!
	@ Set Default [.libhtmlw.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
!
	@ Set Default [.libwww2.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
!
	@ Set Default [.libjpeg.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
!
	@ Set Default [.libnut.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
!
	@ Set Default [.libpng.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
!
	@ Set Default [.zlib.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
!
	@ Set Default [.libtiff.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
!
	@ Set Default [.libliteclue.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
!
	@ Set Default [.libvms.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
!
	@ Set Default [.src.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
