! MMS description file for libopenjpeg
! George E. Cook, WVNET, 29-May-2007
! Mosaic 4.2
! libopenjpeg 1.2 on 19-Jun-2007
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libopenjpeg.olb

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

OBJECTS = Odir:bio.obj Odir:cio.obj Odir:dwt.obj Odir:event.obj \
          Odir:image.obj Odir:j2k.obj Odir:j2k_lib.obj Odir:jp2.obj \
	  Odir:jpt.obj Odir:mct.obj Odir:mqc.obj Odir:openjpeg.obj Odir:pi.obj \
	  Odir:raw.obj Odir:t1.obj Odir:t2.obj Odir:tcd.obj Odir:tgt.obj

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
	@ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.IFDEF PATHWAY
	@ @[-.TWG]def
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
	@ Write SYS$Output "Library libopenjpeg.olb built."

Odir:bio.obj      : bio.c bio.h opj_includes.h openjpeg.h
Odir:cio.obj      : cio.c cio.h opj_includes.h openjpeg.h
Odir:dwt.obj      : dwt.c dwt.h opj_includes.h openjpeg.h fix.h
Odir:event.obj    : event.c event.h opj_includes.h openjpeg.h
Odir:image.obj    : image.c image.h opj_includes.h openjpeg.h
Odir:j2k.obj      : j2k.c j2k.h opj_includes.h openjpeg.h
Odir:j2k_lib.obj  : j2k_lib.c j2k_lib.h opj_includes.h openjpeg.h
Odir:jp2.obj      : jp2.c jp2.h opj_includes.h openjpeg.h
Odir:jpt.obj      : jpt.c jpt.h opj_includes.h openjpeg.h
Odir:mct.obj      : mct.c mct.h opj_includes.h openjpeg.h fix.h
Odir:mqc.obj      : mqc.c mqc.h opj_includes.h openjpeg.h
Odir:openjpeg.obj : openjpeg.c  opj_includes.h openjpeg.h
Odir:pi.obj       : pi.c pi.h opj_includes.h openjpeg.h
Odir:raw.obj      : raw.c raw.h opj_includes.h openjpeg.h
Odir:t1.obj       : t1.c t1.h t1_luts.h opj_includes.h openjpeg.h fix.h
Odir:t2.obj       : t2.c t2.h opj_includes.h openjpeg.h
Odir:tcd.obj      : tcd.c tcd.h opj_includes.h openjpeg.h
Odir:tgt.obj      : tgt.c tgt.h opj_includes.h openjpeg.h

.c.obj
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	Delete/Log $(WDIR)*.OBJ;*
	Delete/Log $(LIBTARGET);*

