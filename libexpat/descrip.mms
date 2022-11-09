! MMS description file for libexpat
! George E. Cook, WVNET, 18-Aug-2008
! Mosaic 4.3
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libexpat.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.IFDEF DECC
CQUALC = /DECC/WARNING=(DISABLE=PTRMISMATCH)
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

OBJECTS = Odir:xmlparse.obj Odir:xmltok.obj Odir:xmlrole.obj
 
.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
        @ Define/NoLog Odir $(WDIR)
.IFDEF GNUC
        @ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
        @ Write SYS$Output "Library libexpat.olb built."

Odir:xmlparse.obj : xmlparse.c expat.h xmlrole.h xmltok.h expat_config.h
Odir:xmlrole.obj  : xmlrole.c ascii.h xmlrole.h expat_config.h
Odir:xmltok.obj   : xmltok.c xmltok_impl.c xmltok_ns.c \
        	    ascii.h asciitab.h iasciitab.h latin1tab.h \
		    nametab.h utf8tab.h xmltok.h xmltok_impl.h expat_config.h

.c.obj :
        $(CC)$(CFLAGS)$(CFLOAT)/OBJECT=$@ $<

.obj.olb
        $(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
        Delete/Log $(WDIR)*.OBJ;*
        Delete/Log $(LIBTARGET);*
