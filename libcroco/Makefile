LIBTARGET = libcroco.a

all: $(LIBTARGET)

CFILES = cr-additional-sel.c cr-attr-sel.c cr-cascade.c \
cr-declaration.c cr-doc-handler.c cr-enc-handler.c \
cr-fonts.c cr-input.c cr-num.c cr-om-parser.c cr-parser.c \
cr-parsing-location.c cr-prop-list.c cr-pseudo.c cr-rgb.c \
cr-selector.c cr-sel-eng.c cr-simple-sel.c cr-statement.c \
cr-string.c cr-style.c cr-stylesheet.c cr-term.c \
cr-tknzr.c cr-token.c cr-utils.c

OBJS = $(CFILES:.c=.o)

$(LIBTARGET): $(OBJS)
	-rm -f $(LIBTARGET)
	ar rv $(LIBTARGET) $(OBJS)
	$(RANLIB) $(LIBTARGET)

$(OBJS): libcroco-config.h

clean:
	rm $(LIBTARGET) *.o
tags:
	etags -t *.[ch]
