/*
 * $Id: recomp.c,v 1.3 1996/07/05 02:10:30 grubba Exp $
 *
 * M68000 to SPARC recompiler.
 *
 * $Log: recomp.c,v $
 * Revision 1.2  1996/07/01 19:16:59  grubba
 * Implemented ASL and ASR.
 * Changed semantics for new_codeinfo(), it doesn't allocate space for the code.
 * Added PeepHoleOptimize(). At the moment it just mallocs and copies the code.
 * Removed some warnings.
 *
 * Revision 1.1.1.1  1996/06/30 23:51:50  grubba
 * Entry into CVS
 *
 * Revision 1.5  1996/06/19 11:08:25  grubba
 * *** empty log message ***
 *
 * Revision 1.4  1996/06/01 09:31:29  grubba
 * Major changes.
 * Now generates output split into several files.
 * Now generates gasp assembler preprocessor output.
 * Now uses templates to build instructions.
 *
 * Revision 1.3  1996/05/12 16:43:15  grubba
 * Modified the exception handling. It might even work now.
 * Commented out the old compile() function.
 *
 * Revision 1.2  1996/05/06 14:50:54  grubba
 * Added m68k.h
 *
 * Revision 1.1  1996/05/06 11:19:57  grubba
 * Initial revision
 *
 */

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "recomp.h"
#include "codeinfo.h"
#include "m68k.h"
#include "sparc.h"


/*
 * Define this to get runtime sanity checks of the emulated Amiga
 */

#define SANITY

#ifdef SANITY

void check_sanity(struct code_info *ci, struct m_registers *regs, unsigned char *memory);

#endif /* SANITY */

/*
 * Globals
 */

unsigned char *memory;
extern struct seg_info *code_tree;


/*
 * Functions
 */

void dump_m_regs(struct m_registers *regs)
{
  int i;

  for (i = 0; i < 0x10; i++) {
    fprintf(stderr, "%c%d: %08lx, ", (i & 0x08)?'A':'D', i & 0x07,
	    ((ULONG *)regs)[i]);
    if ((i & 0x03) == 0x03) {
      fprintf(stderr,"\n");
    }
  }
  fprintf(stderr,
	  "usp: %08lx, ssp: %08lx, sr: %08lx, pc: %08lx\n"
	  "low: %08lx, high: %08lx, vbr: %08lx\n",
	  regs->usp, regs->ssp, regs->sr, regs->pc,
	  regs->low, regs->high, regs->vbr);
}

struct code_info *new_codeinfo(ULONG maddr)
{
  struct code_info *ci = calloc(sizeof(struct code_info),1);

  if (ci) {
    ci->maddr = maddr;
  }
  return(ci);
}

volatile void raise_exception(struct m_registers *regs, USHORT *mem, ULONG vec)
{
  ULONG osr = regs->sr;
  
  fprintf(stderr, "raise_exception(0x%08lx)\n", vec);
  
  /* FIXME: Need to check for bad SSP => HALT */
  /* FIXME: Add VBR */

  if (!regs & M_SR_S) {
    regs->sr |= M_SR_S;
    /* Flip stacks */
    regs->usp=regs->a7;
    regs->a7=regs->ssp;
  }
  regs->a7 -= 2;
  *((USHORT *)(memory + regs->a7)) = regs->pc >> 16;
  regs->a7 -= 2;
  *((USHORT *)(memory + regs->a7)) = regs->pc & 0xffff;
  regs->a7 -= 2;
  *((USHORT *)(memory + regs->a7)) = osr;

  /* FIXME: Old standard */
  compile_and_go(regs, *((ULONG *)(memory + (vec<<2))));
}

volatile void compile_and_go(struct m_registers *regs, ULONG maddr)
{
  struct seg_info *segment = NULL;
  struct code_info *ci = NULL;
  struct code_info *old_ci = NULL;

  while (1) {

    /* FIXME: Should test for interrupts here */

    if (maddr & 0xff000001) {
      /* BUS ERROR or ADDRESS ERROR*/
      
      fprintf(stderr, "BUS ERROR! Bad address! 0x%08lx\n", maddr);

      raise_exception(regs, (USHORT *)memory, VEC_BUS_ERROR + (maddr & 1));
      return;
    }

    if ((!segment) || (segment->maddr > maddr) || (segment->mend <= maddr)) {
      segment = find_seg(code_tree, maddr);
#ifdef DEBUG
      if (segment) {
	fprintf(stderr,
		"0x%08lx called from 0x%08lx, New segment [0x%08lx - 0x%08lx]\r",
		maddr, regs->pc, segment->maddr, segment->mend);
      } else {
	fprintf(stderr, "0x%08lx called from 0x%08lx\r", maddr, regs->pc);
      }
#endif /* DEBUG */
    }

    if ((!segment) ||
	(!(ci = find_ci(&segment->codeinfo, maddr)))) {
      ULONG mend;
      ci = new_codeinfo(maddr);

      fprintf(stderr, "Compiling 0x%08lx, called from 0x%08lx...\n",
	      maddr, regs->pc);

      mend = compile(ci);
      
      if (!segment) {
	fprintf(stderr, "Creating new segment, maddr: %08lx, end: %08lx\n",
		maddr, mend);
	segment = insert_seg(&code_tree, maddr, mend);
	if (!segment) {
	  /* BUS ERROR? */
	  return;
	}
      } else if (mend != segment->mend) {
	fprintf(stderr, "Strange ending, orig:%08lx, new:%08lx\n", segment->mend, mend);
	abort();
      }
      /* Link it first */
      ci->next = segment->codeinfo;
      segment->codeinfo = ci;

      fprintf(stderr, "Execute!\n");
    }

    regs->pc = maddr;
    /* Execute the code */
    maddr = ci->code(regs, memory);
    old_ci = ci;

#ifdef SANITY
    check_sanity(ci, regs, memory);
#endif /* SANITY */
  }
}

int main(int argc, char **argv)
{
  int zfd = -1;
  int romfd = -1;
  unsigned char *rommem;

  if ((zfd = open("/dev/zero", O_RDONLY)) >= 0) {
    if ((memory = mmap(NULL, 16*1024*1024, PROT_READ|PROT_WRITE,
		       MAP_PRIVATE, zfd, 0))) {
      if ((romfd = open("./ROM.dump", O_RDONLY)) >= 0) {
	if ((rommem = mmap(memory + 0x00f80000, 512*1024, PROT_READ,
			   MAP_SHARED|MAP_FIXED, romfd, 0))) {
	  if (rommem == memory + 0x00f80000) {
	    struct m_registers regs = {
	      0,0,0,0,0,0,0,0,
	      0,0,0,0,0,0,0,0,
	      0,0,0,0,
	      0,0xa00000
	    };
	    ULONG start_addr = ((ULONG *)rommem)[1];
	    regs.a7 = regs.ssp = ((ULONG *)rommem)[0];
	    regs.sr = 0x00002700;

#ifdef DEBUG
	    fprintf(stderr, "Starting CPU at 0x%08lx\n", start_addr);
#endif /* DEBUG */

	    compile_and_go(&regs, start_addr);

#if 0
	    struct code_info ci = {
	      NULL,
	      0xf800d2,
	      0,
	      NULL,
	      NULL,
	      0,
	    };
	    
	    if ((ci.code = (void *)calloc(4, 1024))) {
	      ULONG mend;
	      ULONG *dump;

	      printf("Compiling...\n");
	      mend = compile(&ci);

	      printf("Motorola code from 0x00f800d2 to 0x%08x\n", mend);
	      printf("Sparc code from 0x%08x to 0x%08x\n", ci.code, ci.codeend);
	      for (dump = (void *)ci.code; dump <= ci.codeend;) {
		int j;
		printf("%08x: ", dump);
		for (j = 0; j < 8; j++) {
		  printf("%08x ", *(dump++));
		}
		printf("\n");
	      }
	    }
#endif /* 0 */
	  } else {
	    fprintf(stderr, "Bad ROM-map\n");
	  }
	} else {
	  fprintf(stderr, "Couldn't mmap ./ROM.dump\n");
	}
      } else {
	fprintf(stderr, "Couldn't open ./ROM.dump\n");
      }
    } else {
      fprintf(stderr, "Couldn't mmap /dev/zero\n");
    }
  } else {
    fprintf(stderr, "Couldn't open /dev/zero\n");
  }
  return(0);
}

