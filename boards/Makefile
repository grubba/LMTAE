#
# $Id: Makefile,v 1.3 1998/02/10 01:01:01 marcus Exp $
#
# $Log: Makefile,v $
# Revision 1.2  1997/08/29 00:10:08  grubba
# Moved to Infovav.
#
# Revision 1.1  1996/07/12 13:09:07  marcus
# Created a hard disk emulating Zorro board
#
#

CC = gcc
RM = rm -f
CFLAGS = -g -O4711 -Wall -pedantic -DDEBUG -I.. -I../AmigaInclude


all : hardfile.so hardfile.rom

%.so : %.c
	$(CC) $(CFLAGS) -Wl,-G $< -o $@

hardfile.so : hardfile.c ../common.h ../recomp.h

hardfile.rom : /home/marcus/tmp/hardfile.rom
	cp $< $@
