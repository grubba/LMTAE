/*
 * $Id: interrupt.h,v 1.1 1996/07/19 16:46:19 grubba Exp $
 *
 * Interrupt handling
 *
 * $Log$
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

void request_interrupt(int intnum);
U32 interrupt(struct m_registers *regs, U8 *mem, U32 nextpc, U32 mask);
U32 s_stop(struct m_registers *regs, U8 *mem, U32 nextpc);
void init_interrupt(void);

#endif /* INTERRUPT_H */
