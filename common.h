/*
 * $Id: common.h,v 1.3 1999/04/12 22:11:14 marcus Exp $
 *
 * $Log: common.h,v $
 * Revision 1.2  1996/07/17 16:01:11  grubba
 * Changed from {U,}{LONG,WORD,BYTE} to [SU]{8,16,32}.
 * Hopefully all places got patched.
 *
 *
 */

#ifndef COMMON_H
#define COMMON_H

#ifndef TYPES_H
#include "types.h"
#endif /* TYPES_H */

#ifndef RECOMP_H
#include "recomp.h"
#endif /* RECOMP_H */

#define chipmem ((volatile u8 *)(memory+0))
#define custom ((volatile u16 *)(memory+0xdff000))

extern int devzero;

#define REGACCESS(x) (custom[(x)>>1])
#define DREGACCESS(x) (*(u32*)&(REGACCESS(x)))

#define DMACONR REGACCESS(0x002)
#define BLTCON0 REGACCESS(0x040)
#define BLTCON1 REGACCESS(0x042)
#define BLTAFWM REGACCESS(0x044)
#define BLTALWM REGACCESS(0x046)
#define BLTCPT  DREGACCESS(0x048)
#define BLTBPT  DREGACCESS(0x04c)
#define BLTAPT  DREGACCESS(0x050)
#define BLTDPT  DREGACCESS(0x054)
#define BLTSIZE REGACCESS(0x058)
#define BLTCMOD REGACCESS(0x060)
#define BLTBMOD REGACCESS(0x062)
#define BLTAMOD REGACCESS(0x064)
#define BLTDMOD REGACCESS(0x066)
#define BLTCDAT REGACCESS(0x070)
#define BLTBDAT REGACCESS(0x072)
#define BLTADAT REGACCESS(0x074)

#endif /* COMMON_H */
