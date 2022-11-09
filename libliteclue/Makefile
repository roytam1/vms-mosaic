#
# Makefile for libliteclue
#

CC=gcc
CFLAGS=-g -O
ODIR=./objs
OBJDIR=objs
LIB=libliteclue.a
####Set in main Mosaic Makefile -- uncomment if not compiled under Mosaic
#RANLIB=ranlib
OBJ=$(ODIR)/LiteClue.o
all: objs $(LIB)

$(LIB): $(OBJ)
	-rm -f $(LIB)
	ar rv $(LIB) $(OBJ)
	$(RANLIB) $(LIB)

$(ODIR)/LiteClue.o: LiteClue.c LiteClue.h LiteClueP.h
	$(CC) $(CFLAGS) -c LiteClue.c -o $(ODIR)/LiteClue.o

objs:
	mkdir objs

clean:
	rm -f $(LIB) $(OBJ)
