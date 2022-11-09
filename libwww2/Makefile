LIBTARGET = libwww.a

all: $(LIBTARGET)

CFILES = HTAABrow.c HTAAUtil.c HTAccess.c \
  HTAlert.c HTAnchor.c HTAssoc.c HTAtom.c \
  HTChunk.c HTCompressed.c HTFile.c HTFormat.c \
  HTFTP.c HTFWriter.c HTGopher.c HTIcon.c \
  HTInit.c HTList.c HTMailto.c HTMIME.c \
  HTML.c HTMLDTD.c HTMLGen.c HTMosaicHTML.c \
  HTNews.c HTParse.c HTPlain.c HTSort.c \
  HTString.c HTTCP.c HTTelnet.c HTTP.c \
  HTUU.c HTWriter.c HTWSRC.c \
  HTBTree.c HTFinger.c HTCookie.c SGML.c

OBJS = $(CFILES:.c=.o)

$(LIBTARGET): $(OBJS)
	-rm -f $(LIBTARGET)
	ar rv $(LIBTARGET) $(OBJS)
	$(RANLIB) $(LIBTARGET)

$(OBJS): HTUtils.h
HTFTP.o: HTFTP.h

clean:
	rm $(LIBTARGET) *.o
tags:
	etags -t *.[ch]
