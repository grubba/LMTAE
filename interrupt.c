/*
 * $Id: interrupt.c,v 1.4 1998/02/10 02:41:56 marcus Exp $
 *
 * Interrupt handling
 *
 * $Log: interrupt.c,v $
 * Revision 1.3  1998/02/10 01:01:15  marcus
 * Bugfix.
 *
 * Revision 1.2  1996/07/21 16:16:16  grubba
 * custom_write_intena() and custom_write_intreq() moved to interrupt.[ch].
 * Serialport emulation on stdin/stdout added.
 * Custom registers that are write-only are now declared as such.
 *
 * Revision 1.1  1996/07/19 16:46:17  grubba
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
#include "m68k.h"
#include "interrupt.h"

/*
 * Globals
 */

/*
 * CPU interrupts
 */
mutex_t cpu_irq_lock;
cond_t cpu_irq_signal;		/* Send this when cpu_irq changes */
U32 cpu_irq = 0;

/*
 * Interrupt Control
 */
mutex_t irq_ctrl_lock;
cond_t irq_ctrl_signal;		/* Send this when INTENA or INTREQ changes */

/*
 * Functions
 */

/*
 * Hardware interface
 */

void custom_write_intena(U32 reg, U16 val)
{
  U16 new_value;

  mutex_lock(&irq_ctrl_lock);
  new_value = ((U16 *)memory)[0xdff01c>>1];

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

void custom_write_intreq(U32 reg, U16 val)
{
  U16 new_value;

  mutex_lock(&irq_ctrl_lock);
  new_value = ((U16 *)memory)[0xdff01e>>1];

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
 * CPU interface
 */

/* Called when the CPU is about to take an interrupt. */
U32 interrupt(struct m_registers *regs, U8 *mem, U32 nextpc, U32 mask)
{
  /* NOTE: On entry cpu_irq_lock is locked */

#ifdef DEBUG
  printf("interrupt(0x%08x, 0x%08x)\n", nextpc, mask);
#endif /* DEBUG */
  if (mask & 0x7f) {
    U32 bitmask = 0xc0;
    int intnum = 7;
    U32 vecnum;

    while (!(mask & bitmask)) {
      bitmask >>= 1;
      intnum--;
    }
    cpu_irq &= ~bitmask; 

    mutex_unlock(&cpu_irq_lock);

    if (!(regs->sr & 0x2000)) {
      /* Was in Usermode */

      /* Flip stacks */

      regs->usp = regs->a7;
      regs->a7 = regs->ssp;
    }
    
    /* FIXME: Should check for code clobbering and legal memory */

    /* Push vector number */
    vecnum = VEC_SPURIOUS + intnum;
    regs->a7 -= 2;
    *((U16 *)(mem + regs->a7)) = vecnum;

    /* Push next PC */
    regs->a7 -= 2;
    *((U16 *)(mem + regs->a7)) = nextpc;
    regs->a7 -= 2;
    *((U16 *)(mem + regs->a7)) = (nextpc >> 16);

    /* Push old SR */
    regs->a7 -= 2;
    *((U16 *)(mem + regs->a7)) = regs->sr;

    /* Update SR */
    regs->sr &= ~0x0700;
    regs->sr |= 0x2000 | (intnum << 8);

    vecnum = regs->vbr + (vecnum << 2);

    return ((((U16 *)(mem + vecnum))[0]<<16) | (((U16 *)(mem + vecnum))[1]));
  } else {
    abort();
  }
}

/* Called when the CPU executes a STOP instruction */
U32 s_stop(struct m_registers *regs, U8 *mem, U32 nextpc)
{
  U32 irq_mask;

  printf("Stop(0x%04x, 0x%08x)!\n", regs->sr, nextpc);

  mutex_lock(&cpu_irq_lock);
  while (!(irq_mask = (cpu_irq & (~0 << ((regs->sr & 0x0700)>>8))))) {
    cond_wait(&cpu_irq_signal, &cpu_irq_lock);
  }

  /* NOTE: interrupt() releases the lock */
  return(interrupt(regs, mem, nextpc, irq_mask));
}

/* The interrupt controller */
static void *interrupt_control_main(void *args)
{
  volatile U16 *custom = (U16 *)(memory + 0x00dff000);

  mutex_lock(&irq_ctrl_lock);

  while (1) {
    if (custom[0x1c>>1] & 0x4000) {
      /* Interrupts enabled */

      U16 mask = (custom[0x1c>>1] & custom[0x1e>>1]);

      if (mask) {
	/* Interrupt requested */
	const U32 customtomotorola[15] = {
	  0x01, 0x01, 0x01, 0x02,
	  0x04, 0x04, 0x04, 0x08,
	  0x08, 0x08, 0x08, 0x10,
	  0x10, 0x20, 0x20
	};
	U32 irq = 0;
	int bitnum;
	U16 bitmask;

	for (bitnum = 14, bitmask = 1<<14; bitmask; bitnum--, bitmask>>=1) {
	  if (mask & bitmask) {
	    irq |= customtomotorola[bitnum];
	  }
	}
	mutex_lock(&cpu_irq_lock);
	cpu_irq |= irq;
	cond_signal(&cpu_irq_signal);
	mutex_unlock(&cpu_irq_lock);
      }
    }
    cond_wait(&irq_ctrl_signal, &irq_ctrl_lock);
  }
  /* NOT REACHED */

  return(NULL);
}

void init_interrupt(void)
{
  thread_t int_ctrl_thread;

  mutex_init(&cpu_irq_lock, USYNC_THREAD, NULL);
  cond_init(&cpu_irq_signal, USYNC_THREAD, NULL);

  mutex_init(&irq_ctrl_lock, USYNC_THREAD, NULL);
  cond_init(&irq_ctrl_signal, USYNC_THREAD, NULL);

  thr_create(NULL, 0, interrupt_control_main, NULL,
	     THR_DETACHED | THR_DAEMON, &int_ctrl_thread);
}
