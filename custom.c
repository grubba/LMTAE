/*
 * $Id: custom.c,v 1.1 1996/07/19 16:46:10 grubba Exp $
 *
 * Custom chip emulation
 *
 * $Log$
 *
 */

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>

#include "types.h"

#include "recomp.h"
#include "interrupt.h"
#include "hardware.h"

/*
 * Functions
 */

static void custom_read_only(U32 reg, U16 val)
{
  printf("Write to read-only register!\n");
}

static S16 custom_write_only(U32 reg)
{
  printf("Read from write-only register!\n");

  return(0);
}

static S16 custom_read_vhposr(U32 reg)
{
  fprintf(stdout, "Reading VHPOSR 0x%08x\n", ((U32 *)memory)[0xdff004>>2]);
  ((U32 *)memory)[0xdff004>>2] += 0x00000101;
  ((U32 *)memory)[0xdff004>>2] &= 0x0001ffff;
  return (((S16 *)memory)[0xdff006>>1]);
}

static void custom_write_intena(U32 reg, U16 val)
{
  U16 new_value;

  mutex_lock(&irq_ctrl_lock);
  new_value = ((U16 *)memory)[0xdff01c];

  if (val & 0x8000) {
#ifdef DEBUG
    if (val & 0x4000) {
      printf("Enabling interrupts\n");
    }
#endif /* DEBUG */
    new_value |= val & 0x7fff;
  } else {
#ifdef DEBUG
    if (val & 0x4000) {
      printf("Disabling interrupts\n");
    }
#endif /* DEBUG */
    new_value &= (~val) & 0x7fff;
  }
  ((U16 *)memory)[0xdff01c>>1] = new_value;

  cond_signal(&irq_ctrl_signal);
  mutex_unlock(&irq_ctrl_lock);
}

static void custom_write_intreq(U32 reg, U16 val)
{
  U16 new_value;

  mutex_lock(&irq_ctrl_lock);
  new_value = ((U16 *)memory)[0xdff01e];

  if (val & 0x8000) {
#ifdef DEBUG
    if (val & 0x7fff) {
      printf("Requesting an interrupt\n");
    }
#endif /* DEBUG */
    new_value |= val & 0x7fff;
  } else {
#ifdef DEBUG
    if (val & 0x7fff) {
      printf("Clearing an interrupt\n");
    }
#endif /* DEBUG */
    new_value &= (~val) & 0x7fff;
  }
  ((U16 *)memory)[0xdff01e>>1] = new_value;

  cond_signal(&irq_ctrl_signal);
  mutex_unlock(&irq_ctrl_lock);
}

/*
 * Tables
 */

#include "custom_tab.c"

/*
 * Functions
 */

U32 read_custom(U32 addr, U32 base)
{
  U32 reg = (addr & 0xfff)>>1;

  fprintf(stdout, "read_custom(0x%08x, 0x%08x)\n", addr, base);

  if (reg < 0xe0) {
    S16 (*read_func)(U32) = custom_read_tab[reg];

    if (read_func) {
      return(read_func(reg));
    } else {
      return (((S16 *)memory)[addr >> 1]);
    }
  } else {
    printf("Unknown custom register!\n");
    return (0);
  }
}

void write_custom(U32 addr, U32 val, U32 base)
{
  U32 reg = (addr & 0xfff)>>1;

  fprintf(stdout, "write_custom(0x%08x, 0x%08x, 0x%08x)\n", addr, val, base);

  if (reg < 0xc0) {
    void (*write_func)(U32, U16) = custom_write_tab[reg];

    if (write_func) {
      write_func(reg, val);
    } else {
      ((U16 *)memory)[addr>>1] = val;
    }
  } else {
    printf("Unknown custom register!\n");
  }

  if (((addr - 0xdff000) >= 0x180) && ((addr - 0xdff000) < (0x180 + 0x20))) {
    fprintf(stdout, "Setting color %d to 0x%04x\n",
	    ((addr - 0xdff000) - 0x180), (val & 0xffff));
  }
}

void reset_custom(U32 base)
{
  fprintf(stdout, "Custom base is 0x%08x\n", base);
}

void init_custom(void)
{
  add_hw(0x00dff000, 0x00001000, "Custom Chips", 
	 NULL, read_custom, read_bad,
	 write_bad, write_custom, write_bad,
	 reset_custom);
}

