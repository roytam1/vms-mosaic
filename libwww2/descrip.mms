! MMS description file for LIBWWW2
! Bjorn S. Nilsson, Aleph, CERN, 20-Nov-1993
! (Mosaic version 2.0)
! Mosaic version 2.4 20-Aug-1994
! Mosaic version 2.6 1-Nov-1995, George Cook
!
! Copyright (C) 2005, 2006 - The VMS Mosaic Project
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libwww.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.FIRST
        @ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
.IFDEF GNUC
	@ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.ENDIF
.IFDEF PATHWAY
	@ @[-.TWG]def
.ENDIF
.IFDEF DECC
.IFDEF ALPHA
	@ If F$Trnlnm("ALPHA$LIBRARY") .NES. "" Then Define/NoLog Sys Alpha$Library
.ELSE
	@ If F$Trnlnm("DECC$LIBRARY_INCLUDE") .NES. "" Then Define/NoLog Sys DECC$Library_Include
.ENDIF
.ELSE
.IFDEF GNUC
	@ Define/NoLog Sys GNU_CC_Include
.ELSE
	@ Define/NoLog Sys SYS$Library
.ENDIF
.ENDIF
.IFDEF NOWAIS
.ELSE
	@ Topdir = F$Environment("Default")-"LIBWWW2]"
	@ Define WAIS_IR  'Topdir'FREEWAIS-0_5.IR]
.IFDEF DECC
	@ Define DECC$User_Include 'F$Environment("Default")',WAIS_IR
	@ Define DECC$System_Include 'F$Environment("Default")',WAIS_IR,SYS
.ELSE
.IFDEF GNUC
.ELSE
	@ Define C$Include 'F$Environment("Default")',WAIS_IR
	@ Define VAXC$Include 'F$Environment("Default")',WAIS_IR,SYS
.ENDIF
.ENDIF
WAISMODULES = Odir:HTWAIS.obj
.ENDIF

.IFDEF DECC
.IFDEF PATHWAY
.INCLUDE [-.TWG]DECC_PREFIX_RULES.MMS
CQUALC=/DECC/Standard=VAXC/Precision=SINGLE $(CC_PREFIX_NO_SIN)
.ELSE
.IFDEF MULTINET
CQUALC=/DECC/Standard=VAXC/Precision=SINGLE/Prefix=ANSI
.ELSE
.IFDEF SOCKETSHR
CQUALC=/DECC/Standard=VAXC/Precision=SINGLE/Prefix=(All,Except=Ioctl)
.ELSE
CQUALC=/DECC/Precision=SINGLE/Prefix=(All,Except=Ioctl)
.ENDIF
.ENDIF
.ENDIF
.ELSE
.IFDEF DECCVAXC
CQUALC=/VAXC/Precision=SINGLE
.ELSE
.IFDEF GNUC
CQUALC=
.ELSE
CQUALC=/Precision=SINGLE
.ENDIF
.ENDIF
.ENDIF

.IFDEF DEBUG
CFLAGS = $(CQUALC)/NoOpt/Debug
.ELSE
CFLAGS = $(CQUALC)
.ENDIF

OBJECTS = Odir:HTAABrow.obj Odir:HTAAUtil.obj Odir:HTAccess.obj \
  Odir:HTAlert.obj Odir:HTAnchor.obj Odir:HTAssoc.obj Odir:HTAtom.obj \
  Odir:HTChunk.obj Odir:HTCompressed.obj Odir:HTFile.obj Odir:HTFormat.obj \
  Odir:HTFTP.obj Odir:HTFWriter.obj Odir:HTGopher.obj Odir:HTIcon.obj \
  Odir:HTInit.obj Odir:HTList.obj Odir:HTMailto.obj Odir:HTMIME.obj \
  Odir:HTML.obj Odir:HTMLDTD.obj Odir:HTMLGen.obj Odir:HTMosaicHTML.obj \
  Odir:HTNews.obj Odir:HTParse.obj Odir:HTPlain.obj Odir:HTSort.obj \
  Odir:HTString.obj Odir:HTTCP.obj Odir:HTTelnet.obj Odir:HTTP.obj \
  Odir:HTUU.obj Odir:HTVMSUtils.obj Odir:HTWriter.obj Odir:HTWSRC.obj \
  Odir:HTBTree.obj Odir:HTFinger.obj Odir:HTCookie.obj Odir:SGML.obj \
  $(WAISMODULES)

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
	@ Write SYS$Output "Library libwww.olb built."

Odir:HTAABrow.obj :     HTAABrow.c htaabrow.h htaautil.h htalert.h htassoc.h \
                        htlist.h htparse.h htstring.h htutils.h htuu.h tcp.h \
                        [-.src]md5.h [-.libnut]str-tools.h [-]config.h \
			[-]config_$(WORK).h
Odir:HTAAUtil.obj :     HTAAUtil.c htaautil.h htassoc.h htlist.h htstring.h \
                        htutils.h tcp.h [-]config.h [-]config_$(WORK).h \
			[-]ssl_$(WORK).h
Odir:HTAccess.obj :     HTAccess.c htaccess.h htalert.h htanchor.h htatom.h \
                        htext.h htformat.h htlist.h html.h htmldtd.h htmime.h\
                        htparse.h htstream.h htstring.h htutils.h sgml.h \
                        tcp.h htmultiload.h [-.src]proxy.h \
			[-.libnut]str-tools.h [-]config.h [-]config_$(WORK).h \
			[-]ssl_$(WORK).h
Odir:HTAlert.obj :      HTAlert.c htalert.h htstring.h htutils.h tcp.h \
                        [-]config.h [-]config_$(WORK).h
Odir:HTAnchor.obj :     HTAnchor.c htanchor.h htatom.h htformat.h htlist.h \
                        htparse.h htstream.h htstring.h htutils.h tcp.h \
                        [-]config.h [-]config_$(WORK).h
Odir:HTAssoc.obj :      HTAssoc.c htaautil.h htassoc.h htlist.h htstring.h \
                        htutils.h tcp.h [-.libnut]str-tools.h [-]config.h \
			[-]config_$(WORK).h
Odir:HTAtom.obj :       HTAtom.c htatom.h htstring.h htutils.h tcp.h \
			[-]config.h [-]config_$(WORK).h
Odir:HTBTree.obj :      HTBTree.c htbtree.h htutils.h tcp.h \
			[-]config.h [-]config_$(WORK).h
Odir:HTChunk.obj :      HTChunk.c htchunk.h htstring.h htutils.h [-]config.h \
			[-]config_$(WORK).h
Odir:HTCompressed.obj : HTCompressed.c htaccess.h htalert.h htanchor.h \
                        htatom.h htext.h htfile.h htformat.h htfwriter.h \
                        htinit.h htstream.h htstring.h htutils.h htcompressed.h\
			tcp.h [-.libnut]system.h [-]config.h [-]config_$(WORK).h
Odir:HTCookie.obj :     HTCookie.c htcookie.h htaccess.h htalert.h htparse.h \
			httcp.h htstring.h htutils.h htfile.h htbtree.h \
			http.h htlist.h [-.libxmx]xmx.h \
			[-.libnut]str-tools.h [-.libnut]system.h \
			[-.src]mosaic.h [-.src]prefs.h [-]config.h \
			[-]config_$(WORK).h
Odir:HTFile.obj :       HTFile.c htaccess.h htalert.h htanchor.h htatom.h \
                        htext.h htfile.h htformat.h htftp.h htfwriter.h \
                        htinit.h htlist.h html.h htmldtd.h htparse.h htsort.h \
                        htstream.h htstring.h httcp.h htutils.h htwriter.h \
                        hticon.h sgml.h tcp.h [-.libnut]system.h [-]config.h \
			[-]config_$(WORK).h
Odir:HTFinger.obj :	HTFinger.c htfinger.h htaccess.h htalert.h htanchor.h \
			html.h htparse.h htformat.h httcp.h htstring.h \
			htutils.h tcp.h [-]config.h [-]config_$(WORK).h
Odir:HTFormat.obj :     HTFormat.c htalert.h htanchor.h htatom.h htext.h \
                        htformat.h htfwriter.h htinit.h htlist.h htmime.h \
                        html.h htmldtd.h htmlgen.h htplain.h htstream.h \
                        htstring.h htutils.h sgml.h tcp.h [-]config.h \
			[-]config_$(WORK).h [-]ssl_$(WORK).h
Odir:HTFTP.obj :        HTFTP.c htaautil.h htaccess.h htalert.h htanchor.h \
                        htatom.h htchunk.h htext.h htfile.h htformat.h \
                        htftp.h hticon.h htlist.h html.h htmldtd.h htparse.h \
                        htstream.h htstring.h httcp.h htutils.h sgml.h tcp.h \
                        tcp.h [-.libnut]str-tools.h [-]config.h \
			[-]config_$(WORK).h
Odir:HTFWriter.obj :    HTFWriter.c htaccess.h htalert.h htanchor.h htatom.h \
                        htcompressed.h htext.h htfile.h htformat.h htmime.h\
                        htfwriter.h htlist.h html.h htmldtd.h htstream.h \
                        htstring.h htutils.h sgml.h tcp.h [-.libxmx]xmx.h \
                        [-.src]mosaic.h [-.src]prefs.h [-.src]prefs_defs.h \
                        [-.src]toolbar.h [-.src]child.h [-.src]mo-www.h\
			[-]config.h [-]config_$(WORK).h
Odir:HTGopher.obj :     HTGopher.c htaccess.h htalert.h htanchor.h htatom.h \
			htfile.h htformat.h htgopher.h htlist.h html.h \
			htmldtd.h htparse.h htstream.h htstring.h httcp.h \
			htutils.h sgml.h tcp.h [-]config.h [-]config_$(WORK).h
Odir:HTIcon.obj :       HTIcon.c htaccess.h htanchor.h htatom.h htfile.h \
                        htformat.h htlist.h html.h htmldtd.h htstream.h \
                        hticon.h htstring.h htutils.h sgml.h tcp.h [-]config.h \
			[-]config_$(WORK).h
Odir:HTInit.obj :       HTInit.c htaccess.h htanchor.h htatom.h htfile.h \
                        htformat.h htfwriter.h htinit.h htlist.h htmime.h \
			html.h htmldtd.h htmlgen.h htmosaichtml.h htplain.h \
                        htstream.h htstring.h htutils.h htwsrc.h sgml.h tcp.h \
                        [-]config.h [-]config_$(WORK).h
Odir:HTList.obj :       HTList.c htlist.h htstring.h htutils.h [-]config.h \
			[-]config_$(WORK).h
Odir:HTMailto.obj :     HTMailto.c htaccess.h htalert.h htanchor.h htatom.h \
			htformat.h htlist.h html.h htmldtd.h htstream.h \
                        htstring.h htutils.h sgml.h\
                        [-.libnut]str-tools.h [-]config.h [-]config_$(WORK).h
Odir:HTMIME.obj :       HTMIME.c htaautil.h htaccess.h htalert.h htanchor.h \
                        htatom.h htfile.h htformat.h htlist.h htmime.h html.h \
                        htmldtd.h htstream.h htstring.h htutils.h sgml.h tcp.h \
                        htcookie.h [-.libnut]str-tools.h [-]config.h \
			[-]config_$(WORK).h
Odir:HTML.obj :         HTML.c htalert.h htanchor.h htatom.h htchunk.h \
                        htext.h htformat.h htlist.h html.h htmldtd.h \
                        htmlgen.h htparse.h htstream.h htstring.h htutils.h \
                        sgml.h tcp.h [-]config.h [-]config_$(WORK).h
Odir:HTMLDTD.obj :      HTMLDTD.c htmldtd.h htstream.h htstring.h htutils.h \
                        sgml.h [-]config.h [-]config_$(WORK).h
Odir:HTMLGen.obj :      HTMLGen.c htanchor.h htatom.h htformat.h htlist.h \
                        html.h htmldtd.h htmlgen.h htstream.h htstring.h \
                        htutils.h sgml.h [-]config.h [-]config_$(WORK).h
Odir:HTMosaicHTML.obj : HTMosaicHTML.c htaccess.h htanchor.h htatom.h \
                        htcompressed.h htext.h htfile.h htformat.h htlist.h \
                        html.h htmldtd.h htmosaichtml.h htstream.h htstring.h \
                        htutils.h sgml.h tcp.h [-]config.h [-]config_$(WORK).h
Odir:HTNews.obj :       HTNews.c htaccess.h htanchor.h htatom.h htformat.h \
                        htalert.h htlist.h html.h htmldtd.h htnews.h htparse.h \
                        htstream.h htstring.h htutils.h sgml.h tcp.h \
                        [-.src]mosaic.h [-.src]newsrc.h \
                        [-.src]prefs.h [-.src]prefs_defs.h [-.src]toolbar.h \
                        [-]config.h [-]config_$(WORK).h
Odir:HTParse.obj :      HTParse.c htparse.h htstring.h htutils.h \
                        [-]config.h [-]config_$(WORK).h
Odir:HTPlain.obj :      HTPlain.c htaccess.h htanchor.h htatom.h \
                        htcompressed.h htext.h htfile.h htformat.h htlist.h \
                        html.h htmldtd.h htplain.h htstream.h htstring.h \
                        htutils.h sgml.h tcp.h [-]config.h [-]config_$(WORK).h
Odir:HTSort.obj :       HTSort.c htstring.h htutils.h [-]config.h \
			[-]config_$(WORK).h
Odir:HTString.obj :     HTString.c htstring.h htutils.h [-]config.h \
			[-]config_$(WORK).h
Odir:HTTCP.obj :        HTTCP.c htaccess.h htalert.h htanchor.h htatom.h \
                        htformat.h htlist.h htparse.h htstream.h htstring.h \
                        httcp.h htutils.h tcp.h [-]config.h [-]config_$(WORK).h
Odir:HTTelnet.obj :     HTTelnet.c htaccess.h htalert.h htanchor.h htatom.h \
                        htfile.h htformat.h htlist.h html.h htmldtd.h \
                        htparse.h htstream.h htstring.h httelnet.h http.h \
                        htutils.h sgml.h tcp.h [-.libnut]str-tools.h \
			[-]config.h [-]config_$(WORK).h
Odir:HTTP.obj :         HTTP.c htaabrow.h htaautil.h htaccess.h htalert.h \
                        htanchor.h htatom.h htfile.h htformat.h htinit.h \
                        htlist.h htmime.h html.h htmldtd.h htparse.h \
                        htstream.h htstring.h httcp.h http.h htutils.h sgml.h \
                        htcookie.h htmultiload.h tcp.h [-.src]mosaic.h \
			[-.libxmx]xmx.h [-]config.h [-]config_$(WORK).h \
		        [-]ssl_$(WORK).h
Odir:HTUU.obj :         HTUU.c htstring.h htutils.h htuu.h [-]config.h \
			[-]config_$(WORK).h
Odir:HTVMSUtils.obj :   HTVMSUtils.c htvmsutils.h htalert.h htbtree.h \
			htformat.h htanchor.h htfile.h htstream.h htutils.h \
			tcp.h httcp.h [-.libnut]str-tools.h [-]config.h \
			[-]config_$(WORK).h
Odir:HTWAIS.obj :       HTWAIS.c htaccess.h htalert.h htanchor.h htatom.h \
                        htfile.h htformat.h htlist.h html.h htmldtd.h \
                        htparse.h htstream.h htstring.h httcp.h htutils.h \
                        sgml.h tcp.h [-.freewais-0_5.ir]cdialect.h \
                        [-.freewais-0_5.ir]cutil.h [-.freewais-0_5.ir]panic.h \
                        [-.freewais-0_5.ir]server.h \
                        [-.freewais-0_5.ir]transprt.h [-.freewais-0_5.ir]ui.h \
                        [-.freewais-0_5.ir]version.h \
                        [-.freewais-0_5.ir]wmessage.h \
                        [-.freewais-0_5.ir]wprot.h [-.freewais-0_5.ir]zprot.h \
                        [-.freewais-0_5.ir]ztype1.h [-.freewais-0_5.ir]zutil.h \
                        [-]config.h [-]config_$(WORK).h
Odir:HTWriter.obj :     HTWriter.c htstream.h htstring.h htutils.h htwriter.h \
                        tcp.h [-]config.h [-]config_$(WORK).h
Odir:HTWSRC.obj :       HTWSRC.c htanchor.h htatom.h htformat.h htlist.h \
                        html.h htmldtd.h htparse.h htstream.h htstring.h \
                        htutils.h htwsrc.h sgml.h tcp.h [-.libnut]str-tools.h \
                        [-]config.h [-]config_$(WORK).h
Odir:SGML.obj :         SGML.c htchunk.h htstream.h htstring.h htutils.h \
                        sgml.h [-.libnut]str-tools.h [-]config.h \
			[-]config_$(WORK).h

.c.obj :
	$(CC)$(CFLAGS)/OBJECT=$@ $<

.obj.olb
	$(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
	Delete/Log $(WDIR)*.OBJ;*
	Delete/Log $(LIBTARGET);*

