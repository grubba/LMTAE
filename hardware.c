/*
 * $Id: hardware.c,v 1.10 1996/07/19 16:46:15 grubba Exp $
 *
 * Hardware emulation for the M68000 to Sparc recompiler.
 *
 * $Log: hardware.c,v $
 * Revision 1.9  1996/07/17 16:01:31  grubba
 * Changed from {U,}{LONG,WORD,BYTE} to [SU]{8,16,32}.
 * Hopefully all places got patched.
 *
 * Revision 1.8  1996/07/17 00:13:05  grubba
 * Fixed a sorting bug in add_hw() -- it didn't sort the first element.
 * Added name arg to add_hw().
 *
 * Revision 1.7  1996/07/14 21:44:21  grubba
 * Added support for adding hardware dynamically.
 * Added CIAA time of day clock (50Hz).
 * Moved some debug output from stderr to stdout.
 *
 * Revision 1.6  1996/07/13 19:32:15  grubba
 * Now defaults to very little debuginfo.
 * Added (un|set)patch().
 * Patches added to MakeLibrary(), MakeFunctions(), Abort() and AddLibrary().
 *
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
#include "hardware.h"

/*
 * Hardware functions
 */

U32 read_bad(U32 addr, U32 base)
{
  if (addr <= 0x01000000L) {
    /* To avoid a lot of debug output during ROMTag scan. */
    fprintf(stdout, "read_bad(0x%08x, 0x%08x)\n", addr, base);
  }
  return(0);
}

void write_bad(U32 addr, U32 val, U32 base)
{
  fprintf(stdout, "write_bad(0x%08x, 0x%08x, 0x%08x)\n", addr, val, base);
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

U32 read_conf(U32 addr, U32 base)
{
  U32 addr2 = addr & 0xffff;

  if (addr2 < 0x0050) {
    fprintf(stdout, "read_conf 0x%08x (%s)\n",
	    addr,
	    auto_conf_fields[addr2>>2]);
  } else {
    fprintf(stdout, "read_conf 0x%08x (Unknown)\n", addr);
  }
  return(0);
}

#define read_conf_w	read_conf
#define read_conf_b	read_conf

void write_conf(U32 addr, U32 val, U32 base)
{
  U32 addr2 = addr & 0xffff;

  if (addr2 < 0x0050) {
    fprintf(stdout, "write_conf 0x%08x (%s), 0x%08x\n",
	    addr,
	    auto_conf_fields[addr2>>2],
	    val);
  } else {
    fprintf(stdout, "read_conf 0x%08x (Unknown), 0x%08x\n", addr, val);
  }
}

#define write_conf_w	write_conf
#define write_conf_b	write_conf

void reset_conf(U32 base)
{
  fprintf(stdout, "Autoconfig space at 0x%08x\n", base);
}

#else

#define read_conf     zorro_readlong
#define read_conf_w   zorro_readword
#define read_conf_b   zorro_readbyte
#define write_conf    zorro_writelong
#define write_conf_w  zorro_writeword
#define write_conf_b  zorro_writebyte
#define reset_conf    zorro_reset

extern U32 zorro_readlong(U32, U32);
extern U32 zorro_readword(U32, U32);
extern U32 zorro_readbyte(U32, U32);
extern void zorro_writelong(U32, U32, U32);
extern void zorro_writeword(U32, U32, U32);
extern void zorro_writebyte(U32, U32, U32);
extern void zorro_reset(U32);

#endif

void init_zorro(void)
{
  add_hw(0x00e80000, 0x00080000, "Zorro II bus",
	 zorro_readlong, zorro_readword, zorro_readbyte,
	 zorro_writelong, zorro_writeword, zorro_writebyte,
	 zorro_reset);
}

U32 read_slow_w(U32 addr, U32 base)
{
  return(*((S16 *)(memory + addr)));
}

void write_slow_w(U32 addr, U32 val, U32 base)
{
  *((U16 *)(memory + addr)) = val;
}

U32 read_slow_b(U32 addr, U32 base)
{
  return(*((S8 *)(memory + addr)));
}

void write_slow_b(U32 addr, U32 val, U32 base)
{
  *(memory + addr) = val;
}

void reset_slow(U32 base)
{
  fprintf(stdout, "Ranger (SLOW) memory at 0x%08x\n", base);
}

void init_slow(void)
{
  add_hw(0x00c00000, 0x001c0000, "Ranger (SLOW) memory",
	 NULL, read_slow_w, read_slow_b,
	 NULL, write_slow_w, write_slow_b,
	 reset_slow);
}

U32 read_rtc(U32 addr, U32 base)
{
  fprintf(stdout, "read_rtc 0x%08x\n", addr);
  return(0);
}

void write_rtc(U32 addr, U32 val, U32 base)
{
  fprintf(stdout, "write_rtc 0x%08x, 0x%08x\n", addr, val);
}

void reset_rtc(U32 base)
{
  fprintf(stdout, "Real time clock at 0x%08x\n", base);
}

void init_rtc(void)
{
  add_hw(0x00dc0000, 0x00010000, "Real time clock",
	 read_rtc, read_rtc, read_rtc,
	 write_rtc, write_rtc, write_rtc,
	 reset_rtc);
}

/*
 * Higher level functions
 */

/*
 * Structures
 */

struct hw {
  U32 start, end;
  const char *name;
  U32 (*read)(U32 addr, U32 base);
  U32 (*read_short)(U32 addr, U32 base);
  U32 (*read_byte)(U32 addr, U32 base);
  void (*write)(U32 addr, U32 val, U32 base);
  void (*write_short)(U32 addr, U32 val, U32 base);
  void (*write_byte)(U32 addr, U32 val, U32 base);
  void (*reset)(U32 base);
};

/*
 * Globals
 */

struct hw *hardware = NULL;
int num_hw_banks = 0;

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

__inline__ int find_hw(U32 maddr)
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

U32 load_hw_byte(U32 maddr)
{
  int i = find_hw(maddr);

  if (i != ~0) {
    return(hardware[i].read_byte(maddr, hardware[i].start));
  }
  return(read_bad(maddr, 0));
}

U32 load_hw_short(U32 maddr)
{
  int i = find_hw(maddr);

  if (i != ~0) {
    return(hardware[i].read_short(maddr, hardware[i].start));
  }
  return(read_bad(maddr, 0));
}

U32 load_hw(U32 maddr)
{
  int i = find_hw(maddr);

  if (i != ~0) {
    if (hardware[i].read) {
      return(hardware[i].read(maddr, hardware[i].start));
    } else {
      U32 res = hardware[i].read_short(maddr, hardware[i].start);
      res <<= 16;
      return (res | (hardware[i].read_short(maddr + 2, hardware[i].start) & 0xffff));
    }
  }
  return(read_bad(maddr, 0));
}

void store_hw_byte(U32 maddr, U8 val)
{
  int i = find_hw(maddr);

  if (i != ~0) {
    hardware[i].write_byte(maddr, val, hardware[i].start);
  } else {
    write_bad(maddr, val, 0);
  }
}

void store_hw_short(U32 maddr, U16 val)
{
  int i = find_hw(maddr);

  if (i != ~0) {
    hardware[i].write_short(maddr, val, hardware[i].start);
  } else {
    write_bad(maddr, val, 0);
  }
}

void store_hw(U32 maddr, U32 val)
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

U32 clobber_code_byte(U32 maddr, U8 val)
{
  fprintf(stdout, "clobber_code_byte(0x%08x, 0x%02x)\n", maddr, val);
  return (0);
}

U32 clobber_code_short(U32 maddr, U16 val)
{
  fprintf(stdout, "clobber_code_short(0x%08x, 0x%04x)\n", maddr, val);
  return (0);
}

void add_hw(U32 start, U32 size,
	    const char *name,
	    U32 (*read_long)(U32, U32),
	    U32 (*read_word)(U32, U32),
	    U32 (*read_byte)(U32, U32),
	    void (*write_long)(U32, U32, U32),
	    void (*write_word)(U32, U32, U32),
	    void (*write_byte)(U32, U32, U32),
	    void (*reset)(U32))
{
  int i;

  num_hw_banks++;
  
  if (!(num_hw_banks & (num_hw_banks - 1))) {
    if (hardware) {
      hardware = realloc(hardware, 2*num_hw_banks*sizeof(struct hw));
    } else {
      hardware = calloc(2*num_hw_banks, sizeof(struct hw));
    }
    if (!hardware) {
      fprintf(stderr, "add_hw: Out of memory\n");
      abort();
    }
  }
  for (i = num_hw_banks - 2; i >= 0; i--) {
    if (hardware[i].start > start) {
      hardware[i + 1] = hardware[i];
    } else {
      if (hardware[i].end > start) {
	printf("Overlapping hardware (0x%08x - 0x%08x) and (0x%08x - 0x%08x)!\n",
	       start, start + size, hardware[i].start, hardware[i].end);
	abort();
      }
      break;
    }
  }
  hardware[i + 1].start = start;
  hardware[i + 1].end = start + size;
  hardware[i + 1].name = name;
  hardware[i + 1].read = read_long;
  hardware[i + 1].read_short = read_word;
  hardware[i + 1].read_byte = read_byte;
  hardware[i + 1].write = write_long;
  hardware[i + 1].write_short = write_word;
  hardware[i + 1].write_byte = write_byte;
  hardware[i + 1].reset = reset;
}

/*
 * Hook in new default hardware here
 */

void init_slow(void);
void init_rtc(void);
void init_cia(void);
void init_custom(void);
void init_zorro(void);

void (*hw_init_tab[])(void) = {
  init_slow,
  init_rtc,
  init_cia,
  init_custom,
  init_zorro,
  NULL
};

/*
 * Master hardware initialization function
 */

void init_hardware(void)
{
  int i;

  for (i = 0; hw_init_tab[i]; i++) {
    printf("Initializing hardware: %d\r", i);
    hw_init_tab[i]();
  }
  printf("\nHardware map:\n");
  for (i = 0; i < num_hw_banks; i++) {
    printf(" 0x%08x - 0x%08x : \"%s\"\n",
	   hardware[i].start, hardware[i].end, hardware[i].name);
  }
}
