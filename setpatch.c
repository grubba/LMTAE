/*
 * $Id: setpatch.c,v 1.1 1996/07/13 19:32:05 grubba Exp $
 *
 * Patches compiled M68000 code with Sparc code.
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
#include "setpatch.h"

#ifdef DEBUG
void breakme(void);
#else
#define breakme		abort
#endif /* DEBUG */

/*
 * Functions
 */

ULONG (*setpatch(ULONG maddr, ULONG (*code)(struct m_registers *, void *)))(struct m_registers *, void *)
{
  struct seg_info *seg;
  struct code_info *ci;
  
#ifdef DEBUG
  printf("Setpatch(0x%08lx)\n", maddr);
#endif /* DEBUG */

  if (maddr & 0xff000001) {
    fprintf(stderr, "setpatch: Bad address 0x%08lx\n", maddr);
    return(NULL);
  }

  if ((!(seg = find_seg(code_tree, maddr))) ||
      (!(ci = find_ci(&seg->codeinfo, maddr)))) {
    ULONG mend;
    ci = new_codeinfo(maddr);

#ifdef DEBUG
    if (debuglevel & DL_RUNTIME_TRACE) {
      fprintf(stdout, "setpatch: Compiling 0x%08lx...\n", maddr);
    }
#endif /* DEBUG */
    mend = compile(ci);
      
    if (!seg) {
#ifdef DEBUG
      if (debuglevel & DL_COMPILER_VERBOSE) {
	fprintf(stdout, "Creating new segment, maddr: %08lx, end: %08lx\n",
		maddr, mend);
      }
#endif /* DEBUG */
      seg = insert_seg(&code_tree, maddr, mend);
      if (!seg) {
	/* BUS ERROR? */
	fprintf(stderr, "Out of memory?\n");
	abort();
      }
    } else if (mend != seg->mend) {
      fprintf(stderr, "Strange ending, orig:%08lx, new:%08lx\n", seg->mend, mend);
      abort();
    }
    /* Link it first */
    ci->next = seg->codeinfo;
    seg->codeinfo = ci;

#ifdef DEBUG
    if (debuglevel & DL_COMPILER_DISASSEMBLY) {
      disassemble(maddr, mend);
    }
#endif /* DEBUG */    
  }
  /* Don't attempt to patch an already patched function */
  if (ci->flags & CIF_PATCHED) {
    return(NULL);
  } else {
    ULONG (*retval)(struct m_registers *, void *);

     /* Don't attempt to free a patched function in the garbage collector */
    retval = ci->code;
    if (retval) {
      ci->flags |= CIF_LOCKED | CIF_PATCHED;
      ci->code = code;
    } else {
      fprintf(stderr, "Setpatch(0x%08lx): retval == NULL\n", maddr);
      breakme();
    }
    return(retval);
  }
}

void unpatch(ULONG maddr, ULONG (*code)(struct m_registers *, void *)) {
  struct seg_info *seg;
  struct code_info *ci;

  if ((!(seg = find_seg(code_tree, maddr))) ||
      (!(ci = find_ci(&seg->codeinfo, maddr))) ||
      (!(ci->flags & CIF_PATCHED))) {
    return;
  }
  ci->code = code;
  ci->flags &= ~CIF_PATCHED;
}
