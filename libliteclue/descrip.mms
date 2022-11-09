! MMS description file for LIBLITECLUE
! George Cook, 22-Feb-2006
! (Mosaic version 4.0)
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libliteclue.olb

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

OBJECTS = Odir:liteclue.obj

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
	@ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.IFDEF PATHWAY
	@ @[-.TWG]def
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
	@ Write SYS$Output "Library libliteclue.olb built."

Odir:liteclue.obj  : liteclue.c [-]config.h [-]config_$(WORK).h liteclue.h \
		     litecluep.h

.c.obj :
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	Delete/Log $(WDIR)*.OBJ;*
	Delete/Log $(LIBTARGET);*

