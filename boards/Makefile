#
# $Id: Makefile,v 1.1 1996/07/12 13:09:07 marcus Exp $
#
# $Log: $
#

CC = gcc
RM = rm -f
CFLAGS = -g -O4711 -Wall -pedantic -DDEBUG -I.. -I../AmigaInclude


all : hardfile.so hardfile.rom

%.so : %.c
	$(CC) $(CFLAGS) -Wl,-r $< -o $@

hardfile.so : hardfile.c ../common.h ../recomp.h

hardfile.rom : /home/lysdisk2/marcus/tmp/hardfile.rom
	cp $< $@
