/*
 * $Id: compglue.c,v 1.4 1996/07/10 20:22:08 grubba Exp $
 *
 * Help functions for the M68000 to Sparc compiler.
 *
 * $Log: compglue.c,v $
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
#define DPRINTF(x)	do { printf x; fflush(stdout); } while (0)
#else
#define DPRINTF(x)
#endif

/*
 * Globals
 */

static ULONG ScratchPad[0x00080000];	/* 2Mb */

/*
 * Functions
 */

inline void copy_template(ULONG **code, ULONG *template)
{
  while (*template) {
    *((*code)++) = *(template++);
  }
}

void emit_exception(ULONG **code, ULONG pc, UBYTE vec)
{
  S_MOVI(vec, S_O2);
  copy_template(code, s_raise_exception);
}

/* Clobber a longword */
void emit_clobber(ULONG **code)
{
  copy_template(code, s_clobber);
}

/* Clobber a short */
void emit_clobber_short(ULONG **code)
{
  copy_template(code, s_clobber_short);
}

void emit_clobber_byte(ULONG **code, UBYTE sdst, UBYTE sval)
{
  copy_template(code, s_clobber_byte);
}

void break_me(void)
{
  fprintf(stdout, "Leaving compiler\n");
}

/*
 * Help functions
 */


void calc_ea(ULONG **code, ULONG *pc, ULONG flags, ULONG oldpc)
{
  USHORT *mem = (USHORT *)memory;

  if ((flags & 0x0030) && ((flags & 0x003f) != 0x003c)) {
    /* Not Register direct or immediate */
    if ((flags & 0x003f) == 0x39) {
      /* (d32).L */
      ULONG val = mem[(*pc)++];
      val = (val<<16) | mem[(*pc)++];
      DPRINTF(("(0x%08lx).L ", val));
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
      ULONG val = mem[(*pc)++];
#ifdef DEBUG
      if ((flags & 0x38) == 0x30) {
	if (val & 0x80) {
	  DPRINTF(("(-0x%02lx, A%ld, ", (0x100 - (val & 0xff)), flags & 0x07));
	} else {
	  DPRINTF(("(0x%02lx, A%ld, ", val & 0xff, flags & 0x07));
	}
      } else {
	if (val & 0x80) {
	  DPRINTF(("(0x%08lx, PC, ", oldpc + (val & 0xff) - 0x100));
	} else {
	  DPRINTF(("(0x%08lx, PC, ", oldpc + (val & 0xff)));
	}
      }
      if (val & 0x8000) {
	DPRINTF(("A%ld", (val & 0x7000)>>12));
      } else {
	DPRINTF(("D%ld", (val & 0x7000)>>12));
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
      ULONG val = mem[(*pc)++];

#ifdef DEBUG
      if ((flags & 0x0038) == 0x0028) {
	if (val & 0x8000) {
	  DPRINTF(("(-0x%04lx, A%ld)", 0x00010000 - val, flags & 0x0007));
	} else {
	  DPRINTF(("(0x%04lx, A%ld)", val, flags & 0x0007));
	}
      } else if ((flags & 0x003f) == 0x003a) {
	if (val & 0x8000) {
	  DPRINTF(("(0x%08lx, PC)", oldpc + val - 0xfffe));
	} else {
	  DPRINTF(("(0x%08lx, PC)", oldpc + val + 2));
	}
      } else {
	if (val & 0x8000) {
	  DPRINTF(("(-0x%04lx).W", 0x00010000 - val));
	} else {
	  DPRINTF(("(0x%04lx).W", val));
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
	DPRINTF(("(A%ld)", flags & 0x07));
	break;
      case 0x18:
	DPRINTF(("(A%ld)+", flags & 0x07));
	break;
      case 0x20:
	DPRINTF(("-(A%ld)", flags & 0x07));
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

ULONG compile(struct code_info *ci)
{
  ULONG pc = ci->maddr>>1;
  ULONG *code = ScratchPad;
  ULONG oldpc;
  ULONG flags;
  ULONG regs = 0;
  USHORT *mem = (USHORT *)memory;
  USHORT opcode;

  asm("	flush	%g0\n");	/* Flush instruction cache */

  copy_template(&code, s_pre_amble);

  do {
    oldpc = pc;
    opcode = mem[pc++];

    DPRINTF(("\nPC:%08lx\tOpcode:%04x\tMnemonic:%s\n"
	     "\tFlags:%08lx\tTemplate:%08lx\n",
	     (pc - 1)<<1, opcode, compiler_tab[opcode].mnemonic,
	     compiler_tab[opcode].flags, (ULONG)compiler_tab[opcode].template));
#ifdef DEBUG
    {
      extern ULONG opcode_4afc[];
      if (compiler_tab[opcode].template == opcode_4afc) {
	DPRINTF(("!! ILLEGAL OPCODE !!"));
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
	  ULONG regs2;

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
	ULONG newflags = ((flags & (TEF_SRC_MASK | TEF_SRC_SIZE))>>TEF_SRC_SHIFT);

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
		USHORT immop = mem[pc++] & 0xff;

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
		USHORT immop = mem[pc++];
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
		ULONG immop = mem[pc++];
		immop = (immop << 0x10) | mem[pc++];
		DPRINTF(("#0x%08lx.L", immop));
		/* sethi %hi(immop), %acc1 */
		*(code++) = 0x27000000 | (immop >> 10);
		/* or %lo(immop), %acc1, %acc1 */
		*(code++) = 0xa614e000 | (immop & 0x0fff);
	      }
	    break;
	    default:
	      fprintf(stderr, "Error in immediate operand for opcode 0x%04x, 0x%08lx!\n", opcode, flags);
	      abort();
	      break;
	    }
	  } else {
#ifdef DEBUG
	    if (!(newflags & 0x0030)) {
	      DPRINTF(("%c%ld", ((newflags & 0x0008)?'A':'D'), newflags & 0x0007));
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
	  ULONG regs2;
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
		USHORT immop = mem[pc++] & 0xff;

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
		USHORT immop = mem[pc++];
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
		ULONG immop = mem[pc++];
		DPRINTF(("#0x%08lx.L, ", immop));
		immop = (immop << 0x10) | mem[pc++];
		/* sethi %hi(immop), %acc0 */
		*(code++) = 0x29000000 | (immop >> 10);
		/* or %lo(immop), %acc0, %acc0 */
		*(code++) = 0xa8152000 | (immop & 0x0fff);
	      }
	    break;
	    default:
	      fprintf(stderr, "Error in immediate operand for opcode 0x%04x, 0x%08lx!\n", opcode, flags);
	      abort();
	      break;
	    }
	  } else {
#ifdef DEBUG
	    if (!(flags & 0x0030)) {
	      DPRINTF(("%c%ld", ((flags & 0x0008)?'A':'D'), flags & 0x0007));
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
	  ULONG newregs = 0;
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

    if (flags & TEF_FIX_SR) {
      copy_template(&code, s_fix_sr);
    }

    if (flags & TEF_WRITE_BACK) {
      if (flags & TEF_MOVEM) {
	/* st	%ea, [ %regs + regno ] */
	*(code++) = 0xea262000 | ((0x0008 | (opcode & 0x0007))<<2);
      } else {
#ifdef DEBUG
	if ((!(flags & TEF_DST_LOAD)) && (!(flags & 0x0030))) {
	  DPRINTF(("%c%ld", ((flags & 0x0008)?'A':'D'), (flags & 0x0007)));
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

    if (flags & TEF_MOVEM) {
      fprintf(stderr, "MOVEM HALT!\nstart 0x%08lx, end 0x%08lx\tpc 0x%08lx\n\n",
	      ((ULONG)ci->code), ((ULONG)code), oldpc*2);
    }
  } while (!(flags & TEF_TERMINATE));
#ifdef DEBUG
  {
    extern ULONG opcode_4afc[];
    if (compiler_tab[opcode].template == opcode_4afc) {
      DPRINTF(("!! ILLEGAL OPCODE !!"));
      abort();
    }
  }
#endif /* DEBUG */

  PeepHoleOptimize(ci, ScratchPad, code);
  return(pc<<1);
}

