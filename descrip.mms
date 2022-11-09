! Descrip.MMS to build all of Mosaic on VMS
!
! Copyright (C) 2007, 2008 - The VMS Mosaic Project
!
! Björn S. Nilsson, Aleph, CERN, 22-Nov-1993
! Support for Motif 1.2 added on 3-Jun-1994
! Mosaic 2.4 on 21-Aug-1994
! Mosaic 2.6 on 1-Nov-1995, George Cook
! Libvms added on 12-May-1996, George Cook
! Libliteclue added on 22-Feb-2006, George Cook
! Libtiff added on 5-Jul-2006, George Cook
! Libsvg added on 21-Nov-2007, George Cook
! Libxml added on 22-Nov-2007, George Cook
! Libintl added on 5-Jan-2008, George Cook
! Libcroco and Glib added on 21-Jan-2008, George Cook
! Librsvg replaced Libsvg on 15-Aug-2008
! Libgdk-pixbuf added on 17-Aug-2008, George Cook 
! Libexpat and Libfontconfig added on 18-Aug-2008, George Cook 
! Libfreetype added on 19-Aug-2008, George Cook 
! Libpango added on 22-Aug-2008, George Cook 
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
!  NOSVG=1                 Compile without SVG image support
!  NOTIFF=1                Compile without tiff image support
!  NOWAIS=1                Compile without direct WAIS support
!  USE_MMK=1               Build using MMK instead of MMS
!  SOCKETSHR=1             Use SOCKETSHR and NETLIB for TCP interface - BGT
!

WDIR = [.$(WORK)]

.IFDEF NOTIFF
.IFDEF NOWAIS
LIBS = libwww2 libXmx libhtmlw libjpeg libopenjpeg libnut libpng zlib \
       librsvg libcroco libxml2 glib libintl libart libgdk-pixbuf libexpat\
       libfontconfig libfreetype libpango libliteclue libvms src
.ELSE
LIBS = libWAIS libwww2 libXmx libhtmlw libjpeg libopenjpeg libnut libpng zlib \
       librsvg libcroco libxml2 glib libintl libart libgdk-pixbuf libexpat\
       libfontconfig libfreetype libpango libliteclue libvms src
.ENDIF

.ELSE

.IFDEF NOWAIS
LIBS = libwww2 libXmx libhtmlw libjpeg libopenjpeg libnut libpng zlib \
       libtiff librsvg libcroco libxml2 glib libintl libart libgdk-pixbuf \
       libexpat libfontconfig libfreetype libpango libliteclue libvms src
.ELSE
LIBS = libWAIS libwww2 libXmx libhtmlw libjpeg libopenjpeg libnut libpng zlib \
       libtiff librsvg libcroco libxml2 glib libintl libart libgdk-pixbuf \
       libexpat libfontconfig libfreetype libpango libliteclue libvms src
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

librsvg :
.IFDEF NOSVG
	@ Write SYS$Output "--- Skipping librsvg build"
.ELSE
	@ Write SYS$Output "--- Building librsvg"
	@ Set Default [.librsvg]
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
.ENDIF

libcroco :
	@ Write SYS$Output "--- Building libcroco"
	@ Set Default [.libcroco]
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

libxml2 :
	@ Write SYS$Output "--- Building libxml2"
	@ Set Default [.libxml2]
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

glib :
	@ Write SYS$Output "--- Building glib and gobjectlib"
	@ Set Default [.glib]
        @ If F$Search("$(WORK).dir") .EQS. "" Then Create/Dir $(WDIR)
        @ If (F$Search("$(WDIR)work.mark") .NES. "") .AND. (F$Search("$(WDIR)*.olb") .NES. "") Then Delete $(WDIR)*.olb;*
	@ If (F$Search("$(WDIR)work.mark") .EQS. "") .AND. (F$Search("$(WDIR)work.done") .EQS. "") Then Copy Descrip.mms $(WDIR)work.mark
.IFDEF USE_MMK
	@ MMK$(MMSQUALIFIERS)
.IFDEF NOSVG
	@ Write SYS$Output "--- Skipping gobjectlib build"
.ELSE
	@ Set Default [.gobject]
	@ MMK$(MMSQUALIFIERS)
	@ Set Default [-]
.ENDIF
.ELSE
	@ $(MMS)$(MMSQUALIFIERS)
.IFDEF NOSVG
	@ Write SYS$Output "--- Skipping gobjectlib build"
.ELSE
	@ Set Default [.gobject]
	@ $(MMS)$(MMSQUALIFIERS)
	@ Set Default [-]
.ENDIF
.ENDIF
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
        @ If F$Search("$(WDIR)*.obj") .NES. "" Then Delete $(WDIR)*.obj;*
	@ Set Default [-]

libintl :
	@ Write SYS$Output "--- Building libintl"
	@ Set Default [.libintl]
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

libart :
	@ Write SYS$Output "--- Building libart"
	@ Set Default [.libart]
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

libgdk-pixbuf :
.IFDEF NOSVG
	@ Write SYS$Output "--- Skipping libgdk-pixbuf build"
.ELSE
	@ Write SYS$Output "--- Building libgdk-pixbuf"
	@ Set Default [.libgdk-pixbuf]
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
.ENDIF

libexpat :
.IFDEF NOSVG
	@ Write SYS$Output "--- Skipping libexpat build"
.ELSE
	@ Write SYS$Output "--- Building libexpat"
	@ Set Default [.libexpat]
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
.ENDIF

libfontconfig :
.IFDEF NOSVG
	@ Write SYS$Output "--- Skipping libfontconfig build"
.ELSE
	@ Write SYS$Output "--- Building libfontconfig"
	@ Set Default [.libfontconfig]
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
.ENDIF

libfreetype :
.IFDEF NOSVG
	@ Write SYS$Output "--- Skipping libfreetype build"
.ELSE
	@ Write SYS$Output "--- Building libfreetype"
	@ Set Default [.libfreetype]
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
.ENDIF

libpango :
.IFDEF NOSVG
	@ Write SYS$Output "--- Skipping libpango build"
.ELSE
	@ Write SYS$Output "--- Building libpango"
	@ Set Default [.libpango]
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
.ENDIF

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
.IFDEF CLEAN
	@ Write SYS$Output "--- Cleaning tree"
.IFDEF NOWAIS
.ELSE
	@ Set Default [.freeWAIS-0_5.ir.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [---]
.ENDIF
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
	@ Set Default [.libopenjpeg.$(WORK)]
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
.IFDEF NOTIFF
.ELSE
	@ Set Default [.libtiff.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
.ENDIF
!
.IFDEF NOSVG
.ELSE
	@ Set Default [.librsvg.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
.ENDIF
!
	@ Set Default [.libcroco.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
!
	@ Set Default [.libxml2.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
!
	@ Set Default [.libintl.$(WORK)]
	@- Delete *.obj.*, *.olb.*, work.*.*
	@ Set Default [--]
!
	@ Set Default [.glib.$(WORK)]
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
.ENDIF
