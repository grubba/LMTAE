/*
 * $Id: compgen.c,v 1.18 1996/07/17 19:16:23 grubba Exp $
 *
 * Compilergenerator. Generates a compiler from M68000 to Sparc binary code.
 *
 * $Log: compgen.c,v $
 * Revision 1.17  1996/07/17 16:01:15  grubba
 * Changed from {U,}{LONG,WORD,BYTE} to [SU]{8,16,32}.
 * Hopefully all places got patched.
 *
 * Revision 1.16  1996/07/16 23:29:17  grubba
 * MOVE_USP was broken -- It only saved the lowest byte of the USP.
 *
 * Revision 1.15  1996/07/16 22:42:39  grubba
 * Fixed typo in the last patch.
 *
 * Revision 1.14  1996/07/16 22:36:56  grubba
 * The last bug-fix saved the registers to memory instead of the registerbank!
 * Fixed same problem with XORI_SR.
 *
 * Revision 1.13  1996/07/16 22:28:54  grubba
 * Fixed major bug with ANDI_SR -- It didn't flip stacks when going to Usermode.
 * Implemented ADDX.
 *
 * Revision 1.12  1996/07/15 20:32:10  grubba
 * Cleaned up shift code.
 * Implemented C & X in ASR and LSR.
 *
 * Revision 1.11  1996/07/15 14:35:22  grubba
 * Added Left and Right to the various shift mnemonics.
 *
 * Revision 1.10  1996/07/14 15:14:58  grubba
 * Fixed bug with ROd.
 * Added MULS.
 * MULU now modifies SR.
 *
 * Revision 1.9  1996/07/08 21:22:13  grubba
 * Added TEF_SRC_QUICK8 in some places.
 *
 * Revision 1.8  1996/07/07 13:29:18  grubba
 * Removed the obsolete dis_*() functions, and their entry in the opcode_info struct.
 *
 * Revision 1.7  1996/07/04 18:08:23  grubba
 * Added DIVU and EXT.
 *
 * Revision 1.6  1996/07/03 15:39:47  grubba
 * Added opcode NEG.
 *
 * Revision 1.5  1996/07/03 15:01:14  grubba
 * Fixed a bug with OR Dn, <ea>. The mask was wrong.
 *
 * Revision 1.4  1996/07/02 22:17:31  grubba
 * The immediate shift opcodes didn't shift when the arg was 0 i.e. 8.
 *
 * Revision 1.3  1996/07/01 19:16:44  grubba
 * Implemented ASL and ASR.
 * Changed semantics for new_codeinfo(), it doesn't allocate space for the code.
 * Added PeepHoleOptimize(). At the moment it just mallocs and copies the code.
 * Removed some warnings.
 *
 * Revision 1.2  1996/07/01 11:44:26  grubba
 * Missing opcodes NBCD and PEA added to the table.
 * Opcode PEA implemented.
 *
 * Revision 1.1.1.1  1996/06/30 23:51:52  grubba
 * Entry into CVS
 *
 * Revision 1.20  1996/06/30 23:03:39  grubba
 * Fixed some more bugs.
 * Fixed a major bug with RTE.
 * Implemented MULU, LINE_F.
 * In some places SR wasn't saved on return.
 * Attempts to start Schedule()!
 *
 * Revision 1.19  1996/06/30 01:20:31  grubba
 * Fixed a bug in EXG the table mask was wrong, it always wrote one result to D0.
 * There seems to be a bug with BGT or CMP.
 *
 * Revision 1.18  1996/06/25 21:30:11  grubba
 * Fixed bug in UNLK -- it missed a TEF_DST_LOAD.
 * New implementation of MOVEM -- Kludgy, but it generates better code.
 *
 * Revision 1.17  1996/06/24 21:28:23  grubba
 * MOVEM might still be broken.
 *
 * Revision 1.16  1996/06/24 19:15:31  grubba
 * Fixed many subtle bugs in the tab_* and as_* functions.
 *
 * Revision 1.15  1996/06/24 18:02:02  grubba
 * Fixed a major bug with LSR/LSL. They shifted in the wrong direction.
 *
 * Revision 1.14  1996/06/23 22:05:02  grubba
 * Now uses (d16, PC) addressing mode for short branches.
 * Removed compgen version information from the generated files -- less to recompile.
 *
 * Revision 1.13  1996/06/23 20:34:10  grubba
 * Fixed a bug with 16bit conditional branches.
 *
 * Revision 1.12  1996/06/20 22:14:31  grubba
 * Fixed bugs in the implementation of LINK and UNLK.
 *
 * Revision 1.11  1996/06/20 22:09:51  grubba
 * Fixed bugs with short backward branches.
 * Opcode table is now compiled in parts.
 * Implemented LINK, UNLK, ADDA and others.
 *
 * Revision 1.10  1996/06/20 16:48:44  grubba
 * Implemented SUBI.
 * Next version will hopefully split the opcode_table into smaller chunks, that
 * gas can compile separately.
 *
 * Revision 1.9  1996/06/19 11:08:25  grubba
 * Fixed several bugs.
 * Added several new opcodes.
 *
 * Revision 1.8  1996/06/07 21:42:09  grubba
 * Seems to work somewhat.
 * Next version will create tables in an assembler file instead.
 *
 * Revision 1.7  1996/06/06 11:36:37  grubba
 * Seems to generate correct code.
 * Able to run the ROM checksum-test.
 * MOVE not implemented yet.
 *
 * Revision 1.6  1996/06/01 09:31:29  grubba
 * Major changes.
 * Now generates output split into several files.
 * Now generates gasp assembler preprocessor output.
 * Now uses templates to build instructions.
 *
 * Revision 1.5  1996/05/19 23:41:02  grubba
 * Added UNLK.
 *
 * Revision 1.4  1996/05/18 14:27:21  grubba
 * Now with DBcc and Bcc.
 *
 * Revision 1.3  1996/05/14 21:39:07  grubba
 * Added generation of emit_load_%02x().
 * Fixed some bugs.
 * comp_load_ea() now supports most (all?) M68000 addressing modes.
 * Generated code-size reduced from ~23M to ~16M, but still needs to be reduced more.
 *
 * Revision 1.2  1996/05/13 10:46:55  grubba
 * Implemented some more opcodes.
 * Now supports two different compiler models. (SLOW_COMPILER).
 * comp_* functions conventions changed. The generated code may now use:
 * ULONG **code -- Pointer to pointer to next instruction to generate.
 * ULONG *pc -- Pointer to next M68000 programcounter (word address).
 * USHORT *mem -- 16MB M68000 emulated memory.
 *
 * Revision 1.1  1996/05/12 16:37:15  grubba
 * Initial revision
 *
 *
 */

/* TODO:
 *
 *  * X bit not implemented.
 *
 *  * Need to check if the Supervisor bit changes in several places.
 *
 *  * Several places where there are many stack accesses may be optimized
 *    to a single add, and offsetted memory accesses.
 *
 *  * Most op-codes still not implemented.
 */

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>

#include "recomp.h"
#include "m68k.h"
#include "sparc.h"
#include "compiler.h"

/*
 * Prototypes
 */

int comp_supervisor(FILE *, U16, const char *);
int comp_clobber(FILE *, U8, U8, U8);


/*
 * Functions
 */

void as_exception(FILE *fp, U16 opcode)
{
  /* On entry:
   * 	%acc0 : vector number
   *	%acc1 : old status register
   *
   * Returns:
   *	%i0 : Next address to start executing at.
   */
  /* FIXME: Need to check for bad SSP => HALT */
  /* FIXME: Need to check for zapped code */
  fputs("	! Were we in supervisormode before?\n"
	"	sethi	%hi(0x2000), %o1\n"
	"	btst	%o1, %sr\n"
	"	bne	0f\n"
	"	ld	[ %regs + _A7 ], %o0\n"
	"	! Flip stacks\n"
	"	st	%o0, [ %regs + _USP ]\n"
	"	ld	[ %regs + _SSP ], %o0\n"
	"	st	%o0, [ %regs + _A7 ]\n"
	"	! Set supervisor bit\n"
	"	or	%sr, %o1, %sr\n"
	"0:\n"
	"	! Push format/offset on stack\n"
	"	add	-2, %o0, %o0\n"
	"	sth	%acc0, [ %mem + %o0 ]\n"
	"	! Push PC on stack\n"
	"	add	-2, %o0, %o0\n"
	"	sth	%pc, [ %mem + %o0 ]\n"
	"	srl	%pc, 0x10, %o1\n"
	"	add	-2, %o0, %o0\n"
	"	sth	%o1, [ %mem + %o0 ]\n"
	"	! Push old sr on stack\n"
	"	add	-2, %o0, %o0\n"
	"	sth	%acc1, [ %mem + %o0 ]\n"
	"	! Store the new stack pointer\n"
	"	st	%o0, [ %regs + _A7 ]\n"
	"	! Store PC\n"
	"	st	%pc, [ %regs + _PC ]\n"
	"	! Fech VBR\n"
	"	ld	[ %regs + _VBR ], %o0\n"
	"	! Store SR\n"
	"	st	%sr, [ %regs + _SR ]\n"
	"	and	0x3ff, %acc0, %acc0\n"
	"	sll	%acc0, 2, %acc0\n"
	"	add	%o0, %acc0, %o0\n"
	"	ld	[ %mem + %o0 ], %i0\n"
	"	ret\n"
	"	restore\n", fp);
}

/*
 * Defaults
 */

int head_default(U16 opcode)
{
  return(1);
}

int head_not_implemented(U16 opcode)
{
  return(0);
}

void tab_default(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* LOAD_EA, LOAD_EO, WRITE_BACK */
  fprintf(fp, "0x%08x, opcode_%04x",
	  TEF_DST | TEF_DST_LOAD | TEF_WRITE_BACK |
	  (opcode & TEF_DST_MASK), opcode);
}

int comp_default(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* Usually not needed */
  return(0);
}

/*
 * M68000 instruction implementations
 */

int head_addi(U16 opcode)
{
  return (opcode == 0x0600);
}

void tab_addi(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* LOAD_IMM, LOAD_EA, LOAD_EO, WRITE_BACK */
  fprintf(fp, "0x%08x, opcode_0600",
	  TEF_SRC | TEF_SRC_LOAD | TEF_SRC_IMM | ((opcode & 0x00c0)<<9) |
	  TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff) |
	  TEF_WRITE_BACK | TEF_FIX_SR);
}

void as_addi(FILE *fp, U16 opcode, const char *mnemonic)
{
  fputs("	addcc	%acc1, %acc0, %acc0\n",	fp);
}

int head_addx(U16 opcode)
{
  return(opcode == 0xd100);
}

void tab_addx(FILE *fp, U16 opcode, const char *mnemonic)
{
  if (opcode & 0x0008) {
    /* ADDX -(Ay), -(Ax) */
    fprintf(fp, "0x%08x, opcode_d100",
	    TEF_SRC | TEF_SRC_LOAD | (opcode & 0x0e00) | ((opcode & 0x00c0)<<9) |
	    TEF_DST | TEF_DST_LOAD | (opcode & 0x00c7) | TEF_WRITE_BACK | 0x8002 |
	    TEF_FIX_SR);
  } else {
    /* ADDX Dy, Dx */
    fprintf(fp, "0x%08x, opcode_d100",
	    TEF_SRC | TEF_SRC_LOAD | (opcode & 0x0e00) | ((opcode & 0x00c0)<<9) |
	    TEF_DST | TEF_DST_LOAD | (opcode & 0x00c7) | TEF_WRITE_BACK |
	    TEF_FIX_SR);
  }
}

void as_addx(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	0x10, %%sr, %%o0\n"
	  "	add	%%acc0, %%acc1, %%acc0\n"
	  "	srl	%%o0, 8, %%o0\n"
	  "	addcc	%%o0, %%acc0, %%acc0\n");
}

int head_andi(U16 opcode)
{
  return(opcode == 0x0200);
}

void tab_andi(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_0200",
	  TEF_SRC | TEF_SRC_LOAD | TEF_SRC_IMM | ((opcode & 0x00c0)<<9) |
	  TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff) | TEF_WRITE_BACK);
}

void as_andi(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	%%acc1, %%acc0, %%acc0\n"
	  "	and	-16, %%sr, %%sr\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

void tab_andi_sr(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* SUPERVISOR, IMMEDIATE */
  fprintf(fp, "0x%08x, opcode_%04x",
	  TEF_SUPERVISOR | TEF_SRC | TEF_SRC_LOAD | TEF_SRC_IMM | TEF_SRC_WORD,
	  opcode);
}

void as_andi_sr(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	sethi	%%hi(0x2000), %%o0\n"
	  "	btst	%%o0, %%acc1\n"
	  "	bne	0f\n"
	  "	and	%%acc1, %%sr, %%sr\n"
	  "	! Stack flip-time.\n"
	  "	ld	[ %%regs + _A7 ], %%o0\n"
	  "	st	%%o0, [ %%regs + _SSP ]\n"
	  "	ld	[ %%regs + _USP ], %%o0\n"
	  "	st	%%o0, [ %%regs + _A7 ]\n");
}

int head_bchg(U16 opcode)
{
  return(opcode == 0x0140);
}

void tab_bchg(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* FIXME: This will fail on #xxx, Dn  FIXED? */

  if ((opcode & 0xffc0) == 0x0840) {
    /* Immediate */
    if (opcode & 0x0038) {
      fprintf(fp, "0x%08x, opcode_0140",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | TEF_SRC_IMM |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_BYTE | (opcode & 0x003f) |
	      TEF_WRITE_BACK);
    } else {
      /* Dn */
      fprintf(fp, "0x%08x, opcode_0140",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | TEF_SRC_IMM |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_LONG | (opcode & 0x003f) |
	      TEF_WRITE_BACK);
    }
  } else {
    /* LOAD_EA, LOAD_EO, WRITE_BACK */
    if (opcode & 0x0038) {
      fprintf(fp, "0x%08x, opcode_0140",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_LONG | (opcode & 0x0e00) |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_BYTE | (opcode & 0x003f) |
	      TEF_WRITE_BACK);
    } else {
      /* Dn */
      fprintf(fp, "0x%08x, opcode_0140",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_LONG | (opcode & 0x0e00) |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_LONG | (opcode & 0x003f) |
	      TEF_WRITE_BACK);
    }
  }
}

void as_bchg(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	mov	0x01, %%o0\n"
	  "	sll	%%o0, %%acc1, %%o0\n"
	  "	btst	%%acc0, %%o0\n"
	  "	xor	%%acc0, %%o0, %%acc0\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "	and	-5, %%sr, %%sr\n"
	  "0:\n");
}

int head_bclr(U16 opcode)
{
  return(opcode == 0x0180);
}

void tab_bclr(FILE *fp, U16 opcode, const char *mnemonic)
{
  if ((opcode & 0xffc0) == 0x0880) {
    /* Immediate */
    if (opcode & 0x0038) {
      fprintf(fp, "0x%08x, opcode_0180",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | TEF_SRC_IMM |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_BYTE | (opcode & 0x003f) |
	      TEF_WRITE_BACK);
    } else {
      /* Dn */
      fprintf(fp, "0x%08x, opcode_0180",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | TEF_SRC_IMM |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_LONG | (opcode & 0x003f) |
	      TEF_WRITE_BACK);
    }
  } else {
    if (opcode & 0x0038) {
      fprintf(fp, "0x%08x, opcode_0180",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | (opcode & 0x0e00) |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_BYTE | (opcode & 0x003f) |
	      TEF_WRITE_BACK);
    } else {
      /* Dn */
      fprintf(fp, "0x%08x, opcode_0180",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | (opcode & 0x0e00) |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_LONG | (opcode & 0x003f) |
	      TEF_WRITE_BACK);
    }
  }
}

void as_bclr(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	mov	0x01, %%o0\n"
	  "	sll	%%o0, %%acc1, %%o0\n"
	  "	btst	%%acc0, %%o0\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "	and	-5, %%sr, %%sr\n"
	  "0:\n"
	  "	andn	%%acc0, %%o0, %%acc0\n");
}

int head_bset(U16 opcode)
{
  return(opcode == 0x01c0);
}

void tab_bset(FILE *fp, U16 opcode, const char *mnemonic)
{
  if ((opcode & 0xffc0) == 0x08c0) {
    /* Immediate */
    if (opcode & 0x0038) {
      fprintf(fp, "0x%08x, opcode_01c0",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | TEF_SRC_IMM |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_BYTE | (opcode & 0x003f) |
	      TEF_WRITE_BACK);
    } else {
      /* Dn */
      fprintf(fp, "0x%08x, opcode_01c0",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | TEF_SRC_IMM |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_LONG | (opcode & 0x003f) |
	      TEF_WRITE_BACK);
    }
  } else {
    if (opcode & 0x0038) {
      fprintf(fp, "0x%08x, opcode_01c0",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | (opcode & 0x0e00) |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_BYTE | (opcode & 0x003f) |
	      TEF_WRITE_BACK);
    } else {
      /* Dn */
      fprintf(fp, "0x%08x, opcode_01c0",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | (opcode & 0x0e00) |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_LONG | (opcode & 0x003f) |
	      TEF_WRITE_BACK);
    }
  }
}

void as_bset(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	mov	0x01, %%o0\n"
	  "	sll	%%o0, %%acc1, %%o0\n"
	  "	btst	%%acc0, %%o0\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "	and	-5, %%sr, %%sr\n"
	  "0:\n"
	  "	or	%%acc0, %%o0, %%acc0\n");
}

int head_btst(U16 opcode)
{
  return(opcode == 0x0100);
}

void tab_btst(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* FIXME: This will fail on #xxx, Dn */

  if ((opcode & 0xffc0) == 0x0800) {
    /* Immediate */
    if (opcode & 0x0038) {
      fprintf(fp, "0x%08x, opcode_0100",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | TEF_SRC_IMM |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_BYTE | (opcode & 0x003f));
    } else {
      /* Dn */
      fprintf(fp, "0x%08x, opcode_0100",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | TEF_SRC_IMM |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_LONG | (opcode & 0x003f));
    }
  } else {
    /* LOAD_EA, LOAD_EO, WRITE_BACK */
    if (opcode & 0x0038) {
      fprintf(fp, "0x%08x, opcode_0100",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | (opcode & 0x0e00) |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_BYTE | (opcode & 0x003f));
    } else {
      /* Dn */
      fprintf(fp, "0x%08x, opcode_0100",
	      TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | (opcode & 0x0e00) |
	      TEF_DST | TEF_DST_LOAD | TEF_DST_LONG | (opcode & 0x003f));
    }
  }
}

void as_btst(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	mov	0x01, %%o0\n"
	  "	sll	%%o0, %%acc1, %%o0\n"
	  "	btst	%%acc0, %%o0\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "	and	-5, %%sr, %%sr\n"
	  "0:\n");
}

int head_cmp(U16 opcode)
{
  return(opcode == 0xb000);
}

void tab_cmp(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_b000",
	  TEF_SRC | TEF_SRC_LOAD | ((opcode & 0x00ff)<<9) |
	  TEF_DST | TEF_DST_LOAD | (opcode & 0x00c0) | ((opcode & 0x0e00)>>9));
}

void as_cmp(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	-16, %%sr, %%sr\n"
	  "	subcc	%%acc0, %%acc1, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n"
	  "	bvs,a	0f\n"
	  "	or	2, %%sr, %%sr\n"
	  "0:\n"
	  "	bcs,a	0f\n"
	  "	or	1, %%sr, %%sr\n"
	  "0:\n");
}

int head_cmpa(U16 opcode)
{
  return(opcode == 0xb0c0);
}

void tab_cmpa(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* FIXME: BUGS on 0xb3e0 => cmpa.l a1, (xxxx.W, PC) */
  /* FIXME: This will fail on .W  FIXED? */
  if (opcode & 0x100) {
    /* Long */
    fprintf(fp, "0x%08x, opcode_b0c0",
	    TEF_SRC | TEF_SRC_LOAD | ((opcode & 0x003f)<<9) | TEF_SRC_LONG |
	    TEF_DST | TEF_DST_LOAD | ((opcode & 0x1e00)>>9) | TEF_DST_LONG |
	    TEF_FIX_SR);
  } else {
    /* Word */
    fprintf(fp, "0x%08x, opcode_b0c0",
	    TEF_SRC | TEF_SRC_LOAD | ((opcode & 0x003f)<<9) | TEF_SRC_WORD |
	    TEF_DST | TEF_DST_LOAD | ((opcode & 0x1e00)>>9) | TEF_DST_LONG);
  }
}

void as_cmpa(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	-16, %%sr, %%sr\n"
	  "	subcc	%%acc0, %%acc1, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n"
	  "	bvs,a	0f\n"
	  "	or	2, %%sr, %%sr\n"
	  "0:\n"
	  "	bcs,a	0f\n"
	  "	or	1, %%sr, %%sr\n"
	  "0:\n");
}

int head_cmpi(U16 opcode)
{
  return(opcode == 0x0c00);
}

void tab_cmpi(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* Since we always sign-extend to 32-bits, we can always do a longword compare */
  fprintf(fp, "0x%08x, opcode_0c00",
	  TEF_SRC | TEF_SRC_LOAD | TEF_SRC_IMM | ((opcode & 0x00c0)<<9) |
	  TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff));
}

void as_cmpi(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	-16, %%sr, %%sr\n"
	  "	subcc	%%acc0, %%acc1, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n"
	  "	bvs,a	0f\n"
	  "	or	2, %%sr, %%sr\n"
	  "0:\n"
	  "	bcs,a	0f\n"
	  "	or	1, %%sr, %%sr\n"
	  "0:\n");
}

int head_cmpm(U16 opcode)
{
  return(opcode == 0xb108);
}

void tab_cmpm(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_b108",
	  TEF_SRC | TEF_SRC_LOAD | 0x3000 | (opcode & 0x0e00) |
	  TEF_DST | TEF_DST_LOAD | 0x0018 | (opcode & 0x00c7) |
	  ((opcode & 0x00c0)<<9) | TEF_FIX_SR);
}

void as_cmpm(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	-16, %%sr, %%sr\n"
	  "	subcc	%%acc0, %%acc1, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n"
	  "	bvs,a	0f\n"
	  "	or	2, %%sr, %%sr\n"
	  "0:\n"
	  "	bcs,a	0f\n"
	  "	or	1, %%sr, %%sr\n"
	  "0:\n");
}


void tab_eori_sr(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* SUPERVISOR, IMMEDIATE */
  fprintf(fp, "0x%08x, opcode_%04x",
	  TEF_SUPERVISOR |
	  TEF_SRC | TEF_SRC_IMM | TEF_SRC_WORD | TEF_SRC_LOAD 
	  , opcode);
}

void as_eori_sr(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	sethi	%%hi(0x2000), %%o0\n"
	  "	btst	%%o0, %%acc1\n"
	  "	be	0f\n"
	  "	xor	%%acc1, %%sr, %%sr\n"
	  "	! Time to flip stacks.\n"
	  "	ld	[ %%regs + _A7 ], %%o0\n"
	  "	st	%%o0, [ %%regs + _SSP ]\n"
	  "	ld	[ %%regs + _USP ], %%o0\n"
	  "	st	%%o0, [ %%regs + _A7 ]\n"
	  "0:\n");
}

int head_illegal(U16 opcode)
{
  return(opcode == 0x4afc);
}

void tab_illegal(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_4afc", TEF_TERMINATE);
}

void as_illegal(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	mov	0x%04x, %%acc0\n"
	  "	mov	%%sr, %%acc1\n", VEC_ILL_INSTR);
  as_exception(fp, opcode);
}

int head_lea(U16 opcode)
{
  return(opcode == 0x41c0);
}

void tab_lea(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* LOAD_EA */
  fprintf(fp, "0x%08x, opcode_41c0",
	  TEF_SRC | TEF_SRC_LONG | ((opcode & 0x003f)<<9) |
	  TEF_DST | TEF_DST_LONG | ((opcode & 0x0e00)>>9) | 0x0008 | TEF_WRITE_BACK);
}

void as_lea(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "	mov	%%ea, %%acc0\n");
}

int head_link(U16 opcode)
{
  return(opcode == 0x4e50);
}

void tab_link(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_4e50",
	  TEF_SRC | TEF_SRC_IMM | TEF_SRC_LOAD | TEF_SRC_WORD | TEF_WRITE_BACK |
	  TEF_DST | (opcode & 0x0007) | 0x0008 | TEF_DST_LONG | TEF_DST_LOAD);
}

void as_link(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	ld	[ %%regs + _A7 ], %%o0\n"
	  "	add	-4, %%o0, %%o0\n"
	  "	ld	[ %%vecs + _VEC_STORE_LONG ], %%o7\n"
	  "	mov	%%o0, %%ea\n"
	  "	st	%%o0, [ %%regs + _A7 ]\n"
	  "	call	%%o7\n"
	  "	mov	%%acc0, %%o1\n"
	  "	add	%%ea, %%acc1, %%o0\n"
	  "	mov	%%ea, %%acc0\n"
	  "	st	%%o0, [ %%regs + _A7 ]\n");
}

int head_move(U16 opcode)
{
  return (opcode == 0x1000);
}

void tab_move(FILE *fp, U16 opcode, const char *mnemonic)
{
  U32 size = 0;
  U32 base;

  switch(opcode & 0x3000) {
  case 0x1000:	/* Byte */
    size = TEF_SRC_BYTE | TEF_DST_BYTE;
    break;
  case 0x2000:	/* Long */
    size = TEF_SRC_LONG | TEF_DST_LONG;
    break;
  case 0x3000:	/* Word */
    size = TEF_SRC_WORD | TEF_DST_WORD;
    break;
  }

  base = ((opcode & 0x003f)<<9) | ((opcode & 0x0e00)>>9) | ((opcode & 0x01c0)>>3);

  fprintf(fp, "0x%08x, opcode_1000",
	  TEF_SRC | TEF_DST | base | size | TEF_SRC_LOAD | TEF_WRITE_BACK);
}

void as_move(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* Fix SR */
  fprintf(fp,
	  "	mov	%%acc1, %%acc0\n"
	  "	and	-16, %%sr, %%sr\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

int head_move_sr(U16 opcode)
{
  return((opcode == 0x46c0) || (opcode == 0x40c0));
}

void tab_move_sr(FILE *fp, U16 opcode, const char *mnemonic)
{
  if ((opcode & 0xffc0) == 0x46c0) {
    /* Move to SR */
    fprintf(fp, "0x%08x, opcode_46c0", TEF_SUPERVISOR |
	    TEF_DST | TEF_DST_WORD | (opcode & 0x003f) | TEF_DST_LOAD);
  } else {
    /* Move from SR */
    fprintf(fp, "0x%08x, opcode_40c0", TEF_SUPERVISOR |
	    TEF_DST | TEF_DST_WORD | (opcode & 0x003f) | TEF_WRITE_BACK);
  }
}

void as_move_sr(FILE *fp, U16 opcode, const char *mnemonic)
{
  if (opcode == 0x46c0) {
    /* Move to SR */
    fprintf(fp,
	    "	sethi	%%hi(0x2000), %%o0\n"
	    "	btst	%%acc0, %%o0\n"
	    "	bne	0f\n"
	    "	mov	%%acc0, %%sr\n"
	    "	ld	[ %%regs + _A7 ], %%o0\n"
	    "	st	%%o0, [ %%regs + _SSP ]\n"
	    "	ld	[ %%regs + _USP ], %%o0\n"
	    "	st	%%o0, [ %%regs + _A7 ]\n"
	    "0:\n");
  } else {
    /* Move from SR */
    fprintf(fp,
	    "	mov	%%sr, %%acc0\n");
  }
}

int head_movea(U16 opcode)
{
  return (opcode == 0x2040);
}

void tab_movea(FILE *fp, U16 opcode, const char *mnemonic)
{
  U32 size = 0;

  switch(opcode & 0x3000) {
  case 0x2000:	/* Long */
    size = TEF_SRC_LONG;
    break;
  case 0x3000:	/* Word */
    size = TEF_SRC_WORD;
    break;
  }

  fprintf(fp, "0x%08x, opcode_2040",
	  TEF_SRC | TEF_SRC_LOAD | ((opcode & 0x3f)<<9) | size |
	  TEF_DST | TEF_DST_LONG | ((opcode & 0x0e00)>>9) | 0x0008 |
	  TEF_WRITE_BACK);
}

void as_movea(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "	mov	%%acc1, %%acc0\n");
}

#if 0
int comp_moves(FILE *fp, U16 opcode, const char *mnemonic){
  comp_supervisor(fp, opcode, mnemonic);
  return(comp_illegal(fp, opcode, mnemonic));
}
#endif /* 0 */

int head_ori(U16 opcode)
{
  return (opcode == 0x0000);
}

void tab_ori(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_0000",
	  TEF_SRC | TEF_SRC_LOAD | TEF_SRC_IMM | ((opcode & 0x00c0)<<9) |
	  TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff) | TEF_WRITE_BACK);
}

void as_ori(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	or	%%acc1, %%acc0, %%acc0\n"
	  "	and	-16, %%sr, %%sr\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

int head_ori_ccr(U16 opcode)
{
  return (opcode == 0x003c);
}

void tab_ori_ccr(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_003c",
	  TEF_SRC | TEF_SRC_LOAD | TEF_SRC_IMM | TEF_SRC_BYTE);
}

void as_ori_ccr(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "	or	%%acc1, %%sr, %%sr\n");
}

int head_ori_sr(U16 opcode)
{
  return(opcode == 0x007c);
}

void tab_ori_sr(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* SUPERVISOR, IMMEDIATE */
  fprintf(fp, "0x%08x, opcode_%04x",
	  TEF_SUPERVISOR |
	  TEF_SRC | TEF_SRC_LOAD | TEF_SRC_IMM | TEF_SRC_WORD,
	  opcode);
}

void as_ori_sr(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "	or	%%acc1, %%sr, %%sr\n");
}

int head_clr(U16 opcode)
{
  return (opcode == 0x4200);
}

void tab_clr(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* LOAD_EA, WRITE_BACK */
  fprintf(fp, "0x%08x, opcode_4200",
	  TEF_DST | (opcode & 0xff) | TEF_WRITE_BACK);
}

void as_clr(FILE *fp, U16 opcode, const char *mnemonic)
{
  fputs("	and	-0x10, %sr, %sr\n"
	"	mov	%g0, %acc0\n"
	"	or	0x04, %sr, %sr\n",
	fp);
}

int head_neg(U16 opcode)
{
  return (opcode == 0x4400);
}

void tab_neg(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_4400",
	  TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff) | TEF_WRITE_BACK);
}

void as_neg(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	-32, %%sr, %%sr\n"
	  "	subcc	%%g0, %%acc0, %%acc0\n"
	  "	bcs,a	0f\n"
	  "	or	0x11, %%sr, %%sr\n"
	  "	or      0x04, %%sr, %%sr\n"
	  "0:\n"
	  "	blt,a	0f\n"
	  "	or	0x08, %%sr, %%sr\n"
	  "0:\n"
	  "	bvs,a	0f\n"
	  "	or	0x02, %%sr, %%sr\n"
	  "0:\n");
}

int head_not(U16 opcode)
{
  return(opcode == 0x4600);
}

void tab_not(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_4600",
	  TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff) | TEF_WRITE_BACK);
}

void as_not(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	xor	-1, %%acc0, %%acc0\n"
	  "	and	-16, %%sr, %%sr\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

int head_pea(U16 opcode)
{
  return(opcode == 0x4850);
}

void tab_pea(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_4850",
	  TEF_DST | TEF_DST_LONG | (opcode & 0x003f));
}

void as_pea(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	ld	[ %%regs + _A7 ], %%o0\n"
	  "	mov	%%ea, %%o1\n"
	  "	add	-4, %%o0, %%o0\n"
	  "	ld	[ %%vecs + _VEC_STORE_LONG ], %%o7\n"
	  "	call	%%o7\n"
	  "	st	%%o0, [ %%regs + _A7 ]\n");
}

int head_movem(U16 opcode)
{
  return (0);
}

void tab_movem(FILE *fp, U16 opcode, const char *mnemonic)
{
  U32 base = TEF_DST | TEF_SRC |
    ((opcode & 0x0040)?TEF_DST_LONG:TEF_DST_WORD);

  if (opcode & 0x0400) {
    if ((opcode & 0x0038) == 0x0018) {
      /* MOVEM (An)+, xxxx */
      base |= TEF_DST_MOVEM | TEF_WRITE_BACK | ((opcode & 0x003f)<<9);
    } else {
      base |= TEF_DST_MOVEM | ((opcode & 0x003f)<<9);
    }
  } else {
    if ((opcode & 0x0038) == 0x0020) {
      /* MOVEM xxxx, -(An) */
      base |= TEF_SRC_MOVEM | TEF_WRITE_BACK | (opcode & 0x003f);
    } else {
      base |= TEF_SRC_MOVEM | (opcode & 0x003f);
    }
  }
  fprintf(fp, "0x%08x, 0x00000000", base);
}

void as_movem(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* Everything is magic */
}

void tab_move_usp(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* SUPERVISOR */
  if (opcode & 0x0008) {
    /* From USP */
    fprintf(fp, "0x%08x, opcode_%04x", TEF_SUPERVISOR |
	    TEF_DST | TEF_DST_LONG | 0x0008 | (opcode & 0x0007) | TEF_WRITE_BACK,
	    opcode & 0xfff8);
  } else {
    /* To USP */
    fprintf(fp, "0x%08x, opcode_%04x", TEF_SUPERVISOR |
	    TEF_DST | TEF_DST_LONG | 0x0008 | (opcode & 0x0007) | TEF_DST_LOAD,
	    opcode & 0xfff8);
  }
}

void as_move_usp(FILE *fp, U16 opcode, const char *mnemonic)
{
  if (opcode & 0x0008) {
    /* From USP */
    fprintf(fp, "	ld	[ %%regs + _USP ], %%acc0\n");
  } else {
    /* To USP */
    fprintf(fp, "	st	%%acc0, [ %%regs + _USP ]\n");
  }
}

void tab_reset(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_%04x", TEF_SUPERVISOR, opcode);
}

void as_reset(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	ld	[ %%vecs + _VEC_RESET ], %%o7\n"
	  "	call	%%o7\n"
	  "	nop\n");
}

void tab_nop(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* No need to do anything */
  fprintf(fp, "0, opcode_4e71");
}

void as_nop(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* No need to do anything */
}

void tab_stop(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_4e72",
	  TEF_SUPERVISOR | TEF_SRC | TEF_SRC_LOAD | TEF_SRC_WORD | TEF_SRC_IMM |
	  TEF_TERMINATE);
}

void as_stop(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	st	%%acc1, [ %%regs + _SR ]\n"
	  "	ld	[ %%vecs + _VEC_STOP ], %%o7\n"
	  "	add	4, %%pc, %%i2\n"
	  "	jmpl	%%o7, %%g0\n"
	  "	restore\n");
}

void tab_rte(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_4e73",
	  TEF_SUPERVISOR | TEF_TERMINATE);
}

void as_rte(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* FIXME: Should check that the stack is in valid memory */
  fputs("	ld	[ %regs + _A7 ], %acc0\n"
	"	add	%mem, %acc0, %o0\n"
	"	! Get SR\n"
	"	lduh	[ %o0 ], %sr\n"
	"	! Get PC\n"
	"	lduh	[ %o0 + 2 ], %ea\n"
	"	sll	%ea, 0x10, %ea\n"
	"	lduh	[ %o0 + 4 ], %o1\n"
	"	or	%ea, %o1, %ea\n"
	"	! Get Format/Offset\n"
	"	lduh	[ %o0 + 6 ], %o1\n"
	"	srl	%o1, 0x0c, %o1\n"
	"	btst	0x0e, %o1\n"
	"	be,a	0f\n"
	"	add	8, %acc0, %acc0\n"
	"	cmp	%o1, 0x02\n"
	"	be,a	0f\n"
	"	add	12, %acc0, %acc0\n"
	"	cmp	%o1, 0x08\n"
	"	be,a	0f\n"
	"	add	58, %acc0, %acc0\n"
	"	cmp	%o1, 0x09\n"
	"	be,a	0f\n"
	"	add	20, %acc0, %acc0\n"
	"	cmp	%o1, 0x0a\n"
	"	be,a	0f\n"
	"	add	32, %acc0, %acc0\n"
	"	cmp	%o1, 0x0b\n"
	"	be,a	0f\n"
	"	add     92, %acc0, %acc0\n"
	"	! FIXME: Format error exception\n"
	"0:\n"
	"	sethi	%hi(0x2000), %o0\n"
	"	! Save the new SSP\n"
	"	st	%acc0, [ %regs + _SSP ]\n"
	"	! Are we still in Supervisormode?\n"
	"	btst	%o0, %sr\n"
	"	! Store SR\n"
	"	st	%sr, [ %regs + _SR ]\n"
	"	be,a	0f\n"
	"	ld	[ %regs + _USP ], %acc0	! No\n"
	"0:\n"
	"	st	%acc0, [ %regs + _A7 ]	! Yes and No\n"
	"	! Time to return\n"
	"	mov	%ea, %i0\n"
	"	ret\n"
	"	restore\n", fp);
}

void tab_rtd(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_4e74",
	  TEF_TERMINATE |
	  TEF_DST | TEF_DST_LOAD | TEF_DST_IMM | TEF_DST_WORD);
}

void as_rtd(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* FIXME: Check legal memory */
  fputs("	ld	[ %regs + _A7 ], %o0\n"
	"	! Get PC\n"
	"	lduh	[ %mem + %o0 ], %o1\n"
	"	add	2, %o0, %o0\n"
	"	sll	%o1, 0x10, %o1\n"
	"	lduh	[ %mem + %o0 ], %o2\n"
	"	add	2, %o0, %o0\n"
	"	! Add displacement\n"
	"	add	%acc0, %o0, %o0\n"
	"	st	%o0, [ %regs + _A7 ]\n"
	"	! Save away the cached registers\n"
	"	st	%sr, [ %regs + _SR ]\n"
	"	st	%pc, [ %regs + _PC ]\n"
	"	or	%o1, %o2, %i0\n"
	"	ret\n"
	"	restore\n", fp);
}

void tab_rts(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* TERMINATE */
  fprintf(fp, "0x%08x, opcode_4e75", TEF_TERMINATE);
}

void as_rts(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* FIXME: Check that SP is legal memory */
  fputs("	ld	[ %regs + _A7 ], %o0\n"
	"	! Fetch PC\n"
	"	lduh	[ %mem + %o0 ], %o1\n"
	"	add	2, %o0, %o0\n"
	"	sll	%o1, 0x10, %o1\n"
	"	lduh	[ %mem + %o0 ],	%o2\n"
	"	add	2, %o0, %o0\n"
	"	st	%o0, [ %regs + _A7 ]\n"
	"	! Save away the cached registers\n"
	"	st	%sr, [ %regs + _SR ]\n"
	"	st	%pc, [ %regs + _PC ]\n"
	"	or	%o1, %o2, %i0\n"
	"	ret\n"
	"	restore\n", fp);
}

void tab_movec(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* SUPERVISOR, IMMEDIATE */
  fprintf(fp, "0x%08x, opcode_%04x",
	  TEF_SUPERVISOR |
	  TEF_SRC | TEF_SRC_LOAD | TEF_SRC_IMM | TEF_SRC_WORD, opcode);
}

void as_movec(FILE *fp, U16 opcode, const char *mnemonic)
{
  fputs("	! Get the Register number\n"
	"	srl	%acc1, 0x0a, %o1\n"
	"	and	0x003c, %o1, %o1\n"
	"	! Compare to known control registers\n"
	"	btst	0x07fe, %acc1\n"
	"	bne	0f\n"
	"	and	0x0fff, %acc1, %o0\n"
	"	cmp	%o0, 0x0800\n"
	"	be,a	1f\n"
	"	mov	_USP, %o0\n"
	"	cmp	%o0, 0x0801\n"
	"	be,a	1f\n"
	"	mov	_VBR, %o0\n"
	"	! SFC and DFC not implemented\n"
	"0:\n"
	"	! FIXME: Illegal instruction exception\n"
	"	! FIXME: More effective with one add + offsets\n"
	"	ld	[ %regs + _A7 ], %o0\n"
	"	mov	0x04, %acc0\n"
	"	add	-2, %o0, %o0\n"
	"	sth	%acc0, [ %mem + %o0 ]\n"
	"	! Push PC on stack\n"
	"	add	-2, %o0, %o0\n"
	"	sth	%pc, [ %mem + %o0 ]\n"
	"	srl	%pc, 0x10, %o1\n"
	"	add	-2, %o0, %o0\n"
	"	sth	%o1, [ %mem + %o0 ]\n"
	"	! Push old sr on stack\n"
	"	add	-2, %o0, %o0\n"
	"	sth	%sp, [ %mem + %o0 ]\n"
	"	! Store the new stack pointer\n"
	"	st	%o0, [ %regs + _A7 ]\n"
	"	! Store PC\n"
	"	st	%pc, [ %regs + _PC ]\n"
	"	! Store SR\n"
	"	st	%sr, [ %regs + _SR ]\n"
	"	! Fech VBR\n"
	"	ld	[ %regs + _VBR ], %o0\n"
	"	and	0x3ff, %acc0, %acc0\n"
	"	sll	%acc0, 2, %acc0\n"
	"	add	%o0, %acc0, %o0\n"
	"	ld	[ %mem + %o0 ], %i0\n"
	"	ret\n"
	"	restore\n"
	"1:\n"
	, fp);
  if (opcode & 1) {
    /* To control register */
    fputs("	ld	[ %regs + %o1 ], %o2\n"
	 "	st	%o2, [ %regs + %o0 ]\n",
	 fp);
  } else {
    /* From control register */
    fputs("	ld	[ %regs + %o0 ], %o2\n"
	 "	st	%o2, [ %regs + %o1 ]\n",
	 fp);
  }
}

int head_swap(U16 opcode)
{
  return (opcode == 0x4840);
}

void tab_swap(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_4840",
	  TEF_DST | TEF_DST_LOAD | TEF_DST_LONG | (opcode & 0x0007) |
	  TEF_WRITE_BACK);
}

void as_swap(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	srl	%%acc0, 0x10, %%o0\n"
	  "	sll	%%acc0, 0x10, %%acc0\n"
	  "	and	-16, %%sr, %%sr\n"
	  "	or	%%acc0, %%o0, %%acc0\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

int head_ext(U16 opcode)
{
  return(opcode == 0x4880);
}

void tab_ext(FILE *fp, U16 opcode, const char *mnemonic)
{
  if (opcode & 0x0040) {
    /* WORD => LONG */

    fprintf(fp, "0x%08x, opcode_4880",
	    TEF_SRC | TEF_SRC_WORD | TEF_SRC_LOAD | ((opcode & 0x0007)<<9) |
	    TEF_DST | TEF_DST_LONG | (opcode & 0x0007) | TEF_WRITE_BACK);
  } else {
    /* BYTE => WORD */

    fprintf(fp, "0x%08x, opcode_4880",
	    TEF_SRC | TEF_SRC_BYTE | TEF_SRC_LOAD | ((opcode & 0x0007)<<9) |
	    TEF_DST | TEF_DST_WORD | (opcode & 0x0007) | TEF_WRITE_BACK);
  }
}

void as_ext(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* We already sign-extend on load */
  fprintf(fp,
	  "	and	-16, %%sr, %%sr\n"
	  "	mov	%%acc1, %%acc0\n"
	  "	cmp	%%acc1, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

int head_subi(U16 opcode)
{
  return(opcode == 0x0400);
}

void tab_subi(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_0400",
	  TEF_SRC | TEF_SRC_LOAD | TEF_SRC_IMM | ((opcode & 0x00c0)<<9) |
	  TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff) | TEF_WRITE_BACK |
	  TEF_FIX_SR);
}

void as_subi(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	subcc	%%acc0, %%acc1, %%acc0\n");
}

int head_tst(U16 opcode)
{
  return(opcode == 0x4a00);
}

void tab_tst(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_4a00",
	  TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff));
}

void as_tst(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	-16, %%sr, %%sr\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

int head_unlk(U16 opcode)
{
  return(opcode == 0x4e58);
}

void tab_unlk(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_4e58",
	  TEF_DST | TEF_DST_LONG | TEF_DST_LOAD | (opcode & 0x000f) |
	  TEF_WRITE_BACK);
}

void as_unlk(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	ld	[ %%vecs + _VEC_LOAD_LONG ], %%o7\n"
	  "	mov	%%acc0, %%o0\n"
	  "	call	%%o7\n"
	  "	add	4, %%acc0, %%acc1\n"
	  "	mov	%%o0, %%acc0\n"
	  "	st	%%acc1, [ %%regs + _A7 ]\n");
}

int head_jsr(U16 opcode)
{
  return(opcode == 0x4e80);
}

void tab_jsr(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_4e80",
	  TEF_TERMINATE | TEF_DST | TEF_DST_LONG | (opcode & 0x003f) | TEF_PUSH_PC);
}

void as_jsr(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	st	%%sr, [ %%regs + _SR ]\n"
	  "	st	%%pc, [ %%regs + _PC ]\n"
	  "	mov	%%ea, %%i0\n"
	  "	ret\n"
	  "	restore\n");
}

int head_jmp(U16 opcode)
{
  return(opcode == 0x4ec0);
}

void tab_jmp(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_4ec0",
	  TEF_TERMINATE | TEF_DST | TEF_DST_LONG | (opcode & 0x003f));
}

void as_jmp(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	st	%%sr, [ %%regs + _SR ]\n"
	  "	st	%%pc, [ %%regs + _PC ]\n"
	  "	mov	%%ea, %%i0\n"
	  "	ret\n"
	  "	restore\n");
}

void tab_dbcc(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* IMM16 */
  /* NOTE: The SRC args are there only for the disassembly! */
  fprintf(fp, "0x%08x, opcode_%04x",
	  TEF_SRC | ((opcode & 0x0007)<<9) |
	  TEF_DST | TEF_DST_WORD | TEF_DST_PC, opcode);
}

void as_dbcc(FILE *fp, U16 opcode, const char *mnemonic)
{
  if (opcode & 0x0e00) {
    switch (opcode & 0x0e00) {
    case 0x0000:	/* T	BRA, BSR */
    case 0x0200:	/* HI	/C * /Z */
      fprintf(fp, "	and	0x05, %%sr, %%o0\n");
      break;
    case 0x0400:	/* CC	/C */
      fprintf(fp, "	and	0x01, %%sr, %%o0\n");
      break;
    case 0x0600:	/* NE	/Z */
      fprintf(fp, "	and	0x04, %%sr, %%o0\n");
      break;
    case 0x0800:	/* VC	/V */
      fprintf(fp, "	and	0x02, %%sr, %%o0\n");
      break;
    case 0x0a00:	/* PL	/N */
      fprintf(fp, "	and	0x08, %%sr, %%o0\n");
      break;
    case 0x0c00:	/* GE	N * V + /N * /V */
      fprintf(fp,
	      "	srl	%%sr, 0x02, %%o0\n"
	      "	xor	%%sr, %%o0, %%o0\n"
	      "	and	0x02, %%o0, %%o0\n");
      break;
    case 0x0e00:	/* GT	N * V * /Z + /N * /V * /Z */
      fprintf(fp,
	      "	srl	%%sr, 0x02, %%o0\n"
	      "	xor	%%sr, %%o0, %%o0\n"
	      "	and	0x04, %%sr, %%o1\n"
	      "	and	0x02, %%o0, %%o0\n"
	      "	or	%%o0, %%o1, %%o0\n");
      break;
    default:
      break;
    }
    fprintf(fp,
	    "	cmp	%%o0, %%g0\n");
    if (opcode & 0x0100) {
      /* Do the branch if == 0 */
      fprintf(fp, "	bne	0f\n");
    } else {
      /* Do the branch if != 0 */
      fprintf(fp, "	be	0f\n");
    }
  }
  if (opcode & 0x0f00) {
    /* Do the decrement NOTE: Delay slot */
    /* NOTE: %ea holds the destination address */
    fprintf(fp,
	    "	ldsh	[ %%regs + 0x%02x], %%acc0\n"
	    "	addcc	-1, %%acc0, %%acc0\n"
	    "	bcc	0f\n"
	    "	sth	%%acc0, [ %%regs + 0x%02x]\n",
	    ((opcode & 0x07)<<2)+2,
	    ((opcode & 0x07)<<2)+2);
    fprintf(fp,
	    "	st	%%sr, [ %%regs + _SR ]\n"
	    "	st	%%pc, [ %%regs + _PC ]\n"
	    "	mov	%%ea, %%i0\n"
	    "	ret\n"
	    "	restore\n");

    fprintf(fp,"0:\n");
  }
}

int head_scc(U16 opcode)
{
  return(!(opcode & 0x003f));
}

void tab_scc(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_%04x",
	  TEF_DST | TEF_DST_BYTE | (opcode & 0x003f) | TEF_WRITE_BACK,
	  opcode & 0xffc0);
}

void as_scc(FILE *fp, U16 opcode, const char *mnemonic)
{
  switch (opcode & 0x0e00) {
  case 0x0000:	/* T	BRA, BSR */
  case 0x0200:	/* HI	/C * /Z */
    fprintf(fp, "	and	0x05, %%sr, %%o0\n");
    break;
  case 0x0400:	/* CC	/C */
    fprintf(fp, "	and	0x01, %%sr, %%o0\n");
    break;
  case 0x0600:	/* NE	/Z */
    fprintf(fp, "	and	0x04, %%sr, %%o0\n");
    break;
  case 0x0800:	/* VC	/V */
    fprintf(fp, "	and	0x02, %%sr, %%o0\n");
    break;
  case 0x0a00:	/* PL	/N */
    fprintf(fp, "	and	0x08, %%sr, %%o0\n");
    break;
  case 0x0c00:	/* GE	N * V + /N * /V */
    fprintf(fp,
	    "	and	0x0a, %%sr, %%o0	! FIXME: Is this needed?\n"
	    "	srl	%%o0, 0x02, %%o1\n"
	    "	xor	%%o0, %%o1, %%o0\n"
	    "	and	0x02, %%o0, %%o0\n");
    break;
  case 0x0e00:	/* GT	N * V * /Z + /N * /V * /Z */
    fprintf(fp,
	    "	and	0x0a, %%sr, %%o0\n"
	    "	srl	%%o0, 0x02, %%o1\n"
	    "	xor	%%o0, %%o1, %%o0\n"
	    "	and	0x02, %%o0, %%o0\n"
	    "	and	0x04, %%sr, %%o1\n"
	    "	or	%%o0, %%o1, %%o0\n");
    break;
  default:
    break;
  }
  fprintf(fp,
	  "	cmp	%%o0, %%g0\n");
  if (opcode & 0x0100) {
    /* Set the bits if != 0 */
    fprintf(fp,
	    "	be,a	0f\n"
	    "	mov	0, %%acc0\n"
	    "	mov	-1, %%acc0\n"
	    "0:\n");
  } else {
    /* Set the bits if == 0 */
    fprintf(fp,
	    "	bne,a	0f\n"
	    "	mov	0, %%acc0\n"
	    "	mov	-1, %%acc0\n"
	    "0:\n");
  }
}

int head_addq(U16 opcode)
{
  return(opcode == (opcode & 0xff00));
}

void tab_addq(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* LOAD_EA, LOAD_EO, WRITE_BACK */
  if ((opcode & 0x023f) == 0x020f) {
    /* Special case for adding odd values to A7 */
    fprintf(fp, "0x%08x, opcode_%04x",
	    TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff) |
	    TEF_SRC_QUICK8 | ((opcode + 0x0200) & 0x0e00) |
	    TEF_WRITE_BACK | TEF_FIX_SR,
	    (opcode & 0xf100) | ((opcode + 0x0200) & 0x0e00));
  } else {
    if (opcode & 0x0e00) {
      fprintf(fp, "0x%08x, opcode_%04x",
	      TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff) |
	      TEF_SRC_QUICK8 | (opcode & 0x0e00) |
	      TEF_WRITE_BACK | TEF_FIX_SR,
	      opcode & 0xff00);
    } else {
      fprintf(fp, "0x%08x, opcode_%04x",
	      TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff) |
	      TEF_SRC_QUICK8 | (0x08 << 9) |
	      TEF_WRITE_BACK | TEF_FIX_SR,
	      opcode & 0xff00);
    }
  }
}

void as_addq(FILE *fp, U16 opcode, const char *mnemonic)
{
  if (opcode & 0x0e00) {
    fprintf(fp, "	addcc	0x%02x, %%acc0, %%acc0\n",
	    (opcode & 0x0e00)>>9);
  } else {
    fputs("	addcc	0x08, %acc0, %acc0\n", fp);
  }
}

/* FIXME: SUBQ code is a case of cut-and-paste read it */

int head_subq(U16 opcode)
{
  return(opcode == (opcode & 0xff00));
}

void tab_subq(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* LOAD_EA, LOAD_EO, WRITE_BACK */
  if ((opcode & 0x023f) == 0x020f) {
    /* Special case for subtracting odd values to A7 */
    fprintf(fp, "0x%08x, opcode_%04x",
	    TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff) |
	    TEF_SRC_QUICK8 | ((opcode + 0x0200) & 0x0e00) |
	    TEF_WRITE_BACK | TEF_FIX_SR,
	    (opcode & 0xf100) | ((opcode + 0x0200) & 0x0e00));
  } else {
    if (opcode & 0x0e00) {
      fprintf(fp, "0x%08x, opcode_%04x",
	      TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff) |
	      TEF_SRC_QUICK8 | (opcode & 0x0e00) |
	      TEF_WRITE_BACK | TEF_FIX_SR,
	      opcode & 0xff00);
    } else {
      fprintf(fp, "0x%08x, opcode_%04x",
	      TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff) |
	      TEF_SRC_QUICK8 | (0x08 << 9) |
	      TEF_WRITE_BACK | TEF_FIX_SR,
	      opcode & 0xff00);
    }
  }
}

void as_subq(FILE *fp, U16 opcode, const char *mnemonic)
{
  if (opcode & 0x0e00) {
    fprintf(fp, "	addcc	-0x%02x, %%acc0, %%acc0\n",
	    (opcode & 0x0e00)>>9);
  } else {
    fputs("	addcc	-0x08, %acc0, %acc0\n", fp);
  }
}

void tab_bra(FILE *fp, U16 opcode, const char *mnemonic)
{
  if (opcode & 0xff) {
    /* 8-bit immediate displacement */
    fprintf(fp, "0x%08x, opcode_%04x", TEF_TERMINATE, opcode);
  } else {
    /* 16-bit displacement */
    /* IMM16 */
    fprintf(fp, "0x%08x, opcode_%04x",
	    TEF_TERMINATE | TEF_DST | TEF_DST_WORD | TEF_DST_PC,
	    opcode);
  }
}

void as_bra(FILE *fp, U16 opcode, const char *mnemonic)
{
  if (opcode & 0xff) {
    int val = (opcode & 0xff) + 2;
    /* 8-bit immediate */
    if (opcode & 0x80) {
      fprintf(fp, "	add	-0x%02x, %%pc, %%ea\n", 0x0100 - val);
    } else {
      fprintf(fp, "	add	0x%02x, %%pc, %%ea\n", val);
    }
  } else {
    /* 16-bit immediate */
  }
  fprintf(fp,
	  "	st	%%sr, [ %%regs + _SR ]\n"
	  "	st	%%pc, [ %%regs + _PC ]\n"
	  "	mov	%%ea, %%i0\n"
	  "	ret\n"
	  "	restore\n");
}

void tab_bsr(FILE *fp, U16 opcode, const char *mnemonic)
{
  if (opcode & 0xff) {
    /* 8-bit immediate displacement */
    fprintf(fp, "0x%08x, opcode_%04x", TEF_TERMINATE | TEF_PUSH_PC, opcode);
  } else {
    /* 16-bit displacement */
    /* IMM16 */
    fprintf(fp, "0x%08x, opcode_%04x",
	    TEF_TERMINATE | TEF_PUSH_PC | TEF_DST | TEF_DST_WORD | TEF_DST_PC,
	    opcode);
  }
}

void as_bsr(FILE *fp, U16 opcode, const char *mnemonic)
{
  if (opcode & 0xff) {
    int val = (opcode & 0xff) + 2;
    /* 8-bit immediate */
    if (opcode & 0x80) {
      fprintf(fp, "	add	-0x%02x, %%pc, %%ea\n", 0x100 - val);
    } else {
      fprintf(fp, "	add	0x%02x, %%pc, %%ea\n", val);
    }
  } else {
    /* 16-bit immediate */
  }
  fprintf(fp,
	  "	st	%%sr, [ %%regs + _SR ]\n"
	  "	st	%%pc, [ %%regs + _PC ]\n"
	  "	mov	%%ea, %%i0\n"
	  "	ret\n"
	  "	restore\n");
}

void tab_bcc(FILE *fp, U16 opcode, const char *mnemonic)
{
  if (opcode & 0xff) {
    /* 8-bit immediate displacement */
    fprintf(fp, "0, opcode_%04x", opcode);
  } else {
    /* 16-bit displacement */
    /* IMM16 */
    fprintf(fp, "0x%08x, opcode_%04x",
	    TEF_DST | TEF_DST_WORD | TEF_DST_PC,
	    opcode);
  }
}

void as_bcc(FILE *fp, U16 opcode, const char *mnemonic)
{
  switch (opcode & 0x0e00) {
  case 0x0000:	/* T	BRA, BSR */
  case 0x0200:	/* HI	/C * /Z */
    fprintf(fp, "	and	0x05, %%sr, %%o0\n");
    break;
  case 0x0400:	/* CC	/C */
    fprintf(fp, "	and	0x01, %%sr, %%o0\n");
    break;
  case 0x0600:	/* NE	/Z */
    fprintf(fp, "	and	0x04, %%sr, %%o0\n");
    break;
  case 0x0800:	/* VC	/V */
    fprintf(fp, "	and	0x02, %%sr, %%o0\n");
    break;
  case 0x0a00:	/* PL	/N */
    fprintf(fp, "	and	0x08, %%sr, %%o0\n");
    break;
  case 0x0c00:	/* GE	N * V + /N * /V */
    fprintf(fp,
	    "	and	0x0a, %%sr, %%o0	! FIXME: Is this needed?\n"
	    "	srl	%%o0, 0x02, %%o1\n"
	    "	xor	%%o0, %%o1, %%o0\n"
	    "	and	0x02, %%o0, %%o0\n");
    break;
  case 0x0e00:	/* GT	N * V * /Z + /N * /V * /Z */
    fprintf(fp,
	    "	and	0x0a, %%sr, %%o0\n"
	    "	srl	%%o0, 0x02, %%o1\n"
	    "	xor	%%o0, %%o1, %%o0\n"
	    "	and	0x02, %%o0, %%o0\n"
	    "	and	0x04, %%sr, %%o1\n"
	    "	or	%%o0, %%o1, %%o0\n");
    break;
  default:
    break;
  }
  fprintf(fp,
	  "	cmp	%%o0, %%g0\n");
  if (opcode & 0x0100) {
    /* Do the branch if != 0 */
    fprintf(fp, "	be	0f\n");
  } else {
    /* Do the branch if == 0 */
    fprintf(fp, "	bne	0f\n");
  }
  /* Calculate new address NOTE: Delay slot */
  if (opcode & 0xff) {
    /* 8-bit immediate */
    if (opcode & 0x80) {
      fprintf(fp, "	add	-0x%02x, %%pc, %%ea\n",
	      0xfe - (opcode & 0xff));
    } else {
      fprintf(fp, "	add	0x%02x, %%pc, %%ea\n",
	      (opcode & 0x7f) + 2);
    }
  } else {
    /* 16-bit immediate */
    fprintf(fp,
	    "	nop\n");
  }
  fprintf(fp,
	  "	st	%%sr, [ %%regs + _SR ]\n"
	  "	st	%%pc, [ %%regs + _PC ]\n"
	  "	mov	%%ea, %%i0\n"
	  "	ret\n"
	  "	restore\n");

  fprintf(fp,"0:\n");
}

int head_moveq(U16 opcode)
{
  return (opcode == (opcode & 0xf1ff));
}

void tab_moveq(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_%04x",
	  TEF_SRC_QUICK8 | ((opcode & 0xff)<<9) |
	  TEF_DST | TEF_DST_LONG | ((opcode & 0x0e00)>>9) | TEF_WRITE_BACK,
	  (opcode & 0xf1ff));
}

void as_moveq(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	-16, %%sr, %%sr\n");
  if (opcode & 0x80) {
    /* negative */
    fprintf(fp,
	    "	mov	-0x%02x, %%acc0\n"
	    "	or	0x08, %%sr, %%sr\n",
	    0x80 - (opcode & 0x7f));
  } else {
    /* positive */
    if (opcode & 0x7f) {
      fprintf(fp,
	      "	mov	0x%02x, %%acc0\n",
	      opcode & 0x7f);
    } else {
      fprintf(fp,
	      "	mov	%%g0, %%acc0\n"
	      "	or	0x04, %%sr, %%sr\n");
    }
  }
}

int head_divu(U16 opcode)
{
  return(opcode == 0x80c0);
}

void tab_divu(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_80c0",
	  TEF_SRC | TEF_SRC_WORD | TEF_SRC_LOAD | ((opcode & 0x003f)<<9) |
	  TEF_DST | TEF_DST_LONG | TEF_DST_LOAD | ((opcode & 0x0e00)>>9) |
	  TEF_WRITE_BACK);
}

void as_divu(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	cmp	%%acc1, %%g0\n"
	  "	and	-16, %%sr, %%sr\n"
	  "	bne,a	0f\n"
	  "	sethi	%%hi(0xffff0000), %%o0\n"
	  "	! FIXME: Division by 0 Exception\n"
	  "	! Insert code here\n"
	  "0:\n"
	  "	udiv	%%acc0, %%acc1, %%o2\n"
	  "	btst	%%o2, %%o0\n"
	  "	smul	%%o2, %%acc1, %%o1\n"
	  "	andn	%%o2, %%o0, %%o2\n"
	  "	sub	%%acc0, %%o1, %%o1\n"
	  "	sll	%%o1, 0x10, %%o1\n"
	  "	or	%%o2, %%o1, %%acc0\n"
	  "	bne,a	1f\n"
	  "	or	2, %%sr, %%sr\n"
	  "1:\n"
	  "	andncc	%%acc0, %%o0, %%g0\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "	sethi	%%hi(0x8000), %%o0\n"
	  "	btst	%%o0, %%acc0\n"
	  "	bne,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "0:\n");
}

int head_or(U16 opcode)
{
  return(opcode == 0x8000);
}

void tab_or(FILE *fp, U16 opcode, const char *mnemonic)
{
  U32 base = TEF_SRC | TEF_SRC_LOAD | TEF_DST | TEF_DST_LOAD | TEF_WRITE_BACK |
    (opcode & 0x00c0) | ((opcode & 0x00c0)<<9);
  if (opcode & 0x0100) {
    fprintf(fp, "0x%08x, opcode_8000", base | (opcode & 0x0e3f));
  } else {
    fprintf(fp, "0x%08x, opcode_8000",
	    base | ((opcode & 0x0e00)>>9) | ((opcode & 0x003f)<<9));
  }
}

void as_or(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	or	%%acc0, %%acc1, %%acc0\n"
	  "	and	-16, %%sr, %%sr\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

int head_suba(U16 opcode)
{
  return(opcode == 0x90c0);
}

void tab_suba(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_90c0",
	  TEF_SRC | TEF_SRC_LOAD |
	  ((opcode & 0x0100)?TEF_SRC_LONG:TEF_SRC_WORD) |
	  ((opcode & 0x003f)<<9) |
	  TEF_DST | TEF_DST_LOAD | TEF_DST_LONG |
	  ((opcode & 0x0e00)>>9) | 0x0008 | TEF_WRITE_BACK);
}

void as_suba(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "	sub	%%acc0, %%acc1, %%acc0\n");
}

int head_sub(U16 opcode)
{
  return(opcode == 0x9000);
}

void tab_sub(FILE *fp, U16 opcode, const char *mnemonic)
{
  U32 base = TEF_SRC | TEF_SRC_LOAD | TEF_DST | TEF_DST_LOAD |
    TEF_WRITE_BACK | TEF_FIX_SR;

  base |= (opcode & 0x00c0) | ((opcode & 0x00c0)<<9);

  if (opcode & 0x0100) {
    base |= opcode & 0x0e3f;
  } else {
    base |= ((opcode & 0x0e00)>>9) | ((opcode & 0x003f)<<9);
  }
  fprintf(fp, "0x%08x, opcode_9000", base);
}

void as_sub(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "	subcc	%%acc0, %%acc1, %%acc0\n");
}

int head_mulu(U16 opcode)
{
  return (opcode == 0xc0c0);
}

void tab_mulu(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_c0c0",
	  TEF_SRC | TEF_SRC_WORD | TEF_SRC_LOAD | ((opcode & 0x003f)<<9) |
	  TEF_DST | TEF_DST_LONG | TEF_DST_LOAD | ((opcode & 0x0e00)>>9) |
	  TEF_WRITE_BACK);
}

void as_mulu(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* FIXME: Should the parameters to ANDN be reversed? */
  /* FIXME: Fix SR */
  fprintf(fp,
	  "	sethi	%%hi(0xffff0000), %%o0\n"
	  "	and	-16, %%sr, %%sr\n"
	  "	andn	%%acc0, %%o0, %%acc0\n"
	  "	andn	%%acc1, %%o0, %%acc1\n"
	  "	smul	%%acc0, %%acc1, %%acc0\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

int head_muls(U16 opcode)
{
  return (opcode == 0xc1c0);
}

void tab_muls(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_c1c0",
	  TEF_SRC | TEF_SRC_WORD | TEF_SRC_LOAD | ((opcode & 0x003f)<<9) |
	  TEF_DST | TEF_DST_LONG | TEF_DST_LOAD | ((opcode & 0x0e00)>>9) |
	  TEF_WRITE_BACK);
}

void as_muls(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	sll	%%acc0, 0x10, %%acc0\n"
	  "	and	-16, %%sr, %%sr\n"
	  "	srl	%%acc0, 0x10, %%acc0\n"
	  "	smul	%%acc0, %%acc1, %%acc0\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

int head_exg(U16 opcode)
{
  return(((opcode & 0x00ff) == 0x0048) ||
	 ((opcode & 0x000f) == 0x0000));
}

void tab_exg(FILE *fp, U16 opcode, const char *mnemonic)
{
  U32 base = TEF_SRC | TEF_SRC_LOAD | TEF_DST | TEF_DST_LOAD |
    (opcode & 0x0e0f) | TEF_SRC_LONG | TEF_DST_LONG | TEF_WRITE_BACK;

  if ((opcode & 0x00f8) == 0x0048) {
    /* Address registers */
    fprintf(fp, "0x%08x, opcode_%04x",
	    base | 0x1000, opcode & 0xfff8);
  } else {
    fprintf(fp, "0x%08x, opcode_%04x",
	    base, (opcode & 0xff30) | 0x0040);
  }
}

void as_exg(FILE *fp, U16 opcode, const char *mnemonic)
{
  if ((opcode & 0x00f8) == 0x0048) {
    /* Address registers */
    fprintf(fp,
	    "	st	%%acc0, [ %%regs + 0x%02x ]\n"
	    "	mov	%%acc1, %%acc0\n",
	    (((opcode & 0x0e00)>>7) + (M_A0 <<2)));
  } else {
    /* src is a data register */
    fprintf(fp,
	    "	st	%%acc0, [ %%regs + 0x%02x ]\n"
	    "	mov	%%acc1, %%acc0\n",
	    ((opcode & 0x0e00)>>7));
  }
}

int head_and(U16 opcode)
{
  return(opcode == 0xc000);
}

void tab_and(FILE *fp, U16 opcode, const char *mnemonic)
{
  U32 base = TEF_SRC | TEF_SRC_LOAD | TEF_DST | TEF_DST_LOAD |
    (opcode & 0x00c0) | ((opcode & 0x00c0)<<9) | TEF_WRITE_BACK;

  if (opcode & 0x0100) {
    fprintf(fp, "0x%08x, opcode_c000",
	    base | (opcode & 0x0e03f));
  } else {
    fprintf(fp, "0x%08x, opcode_c000",
	    base | ((opcode & 0x0e00)>>9) | ((opcode & 0x003f)<<9));
  }
}

void as_and(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	%%acc0, %%acc1, %%acc0\n"
	  "	and	-16, %%sr, %%sr\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

int head_adda(U16 opcode)
{
  return(opcode == 0xd0c0);
}

void tab_adda(FILE *fp, U16 opcode, const char *mnemonic)
{
  U32 base = TEF_SRC | TEF_SRC_LOAD | ((opcode & 0x3f)<<9) |
    TEF_DST | TEF_DST_LOAD | TEF_DST_LONG | ((opcode & 0x0e00)>>9) | 0x0008 |
    TEF_WRITE_BACK;
  base |= ((opcode & 0x0100)?TEF_SRC_LONG:TEF_SRC_WORD);
  fprintf(fp, "0x%08x, opcode_d0c0", base);
}

void as_adda(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "	add	%%acc0, %%acc1, %%acc0\n");
}

int head_add(U16 opcode)
{
  return(opcode == 0xd000);
}

void tab_add(FILE *fp, U16 opcode, const char *mnemonic)
{
  if (opcode & 0x0100) {
    /* Dn + (Ea) => (Ea) */
    /* LOAD_EA, LOAD_EO, WRITE_BACK */
    fprintf(fp, "0x%08x, opcode_d000",
	    TEF_SRC | TEF_SRC_LOAD | (opcode & 0x0e00) | ((opcode & 0x00c0)<<9) |
	    TEF_DST | TEF_DST_LOAD | (opcode & 0x00ff) |
	    TEF_WRITE_BACK | TEF_FIX_SR);
  } else {
    /* Dn + (Ea) => Dn */
    /* LOAD_EA, LOAD_EO */
    fprintf(fp, "0x%08x, opcode_d000",
	    TEF_SRC | TEF_SRC_LOAD | ((opcode & 0x00ff)<<9) |
	    TEF_DST | TEF_DST_LOAD | ((opcode & 0x0e00)>>9) | (opcode & 0x00c0) |
	    TEF_WRITE_BACK | TEF_FIX_SR);
  }
}

void as_add(FILE *fp, U16 opcode, const char *mnemonic)
{
  fputs("	addcc	%acc0, %acc1, %acc0\n", fp);
}

int head_shift(U16 opcode)
{
  return(((opcode & 0xf0ff) == 0xe0c0)	/* Memory shift */ ||
	 ((opcode & 0xf027) == 0xe000)	/* Immediate shift */ ||
	 ((opcode & 0xfe27) == 0xe020)); /* Register shift */
}

void tab_shift_mem(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_%04x",
	  TEF_DST | TEF_DST_LOAD | TEF_DST_BYTE | (opcode & 0x003f) |
	  TEF_WRITE_BACK, opcode & 0xffc0);
}

void tab_shift_imm(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_%04x",
	  TEF_DST | TEF_DST_LOAD | (opcode & 0x00c7) | TEF_WRITE_BACK |
	  TEF_SRC_QUICK8 | ((opcode & 0x0e00)?(opcode & 0x0e00):0x1000),
	  opcode & 0xfff8);
}

void tab_shift_reg(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_%04x",
	  TEF_SRC | TEF_SRC_LOAD | TEF_SRC_BYTE | (opcode & 0x0e00) |
	  TEF_DST | TEF_DST_LOAD | (opcode & 0x00c7) | TEF_WRITE_BACK,
	  opcode & 0xf1f8);
}

void as_asr_mem(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	-32, %%sr, %%sr\n"
	  "	btst	1, %%acc0\n"
	  "	sra	%%acc0, 1, %%acc0\n"
	  "	bne,a	0f\n"
	  "	or	0x11, %%sr, %%sr\n"
	  "0:\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

void as_asr_imm(FILE *fp, U16 opcode, const char *mnemonic)
{
  int shiftval = ((opcode & 0x0e00)?((opcode & 0x0e00)>>9):8);

  fprintf(fp,
	  "	and	-32, %%sr, %%sr\n"
	  "	btst	0x%02x, %%acc0\n"
	  "	sra	%%acc0, 0x%02x, %%acc0\n",
	  (1 << (shiftval - 1)),
	  shiftval);
  fprintf(fp,
	  "	bne,a	0f\n"
	  "	or	0x11, %%sr, %%sr\n"
	  "0:\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

void as_asr_reg(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* FIXME: C & X support fails on shifts > 31bits */
  fprintf(fp,
	  "	sll	%%acc0, 1, %%o0\n"
	  "	and	0x3f, %%acc1, %%acc1\n"
	  "	and	-32, %%sr, %%sr\n"
	  "	sra	%%o0, %%acc1, %%o0\n"
	  "	sra	%%acc0, %%acc1, %%acc0\n"
	  "	btst	1, %%o0\n"
	  "	bne,a	0f\n"
	  "	or	0x11, %%sr, %%sr\n"
	  "0:\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	blt,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

void as_asl_mem(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	-32, %%sr, %%sr\n"
	  "	andcc	0x80, %%acc0, %%o0\n"
	  "	sll	%%acc0, 1, %%acc0\n"
	  "	bne,a	0f\n"
	  "	or	0x11, %%sr, %%sr\n"	/* C & X */
	  "0:\n"
	  "	xor	%%o0, %%acc0, %%o0\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	ble,a	0f\n"
	  "	or	8, %%sr, %%sr\n"	/* N */
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"	/* Z */
	  "0:\n"
	  "	btst	0x80, %%o0\n"
	  "	bne,a	0f\n"
	  "	or	2, %%sr, %%sr\n"	/* V */
	  "0:\n");
}

void as_lsr_mem(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* Memory shift */
  fprintf(fp,
	  "	and	-32, %%sr, %%sr\n"
	  "	btst	1, %%acc0\n"
	  "	srl	%%acc0, 1, %%acc0\n"
	  "	bne,a	0f\n"
	  "	or	0x11, %%sr, %%sr\n"
	  "0:\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	ble,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

void as_lsr_imm(FILE *fp, U16 opcode, const char *mnemonic)
{
  int shiftval = ((opcode & 0x0e00)?((opcode & 0x0e00)>>9):8);

  fprintf(fp, "	and	-32, %%sr, %%sr\n");

  if ((opcode & 0xc0) == 0x40) {
    fprintf(fp, "	sethi	%%hi(0xffff0000), %%o0\n");
  }

  fprintf(fp, "	btst	0x%02x, %%acc0\n", (1 << (shiftval - 1)));

  if (!(opcode & 0x00c0)) {
    fprintf(fp,
	    "	and	0xff, %%acc0, %%acc0\n");
  } else if ((opcode & 0x00c0) == 0x0040) {
    fprintf(fp,
	    "	andn	%%acc0, %%o0, %%acc0\n");
  }
  
  fprintf(fp, "	srl	%%acc0, 0x%02x, %%acc0\n", shiftval);
  fprintf(fp, 
	  "	bne,a	0f\n"
	  "	or	0x11, %%sr, %%sr\n"
	  "0:\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	ble,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

void as_lsr_reg(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* FIXME: C & X support fails on shifts > 31bits */

  fprintf(fp, "	and	-32, %%sr, %%sr\n");

  if ((opcode & 0xc0) == 0x40) {
    fprintf(fp, "	sethi	%%hi(0xffff0000), %%o0\n");
  }

  fprintf(fp, "	and	0x3f, %%acc1, %%acc1\n");

  if (!(opcode & 0x00c0)) {
    fprintf(fp,
	    "	and	0xff, %%acc0, %%o0\n");
  } else if ((opcode & 0x00c0) == 0x0040) {
    fprintf(fp,
	    "	andn	%%acc0, %%o0, %%o0\n");
  }
  fprintf(fp,
	  "	sll	%%acc0, 1, %%o1\n"
	  "	srl	%%o0, %%acc1, %%acc0\n"
	  "	srl	%%o1, %%acc1, %%o1\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	ble,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n"
	  "	btst	1, %%o1\n"
	  "	bne,a	0f\n"
	  "	or	0x11, %%sr, %%sr\n"
	  "0:\n");
}

void as_lsl_mem(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	-32, %%sr, %%sr\n"
	  "	btst	0x80, %%acc0\n"
	  "	sll	%%acc0, 1, %%acc0\n"
	  "	bne,a	0f\n"
	  "	or	0x11, %%sr, %%sr\n"
	  "0:\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	ble,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

void as_lsl_imm(FILE *fp, U16 opcode, const char *mnemonic)
{
  int shiftval = ((opcode & 0x0e00)?((opcode & 0x0e00)>>9):8);
  int size = (8<<((opcode & 0xc0)>>6));
  unsigned mask = 1<<(size - shiftval);

  if (mask > 0xfff) {
    fprintf(fp,
	    "	and	-32, %%sr, %%sr\n"
	    "	sethi	%%hi(0x%08x), %%o0\n"
	    "	mov	0x%02x, %%acc1\n"
	    "	btst	%%o0, %%acc0\n",
	    mask, shiftval);
  } else {
    fprintf(fp,
	    "	and	-32, %%sr, %%sr\n"
	    "	mov	0x%02x, %%acc1\n"
	    "	btst	0x%04x, %%acc0\n",
	    shiftval, mask);
  }
  fprintf(fp,
	  "	sll	%%acc0, %%acc1, %%acc0\n"
	  "	bne,a	0f\n"
	  "	or	0x11, %%sr, %%sr\n"
	  "0:\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	ble,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

void as_lsl_reg(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	0x3f, %%acc1, %%acc1\n"
	  "	mov	1, %%o0\n"
	  "	add	-1, %%acc1, %%o1\n"
	  "	and	-32, %%sr, %%sr\n"
	  "	sll	%%o0, %%o1, %%o0\n"
	  "	btst	%%acc0, %%o0\n"
	  "	sll	%%acc0, %%acc1, %%acc0\n"
	  "	bne,a	0f\n"
	  "	or	0x11, %%sr, %%sr\n"
	  "0:\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	ble,a	0f\n"
	  "	or	8, %%sr, %%sr\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "0:\n");
}

void as_ror_mem(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* Check if operand zero */
  fprintf(fp,
	  "	and	-16, %%sr, %%sr\n"
	  "	andcc	0xff, %%acc0, %%o0\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"	/* Z */
	  "	btst	1, %%acc0\n"
	  "	sll	%%acc0, 7, %%o0\n"
	  "	srl	%%acc0, 1, %%acc0\n"
	  "	or	%%acc0, %%o0, %%acc0\n"
	  "	bne,a	0f\n"
	  "	or	9, %%sr, %%sr\n"	/* N & C */
	  "0:\n");
}

void as_ror_imm(FILE *fp, U16 opcode, const char *mnemonic)
{
  int shiftval = ((opcode & 0x0e00)?((opcode & 0x0e00)>>9):8);
  unsigned mask = 1 << (shiftval - 1);

  fprintf(fp,
	  "	and	-16, %%sr, %%sr\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"	/* Z */
	  "	btst	0x%02x, %%acc0\n",
	  mask);

  switch (opcode & 0x00c0) {
  case 0x00:
    /* ROR.B */
    fprintf(fp,
	    "	and	0xff, %%acc0, %%o1\n"
	    "	sll	%%acc0, 0x%02x, %%o0\n"
	    "	srl	%%o1, 0x%02x, %%acc0\n"
	    "	or	%%acc0, %%o0, %%acc0\n"
	    "	bne,a	0f\n"
	    "	or	9, %%sr, %%sr\n",	/* N & C */
	    8 - shiftval, shiftval);
    break;
  case 0x40:
    /* ROR.W */
    fprintf(fp,
	    "	sethi	%%hi(0xffff0000), %%o0\n"
	    "	andn	%%acc0, %%o0, %%o0\n"
	    "	sll	%%acc0, 0x%02x, %%acc0\n"
	    "	srl	%%o0, 0x%02x, %%o0\n"
	    "	or	%%acc0, %%o0, %%acc0\n"
	    "	bne,a	0f\n"
	    "	or	9, %%sr, %%sr\n",	/* N & C */
	    16 - shiftval, shiftval);
    break;
  case 0x80:
    /* ROR.L */
    fprintf(fp,
	    "	sll	%%acc0, 0x%02x, %%o0\n"
	    "	srl	%%acc0, 0x%02x, %%acc0\n"
	    "	or	%%acc0, %%o0, %%acc0\n"
	    "	bne,a	0f\n"
	    "	or	9, %%sr, %%sr\n",	/* N & C */
	    32 - shiftval, shiftval);
    break;
  }
  fprintf(fp, "0:\n");
}

void as_ror_reg(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	-16, %%sr, %%sr\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"	/* Z */
	  "	and	0x3f, %%acc1, %%acc1\n");

  switch (opcode & 0x00c0) {
  case 0x00:
    /* ROR.B */
    fprintf(fp,
	    "	mov	8, %%o0\n"
	    "	and	0xff, %%acc0, %%acc0\n"
	    "	sub	%%o0, %%acc1, %%o0\n"
	    "	srl	%%acc0, %%acc1, %%o1\n"
	    "	sll	%%acc0, %%o0, %%acc0\n"
	    "	or	%%acc0, %%o1, %%acc0\n"
	    "	btst	0x80, %%acc0\n"
	    "	bne,a	0f\n"
	    "	or	9, %%sr, %%sr\n");
    break;
  case 0x40:
    /* ROR.W */
    fprintf(fp,
	    "	sethi	%%hi(0xffff0000), %%o2\n"
	    "	mov	16, %%o0\n"
	    "	andn	%%acc0, %%o2, %%acc0\n"
	    "	sub	%%o0, %%acc1, %%o0\n"
	    "	srl	%%acc0, %%acc1, %%o1\n"
	    "	sll	%%acc0, %%o0, %%acc0\n"
	    "	sethi	%%hi(0x8000), %%o2\n"
	    "	or	%%acc0, %%o1, %%acc0\n"
	    "	btst	%%acc0, %%o2\n"
	    "	bne,a	0f\n"
	    "	or	9, %%sr, %%sr\n");
    break;
  case 0x80:
    /* ROR.L */
    fprintf(fp,
	    "	mov	32, %%o0\n"
	    "	srl	%%acc0, %%acc1, %%o1\n"
	    "	sub	%%o0, %%acc1, %%o0\n"
	    "	sethi	%%hi(0x80000000), %%o2\n"
	    "	sll	%%acc0, %%o0, %%acc0\n"
	    "	btst	%%o2, %%acc0\n"
	    "	or	%%acc0, %%o1, %%acc0\n"
	    "	bne,a	0f\n"
	    "	or	9, %%sr, %%sr\n");
    break;
  }
  fprintf(fp, "0:\n");
}

void as_rol_mem(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	and	-16, %%sr, %%sr\n"
	  "	andcc	0xff, %%acc0, %%o0\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"	/* Z */
	  "	srl	%%o0, 7, %%o0\n"
	  "	sll	%%acc0, 1, %%acc0\n"
	  "	btst	1, %%o0\n"
	  "	or	%%acc0, %%o0, %%acc0\n"
	  "	bne,a	1f\n"
	  "	or	1, %%sr, %%sr\n"	/* C */
	  "1:\n"
	  "	btst	0x80, %%acc0\n"
	  "	bne,a	0f\n"
	  "	or	8, %%sr, %%sr\n"	/* N */
	  "0:\n");
}

void as_rol_imm(FILE *fp, U16 opcode, const char *mnemonic)
{
  int shiftval = ((opcode & 0x0e00)?((opcode & 0x0e00)>>9):8);

  fprintf(fp,
	  "	and	-16, %%sr, %%sr\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n");

  switch (opcode & 0x00c0) {
  case 0x00:
    /* ROL.B */
    fprintf(fp,
	    "	and	0xff, %%acc0, %%o0\n"
	    "	sll	%%acc0, 0x%02x, %%acc0\n"
	    "	srl	%%o0, 0x%02x, %%o0\n"
	    "	and	1, %%o0, %%o1\n"
	    "	and	0x80, %%acc0, %%o2\n"
	    "	or	%%o0, %%acc0, %%acc0\n"
	    "	srl	%%o2, 4, %%o0\n"
	    "	or	%%o1, %%sr, %%sr\n"	/* C */
	    "	or	%%o0, %%sr, %%sr\n",	/* N */
	    shiftval, 8 - shiftval);
    break;
  case 0x40:
    /* ROL.W */
    fprintf(fp,
	    "	sethi	%%hi(0xffff0000), %%o1\n"
	    "	sll	%%acc0, 0x%02x, %%o0\n"
	    "	andn	%%acc0, %%o1, %%acc0\n"
	    "	sethi	%%hi(0x8000), %%o1\n"
	    "	srl	%%acc0, 0x%02x, %%acc0\n"
	    "	and	%%o0, %%o1, %%o0\n"
	    "	and	1, %%acc0, %%o2\n"
	    "	or	%%acc0, %%o1, %%acc0\n"
	    "	srl	%%o0, 12, %%o0\n"
	    "	or	%%o2, %%sr, %%sr\n"
	    "	or	%%o0, %%sr, %%sr\n",
	    shiftval, 16 - shiftval);
    break;
  case 0x80:
    /* ROL.L */
    fprintf(fp,
	    "	sll	%%acc0, 0x%02x, %%o0\n"
	    "	sethi	%%hi(0x80000000), %%o1\n"
	    "	srl	%%acc0, 0x%02x, %%acc0\n"
	    "	and	%%o0, %%o1, %%o1\n"
	    "	and	1, %%acc0, %%o2\n"
	    "	or	%%acc0, %%o0, %%acc0\n"
	    "	srl	%%o1, 28, %%o1\n"
	    "	or	%%o2, %%sr, %%sr\n"
	    "	or	%%o1, %%sr, %%sr\n",
	    shiftval, 32 - shiftval);
    break;
  }
  fprintf(fp, "0:\n");
}

void as_rol_reg(FILE *fp, U16 opcode, const char *mnemonic)
{
  /* FIXME: Doesn't clear C when rotating with 0 */

  fprintf(fp,
	  "	and	-16, %%sr, %%sr\n"
	  "	cmp	%%acc0, %%g0\n"
	  "	be,a	0f\n"
	  "	or	4, %%sr, %%sr\n"
	  "	and	0x3f, %%acc1, %%acc1\n");

  /* Rotate left */
  switch (opcode & 0x00c0) {
  case 0x00:
    /* ROL.B */
    fprintf(fp,
	    "	mov	8, %%o0\n"
	    "	sll	%%acc0, %%acc1, %%o1\n"
	    "	sub	%%o0, %%acc1, %%o0\n"
	    "	and	0xff, %%acc0, %%acc0\n"
	    "	srl	%%acc0, %%o0, %%acc0\n"
	    "	or	%%acc0, %%o1, %%acc0\n"
	    "	and	1, %%acc0, %%o0\n"
	    "	and	0x80, %%acc0, %%o1\n"
	    "	or	%%o0, %%sr, %%sr\n"
	    "	srl	%%o1, 4, %%o1\n"
	    "	or	%%o1, %%sr, %%sr\n");
    break;
  case 0x40:
    /* ROL.W */
    fprintf(fp,
	    "	mov	16, %%o0\n"
	    "	sethi	%%hi(0xffff0000), %%o2\n"
	    "	sll	%%acc0, %%acc1, %%o1\n"
	    "	sub	%%o0, %%acc1, %%o0\n"
	    "	andn	%%acc0, %%o2, %%acc0\n"
	    "	srl	%%acc0, %%o0, %%acc0\n"
	    "	or	%%acc0, %%o1, %%acc0\n"
	    "	sethi	%%hi(0x8000), %%o1\n"
	    "	and	1, %%acc0, %%o0\n"
	    "	and	%%o1, %%acc0, %%o1\n"
	    "	or	%%o0, %%sr, %%sr\n"
	    "	srl	%%o1, 12, %%o1\n"
	    "	or	%%o1, %%sr, %%sr\n");
    break;
  case 0x80:
    /* ROL.L */
    fprintf(fp,
	    "	mov	32, %%o0\n"
	    "	sll	%%acc0, %%acc1, %%o1\n"
	    "	sub	%%o0, %%acc1, %%o0\n"
	    "	sethi	%%hi(0x80000000), %%o2\n"
	    "	srl	%%acc0, %%o0, %%acc0\n"
	    "	and	%%o2, %%o1, %%o2\n"
	    "	or	%%acc0, %%o1, %%acc0\n"
	    "	srl	%%o2, 28, %%o1\n"
	    "	and	1, %%acc0, %%o0\n"
	    "	or	%%o1, %%sr, %%sr\n"
	    "	or	%%o0, %%sr, %%sr\n");
    break;
  }
  fprintf(fp, "0:\n");
}

int head_line_f(U16 opcode)
{
  return (opcode == 0xf000);
}

void tab_line_f(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "0x%08x, opcode_f000", TEF_TERMINATE);
}

void as_line_f(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp,
	  "	mov	0x%04x, %%acc0\n"
	  "	mov	%%sr, %%acc1\n", VEC_LINE_F);
  as_exception(fp, opcode);
}

/*
 * Some help functions
 */

int comp_supervisor(FILE *fp, U16 opcode, const char *mnemonic)
{
  fprintf(fp, "/* FIXME: Supervisor mode only instruction */\n");
  return(0);
}

int comp_clobber(FILE *fp, U8 sdst, U8 sval, U8 size)
{
  switch(size) {
  case 0: /* BYTE */
    fprintf(fp, "emit_clobber_byte(code, 0x%02x, 0x%02x);\n", sdst, sval);
    break;
  case 1: /* SHORT */
    fprintf(fp, "emit_clobber_short(code, 0x%02x, 0x%02x);\n", sdst, sval);
    break;
  case 2: /* LONG */
    fprintf(fp, "emit_clobber(code, 0x%02x, 0x%02x);\n", sdst, sval);
    break;
  default: /* ERROR */
    fprintf(fp, "/* Bad clobber size %d, dst=0x%02x, val=0x%02x */\n",
	    size, sdst, sval);
    break;
  }
  return(0);
}

int comp_load_ea(FILE *fp, U8 size, U8 mode, U8 mreg)
{
  switch(mode) {
  case 0x02:	/* (An) */
    fprintf(fp, "S_LD(S_I0, 0x%02x, eareg);\n", (M_A0 + mreg)<<2);
    break;
  case 0x03:	/* (An)+ */
    fprintf(fp, "S_LD(S_I0, 0x%02x, eareg);\n", (M_A0 + mreg)<<2);
    if ((mreg == 7) && (!size)) {
      size = 1;
    }
    fprintf(fp, "S_ADDI(0x%02x, eareg, S_O0);\n", 1<<size);
    fprintf(fp, "S_STI(S_O0, S_I0, 0x%02x);\n", (M_A0 + mreg)<<2);
    break;
  case 0x04:	/* -(An) */
    fprintf(fp, "S_LD(S_I0, 0x%02x, eareg);\n", (M_A0 + mreg)<<2);
    if ((mreg == 7) && (!size)) {
      size = 1;
    }
    fprintf(fp, "S_SUBI(0x%02x, eareg, eareg);\n", 1<<size);
    fprintf(fp, "S_STI(eareg, S_I0, 0x%02x);\n", (M_A0 + mreg)<<2);
    break;
  case 0x05:	/* (d16, An) */
    fprintf(fp, "{\nSHORT off = mem[(*pc)++];\n");
    fprintf(fp, "S_LD(S_I0, 0x%02x, eareg);\n", (M_A0 + mreg)<<2);
    fprintf(fp, "if ((off & 0xf000) && ((off & 0xf000) != 0xf000)) {\n");
    fprintf(fp, "S_SETHI(off, S_O0);\n");
    fprintf(fp, "S_ADD(S_O0, eareg, eareg);\n");
    fprintf(fp, "S_ADDI(off & 0x0fff, eareg, eareg);\n");
    fprintf(fp, "} else {\n");
    fprintf(fp, "S_ADDI(off & 0x1fff, eareg, eareg);\n");
    fprintf(fp, "}\n");
    fprintf(fp, "}\n");
    break;
  case 0x06:	/* (d8, An, Xn) */
    fprintf(fp, "{\n");
    fprintf(fp, "U16 format = mem[(*pc)++];\n");
    fprintf(fp, "if (format & 0x0100) {\n");	/* M68020+? */
    fprintf(fp, "emit_exception(code, *pc - 1, VEC_ILL_INSTR);\n");
    fprintf(fp, "return;/* FIXME: */\n");
    fprintf(fp, "}\n");
    /* Fetch address register */
    fprintf(fp, "S_LD(S_I0, 0x%02x, eareg);\n", (M_A0 + mreg)<<2);
    /* Displacement */
    fprintf(fp, "if (format & 0x0080) {\n");
    fprintf(fp, "S_ADDI(0x1f00 | (format & 0x00ff), eareg, eareg);\n");
    fprintf(fp, "} else {\n");
    fprintf(fp, "S_ADDI((format & 0x007f), eareg, eareg);\n");
    fprintf(fp, "}\n");
    /* Index */
    fprintf(fp, "if (format & 0x0800) {\n");
    fprintf(fp, "S_LDH(S_I0, ((format & 0xf000)>>10)+2, S_O0);\n");
    fprintf(fp, "} else {\n");
    fprintf(fp, "S_LD(S_I0, ((format & 0xf000)>>10), S_O0);\n");
    fprintf(fp, "}\n");
    /* Scale */
    fprintf(fp, "if (format & 0x0600) {\n");
    fprintf(fp, "S_SLLI(S_O0, ((format & 0x0600)>>9), S_O0);\n");
    fprintf(fp, "}\n");
    /* Add on scaled index */
    fprintf(fp, "S_ADD(S_O0, eareg, eareg);\n");
    fprintf(fp, "}\n");
    break;
  case 0x07:	/* Other modes */
    switch(mreg) {
    case 0x00:	/* (d16).W */
      fprintf(fp, "{\nSHORT addr = mem[(*pc)++];\n");
      fprintf(fp, "if ((addr & 0xf000) && ((addr & 0xf000) != 0xf000)) {\n");
      fprintf(fp, "S_SETHI(addr, eareg);\n");
      fprintf(fp, "S_ORI(addr & 0xfff, eareg, eareg);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_MOVI(addr & 0x1fff, eareg);\n");
      fprintf(fp, "}\n");
      fprintf(fp, "}\n");
      break;
    case 0x01:	/* (d32).L */
      fprintf(fp, "{\nU32 addr = mem[(*pc)++];\n");
      fprintf(fp, "addr = (addr << 16) | mem[(*pc)++];\n");
      fprintf(fp, "if ((addr & 0xfffff000) && ((addr & 0xfffff000) != 0xfffff000)) {\n");
      fprintf(fp, "S_SETHI(addr, eareg);\n");
      fprintf(fp, "S_ORI(addr & 0xfff, eareg, eareg);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_MOVI(addr & 0x1fff, eareg);\n");
      fprintf(fp, "}\n");
      fprintf(fp, "}\n");
      break;
    case 0x02:	/* (d16, PC) */
      fprintf(fp, "{\nU32 addr = (*pc)<<1;\n");
      fprintf(fp, "addr += ((SHORT *)mem)[(*pc)++];\n");
      fprintf(fp, "if ((addr & 0xfffff000) && ((addr & 0xfffff000) != 0xfffff000)) {\n");
      fprintf(fp, "S_SETHI(addr, eareg);\n");
      fprintf(fp, "S_ORI(addr & 0xfff, eareg, eareg);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_MOVI(addr & 0x1fff, eareg);\n");
      fprintf(fp, "}\n");
      fprintf(fp, "}\n");
      break;
    case 0x03:	/* (d8, PC, Xn) */
      fprintf(fp, "{\n");
      fprintf(fp, "U32 addr = (*pc)<<1;\n");
      fprintf(fp, "U16 format = mem[(*pc)++];\n");
      fprintf(fp, "if (format & 0x0100) {\n");	/* M68020+? */
      fprintf(fp, "emit_exception(code, *pc - 1, VEC_ILL_INSTR)\n");
      fprintf(fp, "return(1);/* FIXME: */\n");
      fprintf(fp, "}\n");
      /* Displacement */
      fprintf(fp, "if (format & 0x0080) {\n");
      fprintf(fp, "addr += 0xffffff00 | (format & 0xff);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "addr += (format & 0x7f);\n");
      fprintf(fp, "}\n");
      /* Index */
      fprintf(fp, "if (format & 0x0800) {\n");
      fprintf(fp, "S_LDH(S_I0, ((format & 0xf000)>>10)+2, S_O0);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_LD(S_I0, ((format & 0xf000)>>10), S_O0);\n");
      fprintf(fp, "}\n");
      /* Scale */
      fprintf(fp, "if (format & 0x0600) {\n");
      fprintf(fp, "S_SLLI(S_O0, ((format & 0x0600)>>9), S_O0);\n");
      fprintf(fp, "}\n");
      fprintf(fp, "if ((addr & 0xfffff000) && ((addr & 0xfffff000) != 0xfffff000)) {\n");
      fprintf(fp, "S_SETHI(addr, eareg);\n");
      fprintf(fp, "S_ORI(addr & 0xfff, eareg, eareg);\n");
      fprintf(fp, "S_ADD(S_O0, eareg, eareg);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_ADDI(addr & 0x1fff, S_O0, eareg);\n");
      fprintf(fp, "}\n");
      fprintf(fp, "}\n");
      break;
    default:
      fprintf(fp, "/* Unknown addressing mode 0x%02x, mreg 0x%02x */\n", mode, mreg);
      break;
    }
    break;
  default:
    fprintf(fp, "/* Unknown addressing mode 0x%02x, mreg 0x%02x */\n", mode, mreg);
    break;
  }
  return(0);
}

int comp_load(FILE *fp, U8 size, U8 mode, U8 mreg)
{
  if ((mode & 0xfe) && (!((mode == 0x07) && (mreg == 0x04)))) {
    /* Memory access */
    if (mode == 2) {
      /* Inline optimization */
      fprintf(fp, "U8 eareg=S_L5;\n");
      comp_load_ea(fp, size, mode, mreg);
    } else {
      fprintf(fp, "emit_load_ea_%02x(pc, code, mem, S_L5);\n",
	      (size << 6) | (mode << 3) | (mreg));
    }
    fprintf(fp, "S_CMP(S_L5, S_I4);\n");
    fprintf(fp, "S_BCC(((U32)code)+0x18);\n");	/* To memory access ***** */
    /* Hw */
    fprintf(fp, "S_MOV(sreg, S_O1);\n");	/* Before-value */
    switch(size) {
    case 0: /* BYTE */
      fprintf(fp, "S_CALL(read_hw_byte);\n");
      break;
    case 1: /* SHORT */
      fprintf(fp, "S_CALL(read_hw_short);\n");
      break;
    case 2:
      fprintf(fp, "S_CALL(read_hw_long);\n");
      break;
    default:
      fprintf(fp, "/* Unknown size: %d, mode 0x%02x, reg 0x%02x */\n",
	      size, mode, mreg);
      fprintf(fp, "S_NOP;\n");
      break;
    }
    fprintf(fp, "S_MOV(S_L5, S_O0);\n");		/* maddr */
    if (size == 2) {
      fprintf(fp, "S_B(((U32)code)+0x18);\n");	/* to end **** */
    } else {
      fprintf(fp, "S_B(((U32)code)+0x08);\n");	/* to end **** */
    }
    fprintf(fp, "S_MOV(S_O0, sreg);\n");		/* result */
    
    /* Memory access */
    switch(size) {
    case 0: /* BYTE */
      fprintf(fp, "S_LDUB(S_L5, S_I1, sreg);\n");
      break;
    case 1: /* SHORT */
      fprintf(fp, "S_LDUH(S_L5, S_I1, sreg);\n");
      break;
    case 2: /* LONG */
      fprintf(fp, "S_LDUH(S_L5, S_I1, sreg);\n");
      fprintf(fp, "S_SLLI(sreg, 0x10, sreg);\n");
      fprintf(fp, "S_ADDI(0x02, S_L5, S_O0);\n");
      fprintf(fp, "S_LDUH(S_O0, S_I1, S_O0);\n");
      fprintf(fp, "S_OR(S_O0, sreg, sreg);\n");
      break;
    default:
      fprintf(fp, "/* Unknown size: %d, mode 0x%02x, mreg 0x%02x */\n",
	      size, mode, mreg);
      fprintf(fp, "S_NOP;\n");
      break;
    }
  } else if (mode == 0x07) {
    /* Immediate */
    switch(size) {
    case 0:	/* BYTE */
      fprintf(fp, "{\nU16 val = mem[(*pc)++];\n");
      fprintf(fp, "if (val & 0x0080) {\n");
      fprintf(fp, "S_MOVI((0x1f00 | (val & 0xff)), sreg);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_MOVI((val & 0x7f), sreg);\n");
      fprintf(fp, "}\n}\n");
      break;
    case 1:	/* WORD */
      fprintf(fp, "{\nU32 val = ((SHORT *)mem)[(*pc)++];\n");
      fprintf(fp, "if ((val & 0xfffff000) && ((val & 0xfffff000) != 0xfffff000)) {\n");
      fprintf(fp, "S_SETHI(val, sreg);\n");
      fprintf(fp, "S_ORI((val & 0xfff), sreg, sreg);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_MOVI((val & 0x1fff), sreg);\n");
      fprintf(fp, "}\n}\n");
      break;
    case 2:	/* LONG */
      fprintf(fp, "{\nU32 val = mem[(*pc)++];\n");
      fprintf(fp, "val = (val << 16) | mem[(*pc)++];\n");
      fprintf(fp, "if ((val & 0xfffff000) && ((val & 0xfffff000) != 0xfffff000)) {\n");
      fprintf(fp, "S_SETHI(val, sreg);\n");
      fprintf(fp, "S_ORI((val & 0xfff), sreg, sreg);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_MOVI((val & 0x1fff), sreg);\n");
      fprintf(fp, "}\n}\n");
      break;
    default:
      /* ERROR */
      fprintf(fp, "/* Unknown size %d, mode 0x%02x, mreg 0x%02x */\n",
	      size, mode, mreg);
      break;
    }
  } else {
    /* Register access */
    switch(size) {
    case 0: /* BYTE */
      fprintf(fp, "S_LDUBI(S_I0, 0x%02x, sreg);\n", ((M_A0*mode + mreg) << 2) + 3);
      break;
    case 1: /* SHORT */
      fprintf(fp, "S_LDUHI(S_I0, 0x%02x, sreg);\n", ((M_A0*mode + mreg) << 2) + 2);
      break;
    case 2: /* LONG */
      fprintf(fp, "S_LDI(S_I0, 0x%02x, sreg);\n", ((M_A0*mode + mreg)<<2));
      break;
    default:
      fprintf(fp, "/* Unknown size: %d, mode 0x%02x, mreg 0x%02x */\n",
	      size, mode, mreg);
      break;
    }
  }
  return(0);
}

/*
 * Structures
 */

struct opcode_info {
  U16 mask, tag;
  const char *mnemonic;
  int (*as_header)(U16 opcode);
  void (*tab_entry)(FILE *fp, U16 opcode, const char *);
  void (*assembler)(FILE *fp, U16 opcode, const char *);
  int (*compiler)(FILE *fp, U16 opcode, const char *);
};

/*
 * Globals
 */

struct opcode_info opcodes[] = {
  { 0xf138, 0x0108, "MOVEP", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xf1c0, 0x0100, "BTST", head_btst, tab_btst, as_btst, comp_default },
  { 0xf1c0, 0x0140, "BCHG", head_bchg, tab_bchg, as_bchg, comp_default },
  { 0xf1c0, 0x0180, "BCLR", head_bclr, tab_bclr, as_bclr, comp_default },
  { 0xf1c0, 0x01c0, "BSET", head_bset, tab_bset, as_bset, comp_default },
  { 0xffc0, 0x0800, "BTST", head_btst, tab_btst, as_btst, comp_default },
  { 0xffc0, 0x0840, "BCHG", head_bchg, tab_bchg, as_bchg, comp_default },
  { 0xffc0, 0x0880, "BCLR", head_bclr, tab_bclr, as_bclr, comp_default },
  { 0xffc0, 0x08c0, "BSET", head_bset, tab_bset, as_bset, comp_default },
  { 0xffff, 0x003c, "ORI_CCR", head_ori_ccr, tab_ori_ccr, as_ori_ccr, comp_default },
  { 0xffff, 0x007c, "ORI_SR", head_ori_sr, tab_ori_sr, as_ori_sr, comp_default },
  { 0xff00, 0x0000, "ORI", head_ori, tab_ori, as_ori, comp_default },
  { 0xffff, 0x023c, "ANDI_CCR", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xffff, 0x027c, "ANDI_SR", head_default, tab_andi_sr, as_andi_sr, comp_default },
  { 0xff00, 0x0200, "ANDI", head_andi, tab_andi, as_andi, comp_default },
  { 0xff00, 0x0400, "SUBI", head_subi, tab_subi, as_subi, comp_default },
  { 0xff00, 0x0600, "ADDI", head_addi, tab_addi, as_addi, comp_default },
  { 0xffff, 0x0a3c, "EORI_CCR", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xffff, 0x0a7c, "EORI_SR", head_default, tab_eori_sr, as_eori_sr, comp_default },
  { 0xff00, 0x0a00, "EORI", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xff00, 0x0c00, "CMPI", head_cmpi, tab_cmpi, as_cmpi, comp_default },
  { 0xff00, 0x0e00, "MOVES", head_not_implemented, tab_illegal, as_illegal, comp_default },
  
  { 0xf1c0, 0x1040, "MOVEA", head_movea, tab_movea, as_movea, comp_default },
  { 0xf000, 0x1000, "MOVE", head_move, tab_move, as_move, comp_default },
  { 0xf1c0, 0x2040, "MOVEA", head_movea, tab_movea, as_movea, comp_default },
  { 0xf000, 0x2000, "MOVE", head_move, tab_move, as_move, comp_default },
  { 0xf1c0, 0x3040, "MOVEA", head_movea, tab_movea, as_movea, comp_default },
  { 0xf000, 0x3000, "MOVE", head_move, tab_move, as_move, comp_default },

  { 0xf1c0, 0x4180, "CHK", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xf1c0, 0x41c0, "LEA", head_lea, tab_lea, as_lea, comp_default },

  { 0xffc0, 0x40c0, "MOVE_SR", head_move_sr, tab_move_sr, as_move_sr, comp_default },
  { 0xff00, 0x4000, "NEGX", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xffc0, 0x42c0, "MOVE_CCR", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xff00, 0x4200, "CLR", head_clr, tab_clr, as_clr, comp_default },
  { 0xffc0, 0x44c0, "MOVE_CCR", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xff00, 0x4400, "NEG", head_neg, tab_neg, as_neg, comp_default },
  { 0xffc0, 0x46c0, "MOVE_SR", head_move_sr, tab_move_sr, as_move_sr, comp_default },
  { 0xff00, 0x4600, "NOT", head_not, tab_not, as_not, comp_default },
  
  { 0xffc0, 0x4800, "NBCD", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xfff8, 0x4840, "SWAP", head_swap, tab_swap, as_swap, comp_default },
  { 0xfff8, 0x4848, "BKPT", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xffc0, 0x4840, "PEA", head_pea, tab_pea, as_pea, comp_default },
  { 0xffb8, 0x4880, "EXT", head_ext, tab_ext, as_ext, comp_default },
  { 0xfb80, 0x4880, "MOVEM", head_movem, tab_movem, as_movem, comp_default },

  { 0xfff0, 0x4e60, "MOVE_USP", head_default, tab_move_usp, as_move_usp, comp_default },
  
  { 0xffff, 0x4e70, "RESET", head_default, tab_reset, as_reset, comp_default },
  { 0xffff, 0x4e71, "NOP", head_default, tab_nop, as_nop, comp_default },
  { 0xffff, 0x4e72, "STOP", head_default, tab_stop, as_stop, comp_default },
  { 0xffff, 0x4e73, "RTE", head_default, tab_rte, as_rte, comp_default },
  { 0xffff, 0x4e74, "RTD", head_default, tab_rtd, as_rtd, comp_default },
  { 0xffff, 0x4e75, "RTS", head_default, tab_rts, as_rts, comp_default },
  { 0xffff, 0x4e77, "RTR", head_default, tab_illegal, as_illegal, comp_default },
  { 0xfffe, 0x4e7a, "MOVEC", head_default, tab_movec, as_movec, comp_default },

  { 0xffff, 0x4afc, "ILLEGAL", head_default, tab_illegal, as_illegal, comp_default },
  { 0xffc0, 0x4ac0, "TAS", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xff00, 0x4a00, "TST", head_tst, tab_tst, as_tst, comp_default },
  { 0xfff0, 0x4e40, "TRAP", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xfff8, 0x4e50, "LINK", head_link, tab_link, as_link, comp_default },
  { 0xfff8, 0x4e58, "UNLK", head_unlk, tab_unlk, as_unlk, comp_default },
  { 0xffff, 0x4e76, "TRAPV", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xffc0, 0x4e80, "JSR", head_jsr, tab_jsr, as_jsr, comp_default },
  { 0xffc0, 0x4ec0, "JMP", head_jmp, tab_jmp, as_jmp, comp_default },

  { 0xf0f8, 0x50c8, "DB", head_default, tab_dbcc, as_dbcc, comp_default },
  { 0xf0c0, 0x50c0, "S", head_scc, tab_scc, as_scc, comp_default },
  { 0xf100, 0x5000, "ADDQ", head_addq, tab_addq, as_addq, comp_default },
  { 0xf100, 0x5100, "SUBQ", head_subq, tab_subq, as_subq, comp_default },
  
  { 0xff00, 0x6000, "BRA", head_default, tab_bra, as_bra, comp_default },
  { 0xff00, 0x6100, "BSR", head_default, tab_bsr, as_bsr, comp_default },
  { 0xf000, 0x6000, "B", head_default, tab_bcc, as_bcc, comp_default },

  { 0xf000, 0x7000, "MOVEQ", head_moveq, tab_moveq, as_moveq, comp_default },

  { 0xf1c0, 0x80c0, "DIVU", head_divu, tab_divu, as_divu, comp_default },
  { 0xf1f0, 0x8100, "SBCD", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xf1c0, 0x81c0, "DIVS", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xf000, 0x8000, "OR", head_or, tab_or, as_or, comp_default },

  { 0xf0c0, 0x90c0, "SUBA", head_suba, tab_suba, as_suba, comp_default },
  { 0xf130, 0x9100, "SUBX", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xf000, 0x9000, "SUB", head_sub, tab_sub, as_sub, comp_default },

  { 0xf000, 0xa000, "LINE A", head_not_implemented, tab_illegal, as_illegal, comp_default },

  { 0xf0c0, 0xb0c0, "CMPA", head_cmpa, tab_cmpa, as_cmpa, comp_default },
  { 0xf100, 0xb000, "CMP", head_cmp, tab_cmp, as_cmp, comp_default },
  { 0xf138, 0xb108, "CMPM", head_cmpm, tab_cmpm, as_cmpm, comp_default },
  { 0xf100, 0xb100, "EOR", head_not_implemented, tab_illegal, as_illegal, comp_default },

  { 0xf1c0, 0xc0c0, "MULU", head_mulu, tab_mulu, as_mulu, comp_default },
  { 0xf1f0, 0xc100, "ABCD", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xf1f0, 0xc140, "EXG", head_exg, tab_exg, as_exg, comp_default },
  { 0xf1f8, 0xc188, "EXG", head_exg, tab_exg, as_exg, comp_default },
  { 0xf1c0, 0xc1c0, "MULS", head_muls, tab_muls, as_muls, comp_default },
  { 0xf000, 0xc000, "AND", head_and, tab_and, as_and, comp_default },

  { 0xf0c0, 0xd0c0, "ADDA", head_adda, tab_adda, as_adda, comp_default },
  { 0xf130, 0xd100, "ADDX", head_addx, tab_addx, as_addx, comp_default },
  { 0xf000, 0xd000, "ADD", head_add, tab_add, as_add, comp_default },

  { 0xffc0, 0xe0c0, "ASR", head_shift, tab_shift_mem, as_asr_mem, comp_default },
  { 0xffc0, 0xe1c0, "ASL", head_shift, tab_shift_mem, as_asl_mem, comp_default },
  { 0xffc0, 0xe2c0, "LSR", head_shift, tab_shift_mem, as_lsr_mem, comp_default },
  { 0xffc0, 0xe3c0, "LSL", head_shift, tab_shift_mem, as_lsl_mem, comp_default },
  { 0xffc0, 0xe4c0, "ROXR", head_shift, tab_shift_mem, as_illegal, comp_default },
  { 0xffc0, 0xe5c0, "ROXL", head_shift, tab_shift_mem, as_illegal, comp_default },
  { 0xffc0, 0xe7c0, "ROR", head_shift, tab_shift_mem, as_ror_mem, comp_default },
  { 0xffc0, 0xe7c0, "ROL", head_shift, tab_shift_mem, as_rol_mem, comp_default },
  { 0xf0c0, 0xe0c0, "UNKNOWN", head_not_implemented, tab_illegal, as_illegal, comp_default },
  { 0xf138, 0xe000, "ASR", head_shift, tab_shift_imm, as_asr_imm, comp_default },
  { 0xf138, 0xe100, "ASL", head_shift, tab_shift_imm, as_lsl_imm, comp_default }, /**/
  { 0xf138, 0xe008, "LSR", head_shift, tab_shift_imm, as_lsr_imm, comp_default },
  { 0xf138, 0xe108, "LSL", head_shift, tab_shift_imm, as_lsl_imm, comp_default },
  { 0xf138, 0xe010, "ROXR", head_shift, tab_shift_imm, as_illegal, comp_default },
  { 0xf138, 0xe110, "ROXL", head_shift, tab_shift_imm, as_illegal, comp_default },
  { 0xf138, 0xe018, "ROR", head_shift, tab_shift_imm, as_ror_imm, comp_default },
  { 0xf138, 0xe118, "ROL", head_shift, tab_shift_imm, as_rol_imm, comp_default },
  { 0xf138, 0xe020, "ASR", head_shift, tab_shift_reg, as_asr_reg, comp_default },
  { 0xf138, 0xe120, "ASL", head_shift, tab_shift_reg, as_lsl_reg, comp_default }, /**/
  { 0xf138, 0xe028, "LSR", head_shift, tab_shift_reg, as_lsr_reg, comp_default },
  { 0xf138, 0xe128, "LSL", head_shift, tab_shift_reg, as_lsl_reg, comp_default },
  { 0xf138, 0xe030, "ROXR", head_shift, tab_shift_reg, as_illegal, comp_default },
  { 0xf138, 0xe130, "ROXL", head_shift, tab_shift_reg, as_illegal, comp_default },
  { 0xf138, 0xe038, "ROR", head_shift, tab_shift_reg, as_ror_reg, comp_default },
  { 0xf138, 0xe138, "ROL", head_shift, tab_shift_reg, as_rol_reg, comp_default },

  { 0xf000, 0xf000, "LINE F", head_line_f, tab_line_f, as_line_f, comp_default },

  { 0x0000, 0x0000, "UNKNOWN", head_not_implemented, tab_illegal, as_illegal, comp_default }
};

const char *compiler_head =
"/* Automatically generated file\n"
" * This file was generated by compgen\n"
" *\n"
" * DO NOT EDIT!\n"
" *\n"
" * $Author: grubba $\n"
" */\n"
"\n";

const char *compiler_includes =
"/*\n"
" * Includes\n"
" */\n"
"\n"
"#include \"recomp.h\"\n"
"#include \"sparc.h\"\n"
"#include \"m68k.h\"\n"
"#include \"codeinfo.h\"\n"
"#include \"opcodes.h\"\n"
"#include \"compiler.h\"\n"
"\n"
"#include <stdio.h>\n"
"\n";

const char *gasp_head =
"	! Automatically generated file\n"
"	! This file was generated by compgen\n"
"	!\n"
"	! Do NOT edit!\n"
"	!\n"
"	! $Author: grubba $\n"
"	!\n"
"\n\n"
"	!\n"
"	! Register definitions\n"
"	!\n"
"regs	.reg	(i0)\n"
"mem	.reg	(i1)\n"
"vecs	.reg	(i2)\n"
"pc	.reg	(l7)\n"
"sr	.reg	(l6)\n"
"ea	.reg	(l5)\n"
"acc0	.reg	(l4)\n"
"acc1	.reg	(l3)\n"
"\n\n"
"	!\n"
"	! Offsets\n"
"	!\n"
"_D0	.reg	(0x00)\n"
"_A0	.reg	(0x20)\n"
"_A7	.reg	(0x3c)\n"
"_USP	.reg	(0x40)\n"
"_SSP	.reg	(0x44)\n"
"_SR	.reg	(0x48)\n"
"_PC	.reg	(0x4c)\n"
"_VBR	.reg	(0x50)\n"
"\n"
"	!\n"
"	! Vectors\n"
"	!\n"
"_VEC_LOAD_LONG	.reg	(0x0000)\n"
"_VEC_LOAD_WORD	.reg	(0x0004)\n"
"_VEC_LOAD_BYTE	.reg	(0x0008)\n"
"_VEC_STORE_LONG	.reg	(0x000c)\n"
"_VEC_STORE_WORD	.reg	(0x0010)\n"
"_VEC_STORE_BYTE	.reg	(0x0014)\n"
"_VEC_RESET	.reg	(0x0018)\n"
"_VEC_STOP	.reg	(0x001c)\n"
"\n\n";


/*
 * The main compiler generator loop
 */

void make_compiler(FILE *fp, U32 start, U32 end)
{
  U32 opcode = 0;

  fprintf(fp, "%s%s", compiler_head, compiler_includes);
#ifdef DEBUG
  fprintf(fp,
	  "#ifndef DEBUG\n"
	  "#define DEBUG\n"
	  "#endif\n\n");
#endif /* DEBUG */
#ifndef NDEBUG
  fprintf(fp, "/*\n * Debug functions\n */\n\n");
  fprintf(fp, "#ifdef DEBUG\n");
  fprintf(fp, "#define KPRINT(x) fprintf(stderr, x)\n");
  fprintf(fp, "#else\n");
  fprintf(fp, "#define KPRINT(x)\n");
  fprintf(fp, "#endif\n\n");
#endif /* NDEBUG */

  fprintf(fp, "/*\n * Compilerfunctions range 0x%04x to 0x%04x\n */\n\n",
	  (unsigned int)start, ((unsigned int)end-1));
  
  for (opcode=start; opcode < end; opcode++) {
    int i;

    fprintf(fp, "int comp_%04x(U32 *pc, U32 **code, U16 *mem)\n",
	    (unsigned int)opcode);
    for (i=0; (opcode & opcodes[i].mask) != opcodes[i].tag; i++)
      ;
    fprintf(fp, "{ /* %s */\n", opcodes[i].mnemonic);
#ifndef NDEBUG
    fprintf(fp, "  KPRINT(\"%s\\n\");\n", opcodes[i].mnemonic);
#endif
    if (opcodes[i].compiler(fp, opcode, opcodes[i].mnemonic)) {
      fprintf(fp, "  return(1);\n");
    } else {
      fprintf(fp, "  return(0);\n");
    }
    fprintf(fp, "} /* %s */\n\n", opcodes[i].mnemonic);
  }

#if 0
  fprintf(fp, "\n/*\n * Compilerfunction lookup table\n */\n\n");
  fprintf(fp, "static int (*comp_tab)(U32 *, U32 **, U16 *)[] = {\n");
  for (opcode=0; opcode < 0x00010000; opcode++) {
    fprintf(fp, "  comp_%04x,\n", (unsigned int)opcode);
  }
  fprintf(fp, "};\n");
#endif /* 0 */
}

void make_opcodes(FILE *fp, U32 start, U32 end)
{
  U32 opcode;

  fprintf(fp, gasp_head);

  for (opcode = start; opcode < end; opcode++) {
    int i;

    for (i=0; (opcode & opcodes[i].mask) != opcodes[i].tag; i++)
      ;
    if (opcodes[i].as_header(opcode)) {
      fprintf(fp,
	      "	! %s\n"
	      "	.globl	opcode_%04x\n"
	      "opcode_%04x:\n",
	      opcodes[i].mnemonic, opcode, opcode);
      opcodes[i].assembler(fp, opcode, opcodes[i].mnemonic);
      fprintf(fp,
	      "	.long	0\n\n");
    }
  }
  fprintf(fp, "\t.end\n");
}

void make_memory(FILE *fp)
{
  U32 opcode;

  fprintf(fp, gasp_head);

  for (opcode = 0x00; opcode < 0xc0; opcode++) {
    /* Load effective address */
    if (opcode & 0x30) {
      fprintf(fp, "	.globl	s_load_ea_%02x\n"
	          "s_load_ea_%02x:\n",
	      opcode, opcode);
      switch(opcode & 0x38) {
      case 0x10:	/* (An) */
	fprintf(fp, "	ld	[ %%regs + 0x%02x ], %%ea\n",
		(M_A0 + (opcode & 0x07))<<2);
	break;
      case 0x18:	/* (An)+ */
	fprintf(fp, "	ld	[ %%regs + 0x%02x ], %%ea\n",
		(M_A0 + (opcode & 0x07))<<2);
	if ((opcode & 0xc7)==0x07) {
	  fprintf(fp, "	add	0x02, %%ea, %%o0\n");
	} else {
	  fprintf(fp, "	add	0x%02x, %%ea, %%o0\n",
		  1<<((opcode & 0xc0)>>6));
	}
	fprintf(fp, "	st	%%o0, [ %%regs + 0x%02x ]\n",
		(M_A0 + (opcode & 0x07))<<2);
	break;
      case 0x20:	/* -(An) */
	fprintf(fp, "	ld	[ %%regs + 0x%02x ], %%ea\n",
		(M_A0 + (opcode & 0x07))<<2);
	if ((opcode & 0xc7)==0x07) {
	  fprintf(fp, "	add	-0x02, %%ea, %%ea\n");
	} else {
	  fprintf(fp, "	add	-0x%02x, %%ea, %%ea\n",
		  1<<((opcode & 0xc0)>>6));
	}
	fprintf(fp, "	st	%%ea, [ %%regs + 0x%02x ]\n",
		(M_A0 + (opcode & 0x07))<<2);
	break;
      case 0x28:	/* (d16, An) */
      case 0x30:	/* (d8, An, Xn) */
	fprintf(fp,
		"	ld	[ %%regs + 0x%02x ], %%o0\n"
		"	add	%%o0, %%ea, %%ea\n",
		(M_A0 + (opcode & 0x07))<<2);
	break;
      case 0x38:	/* Other modes */
	switch(opcode & 0x07) {
	case 0x00:	/* (d16).W */
	case 0x01:	/* (d32).L */
	  /* Do nothing */
	  break;
	case 0x02:	/* (d16, PC) */
	case 0x03:	/* (d8, PC, Xn) */
	  fprintf(fp,
		  "	add	%%pc, %%ea, %%ea\n");
	  break;
	}
	break;
      }
      fprintf(fp, "	.long	0\n\n");
    }

    /* Scale index register */
    if (!(opcode & 0x80)) {
      fprintf(fp, "	.globl	scale_reg_%02x\n"
	      "scale_reg_%02x:\n", opcode, opcode);
      
      /* Fetch the register */
      if (opcode & 0x04) {
	fprintf(fp, "	ld	[ %%regs + 0x%02x ], %%ea\n",
		(opcode & 0x78)>>1);
      } else {
	fprintf(fp, "	ldsh	[ %%regs + 0x%02x ], %%ea\n",
		((opcode & 0x78)>>1) + 2);
      }
      if (opcode & 3) {
	fprintf(fp, "	sll	%%ea, %02x, %%ea\n", (opcode & 0x03));
      }
      fprintf(fp, "	.long 0\n\n");
    }

    /* Load Effective Object to dst */
    if (!(opcode & 0x30)) {
      /* Dn, An */
      fprintf(fp, "	.globl	load_eo0_%02x\n"
	      "load_eo0_%02x:\n", opcode, opcode);
      switch (opcode & 0xc0) {
      case 0x00:	/* BYTE */
	fprintf(fp, "	ldsb	[ %%regs + 0x%02x ], %%acc0\n",
		((opcode & 0x0f)<<2)+3);
	break;
      case 0x40:	/* WORD */
	fprintf(fp, "	ldsh	[ %%regs + 0x%02x ], %%acc0\n",
		((opcode & 0x0f)<<2)+2);
	break;
      case 0x80:	/* LONG */
	fprintf(fp, "	ld	[ %%regs + 0x%02x ], %%acc0\n",
		(opcode & 0x0f)<<2);
	break;
      }
      fprintf(fp, "	.long	0\n\n");
    }

    /* Load Effective Object to src */
    if (!(opcode & 0x30)) {
      /* Dn, An */
      fprintf(fp, "	.globl	load_eo1_%02x\n"
	      "load_eo1_%02x:\n", opcode, opcode);
      switch (opcode & 0xc0) {
      case 0x00:	/* BYTE */
	fprintf(fp, "	ldsb	[ %%regs + 0x%02x ], %%acc1\n",
		((opcode & 0x0f)<<2)+3);
	break;
      case 0x40:	/* WORD */
	fprintf(fp, "	ldsh	[ %%regs + 0x%02x ], %%acc1\n",
		((opcode & 0x0f)<<2)+2);
	break;
      case 0x80:	/* LONG */
	fprintf(fp, "	ld	[ %%regs + 0x%02x ], %%acc1\n",
		(opcode & 0x0f)<<2);
	break;
      }
      fprintf(fp, "	.long	0\n\n");
    }

    /* Write back */
    if (!(opcode & 0x30)) {
      /* Dn, An */
      fprintf(fp, "	.globl	write_back_%02x\n"
	      "write_back_%02x:\n", opcode, opcode);
      switch (opcode & 0xc0) {
      case 0x00:	/* BYTE */
	fprintf(fp, "	stb	%%acc0, [ %%regs + 0x%02x ]\n",
		((opcode & 0x0f)<<2)+3);
	break;
      case 0x40:	/* WORD */
	fprintf(fp, "	sth	%%acc0, [ %%regs + 0x%02x ]\n",
		((opcode & 0x0f)<<2)+2);
	break;
      case 0x80:	/* LONG */
	fprintf(fp, "	st	%%acc0, [ %%regs + 0x%02x ]\n",
		(opcode & 0x0f)<<2);
	break;
      }
      fprintf(fp, "	.long	0\n\n");
    }
  }
  fprintf(fp, "	.end\n");
}

void make_opcode_table(FILE *fp, U32 start, U32 end)
{
  U32 opcode;

  fprintf(fp, "%s", gasp_head);

  fprintf(fp, "\n\t.text\n\n");

  if (!start) {
    fprintf(fp, "\t!\n\t! The opcode table\n\t!\n\n"
	    "	.globl	compiler_tab\n"
	    "compiler_tab:\n");
  }
  fprintf(fp, "	.globl	comp_tab_%04x\n"
	  "comp_tab_%04x:\n",
	  start, start);

  for (opcode = start; opcode < end; opcode++) {
    int i;

    for (i=0; (opcode & opcodes[i].mask) != opcodes[i].tag; i++)
      ;
    fprintf(fp, "	.long	");
    opcodes[i].tab_entry(fp, opcode, opcodes[i].mnemonic);
    fprintf(fp, ", opcode_mnemonic_%d\t! 0x%04x: %s\n", i,
	    opcode, opcodes[i].mnemonic);
  }
  fprintf(fp, "\n\n");
  fprintf(fp, "	.end\n\n");
}

void make_more_tables(FILE *fp)
{
  U32 opcode;

  fprintf(fp, "%s", gasp_head);

  fprintf(fp, "\n\t.text\n\n");

  fprintf(fp, "\t!\n\t! The Effective Address lookup table\n\t!\n\n"
	  "	.globl	ea_tab\n"
	  "ea_tab:\n");
  for (opcode = 0x00; opcode < 0x0100; opcode++) {
    if (opcode & 0x30) {
      if ((opcode & 0xc0) == 0xc0) {
	/* Kludge to help LEA FIXME: Is this still needed? */
	fprintf(fp, "	.long	s_load_ea_%02x\n", opcode & 0xbf);
      } else {
	fprintf(fp, "	.long	s_load_ea_%02x\n", opcode);
      }
    } else {
      fprintf(fp, "	.long	0\n");
    }
  }
  fprintf(fp, "\n\n");

  fprintf(fp, "\t!\n\t! The register prescale table\n\t!\n\n"
	  "	.globl	scale_reg_tab\n"
	  "scale_reg_tab:\n");
  for (opcode = 0x00; opcode < 0x80; opcode++) {
    fprintf(fp, "	.long	scale_reg_%02x\n", opcode);
  }
  fprintf(fp, "\n\n");

  fprintf(fp, "\t!\n\t! The load effective object to %%acc0 table\n\t!/\n\n"
	  "	.globl	load_eo0_tab\n"
	  "load_eo0_tab:\n");
  for (opcode = 0x0000; opcode < 0x0100; opcode++) {
    if (opcode & 0x30) {
      /* Memory access */
      switch (opcode & 0xc0) {
      case 0x00:	/* BYTE */
	fprintf(fp, "	.long	s_mem_load_byte0\n");
	break;
      case 0x40:	/* SHORT */
	fprintf(fp, "	.long	s_mem_load_short0\n");
	break;
      case 0x80:	/* LONG */
	fprintf(fp, "	.long	s_mem_load0\n");
	break;
      default:
	fprintf(fp, "	.long	0\n");
	break;
      }
    } else {
      /* Register Direct */
      if ((opcode & 0xc0) == 0xc0) {
	fprintf(fp, "	.long	0\n");
      } else {
	fprintf(fp, "	.long	load_eo0_%02x\n", opcode);
      }
    }
  }
  fprintf(fp, "\n\n");

  fprintf(fp, "\t!\n\t! The load effective object to %%acc1 table\n\t!\n\n"
	  "	.globl	load_eo1_tab\n"
	  "load_eo1_tab:\n");
  for (opcode = 0x0000; opcode < 0x0100; opcode++) {
    if (opcode & 0x30) {
      /* Memory access */
      switch (opcode & 0xc0) {
      case 0x00:	/* BYTE */
	fprintf(fp, "	.long	s_mem_load_byte1\n");
	break;
      case 0x40:	/* SHORT */
	fprintf(fp, "	.long	s_mem_load_short1\n");
	break;
      case 0x80:	/* LONG */
	fprintf(fp, "	.long	s_mem_load1\n");
	break;
      default:
	fprintf(fp, "	.long	0\n");
	break;
      }
    } else {
      /* Register Direct */
      if ((opcode & 0xc0) == 0xc0) {
	fprintf(fp, "	.long	0\n");
      } else {
	fprintf(fp, "	.long	load_eo1_%02x\n", opcode);
      }
    }
  }
  fprintf(fp, "\n\n");

  fprintf(fp, "\t!\n\t! The memory write table\n\t!\n\n"
	  "	.globl	write_back_tab\n"
	  "write_back_tab:\n");
  for (opcode = 0x0000; opcode < 0x0100; opcode++) {
    if (opcode & 0x30) {
      /* Memory access */
      switch (opcode & 0xc0) {
      case 0x00:	/* BYTE */
	fprintf(fp, "	.long	s_clobber_byte\n");
	break;
      case 0x40:	/* SHORT */
	fprintf(fp, "	.long	s_clobber_short\n");
	break;
      case 0x80:	/* LONG */
	fprintf(fp, "	.long	s_clobber\n");
	break;
      default:
	fprintf(fp, "	.long	0\n");
	break;
      }
    } else {
      /* Register Direct */
      if ((opcode & 0xc0) == 0xc0) {
	fprintf(fp, "	.long	0\n");
      } else {
	fprintf(fp, "	.long	write_back_%02x\n", opcode);
      }
    }
  }
  fprintf(fp, "\n\n");

  fprintf(fp, "\t!\n\t! The mnemonics for the opcodes\n\t!\n\n");

  for(opcode = 0; opcodes[opcode].mask; opcode++) {
    fprintf(fp,
	    "	.globl	opcode_mnemonic_%d\n"
	    "opcode_mnemonic_%d:\n"
	    "	.asciz	\"%s\"\n",
	    opcode, opcode, opcodes[opcode].mnemonic);
  }
  fprintf(fp,
	  "	.globl	opcode_mnemonic_%d\n"
	  "opcode_mnemonic_%d:\n"
	  "	.asciz	\"%s\"\n",
	  opcode, opcode, opcodes[opcode].mnemonic);

  fprintf(fp, "\n\n");

  fprintf(fp, "\t.align	4\n\n");

  fprintf(fp, "\t.end\n");
}

void make_opcode_headers(FILE *fp)
{
  U32 opcode;

  fprintf(fp, compiler_head);

  fprintf(fp, "#ifndef OPCODES_H\n"
	  "#define OPCODES_H\n\n");
  fprintf(fp, "/*\n * Declarations\n */\n\n");

  fprintf(fp, "/* Opcodes */\n\n");
  for (opcode = 0x0000; opcode < 0x00010000; opcode++) {
    int i;

    for (i=0; (opcode & opcodes[i].mask) != opcodes[i].tag; i++)
      ;
    if (opcodes[i].as_header(opcode)) {
      fprintf(fp, "extern U32 opcode_%04x[];\t/* %s */\n",
	      (unsigned int)opcode, opcodes[i].mnemonic);
    }
  }
  fprintf(fp, "\n/* Scale index register */\n\n");
  for(opcode = 0x00; opcode < 0x80; opcode++) {
    fprintf(fp, "extern U32 scale_reg_%02x[];\n", opcode);
  }

  fprintf(fp, "\n/* Load effective address */\n\n");
  for (opcode = 0x00; opcode < 0xc0; opcode++) {
    if (opcode & 0x30) {
      fprintf(fp, "extern U32 s_load_ea_%02x[];\n", opcode);
    }
  }

  fprintf(fp, "\n/* Load effective object */\n\n");
  for (opcode = 0x00; opcode < 0xc0; opcode++) {
    if (!(opcode & 0x30)) {
      fprintf(fp, "extern U32 load_eo0_%02x[];\n", opcode);
      fprintf(fp, "extern U32 load_eo1_%02x[];\n", opcode);
    }
  }

  fprintf(fp, "\n/* Register write */\n\n");
  for (opcode = 0x00; opcode < 0xc0; opcode++) {
    if (!(opcode & 0x30)) {
      fprintf(fp, "extern U32 write_back_%02x[];\n", opcode);
    }
  }
  fprintf(fp, "\n\n#endif /* OPCODES_H */\n");
}

int main(int argc, char **argv)
{
  FILE *fp;
  U32 start;

  if (argc == 2) {
    if (!strcmp(argv[1], "--gasp")) {

      /* FIXME: Create the opcodes directory */

      for (start = 0x0000; start < 0x00010000; start += 0x1000) {
	char fname[32];

	sprintf(fname, "opcodes/opcode_%04x.gasp", (unsigned int)start);
	printf(".");
	fflush(stdout);
	if ((fp = fopen(fname, "wb"))) {
	  make_opcodes(fp, start, start + 0x1000);
	  fclose(fp);
	} else {
	  fprintf(stderr, "Couldn't open file \"%s\"\n", fname);
	  exit(1);
	}
      }
      printf(" Ok\n");
      exit(0);
    } else if (!strcmp(argv[1], "--header")) {
      if ((fp = fopen("opcodes.h", "wb"))) {
	make_opcode_headers(fp);
	fclose(fp);
      } else {
	fprintf(stderr, "Couldn't open file \"opcodes.h\"\n");
	exit(1);
      }
      exit(0);
    } else if (!strcmp(argv[1], "--memory")) {
      if ((fp = fopen("memory.gasp", "wb"))) {
	make_memory(fp);
	fclose(fp);
      } else {
	fprintf(stderr,"Couldn't open file \"memory.c\"\n");
	exit(1);
      }
      exit(0);
    } else if (!strcmp(argv[1], "--tables")) {
      
      for (start = 0x0000; start < 0x00010000; start += 0x1000) {
	char fname[32];

	sprintf(fname, "tables/opcode_tab_%04x.gasp", (unsigned int)start);
	printf(".");
	fflush(stdout);
	if ((fp = fopen(fname, "wb"))) {
	  make_opcode_table(fp, start, start + 0x1000);
	  fclose(fp);
	} else {
	  fprintf(stderr, "Couldn't open file \"%s\"\n", fname);
	  exit(1);
	}
      }
      printf(".");
      fflush(stdout);
      if ((fp = fopen("tables/more_tables.gasp", "wb"))) {
	make_more_tables(fp);
	fclose(fp);
      } else {
	fprintf(stderr, "Couldn't open file \"tables/more_tables.gasp\"\n");
	exit(1);
      }
      printf(" Ok\n");
      exit(0);
    }
  }

  fprintf(stderr, "Usage:\n\t%s [option]\n\nWhere option is one of:\n", argv[0]);
  fprintf(stderr, "\t--gasp\t\tGenerate opcode-implementation files\n");
  fprintf(stderr, "\t--header\tGenerate headerfile\n");
  fprintf(stderr, "\t--memory\tGenerate memory access functions\n");
  fprintf(stderr, "\t--table\t\tGenerate opcode lookup-table\n\n");
  
  exit(1);
}

