/*
 * $Id: custom.c,v 1.2 1996/07/21 16:16:10 grubba Exp $
 *
 * Custom chip emulation
 *
 * $Log: custom.c,v $
 * Revision 1.1  1996/07/19 16:46:10  grubba
 * Cleaned up interrupt handling.
 * Cleaned up custom chip emulation.
 * INTENA/INTREQ should work.
 *
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
 * Globals
 */

/* Serial port */
mutex_t serial_lock;
cond_t serial_signal;

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

static S16 custom_read_serdatr(U32 reg)
{
  S16 retval;
  mutex_lock(&serial_lock);
  retval = ((S16 *)memory)[0xdff018>>1];
  ((S16 *)memory)[0xdff018>>1] = 0x3000;	/* Always ready to transmit */

  if (retval & 0x4000) {
    cond_signal(&serial_signal);
  }
  mutex_unlock(&serial_lock);

  return (retval);
}

static void custom_write_serdat(U32 reg, U16 val)
{
  fputc(val & 0xff, stdout);
  fflush(stdout);
  memory[0xdff018] |= 0x30;		/* TSRE | TBE */
  custom_write_intreq(0, 0x8001);	/* TBE Transmit Buffer Empty */
}

/*
 * Tables
 */

#include "custom_tab.c"

/*
 * Functions
 */

void *serial_main(void *arg)
{
  while (1) {
    int c = fgetc(stdin);

    if (c == EOF) {
      printf("Serial buffer received EOF!\nDisconnecting...\n");
      break;
    }

    mutex_lock(&serial_lock);

    while (memory[0xdff018] & 0x40) {
      /* Receive Buffer Full */
      cond_wait(&serial_signal, &serial_lock);
    }
    ((U16 *)memory)[0xdff018>>1] = 0x7300 | (c & 0xff);

    custom_write_intreq(0, 0x8800);	/* RBF Receive Buffer Full */

    mutex_unlock(&serial_lock);
  }
  return (NULL);
}

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
  thread_t serial_thread;

  add_hw(0x00dff000, 0x00001000, "Custom Chips", 
	 NULL, read_custom, read_bad,
	 write_bad, write_custom, write_bad,
	 reset_custom);

  /* Serial port */
  mutex_init(&serial_lock, USYNC_THREAD, NULL);
  cond_init(&serial_signal, USYNC_THREAD, NULL);
  thr_create(NULL, 0, serial_main, NULL,
	     THR_DETACHED | THR_DAEMON, &serial_thread);
}

