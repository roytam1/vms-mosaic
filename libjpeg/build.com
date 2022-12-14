$! Makefile for Independent JPEG Group's software
$!
$! This is a command procedure for Digital VMS systems that do not have MMS.
$! It builds the JPEG software by brute force, recompiling everything whether
$! or not it is necessary.  It then runs the basic self-test.
$! Thanks to Rick Dyson (dyson@iowasp.physics.uiowa.edu)
$! and Tim Bell (tbell@netcom.com) for their help.
$!
$! Modified for VMS Mosaic 2.6 build, 5-Nov-1995, George Cook
$! Modified for VMS Mosaic 2.7 GNU C build, 10-Sep-1996, George Cook
$! Modified for VMS Mosaic 4.3 build, 17-Nove-2007, George Cook
$!
$ OPT = ""
$ If (F$Locate("DECC=1", P1) .NE. F$Length(P1))
$  Then DoCompile := CC/DECC/NoDebug/Optimize
$  Endif
$ If (F$Locate("DECCVAXC=1", P1) .NE. F$Length(P1))
$  Then
$   DoCompile := CC/VAXC/PRECISION=SINGLE/NoDebug/Optimize
$   OPT = ",Sys$Disk:[]makvms.opt/Option"
$  Endif
$ If (F$Locate("VAXC=2", P1) .NE. F$Length(P1))
$  Then
$   DoCompile := CC/PRECISION=SINGLE/NoDebug/Optimize
$   OPT = ",Sys$Disk:[]makvms.opt/Option"
$  Endif
$ If (F$Locate("GNUC", P1) .NE. F$Length(P1))
$  Then
$    DoCompile := GCC/NoDebug/Optimize/Names=Upper
$    OPT = ",GNU_CC:[000000]gcclib.olb/lib,Sys$Disk:[]makvms.opt/Option"
$  EndIf
$!
$ If (F$Locate("ALPHA=1", P1) .NE. F$Length(P1))
$  Then DoCompile = DoCompile + "/FLOAT=IEEE"
$  Endif
$ If (F$Locate("VAX=1", P1) .NE. F$Length(P1))
$  Then DoCompile = DoCompile + "/G_FLOAT"
$  Endif
$!
$ DoCompile jcapimin.c
$ DoCompile jcapistd.c
$ DoCompile jctrans.c
$ DoCompile jcparam.c
$ DoCompile jdatadst.c
$ DoCompile jcinit.c
$ DoCompile jcmaster.c
$ DoCompile jcmarker.c
$ DoCompile jcmainct.c
$ DoCompile jcprepct.c
$ DoCompile jccoefct.c
$ DoCompile jccolor.c
$ DoCompile jcsample.c
$ DoCompile jchuff.c
$ DoCompile jcphuff.c
$ DoCompile jcdctmgr.c
$ DoCompile jfdctfst.c
$ DoCompile jfdctflt.c
$ DoCompile jfdctint.c
$ DoCompile jdapimin.c
$ DoCompile jdapistd.c
$ DoCompile jdtrans.c
$ DoCompile jdatasrc.c
$ DoCompile jdmaster.c
$ DoCompile jdinput.c
$ DoCompile jdmarker.c
$ DoCompile jdhuff.c
$ DoCompile jdphuff.c
$ DoCompile jdmainct.c
$ DoCompile jdcoefct.c
$ DoCompile jdpostct.c
$ DoCompile jddctmgr.c
$ DoCompile jidctfst.c
$ DoCompile jidctflt.c
$ DoCompile jidctint.c
$ DoCompile jidctred.c
$ DoCompile jdsample.c
$ DoCompile jdcolor.c
$ DoCompile jquant1.c
$ DoCompile jquant2.c
$ DoCompile jdmerge.c
$ DoCompile jcomapi.c
$ DoCompile jutils.c
$ DoCompile jerror.c
$ DoCompile jmemmgr.c
$ DoCompile jmemnobs.c
$!
$ Library/Create libjpeg.olb  jcapimin.obj,jcapistd.obj,jctrans.obj, -
          jcparam.obj,jdatadst.obj,jcinit.obj,jcmaster.obj,jcmarker.obj, -
          jcmainct.obj,jcprepct.obj,jccoefct.obj,jccolor.obj,jcsample.obj, -
          jchuff.obj,jcphuff.obj,jcdctmgr.obj,jfdctfst.obj,jfdctflt.obj, -
          jfdctint.obj,jdapimin.obj,jdapistd.obj,jdtrans.obj,jdatasrc.obj, -
          jdmaster.obj,jdinput.obj,jdmarker.obj,jdhuff.obj,jdphuff.obj, -
          jdmainct.obj,jdcoefct.obj,jdpostct.obj,jddctmgr.obj,jidctfst.obj, -
          jidctflt.obj,jidctint.obj,jidctred.obj,jdsample.obj,jdcolor.obj, -
          jquant1.obj,jquant2.obj,jdmerge.obj,jcomapi.obj,jutils.obj, -
          jerror.obj,jmemmgr.obj,jmemnobs.obj
$!
$! Skip self-test since it causes too many problems for Mosaic builds.
$! Also skip building the executables since Mosaic doesn't need them.
$!
$ Exit
$!
$ DoCompile cjpeg.c
$ DoCompile rdppm.c
$ DoCompile rdgif.c
$ DoCompile rdtarga.c
$ DoCompile rdrle.c
$ DoCompile rdbmp.c
$ DoCompile rdswitch.c
$ DoCompile cdjpeg.c
$!
$ Link/Executable=cjpeg.exe  cjpeg.obj,rdppm.obj,rdgif.obj,rdtarga.obj, -
          rdrle.obj,rdbmp.obj,rdswitch.obj,cdjpeg.obj,libjpeg.olb/Library'OPT'
$!
$ DoCompile djpeg.c
$ DoCompile wrppm.c
$ DoCompile wrgif.c
$ DoCompile wrtarga.c
$ DoCompile wrrle.c
$ DoCompile wrbmp.c
$ DoCompile rdcolmap.c
$ DoCompile cdjpeg.c
$!
$ Link/Executable=djpeg.exe  djpeg.obj,wrppm.obj,wrgif.obj,wrtarga.obj, -
          wrrle.obj,wrbmp.obj,rdcolmap.obj,cdjpeg.obj,libjpeg.olb/Library'OPT'
$!
$ DoCompile jpegtran.c
$ DoCompile rdswitch.c
$ DoCompile cdjpeg.c
$!
$ Link/Executable=jpegtran.exe  jpegtran.obj,rdswitch.obj,cdjpeg.obj, -
	  libjpeg.olb/Library'OPT'
$!
$ DoCompile rdjpgcom.c
$ Link/Executable=rdjpgcom.exe  rdjpgcom.obj'OPT'
$!
$ DoCompile wrjpgcom.c
$ Link/Executable=wrjpgcom.exe  wrjpgcom.obj'OPT'
$!
$! Run the self-test
$!
$ mcr sys$disk:[]djpeg -dct int -ppm -outfile testout.ppm testorig.jpg
$ mcr sys$disk:[]djpeg -dct int -gif -outfile testout.gif testorig.jpg
$ mcr sys$disk:[]cjpeg -dct int      -outfile testout.jpg testimg.ppm
$ mcr sys$disk:[]djpeg -dct int -ppm -outfile testoutp.ppm testprog.jpg
$ mcr sys$disk:[]cjpeg -dct int -progressive -opt -outfile testoutp.jpg testimg.ppm
$ mcr sys$disk:[]jpegtran -outfile testoutt.jpg testprog.jpg
$ Backup/Compare/Log testimg.ppm testout.ppm;
$ Backup/Compare/Log testimg.gif testout.gif;
$ Backup/Compare/Log testimg.jpg testout.jpg;
$ Backup/Compare/Log testimg.ppm testoutp.ppm;
$ Backup/Compare/Log testimgp.jpg testoutp.jpg;
$ Backup/Compare/Log testorig.jpg testoutt.jpg;
$!
$ Exit
