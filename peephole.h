/*
 * $Id: peephole.h,v 1.1 1996/07/01 19:16:55 grubba Exp $
 *
 * Peephole optimizer for the M68000 to Sparc recompiler.
 *
 * $Log$
 *
 */

#ifndef PEEPHOLE_H
#define PEEPHOLE_H

/*
 * Functions
 */

void PeepHoleOptimize(struct code_info *ci, ULONG *start, ULONG *end);

#endif /* PEEPHOLE_H */
