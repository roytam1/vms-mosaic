! MMS description file for glib
! George E. Cook, WVNET, 8-Jan-2008
! Mosaic 4.3
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!

WDIR = [.$(WORK)]

LIBTARGET = $(WDIR)glib.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.IFDEF DECC
CQUALC = /DECC/INCLUDE=[-.LIBINTL]
.ELSE
.IFDEF DECCVAXC
CQUALC = /VAXC/INCLUDE=[-.LIBINTL]
.ELSE
.IFDEF GNUC
CQUALC = /INCLUDE=(GCC_Include)
.ELSE
CQUALC = /INCLUDE=[-.LIBINTL]
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

OBJECTS = Odir:garray.obj Odir:gatomic.obj Odir:gconvert.obj Odir:gdataset.obj \
	  Odir:gerror.obj Odir:gdir.obj Odir:gfileutils.obj Odir:ghook.obj \
	  Odir:ghash.obj Odir:glist.obj Odir:gmain.obj Odir:gmarkup.obj \
	  Odir:gmem.obj Odir:gmessages.obj Odir:gprimes.obj \
	  Odir:gqueue.obj Odir:gprintf.obj Odir:grand.obj \
	  Odir:gslist.obj Odir:gstdio.obj Odir:gstrfuncs.obj Odir:gstring.obj \
	  Odir:gthread.obj Odir:gtree.obj Odir:gunibreak.obj \
	  Odir:gunidecomp.obj Odir:guniprop.obj Odir:gutf8.obj Odir:gutils.obj \
	  Odir:printf-args.obj Odir:printf-parse.obj Odir:printf.obj \
	  Odir:vasnprintf.obj Odir:localcharset.obj
! Not needed for Mosaic
!	  Odir:gqsort.obj Odir:gscanner.obj

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
	@ Write SYS$Output "Library glib.olb built."

Odir:garray.obj : garray.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:gatomic.obj : gatomic.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h gatomic.h
Odir:gconvert.obj : gconvert.c config.h glib.h galloca.h \
	gtypes.h glibconfig.h gmacros.h garray.h gasyncqueue.h \
	gthread.h gerror.h gquark.h gbacktrace.h gcache.h glist.h \
	gmem.h gcompletion.h gconvert.h gdataset.h gdate.h gdir.h \
	gfileutils.h ghash.h ghook.h giochannel.h gmain.h gslist.h \
	gstring.h gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h \
	gprimes.h gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h \
	gspawn.h gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h \
	gprintfint.h glibintl.h
Odir:gdataset.obj : gdataset.c config.h glib.h galloca.h \
	gtypes.h glibconfig.h gmacros.h garray.h gasyncqueue.h \
	gthread.h gerror.h gquark.h gbacktrace.h gcache.h glist.h \
	gmem.h gcompletion.h gconvert.h gdataset.h gdate.h gdir.h \
	gfileutils.h ghash.h ghook.h giochannel.h gmain.h gslist.h \
	gstring.h gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h \
	gprimes.h gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h \
	gspawn.h gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:gerror.obj : gerror.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:gdir.obj : gdir.c config.h glib.h galloca.h gtypes.h \
        glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
        gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
        gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
        ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
        gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
        gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
        gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h gdir.h \
        glibintl.h
Odir:gfileutils.obj : gfileutils.c config.h glib.h galloca.h \
	gtypes.h glibconfig.h gmacros.h garray.h gasyncqueue.h \
	gthread.h gerror.h gquark.h gbacktrace.h gcache.h glist.h \
	gmem.h gcompletion.h gconvert.h gdataset.h gdate.h gdir.h \
	gfileutils.h ghash.h ghook.h giochannel.h gmain.h gslist.h \
	gstring.h gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h \
	gprimes.h gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h \
	gspawn.h gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h \
	glibintl.h
Odir:ghash.obj : ghash.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:ghook.obj : ghook.c config.h glib.h galloca.h gtypes.h \
        glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
        gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
        gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
        ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
        gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
        gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
        gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:glist.obj : glist.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:gmain.obj : gmain.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:gmarkup.obj : gmarkup.c config.h glib.h galloca.h gtypes.h \
        glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
        gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
        gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
        ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
        gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
        gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
        gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h glibintl.h
Odir:gmem.obj : gmem.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:gmessages.obj : gmessages.c config.h glib.h galloca.h \
	gtypes.h glibconfig.h gmacros.h garray.h gasyncqueue.h \
	gthread.h gerror.h gquark.h gbacktrace.h gcache.h glist.h \
	gmem.h gcompletion.h gconvert.h gdataset.h gdate.h gdir.h \
	gfileutils.h ghash.h ghook.h giochannel.h gmain.h gslist.h \
	gstring.h gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h \
	gprimes.h gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h \
	gspawn.h gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h \
	gdebug.h gprintfint.h
Odir:gqsort.obj : gqsort.c config.h glib.h galloca.h gtypes.h \
        glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
        gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
        gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
        ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
        gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
        gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
        gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:gqueue.obj : gqueue.c config.h glib.h galloca.h gtypes.h \
        glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
        gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
        gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
        ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
        gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
        gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
        gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:gprimes.obj : gprimes.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:gprintf.obj : gprintf.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h gprintf.h \
	gprintfint.h
Odir:grand.obj : grand.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
gscanner.obj : gscanner.c config.h glib.h galloca.h \
        gtypes.h [-]glibconfig.h gmacros.h garray.h gasyncqueue.h \
        gthread.h gerror.h gquark.h gbacktrace.h gcache.h glist.h \
        gmem.h gcompletion.h gconvert.h gdataset.h gdate.h gdir.h \
        gfileutils.h ghash.h ghook.h giochannel.h gmain.h gslist.h \
        gstring.h gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h \
        gprimes.h gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h \
        gspawn.h gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h \
        gprintfint.h
Odir:gslist.obj : gslist.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:gstdio.obj : gstdio.c
Odir:gstrfuncs.obj : gstrfuncs.c config.h glib.h galloca.h \
	gtypes.h glibconfig.h gmacros.h garray.h gasyncqueue.h \
	gthread.h gerror.h gquark.h gbacktrace.h gcache.h glist.h \
	gmem.h gcompletion.h gconvert.h gdataset.h gdate.h gdir.h \
	gfileutils.h ghash.h ghook.h giochannel.h gmain.h gslist.h \
	gstring.h gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h \
	gprimes.h gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h \
	gspawn.h gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h \
	gprintfint.h
Odir:gstring.obj : gstring.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:gthread.obj : gthread.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:gtree.obj : gtree.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h
Odir:gunibreak.obj : gunibreak.c config.h glib.h galloca.h gtypes.h \
        glibconfig.h gmacros.h garray.h gasyncqueue.h \
        gthread.h gerror.h gquark.h gbacktrace.h gcache.h glist.h \
        gmem.h gcompletion.h gconvert.h gdataset.h gdate.h gdir.h \
        gfileutils.h ghash.h ghook.h giochannel.h gmain.h gslist.h \
        gstring.h gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h \
        gprimes.h gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h \
        gspawn.h gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h \
        gunibreak.h
Odir:gunidecomp.obj : gunidecomp.c [-]config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h \
        gthread.h gerror.h gquark.h gbacktrace.h gcache.h glist.h \
        gmem.h gcompletion.h gconvert.h gdataset.h gdate.h gdir.h \
        gfileutils.h ghash.h ghook.h giochannel.h gmain.h gslist.h \
        gstring.h gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h \
        gprimes.h gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h \
        gspawn.h gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h \
        gunidecomp.h gunicomp.h
Odir:guniprop.obj : guniprop.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h \
        gthread.h gerror.h gquark.h gbacktrace.h gcache.h glist.h \
        gmem.h gcompletion.h gconvert.h gdataset.h gdate.h gdir.h \
        gfileutils.h ghash.h ghook.h giochannel.h gmain.h gslist.h \
        gstring.h gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h \
        gprimes.h gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h \
        gspawn.h gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h \
        gunichartables.h
Odir:gutf8.obj : gutf8.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h \
	libcharset.h glibintl.h
Odir:gutils.obj : gutils.c config.h glib.h galloca.h gtypes.h \
	glibconfig.h gmacros.h garray.h gasyncqueue.h gthread.h \
	gerror.h gquark.h gbacktrace.h gcache.h glist.h gmem.h \
	gcompletion.h gconvert.h gdataset.h gdate.h gdir.h gfileutils.h \
	ghash.h ghook.h giochannel.h gmain.h gslist.h gstring.h \
	gunicode.h gmarkup.h gmessages.h gnode.h gpattern.h gprimes.h \
	gqsort.h gqueue.h grand.h grel.h gscanner.h gshell.h gspawn.h \
	gstrfuncs.h gthreadpool.h gtimer.h gtree.h gutils.h \
	gprintfint.h
Odir:printf-args.obj : printf-args.c printf-args.h
Odir:printf-parse.obj : printf-parse.c printf-parse.h
Odir:printf.obj : printf.c printf.h
Odir:vasnprintf.obj : vasnprintf.c vasnprintf.h
Odir:localcharset.obj : localcharset.c libcharset.h

.c.obj :
        $(CC)$(CFLAGS)$(CFLOAT)/OBJECT=$@ $<

.obj.olb
        $(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
        Delete/Log $(WDIR)*.OBJ;*
        Delete/Log $(LIBTARGET);*
