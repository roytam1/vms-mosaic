!
! MMS description file for gobject
! George E. Cook, WVNET, 17-Aug-2008
! Mosaic 4.3
!
! This description file is intended to be invoked by the top level
! description file.  It should not be invoked directly.
!
! You may have to use the /IGNORE=WARNING qualifier to make MMS run all
! the way through if you get (acceptable) compilation warnings.
!
#*****************************************************************************
#                                                                            *
# Make file for VMS                                                          *
# Author : J.Jansen (joukj@hrem.stm.tudelft.nl)                              *
# Date : 13 May 2004                                                         *
#                                                                            *
#*****************************************************************************

WDIR = [-.$(WORK)]

LIBTARGET = $(WDIR)gobjectlib.olb

.IFDEF GNUC
CC = GCC
.ELSE
CC = CC
.ENDIF

.IFDEF DECC
CQUALC = /DECC/INCLUDE=[-]/DEFINE=("GOBJECT_COMPILATION=1")
.ELSE
.IFDEF DECCVAXC
CQUALC = /VAXC/INCLUDE=[-]/DEFINE=("GOBJECT_COMPILATION=1")
.ELSE
.IFDEF GNUC
CQUALC = /INCLUDE=(GCC_Include)/DEFINE=("GOBJECT_COMPILATION=1")
.ELSE
CQUALC = /INCLUDE=[-]/DEFINE=("GOBJECT_COMPILATION=1")
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

OBJECTS = Odir:gboxed.obj Odir:gclosure.obj Odir:genums.obj Odir:gobject.obj \
	  Odir:gparam.obj Odir:gparamspecs.obj, Odir:gsignal.obj \
	  Odir:gsourceclosure.obj Odir:gtype.obj Odir:gtypemodule.obj \
	  Odir:gtypeplugin.obj Odir:gvalue.obj Odir:gvaluearray.obj \
	  Odir:gvaluetransform.obj Odir:gvaluetypes.obj

.FIRST
	@ If F$Search("$(LIBTARGET)") .EQS. "" Then Library/Create $(LIBTARGET)
	@ Define/NoLog Odir $(WDIR)
	@ Topdir = F$Environment("Default")-"GLIB.GOBJECT]"
	@ Define/NoLog GLIB 'Topdir'GLIB]
	@ Define/NoLog GOBJECT 'Topdir'GLIB.GOBJECT]
.IFDEF GNUC
        @ GCC = "GCC" + F$Trnlnm("GCC_DEFINES")
.ENDIF

$(LIBTARGET) : $(LIBTARGET)($(OBJECTS))
        @ Write SYS$Output "Library gobjectlib.olb built"

Odir:gboxed.obj : gboxed.c gboxed.h gtype.h [-]glib.h \
	[-]galloca.h [-]gtypes.h [-]glibconfig.h \
	[-]gmacros.h [-]garray.h [-]gasyncqueue.h \
	[-]gthread.h [-]gerror.h [-]gquark.h \
	[-]gbacktrace.h [-]gcache.h [-]glist.h \
	[-]gmem.h [-]gcompletion.h [-]gconvert.h \
	[-]gdataset.h [-]gdate.h [-]gdir.h \
	[-]gfileutils.h [-]ghash.h [-]ghook.h \
	[-]giochannel.h [-]gmain.h [-]gslist.h \
	[-]gstring.h [-]gunicode.h [-]gmarkup.h \
	[-]gmessages.h [-]gnode.h [-]gpattern.h \
	[-]gprimes.h [-]gqsort.h [-]gqueue.h \
	[-]grand.h [-]grel.h [-]gscanner.h \
	[-]gshell.h [-]gspawn.h [-]gstrfuncs.h \
	[-]gthreadpool.h [-]gtimer.h [-]gtree.h \
	[-]gutils.h [-]gbsearcharray.h gvalue.h gvaluearray.h \
	gvalue.h gclosure.h gvaluecollector.h [-]glib-object.h \
	gboxed.h genums.h gobject.h gparam.h gclosure.h gsignal.h \
	gmarshal.h gparamspecs.h gsourceclosure.h gtypemodule.h \
	gtypeplugin.h gvaluearray.h gvaluetypes.h
Odir:gclosure.obj : gclosure.c gclosure.h gtype.h [-]glib.h \
	[-]galloca.h [-]gtypes.h [-]glibconfig.h \
	[-]gmacros.h [-]garray.h [-]gasyncqueue.h \
	[-]gthread.h [-]gerror.h [-]gquark.h \
	[-]gbacktrace.h [-]gcache.h [-]glist.h \
	[-]gmem.h [-]gcompletion.h [-]gconvert.h \
	[-]gdataset.h [-]gdate.h [-]gdir.h \
	[-]gfileutils.h [-]ghash.h [-]ghook.h \
	[-]giochannel.h [-]gmain.h [-]gslist.h \
	[-]gstring.h [-]gunicode.h [-]gmarkup.h \
	[-]gmessages.h [-]gnode.h [-]gpattern.h \
	[-]gprimes.h [-]gqsort.h [-]gqueue.h \
	[-]grand.h [-]grel.h [-]gscanner.h \
	[-]gshell.h [-]gspawn.h [-]gstrfuncs.h \
	[-]gthreadpool.h [-]gtimer.h [-]gtree.h \
	[-]gutils.h gvalue.h
Odir:genums.obj : genums.c genums.h gtype.h [-]glib.h \
	[-]galloca.h [-]gtypes.h [-]glibconfig.h \
	[-]gmacros.h [-]garray.h [-]gasyncqueue.h \
	[-]gthread.h [-]gerror.h [-]gquark.h \
	[-]gbacktrace.h [-]gcache.h [-]glist.h \
	[-]gmem.h [-]gcompletion.h [-]gconvert.h \
	[-]gdataset.h [-]gdate.h [-]gdir.h \
	[-]gfileutils.h [-]ghash.h [-]ghook.h \
	[-]giochannel.h [-]gmain.h [-]gslist.h \
	[-]gstring.h [-]gunicode.h [-]gmarkup.h \
	[-]gmessages.h [-]gnode.h [-]gpattern.h \
	[-]gprimes.h [-]gqsort.h [-]gqueue.h \
	[-]grand.h [-]grel.h [-]gscanner.h \
	[-]gshell.h [-]gspawn.h [-]gstrfuncs.h \
	[-]gthreadpool.h [-]gtimer.h [-]gtree.h \
	[-]gutils.h gvalue.h gvaluecollector.h \
	[-]glib-object.h gboxed.h genums.h gobject.h gvalue.h \
	gparam.h gclosure.h gsignal.h gmarshal.h gparamspecs.h \
	gsourceclosure.h gtypemodule.h gtypeplugin.h gvaluearray.h \
	gvaluetypes.h
Odir:gobject.obj : gobject.c gobject.h gtype.h [-]glib.h \
	[-]galloca.h [-]gtypes.h [-]glibconfig.h \
	[-]gmacros.h [-]garray.h [-]gasyncqueue.h \
	[-]gthread.h [-]gerror.h [-]gquark.h \
	[-]gbacktrace.h [-]gcache.h [-]glist.h \
	[-]gmem.h [-]gcompletion.h [-]gconvert.h \
	[-]gdataset.h [-]gdate.h [-]gdir.h \
	[-]gfileutils.h [-]ghash.h [-]ghook.h \
	[-]giochannel.h [-]gmain.h [-]gslist.h \
	[-]gstring.h [-]gunicode.h [-]gmarkup.h \
	[-]gmessages.h [-]gnode.h [-]gpattern.h \
	[-]gprimes.h [-]gqsort.h [-]gqueue.h \
	[-]grand.h [-]grel.h [-]gscanner.h \
	[-]gshell.h [-]gspawn.h [-]gstrfuncs.h \
	[-]gthreadpool.h [-]gtimer.h [-]gtree.h \
	[-]gutils.h gvalue.h gparam.h gclosure.h gsignal.h \
	gmarshal.h gvaluecollector.h [-]glib-object.h gboxed.h \
	genums.h gobject.h gparamspecs.h gsourceclosure.h gtypemodule.h \
	gtypeplugin.h gvaluearray.h gvaluetypes.h gsignal.h \
	gparamspecs.h gvaluetypes.h gobjectnotifyqueue.c
Odir:gparam.obj : gparam.c gparam.h gvalue.h gtype.h [-]glib.h \
	[-]galloca.h [-]gtypes.h [-]glibconfig.h \
	[-]gmacros.h [-]garray.h [-]gasyncqueue.h \
	[-]gthread.h [-]gerror.h [-]gquark.h \
	[-]gbacktrace.h [-]gcache.h [-]glist.h \
	[-]gmem.h [-]gcompletion.h [-]gconvert.h \
	[-]gdataset.h [-]gdate.h [-]gdir.h \
	[-]gfileutils.h [-]ghash.h [-]ghook.h \
	[-]giochannel.h [-]gmain.h [-]gslist.h \
	[-]gstring.h [-]gunicode.h [-]gmarkup.h \
	[-]gmessages.h [-]gnode.h [-]gpattern.h \
	[-]gprimes.h [-]gqsort.h [-]gqueue.h \
	[-]grand.h [-]grel.h [-]gscanner.h \
	[-]gshell.h [-]gspawn.h [-]gstrfuncs.h \
	[-]gthreadpool.h [-]gtimer.h [-]gtree.h \
	[-]gutils.h gvaluecollector.h [-]glib-object.h \
	gboxed.h genums.h gobject.h gparam.h gclosure.h gsignal.h \
	gmarshal.h gparamspecs.h gsourceclosure.h gtypemodule.h \
	gtypeplugin.h gvaluearray.h gvaluetypes.h
Odir:gparamspecs.obj : gparamspecs.c [-]config.h gparamspecs.h \
	gvalue.h gtype.h [-]glib.h [-]galloca.h \
	[-]gtypes.h [-]glibconfig.h [-]gmacros.h \
	[-]garray.h [-]gasyncqueue.h [-]gthread.h \
	[-]gerror.h [-]gquark.h [-]gbacktrace.h \
	[-]gcache.h [-]glist.h [-]gmem.h \
	[-]gcompletion.h [-]gconvert.h [-]gdataset.h \
	[-]gdate.h [-]gdir.h [-]gfileutils.h \
	[-]ghash.h [-]ghook.h [-]giochannel.h \
	[-]gmain.h [-]gslist.h [-]gstring.h \
	[-]gunicode.h [-]gmarkup.h [-]gmessages.h \
	[-]gnode.h [-]gpattern.h [-]gprimes.h \
	[-]gqsort.h [-]gqueue.h [-]grand.h \
	[-]grel.h [-]gscanner.h [-]gshell.h \
	[-]gspawn.h [-]gstrfuncs.h [-]gthreadpool.h \
	[-]gtimer.h [-]gtree.h [-]gutils.h genums.h \
	gboxed.h gobject.h gparam.h gclosure.h gsignal.h gmarshal.h \
	gvaluecollector.h [-]glib-object.h gparamspecs.h \
	gsourceclosure.h gtypemodule.h gtypeplugin.h gvaluearray.h \
	gvaluetypes.h gvaluearray.h
Odir:gsignal.obj : gsignal.c [-]config.h gsignal.h gclosure.h \
	gtype.h [-]glib.h [-]galloca.h [-]gtypes.h \
	[-]glibconfig.h [-]gmacros.h [-]garray.h \
	[-]gasyncqueue.h [-]gthread.h [-]gerror.h \
	[-]gquark.h [-]gbacktrace.h [-]gcache.h \
	[-]glist.h [-]gmem.h [-]gcompletion.h \
	[-]gconvert.h [-]gdataset.h [-]gdate.h \
	[-]gdir.h [-]gfileutils.h [-]ghash.h \
	[-]ghook.h [-]giochannel.h [-]gmain.h \
	[-]gslist.h [-]gstring.h [-]gunicode.h \
	[-]gmarkup.h [-]gmessages.h [-]gnode.h \
	[-]gpattern.h [-]gprimes.h [-]gqsort.h \
	[-]gqueue.h [-]grand.h [-]grel.h \
	[-]gscanner.h [-]gshell.h [-]gspawn.h \
	[-]gstrfuncs.h [-]gthreadpool.h [-]gtimer.h \
	[-]gtree.h [-]gutils.h gvalue.h gparam.h gmarshal.h \
	[-]gbsearcharray.h gvaluecollector.h [-]glib-object.h \
	gboxed.h genums.h gobject.h gsignal.h gparamspecs.h \
	gsourceclosure.h gtypemodule.h gtypeplugin.h gvaluearray.h \
	gvaluetypes.h gvaluetypes.h gboxed.h gobject.h genums.h \
	gmarshal.c
Odir:gsourceclosure.obj : gsourceclosure.c gsourceclosure.h \
	gclosure.h gtype.h [-]glib.h [-]galloca.h \
	[-]gtypes.h [-]glibconfig.h [-]gmacros.h \
	[-]garray.h [-]gasyncqueue.h [-]gthread.h \
	[-]gerror.h [-]gquark.h [-]gbacktrace.h \
	[-]gcache.h [-]glist.h [-]gmem.h \
	[-]gcompletion.h [-]gconvert.h [-]gdataset.h \
	[-]gdate.h [-]gdir.h [-]gfileutils.h \
	[-]ghash.h [-]ghook.h [-]giochannel.h \
	[-]gmain.h [-]gslist.h [-]gstring.h \
	[-]gunicode.h [-]gmarkup.h [-]gmessages.h \
	[-]gnode.h [-]gpattern.h [-]gprimes.h \
	[-]gqsort.h [-]gqueue.h [-]grand.h \
	[-]grel.h [-]gscanner.h [-]gshell.h \
	[-]gspawn.h [-]gstrfuncs.h [-]gthreadpool.h \
	[-]gtimer.h [-]gtree.h [-]gutils.h gboxed.h \
	genums.h gmarshal.h gvalue.h gvaluetypes.h gvalue.h
Odir:gtype.obj : gtype.c [-]config.h gtype.h [-]glib.h \
	[-]galloca.h [-]gtypes.h [-]glibconfig.h \
	[-]gmacros.h [-]garray.h [-]gasyncqueue.h \
	[-]gthread.h [-]gerror.h [-]gquark.h \
	[-]gbacktrace.h [-]gcache.h [-]glist.h \
	[-]gmem.h [-]gcompletion.h [-]gconvert.h \
	[-]gdataset.h [-]gdate.h [-]gdir.h \
	[-]gfileutils.h [-]ghash.h [-]ghook.h \
	[-]giochannel.h [-]gmain.h [-]gslist.h \
	[-]gstring.h [-]gunicode.h [-]gmarkup.h \
	[-]gmessages.h [-]gnode.h [-]gpattern.h \
	[-]gprimes.h [-]gqsort.h [-]gqueue.h \
	[-]grand.h [-]grel.h [-]gscanner.h \
	[-]gshell.h [-]gspawn.h [-]gstrfuncs.h \
	[-]gthreadpool.h [-]gtimer.h [-]gtree.h \
	[-]gutils.h gtypeplugin.h gtype.h gvaluecollector.h \
	[-]glib-object.h gboxed.h genums.h gobject.h gvalue.h \
	gparam.h gclosure.h gsignal.h gmarshal.h gparamspecs.h \
	gsourceclosure.h gtypemodule.h gtypeplugin.h gvaluearray.h \
	gvaluetypes.h
Odir:gtypemodule.obj : gtypemodule.c gtypeplugin.h gtype.h \
	[-]glib.h [-]galloca.h [-]gtypes.h \
	[-]glibconfig.h [-]gmacros.h [-]garray.h \
	[-]gasyncqueue.h [-]gthread.h [-]gerror.h \
	[-]gquark.h [-]gbacktrace.h [-]gcache.h \
	[-]glist.h [-]gmem.h [-]gcompletion.h \
	[-]gconvert.h [-]gdataset.h [-]gdate.h \
	[-]gdir.h [-]gfileutils.h [-]ghash.h \
	[-]ghook.h [-]giochannel.h [-]gmain.h \
	[-]gslist.h [-]gstring.h [-]gunicode.h \
	[-]gmarkup.h [-]gmessages.h [-]gnode.h \
	[-]gpattern.h [-]gprimes.h [-]gqsort.h \
	[-]gqueue.h [-]grand.h [-]grel.h \
	[-]gscanner.h [-]gshell.h [-]gspawn.h \
	[-]gstrfuncs.h [-]gthreadpool.h [-]gtimer.h \
	[-]gtree.h [-]gutils.h gtypemodule.h gobject.h \
	gvalue.h gparam.h gclosure.h gsignal.h gmarshal.h
Odir:gtypeplugin.obj : gtypeplugin.c gtypeplugin.h gtype.h \
	[-]glib.h [-]galloca.h [-]gtypes.h \
	[-]glibconfig.h [-]gmacros.h [-]garray.h \
	[-]gasyncqueue.h [-]gthread.h [-]gerror.h \
	[-]gquark.h [-]gbacktrace.h [-]gcache.h \
	[-]glist.h [-]gmem.h [-]gcompletion.h \
	[-]gconvert.h [-]gdataset.h [-]gdate.h \
	[-]gdir.h [-]gfileutils.h [-]ghash.h \
	[-]ghook.h [-]giochannel.h [-]gmain.h \
	[-]gslist.h [-]gstring.h [-]gunicode.h \
	[-]gmarkup.h [-]gmessages.h [-]gnode.h \
	[-]gpattern.h [-]gprimes.h [-]gqsort.h \
	[-]gqueue.h [-]grand.h [-]grel.h \
	[-]gscanner.h [-]gshell.h [-]gspawn.h \
	[-]gstrfuncs.h [-]gthreadpool.h [-]gtimer.h \
	[-]gtree.h [-]gutils.h
Odir:gvalue.obj : gvalue.c gvalue.h gtype.h [-]glib.h \
	[-]galloca.h [-]gtypes.h [-]glibconfig.h \
	[-]gmacros.h [-]garray.h [-]gasyncqueue.h \
	[-]gthread.h [-]gerror.h [-]gquark.h \
	[-]gbacktrace.h [-]gcache.h [-]glist.h \
	[-]gmem.h [-]gcompletion.h [-]gconvert.h \
	[-]gdataset.h [-]gdate.h [-]gdir.h \
	[-]gfileutils.h [-]ghash.h [-]ghook.h \
	[-]giochannel.h [-]gmain.h [-]gslist.h \
	[-]gstring.h [-]gunicode.h [-]gmarkup.h \
	[-]gmessages.h [-]gnode.h [-]gpattern.h \
	[-]gprimes.h [-]gqsort.h [-]gqueue.h \
	[-]grand.h [-]grel.h [-]gscanner.h \
	[-]gshell.h [-]gspawn.h [-]gstrfuncs.h \
	[-]gthreadpool.h [-]gtimer.h [-]gtree.h \
	[-]gutils.h gvaluecollector.h [-]glib-object.h \
	gboxed.h genums.h gobject.h gvalue.h gparam.h gclosure.h \
	gsignal.h gmarshal.h gparamspecs.h gsourceclosure.h \
	gtypemodule.h gtypeplugin.h gvaluearray.h gvaluetypes.h \
	[-]gbsearcharray.h
Odir:gvaluearray.obj : gvaluearray.c [-]config.h gvaluearray.h \
	gvalue.h gtype.h [-]glib.h [-]galloca.h \
	[-]gtypes.h [-]glibconfig.h [-]gmacros.h \
	[-]garray.h [-]gasyncqueue.h [-]gthread.h \
	[-]gerror.h [-]gquark.h [-]gbacktrace.h \
	[-]gcache.h [-]glist.h [-]gmem.h \
	[-]gcompletion.h [-]gconvert.h [-]gdataset.h \
	[-]gdate.h [-]gdir.h [-]gfileutils.h \
	[-]ghash.h [-]ghook.h [-]giochannel.h \
	[-]gmain.h [-]gslist.h [-]gstring.h \
	[-]gunicode.h [-]gmarkup.h [-]gmessages.h \
	[-]gnode.h [-]gpattern.h [-]gprimes.h \
	[-]gqsort.h [-]gqueue.h [-]grand.h \
	[-]grel.h [-]gscanner.h [-]gshell.h \
	[-]gspawn.h [-]gstrfuncs.h [-]gthreadpool.h \
	[-]gtimer.h [-]gtree.h [-]gutils.h
Odir:gvaluetransform.obj : gvaluetransform.c gvalue.h \
	gtype.h [-]glib.h [-]galloca.h [-]gtypes.h \
	[-]glibconfig.h [-]gmacros.h [-]garray.h \
	[-]gasyncqueue.h [-]gthread.h [-]gerror.h \
	[-]gquark.h [-]gbacktrace.h [-]gcache.h \
	[-]glist.h [-]gmem.h [-]gcompletion.h \
	[-]gconvert.h [-]gdataset.h [-]gdate.h \
	[-]gdir.h [-]gfileutils.h [-]ghash.h \
	[-]ghook.h [-]giochannel.h [-]gmain.h \
	[-]gslist.h [-]gstring.h [-]gunicode.h \
	[-]gmarkup.h [-]gmessages.h [-]gnode.h \
	[-]gpattern.h [-]gprimes.h [-]gqsort.h \
	[-]gqueue.h [-]grand.h [-]grel.h \
	[-]gscanner.h [-]gshell.h [-]gspawn.h \
	[-]gstrfuncs.h [-]gthreadpool.h [-]gtimer.h \
	[-]gtree.h [-]gutils.h genums.h
Odir:gvaluetypes.obj : gvaluetypes.c gvaluetypes.h gvalue.h \
	gtype.h [-]glib.h [-]galloca.h [-]gtypes.h \
	[-]glibconfig.h [-]gmacros.h [-]garray.h \
	[-]gasyncqueue.h [-]gthread.h [-]gerror.h \
	[-]gquark.h [-]gbacktrace.h [-]gcache.h \
	[-]glist.h [-]gmem.h [-]gcompletion.h \
	[-]gconvert.h [-]gdataset.h [-]gdate.h \
	[-]gdir.h [-]gfileutils.h [-]ghash.h \
	[-]ghook.h [-]giochannel.h [-]gmain.h \
	[-]gslist.h [-]gstring.h [-]gunicode.h \
	[-]gmarkup.h [-]gmessages.h [-]gnode.h \
	[-]gpattern.h [-]gprimes.h [-]gqsort.h \
	[-]gqueue.h [-]grand.h [-]grel.h \
	[-]gscanner.h [-]gshell.h [-]gspawn.h \
	[-]gstrfuncs.h [-]gthreadpool.h [-]gtimer.h \
	[-]gtree.h [-]gutils.h gvaluecollector.h \
	[-]glib-object.h gboxed.h genums.h gobject.h gparam.h \
	gclosure.h gsignal.h gmarshal.h gparamspecs.h gsourceclosure.h \
	gtypemodule.h gtypeplugin.h gvaluearray.h gvaluetypes.h \
	gobject.h gparam.h gboxed.h genums.h

.c.obj :
        $(CC)$(CFLAGS)$(CFLOAT)/OBJECT=$@ $<

.obj.olb
        $(LIBR) $(LIBRFLAGS) $(MMS$TARGET) $(MMS$SOURCE)

clean :
        Delete/Log $(WDIR)*.OBJ;*
        Delete/Log $(LIBTARGET);*
