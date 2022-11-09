! MMS description file for Mosaic
! Bjorn S. Nilsson, Aleph, CERN, 21-Nov-1993
! (Mosaic version 2.0)
! Motif 1.2 support added on 3-Jun-1994
! Mosaic 2.4 on 20-Aug-1994
! Mosaic 2.6 on 21-Oct-1995 by George Cook
! OpenSSL support on 22-Aug-1999 by George Cook
! HP SSL support on 27-Nov-2003 by George Cook
! DXm library added on 8-Feb-2006 by George Cook
! Tiff support added on 5-Jul-2006 by George Cook
!
! Copyright (C) 2003, 2005, 2006, 2007 - The VMS Mosaic Project
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)SRC.OLB

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

NAME = MOSAIC

.IFDEF NOWAIS
LIBWAISQ = 
.ELSE
LIBWAIS = [-.FREEWAIS-0_5.IR.$(WORK)]LIBWAIS.OLB
LIBWAISQ = $(LIBWAIS)/Lib
.ENDIF

.IFDEF NOSSL
LIBCRYPTO =
LIBSSL =
.ELSE
.IFDEF HPSSL
LIBSSL = SYS$SHARE:SSL$LIBSSL_SHR32/SHARE
LIBCRYPTO = SYS$SHARE:SSL$LIBCRYPTO_SHR32/SHARE
.ELSE
LIBSSL = SSLLIB:LIBSSL.OLB/Lib
LIBCRYPTO = SSLLIB:LIBCRYPTO.OLB/Lib
.ENDIF
.ENDIF

.IFDEF DECC
VAXC_RTL = 
.IFDEF PATHWAY
.INCLUDE [-.TWG]DECC_PREFIX_RULES.MMS		! Get our rules
CQUALC=/DECC/Standard=VAXC $(CC_PREFIX_NO_SIN)
.ELSE
.IFDEF MULTINET
CQUALC=/DECC/Standard=VAXC/Prefix=ANSI
.ELSE
.IFDEF SOCKETSHR
CQUALC=/DECC/Standard=VAXC/Prefix=(All,Except=(Getpwuid,Ioctl))
.ELSE
CQUALC=/DECC/Prefix=(All,Except=(Getpwuid,Ioctl))
.ENDIF
.ENDIF
.ENDIF
.ELSE	! VAX C
VAXC_RTL = SYS$Library:VaxCRTL.Exe/Share
.IFDEF DECCVAXC
CQUALC=/VAXC
.ELSE
CQUALC=
.ENDIF
.ENDIF

.IFDEF CMU
.IFDEF DECC
VAXC_INET_LIB = 
.ELSE
VAXC_INET_LIB = [-.libvms]UCX$IPC.Olb/Lib
.ENDIF
.ENDIF

.IFDEF MULTINET
VAXC_INET_LIB = Multinet_Socket_Library/Share
.ENDIF

.IFDEF MULTINET_UCX
.IFDEF DECC
VAXC_INET_LIB = 
.ELSE
VAXC_INET_LIB = Multinet_root:[multinet.library]UCX$IPC.Olb/Lib
.ENDIF
.ENDIF

.IFDEF PATHWAY
VAXC_INET_LIB = SYS$Library:TWGLib/Share
.ENDIF

.IFDEF PATHWAY_UCX
.IFDEF DECC
VAXC_INET_LIB = 
.ELSE
VAXC_INET_LIB = TWG$ETC:[000000]UW$IPC.Olb/Lib
.ENDIF
.ENDIF

.IFDEF SOCKETSHR
VAXC_INET_LIB = Socketshr/Share
.ENDIF

.IFDEF TCPWARE
.IFDEF DECC
VAXC_INET_LIB = 
.ELSE
VAXC_INET_LIB = TCPWARE:UCX$IPC.Olb/Lib
.ENDIF
.ENDIF

.IFDEF UCX
.IFDEF DECC
VAXC_INET_LIB = 
.ELSE
VAXC_INET_LIB = SYS$Library:UCX$IPC.Olb/Lib
.ENDIF
.ENDIF

.IFDEF DEBUG
CFLAGS =$(CQUALC)/NoOpt/Debug
LOPTIONS =/Debug
.ELSE
CFLAGS =$(CQUALC)
.IFDEF TRACE
LOPTIONS =/NoDebug/Trace
.ELSE
LOPTIONS =/NoDebug/NoTrace
.ENDIF
.ENDIF

.IFDEF MOTIF1_2
LIBS = $(OPTION_FILE)/Opt
DXM_LIB = SYS$Library:DECW$DXmLibShr12.Exe/Share
XMU_LIB = SYS$Library:DECW$XmuLibShrR5.Exe/Share
XM_LIB = SYS$Library:DECW$XmLibShr12.Exe/Share
XT_LIB = SYS$Library:DECW$XtLibShrR5.Exe/Share
.ELSE
LIBS = $(OPTION_FILE)/Opt
DXM_LIB = SYS$Library:DECW$DXmLibShr.Exe/Share
XMU_LIB = SYS$Library:DECW$XmuLibShr.Exe/Share
XM_LIB = SYS$Library:DECW$XmLibShr.Exe/Share
XT_LIB = SYS$Library:DECW$XtShr.Exe/Share
.ENDIF
X_LIB = SYS$Library:DECW$XLibShr.Exe/Share
XEXT_LIB = SYS$Library:DECW$XExtLibShr.Exe/Share

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
.IFDEF GNUC
	@ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.ELSE
	@ Topdir = F$Environment("Default")-"SRC]"
	@ Define LIBWWW2     'Topdir'LIBWWW2]
	@ Define LIBXMX      'Topdir'LIBXMX]
	@ Define LIBHTMLW    'Topdir'LIBHTMLW]
	@ Define LIBJPEG     'Topdir'LIBJPEG]
	@ Define LIBOPENJPEG 'Topdir'LIBOPENJPEG]
	@ Define LIBNUT      'Topdir'LIBNUT]
	@ Define LIBPNG      'Topdir'LIBPNG]
	@ Define ZLIB        'Topdir'ZLIB]
	@ Define LIBLITECLUE 'Topdir'LIBLITECLUE]
	@ Define LIBVMS      'Topdir'LIBVMS]
	@ Define LIBTIFF     'Topdir'LIBTIFF]
.ENDIF
.IFDEF PATHWAY
	@ @[-.TWG]DEF
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
.IFDEF DECC
.IFDEF PATHWAY
	@ Define DECC$User_Include 'F$Environment("Default")',LIBWWW2, -
		LIBHTMLW,LIBXMX,LIBTIFF,LIBJPEG,LIBOPENJPEG,LIBPNG,ZLIB, -
		LIBLITECLUE
.ELSE
	@ Define DECC$User_Include 'F$Environment("Default")',LIBWWW2, -
		LIBHTMLW,LIBXMX,LIBTIFF,LIBJPEG,LIBOPENJPEG,LIBPNG,ZLIB, -
		LIBLITECLUE,SYS
	@ Define DECC$SYSTEM_Include 'F$Environment("Default")',LIBWWW2, -
		LIBHTMLW,LIBXMX,LIBTIFF,LIBJPEG,LIBOPENJPEG,LIBPNG,ZLIB, -
		LIBLITECLUE,SYS
.ENDIF
.ELSE	! VAX C or GNU C compilation
.IFDEF PATHWAY
	@ Define C$Include 'F$Environment("Default")',LIBWWW2,LIBHTMLW, -
		LIBXMX,LIBTIFF,LIBJPEG,LIBOPENJPEG,LIBPNG,LIBLITECLUE,ZLIB
.ELSE
.IFDEF VAXC
	@ Define C$Include 'F$Environment("Default")',LIBWWW2,LIBHTMLW, -
		LIBXMX,LIBTIFF,LIBJPEG,LIBOPENJPEG,LIBPNG,ZLIB,LIBLITECLUE,SYS
	@ Define VAXC$Include 'F$Environment("Default")',LIBWWW2,LIBHTMLW, -
		LIBXMX,LIBTIFF,LIBJPEG,LIBOPENJPEG,LIBPNG,ZLIB,LIBLITECLUE,SYS
.ENDIF
.ENDIF
.ENDIF

.IFDEF NOTIFF
PROG_LIBS = [-.LIBHTMLW.$(WORK)]LIBHTMLW.OLB [-.LIBXMX.$(WORK)]LIBXMX.OLB \
  [-.LIBWWW2.$(WORK)]LIBWWW.OLB [-.LIBJPEG.$(WORK)]LIBJPEG.OLB \
  [-.LIBOPENJPEG.$(WORK)]LIBOPENJPEG.OLB [-.LIBNUT.$(WORK)]LIBNUT.OLB \
  [-.LIBPNG.$(WORK)]LIBPNG.OLB [-.ZLIB.$(WORK)]LIBZ.OLB \
  [-.LIBLITECLUE.$(WORK)]LIBLITECLUE.OLB [-.LIBVMS.$(WORK)]LIBVMS.OLB $(LIBWAIS)
.ELSE
PROG_LIBS = [-.LIBHTMLW.$(WORK)]LIBHTMLW.OLB [-.LIBXMX.$(WORK)]LIBXMX.OLB \
  [-.LIBWWW2.$(WORK)]LIBWWW.OLB [-.LIBJPEG.$(WORK)]LIBJPEG.OLB \
  [-.LIBOPENJPEG.$(WORK)]LIBOPENJPEG.OLB [-.LIBNUT.$(WORK)]LIBNUT.OLB \
  [-.LIBPNG.$(WORK)]LIBPNG.OLB [-.ZLIB.$(WORK)]LIBZ.OLB \
  [-.LIBLITECLUE.$(WORK)]LIBLITECLUE.OLB [-.LIBVMS.$(WORK)]LIBVMS.OLB \
  [-.LIBTIFF.$(WORK)]LIBTIFF.OLB $(LIBWAIS)
.ENDIF 

OBJS = Odir:accept.obj,Odir:annotate.obj,Odir:audan.obj,Odir:bla.obj,\
 Odir:ccibindings.obj,Odir:ccibindings2.obj,Odir:cciserver.obj,\
 Odir:child.obj,Odir:colors.obj,Odir:comment.obj,Odir:fsdither.obj,\
 Odir:gifread.obj,Odir:globalhist.obj,\
 Odir:grpan-www.obj,Odir:grpan.obj,Odir:gui-dialogs.obj,Odir:gui-documents.obj,\
 Odir:gui-extras.obj,Odir:gui-ftp.obj,Odir:gui-menubar.obj,Odir:gui-news.obj,\
 Odir:gui-popup.obj,Odir:gui.obj,Odir:history.obj,Odir:hotfile.obj,\
 Odir:hotlist.obj,Odir:img.obj,Odir:libtarga.obj,Odir:mailto.obj,\
 Odir:main.obj,Odir:md5.obj,Odir:medcut.obj,Odir:mo-www.obj,\
 Odir:newsrc.obj,Odir:pan.obj,Odir:picread.obj,Odir:pixmaps.obj,\
 Odir:prefs.obj,Odir:proxy-dialogs.obj,Odir:proxy-misc.obj,\
 Odir:quantize.obj,Odir:readbmp.obj,Odir:readj2k.obj,Odir:readjpeg.obj,\
 Odir:readpng.obj,Odir:readsun.obj,Odir:readtga.obj,Odir:readtiff.obj,\
 Odir:readxwd.obj,Odir:support.obj,Odir:xpmhash.obj,Odir:xpmread.obj

default : $(LIBTARGET) $(WDIR)libraries.opt $(NAME).exe_$(WORK)
	@ Continue

$(NAME).exe_$(WORK) : $(LIBTARGET) $(PROG_LIBS) $(WDIR)libraries.opt
.IFDEF NOLINK
	@ Continue
.ELSE
    LINK$(LOPTIONS)/Exe=$(NAME).exe_$(WORK) $(WDIR)libraries.opt/opt
    @ Write SYS$Output "Linking done.  Welcome to VMS Mosaic ''F$Edit("$(IDENT)","LOWERCASE")'"
.ENDIF

$(WDIR)libraries.opt :
	@ open/write libraries_file $(WDIR)libraries.opt
	@ write libraries_file "Identification=""Mosaic ''F$Edit("$(IDENT)","LOWERCASE")'"""
	@ write libraries_file "$(LIBTARGET)/Inc=(main)"
	@ write libraries_file "$(LIBTARGET)/Lib/Inc=(mailto,md5)"
	@ write libraries_file "[-.LIBHTMLW.$(WORK)]LIBHTMLW.OLB/Lib"
	@ write libraries_file "[-.LIBXMX.$(WORK)]LIBXMX.OLB/Lib"
	@ write libraries_file "[-.LIBWWW2.$(WORK)]LIBWWW.OLB/Lib"
.IFDEF NOTIFF
.ELSE
	@ write libraries_file "[-.LIBTIFF.$(WORK)]LIBTIFF.OLB/Lib"
.ENDIF
	@ write libraries_file "[-.LIBJPEG.$(WORK)]LIBJPEG.OLB/Lib"
	@ write libraries_file "[-.LIBOPENJPEG.$(WORK)]LIBOPENJPEG.OLB/Lib"
	@ write libraries_file "[-.LIBNUT.$(WORK)]LIBNUT.OLB/Lib"
	@ write libraries_file "[-.LIBPNG.$(WORK)]LIBPNG.OLB/Lib"
	@ write libraries_file "[-.ZLIB.$(WORK)]LIBZ.OLB/Lib"
	@ write libraries_file "[-.LIBLITECLUE.$(WORK)]LIBLITECLUE.OLB/Lib"
	@ write libraries_file "[-.LIBVMS.$(WORK)]LIBVMS.OLB/Lib"
.IFDEF NOWAIS
.ELSE
      	@ write libraries_file "$(LIBWAISQ)"
.ENDIF
.IFDEF NOSSL
.ELSE
      	@ write libraries_file "$(LIBSSL)"
      	@ write libraries_file "$(LIBCRYPTO)"
.ENDIF
.IFDEF GNUC
.IFDEF ALPHA
	@ write libraries_file "GNU:[000000]LIBGCC.OLB/Lib"
	@ !write libraries_file "SYS$Library:VaxCRTL.OLB/Lib"
	@ write libraries_file "GNU_CC:[000000]CRT0.OBJ"
.ELSE
	@ write libraries_file "GNU_CC:[000000]GCCLIB.OLB/Lib"
.ENDIF
.ENDIF
      	@ write libraries_file "$(DXM_LIB)"
      	@ write libraries_file "$(XMU_LIB)"
	@ write libraries_file "$(XM_LIB)"
	@ write libraries_file "$(XT_LIB)"
	@ write libraries_file "$(X_LIB)"
	@ write libraries_file "$(XEXT_LIB)"
	@ write libraries_file "$(VAXC_INET_LIB)"
.IFDEF VAXC
	@ write libraries_file "$(VAXC_RTL)"
.ENDIF
	@ close libraries_file

$(LIBTARGET) : $(LIBTARGET)($(OBJS))
        @ If F$Search("$(WDIR)work.mark") .NES. "" Then Delete $(WDIR)work.mark;*
        @ If F$Search("$(WDIR)work.done") .EQS. "" Then Copy Descrip.mms $(WDIR)work.done
	@ Write SYS$Output "Library src.olb built."

Odir:accept.obj :        accept.c accept.h cci.h [-.libwww2]tcp.h [-]config.h\
			 [-]config_$(WORK).h
Odir:annotate.obj :      annotate.c annotate.h grpan.h gui.h mo-www.h mosaic.h\
                         pan.h prefs.h prefs_defs.h toolbar.h vms_pwd.h\
                         [-.libnut]system.h [-.libxmx]xmx.h\
                         [-]config.h [-]config_$(WORK).h
Odir:audan.obj :         audan.c audan.h gui.h mo-www.h mosaic.h pan.h\
                         prefs.h prefs_defs.h toolbar.h vms_pwd.h\
                         [-.libnut]system.h [-.libxmx]xmx.h [-]config.h\
			 [-]config_$(WORK).h
Odir:bla.obj :           bla.c cci.h cciserver.h \
			 [-.libwww2]htaccess.h [-.libwww2]htanchor.h\
                         [-.libwww2]htatom.h [-.libwww2]htcompressed.h\
                         [-.libwww2]htext.h [-.libwww2]htfile.h\
                         [-.libwww2]htformat.h [-.libwww2]htlist.h\
                         [-.libwww2]html.h [-.libwww2]htmldtd.h\
                         [-.libwww2]htplain.h [-.libwww2]htstream.h\
                         [-.libwww2]htstring.h [-.libwww2]htutils.h\
                         [-.libwww2]sgml.h [-.libwww2]tcp.h [-.libhtmlw]list.h\
			 [-]config.h [-]config_$(WORK).h
Odir:ccibindings.obj :   ccibindings.c annotate.h cci.h ccibindings.h\
                         ccibindings2.h cciserver.h gui.h mo-www.h mosaic.h\
                         pan.h prefs.h prefs_defs.h toolbar.h\
                         [-.libhtmlw]html.h [-.libnut]str-tools.h\
                         [-.libxmx]xmx.h [-]config.h [-]config_$(WORK).h
Odir:ccibindings2.obj :  ccibindings2.c cci.h ccibindings2.h cciserver.h\
                         gui.h mosaic.h prefs.h prefs_defs.h\
			 toolbar.h [-.libwww2]htanchor.h\
                         [-.libwww2]htatom.h [-.libwww2]htformat.h\
                         [-.libwww2]htlist.h [-.libwww2]htstream.h\
                         [-.libwww2]htstring.h [-.libwww2]htutils.h\
                         [-.libxmx]xmx.h [-.libhtmlw]list.h [-]config.h\
			 [-]config_$(WORK).h
Odir:cciserver.obj :     cciserver.c accept.h cci.h ccibindings2.h cciserver.h\
                         prefs.h prefs_defs.h toolbar.h [-.libnut]str-tools.h\
                         [-.libxmx]xmx.h [-]config.h [-]config_$(WORK).h
Odir:child.obj :         child.c child.h [-.libhtmlw]list.h [-]config.h\
			 [-]config_$(WORK).h
Odir:colors.obj :        colors.c colors.h readjpeg.h\
			 [-.libjpeg]jpeglib.h [-]config.h [-]config_$(WORK).h
Odir:comment.obj :       comment.c comment.h gui.h mosaic.h prefs.h\
                         prefs_defs.h toolbar.h vms_pwd.h [-.libxmx]xmx.h\
                         [-]config.h [-]config_$(WORK).h [-]ssl_$(WORK).h
Odir:fsdither.obj :      fsdither.c [-.libhtmlw]html.h [-]config.h\
			 [-]config_$(WORK).h
Odir:gifread.obj :       gifread.c gifread.h [-]config.h [-]config_$(WORK).h
Odir:globalhist.obj :    globalhist.c globalhist.h mo-www.h mosaic.h prefs.h\
                         prefs_defs.h toolbar.h [-.libhtmlw]html.h\
                         [-.libhtmlw]htmlp.h [-.libnut]system.h [-.libxmx]xmx.h\
			 [-]config.h [-]config_$(WORK).h
Odir:grpan-www.obj :     grpan-www.c grpan-www.h mosaic.h prefs.h prefs_defs.h\
                         toolbar.h [-.libxmx]xmx.h [-]config.h\
			 [-]config_$(WORK).h
Odir:grpan.obj :         grpan.c grpan-www.h grpan.h mo-www.h mosaic.h pan.h\
                         prefs.h prefs_defs.h toolbar.h [-.libxmx]xmx.h\
                         [-.libwww2]htparse.h [-]config.h [-]config_$(WORK).h
Odir:gui-dialogs.obj :   gui-dialogs.c gui-dialogs.h gui-documents.h gui.h\
                         mo-www.h mailto.h mosaic.h prefs.h prefs_defs.h\
			 toolbar.h [-.libhtmlw]html.h [-.libnut]str-tools.h\
                         [-.libnut]system.h [-.libxmx]xmx.h\
			 [-]config.h [-]config_$(WORK).h
Odir:gui-documents.obj : gui-documents.c annotate.h cci.h ccibindings.h\
                         globalhist.h gui-documents.h gui-popup.h\
			 gui-extras.h gui.h\
                         history.h img.h mo-www.h mosaic.h prefs.h\
                         prefs_defs.h toolbar.h [-.libhtmlw]html.h\
			 [-.libhtmlw]htmlp.h [-.libnut]str-tools.h\
			 [-.libwww2]http.h [-.libwww2]htmime.h [-.libxmx]xmx.h\
			 [-]config.h [-]config_$(WORK).h
Odir:gui-extras.obj :    gui-extras.c gui-extras.h gui.h mo-www.h mosaic.h\
                         prefs.h prefs_defs.h toolbar.h [-.libhtmlw]html.h\
                         [-.libnut]system.h [-.libnut]str-tools.h\
			 [-.libxmx]xmx.h [-]config.h [-]config_$(WORK).h
Odir:gui-ftp.obj :       gui-ftp.c gui-ftp.h gui-popup.h gui.h hotlist.h\
                         mosaic.h prefs.h prefs_defs.h toolbar.h vms_pwd.h\
                         [-.libhtmlw]html.h [-.libnut]str-tools.h\
			 [-.libxmx]xmx.h [-.libwww2]htftp.h\
			 [-]config.h [-]config_$(WORK).h
Odir:gui-menubar.obj :   gui-menubar.c annotate.h cci.h ccibindings.h\
                         ccibindings2.h cciserver.h comment.h globalhist.h\
			 grpan.h gui-dialogs.h gui-ftp.h\
			 gui-news.h gui-popup.h gui.h\
                         history.h hotlist.h main.h mo-www.h mosaic.h newsrc.h\
                         pan.h prefs.h prefs_defs.h proxy.h toolbar.h\
                         vms_pwd.h [-.libhtmlw]html.h [-.libwww2]htaautil.h\
                         [-.libwww2]htaccess.h [-.libwww2]htanchor.h\
                         [-.libwww2]htatom.h [-.libwww2]htformat.h\
                         [-.libwww2]htlist.h [-.libwww2]htnews.h\
                         [-.libwww2]htstream.h [-.libwww2]htstring.h\
                         [-.libwww2]htutils.h [-.libwww2]tcp.h [-.libxmx]xmx.h\
                         [-.libhtmlw]htmlp.h [-]config.h [-]config_$(WORK).h
Odir:gui-news.obj :      gui-news.c gui-news.h gui.h mosaic.h newsrc.h prefs.h\
                         prefs_defs.h toolbar.h vms_pwd.h\
			 [-.libhtmlw]html.h [-.libnut]str-tools.h\
                         [-.libnut]system.h [-.libwww2]htaccess.h\
                         [-.libwww2]htanchor.h [-.libwww2]htatom.h\
                         [-.libwww2]htformat.h [-.libwww2]htlist.h\
                         [-.libwww2]htnews.h [-.libwww2]htstream.h\
                         [-.libwww2]htstring.h [-.libwww2]htutils.h\
                         [-.libwww2]tcp.h [-.libxmx]xmx.h [-]config.h\
			 [-]config_$(WORK).h
Odir:gui-popup.obj :     gui-popup.c gui-documents.h gui-ftp.h gui-popup.h\
                         hotlist.h mo-www.h mosaic.h prefs.h prefs_defs.h\
                         toolbar.h vms_pwd.h [-.libhtmlw]html.h\
			 [-.libhtmlw]htmlp.h [-.libhtmlw]htmlputil.h\
			 [-.libwww2]htparse.h [-.libxmx]xmx.h\
			 [-]config.h [-]config_$(WORK).h
Odir:gui.obj :           gui.c cci.h comment.h gui-documents.h gui-menubar.h\
			 gui-popup.h gui.h img.h main.h mo-www.h mosaic.h\
			 pan.h pixmaps.h prefs.h prefs_defs.h proxy.h\
			 quantize.h toolbar.h vms_pwd.h\
                         xresources.h [-.libhtmlw]html.h [-.libnut]str-tools.h\
                         [-.libnut]system.h [-.libwww2]htaabrow.h\
                         [-.libwww2]htaautil.h [-.libwww2]htlist.h\
                         [-.libwww2]htstring.h [-.libwww2]htutils.h\
                         [-.libwww2]tcp.h [-.libwww2]htfile.h\
			 [-.libwww2]htparse.h [-.libwww2]htcookie.h\
			 [-.libwww2]http.h [-.libxmx]xmx.h [-.libhtmlw]htmlp.h\
			 [-.libvms]cmdline.h [-]config.h [-]config_$(WORK).h\
			 [.bitmaps]xmosaic_32_icon.xbm\
			 [.bitmaps]xmosaic_75_icon.xbm
Odir:history.obj :       history.c history.h globalhist.h gui-popup.h gui.h\
			 hotlist.h mo-www.h mailto.h mosaic.h prefs.h\
			 prefs_defs.h toolbar.h vms_pwd.h [-.libhtmlw]htmlp.h\
                         [-.libhtmlw]html.h [-.libwww2]htaautil.h\
                         [-.libxmx]xmx.h [-]config.h [-]config_$(WORK).h
Odir:hotfile.obj :       hotfile.c hotfile.h hotlist.h mo-www.h\
			 mosaic.h prefs.h prefs_defs.h\
			 toolbar.h vms_pwd.h [-.libhtmlw]html.h\
                         [-.libhtmlw]htmlparse.h [-.libhtmlw]htmlp.h\
			 [-.libxmx]xmx.h [-]config.h [-]config_$(WORK).h
Odir:hotlist.obj :       hotlist.c gui.h hotfile.h hotlist.h mo-www.h mosaic.h\
                         mailto.h prefs.h prefs_defs.h toolbar.h vms_pwd.h\
                         [-.libnut]system.h [-.libxmx]xmx.h [-]config.h\
			 [-]config_$(WORK).h
Odir:img.obj :           img.c img.h cci.h globalhist.h gui.h img.h mo-www.h\
			 mosaic.h picread.h prefs.h\
			 prefs_defs.h toolbar.h gifread.h\
			 [-.libhtmlw]html.h [-.libhtmlw]htmlp.h\
			 [-.libhtmlw]htmlputil.h [-.libwww2]htbtree.h\
			 [-.libwww2]htmultiload.h\
			 [-.libnut]str-tools.h [-.libxmx]xmx.h [-]config.h\
			 [-]config_$(WORK).h
Odir:libtarga.obj :	 libtarga.c libtarga.h
Odir:mailto.obj :        mailto.c mailto.h gui.h gui-extras.h mosaic.h prefs.h\
			 prefs_defs.h\
			 toolbar.h [-.libnut]system.h [-.libnut]url-utils.h\
                         [-.libxmx]xmx.h [-]config.h [-]config_$(WORK).h
Odir:main.obj :          main.c main.h cci.h ccibindings2.h cciserver.h\
			 child.h globalhist.h gui.h hotlist.h mosaic.h\
                         newsrc.h pan.h prefs.h prefs_defs.h toolbar.h\
                         vms_pwd.h [-.libxmx]xmx.h [-.libwww2]htcookie.h\
			 [-]config.h [-]config_$(WORK).h [-]built_$(WORK).h\
			 [-]ssl_$(WORK).h
Odir:md5.obj :           md5.c md5.h mosaic.h prefs.h prefs_defs.h toolbar.h\
                         [-.libxmx]xmx.h [-]config.h [-]config_$(WORK).h
Odir:medcut.obj :        medcut.c medcut.h main.h [-]config.h\
			 [-]config_$(WORK).h
Odir:mo-www.obj :        mo-www.c gui-dialogs.h gui.h mo-www.h\
                         mosaic.h prefs.h prefs_defs.h toolbar.h\
                         [-.libhtmlw]html.h [-.libnut]str-tools.h\
                         [-.libnut]system.h [-.libwww2]htaautil.h\
                         [-.libwww2]htaccess.h [-.libwww2]htanchor.h\
                         [-.libwww2]htatom.h [-.libwww2]htext.h\
                         [-.libwww2]htformat.h [-.libwww2]htinit.h\
                         [-.libwww2]html.h [-.libwww2]htmldtd.h\
			 [-.libwww2]htmime.h [-.libwww2]htparse.h\
                         [-.libwww2]htstream.h [-.libwww2]htstring.h\
                         [-.libwww2]httcp.h [-.libwww2]htutils.h\
                         [-.libwww2]sgml.h [-.libwww2]tcp.h [-.libxmx]xmx.h\
                         [-]config.h [-]config_$(WORK).h
Odir:newsrc.obj :        newsrc.c mosaic.h newsrc.h prefs.h prefs_defs.h\
                         gui.h toolbar.h [-.libxmx]xmx.h [-]config.h\
			 [-]config_$(WORK).h
Odir:pan.obj :           pan.c mosaic.h pan.h prefs.h prefs_defs.h toolbar.h\
                         [-.libhtmlw]html.h [-.libxmx]xmx.h [-]config.h\
			 [-]config_$(WORK).h
Odir:picread.obj :       picread.c gifread.h mosaic.h picread.h prefs.h\
                         prefs_defs.h readbmp.h readtga.h readtiff.h\
			 readjpeg.h readpng.h readsun.h readxwd.h toolbar.h\
			 xpmread.h [-.zlib]zconf.h [-.zlib]zlib.h\
                         [-.libpng]png.h [-.libpng]pngconf.h [-.libxmx]xmx.h\
                         [-]config.h [-]config_$(WORK).h
Odir:pixmaps.obj :       pixmaps.c mosaic.h pixmaps.h prefs.h prefs_defs.h\
			 medcut.h toolbar.h xpm.h xpmread.h [-.libxmx]xmx.h\
                         [-]config.h [-]config_$(WORK).h [.pixmaps]icon_1.xpm\
			 [.pixmaps]splash.xpm [.pixmaps]cookie.xpm\
			 [.pixmaps]cookie_large.xpm [.pixmaps]toolbar_stop.xpm\
			 [.pixmaps]xm_error.xpm [.pixmaps]xm_question.xpm\
			 [.pixmaps]xm_information.xpm [.pixmaps]xm_warning.xpm\
			 [.pixmaps]encrypt.xpm [.pixmaps]toolbar_ftp_put_1.xpm\
			 [.pixmaps]toolbar_ftp_mkdir_1.xpm
Odir:prefs.obj :         prefs.c mosaic.h prefs.h prefs_defs.h toolbar.h\
                         vms_pwd.h [-.libxmx]xmx.h [-.libnut]system.h\
			 [-.libnut]str-tools.h [-]config.h [-]config_$(WORK).h
Odir:proxy-dialogs.obj : proxy-dialogs.c mosaic.h prefs.h prefs_defs.h proxy.h\
                         toolbar.h [-.libxmx]xmx.h [-]config.h\
			 [-]config_$(WORK).h
Odir:proxy-misc.obj :    proxy-misc.c proxy.h [-]config.h [-]config_$(WORK).h
Odir:quantize.obj :	 quantize.c quantize.h mosaic.h main.h mo-www.h\
			 [-.libhtmlw]html.h [-]config.h [-]config_$(WORK).h
Odir:readbmp.obj :	 readbmp.c mosaic.h readbmp.h readjpeg.h medcut.h\
			 quantize.h [-]config.h [-]config_$(WORK).h
Odir:readj2k.obj :	 readj2k.c mosaic.h prefs.h prefs_defs.h readj2k.h\
			 quantize.h [-.libopenjpeg]openjpeg.h [-]config.h\
			 [-]config_$(WORK).h
Odir:readjpeg.obj :      readjpeg.c mosaic.h prefs.h prefs_defs.h readjpeg.h\
                         toolbar.h [-.libjpeg]jconfig.h [-.libjpeg]jmorecfg.h\
                         [-.libjpeg]jpeglib.h [-.libxmx]xmx.h [-]config.h\
			 [-]config_$(WORK).h
Odir:readpng.obj :       readpng.c mosaic.h prefs.h prefs_defs.h readpng.h\
                         quantize.h toolbar.h [-.zlib]zconf.h\
			 [-.zlib]zlib.h\
                         [-.libpng]png.h [-.libpng]pngconf.h [-.libxmx]xmx.h\
                         [-]config.h [-]config_$(WORK).h
Odir:readsun.obj :	 readsun.c mosaic.h readsun.h quantize.h\
			 [-]config.h [-]config_$(WORK).h
Odir:readtga.obj :	 readtga.c mosaic.h readtga.h libtarga.h quantize.h\
			 [-]config.h [-]config_$(WORK).h
Odir:readtiff.obj :	 readtiff.c mosaic.h readtiff.h quantize.h\
			 [-.libtiff]tiffio.h [-]config.h [-]config_$(WORK).h
Odir:readxwd.obj :	 readxwd.c mosaic.h readxwd.h quantize.h\
			 [-]config.h [-]config_$(WORK).h
Odir:support.obj :       support.c cci.h [-]config.h [-]config_$(WORK).h
Odir:xpmhash.obj :       xpmhash.c xpm.h [-]config.h [-]config_$(WORK).h
Odir:xpmread.obj :       xpmread.c mosaic.h prefs.h prefs_defs.h toolbar.h\
                         xpm.h xpmread.h [-.libxmx]xmx.h [-]config.h\
			 [-]config_$(WORK).h

.c.obj :
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb :
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	Delete/Log $(WDIR)*.OBJ;*
