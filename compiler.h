/*
 * $Id: compiler.h,v 1.1.1.1 1996/06/30 23:51:53 grubba Exp $
 *
 * Includefile for the M68000 to Sparc recompiler.
 *
 * $Log: compiler.h,v $
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
  ULONG		flags;
  ULONG		*template;
  const char	*mnemonic;
};

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

extern ULONG *scale_reg_tab[];

extern ULONG *ea_tab[];

extern ULONG *load_eo0_tab[];
extern ULONG *load_eo1_tab[];

extern ULONG *write_back_tab[];

/* Templates */

extern ULONG s_fix_sr[];

extern ULONG s_clobber_byte[];
extern ULONG s_clobber_short[];
extern ULONG s_clobber[];

extern ULONG s_mem_load_byte0[];
extern ULONG s_mem_load_short0[];
extern ULONG s_mem_load0[];
extern ULONG s_mem_load_byte1[];
extern ULONG s_mem_load_short1[];
extern ULONG s_mem_load1[];

extern ULONG s_push_o0[];

#endif

