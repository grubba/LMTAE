/*
 * $Id: recomp.h,v 1.7 1996/08/11 17:36:19 grubba Exp $
 *
 * $Log: recomp.h,v $
 * Revision 1.6  1996/08/11 14:49:03  grubba
 * Added option to turn off SR optimization.
 *
 * Revision 1.5  1996/07/17 16:01:48  grubba
 * Changed from {U,}{LONG,WORD,BYTE} to [SU]{8,16,32}.
 * Hopefully all places got patched.
 *
 * Revision 1.4  1996/07/13 19:32:12  grubba
 * Now defaults to very little debuginfo.
 * Added (un|set)patch().
 * Patches added to MakeLibrary(), MakeFunctions(), Abort() and AddLibrary().
 *
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
#ifndef TYPES_H
#include "types.h"
#endif /* TYPES_H */

#ifndef NULL
#define NULL ((void *)0)
#endif

/*
 * The global m68000 memory
 */
extern U8 *memory;

/*
 * The compiler lookup-table
 */
extern U32 (*comp_tab[])(U32 *, U32 **, U16 *, U16);

/*
 * Debug level
 *
 * This is a bitfield.
 */

extern U32 debuglevel;

#define DL_COMPILER_VERBOSE	1
#define DL_COMPILER_DISASSEMBLY	2
#define DL_RUNTIME_TRACE	4
#define DL_NO_SR_OPTIMIZATION	8
#define DL_STATISTICS		16

/*
 * Prototypes
 */

struct m_registers;
struct code_info;

struct code_info *new_codeinfo(U32 maddr);

U32 raise_exception(struct m_registers *regs, U16 *mem, U32 vec);
volatile void compile_and_go(struct m_registers *regs, U32 maddr);
U32 compile(struct code_info *ci);
void disassemble(U32 start, U32 end);

void reset_hw(void);

U32 read_hw_byte(U32 maddr);
U32 read_hw_short(U32 maddr);
U32 read_hw(U32 maddr);

void store_hw_byte(U32 maddr, U8 val);
void store_hw_short(U32 maddr, U16 val);
void store_hw(U32 maddr, U32 val);

U32 clobber_code_byte(U32 maddr, U8 val);
U32 clobber_code_short(U32 maddr, U16 val);
void clobber_code(U32 maddr, U32 val);

void emit_exception(U32 **code, U32 pc, U8 vec);

#endif /* RECOMP_H */
