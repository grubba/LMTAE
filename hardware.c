/*
 * $Id: hardware.c,v 1.6 1996/07/13 19:32:15 grubba Exp $
 *
 * Hardware emulation for the M68000 to Sparc recompiler.
 *
 * $Log: hardware.c,v $
 * Revision 1.5  1996/07/11 23:02:02  marcus
 * Real ZorroII emulation
 *
 * Revision 1.4  1996/07/08 21:18:12  grubba
 * Added some more hardware.
 *
 * Revision 1.3  1996/07/05 02:09:22  grubba
 * Now detects the AutoConfig(TM) space.
 *
 * Revision 1.2  1996/07/03 14:24:10  grubba
 * Fixed a bug with VHPOSR -- It incremented 0xdff004<<1. Amazing it didn't core!
 *
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
    fprintf(stdout, "read_bad(0x%08lx, 0x%08lx)\n", addr, base);
  }
  return(0);
}

void write_bad(ULONG addr, ULONG val, ULONG base)
{
  fprintf(stdout, "write_bad(0x%08lx, 0x%08lx, 0x%08lx)\n", addr, val, base);
}

ULONG read_custom(ULONG addr, ULONG base)
{
  ULONG val;

  fprintf(stdout, "read_custom(0x%08lx, 0x%08lx)\n", addr, base);

  val = ((SHORT *)memory)[addr>>1];

  if (((addr - 0xdff000) >= 0x180) && ((addr - 0xdff000) < (0x180 + 0x20))) {
    fprintf(stdout, "Reading color %ld (0x%04lx)\n",
	    ((addr - 0xdff000) - 0x180), (val & 0xffff));
  } else if (addr == 0xdff006) {
    fprintf(stdout, "Reading VHPOSR 0x%08lx\n", ((ULONG *)memory)[0xdff004>>2]);
    ((ULONG *)memory)[0xdff004>>2] += 0x00000101;
    ((ULONG *)memory)[0xdff004>>2] &= 0x0001ffff;
  }
  return(val);
}

void write_custom(ULONG addr, ULONG val, ULONG base)
{
  fprintf(stdout, "write_custom(0x%08lx, 0x%08lx, 0x%08lx)\n", addr, val, base);

  if (((addr - 0xdff000) >= 0x180) && ((addr - 0xdff000) < (0x180 + 0x20))) {
    fprintf(stdout, "Setting color %ld to 0x%04lx\n",
	    ((addr - 0xdff000) - 0x180), (val & 0xffff));
  }
  ((USHORT *)memory)[addr>>1] = val;
}

void reset_custom(ULONG base)
{
  fprintf(stdout, "Custom base is 0x%08lx\n", base);
}

ULONG read_cia(ULONG addr, ULONG base)
{
  if (addr != 0x00bfe001) {
    fprintf(stdout, "read_cia(0x%08lx, 0x%08lx)\n", addr, base);
  }
  return(memory[addr]);
}

void write_cia(ULONG addr, ULONG val, ULONG base)
{
  if (addr == 0x00bfe001) {
    UBYTE bits = memory[addr] ^ ((UBYTE)val);

    if (bits & 0x02) {
      if (val & 0x02) {
	fprintf(stdout, "LED off\n");
      } else {
	fprintf(stdout, "LED on\n");
      }
    }
    if (bits & ~0x02) {
      fprintf(stdout, "write_cia(0x%08lx, 0x%08lx, 0x%08lx\n", addr, val, base);
    }
  } else {
    fprintf(stdout, "write_cia(0x%08lx, 0x%08lx, 0x%08lx)\n", addr, val, base);
  }

  memory[addr] = val;
}

void reset_cia(ULONG base)
{
  fprintf(stdout, "CIA base is 0x%08lx\n", base);
}

#ifdef notdef

static const char *auto_conf_fields[] = {
  "er_Type",
  "er_Product",
  "er_Flags",
  "er_Reserved03",
  "er_Manufacturer (HI)",
  "er_Manufacturer (LOW)",
  "er_SerialNumber (HI)",
  "er_SerialNumber (HI-LOW)",
  "er_SerialNumber (LOW-HI)",
  "er_SerialNumber (LOW)",
  "er_InitDiagVec (HI)",
  "er_InitDiagVec (LOW)",
  "er_Reserved0c",
  "er_Reserved0d",
  "er_Reserved0e",
  "er_Reserved0f",
  "ec_Interrupt",
  "ec_Z3_HighByte",
  "ec_BaseAddress",
  "ec_Shutup",
};

ULONG read_conf(ULONG addr, ULONG base)
{
  ULONG addr2 = addr & 0xffff;

  if (addr2 < 0x0050) {
    fprintf(stdout, "read_conf 0x%08lx (%s)\n",
	    addr,
	    auto_conf_fields[addr2>>2]);
  } else {
    fprintf(stdout, "read_conf 0x%08lx (Unknown)\n", addr);
  }
  return(0);
}

#define read_conf_w	read_conf
#define read_conf_b	read_conf

void write_conf(ULONG addr, ULONG val, ULONG base)
{
  ULONG addr2 = addr & 0xffff;

  if (addr2 < 0x0050) {
    fprintf(stdout, "write_conf 0x%08lx (%s), 0x%08lx\n",
	    addr,
	    auto_conf_fields[addr2>>2],
	    val);
  } else {
    fprintf(stdout, "read_conf 0x%08lx (Unknown), 0x%08lx\n", addr, val);
  }
}

#define write_conf_w	write_conf
#define write_conf_b	write_conf

void reset_conf(ULONG base)
{
  fprintf(stdout, "Autoconfig space at 0x%08lx\n", base);
}

#else

#define read_conf     zorro_readlong
#define read_conf_w   zorro_readword
#define read_conf_b   zorro_readbyte
#define write_conf    zorro_writelong
#define write_conf_w  zorro_writeword
#define write_conf_b  zorro_writebyte
#define reset_conf    zorro_reset

extern ULONG zorro_readlong(ULONG, ULONG);
extern ULONG zorro_readword(ULONG, ULONG);
extern ULONG zorro_readbyte(ULONG, ULONG);
extern void zorro_writelong(ULONG, ULONG, ULONG);
extern void zorro_writeword(ULONG, ULONG, ULONG);
extern void zorro_writebyte(ULONG, ULONG, ULONG);
extern void zorro_reset(ULONG);

#endif

ULONG read_slow_w(ULONG addr, ULONG base)
{
  return(*((SHORT *)(memory + addr)));
}

void write_slow_w(ULONG addr, ULONG val, ULONG base)
{
  *((USHORT *)(memory + addr)) = val;
}

ULONG read_slow_b(ULONG addr, ULONG base)
{
  return(*((BYTE *)(memory + addr)));
}

void write_slow_b(ULONG addr, ULONG val, ULONG base)
{
  *(memory + addr) = val;
}

void reset_slow(ULONG base)
{
  fprintf(stdout, "Ranger (SLOW) memory at 0x%08lx\n", base);
}

ULONG read_rtc(ULONG addr, ULONG base)
{
  fprintf(stdout, "read_rtc 0x%08lx\n", addr);
  return(0);
}

void write_rtc(ULONG addr, ULONG val, ULONG base)
{
  fprintf(stdout, "write_rtc 0x%08lx, 0x%08lx\n", addr, val);
}

void reset_rtc(ULONG base)
{
  fprintf(stdout, "Real time clock at 0x%08lx\n", base);
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
  { 0x00bfd000, 0x00bfe000, read_bad, read_bad, read_cia, write_bad, write_bad, write_cia, reset_cia },
  { 0x00bfe000, 0x00bff000, read_bad, read_bad, read_cia, write_bad, write_bad, write_cia, reset_cia },
  { 0x00c00000, 0x00dc0000, NULL, read_slow_w, read_slow_b, NULL, write_slow_w, write_slow_b, reset_slow },
  { 0x00dc0000, 0x00dd0000, read_rtc, read_rtc, read_rtc, write_rtc, write_rtc, write_rtc, reset_rtc },
  { 0x00dff000, 0x00e00000, NULL, read_custom, read_bad, write_bad, write_custom, write_bad, reset_custom },
  { 0x00e80000, 0x00e8ffff, read_conf, read_conf_w, read_conf_b, write_conf, write_conf_w, write_conf_b, reset_conf },
};

const int num_hw_banks = 6;

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
    if (hardware[i].write) {
      hardware[i].write(maddr, val, hardware[i].start);
    } else {
      hardware[i].write_short(maddr, (val >> 16), hardware[i].start);
      hardware[i].write_short(maddr + 2, val, hardware[i].start);
    }
  } else {
    write_bad(maddr, val, 0);
  }
}

ULONG clobber_code_byte(ULONG maddr, UBYTE val)
{
  fprintf(stdout, "clobber_code_byte(0x%08lx, 0x%02x)\n", maddr, val);
  return (0);
}

ULONG clobber_code_short(ULONG maddr, USHORT val)
{
  fprintf(stdout, "clobber_code_short(0x%08lx, 0x%04x)\n", maddr, val);
  return (0);
}

