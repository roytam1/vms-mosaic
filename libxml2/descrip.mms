! MMS description file for libxml2
! George E. Cook, WVNET, 22-Nov-2007
! Mosaic 4.3
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libxml2.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.IFDEF DECC
CQUALC = /DECC/INCLUDE=[-.ZLIB]
.ELSE
.IFDEF DECCVAXC
CQUALC = /VAXC/INCLUDE=[-.ZLIB]
.ELSE
.IFDEF GNUC
CQUALC = /INCLUDE=(GCC_Include)
.ELSE
CQUALC = /INCLUDE=[-.ZLIB]
.ENDIF
.ENDIF
.ENDIF

.IFDEF ALPHA
CFLOAT = /FLOAT=IEEE/IEEE_MODE=DENORM_RESULTS
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

OBJECTS = Odir:c14n.obj Odir:catalog.obj Odir:docbparser.obj Odir:encoding.obj \
          Odir:entities.obj Odir:error.obj Odir:globals.obj Odir:hash.obj \
          Odir:htmlparser.obj Odir:htmltree.obj Odir:list.obj \
          Odir:nanoftp.obj Odir:nanohttp.obj Odir:parser.obj \
          Odir:parserinternals.obj Odir:sax.obj Odir:threads.obj \
          Odir:tree.obj Odir:trio.obj Odir:triostr.obj Odir:uri.obj \
          Odir:valid.obj Odir:xinclude.obj Odir:xlink.obj Odir:xmlio.obj \
          Odir:xmlmemory.obj Odir:xmlregexp.obj Odir:xmlschemas.obj \
          Odir:xmlschemastypes.obj Odir:xmlunicode.obj Odir:xpath.obj \
          Odir:xpointer.obj Odir:debugxml.obj

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
	@ Write SYS$Output "Library libxml2.olb built."

Odir:c14n.obj            : c14n.c c14n.h libxml2_config.h xmlversion.h
Odir:catalog.obj         : catalog.c catalog.h libxml2_config.h xmlversion.h
Odir:debugxml.obj        : debugxml.c debugxml.h libxml2_config.h xmlversion.h
Odir:docbparser.obj      : docbparser.c docbparser.h libxml2_config.h \
			   xmlversion.h
Odir:encoding.obj        : encoding.c encoding.h libxml2_config.h xmlversion.h
Odir:entities.obj        : entities.c entities.h libxml2_config.h xmlversion.h
Odir:error.obj           : error.c libxml2_config.h xmlversion.h
Odir:globals.obj         : globals.c globals.h libxml2_config.h xmlversion.h
Odir:hash.obj            : hash.c hash.h libxml2_config.h xmlversion.h
Odir:htmlparser.obj      : htmlparser.c htmlparser.h parserinternals.h \
			   libxml2_config.h xmlversion.h
Odir:htmltree.obj        : htmltree.c htmltree.h libxml2_config.h xmlversion.h
Odir:list.obj            : list.c list.h libxml2_config.h xmlversion.h
Odir:nanoftp.obj         : nanoftp.c nanoftp.h libxml2_config.h xmlversion.h
Odir:nanohttp.obj        : nanohttp.c nanohttp.h libxml2_config.h xmlversion.h
Odir:parser.obj          : parser.c parser.h libxml2_config.h xmlversion.h
Odir:parserinternals.obj : parserinternals.c parserinternals.h \
			   libxml2_config.h xmlversion.h
Odir:sax.obj             : sax.c sax.h libxml2_config.h xmlversion.h
Odir:threads.obj         : threads.c threads.h libxml2_config.h xmlversion.h
Odir:tree.obj            : tree.c tree.h libxml2_config.h xmlversion.h
Odir:trio.obj            : trio.c trio.h libxml2_config.h xmlversion.h
Odir:triostr.obj         : triostr.c triostr.h libxml2_config.h xmlversion.h
Odir:uri.obj             : uri.c uri.h libxml2_config.h xmlversion.h
Odir:valid.obj           : valid.c valid.h libxml2_config.h xmlversion.h 
Odir:xinclude.obj        : xinclude.c xinclude.h libxml2_config.h xmlversion.h
Odir:xlink.obj           : xlink.c xlink.h libxml2_config.h xmlversion.h
Odir:xmlio.obj           : xmlio.c xmlio.h parserinternals.h libxml2_config.h \
			   xmlversion.h
Odir:xmlmemory.obj       : xmlmemory.c xmlmemory.h libxml2_config.h xmlversion.h
Odir:xmlregexp.obj       : xmlregexp.c xmlregexp.h libxml2_config.h xmlversion.h
Odir:xmlschemas.obj      : xmlschemas.c xmlschemas.h libxml2_config.h \
			   xmlversion.h
Odir:xmlschemastypes.obj : xmlschemastypes.c xmlschemastypes.h \
			   libxml2_config.h xmlversion.h
Odir:xmlunicode.obj      : xmlunicode.c xmlunicode.h libxml2_config.h \
			   xmlversion.h
Odir:xpath.obj           : xpath.c xpath.h libxml2_config.h xmlversion.h
Odir:xpointer.obj        : xpointer.c xpointer.h libxml2_config.h xmlversion.h

.c.obj
	$(CC)$(CFLAGS)$(CFLOAT)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	Delete/Log $(WDIR)*.OBJ;*
	Delete/Log $(LIBTARGET);*

