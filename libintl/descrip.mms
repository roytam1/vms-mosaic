# -*- Makefile -*- for gettext-runtime/intl on VMS using the MMS utility
! George E. Cook, WVNET, 5-Jan-2008
! Mosaic 4.3
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!

prefix = SYS$DATA:[
datadir = $(prefix).share
localedir = $(datadir).locale
aliaspath = $(localedir)]

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)libintl.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.IFDEF DECC
CQUALC = /DECC
.ELSE
.IFDEF DECCVAXC
CQUALC = /VAXC
.ELSE
CQUALC =
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

#DEFS = "VMS=1","HAVE_CONFIG_H=1","LOCALEDIR=""$(localedir)]""","LOCALE_ALIAS_PATH=""$(aliaspath)""","LIBDIR=""$(libdir)]""","IN_LIBINTL=1"
DEFS = "LOCALEDIR=""$(localedir)]""","LOCALE_ALIAS_PATH=""$(aliaspath)"""

.IFDEF DEBUG
CFLAGS = $(CQUALC)/NoOpt/Debug/define=($(DEFS))
.ELSE
CFLAGS = $(CQUALC)/define=($(DEFS))
.ENDIF

OBJECTS = Odir:bindtextdom.obj Odir:dcgettext.obj Odir:dgettext.obj \
	  Odir:gettext.obj Odir:finddomain.obj Odir:loadmsgcat.obj \
	  Odir:localealias.obj Odir:textdomain.obj Odir:l10nflist.obj \
	  Odir:explodename.obj Odir:dcigettext.obj Odir:dcngettext.obj \
	  Odir:dngettext.obj Odir:ngettext.obj Odir:plural.obj \
	  Odir:plural-exp.obj Odir:localcharset.obj Odir:relocatable.obj \
	  Odir:langprefs.obj Odir:localename.obj Odir:log.obj Odir:osdep.obj \
	  Odir:intl-compat.obj

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
	@ Write SYS$Output "Library libintl.olb built."

Odir:bindtextdom.obj  : bindtextdom.c libgnuintl.h gettextP.h gmo.h loadinfo.h
Odir:dcgettext.obj    : dcgettext.c libgnuintl.h gettextP.h gmo.h loadinfo.h
Odir:dgettext.obj     : dgettext.c libgnuintl.h gettextP.h gmo.h loadinfo.h
Odir:gettext.obj      : gettext.c libgnuintl.h gettextP.h gmo.h loadinfo.h
Odir:finddomain.obj   : finddomain.c libgnuintl.h gettextP.h gmo.h loadinfo.h
Odir:loadmsgcat.obj   : loadmsgcat.c libgnuintl.h gettextP.h gmo.h loadinfo.h \
		        hash-string.h plural-exp.h
Odir:localealias.obj  : localealias.c libgnuintl.h gettextP.h gmo.h loadinfo.h \
		        relocatable.h
Odir:textdomain.obj   : textdomain.c libgnuintl.h gettextP.h gmo.h loadinfo.h
Odir:l10nflist.obj    : l10nflist.c libgnuintl.h loadinfo.h
Odir:explodename.obj  : explodename.c libgnuintl.h loadinfo.h
Odir:dcigettext.obj   : dcigettext.c libgnuintl.h gettextP.h gmo.h loadinfo.h \
		        hash-string.h plural-exp.h eval-plural.h
Odir:dcngettext.obj   : dcngettext.c libgnuintl.h gettextP.h gmo.h loadinfo.h
Odir:dngettext.obj    : dngettext.c libgnuintl.h gettextP.h gmo.h loadinfo.h
Odir:ngettext.obj     : ngettext.c libgnuintl.h gettextP.h gmo.h loadinfo.h
Odir:plural.obj       : plural.c libgnuintl.h plural-exp.h
Odir:plural-exp.obj   : plural-exp.c libgnuintl.h plural-exp.h
Odir:localcharset.obj : localcharset.c libgnuintl.h localcharset.h relocatable.h
Odir:relocatable.obj  : relocatable.c libgnuintl.h relocatable.h
Odir:langprefs.obj    : langprefs.c libgnuintl.h
Odir:localename.obj   : localename.c libgnuintl.h
Odir:log.obj          : log.c libgnuintl.h
Odir:osdep.obj        : osdep.c libgnuintl.h
Odir:intl-compat.obj  : intl-compat.c libgnuintl.h gettextP.h gmo.h loadinfo.h

.c.obj :
        $(CC)$(CFLAGS)$(CFLOAT)/OBJECT=$@ $<

.obj.olb
        $(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
        Delete/Log $(WDIR)*.OBJ;*
        Delete/Log $(LIBTARGET);*
