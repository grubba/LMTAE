#
# $Id: Makefile,v 1.12 1996/07/14 21:44:13 grubba Exp $
#
# Makefile for the M68000 to Sparc recompiler
#
# $Log: Makefile,v $
# Revision 1.11  1996/07/13 19:32:21  grubba
# Now defaults to very little debuginfo.
# Added (un|set)patch().
# Patches added to MakeLibrary(), MakeFunctions(), Abort() and AddLibrary().
#
# Revision 1.10  1996/07/13 12:45:52  grubba
# Now possible to build on hanna.
#
# Revision 1.9  1996/07/13 12:17:24  grubba
# /usr/local/lib added to library search path to make it possible to run it on hanna.
#
# Revision 1.8  1996/07/12 13:10:03  marcus
# Added rules for the boards subdirectory
#
# Revision 1.7  1996/07/11 23:01:58  marcus
# Real ZorroII emulation
#
# Revision 1.6  1996/07/11 15:41:48  grubba
# Now has a GUI!
# Some bug-fixes.
#
# Revision 1.5  1996/07/08 21:22:41  grubba
# Added disassembler.
#
# Revision 1.4  1996/07/01 19:16:33  grubba
# Implemented ASL and ASR.
# Changed semantics for new_codeinfo(), it doesn't allocate space for the code.
# Added PeepHoleOptimize(). At the moment it just mallocs and copies the code.
# Removed some warnings.
#
# Revision 1.3  1996/07/01 16:20:44  marcus
# Blitter emulation imported into CVS repository.
#
# Revision 1.2  1996/07/01 00:13:53  grubba
# It is now possible to compile from scratch.
#
# Revision 1.1.1.1  1996/06/30 23:51:51  grubba
# Entry into CVS
#
# Revision 1.4  1996/06/20 22:10:58  grubba
# Now compiles the opcode table in parts.
# Now only recompiles a gasp file if it has changed sinc the last time.
#
# Revision 1.3  1996/06/19 11:08:25  grubba
# Now with hardware support.
#
# Revision 1.2  1996/06/01 09:31:29  grubba
#  Major changes.
# Now generates output split into several files.
# Now generates gasp assembler preprocessor output.
# Now uses templates to build instructions.
#
# Revision 1.1  1996/05/12 16:43:40  grubba
# Initial revision
#
#

AS = gas
CC = gcc
GASP = gasp
RM = rm -f
CFLAGS = -g -O4711 -Wall -pedantic -DDEBUG \
	-IAmigaInclude -I/usr/openwin/include -I/usr/X11/include
LDFLAGS = -L/usr/lib -L/usr/openwin/lib -L/usr/X11/lib -L/usr/local/lib\
	-R/usr/lib:/usr/openwin/lib:/usr/X11/lib:/usr/local/lib
LDLIBS = -lthread -ldl -lX -lX11 -lXpm

OPCODES = opcodes/opcode_0000.o opcodes/opcode_1000.o \
	  opcodes/opcode_2000.o opcodes/opcode_3000.o \
	  opcodes/opcode_4000.o opcodes/opcode_5000.o \
	  opcodes/opcode_6000.o opcodes/opcode_7000.o \
	  opcodes/opcode_8000.o opcodes/opcode_9000.o \
	  opcodes/opcode_a000.o opcodes/opcode_b000.o \
	  opcodes/opcode_c000.o opcodes/opcode_d000.o \
	  opcodes/opcode_e000.o opcodes/opcode_f000.o

TABLES = tables/opcode_tab_0000.o tables/opcode_tab_1000.o \
	tables/opcode_tab_2000.o tables/opcode_tab_3000.o \
	tables/opcode_tab_4000.o tables/opcode_tab_5000.o \
	tables/opcode_tab_6000.o tables/opcode_tab_7000.o \
	tables/opcode_tab_8000.o tables/opcode_tab_9000.o \
	tables/opcode_tab_a000.o tables/opcode_tab_b000.o \
	tables/opcode_tab_c000.o tables/opcode_tab_d000.o \
	tables/opcode_tab_e000.o tables/opcode_tab_f000.o \
	tables/more_tables.o

all : AmigaInclude ROM.dump citest compgen recomp rtest allboards

opcodes :
	mkdir opcodes

tables :
	mkdir tables

AmigaInclude :
	ln -s /users/grubba/AmigaInclude .

ROM.dump :
	rm -f ROM.dump
	ln -s /users/grubba/AmigaROM/kick37175.A500 ROM.dump

allboards :
	@(cd boards; $(MAKE) all)

clean :
	-$(RM) *.o opcodes/*.o templates/*.o tables/*.o
	-$(RM) memory.gasp opcodes.h opcode_table.gasp
	-$(RM) opcodes/*.gasp tables/*.gasp
	-$(RM) *.gasp.old */*.gasp.old
	-$(RM) boards/*.so boards/*.rom
	-$(RM) blitgen blittest minterms.c

ci :	clean
	-ci -l *.c *.h templates/*.gasp templates/*.h Makefile templates/Makefile

rtest : rtest.o compglue.o peephole.o opcodes.o\
	templates/glue.o memory.o opcodes.o tables.o

recomp : recomp.o compglue.o peephole.o codeinfo.o\
	hardware.o cia.o zorro.o\
	disassembler.o sanity.o setpatch.o\
	gui/gui.o gui/regdump.o gui/info.o gui/disassembler.o\
	opcodes.o memory.o templates/glue.o opcodes.o tables.o

citest : citest.o codeinfo.o

compgen : compgen.o

opcodes.o : $(OPCODES)
	ld -r -o $@ $^

tables.o : $(TABLES)
	ld -r -o $@ $^

opcodes/%.gasp : compgen
	./compgen --gasp

tables/%.gasp : compgen
	./compgen --tables

opcodes.h : compgen
	./compgen --header

memory.gasp : compgen
	./compgen --memory

blitgen : blitgen.o

minterms.c : blitgen
	./blitgen > minterms.c

blittest : blittest.o blitter.o

blitter.o : sparcblit.o minterms.o
	ld -r -o $@ $^

%.o : %.gasp
	@if cmp $< $<.old 2>&1 >/dev/null; then \
		if [ -f $@ ]; then \
			echo No recompile nesessary for $<; \
			touch $@; \
			exit 0; \
		fi; \
	fi; \
	echo '$(GASP) $< | $(AS) $(ASFLAGS) -o $@'; \
	if $(GASP) $< | $(AS) $(ASFLAGS) -o $@; then \
		cp $< $<.old; \
	fi; \
	exit $$?

compglue.o : opcodes.h

compgen.o : recomp.h m68k.h codeinfo.h

citest.o : recomp.h m68k.h codeinfo.h

codeinfo.o : recomp.h m68k.h codeinfo.h

blitgen.o : blitgen.c

minterms.o : minterms.c blitter.h common.h

sparcblit.o : sparcblit.c blitter.h common.h

blittest.o: blittest.c blitter.h common.h

zorro.o: zorro.c common.h recomp.h
