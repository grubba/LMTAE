/*
 * $Id: peephole.h,v 1.2 1996/07/17 16:01:42 grubba Exp $
 *
 * Peephole optimizer for the M68000 to Sparc recompiler.
 *
 * $Log: peephole.h,v $
 * Revision 1.1  1996/07/01 19:16:55  grubba
 * Implemented ASL and ASR.
 * Changed semantics for new_codeinfo(), it doesn't allocate space for the code.
 * Added PeepHoleOptimize(). At the moment it just mallocs and copies the code.
 * Removed some warnings.
 *
 *
 */

#ifndef PEEPHOLE_H
#define PEEPHOLE_H

/*
 * Functions
 */

void PeepHoleOptimize(struct code_info *ci, U32 *start, U32 *end);

#endif /* PEEPHOLE_H */
