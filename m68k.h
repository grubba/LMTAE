/*
 * $Id: m68k.h,v 1.1.1.1 1996/06/30 23:51:50 grubba Exp $
 *
 * $Log: m68k.h,v $
 * Revision 1.4  1996/06/19 11:08:25  grubba
 * *** empty log message ***
 *
 * Revision 1.3  1996/06/01 09:31:29  grubba
 *  Major changes.
 * Now generates output split into several files.
 * Now generates gasp assembler preprocessor output.
 * Now uses templates to build instructions.
 *
 * Revision 1.2  1996/05/12 16:45:12  grubba
 * Added interrupt vectors.
 * Added SR-bits.
 *
 * Revision 1.1  1996/05/06 14:50:54  grubba
 * Initial revision
 *
 */

#ifndef M68K_H
#define M68K_H

struct m_registers {
  ULONG d0,d1,d2,d3,d4,d5,d6,d7,a0,a1,a2,a3,a4,a5,a6,a7;
  ULONG usp,ssp,sr,pc;
  ULONG vbr;
  ULONG low, high;
};

#define M_A0	8
#define M_A7	15
#define M_USP	16
#define M_SSP	17
#define M_SR	18
#define M_PC	19
#define M_VBR	20

/* Status register bits */

#define M_SR_C	0x0001
#define M_SR_V	0x0002
#define M_SR_Z	0x0004
#define M_SR_N	0x0008
#define M_SR_X	0x0010
#define M_SR_I0	0x0100
#define M_SR_I1	0x0200
#define M_SR_I2	0x0400
#define M_SR_I	0x0700
#define M_SR_S	0x2000
#define M_SR_T	0x8000

/* Vectors */
#define VEC_RESET	1
#define VEC_BUS_ERROR	2
#define VEC_ADDR_ERROR	3
#define VEC_ILL_INSTR	4
#define VEC_DIV_ZERO	5
#define VEC_CHK		6
#define VEC_TRAPV	7
#define VEC_PRIV_VIOL	8
#define VEC_TRACE	9
#define VEC_LINE_A	10
#define VEC_LINE_F	11

#define VEC_COPRO_VIOL	13
#define VEC_FORMAT_ERR	14
#define VEC_UIV		15

#define VEC_SPURIOUS	24	/* IRQ 0 */

#define VEC_TRAP0	32

#define VEC_FPU		48

#endif /* M68K_H */
