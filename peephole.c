/*
 * $Id: peephole.c,v 1.1 1996/07/01 19:16:53 grubba Exp $
 *
 * Peephole optimizer for the M68000 to Sparc recompiler.
 *
 * $Log$
 *
 */

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>

#include "recomp.h"
#include "codeinfo.h"
#include "peephole.h"

/*
 * Functions
 */

void PeepHoleOptimize(struct code_info *ci, ULONG *start, ULONG *end)
{
  ULONG size = ((ULONG)end) - ((ULONG)start);

  if (!(ci->code = (ULONG (*)(struct m_registers *, void *))malloc(size))) {
    fprintf(stderr, "Out of memory in PeepHoleOptimize()\n");
    abort();
  }
  ci->codeend = (ULONG *)(((ULONG)memcpy((void *)ci->code, start, size)) + size);
}
