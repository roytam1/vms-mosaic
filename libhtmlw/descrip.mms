! MMS description file for LIBHTMLW
! Bjorn S. Nilsson, Aleph, CERN, 20-Nov-1993
! (Mosaic version 2.0)
! Motif 1.2 support added on 3-Jun-1994
! Mosaic 2.4 20-Aug-1994
! Mosaic version 2.6 1-Nov-1995, George Cook
!
! Copyright (C) 2005, 2006, 2007 - The VMS Mosaic Project
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libhtmlw.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.IFDEF PATHWAY
.IFDEF DECC
.INCLUDE [-.TWG]DECC_PREFIX_RULES.MMS
CQUALC=/DECC/Precision=SINGLE $(CC_PREFIX_NO_SIN)
.ELSE
.IFDEF DECCVAXC
CQUALC=/VAXC/Precision=SINGLE
.ELSE
CQUALC=/Precision=SINGLE
.ENDIF
.ENDIF

.ELSE ! Not PATHWAY

.IFDEF SOCKETSHR
.IFDEF DECC
CQUALC=/DECC/Precision=SINGLE/PREFIX=ALL
.ELSE
.IFDEF DECCVAXC
CQUALC=/VAXC/Precision=SINGLE
.ELSE
CQUALC=/Precision=SINGLE
.ENDIF
.ENDIF

.ELSE ! Not SOCKETSHR

.IFDEF MULTINET
.IFDEF DECC
CQUALC=/DECC/Precision=SINGLE/Prefix=ANSI
.ELSE
.IFDEF DECCVAXC
CQUALC=/VAXC/Precision=SINGLE
.ELSE
CQUALC=/Precision=SINGLE
.ENDIF
.ENDIF

.ELSE ! Not MultiNet

!UCX
.IFDEF DECC
CQUALC=/DECC/Precision=SINGLE/Prefix=All
.ELSE
.IFDEF DECCVAXC
CQUALC=/VAXC/Precision=SINGLE
.ELSE
.IFDEF GNUC
CQUALC=
.ELSE
CQUALC=/Precision=SINGLE
.ENDIF
.ENDIF
.ENDIF
.ENDIF
.ENDIF

.ENDIF ! BGT

.IFDEF DEBUG
CFLAGS = $(CQUALC)/NoOpt/Debug
.ELSE
CFLAGS = $(CQUALC)
.ENDIF

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
	@ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.IFDEF PATHWAY
	@ @[-.twg]def
.ELSE
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
.ENDIF

OBJECTS = Odir:HTML-PSformat.obj Odir:HTML.obj Odir:HTMLapplet.obj \
 Odir:HTMLform.obj Odir:HTMLframe.obj Odir:HTMLformat.obj \
 Odir:HTMLimages.obj Odir:HTMLlists.obj Odir:HTMLparse.obj Odir:HTMLfont.obj \
 Odir:HTMLtable.obj Odir:HTMLtext.obj Odir:HTMLwidgets.obj Odir:list.obj \
 Odir:HTMLimagemap.obj

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
	@ Write SYS$Output "Library libhtmlw.olb built."

Odir:HTML-PSformat.obj : HTML-PSformat.c [-]config.h htmlwidgets.h htmlp.h \
			 htmlputil.h html.h
Odir:HTML.obj          : HTML.c [-]config.h [-]config_$(WORK).h htmlp.h html.h \
                         htmlparse.h htmlfont.h htmlframe.h htmlputil.h \
			 htmlwidgets.h [-.src]prefs.h [-.libnut]str-tools.h \
			 [-.libnut]system.h [-.libwww2]htbtree.h
Odir:HTMLapplet.obj    : HTMLapplet.c [-]config.h htmlp.h html.h htmlPutil.h \
			 htmlparse.h
Odir:HTMLfont.obj      : HTMLfont.c [-]config.h htmlp.h html.h htmlPutil.h \
			 htmlfont.h htmlparse.h [-.libxmx]xmx.h
Odir:HTMLform.obj      : HTMLform.c [-]config.h htmlp.h html.h htmlPutil.h \
			 htmlform.h htmlparse.h [-.src]mosaic.h
Odir:HTMLframe.obj     : HTMLframe.c [-]config.h htmlp.h html.h htmlPutil.h \
			 htmlframe.h htmlparse.h
Odir:HTMLformat.obj    : HTMLformat.c [-]config.h htmlp.h html.h htmlparse.h
Odir:HTMLimages.obj    : HTMLimages.c [-]config.h htmlp.h html.h noimage.xbm \
                         htmlparse.h htmlputil.h htmlfont.h \
			 [-.src]img.h [-.src]fsdither.h [-.src]mo-www.h \
			 [-.src]mosaic.h [-.src]prefs.h [-.src]prefs_defs.h \
			 delayedimage.xbm anchoreddelayedimage.xbm noimage.xbm
Odir:HTMLimagemap.obj  : HTMLimagemap.c [-]config.h htmlp.h html.h htmlputil.h
Odir:HTMLlists.obj     : HTMLlists.c [-]config.h html.h htmlp.h htmlmiscdefs.h \
			 [-.src]prefs.h [-.src]prefs_defs.h
Odir:HTMLparse.obj     : HTMLparse.c [-]config.h htmlp.h html.h htmlparse.h \
			 htmlputil.h [-.libwww2]htbtree.h
Odir:HTMLtable.obj     : HTMLtable.c [-]config.h htmlp.h html.h htmlparse.h
Odir:HTMLtext.obj      : HTMLtext.c [-]config.h htmlp.h html.h htmlparse.h \
			 htmlmiscdefs.h htmlputil.h list.h
Odir:HTMLwidgets.obj   : HTMLwidgets.c [-]config.h [-]config_$(WORK).h \
			 htmlwidgets.h htmlparse.h [-.libnut]str-tools.h \
			 htmlp.h html.h
Odir:list.obj          : list.c listp.h list.h [-]config.h

.c.obj :
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	Delete/Log $(WDIR)*.OBJ;*
	Delete/Log $(LIBTARGET);*

