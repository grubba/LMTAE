/*
 * $Id: compiler.h,v 1.4 1996/08/04 14:22:50 grubba Exp $
 *
 * Includefile for the M68000 to Sparc recompiler.
 *
 * $Log: compiler.h,v $
 * Revision 1.3  1996/07/17 16:01:25  grubba
 * Changed from {U,}{LONG,WORD,BYTE} to [SU]{8,16,32}.
 * Hopefully all places got patched.
 *
 * Revision 1.2  1996/07/08 21:21:26  grubba
 * Added TEF_SRC_QUICK8 for better disassembly output.
 *
 * Revision 1.1.1.1  1996/06/30 23:51:53  grubba
 * Entry into CVS
 *
 * Revision 1.2  1996/06/19 11:08:25  grubba
 * Added support for pushing next PC.
 *
 * Revision 1.1  1996/06/01 09:31:29  grubba
 * Initial revision
 *
 *
 */

#ifndef COMPILER_H
#define COMPILER_H

/*
 * Structures
 */

struct tab_entry {
  U32		flags;
  U32		*template;
  U32		sr_magic;
  const char	*mnemonic;
};

/*
 * Tab_Entry Flags:
 *
 * 3322 2222 2222 1111 1111 1100 0000 0000
 * 1098 7654 3210 9876 5432 1098 7654 3210
 *
 * 1                                       Supervisormode only
 *      1                                  Push PC on stack
 *         1                               Fix status register after opcode
 *        1                                Write %acc0 to %ea after opcode
 *       1                                 Terminate compiler after this opcode
 *           1                 l ssmm mrrr Destination EA (loaded to %acc0)
 *            1     ls smmm rrr            Source EA (loaded to %acc1)
 *            01     i iiii iii            Source is quick 8bit (loaded to %acc1)
 *    1                                    Source is multiple registers
 *   1                                     Destination is multiple registers
 */

/* Pre instruction flags */
#define TEF_SUPERVISOR	0x80000000

/* Pre instruction flags */
#define TEF_PUSH_PC	0x08000000

/* Post instruction flags */
#define TEF_FIX_SR	0x01000000
#define TEF_WRITE_BACK	0x02000000
#define TEF_TERMINATE	0x04000000

/* Argument information */
#define TEF_DST		0x00800000
#define TEF_SRC		0x00400000

#define TEF_SRC_QUICK8	0x00200000

#define TEF_SRC_MOVEM	0x10000000
#define TEF_DST_MOVEM	0x20000000
#define TEF_MOVEM	(TEF_SRC_MOVEM | TEF_DST_MOVEM)

/* DST info */
#define TEF_DST_MASK	0x0000003f
#define TEF_DST_MODE	0x00000038
#define TEF_DST_LOAD	0x00000100
#define TEF_DST_IMM	0x0000003c
#define TEF_DST_PC	0x0000003a

/* DST Size info */
#define TEF_DST_SIZE	0x000000c0
#define TEF_DST_BYTE	0x00000000
#define TEF_DST_WORD	0x00000040
#define TEF_DST_LONG	0x00000080

/* SRC info */
#define TEF_SRC_MASK	0x00007e00
#define TEF_SRC_LOAD	0x00020000
#define TEF_SRC_IMM	0x00027800
#define TEF_SRC_PC	0x00007400
#define TEF_SRC_SHIFT	9
/* SRC Size info */
#define TEF_SRC_SIZE	0x00018000
#define TEF_SRC_BYTE	0x00000000
#define TEF_SRC_WORD	0x00008000
#define TEF_SRC_LONG	0x00010000


/*
 * Globals
 */

/* Tables */
extern struct tab_entry compiler_tab[];

extern U32 *scale_reg_tab[];

extern U32 *ea_tab[];

extern U32 *load_eo0_tab[];
extern U32 *load_eo1_tab[];

extern U32 *write_back_tab[];

/* Templates */

extern U32 s_fix_sr[];

extern U32 s_clobber_byte[];
extern U32 s_clobber_short[];
extern U32 s_clobber[];

extern U32 s_mem_load_byte0[];
extern U32 s_mem_load_short0[];
extern U32 s_mem_load0[];
extern U32 s_mem_load_byte1[];
extern U32 s_mem_load_short1[];
extern U32 s_mem_load1[];

extern U32 s_push_o0[];

#endif

