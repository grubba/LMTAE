/*
 * $Id: recomp.c,v 1.9 1996/07/13 19:32:09 grubba Exp $
 *
 * M68000 to SPARC recompiler.
 *
 * $Log: recomp.c,v $
 * Revision 1.8  1996/07/12 21:11:49  grubba
 * raise_exception() might work a bit better now.
 * Although at the moment it does an abort() instead of returning.
 *
 * Revision 1.7  1996/07/12 13:11:38  marcus
 * Added the hardfile board
 *
 * Revision 1.6  1996/07/11 23:02:06  marcus
 * Real ZorroII emulation
 *
 * Revision 1.5  1996/07/11 15:41:51  grubba
 * Now has a GUI!
 * Some bug-fixes.
 *
 * Revision 1.4  1996/07/08 21:16:00  grubba
 * Now spawns a separate thread for the CPU emulation.
 *
 * Revision 1.3  1996/07/05 02:10:30  grubba
 * Switched two instances of \n to \r in compile_and_go().
 *
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
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <thread.h>

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

/* Debug level */

ULONG debuglevel;

/*
 * Functions
 */

void dump_m_regs(struct m_registers *regs)
{
  int i;

  for (i = 0; i < 0x10; i++) {
    fprintf(stdout, "%c%d: %08lx, ", (i & 0x08)?'A':'D', i & 0x07,
	    ((ULONG *)regs)[i]);
    if ((i & 0x03) == 0x03) {
      fprintf(stdout,"\n");
    }
  }
  fprintf(stdout,
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

ULONG raise_exception(struct m_registers *regs, USHORT *mem, ULONG vec)
{
  int i;
  ULONG sp = regs->a7;
  ULONG osr = regs->sr;
  
  fprintf(stderr, "raise_exception(0x%08lx)\n", vec);
  
  fprintf(stderr, "RAW register dump:\n");
  for (i=0; i*sizeof(ULONG)<sizeof(struct m_registers); i++) {
    fprintf(stderr, "%02x: 0x%08lx\n", i, ((ULONG *)regs)[i]);
  }

  fprintf(stderr, "STACK dump\n");
  for (i=0; i < 64; i++) {
    if (!(i & 7)) {
      fprintf(stderr, "\n0x%08lx\t", sp + i*sizeof(USHORT));
    }
    fprintf(stderr, "0x%04x ", ((USHORT *)(memory + sp))[i]);
  }
  fprintf(stderr, "\n");
 
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

  abort();

  /* FIXME: Old standard */
  return (*((ULONG *)(memory + (vec<<2))));
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

      if ((maddr = raise_exception(regs,
				   (USHORT *)memory,
				   VEC_BUS_ERROR + (maddr & 1))) && 0xff000001) {
	fprintf(stderr,
		"Double ADDRESS/BUS ERROR!\n"
		"New address = 0x%08lx\n",
		maddr);
	abort();
      }
      return;
    }

    if ((!segment) || (segment->maddr > maddr) || (segment->mend <= maddr)) {
      segment = find_seg(code_tree, maddr);
#ifdef DEBUG
      if (debuglevel & DL_RUNTIME_TRACE) {
	if (segment) {
	  fprintf(stdout,
		  "0x%08lx called from 0x%08lx, New segment [0x%08lx - 0x%08lx]\r",
		  maddr, regs->pc, segment->maddr, segment->mend);
	} else {
	  fprintf(stdout, "0x%08lx called from 0x%08lx\r", maddr, regs->pc);
	}
      }
#endif /* DEBUG */
    }

    if ((!segment) ||
	(!(ci = find_ci(&segment->codeinfo, maddr)))) {
      ULONG mend;
      ci = new_codeinfo(maddr);

#ifdef DEBUG
      if (debuglevel & DL_RUNTIME_TRACE) {
	fprintf(stdout, "Compiling 0x%08lx, called from 0x%08lx...\n",
		maddr, regs->pc);
      }
#endif /* DEBUG */
      mend = compile(ci);
      
      if (!segment) {
#ifdef DEBUG
	if (debuglevel & DL_COMPILER_VERBOSE) {
	  fprintf(stdout, "Creating new segment, maddr: %08lx, end: %08lx\n",
		  maddr, mend);
	}
#endif /* DEBUG */
	segment = insert_seg(&code_tree, maddr, mend);
	if (!segment) {
	  /* BUS ERROR? */
	  fprintf(stderr, "Out of memory?\n");
	  abort();
	}
      } else if (mend != segment->mend) {
	fprintf(stderr, "Strange ending, orig:%08lx, new:%08lx\n", segment->mend, mend);
	abort();
      }
      /* Link it first */
      ci->next = segment->codeinfo;
      segment->codeinfo = ci;

#ifdef DEBUG
      if (debuglevel & DL_COMPILER_DISASSEMBLY) {
	disassemble(maddr, mend);
      }
      if (debuglevel & DL_COMPILER_VERBOSE) {
	fprintf(stdout, "Execute!\n");
      }
#endif /* DEBUG */
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

void *cpu_thread_main(void *args)
{
  struct m_registers regs = {
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,
    0,
    0,0xa00000
  };
  ULONG start_addr = ((ULONG *)(memory + 0x00f80000))[1];
  regs.a7 = regs.ssp = ((ULONG *)(memory + 0x00f80000))[0];
  regs.sr = 0x00002700;

  start_register_dump(&regs);

#ifdef DEBUG
  fprintf(stdout, "Starting CPU at 0x%08lx\n", start_addr);
#endif /* DEBUG */

  compile_and_go(&regs, start_addr);

  /* Probably not reached */

  return(NULL);
}

int start_cpu(void)
{
  thread_t cpu_thread;

  return (thr_create(NULL, 0, cpu_thread_main, NULL, THR_DETACHED, &cpu_thread));
}

int devzero = -1;

int main(int argc, char **argv)
{
  int i;
  int romfd = -1;
  char *romdump = "./ROM.dump";
  unsigned char *rommem;
  
  for (i = 1;i < argc;i++) {
    if (argv[i][0] == '-') {
      switch(argv[i][1]) {
      case 'C':
	debuglevel |= DL_COMPILER_VERBOSE;
	break;
      case 'D':
	debuglevel |= DL_COMPILER_DISASSEMBLY;
	break;
      case 't':
	debuglevel |= DL_RUNTIME_TRACE;
	break;
      default:
	fprintf(stderr,
		"Usage:\n"
		"\t%s [flags] [romdump]\n\n"
		"Available flags are:\n"
		"-C\tVerbose compiler\n"
		"-D\tDisassemble\n"
		"-t\tRuntime trace\n"
		"\nThe current romdump is \"%s\".\n",
		argv[0], romdump);
	exit(1);
	break;
      }
    } else {
      romdump = argv[i];
    }
  }

  if ((devzero = open("/dev/zero", O_RDONLY)) >= 0) {
    if ((memory = (UBYTE *)mmap((caddr_t)NULL, 16*1024*1024, PROT_READ|PROT_WRITE,
				MAP_PRIVATE, devzero, 0))) {
      if ((romfd = open(romdump, O_RDONLY)) >= 0) {
	if ((rommem = (UBYTE *)mmap((caddr_t)(memory + 0x00f80000), 512*1024,
				    PROT_READ, MAP_SHARED|MAP_FIXED, romfd, 0))) {
	  if (rommem == memory + 0x00f80000) {

	    start_info_window();

#ifdef DEBUG
	    printf("Kickstart V%d.%d\n"
		   "exec.library V%d.%d\n",
		   ((USHORT *)(memory + 0x00f8000c))[0],
		   ((USHORT *)(memory + 0x00f8000c))[1],
		   ((USHORT *)(memory + 0x00f8000c))[2],
		   ((USHORT *)(memory + 0x00f8000c))[3]);
	    if (memory[0x00f80018]) {
	      /* Old style kickstart */
	      unsigned char *cr;
	      cr = (unsigned char *)strchr((char *)memory + 0x00f80018, '\0') + 1;
	      while (!*cr) {
		cr++;
	      }
	      if (cr[0] == 255) {
		cr = (unsigned char *)(((ULONG)(cr + 4)) & ~3);
	      }
	      printf("%s", cr);
	    } else {
	      /* New style kickstart */
	      int i;
	      unsigned char *cr = memory + 0x00f80019;

	      for (i = 0; i < 5; i++) {
		printf("%s\n", cr);
		cr = (unsigned char *)strchr((char *)cr, '\0') + 1;
	      }
	    }
#endif /* DEBUG */

	    zorro_addram(2);
	    zorro_addboard("HardFile", "boards/hardfile.rom",
			   "boards/hardfile.so");

	    reset_hw();

	    start_cpu();

	    sleep(500000);

	  } else {
	    fprintf(stderr, "Bad ROM-map\n");
	  }
	} else {
	  fprintf(stderr, "Couldn't mmap \"%s\"\n", romdump);
	}
      } else {
	fprintf(stderr, "Couldn't open \"%s\"\n", romdump);
      }
    } else {
      fprintf(stderr, "Couldn't mmap /dev/zero\n");
    }
  } else {
    fprintf(stderr, "Couldn't open /dev/zero\n");
  }
  return(0);
}

