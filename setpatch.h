/*
 * $Id: setpatch.h,v 1.2 1996/07/17 16:01:59 grubba Exp $
 *
 * Patches compiled M68000 code with Sparc code.
 *
 * $Log: setpatch.h,v $
 * Revision 1.1  1996/07/13 19:32:07  grubba
 * Now defaults to very little debuginfo.
 * Added (un|set)patch().
 * Patches added to MakeLibrary(), MakeFunctions(), Abort() and AddLibrary().
 *
 *
 */

#ifndef SETPATCH_H
#define SETPATCH_H

/*
 * Prototypes
 */

U32 (*setpatch(U32 maddr, U32 (*code)(struct m_registers *, void *)))(struct m_registers *, void *);
void unpatch(U32 maddr, U32 (*code)(struct m_registers *, void *));

#endif /* SETPATCH_H */
