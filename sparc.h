/*
 * $Id: sparc.h,v 1.2 1996/07/17 16:02:02 grubba Exp $
 *
 * $Log: sparc.h,v $
 * Revision 1.1.1.1  1996/06/30 23:51:50  grubba
 * Entry into CVS
 *
 * Revision 1.7  1996/06/19 11:08:25  grubba
 * *** empty log message ***
 *
 * Revision 1.6  1996/06/01 09:31:29  grubba
 *  Major changes.
 * Now generates output split into several files.
 * Now generates gasp assembler preprocessor output.
 * Now uses templates to build instructions.
 *
 * Revision 1.5  1996/05/14 21:32:59  grubba
 * Added some instructions.
 * The old S_EMIT renamed to S_EMIT6.
 * S_EMIT with one argument added.
 * Some bug fixes.
 * Register S_O1 was named S_01.
 *
 * Revision 1.4  1996/05/13 17:33:33  grubba
 * Now with store of bytes and halfwords.
 *
 * Revision 1.3  1996/05/13 10:41:14  grubba
 * code is now ULONG ** (was ULONG *).
 *
 * Revision 1.2  1996/05/12 16:36:33  grubba
 * Added some more opcodes.
 *
 * Revision 1.1  1996/05/06 11:19:57  grubba
 * Initial revision
 *
 */

#ifndef SPARC_H
#define SPARC_H

#ifndef TYPES_H
#include "types.h"
#endif /* TYPES_H */

/* Register definitions */

#define S_G0	0x00
#define S_G1	0x01
#define S_G2	0x02
#define S_G3	0x03
#define S_G4	0x04
#define S_G5	0x05
#define S_G6	0x06
#define S_G7	0x07

#define S_O0	0x08
#define S_O1	0x09
#define S_O2	0x0a
#define S_O3	0x0b
#define S_O4	0x0c
#define S_O5	0x0d
#define S_O6	0x0e
#define S_O7	0x0f

#define S_SP	S_O6

#define S_L0	0x10
#define S_L1	0x11
#define S_L2	0x12
#define S_L3	0x13
#define S_L4	0x14
#define S_L5	0x15
#define S_L6	0x16
#define S_L7	0x17

#define S_I0	0x18
#define S_I1	0x19
#define S_I2	0x1a
#define S_I3	0x1b
#define S_I4	0x1c
#define S_I5	0x1d
#define S_I6	0x1e
#define S_I7	0x1f

/* The Processor Status Register */

#define PSR_CWP_MASK	0x0000001f	/* Current Window Pointer */
#define PSR_ET		0x00000020	/* Enable Traps */
#define PSR_PS		0x00000040	/* Previous Supervisor mode */
#define PSR_S		0x00000080	/* Supervisor mode */
#define PSR_PIL_MASK	0x00000700	/* Interrupt mask */
#define PSR_EF		0x00000800	/* Enable Floating point */
#define PSR_EC		0x00001000	/* Enable Coprocessor */
#define PSR_RESVD_MASK	0x000fe000	/* Reserved for future */
#define PSR_C		0x00100000	/* Carry */
#define PSR_V		0x00200000	/* Overflow */
#define PSR_Z		0x00400000	/* Zero */
#define PSR_N		0x00800000	/* Negative */
#define PSR_CC_MASK	0x00f00000	/* Condition Codes */
#define PSR_VER_MASK	0x0f000000	/* Chip Version */
#define PSR_IMPL_MASK	0xf0000000	/* Implementation */

#define IMPL_FUJITSU	0x00000000
#define IMPL_ROSS	0x10000000
#define IMPL_BIT	0x20000000
#define IMPL_LSI	0x30000000
#define IMPL_TI		0x40000000
#define IMPL_MEI	0x50000000	/* MEI/Solbourne */
#define IMPL_PHILIPS	0x60000000
#define IMPL_HARVEST	0x70000000	/* Harvest VLSI */
#define IMPL_SPEC	0x80000000
#define IMPL_MATRA	0x90000000

/* Opcodes */

/* op dst   op  bits  */
/* 00 11000 100 0000000000000000000000  sethi 0, i0 */
/* 00 11111 100 1111111111111111111111  sethi -1, i7 */

/* 00 00000 100 0000000000000000000000  nop (sethi 0, %g0) */

/* 00 01011010 0000000000000000000100  bge *+0x10 */
/* 00 01000010 0000000000000000000100  b *+0x10 */
/* 00 11000010 0000000000000000000111  b,a *+0x1c */
/* 00 10001010 0000000000000000000010  be,a *+0x08 */
/* 00 00001010 0000000000000000000011  be *+0x0c */
/* 00 01101010 0000000000000000000101  bcc *+0x14 */
/* 00 00101010 0000000000000000000101  bcs *+0x14 */


/* op dst   op     src   i s bits */
/* 10 01001 000001 01001 1 1 111111110011 and -13, %o1, %o1 */

/* 10 11010 000010 11010 1 1 111111111111 or -1, %r26, %r26 */
/* 10 11111 000010 00001 1 1 111111111111 or -1, %r1, %r31 */
/* 10 00001 000010 11111 1 0 000000000001 or 1, %r31, %r1 */

/* 10 00001 000010 00001 1 1 111111111101 mov -3, %g1 */
/* 10 11111 000010 00000 1 0 000000000010 mov 2, %i7 */
/* 10 01000 000010 00000 0 0 000000001001 mov %o1, %o0 */

/* 10 11111 000000 00001 1 1 111111111111 add -1, %g0, %i7 */
/* 10 00001 000000 11111 1 0 000000000000 add 0, %i7, %g1 */
/* 10 11111 000000 11111 0 0 0000000 00000 add %i7, %g0, %i7 */
/* 10 00001 000000 00000 0 0 0000000 11111 add %g0, %i7, %g1 */

/* 10 00000 010001 01000 0 0 000000000010 btst %o0, %g2 */

/* 10 00000 010100 01000 1 0 000000000000 cmp %o0, 0 */
/* 10 00000 010100 01000 0 0 000000001001 cmp %o0, %o1 */

/* 10 01000 100101 01000 1 0 000000011000 sll %o0, 0x18, %o0 */
/* 10 01000 100111 01000 1 0 000000011000 sra %o0, 0x18, %o0 */

/* 10 00000 000011 00000 0 0 000000000000 xor %g0, %g0, %g0 */

/* 11 11111 000100 00001 1 1 111111111111 st %i7, [%g1 -1] */
/* 11 00000 000100 11111 1 1 111111111111 st %g0, [%i7 -1] (clr) */
/* 11 11111 000100 11111 1 0 000000000000 st %i7, [%i7] */

/* 11 11111 000000 01110 1 1 111111111111 ld [ %sp - 1 ], %i7 */
/* 11 00001 000000 11111 1 0 000000000000 ld [ %i7 ], %g1 */
/* 11 00001 000000 00000 1 1 111111111111 ld [ -1 ], %g1 */
/* 11 00010 000001 01000 0 0 000000000000 ldub [ %o0 ], %g2 */

/* 11 00000 000101 01000 0 0 000000000000 clrb [ %o0 ] */
/* 11 00000 000110 01001 0 0 000000000000 clrh [ %o1 ] */
/* 11 00000 000100 01010 0 0 000000000000 clr [ %o2 ] */

/* Missing: */
/* ldhi   bne,a   bne   xor   ld   addcc */

#define S_EMIT(value)	do { **code = (value); (*code)++; } while (0)
#define S_EMIT6(base, dst, op, src, i, imm13) S_EMIT(((base) << 30) | ((dst) << 25) | ((op) << 19) | ((src) << 14) | ((i) << 13) | (imm13))
#define S_EMIT3(base, op, imm22) S_EMIT(((base) << 30) | ((op) << 22) | (imm22))
#define S_EMIT_BRANCH(op, addr)	S_EMIT3(0, op, ((((U32)(addr))-((U32)(*code))))>>2)

#define S_ADD(srca, srcb, dst)	S_EMIT6(2, dst, 0, srca, 0, srcb)
#define S_ADDI(val, src, dst)	S_EMIT6(2, dst, 0, src, 1, ((val) & 0x1fff))
#define S_AND(srca, srcb, dst)	S_EMIT6(2, dst, 1, srca, 0, srcb)
#define S_ANDI(val, src, dst)	S_EMIT6(2, dst, 1, src, 1, ((val) & 0x1fff))
#define S_B(addr)		S_EMIT_BRANCH(0x42, addr)
#define S_B_A(addr)		S_EMIT_BRANCH(0xc2, addr)
#define S_BCC(addr)		S_EMIT_BRANCH(0x6a, addr)
#define S_BCS(addr)		S_EMIT_BRANCH(0x2a, addr)
#define S_BE(addr)		S_EMIT_BRANCH(0x0a, addr)
#define S_BE_A(addr)		S_EMIT_BRANCH(0x8a, addr)
/* #define S_BNE_A(addr)	S_EMIT_BRANCH(0xca, addr) */
#define S_BGE(addr)		S_EMIT_BRANCH(0x5a, addr)
#define S_BTST(rega, regb)	S_EMIT6(2, S_G0, 0x11, rega, 0, regb)
#define S_CLR(rega, regb)	S_ST(S_G0, rega, regb)
#define S_CLRB(rega, regb)	S_STB(S_G0, rega, regb)
#define S_CLRH(rega, regb)	S_STH(S_G0, rega, regb)
#define S_CLRI(reg, off)	S_STI(S_G0, reg, off)
#define S_CLRBI(reg, off)	S_STBI(S_G0, reg, off)
#define S_CLRHI(reg, off)	S_STHI(S_G0, reg, off)
#define S_CMP(rega, regb)	S_EMIT6(2, S_G0, 0x14, rega, 0, regb)
#define S_CMPI(reg, val)	S_EMIT6(2, S_G0, 0x14, reg, 1, ((val) & 0x1fff))
#define S_LDI(src, off, dst)	S_EMIT6(3, dst, 0, src, 1, ((off) & 0x1fff))
#define S_LDUB(srca, srcb, dst)	S_EMIT6(3, dst, 1, srca, 0, srcb)
#define S_MOV(src, dst)		S_OR(src, S_G0, dst)
#define S_MOVI(val, dst)	S_ORI(val, S_G0, dst)
#define S_NOP			S_EMIT(0x01000000)
#define S_OR(srca, srcb, dst)	S_EMIT6(2, dst, 2, srca, 0, srcb)
#define S_ORI(val, src, dst)	S_EMIT6(2, dst, 2, src, 1, ((val) & 0x1fff))
#define S_SETHI(val, reg)	S_EMIT(0x01000000 | ((reg) << 25) | (((U32)(val))>>10))
#define S_SLLI(src, val, dst)	S_EMIT6(2, dst, 0x25, src, 1, ((val) & 0x1fff))
#define S_SRAI(src, val, dst)	S_EMIT6(2, dst, 0x27, src, 1, ((val) & 0x1fff))
#define S_ST(src, dsta, dstb)	S_EMIT6(3, dsta, 4, src, 0, dstb)
#define S_STB(src, dsta, dstb)	S_EMIT6(3, dsta, 5, src, 0, dstb)
#define S_STH(src, dsta, dstb)	S_EMIT6(3, dsta, 6, src, 0, dstb)
#define S_STI(src, dst, off)	S_EMIT6(3, dst, 4, src, 1, ((off) & 0x1fff))
#define S_STBI(src, dst, off)	S_EMIT6(3, dst, 5, src, 1, ((off) & 0x1fff))
#define S_STHI(src, dst, off)	S_EMIT6(3, dst, 6, src, 1, ((off) & 0x1fff))
#define S_XOR(srca, srcb, dst)	S_EMIT6(2, dst, 3, srca, 0, srcb)
#define S_XORI(val, src, dst)	S_EMIT6(2, dst, 3, src, 1, ((val) & 0x1fff))

#endif /* SPARC_H */
