LIBTARGET = libhtmlw.a

all: $(LIBTARGET)

CFILES = HTML.c HTMLlists.c HTMLformat.c HTMLparse.c HTMLimages.c HTMLwidgets.c HTML-PSformat.c HTMLtable.c HTMLapplet.c HTMLfont.c HTMLform.c HTMLframe.c HTMLimagemap.c HTMLtext.c list.c 

OBJS = $(CFILES:.c=.o)
$(OBJS): HTML.h HTMLP.h


$(LIBTARGET): $(OBJS)
	-rm -f $(LIBTARGET)
	ar rv $(LIBTARGET) $(OBJS)
	$(RANLIB) $(LIBTARGET)

HTMLimages.o: AnchoredImage.xbm DelayedImage.xbm

clean:
	-rm $(LIBTARGET) *.o
tags:
	etags -t *.[ch]
