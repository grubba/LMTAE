/*
 * $Id: interrupt.h,v 1.2 1996/07/21 16:16:18 grubba Exp $
 *
 * Interrupt handling
 *
 * $Log: interrupt.h,v $
 * Revision 1.1  1996/07/19 16:46:19  grubba
 * Cleaned up interrupt handling.
 * Cleaned up custom chip emulation.
 * INTENA/INTREQ should work.
 *
 *
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

/*
 * Includes
 */

#include <thread.h>
#include <synch.h>

#ifndef TYPES_H
#include "types.h"
#endif /* TYPES_H */

#ifndef M68K_H
#include "m68k.h"
#endif /* M68K_H */

/*
 * Globals
 */

/*
 * CPU Interrupts
 */
extern mutex_t cpu_irq_lock;
extern cond_t cpu_irq_signal;
extern U32 cpu_irq;

/*
 * Interrupt Control
 */
extern mutex_t irq_ctrl_lock;
extern cond_t irq_ctrl_signal;

/*
 * Prototypes
 */

/* Hardware interface */
void custom_write_intena(U32 reg, U16 val);
void custom_write_intreq(U32 reg, U16 val);

/* CPU interface */
U32 interrupt(struct m_registers *regs, U8 *mem, U32 nextpc, U32 mask);
U32 s_stop(struct m_registers *regs, U8 *mem, U32 nextpc);

/* Initialization */
void init_interrupt(void);

#endif /* INTERRUPT_H */
