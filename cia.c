/*
 * $Id: cia.c,v 1.3 1996/07/17 00:18:13 grubba Exp $
 *
 * CIA emulation.
 *
 * $Log: cia.c,v $
 * Revision 1.2  1996/07/15 05:20:57  grubba
 * Added timer A and B.
 * Real-time timers now go at 1/10th of real-speed.
 *
 * Revision 1.1  1996/07/15 02:50:53  grubba
 * Cleaned up CIA emulation, and put it in a separate file.
 *
 *
 */

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#include "recomp.h"
#include "hardware.h"

/*
 * Globals
 */

static BYTE (*cia_read_tab[32])(BYTE);
static void (*cia_write_tab[32])(BYTE, BYTE);

/*
 * Static Globals
 */

static signed char cia_registers[] = {
  /* CIA A */
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  /* CIA B */
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
};

/*
 * Functions
 */

/*
 * Generic functions
 */

BYTE cia_read_bad(BYTE regnum)
{
  printf("cia_read_bad register #0x%02x\n", regnum);
  return (cia_registers[regnum]);
}

void cia_write_bad(BYTE regnum, BYTE val)
{
  printf("cia_write_bad register #0x%02x, value 0x%02x\n", regnum, val);
}

BYTE cia_read_reg(BYTE regnum)
{
  return(cia_registers[regnum]);
}

void cia_write_reg(BYTE regnum, BYTE val)
{
  cia_registers[regnum] = val;
}

/*
 * Implementation
 */

/*
 * NOTE: Real time timers go a factor 10 slower than the real thing!
 */

/*
 * CIA A
 */

/*
 * CIA A: Peripheral ports
 */

void ciaa_write_pra(BYTE regnum, BYTE val)
{
  BYTE bits = cia_registers[regnum] ^ val;

  if (bits & 1) {
    if (val & 1) {
      printf("Overlay mode\n");
    } else {
      printf("Overlay mode off\n");
    }
  }
  if (bits & 2) {
    if (val & 2) {
      printf("LED off\n");
    } else {
      printf("LED on\n");
    }
  }
  if (bits & ~3) {
    printf("CIA A PRA = 0x%02x\n", ((unsigned) val) & 0xff);
  }
  cia_registers[regnum] = val & 0x7f;
}

void ciaa_write_ddra(BYTE regnum, BYTE val)
{
  BYTE bits = cia_registers[regnum] ^ val;

  cia_registers[regnum] = val;

  if (bits) {
    printf("CIA A DDRA set to 0x%02x\n", ((unsigned)val) & 0xff);
  }
}

/*
 * CIA A: Timers
 */

static int ciaa_timera_latch = 0;
static int ciaa_timera_offset = 0;
hrtime_t ciaa_timera_base = 0;

BYTE ciaa_read_timera(BYTE reg)
{
  if ((cia_registers[0x0e] & 0x20) || (!(cia_registers[0x0e] & 0x01))) {
    /* CNT line */
    printf("Other timermodes not supported!\n");

    return (((BYTE *)&ciaa_timera_offset)[7 - reg]);
  } else {
    /* 10s of CPU cycles */
    hrtime_t current = gethrtime() - ciaa_timera_base;
    int value = ciaa_timera_offset - ((current * 709379) / 10000000000LL);
    if (value < 0) {
      value = 0;
    }

    printf("CIA A: Timer A is 0x%04x\n", value);

    return (((BYTE *)&value)[7-reg]);
  }
}

void ciaa_write_timera(BYTE reg, BYTE val)
{
  ((BYTE *)&ciaa_timera_latch)[7 - reg] = val;
  ((BYTE *)&ciaa_timera_offset)[7 - reg] = val;
  ciaa_timera_base = gethrtime();
}

/*
 * CIA A: Timer B
 */

static int ciaa_timerb_latch = 0;
static int ciaa_timerb_offset = 0;
hrtime_t ciaa_timerb_base = 0;

BYTE ciaa_read_timerb(BYTE reg)
{
  if ((cia_registers[0x0f] & 0x30) || (!cia_registers[0x0f] & 0x01)) {
    /* CNT or Timer A or Timer A and CNT */
    printf("Other timermodes not supported!\n");

    return (((BYTE *)&ciaa_timerb_offset)[9 - reg]);
  } else {
    /* 10s of CPU cycles */
    hrtime_t current = gethrtime() - ciaa_timerb_base;
    int value = ciaa_timerb_offset - ((current * 709379) / 10000000000LL);
    if (value < 0) {
      value = 0;
    }
    
    printf("CIA A: Timer B is 0x%04x\n", value);

    return (((BYTE *)&value)[9-reg]);
  }
}

void ciaa_write_timerb(BYTE reg, BYTE val)
{
  ((BYTE *)&ciaa_timerb_latch)[9 - reg] = val;
  ((BYTE *)&ciaa_timerb_offset)[9 - reg] = val;
  ciaa_timerb_base = gethrtime();
}

/*
 * CIA A: Time of day clock
 */

static int ciaa_tod_state = 0;
static ULONG ciaa_tod_value = 0;
static hrtime_t ciaa_tod_base = 0;
static ULONG ciaa_tod_offset = 0;

BYTE ciaa_read_tod(BYTE reg)
{
  if (!ciaa_tod_state) {
    hrtime_t current = gethrtime() - ciaa_tod_base;

    ciaa_tod_value = (current / 50000000) + ciaa_tod_offset;
  }
  if (reg == 0x0a) {
    ciaa_tod_state |= 1;
  } else if (reg == 0x08) {
    ciaa_tod_state &= ~1;
  }
  return(((BYTE *)&ciaa_tod_value)[0x0b - reg]);
}

void ciaa_write_tod(BYTE reg, BYTE val)
{
  if (reg == 0x0a) {
    ciaa_tod_state |= 2;
  } else if (reg == 0x08) {
    ciaa_tod_state &= ~2;
  }

  ((BYTE *)&ciaa_tod_offset)[0x0b - reg] = val;

  printf("CIAA TOD = 0x%06lx\n", ciaa_tod_offset);

  ciaa_tod_base = gethrtime();
}

void ciaa_write_alarm(BYTE reg, BYTE val)
{
  printf("CIA A: Setting alarm (Not supported yet!)\n");
  cia_registers[reg] = val;
}

/*
 * CIA A: Control registers
 */

void ciaa_write_cra(BYTE regnum, BYTE val)
{
  BYTE bits = cia_registers[regnum] ^ val;

  cia_registers[regnum] = val;

  if (val & 0x10) {
    /* Load Timer A */

    ciaa_timera_offset = ciaa_timera_latch;
  }
  if (bits & 0x01) {
    if (val & 0x01) {
      /* Start Timer A */
      ciaa_timera_base = gethrtime();
    } else {
      /* Stop Timer A */
      hrtime_t current = gethrtime() - ciaa_timera_base;

      ciaa_timera_offset -= ((current * 709379) / 100000000000LL);
      if (ciaa_timera_offset < 0) {
	ciaa_timera_offset = 0;
      }
    }
  }
}

void ciaa_write_crb(BYTE regnum, BYTE val)
{
  BYTE bits = cia_registers[regnum] ^ val;

  cia_registers[regnum] = val;

  if (val & 0x10) {
    /* Load Timer B */

    ciaa_timerb_offset = ciaa_timerb_latch;
  }
  if (bits & 0x01) {
    if (val & 0x01) {
      /* Start Timer B */
      ciaa_timerb_base = gethrtime();
    } else {
      /* Stop Timer B */
      hrtime_t current = gethrtime() - ciaa_timerb_base;

      ciaa_timerb_offset -= ((current * 709379) / 10000000000LL);
      if (ciaa_timerb_offset < 0) {
	ciaa_timerb_offset = 0;
      }
    }
  }
  if (val & 0x80) {
    /* ALARM */
    cia_write_tab[0x08] = ciaa_write_alarm;
    cia_write_tab[0x09] = ciaa_write_alarm;
    cia_write_tab[0x0a] = ciaa_write_alarm;
  } else {
    /* TOD */
    cia_write_tab[0x08] = ciaa_write_tod;
    cia_write_tab[0x09] = ciaa_write_tod;
    cia_write_tab[0x0a] = ciaa_write_tod;
  }
}

/*
 * Tables
 */

static BYTE (*cia_read_tab[])(BYTE) = {
  /* CIA A */
  cia_read_reg,	/* PRA */
  cia_read_bad,	/* PRB */
  cia_read_reg,	/* DDRA */
  cia_read_bad,	/* DDRB */
  ciaa_read_timera,	/* TALO */
  ciaa_read_timera,	/* TAHI */
  ciaa_read_timerb,	/* TBLO */
  ciaa_read_timerb,	/* TBHI */
  ciaa_read_tod,	/* TODLO */
  ciaa_read_tod,	/* TODMID */
  ciaa_read_tod,	/* TODHI */
  cia_read_bad,	/* TODHR */
  cia_read_bad,	/* SDR */
  cia_read_bad,	/* ICR */
  cia_read_reg,	/* CRA */
  cia_read_reg,	/* CRB */
  /* CIA B */
  cia_read_bad,	/* PRA */
  cia_read_bad,	/* PRB */
  cia_read_bad,	/* DDRA */
  cia_read_bad,	/* DDRB */
  cia_read_bad,	/* TALO */
  cia_read_bad,	/* TAHI */
  cia_read_bad,	/* TBLO */
  cia_read_bad,	/* TBHI */
  cia_read_bad,	/* TODLO */
  cia_read_bad,	/* TODMID */
  cia_read_bad,	/* TODHI */
  cia_read_bad,	/* TODHR */
  cia_read_bad,	/* SDR */
  cia_read_bad,	/* ICR */
  cia_read_reg,	/* CRA */
  cia_read_reg,	/* CRB */
};

static void (*cia_write_tab[])(BYTE, BYTE) = {
  /* CIA A */
  ciaa_write_pra,	/* PRA */
  cia_write_bad,	/* PRB */
  ciaa_write_ddra,	/* DDRA */
  cia_write_bad,	/* DDRB */
  ciaa_write_timera,	/* TALO */
  ciaa_write_timera,	/* TAHI */
  ciaa_write_timerb,	/* TBLO */
  ciaa_write_timerb,	/* TBHI */
  ciaa_write_tod,	/* TODLO */
  ciaa_write_tod,	/* TODMID */
  ciaa_write_tod,	/* TODHI */
  cia_write_bad,	/* TODHR */
  cia_write_bad,	/* SDR */
  cia_write_bad,	/* ICR */
  ciaa_write_cra,	/* CRA */
  ciaa_write_crb,	/* CRB */
  /* CIA B */
  cia_write_bad,	/* PRA */
  cia_write_bad,	/* PRB */
  cia_write_bad,	/* DDRA */
  cia_write_bad,	/* DDRB */
  cia_write_bad,	/* TALO */
  cia_write_bad,	/* TAHI */
  cia_write_bad,	/* TBLO */
  cia_write_bad,	/* TBHI */
  cia_write_bad,	/* TODLO */
  cia_write_bad,	/* TODMID */
  cia_write_bad,	/* TODHI */
  cia_write_bad,	/* TODHR */
  cia_write_bad,	/* SDR */
  cia_write_bad,	/* ICR */
  cia_write_bad,	/* CRA */
  cia_write_bad,	/* CRB */
};

/*
 * Glue functions
 */

ULONG read_cia(ULONG offset, ULONG base)
{
  BYTE reg = ((offset >> 8) & 0x1f);

  return(cia_read_tab[reg](reg));
}

void write_cia(ULONG offset, ULONG val, ULONG base)
{
  BYTE reg = ((offset >> 8) & 0x1f);

  cia_write_tab[reg](reg, val);
}

void reset_cia(ULONG base)
{
  fprintf(stdout, "CIA base is 0x%08lx\n", base);
  ciaa_tod_state = 0;
  ciaa_tod_base = gethrtime();
  ciaa_tod_offset = 0;
  ciaa_write_pra(0x00, 1);	/* Overlay on, led on */
  ciaa_write_crb(0x0f, 0);
}

/*
 * Initialization function
 */

void init_cia(void)
{
  add_hw(0x00bfd000, 0x00002000, "CIA A & CIA B",
	 read_bad, read_bad, read_cia,
	 write_bad, write_bad, write_cia,
	 reset_cia);
}
