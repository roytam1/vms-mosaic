! MMS description file for libnut (NCSA Unix Tools Library)
! George E. Cook, WVNET, 14-Feb-1996
! Mosaic 2.6-2
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libnut.olb

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
CQUALC=/DECC/Standard=VAXC/Prefix=ANSI/Define=(MULTINET)
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

OBJECTS =  Odir:ellipsis.obj Odir:mm.obj Odir:str-tools.obj Odir:system.obj \
           Odir:url-utils.obj
           

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
	@ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.IFDEF PATHWAY
	@ @[-.TWG]def
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
	@ Write SYS$Output "Library libnut.olb built."

Odir:ellipsis.obj  : ellipsis.c [-]config.h
Odir:mm.obj        : mm.c [-]config.h mm.h
Odir:str-tools.obj : str-tools.c [-]config.h str-tools.h
Odir:system.obj    : system.c [-]config.h system.h
Odir:url-utils.obj : url-utils.c [-]config.h

.c.obj
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	Delete/Log $(WDIR)*.OBJ;*
	Delete/Log $(LIBTARGET);*

