/*
 * $Id: compglue.c,v 1.14 1996/08/11 17:36:12 grubba Exp $
 *
 * Help functions for the M68000 to Sparc compiler.
 *
 * $Log: compglue.c,v $
 * Revision 1.13  1996/08/11 14:48:52  grubba
 * Added option to turn off SR optimization.
 *
 * Revision 1.12  1996/08/10 18:19:11  grubba
 * Now with enabled SR optimization!
 *
 * Revision 1.11  1996/08/04 17:21:01  grubba
 * Now skips to next opcode in SR scan.
 *
 * Revision 1.10  1996/08/04 16:25:11  grubba
 * Now does the SR scans.
 * Does not yet skip past the opcode arguments in the SR scan.
 * Split the tab_entry for the sr_magic into sr_magic_*.
 *
 * Revision 1.9  1996/07/21 17:39:17  grubba
 * Now does instruction flushes correctly (I think).
 * Instruction flushing moved to peephole.c .
 *
 * Revision 1.8  1996/07/17 16:01:22  grubba
 * Changed from {U,}{LONG,WORD,BYTE} to [SU]{8,16,32}.
 * Hopefully all places got patched.
 *
 * Revision 1.7  1996/07/17 00:19:55  grubba
 * Minor changes.
 *
 * Revision 1.6  1996/07/14 21:44:17  grubba
 * Added support for adding hardware dynamically.
 * Added CIAA time of day clock (50Hz).
 * Moved some debug output from stderr to stdout.
 *
 * Revision 1.5  1996/07/13 19:32:25  grubba
 * Now defaults to very little debuginfo.
 * Added (un|set)patch().
 * Patches added to MakeLibrary(), MakeFunctions(), Abort() and AddLibrary().
 *
 * Revision 1.4  1996/07/10 20:22:08  grubba
 * Fixed bug where "or %lo(val), %g4, %ea" was generated instead of "or %lo(val), %g0, %ea".
 *
 * Revision 1.3  1996/07/08 21:20:44  grubba
 * *** empty log message ***
 *
 * Revision 1.2  1996/07/01 19:16:49  grubba
 * Implemented ASL and ASR.
 * Changed semantics for new_codeinfo(), it doesn't allocate space for the code.
 * Added PeepHoleOptimize(). At the moment it just mallocs and copies the code.
 * Removed some warnings.
 *
 * Revision 1.1.1.1  1996/06/30 23:51:53  grubba
 * Entry into CVS
 *
 * Revision 1.6  1996/06/30 23:05:44  grubba
 * Fixed some bugs.
 * Now calls abort(), when encountering unimplemented opcodes.
 *
 * Revision 1.5  1996/06/20 22:13:02  grubba
 * Loop optimization added.
 * A bug in the debug printout for long immediate values fixed.
 *
 * Revision 1.4  1996/06/19 11:08:25  grubba
 * Fixed several bugs.
 * Added support for hardware.
 * Added support for pushing next PC (needed for BSR and JSR).
 *
 * Revision 1.3  1996/06/03 14:47:25  grubba
 * Version before changes to the flags field.
 *
 * Revision 1.2  1996/06/01 09:31:29  grubba
 * Major changes.
 * Now generates output split into several files.
 * Now generates gasp assembler preprocessor output.
 * Now uses templates to build instructions.
 *
 * Revision 1.1  1996/05/14 21:40:30  grubba
 * Initial revision
 *
 *
 */

/*
 * Includes
 */

#include <stdio.h>

#include "types.h"

#include "recomp.h"
#include "m68k.h"
#include "sparc.h"
#include "codeinfo.h"
#include "compiler.h"
#include "peephole.h"
#include "opcodes.h"
#include "templates/glue.h"

/*
 * Definitions
 */

#ifdef DEBUG
#define DPRINTF(x)	do { if (debuglevel & DL_COMPILER_VERBOSE) { printf x; fflush(stdout); } } while (0)
#else
#define DPRINTF(x)
#endif

/*
 * Globals
 */

static U32 ScratchPad[0x00080000];	/* 2Mb */

static U16 SR_Unmagic[32] = {
  0x0000, 0x0001, 0x0002, 0x0004,
  0x0008, 0x0010, 0x0020, 0x0040,
  0x0080, 0x0100, 0x0200, 0x0400,
  0x0800, 0x1000, 0x2000, 0x4000,
  0x0000, 0x0001, 0x0003, 0x0007,
  0x000f, 0x001f, 0x003f, 0x007f,
  0x00ff, 0x01ff, 0x03ff, 0x07ff,
  0x0fff, 0x1fff, 0x3fff, 0x7fff
};

/*
 * Functions
 */

__inline__ void copy_template(U32 **code, U32 *template)
{
  while (*template) {
    *((*code)++) = *(template++);
  }
}

void emit_exception(U32 **code, U32 pc, U8 vec)
{
  S_MOVI(vec, S_O2);
  copy_template(code, s_raise_exception);
}

/* Clobber a longword */
void emit_clobber(U32 **code)
{
  copy_template(code, s_clobber);
}

/* Clobber a short */
void emit_clobber_short(U32 **code)
{
  copy_template(code, s_clobber_short);
}

void emit_clobber_byte(U32 **code, U8 sdst, U8 sval)
{
  copy_template(code, s_clobber_byte);
}

void break_me(void)
{
  if (debuglevel & DL_COMPILER_VERBOSE) {
    fprintf(stdout, "Leaving compiler\n");
  }
}

/*
 * Help functions
 */

void __inline__ skip_ea(U32 *pc, U32 flags)
{
  if ((flags & 0x0030) && ((flags & 0x003f) != 0x003c)) {
    /* Not Register direct or immediate */
    if ((flags & 0x003f) == 0x39) {
      /* (d32).L */
      (*pc) += 2;
    } else if ((flags & 0x0038) > 0x20) {
      /* (d8, An, Xn), (d8, PC, Xn) */
      /* (d16,An), (d16).W, (d16,PC) */
      (*pc)++;
    }
  }
}

void calc_ea(U32 **code, U32 *pc, U32 flags, U32 oldpc)
{
  U16 *mem = (U16 *)memory;

  if ((flags & 0x0030) && ((flags & 0x003f) != 0x003c)) {
    /* Not Register direct or immediate */
    if ((flags & 0x003f) == 0x39) {
      /* (d32).L */
      U32 val = mem[(*pc)++];
      val = (val<<16) | mem[(*pc)++];
      DPRINTF(("(0x%08x).L ", val));
      if ((val & 0xfffff000) && ((val & 0xfffff000) != 0xfffff000)) {
	/* sethi %hi(val), %ea */
	*((*code)++) = 0x2b000000 | (val >> 10);
	if (val & 0x3ff) {
	  /* or %lo(val), %ea, %ea */
	  *((*code)++) = 0xaa156000 | (val & 0xfff);
	}
      } else {
	/* or %lo(val), %g0, %ea */
	*((*code)++) = 0xaa102000 | (val & 0x1fff);
      }
    } else if (((flags & 0x0038) == 0x30) ||
	       ((flags & 0x003f) == 0x3b)) {
      /* (d8, An, Xn), (d8, PC, Xn) */
      U32 val = mem[(*pc)++];
#ifdef DEBUG
      if ((flags & 0x38) == 0x30) {
	if (val & 0x80) {
	  DPRINTF(("(-0x%02x, A%d, ", (0x100 - (val & 0xff)), flags & 0x07));
	} else {
	  DPRINTF(("(0x%02x, A%d, ", val & 0xff, flags & 0x07));
	}
      } else {
	if (val & 0x80) {
	  DPRINTF(("(0x%08x, PC, ", oldpc + (val & 0xff) - 0x100));
	} else {
	  DPRINTF(("(0x%08x, PC, ", oldpc + (val & 0xff)));
	}
      }
      if (val & 0x8000) {
	DPRINTF(("A%d", (val & 0x7000)>>12));
      } else {
	DPRINTF(("D%d", (val & 0x7000)>>12));
      }
      if (val & 0x0800) {
	DPRINTF((".L"));
      } else {
	DPRINTF((".W"));
      }
      if (val & 0x0600) {
	DPRINTF(("*%d)", 0x1 << ((val & 0x0600)>>9)));
      } else {
	DPRINTF((")"));
      }
#endif /* DEBUG */
	
      copy_template(code, scale_reg_tab[val >> 9]);

      if ((flags & 0x003f) == 0x003b) {
	/* (d8, PC, Xn) */
	val += 2;	/* Compensate for instruction */
      }
      /* add (val & 0xff), %ea, %ea */
      val &= 0xff;
      if (val & 0x80) {
	/* Negative */
	val |= 0x1f80;
      }
      *((*code)++) = 0xaa056000 | val;
    } else if ((flags & 0x0038) > 0x20) {
      /* (d16,An), (d16).W, (d16,PC) */
      U32 val = mem[(*pc)++];

#ifdef DEBUG
      if ((flags & 0x0038) == 0x0028) {
	if (val & 0x8000) {
	  DPRINTF(("(-0x%04x, A%d)", 0x00010000 - val, flags & 0x0007));
	} else {
	  DPRINTF(("(0x%04x, A%d)", val, flags & 0x0007));
	}
      } else if ((flags & 0x003f) == 0x003a) {
	if (val & 0x8000) {
	  DPRINTF(("(0x%08x, PC)", oldpc + val - 0xfffe));
	} else {
	  DPRINTF(("(0x%08x, PC)", oldpc + val + 2));
	}
      } else {
	if (val & 0x8000) {
	  DPRINTF(("(-0x%04x).W", 0x00010000 - val));
	} else {
	  DPRINTF(("(0x%04x).W", val));
	}
      }
#endif /* DEBUG */
    
      if ((flags & 0x003f) == 0x003a) {
	/* (d16, PC) */
	val += 2;	/* Compensate for instruction */
      }

      if ((val & 0xf000) && ((val & 0xf000) != 0xf000)) {
	if (val & 0x8000) {
	  val |= 0xffff0000;
	}
	if (val & 0x3ff) {
	  /* sethi %hi(val), %ea */
	  *((*code)++) = 0x2b000000 | (val >> 10);
	  /* or %lo(val), %ea, %ea */
	  *((*code)++) = 0xaa156000 | (val & 0xfff);
	} else {
	  /* sethi %hi(val), %ea */
	  *((*code)++) = 0x2b000000 | (val >> 10);
	}
      } else {
	/* mov (val & 0x1fff), %ea */
	*((*code)++) = 0xaa102000 | (val & 0x1fff);
      }
    }

#ifdef DEBUG
    else {
      switch(flags & 0x38) {
      case 0x10:
	DPRINTF(("(A%d)", flags & 0x07));
	break;
      case 0x18:
	DPRINTF(("(A%d)+", flags & 0x07));
	break;
      case 0x20:
	DPRINTF(("-(A%d)", flags & 0x07));
	break;
      }
    }
#endif /* DEBUG */
    copy_template(code, ea_tab[flags & 0x00ff]);
  }
}


/*
 * Main compilation function
 */

U32 compile(struct code_info *ci)
{
  U32 pc = ci->maddr>>1;
  U32 *code_start;
  U32 *code;
  U32 oldpc;
  U32 flags;
  U32 regs = 0;
  U16 *mem = (U16 *)memory;
  U16 opcode;

  /* SR scan variables */
  U32 sr_mask = 0x00000000;
  U32 *sr_magic_start = ScratchPad;
  U32 *sr_magic_pos = ScratchPad;

#ifndef NDEBUG
  /* Consistency check variables */
  U32 end_pc;
  int sr_num_opcodes = 0;
  int comp_num_opcodes = 0;
#endif /* NDEBUG */

  /* SR optimization */
  if (debuglevel & DL_NO_SR_OPTIMIZATION) {
    code_start = ScratchPad;
  } else {

    /* Forward SR scan pass */

    do {
#ifndef NDEBUG
      sr_num_opcodes++;
#endif /* NEBUG */
      opcode = mem[pc++];
      flags = compiler_tab[opcode].flags;

      *(sr_magic_pos++) = (SR_Unmagic[compiler_tab[opcode].sr_magic_needed]<<16) |
	                  (SR_Unmagic[compiler_tab[opcode].sr_magic_changed]);

      /* Skip to next opcode */

      /* Is this a MOVEM? -- if so skip the register mask */
      if (flags & TEF_MOVEM) {
	pc++;
      }

      /* Fix the source operand */

      if (flags & TEF_SRC) {
	if (!(flags & TEF_SRC_MOVEM)) {
	  U32 newflags = ((flags & (TEF_SRC_MASK | TEF_SRC_SIZE))>>TEF_SRC_SHIFT);
	  
	  if ((flags & TEF_MOVEM) && (flags & TEF_WRITE_BACK)) {
	    skip_ea(&pc, ((opcode & 0x0007) | 0x0010));
	  } else {
	    /* Get the effective address */
	    skip_ea(&pc, newflags);
	  }

	  if (flags & TEF_SRC_LOAD) {
	    if ((flags & TEF_SRC_MASK) == 0x7800) {
	      /* Immediate operand */
	      switch (flags & TEF_SRC_SIZE) {
	      case TEF_SRC_BYTE:
	      case TEF_SRC_WORD:
		pc++;
		break;
	      case TEF_SRC_LONG:
		pc += 2;
		break;
	      default:
		fprintf(stderr, "Error in immediate operand for opcode 0x%04x, 0x%08x!\n", opcode, flags);
		abort();
		break;
	      }
	    }
	  }
	}
      }

      if (flags & TEF_DST) {
	if (!(flags & TEF_DST_MOVEM)) {
	  if ((flags & TEF_MOVEM) && (flags & TEF_WRITE_BACK)) {
	    skip_ea(&pc, ((opcode & 0x0007) | 0x0010));
	  } else {
	    skip_ea(&pc, flags & 0x00ff);
	  }
	  
	  if (flags & TEF_DST_LOAD) {
	    if ((flags & TEF_DST_MASK) == 0x003c) {
	      /* Immediate operand */
	      switch (flags & TEF_DST_SIZE) {
	      case TEF_DST_BYTE:
	      case TEF_DST_WORD:
		pc++;
		break;
	      case TEF_DST_LONG:
		pc += 2;
		break;
	      default:
		fprintf(stderr, "Error in immediate operand for opcode 0x%04x, 0x%08x!\n", opcode, flags);
		abort();
		break;
	      }
	    }
	  }
	}
      }
      
      /* End skip to next opcode */

    } while (!(flags & TEF_TERMINATE));

#ifndef NDEBUG
    end_pc = pc;
#endif /* NDEBUG */

    code_start = sr_magic_pos;

  /* Reverse SR scan pass */

    DPRINTF(("Needed : Changed - Needed : Needed : Magic\n"));

    while ((sr_magic_pos--) != sr_magic_start) {
      U32 sr_magic = (*sr_magic_pos);

      DPRINTF(("0x%04x :  0x%04x - 0x%04x : 0x%04x : 0x%04x\n",
	       sr_mask, (sr_magic & 0xffff), (sr_magic >> 16),
	       ((sr_mask & ~sr_magic) | (sr_magic>>16)),
	       (sr_magic & sr_mask)));

      *sr_magic_pos = sr_magic & sr_mask;
      sr_mask &= ~sr_magic;
      sr_mask |= (sr_magic>>16);
    }
    sr_magic_pos++;	/* Compensation for loop going past start */
  }

  /* SR prescan finished */

  code = code_start;
  pc = ci->maddr>>1;

  copy_template(&code, s_pre_amble);

  do {
#ifndef NDEBUG
    comp_num_opcodes++;
#endif /* NDEBUG */

    oldpc = pc;
    opcode = mem[pc++];

    DPRINTF(("\nPC:%08x\tOpcode:%04x\tMnemonic:%s\n"
	     "\tFlags:%08x\tTemplate:%08x\n",
	     (pc - 1)<<1, opcode, compiler_tab[opcode].mnemonic,
	     compiler_tab[opcode].flags, (U32)compiler_tab[opcode].template));
#ifdef DEBUG
    {
      extern U32 opcode_4afc[];
      if (compiler_tab[opcode].template == opcode_4afc) {
	printf("!! ILLEGAL OPCODE (0x%04x, \"%s\")!!\n",
	       opcode, compiler_tab[opcode].mnemonic);
      }
    }
#endif /* DEBUG */

    flags = compiler_tab[opcode].flags;

#ifdef DEBUG
    switch (flags & TEF_DST_SIZE) {
    case TEF_DST_BYTE:
      DPRINTF(("%s.B\t", compiler_tab[opcode].mnemonic));
      break;
    case TEF_DST_WORD:
      DPRINTF(("%s.W\t", compiler_tab[opcode].mnemonic));
      break;
    case TEF_DST_LONG:
      DPRINTF(("%s.L\t", compiler_tab[opcode].mnemonic));
      break;
    default:
      DPRINTF(("%s\t", compiler_tab[opcode].mnemonic));
      break;
    }
#endif /* DEBUG */

    if (flags & TEF_SUPERVISOR) {
      DPRINTF(("Supervisor mode only\n"));
      copy_template(&code, s_supervisor);
    }

    /* Is this a MOVEM? -- if so fetch the register mask */
    if (flags & TEF_MOVEM) {
      regs = mem[pc++];
    }

    /* Fix the source operand */

    if (flags & TEF_SRC) {
      if (flags & TEF_SRC_MOVEM) {
#ifdef DEBUG
	{
	  int i = 0;
	  int d = 1;
	  U32 regs2;

	  if ((flags & 0x0038) == 0x0020) {
	    /* Other direction in predecrement mode */
	    d = -1;
	    i = 15;
	  }

	  for (regs2 = regs;regs2;regs2>>=1, i+=d) {
	    if (regs2 & 1) {
	      if (i < 8) {
		DPRINTF(("D%d", i));
	      } else {
		DPRINTF(("A%d", i & 0x07));
	      }
	      if (regs & ~1) {
		DPRINTF(("/"));
	      }
	    }
	  }
	}
#endif /* DEBUG */
      } else {
	U32 newflags = ((flags & (TEF_SRC_MASK | TEF_SRC_SIZE))>>TEF_SRC_SHIFT);

	if ((flags & TEF_MOVEM) && (flags & TEF_WRITE_BACK)) {
	  calc_ea(&code, &pc, ((opcode & 0x0007) | 0x0010), oldpc*2);
	} else {
	  /* Get the effective address */
	  calc_ea(&code, &pc, newflags, oldpc*2);
	}

	if (flags & TEF_SRC_LOAD) {
	  if ((flags & TEF_SRC_MASK) == 0x7800) {
	    /* Immediate operand */
	    switch (flags & TEF_SRC_SIZE) {
	    case TEF_SRC_BYTE:
	      {
		U16 immop = mem[pc++] & 0xff;

		/* mov immop, %acc1 */
		if (immop & 0x80) {
		  /* Negative */
		  DPRINTF(("#-0x%02x.B", 0x80 - immop));
		  *(code++) = 0xa6103f80 | (immop & 0x7f);
		} else {
		  /* Positive */
		  DPRINTF(("#0x%02x.B", immop));
		  *(code++) = 0xa6102000 | (immop & 0x7f);
		}
	      }
	    break;
	    case TEF_SRC_WORD:
	      {
		U16 immop = mem[pc++];
		if (immop & 0x8000) {
		  /* Negative */
		  DPRINTF(("#-0x%04x.W", (0x8000 - immop) & 0x7fff));
		  if ((immop & 0x7000) == 0x7000) {
		    /* mov immop, %acc1 */
		    *(code++) = 0xa6103000 | (immop & 0x0fff);
		  } else {
		    /* sethi %hi(immop), %acc1 */
		    *(code++) = 0x273fffc0 | (immop >> 10);
		    /* or %lo(immop), %acc1, %acc1 */
		    *(code++) = 0xa614e000 | (immop & 0x0fff);
		  }
		} else {
		  /* Positive */
		  DPRINTF(("#0x%04x.W", immop));
		  if (immop & 0x7000) {
		    /* sethi %hi(immop), %acc1 */
		    *(code++) = 0x27000000 | (immop >> 10);
		    /* or %lo(immop), %acc1, %acc1 */
		    *(code++) = 0xa614e000 | (immop & 0x0fff);
		  } else {
		    /* mov immop, %acc1 */
		    *(code++) = 0xa6102000 | (immop & 0x0fff);
		  }
		}
	      }
	    break;
	    case TEF_SRC_LONG:
	      {
		U32 immop = mem[pc++];
		immop = (immop << 0x10) | mem[pc++];
		DPRINTF(("#0x%08x.L", immop));
		/* sethi %hi(immop), %acc1 */
		*(code++) = 0x27000000 | (immop >> 10);
		/* or %lo(immop), %acc1, %acc1 */
		*(code++) = 0xa614e000 | (immop & 0x0fff);
	      }
	    break;
	    default:
	      fprintf(stderr, "Error in immediate operand for opcode 0x%04x, 0x%08x!\n", opcode, flags);
	      abort();
	      break;
	    }
	  } else {
#ifdef DEBUG
	    if (!(newflags & 0x0030)) {
	      DPRINTF(("%c%d", ((newflags & 0x0008)?'A':'D'), newflags & 0x0007));
	    }
#endif /* DEBUG */
	    copy_template(&code, load_eo1_tab[newflags]);
	  }
	}
      }
    }
#ifdef DEBUG
    if ((flags & TEF_SRC) && (flags & TEF_DST)) {
      DPRINTF((", "));
    }
#endif /* DEBUG */
    
    if (flags & TEF_DST) {
      if (flags & TEF_DST_MOVEM) {
#ifdef DEBUG
	{
	  int i = 0;
	  U32 regs2;
	  for (regs2 = regs;regs2;regs2>>=1, i++) {
	    if (regs2 & 1) {
	      if (i < 8) {
		DPRINTF(("D%d", i));
	      } else {
		DPRINTF(("A%d", i & 0x07));
	      }
	      if (regs & ~1) {
		DPRINTF(("/"));
	      }
	    }
	  }
	}
#endif /* DEBUG */
      } else {
	if ((flags & TEF_MOVEM) && (flags & TEF_WRITE_BACK)) {
	  calc_ea(&code, &pc, ((opcode & 0x0007) | 0x0010), oldpc*2);
	} else {
	  calc_ea(&code, &pc, flags & 0x00ff, oldpc*2);
	}

	if (flags & TEF_DST_LOAD) {
	  if ((flags & TEF_DST_MASK) == 0x003c) {
	    /* Immediate operand */
	    switch (flags & TEF_DST_SIZE) {
	    case TEF_DST_BYTE:
	      {
		U16 immop = mem[pc++] & 0xff;

		/* mov immop, %acc0 */
		if (immop & 0x80) {
		  /* Negative */
		  DPRINTF(("#-0x%02x.B, ", 0x80 - immop));
		  *(code++) = 0xa8103f80 | (immop & 0x7f);
		} else {
		  /* Positive */
		  DPRINTF(("#0x%02x.B, ", immop));
		  *(code++) = 0xa8102000 | (immop & 0x7f);
		}
	      }
	    break;
	    case TEF_DST_WORD:
	      {
		U16 immop = mem[pc++];
		if (immop & 0x8000) {
		  /* Negative */
		  DPRINTF(("#-0x%04x.W, ", 0x8000 - immop));
		  if ((immop & 0x7000) == 0x7000) {
		    /* mov immop, %acc0 */
		    *(code++) = 0xa8103000 | (immop & 0x0fff);
		  } else {
		    /* sethi %hi(immop), %acc0 */
		    *(code++) = 0x293fffc0 | (immop >> 10);
		    /* or %lo(immop), %acc0, %acc0 */
		    *(code++) = 0xa8152000 | (immop & 0x0fff);
		  }
		} else {
		  /* Positive */
		  DPRINTF(("#0x%04x.W, ", immop));
		  if (immop & 0x7000) {
		    /* sethi %hi(immop), %acc0 */
		    *(code++) = 0x29000000 | (immop >> 10);
		    /* or %lo(immop), %acc0, %acc0 */
		    *(code++) = 0xa8152000 | (immop & 0x0fff);
		  } else {
		    /* mov immop, %acc0 */
		    *(code++) = 0xa8102000 | (immop & 0x7f);
		  }
		}
	      }
	    break;
	    case TEF_DST_LONG:
	      {
		U32 immop = mem[pc++];
		DPRINTF(("#0x%08x.L, ", immop));
		immop = (immop << 0x10) | mem[pc++];
		/* sethi %hi(immop), %acc0 */
		*(code++) = 0x29000000 | (immop >> 10);
		/* or %lo(immop), %acc0, %acc0 */
		*(code++) = 0xa8152000 | (immop & 0x0fff);
	      }
	    break;
	    default:
	      fprintf(stderr, "Error in immediate operand for opcode 0x%04x, 0x%08x!\n", opcode, flags);
	      abort();
	      break;
	    }
	  } else {
#ifdef DEBUG
	    if (!(flags & 0x0030)) {
	      DPRINTF(("%c%d", ((flags & 0x0008)?'A':'D'), flags & 0x0007));
	    }
#endif /* DEBUG */
	    copy_template(&code, load_eo0_tab[flags & 0x00ff]);
	  }
	}
      }
    }

    if (flags & TEF_PUSH_PC) {
      /* Used by BSR and JSR */
      /* add size, %pc, %o0 */
      *(code++) = 0x9005e000 + 2*(pc - oldpc);
      copy_template(&code, s_push_o0);
    }

    /* Make opcode */
    if (flags & TEF_MOVEM) {
      /* ld	[ %vecs + xxxx ], %acc0 */
      switch (flags & (TEF_MOVEM | 0x0080)) {
      case 0x10000000:	/* SRC_MOVEM WORD */
	*(code++) = 0xe806a000 | 0x0010;	/* STORE_WORD */
	break;
      case 0x10000080:	/* SRC_MOVEM LONG */
	*(code++) = 0xe806a000 | 0x000c;	/* STORE_LONG */
	break;
      case 0x20000000:	/* DST_MOVEM WORD */
	*(code++) = 0xe806a000 | 0x0004;	/* LOAD_WORD */
	break;
      case 0x20000080:	/* DST_MOVEM LONG */
	*(code++) = 0xe806a000 | 0x0000;	/* LOAD_LONG */
	break;
      }
      if (flags & TEF_DST_MOVEM) {
	int regno = 0;
	int moffset = 0;
	/* Memory -> Regs */
	while (regs) {
	  if (regs & 1) {
	    /* call	%%acc0 */
	    *(code++) = 0x9fc50000;
	    /* add	moffset, %%ea, %%o0 */
	    *(code++) = 0x90056000 | moffset;
	    /* st	%%o0, [ %%regs + regno ] */
	    *(code++) = 0xd0262000 | regno;

	    if (flags & 0x0080) {
	      moffset += 4;
	    } else {
	      moffset += 2;
	    }
	  }
	  regs >>= 1;
	  regno += 4;
	}
	if (flags & TEF_WRITE_BACK) {
	  /* add	moffset, %ea, %ea */
	  *(code++) = 0xaa056000 | moffset;
	}
      } else {
	int regno = 0;
	int moffset = 0;
	if (flags & TEF_WRITE_BACK) {
	  /* Reverse the bits */
	  U32 newregs = 0;
	  int i;
	  for (i = 0; i < 16; i++) {
	    newregs <<= 1;
	    if (regs & 1) {
	      newregs |= 1;
	      if (flags & 0x80) {
		moffset -= 4;
	      } else {
		moffset -= 2;
	      }
	    }
	    regs >>= 1;
	  }
	  regs = newregs;
	  /* add	moffset, %ea, %ea */
	  *(code++) = 0xaa056000 | (moffset & 0x1fff);
	  moffset = 0;
	}
	/* Regs -> Memory*/
	while (regs) {
	  if (regs & 1) {
	    /* ld	[ %%regs + regno ], %%o1 */
	    *(code++) = 0xd2062000 | regno;
	    /* call	%%acc0 */
	    *(code++) = 0x9fc50000;
	    /* add	moffset, %%ea, %%o0 */
	    *(code++) = 0x90056000 | moffset;

	    if (flags & 0x0080) {
	      moffset += 4;
	    } else {
	      moffset += 2;
	    }
	  }
	  regs >>= 1;
	  regno += 4;
	}
      }
    } else {
      copy_template(&code, compiler_tab[opcode].template);
    }

    /* SR post instruction fixup */

    if (debuglevel & DL_NO_SR_OPTIMIZATION) {
      if (compiler_tab[opcode].sr_magic_reserved) {
	copy_template(&code, s_sr_post_tab[compiler_tab[opcode].sr_magic_reserved]);
      }
    } else {
      if ((sr_mask = *(sr_magic_pos++))) {
	int post_sr_kind = compiler_tab[opcode].sr_magic_reserved;

#ifndef NDEBUG
	if ((flags & TEF_FIX_SR) && (post_sr_kind != 1)) {
	  printf("Opcode \"%s\": Bad post_sr_kind (0x%02x). Forceing type 0x01!\n",
		 compiler_tab[opcode].mnemonic, post_sr_kind);
	  post_sr_kind = 1;
	}
#endif /* NDEBUG */
	DPRINTF(("<%04x>", sr_mask));
	if (post_sr_kind) {
	  copy_template(&code, s_sr_post_tab[post_sr_kind]);
	}
#ifdef DEBUG
      } else if (compiler_tab[opcode].sr_magic_reserved) {
	DPRINTF(("<NOCC>"));
#endif
      }
    }

    if (flags & TEF_WRITE_BACK) {
      if (flags & TEF_MOVEM) {
	/* st	%ea, [ %regs + regno ] */
	*(code++) = 0xea262000 | ((0x0008 | (opcode & 0x0007))<<2);
      } else {
#ifdef DEBUG
	if ((!(flags & TEF_DST_LOAD)) && (!(flags & 0x0030))) {
	  DPRINTF(("%c%d", ((flags & 0x0008)?'A':'D'), (flags & 0x0007)));
	}
#endif /* DEBUG */
	copy_template(&code, write_back_tab[flags & 0x00ff]);
      }
    }

    DPRINTF(("\n"));
    
    if (!(flags & TEF_TERMINATE)) {
      /* Adjust the programcounter (%l7) */
      *(code++) = 0xae05e000 + 2*(pc - oldpc);
    }

  } while (!(flags & TEF_TERMINATE));

#ifndef NDEBUG
  if (!(debuglevel & DL_NO_SR_OPTIMIZATION)) {
    if (end_pc != pc) {
      printf("!! END_PC (0x%08x) from SR scan differs from from compile scan (0x%08x) !!\n",
	     end_pc, pc);
      abort();
    }
    if (sr_num_opcodes != comp_num_opcodes) {
      printf("!! NUM_OPCODES from SR scan pass (%d) differs from from compile scan (%d) !!\n",
	     sr_num_opcodes, comp_num_opcodes);
      abort();
    }
  }
#endif /* NDEBUG */

  DPRINTF(("Compiled %d opcodes\n", comp_num_opcodes));

  ci->num_opcodes = comp_num_opcodes;

#ifdef DEBUG
  {
    extern U32 opcode_4afc[];
    if (compiler_tab[opcode].template == opcode_4afc) {
      printf("!! ILLEGAL OPCODE !!\n");
      abort();
    }
  }
#endif /* DEBUG */

  PeepHoleOptimize(ci, code_start, code);
  return(pc<<1);
}

