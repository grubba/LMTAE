/*
 * $Id: setpatch.h,v 1.1 1996/07/13 19:32:07 grubba Exp $
 *
 * Patches compiled M68000 code with Sparc code.
 *
 * $Log$
 *
 */

#ifndef SETPATCH_H
#define SETPATCH_H

/*
 * Prototypes
 */

ULONG (*setpatch(ULONG maddr, ULONG (*code)(struct m_registers *, void *)))(struct m_registers *, void *);
void unpatch(ULONG maddr, ULONG (*code)(struct m_registers *, void *));

#endif /* SETPATCH_H */
