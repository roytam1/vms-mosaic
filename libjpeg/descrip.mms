# Makefile for Independent JPEG Group's software
#
# This makefile is for use with MMS on Digital VMS systems.
# Thanks to Rick Dyson (dyson@iowasp.physics.uiowa.edu)
# and Tim Bell (tbell@netcom.com) for their help.
#
# Read installation instructions before saying "MMS" !!
#
# Generally, we recommend defining any configuration symbols in jconfig.h,
# NOT via /Define switches here.
#
# Mosaic version 2.7 13-Mar-1996, George Cook
# Only builds the library required for linking with Mosaic.
# This description file is intended to be invoked by the top level
# description file.  It should not be invoked directly.

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libjpeg.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.IFDEF DECC
CFLAGS= /DECC/NoDebug/Optimize
OPT=
.ELSE
.IFDEF DECCVAXC
CFLAGS= /VAXC/PRECISION=SINGLE/NoDebug/Optimize
.ELSE
.IFDEF GNUC
CFLAGS= /NoDebug/Optimize
.ELSE
CFLAGS= /PRECISION=SINGLE/NoDebug/Optimize
.ENDIF
.ENDIF
OPT= ,Sys$Disk:[]MAKVMS.OPT/Option
.ENDIF

# Put here the object file name for the correct system-dependent memory
# manager file.  For Unix this is usually jmemnobs.o, but you may want
# to use jmemansi.o or jmemname.o if you have limited swap space.
SYSDEPMEM= jmemnobs.obj

# End of configurable options.


# source files: JPEG library proper
LIBSOURCES= jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c \
        jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c \
        jcphuff.c jcprepct.c jcsample.c jctrans.c jdapimin.c jdapistd.c \
        jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c \
        jdinput.c jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdphuff.c \
        jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c jfdctfst.c \
        jfdctint.c jidctflt.c jidctfst.c jidctint.c jidctred.c jquant1.c \
        jquant2.c jutils.c jmemmgr.c jmemansi.c jmemname.c jmemnobs.c \
        jmemdos.c
# source files: cjpeg/djpeg/jpegtran applications, also rdjpgcom/wrjpgcom
APPSOURCES= cjpeg.c djpeg.c jpegtran.c cdjpeg.c rdcolmap.c rdswitch.c \
        rdjpgcom.c wrjpgcom.c rdppm.c wrppm.c rdgif.c wrgif.c rdtarga.c \
        wrtarga.c rdbmp.c wrbmp.c rdrle.c wrrle.c
SOURCES= $(LIBSOURCES) $(APPSOURCES)
# files included by source files
INCLUDES= jchuff.h jdhuff.h jdct.h jerror.h jinclude.h jmemsys.h jmorecfg.h \
        jpegint.h jpeglib.h jversion.h cdjpeg.h cderror.h
# documentation, test, and support files
DOCS= README install.doc usage.doc cjpeg.1 djpeg.1 jpegtran.1 rdjpgcom.1 \
        wrjpgcom.1 wizard.doc example.c libjpeg.doc structure.doc \
        coderules.doc filelist.doc change.log
MKFILES= configure makefile.cfg makefile.ansi makefile.unix makefile.bcc \
        makefile.mc6 makefile.dj makefile.wat makcjpeg.st makdjpeg.st \
        makljpeg.st maktjpeg.st makefile.manx makefile.sas makefile.mms \
        makefile.vms makvms.opt
CONFIGFILES= jconfig.cfg jconfig.manx jconfig.sas jconfig.st jconfig.bcc \
        jconfig.mc6 jconfig.dj jconfig.wat jconfig.vms
OTHERFILES= jconfig.doc ckconfig.c ansi2knr.c ansi2knr.1 jmemdosa.asm
TESTFILES= testorig.jpg testimg.ppm testimg.gif testimg.jpg testprog.jpg \
        testimgp.jpg
DISTFILES= $(DOCS) $(MKFILES) $(CONFIGFILES) $(SOURCES) $(INCLUDES) \
        $(OTHERFILES) $(TESTFILES)
# library object files common to compression and decompression
COMOBJECTS= Odir:jcomapi.obj Odir:jutils.obj Odir:jerror.obj Odir:jmemmgr.obj \
	Odir:$(SYSDEPMEM)
# compression library object files
CLIBOBJECTS= Odir:jcapimin.obj Odir:jcapistd.obj Odir:jctrans.obj \
	Odir:jcparam.obj Odir:jdatadst.obj Odir:jcinit.obj Odir:jcmaster.obj \
	Odir:jcmarker.obj Odir:jcmainct.obj Odir:jcprepct.obj \
	Odir:jccoefct.obj Odir:jccolor.obj Odir:jcsample.obj Odir:jchuff.obj \
	Odir:jcphuff.obj Odir:jcdctmgr.obj Odir:jfdctfst.obj Odir:jfdctflt.obj \
	Odir:jfdctint.obj
# decompression library object files
DLIBOBJECTS= Odir:jdapimin.obj Odir:jdapistd.obj Odir:jdtrans.obj \
	Odir:jdatasrc.obj Odir:jdmaster.obj Odir:jdinput.obj Odir:jdmarker.obj \
	Odir:jdhuff.obj Odir:jdphuff.obj Odir:jdmainct.obj Odir:jdcoefct.obj \
	Odir:jdpostct.obj Odir:jddctmgr.obj Odir:jidctfst.obj \
        Odir:jidctflt.obj Odir:jidctint.obj Odir:jidctred.obj \
	Odir:jdsample.obj Odir:jdcolor.obj Odir:jquant1.obj Odir:jquant2.obj \
	Odir:jdmerge.obj
# These objectfiles are included in libjpeg.olb
LIBOBJECTS= $(CLIBOBJECTS) $(DLIBOBJECTS) $(COMOBJECTS)
# object files for sample applications (excluding library files)
COBJECTS= cjpeg.obj rdppm.obj rdgif.obj rdtarga.obj rdrle.obj rdbmp.obj \
        rdswitch.obj cdjpeg.obj
DOBJECTS= djpeg.obj wrppm.obj wrgif.obj wrtarga.obj wrrle.obj wrbmp.obj \
        rdcolmap.obj cdjpeg.obj
TROBJECTS= jpegtran.obj rdswitch.obj cdjpeg.obj
# objectfile lists with commas --- what a crock
COBJLIST= cjpeg.obj,rdppm.obj,rdgif.obj,rdtarga.obj,rdrle.obj,rdbmp.obj,\
          rdswitch.obj,cdjpeg.obj
DOBJLIST= djpeg.obj,wrppm.obj,wrgif.obj,wrtarga.obj,wrrle.obj,wrbmp.obj,\
          rdcolmap.obj,cdjpeg.obj
TROBJLIST= jpegtran.obj,rdswitch.obj,cdjpeg.obj
LIBOBJLIST= Odir:jcapimin.obj,Odir:jcapistd.obj,Odir:jctrans.obj,Odir:jcparam.obj,Odir:jdatadst.obj,\
          Odir:jcinit.obj,Odir:jcmaster.obj,Odir:jcmarker.obj,Odir:jcmainct.obj,Odir:jcprepct.obj,\
          Odir:jccoefct.obj,Odir:jccolor.obj,Odir:jcsample.obj,Odir:jchuff.obj,Odir:jcphuff.obj,\
          Odir:jcdctmgr.obj,Odir:jfdctfst.obj,Odir:jfdctflt.obj,Odir:jfdctint.obj,Odir:jdapimin.obj,\
          Odir:jdapistd.obj,Odir:jdtrans.obj,Odir:jdatasrc.obj,Odir:jdmaster.obj,Odir:jdinput.obj,\
          Odir:jdmarker.obj,Odir:jdhuff.obj,Odir:jdphuff.obj,Odir:jdmainct.obj,Odir:jdcoefct.obj,\
          Odir:jdpostct.obj,Odir:jddctmgr.obj,Odir:jidctfst.obj,Odir:jidctflt.obj,Odir:jidctint.obj,\
          Odir:jidctred.obj,Odir:jdsample.obj,Odir:jdcolor.obj,Odir:jquant1.obj,Odir:jquant2.obj,\
          Odir:jdmerge.obj,Odir:jcomapi.obj,Odir:jutils.obj,Odir:jerror.obj,Odir:jmemmgr.obj,Odir:$(SYSDEPMEM)


.first
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
	@ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.IFDEF GNUC
	@- Define/NoLog Sys GNU_CC_Include
.ELSE
	@- Define/NoLog Sys SYS$Library
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(LIBOBJECTS))
	@ Write SYS$Output "Library libjpeg.olb built."

!ALL : libjpeg.olb cjpeg.exe djpeg.exe jpegtran.exe rdjpgcom.exe wrjpgcom.exe
!	@ Continue

!libjpeg.olb : $(LIBOBJECTS)
!	Library /Create libjpeg.olb $(LIBOBJLIST)

!cjpeg.exe : $(COBJECTS) libjpeg.olb
!	$(LINK) $(LFLAGS) /Executable = cjpeg.exe $(COBJLIST),libjpeg.olb/Library$(OPT)

!djpeg.exe : $(DOBJECTS) libjpeg.olb
!	$(LINK) $(LFLAGS) /Executable = djpeg.exe $(DOBJLIST),libjpeg.olb/Library$(OPT)

!jpegtran.exe : $(TROBJECTS) libjpeg.olb
!	$(LINK) $(LFLAGS) /Executable = jpegtran.exe $(TROBJLIST),libjpeg.olb/Library$(OPT)

!rdjpgcom.exe : rdjpgcom.obj
!	$(LINK) $(LFLAGS) /Executable = rdjpgcom.exe rdjpgcom.obj$(OPT)

!wrjpgcom.exe : wrjpgcom.obj
!	$(LINK) $(LFLAGS) /Executable = wrjpgcom.exe wrjpgcom.obj$(OPT)

jconfig.h : jconfig.vms
	@- Copy jconfig.vms jconfig.h

.c.obj
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	@- Set Protection = Owner:RWED *.*;-1
	@- Set Protection = Owner:RWED *.OBJ
	- Purge /NoLog /NoConfirm *.*
	- Delete /NoLog /NoConfirm *.OBJ;
	- Delete /NoLog /NoConfirm $(WDIR)*.OBJ;*

test : cjpeg.exe djpeg.exe jpegtran.exe
	mcr sys$disk:[]djpeg -dct int -ppm -outfile testout.ppm testorig.jpg
	mcr sys$disk:[]djpeg -dct int -gif -outfile testout.gif testorig.jpg
	mcr sys$disk:[]cjpeg -dct int      -outfile testout.jpg testimg.ppm
	mcr sys$disk:[]djpeg -dct int -ppm -outfile testoutp.ppm testprog.jpg
	mcr sys$disk:[]cjpeg -dct int -progressive -opt -outfile testoutp.jpg testimg.ppm
	mcr sys$disk:[]jpegtran -outfile testoutt.jpg testprog.jpg
	- Backup /Compare/Log	  testimg.ppm testout.ppm;
	- Backup /Compare/Log	  testimg.gif testout.gif;
	- Backup /Compare/Log	  testimg.jpg testout.jpg;
	- Backup /Compare/Log	  testimg.ppm testoutp.ppm;
	- Backup /Compare/Log	  testimgp.jpg testoutp.jpg;
	- Backup /Compare/Log	  testorig.jpg testoutt.jpg;


Odir:jcapimin.obj : jcapimin.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jcapistd.obj : jcapistd.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jccoefct.obj : jccoefct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jccolor.obj : jccolor.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jcdctmgr.obj : jcdctmgr.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
Odir:jchuff.obj : jchuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jchuff.h
Odir:jcinit.obj : jcinit.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jcmainct.obj : jcmainct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jcmarker.obj : jcmarker.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jcmaster.obj : jcmaster.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jcomapi.obj : jcomapi.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jcparam.obj : jcparam.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jcphuff.obj : jcphuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jchuff.h
Odir:jcprepct.obj : jcprepct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jcsample.obj : jcsample.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jctrans.obj : jctrans.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jdapimin.obj : jdapimin.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jdapistd.obj : jdapistd.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jdatadst.obj : jdatadst.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h
Odir:jdatasrc.obj : jdatasrc.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h
Odir:jdcoefct.obj : jdcoefct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jdcolor.obj : jdcolor.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jddctmgr.obj : jddctmgr.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
Odir:jdhuff.obj : jdhuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdhuff.h
Odir:jdinput.obj : jdinput.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jdmainct.obj : jdmainct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jdmarker.obj : jdmarker.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jdmaster.obj : jdmaster.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jdmerge.obj : jdmerge.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jdphuff.obj : jdphuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdhuff.h
Odir:jdpostct.obj : jdpostct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jdsample.obj : jdsample.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jdtrans.obj : jdtrans.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jerror.obj : jerror.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jversion.h jerror.h
Odir:jfdctflt.obj : jfdctflt.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
Odir:jfdctfst.obj : jfdctfst.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
Odir:jfdctint.obj : jfdctint.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
Odir:jidctflt.obj : jidctflt.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
Odir:jidctfst.obj : jidctfst.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
Odir:jidctint.obj : jidctint.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
Odir:jidctred.obj : jidctred.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
Odir:jquant1.obj : jquant1.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jquant2.obj : jquant2.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jutils.obj : jutils.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
Odir:jmemmgr.obj : jmemmgr.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
Odir:jmemansi.obj : jmemansi.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
Odir:jmemname.obj : jmemname.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
Odir:jmemnobs.obj : jmemnobs.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
Odir:jmemdos.obj : jmemdos.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
cjpeg.obj : cjpeg.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h jversion.h
djpeg.obj : djpeg.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h jversion.h
jpegtran.obj : jpegtran.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h jversion.h
cdjpeg.obj : cdjpeg.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
rdcolmap.obj : rdcolmap.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
rdswitch.obj : rdswitch.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
rdjpgcom.obj : rdjpgcom.c jinclude.h jconfig.h
wrjpgcom.obj : wrjpgcom.c jinclude.h jconfig.h
rdppm.obj : rdppm.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
wrppm.obj : wrppm.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
rdgif.obj : rdgif.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
wrgif.obj : wrgif.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
rdtarga.obj : rdtarga.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
wrtarga.obj : wrtarga.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
rdbmp.obj : rdbmp.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
wrbmp.obj : wrbmp.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
rdrle.obj : rdrle.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
wrrle.obj : wrrle.c cdjpeg.h jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h cderror.h
