/*
 * $Id: recomp.c,v 1.17 1996/08/11 17:36:16 grubba Exp $
 *
 * M68000 to SPARC recompiler.
 *
 * $Log: recomp.c,v $
 * Revision 1.16  1996/08/11 14:48:59  grubba
 * Added option to turn off SR optimization.
 *
 * Revision 1.15  1996/07/19 16:46:22  grubba
 * Cleaned up interrupt handling.
 * Cleaned up custom chip emulation.
 * INTENA/INTREQ should work.
 *
 * Revision 1.14  1996/07/17 19:16:30  grubba
 * Implemented interrupts. None generated yet though.
 * Implemented STOP.
 *
 * Revision 1.13  1996/07/17 16:01:45  grubba
 * Changed from {U,}{LONG,WORD,BYTE} to [SU]{8,16,32}.
 * Hopefully all places got patched.
 *
 * Revision 1.12  1996/07/17 00:21:45  grubba
 * Added USP and SSP info to the Supervisor state change output.
 *
 * Revision 1.11  1996/07/15 20:36:20  grubba
 * Now tracks changes Supervisor mode <=> User mode.
 *
 * Revision 1.10  1996/07/14 21:44:23  grubba
 * Added support for adding hardware dynamically.
 * Added CIAA time of day clock (50Hz).
 * Moved some debug output from stderr to stdout.
 *
 * Revision 1.9  1996/07/13 19:32:09  grubba
 * Now defaults to very little debuginfo.
 * Added (un|set)patch().
 * Patches added to MakeLibrary(), MakeFunctions(), Abort() and AddLibrary().
 *
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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <thread.h>
#include <synch.h>

#include "types.h"

#include "recomp.h"
#include "codeinfo.h"
#include "m68k.h"
#include "sparc.h"
#include "hardware.h"
#include "interrupt.h"

/*
 * Define this to get runtime sanity checks of the emulated Amiga
 */

#define SANITY

#ifdef SANITY

void check_sanity(struct code_info *ci, struct m_registers *regs, unsigned char *memory);

#endif /* SANITY */

/*
 * Threshold for statistics
 */

#define STATISTICS_THRESHOLD	100000

/*
 * Globals
 */

unsigned char *memory;
extern struct seg_info *code_tree;


/* Debug level */

U32 debuglevel = 0;

/*
 * Functions
 */

void dump_m_regs(struct m_registers *regs)
{
  int i;

  for (i = 0; i < 0x10; i++) {
    fprintf(stdout, "%c%d: %08x, ", (i & 0x08)?'A':'D', i & 0x07,
	    ((U32 *)regs)[i]);
    if ((i & 0x03) == 0x03) {
      fprintf(stdout,"\n");
    }
  }
  fprintf(stdout,
	  "usp: %08x, ssp: %08x, sr: %08x, pc: %08x\n"
	  "low: %08x, high: %08x, vbr: %08x\n",
	  regs->usp, regs->ssp, regs->sr, regs->pc,
	  regs->low, regs->high, regs->vbr);
}

struct code_info *new_codeinfo(U32 maddr)
{
  struct code_info *ci = calloc(sizeof(struct code_info),1);

  if (ci) {
    ci->maddr = maddr;
  }
  return(ci);
}

U32 raise_exception(struct m_registers *regs, U16 *mem, U32 vec)
{
  int i;
  U32 sp = regs->a7;
  U32 osr = regs->sr;
  
  fprintf(stderr, "raise_exception(0x%08x)\n", vec);
  
  fprintf(stderr, "RAW register dump:\n");
  for (i=0; i*sizeof(U32)<sizeof(struct m_registers); i++) {
    fprintf(stderr, "%02x: 0x%08x\n", i, ((U32 *)regs)[i]);
  }

  fprintf(stderr, "STACK dump\n");
  for (i=0; i < 64; i++) {
    if (!(i & 7)) {
      fprintf(stderr, "\n0x%08x\t", sp + i*sizeof(U16));
    }
    fprintf(stderr, "0x%04x ", ((U16 *)(memory + sp))[i]);
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
  *((U16 *)(memory + regs->a7)) = regs->pc >> 16;
  regs->a7 -= 2;
  *((U16 *)(memory + regs->a7)) = regs->pc & 0xffff;
  regs->a7 -= 2;
  *((U16 *)(memory + regs->a7)) = osr;

  abort();

  return (*((U32 *)(memory + (vec<<2))));
}

volatile void compile_and_go(struct m_registers *regs, U32 maddr)
{
  struct seg_info *segment = NULL;
  struct code_info *ci = NULL;
  struct code_info *old_ci = NULL;
  U32 oldsr = regs->sr;
  S64 run_time;
  S64 comp_time;
  U32 run_instr;
  U32 comp_instr;

  if (debuglevel & DL_STATISTICS) {
    run_time = -gethrtime();
    comp_time = 0;
    run_instr = 0;
    comp_instr = 0;
  }

  while (1) {
    U32 irq_mask;

    /* FIXME: Should test for interrupts here */

    mutex_lock(&cpu_irq_lock);

    if ((irq_mask = (cpu_irq & (~0 << ((regs->sr & 0x0700)>>8))))) {
      maddr = interrupt(regs, memory, maddr, irq_mask);
    } else {
      mutex_unlock(&cpu_irq_lock);
    }

    if ((!maddr) || (maddr & 0xff000001)) {
      /* BUS ERROR or ADDRESS ERROR*/
      
      fprintf(stderr, "BUS ERROR! Bad address! 0x%08x\n", maddr);

      if ((maddr = raise_exception(regs,
				   (U16 *)memory,
				   VEC_BUS_ERROR + (maddr & 1))) && 0xff000001) {
	fprintf(stderr,
		"Double ADDRESS/BUS ERROR!\n"
		"New address = 0x%08x\n",
		maddr);
	abort();
      }
      return;
    }

#ifdef DEBUG
    if ((regs->sr ^ oldsr) & 0x2000) {
      if (regs->sr & 0x2000) {
	printf("Entering Supervisor mode. SR:0x%04x SSP:0x%08x USP:0x%08x\n",
	       regs->sr, regs->ssp, regs->usp);
      } else {
	printf("Leaving Supervisor mode. SR:0x%04x SSP:0x%08x USP:0x%08x\n",
	       regs->sr, regs->ssp, regs->usp);
      }
      oldsr = regs->sr;
    }
#endif /* DEBUG */
    if ((!segment) || (segment->maddr > maddr) || (segment->mend <= maddr)) {
      segment = find_seg(code_tree, maddr);
#ifdef DEBUG
      if (debuglevel & DL_RUNTIME_TRACE) {
	if (segment) {
	  fprintf(stdout,
		  "0x%08x called from 0x%08x, New segment [0x%08x - 0x%08x]\r",
		  maddr, regs->pc, segment->maddr, segment->mend);
	} else {
	  fprintf(stdout, "0x%08x called from 0x%08x\r", maddr, regs->pc);
	}
      }
#endif /* DEBUG */
    }

    if ((!segment) ||
	(!(ci = find_ci(&segment->codeinfo, maddr)))) {
      U32 mend;
      ci = new_codeinfo(maddr);

#ifdef DEBUG
      if (debuglevel & DL_RUNTIME_TRACE) {
	fprintf(stdout, "Compiling 0x%08x, called from 0x%08x...\n",
		maddr, regs->pc);
      }
#endif /* DEBUG */
      if (debuglevel & DL_STATISTICS) {
	comp_time = -gethrtime();
      }
      mend = compile(ci);
      
      if (!segment) {
#ifdef DEBUG
	if (debuglevel & DL_COMPILER_VERBOSE) {
	  fprintf(stdout, "Creating new segment, maddr: %08x, end: %08x\n",
		  maddr, mend);
	}
#endif /* DEBUG */
	segment = insert_seg(&code_tree, maddr, mend);
	if (debuglevel & DL_STATISTICS) {
	  comp_time += gethrtime();
	  comp_instr += ci->num_opcodes;
	
	  if (comp_instr > STATISTICS_THRESHOLD) {
	    run_time += gethrtime();
	    printf("Compiling: %d instructions in %lld ns (%lld ns/instruction)\n",
		   comp_instr, comp_time, comp_time/comp_instr);
	    comp_instr = 0;
	    comp_time = 0;
	    run_time -= gethrtime();
	  }
	}
	if (!segment) {
	  /* BUS ERROR? */
	  fprintf(stderr, "Out of memory?\n");
	  abort();
	}
      } else if (mend != segment->mend) {
	fprintf(stderr, "Strange ending, orig:%08x, new:%08x\n", segment->mend, mend);
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
    if (debuglevel & DL_STATISTICS) {
      run_instr += ci->num_opcodes;
      if (run_instr > STATISTICS_THRESHOLD) {
	run_time += gethrtime();
	printf("Runtime: %d instructions in %lld ns (%lld ns/instruction)\n",
	       run_instr, run_time, run_time/run_instr);
	run_instr = 0;
	run_time = -gethrtime();
      }
    }
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
  U32 start_addr = ((U32 *)(memory + 0x00f80000))[1];
  regs.a7 = regs.ssp = ((U32 *)(memory + 0x00f80000))[0];
  regs.sr = 0x00002700;

  start_register_dump(&regs);

#ifdef DEBUG
  fprintf(stdout, "Starting CPU at 0x%08x\n", start_addr);
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

volatile void Usage(char *arg0, char *romdump)
{
  fprintf(stderr,
	  "Usage:\n"
	  "\t%s [flags] [romdump]\n\n"
	  "Available flags are:\n"
	  "-C\tVerbose compiler\n"
	  "-D\tDisassemble\n"
	  "-s\tRuntime statistics\n"
	  "-t\tRuntime trace\n"
	  "\n"
	  "+O\tTurn off SR optimization\n"
	  "\nThe current romdump is \"%s\".\n",
	  arg0, romdump);
  exit(1);
}

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
      case 's':
	debuglevel |= DL_STATISTICS;
	break;
      case 't':
	debuglevel |= DL_RUNTIME_TRACE;
	break;
      default:
	Usage(argv[0], romdump);
	break;
      }
    } else if (argv[i][0] == '+') {
      switch(argv[i][1]) {
      case 'O':
	debuglevel |= DL_NO_SR_OPTIMIZATION;
	break;
      default:
	Usage(argv[0], romdump);
	break;
      }
    } else {
      romdump = argv[i];
    }
  }

  thr_setconcurrency(128);

  if ((devzero = open("/dev/zero", O_RDONLY)) >= 0) {
    if ((memory = (U8 *)mmap((caddr_t)NULL, 16*1024*1024, PROT_READ|PROT_WRITE,
				MAP_PRIVATE, devzero, 0))) {
      if ((romfd = open(romdump, O_RDONLY)) >= 0) {
	if ((rommem = (U8 *)mmap((caddr_t)(memory + 0x00f80000), 512*1024,
				    PROT_READ, MAP_SHARED|MAP_FIXED, romfd, 0))) {
	  if (rommem == memory + 0x00f80000) {

	    init_interrupt();

	    start_info_window();

	    init_hardware();

#ifdef DEBUG
	    printf("Kickstart V%d.%d\n"
		   "exec.library V%d.%d\n",
		   ((U16 *)(memory + 0x00f8000c))[0],
		   ((U16 *)(memory + 0x00f8000c))[1],
		   ((U16 *)(memory + 0x00f8000c))[2],
		   ((U16 *)(memory + 0x00f8000c))[3]);
	    if (memory[0x00f80018]) {
	      /* Old style kickstart */
	      unsigned char *cr;
	      cr = (unsigned char *)strchr((char *)memory + 0x00f80018, '\0') + 1;
	      while (!*cr) {
		cr++;
	      }
	      if (cr[0] == 255) {
		cr = (unsigned char *)(((U32)(cr + 4)) & ~3);
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

