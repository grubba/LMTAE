/*
 * $Id: recomp.h,v 1.4 1996/07/13 19:32:12 grubba Exp $
 *
 * $Log: recomp.h,v $
 * Revision 1.3  1996/07/12 21:11:52  grubba
 * raise_exception() might work a bit better now.
 * Although at the moment it does an abort() instead of returning.
 *
 * Revision 1.2  1996/07/08 21:17:42  grubba
 * reset_hw() added.
 *
 * Revision 1.1.1.1  1996/06/30 23:51:50  grubba
 * Entry into CVS
 *
 * Revision 1.5  1996/06/19 11:08:25  grubba
 * *** empty log message ***
 *
 * Revision 1.4  1996/06/01 09:31:29  grubba
 *  Major changes.
 * Now generates output split into several files.
 * Now generates gasp assembler preprocessor output.
 * Now uses templates to build instructions.
 *
 * Revision 1.3  1996/05/12 16:39:55  grubba
 * Added some more prototypes.
 *
 * Revision 1.2  1996/05/06 14:50:54  grubba
 * Added m68k.h
 *
 * Revision 1.1  1996/05/06 11:19:57  grubba
 * Initial revision
 *
 */

#ifndef RECOMP_H
#define RECOMP_H

/* Common types used */

typedef long LONG;
typedef unsigned long ULONG;
typedef short SHORT;
typedef unsigned short USHORT;
typedef signed char BYTE;
typedef unsigned char UBYTE;

#ifndef NULL
#define NULL ((void *)0)
#endif

/*
 * The global m68000 memory
 */
extern unsigned char *memory;

/*
 * The compiler lookup-table
 */
extern int (*comp_tab[])(ULONG *, ULONG **, USHORT *, USHORT);

/*
 * Debug level
 *
 * This is a bitfield.
 */

extern ULONG debuglevel;

#define DL_COMPILER_VERBOSE	1
#define DL_COMPILER_DISASSEMBLY	2
#define DL_RUNTIME_TRACE	4

/*
 * Prototypes
 */

struct m_registers;
struct code_info;

struct code_info *new_codeinfo(ULONG maddr);

ULONG raise_exception(struct m_registers *regs, USHORT *mem, ULONG vec);
volatile void compile_and_go(struct m_registers *regs, ULONG maddr);
ULONG compile(struct code_info *ci);
void disassemble(ULONG start, ULONG end);

void reset_hw(void);

ULONG read_hw_byte(ULONG maddr);
ULONG read_hw_short(ULONG maddr);
ULONG read_hw(ULONG maddr);

void store_hw_byte(ULONG maddr, UBYTE val);
void store_hw_short(ULONG maddr, USHORT val);
void store_hw(ULONG maddr, ULONG val);

ULONG clobber_code_byte(ULONG maddr, UBYTE val);
ULONG clobber_code_short(ULONG maddr, USHORT val);
void clobber_code(ULONG maddr, ULONG val);

void emit_exception(ULONG **code, ULONG pc, UBYTE vec);

#endif /* RECOMP_H */
