/*
 * $Id: interrupt.c,v 1.1 1996/07/19 16:46:17 grubba Exp $
 *
 * Interrupt handling
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

void request_interrupt(int intnum)
{
  printf("request_interrupt(%d)\n", intnum);
  if (intnum) {
    U32 intmask;
    if (intnum >= 7) {
      intmask = 0xc0;
    } else {
      intmask = 1<<(intnum-1);
    }
    mutex_lock(&cpu_irq_lock);
    cpu_irq |= intmask;
    mutex_unlock(&cpu_irq_lock);
    cond_signal(&cpu_irq_signal);
  } else {
    abort();
  }
}

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
    *((U16 *)(mem + regs->a7)) = vecnum;
    regs->a7 -= 2;

    /* Push next PC */
    *((U16 *)(mem + regs->a7)) = (nextpc >> 16);
    regs->a7 -= 2;
    *((U16 *)(mem + regs->a7)) = nextpc;
    regs->a7 -= 2;

    /* Push old SR */
    *((U16 *)(mem + regs->a7)) = regs->sr;
    regs->a7 -= 2;

    /* Update SR */
    regs->sr &= ~0x0700;
    regs->sr |= 0x2000 | (intnum << 8);

    vecnum = regs->vbr + (vecnum << 2);

    return ((((U16 *)(mem + vecnum))[0]<<16) | (((U16 *)(mem + vecnum))[1]));
  } else {
    abort();
  }
}

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

void *interrupt_control_main(void *args)
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
