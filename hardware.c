/*
 * $Id: hardware.c,v 1.2 1996/07/03 14:24:10 grubba Exp $
 *
 * Hardware emulation for the M68000 to Sparc recompiler.
 *
 * $Log: hardware.c,v $
 * Revision 1.1.1.1  1996/06/30 23:51:53  grubba
 * Entry into CVS
 *
 * Revision 1.5  1996/06/30 23:03:59  grubba
 * *** empty log message ***
 *
 * Revision 1.4  1996/06/23 14:26:50  grubba
 * Fixed a minor bug.
 *
 * Revision 1.3  1996/06/23 14:14:04  grubba
 * Added reset_hw().
 *
 * Revision 1.2  1996/06/20 22:12:08  grubba
 * Some minor bugs fixed.
 *
 * Revision 1.1  1996/06/19 11:47:37  grubba
 * Initial revision
 *
 *
 */

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>

#include "recomp.h"

/*
 * Hardware functions
 */

ULONG read_bad(ULONG addr, ULONG base)
{
  if (addr <= 0x01000000L) {
    /* To avoid a lot of debug output during ROMTag scan. */
    fprintf(stderr, "read_bad(0x%08lx, 0x%08lx)\n", addr, base);
  }
  return(0);
}

void write_bad(ULONG addr, ULONG val, ULONG base)
{
  fprintf(stderr, "write_bad(0x%08lx, 0x%08lx, 0x%08lx)\n", addr, val, base);
}

ULONG read_custom(ULONG addr, ULONG base)
{
  ULONG val;

  fprintf(stderr, "read_custom(0x%08lx, 0x%08lx)\n", addr, base);

  val = ((SHORT *)memory)[addr>>1];

  if (((addr - 0xdff000) >= 0x180) && ((addr - 0xdff000) < (0x180 + 0x20))) {
    fprintf(stderr, "Reading color %ld (0x%04lx)\n",
	    ((addr - 0xdff000) - 0x180), (val & 0xffff));
  } else if (addr == 0xdff006) {
    fprintf(stderr, "Reading VHPOSR 0x%08lx\n", ((ULONG *)memory)[0xdff004>>2]);
    ((ULONG *)memory)[0xdff004>>2] += 0x00000101;
    ((ULONG *)memory)[0xdff004>>2] &= 0x0001ffff;
  }
  return(val);
}

void write_custom(ULONG addr, ULONG val, ULONG base)
{
  fprintf(stderr, "write_custom(0x%08lx, 0x%08lx, 0x%08lx)\n", addr, val, base);

  if (((addr - 0xdff000) >= 0x180) && ((addr - 0xdff000) < (0x180 + 0x20))) {
    fprintf(stderr, "Setting color %ld to 0x%04lx\n",
	    ((addr - 0xdff000) - 0x180), (val & 0xffff));
  }
  ((USHORT *)memory)[addr>>1] = val;
}

void reset_custom(ULONG base)
{
  fprintf(stderr, "Custom base is 0x%08lx\n", base);
}

ULONG read_cia(ULONG addr, ULONG base)
{
  if (addr != 0x00bfe001) {
    fprintf(stderr, "read_cia(0x%08lx, 0x%08lx)\n", addr, base);
  }
  return(memory[addr]);
}

void write_cia(ULONG addr, ULONG val, ULONG base)
{
  if (addr == 0x00bfe001) {
    UBYTE bits = memory[addr] ^ ((UBYTE)val);

    if (bits & 0x02) {
      if (val & 0x02) {
	fprintf(stderr, "LED off\n");
      } else {
	fprintf(stderr, "LED on\n");
      }
    }
    if (bits & ~0x02) {
      fprintf(stderr, "write_cia(0x%08lx, 0x%08lx, 0x%08lx\n", addr, val, base);
    }
  } else {
    fprintf(stderr, "write_cia(0x%08lx, 0x%08lx, 0x%08lx)\n", addr, val, base);
  }

  memory[addr] = val;
}

void reset_cia(ULONG base)
{
  fprintf(stderr, "CIA base is 0x%08lx\n", base);
}

/*
 * Higher level functions
 */

/*
 * Structures
 */

struct hw {
  ULONG start, end;
  ULONG (*read)(ULONG addr, ULONG base);
  ULONG (*read_short)(ULONG addr, ULONG base);
  ULONG (*read_byte)(ULONG addr, ULONG base);
  void (*write)(ULONG addr, ULONG val, ULONG base);
  void (*write_short)(ULONG addr, ULONG val, ULONG base);
  void (*write_byte)(ULONG addr, ULONG val, ULONG base);
  void (*reset)(ULONG base);
};

/*
 * Globals
 */

struct hw hardware[] = {
  { 0x00bfe000, 0x00bff000, read_bad, read_bad, read_cia, write_bad, write_bad, write_cia, reset_cia },
  { 0x00dff000, 0x00e00000, NULL, read_custom, read_bad, write_bad, write_custom, write_bad, reset_custom },
};

const int num_hw_banks = 2;

/*
 * Functions
 */

void reset_hw(void)
{
  int i;

  fprintf(stdout, "RESET!\nResetting...");
  fflush(stdout);

  for(i=num_hw_banks; i--;) {
    fprintf(stdout, "%d ", i);
    fflush(stdout);
    hardware[i].reset(hardware[i].start);
  }
  fprintf(stdout, "Go!\n");
}

inline int find_hw(ULONG maddr)
{
  int b=0;
  int t=num_hw_banks;
  int i;

  while (b < t) {
    i = (b + t)/2;

    if (maddr < hardware[i].start) {
      t = i;
    } else if (maddr >= hardware[i].end) {
      b = i+1;
    } else {
      return(i);
    }
  }
  i = (b + t)/2;
  
  if ((maddr >= hardware[i].start) &&
      (maddr < hardware[i].end)) {
    return(i);		/* FIXME: Is this needed? */
  }
  return(~0);
}

ULONG load_hw_byte(ULONG maddr)
{
  int i = find_hw(maddr);

  if (i != ~0) {
    return(hardware[i].read_byte(maddr, hardware[i].start));
  }
  return(read_bad(maddr, 0));
}

ULONG load_hw_short(ULONG maddr)
{
  int i = find_hw(maddr);

  if (i != ~0) {
    return(hardware[i].read_short(maddr, hardware[i].start));
  }
  return(read_bad(maddr, 0));
}

ULONG load_hw(ULONG maddr)
{
  int i = find_hw(maddr);

  if (i != ~0) {
    if (hardware[i].read) {
      return(hardware[i].read(maddr, hardware[i].start));
    } else {
      ULONG res = hardware[i].read_short(maddr, hardware[i].start);
      res <<= 16;
      return (res | (hardware[i].read_short(maddr + 2, hardware[i].start) & 0xffff));
    }
  }
  return(read_bad(maddr, 0));
}

void store_hw_byte(ULONG maddr, UBYTE val)
{
  int i = find_hw(maddr);

  if (i != ~0) {
    hardware[i].write_byte(maddr, val, hardware[i].start);
  } else {
    write_bad(maddr, val, 0);
  }
}

void store_hw_short(ULONG maddr, USHORT val)
{
  int i = find_hw(maddr);

  if (i != ~0) {
    hardware[i].write_short(maddr, val, hardware[i].start);
  } else {
    write_bad(maddr, val, 0);
  }
}

void store_hw(ULONG maddr, ULONG val)
{
  int i = find_hw(maddr);

  if (i != ~0) {
    hardware[i].write(maddr, val, hardware[i].start);
  } else {
    write_bad(maddr, val, 0);
  }
}

ULONG clobber_code_byte(ULONG maddr, UBYTE val)
{
  fprintf(stderr, "clobber_code_byte(0x%08lx, 0x%02x)\n", maddr, val);
  return (0);
}

ULONG clobber_code_short(ULONG maddr, USHORT val)
{
  fprintf(stderr, "clobber_code_short(0x%08lx, 0x%04x)\n", maddr, val);
  return (0);
}

