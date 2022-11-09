! MMS description file for libfontconfig
! George E. Cook, WVNET, 18-Aug-2008
! Mosaic 4.3
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!
#*****************************************************************************
#                                                                            *
# Make file for VMS                                                          *
# Author : J.Jansen (joukj@hrem.stm.tudelft.nl)                              *
# Date : 12 May 2005                                                         *
#                                                                            *
#*****************************************************************************

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libfontconfig.olb

.IFDEF GNUC
CC = GCC
CINCLUDE =
.ELSE
CC = CC
CINCLUDE = /INCLUDE=([-.LIBEXPAT],[-.LIBFREETYPE])
.ENDIF

.IFDEF DECC
CQUALC = /DECC
.IFDEF VAX
NOVAXOPT = /NoOpt
.ENDIF
.ELSE
.IFDEF DECCVAXC
CQUALC = /VAXC
.ELSE
.IFDEF GNUC
CQUALC = /INCLUDE=(GCC_Include)
.ELSE
CQUALC =
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

OBJECTS = Odir:fcatomic.obj Odir:fcblanks.obj Odir:fccache.obj Odir:fccfg.obj \
	  Odir:fccharset.obj Odir:fcdbg.obj Odir:fcdefault.obj Odir:fcdir.obj \
	  Odir:fcfreetype.obj Odir:fcfs.obj Odir:fcinit.obj Odir:fclang.obj \
	  Odir:fclist.obj Odir:fcmatch.obj Odir:fcmatrix.obj Odir:fcname.obj \
	  Odir:fcpat.obj Odir:fcstr.obj Odir:fcxml.obj Odir:ftglue.obj

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
        @ Define/NoLog Odir $(WDIR)
	@ Topdir = F$Environment("Default")-"LIBFONTCONFIG]"
        @ Define/NoLog FREETYPE 'Topdir'LIBFREETYPE]
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
.IFDEF GNUC
        @ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
        @ Write SYS$Output "Library libfontconfig.olb built."
 
Odir:fcatomic.obj : fcatomic.c fcint.h fontconfig_config.h
Odir:fcblanks.obj : fcblanks.c fcint.h fontconfig_config.h
Odir:fccfg.obj : fccfg.c fcint.h fontconfig_config.h
Odir:fccharset.obj : fccharset.c fcint.h fontconfig_config.h
Odir:fcdbg.obj : fcdbg.c fcint.h fontconfig_config.h
Odir:fcdefault.obj : fcdefault.c fcint.h fontconfig_config.h
Odir:fcdir.obj : fcdir.c fcint.h fontconfig_config.h
Odir:fcfreetype.obj : fcfreetype.c fcint.h fontconfig_config.h
Odir:fcfs.obj : fcfs.c fcint.h fontconfig_config.h
Odir:fcinit.obj : fcinit.c fcint.h fontconfig_config.h
Odir:fclang.obj : fclang.c fclang.h fcint.h fontconfig_config.h
Odir:fclist.obj : fclist.c fcint.h fontconfig_config.h
Odir:fcmatch.obj : fcmatch.c fcint.h fontconfig_config.h
Odir:fcmatrix.obj : fcmatrix.c fcint.h fontconfig_config.h
Odir:fcname.obj : fcname.c fcint.h fontconfig_config.h
Odir:fcpat.obj : fcpat.c fcint.h fontconfig_config.h
Odir:fcstr.obj : fcstr.c fcint.h fontconfig_config.h
Odir:fcxml.obj : fcxml.c fcint.h fontconfig_config.h
Odir:ftglue.obj : ftglue.c fcint.h fontconfig_config.h

# Prevent hang in VAX DEC C compiler
Odir:fccache.obj : fccache.c fontconfig_config.h
        $(CC)$(CFLAGS)$(CFLOAT)$(CINCLUDE)$(NOVAXOPT)/OBJECT=$@ $<

.c.obj :
        $(CC)$(CFLAGS)$(CFLOAT)$(CINCLUDE)/OBJECT=$@ $<

.obj.olb
        $(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
        Delete/Log $(WDIR)*.OBJ;*
        Delete/Log $(LIBTARGET);*
