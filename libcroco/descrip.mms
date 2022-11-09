! MMS description file for libcroco
! George E. Cook, WVNET, 21-Jan-2008
! Mosaic 4.3
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libcroco.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.IFDEF DECC
CQUALC = /DECC/INCLUDE=([-.GLIB],[-.LIBXML2])/WARNING=(DISABLE=PTRMISMATCH1)
.ELSE
.IFDEF DECCVAXC
CQUALC = /VAXC/INCLUDE=([-.GLIB],[-.LIBXML2])
.ELSE
.IFDEF GNUC
CQUALC = /INCLUDE=(GCC_Include)
.ELSE
CQUALC = /INCLUDE=([-.GLIB],[-.LIBXML2])
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

OBJECTS = Odir:cr-additional-sel.obj Odir:cr-attr-sel.obj Odir:cr-cascade.obj \
	  Odir:cr-declaration.obj Odir:cr-doc-handler.obj \
	  Odir:cr-enc-handler.obj Odir:cr-fonts.obj Odir:cr-input.obj \
	  Odir:cr-num.obj Odir:cr-om-parser.obj Odir:cr-parser.obj \
	  Odir:cr-parsing-location.obj Odir:cr-prop-list.obj \
	  Odir:cr-pseudo.obj Odir:cr-rgb.obj Odir:cr-sel-eng.obj \
	  Odir:cr-selector.obj Odir:cr-simple-sel.obj Odir:cr-statement.obj \
	  Odir:cr-string.obj Odir:cr-style.obj Odir:cr-stylesheet.obj \
	  Odir:cr-term.obj Odir:cr-tknzr.obj Odir:cr-token.obj \
	  Odir:cr-utils.obj

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
        @ Define/NoLog Odir $(WDIR)
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
	@ Write SYS$Output "Library libcroco.olb built."

Odir:cr-additional-sel.obj : cr-additional-sel.c cr-additional-sel.h \
			     libcroco-config.h
Odir:cr-attr-sel.obj       : cr-attr-sel.c cr-attr-sel.h libcroco-config.h
Odir:cr-cascade.obj        : cr-cascade.c cr-cascade.h libcroco-config.h
Odir:cr-declaration.obj    : cr-declaration.c cr-declaration.h libcroco-config.h
Odir:cr-doc-handler.obj    : cr-doc-handler.c cr-doc-handler.h libcroco-config.h
Odir:cr-enc-handler.obj    : cr-enc-handler.c cr-enc-handler.h libcroco-config.h
Odir:cr-input.obj          : cr-input.c cr-input.h libcroco-config.h
Odir:cr-fonts.obj          : cr-fonts.c cr-fonts.h libcroco-config.h
Odir:cr-num.obj            : cr-num.c cr-num.h libcroco-config.h
Odir:cr-om-parser.obj      : cr-om-parser.c cr-om-parser.h libcroco-config.h
Odir:cr-parser.obj         : cr-parser.c cr-parser.h libcroco-config.h
Odir:cr-parsing-location.obj : cr-parsing-location.c cr-parsing-location.h \
			     libcroco-config.h
Odir:cr-prop-list.obj      : cr-prop-list.c cr-prop-list.h libcroco-config.h
Odir:cr-pseudo.obj         : cr-pseudo.c cr-pseudo.h libcroco-config.h
Odir:cr-rgb.obj            : cr-rgb.c cr-rgb.h libcroco-config.h
Odir:cr-sel-eng.obj        : cr-sel-eng.c cr-sel-eng.h libcroco-config.h
Odir:cr-selector.obj       : cr-selector.c cr-selector.h libcroco-config.h
Odir:cr-simple-sel.obj     : cr-simple-sel.c cr-simple-sel.h libcroco-config.h
Odir:cr-statement.obj      : cr-statement.c cr-statement.h libcroco-config.h
Odir:cr-string.obj         : cr-string.c cr-string.h libcroco-config.h
Odir:cr-style.obj          : cr-style.c cr-style.h libcroco-config.h
Odir:cr-stylesheet.obj     : cr-stylesheet.c cr-stylesheet.h libcroco-config.h
Odir:cr-term.obj           : cr-term.c cr-term.h libcroco-config.h
Odir:cr-tknzr.obj          : cr-tknzr.c cr-tknzr.h libcroco-config.h
Odir:cr-token.obj          : cr-token.c cr-token.h libcroco-config.h
Odir:cr-utils.obj          : cr-utils.c cr-utils.h libcroco-config.h

.c.obj :
        $(CC)$(CFLAGS)$(CFLOAT)/OBJECT=$@ $<

.obj.olb
        $(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
        Delete/Log $(WDIR)*.OBJ;*
        Delete/Log $(LIBTARGET);*
