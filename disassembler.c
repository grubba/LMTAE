/*
 * $Id: disassembler.c,v 1.2 1996/07/11 15:41:55 grubba Exp $
 *
 * M68000 disassembler
 *
 * $Log: disassembler.c,v $
 * Revision 1.1  1996/07/08 21:19:47  grubba
 * Disassembler added.
 *
 *
 */

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "recomp.h"
#include "m68k.h"
#include "compiler.h"
#include "opcodes.h"

/*
 * Functions
 */

void print_reg(char *buffer, ULONG reg)
{
  if (reg & 8) {
    sprintf(strchr(buffer, '\0'), "A%ld", reg & 7);
  } else {
    sprintf(strchr(buffer, '\0'), "D%ld", reg & 7);
  }
}

void print_multiple_registers(char *buffer, ULONG regs)
{
  int i;
  int f = 0;

  for (i = 0; regs; regs>>=1, i++) {
    if (regs & 1) {
      if (!f) {
	print_reg(buffer, i);
	f = 1;
      } else if (f == 1) {
	strcat(buffer, "-");
	f = 2;
      }
    } else if (f) {
      if (f == 2) {
	print_reg(buffer, i-1);
      }
      strcat(buffer, "/");
      f = 0;
    }
  }
  if (f == 2) {
    print_reg(buffer, i-1);
  }
}

void disass_ea(char *buffer, ULONG *pc, ULONG flags, ULONG oldpc)
{
  USHORT *mem = (USHORT *)memory;

  if ((flags & 0x0030) && ((flags & 0x003f) != 0x003c)) {
    /* Not Register direct or immediate */

    if ((flags & 0x003f) == 0x39) {
      /* (d32).L */
      ULONG val = mem[(*pc)++];
      val = (val<<16) | mem[(*pc)++];
      sprintf(strchr(buffer, '\0'), "(0x%08lx).L", val);
    } else if (((flags & 0x0038) == 0x0030) ||
	       ((flags & 0x003f) == 0x003b)) {
      /* (d8, An, Xn), (d8, PC, Xn) */
      ULONG val = mem[(*pc)++];

      if ((flags & 0x0038) == 0x0030) {
	if (val & 0x80) {
	  sprintf(strchr(buffer, '\0'), "(-0x%02lx, A%ld, ",
		  (0x100 - (val & 0xff)), flags & 0x07);
	} else {
	  sprintf(strchr(buffer, '\0'), "(0x%02lx, A%ld, ",
		  val % 0xff, flags & 0x07);
	}
      } else {
	if (val & 0x80) {
	  sprintf(strchr(buffer, '\0'), "(0x%08lx, PC, ",
		  oldpc + (val & 0xff) - 0x100);
	} else {
	  printf(strchr(buffer, '\0'), "(0x%08lx, PC, ",
		 oldpc + (val & 0xff));
	}
      }
      print_reg(buffer, (val & 0xf000)>>12);
      if (val & 0x0800) {
	strcat(buffer, ".L");
      } else {
	strcat(buffer, ".W");
      }
      if (val & 0x0600) {
	sprintf(strchr(buffer, '\0'), "*%d)", 0x1 << ((val & 0x0600)>>9));
      } else {
	strcat(buffer, ")");
      }
    } else if ((flags & 0x0038) > 0x20) {
      /* (d16, An), (d16).W, (d16, PC) */
      ULONG val = mem[(*pc)++];

      if ((flags & 0x038) == 0x0028) {
	if (val & 0x8000) {
	  sprintf(strchr(buffer, '\0'), "(-0x%04lx, A%ld)",
		  0x00010000 - val, flags & 0x0007);
	} else {
	  sprintf(strchr(buffer, '\0'), "(0x%04lx, A%ld)", val, flags & 0x0007);
	}
      } else if ((flags & 0x003f) == 0x003a) {
	if (val & 0x8000) {
	  sprintf(strchr(buffer, '\0'), "(0x%08lx, PC)", oldpc + val - 0xfffe);
	} else {
	  sprintf(strchr(buffer, '\0'), "(0x%08lx, PC)", oldpc + val + 2);
	}
      } else {
	if (val & 0x8000) {
	  sprintf(strchr(buffer, '\0'), "(-0x%04lx).W", 0x00010000 - val);
	} else {
	  sprintf(strchr(buffer, '\0'), "(0x%04lx).W", val);
	}
      }
    } else {
      switch (flags & 0x0038) {
      case 0x10:
	sprintf(strchr(buffer, '\0'), "(A%ld)", flags & 0x07);
	break;
      case 0x18:
	sprintf(strchr(buffer, '\0'), "(A%ld)+", flags & 0x07);
	break;
      case 0x20:
	sprintf(strchr(buffer, '\0'), "-(A%ld)", flags & 0x07);
	break;
      }
    }
  } else if ((flags & 0x003f) == 0x003c) {
    ULONG val = mem[(*pc)++];
    if ((flags & 0x00c0) == 0x80) {
      val = (val << 16) | mem[(*pc)++];
      sprintf(strchr(buffer, '\0'), "#0x%08lx", val);
    } else if ((flags & 0x00c0) == 0x40) {
      if (val & 0x8000) {
	sprintf(strchr(buffer, '\0'), "#-0x%04lx", 0x00010000 - val);
      } else {
	sprintf(strchr(buffer, '\0'), "#0x%04lx", val);
      }
    } else {
      if (val & 0x0080) {
	sprintf(strchr(buffer, '\0'), "#-0x%02lx", 0x0100 - (val & 0xff));
      } else {
	sprintf(strchr(buffer, '\0'), "#0x%02lx", (val & 0xff));
      }
    }
  } else {
    print_reg(buffer, flags & 0x000f);
    if (flags & 0x30) {
      sprintf(strchr(buffer, '\0'), "Bad mode! 0x%08lx", flags);
    }
  }
}

static const char *branch_types[16] = {
  "%sT",  "%sF",
  "%sHI", "%sLS",
  "%sCC", "%sCS",
  "%sNE", "%sEQ",
  "%sVC", "%sVS",
  "%sPL", "%sMI",
  "%sGE", "%sLT",
  "%sGT", "%sLE",
};

void disassemble(ULONG start, ULONG end)
{
  ULONG pc = start>>1;
  ULONG oldpc;
  ULONG flags;
  ULONG regs = 0;
  USHORT *mem = (USHORT *)memory;
  USHORT opcode;
  char buffer[1024];
  const char *mnemonic;
  ULONG i;

  printf("Disassembly of 0x%08lx to 0x%08lx:\n", start, end);

  while ((pc<<1) <= end) {
    oldpc = pc;
    opcode = mem[pc++];
    flags = compiler_tab[opcode].flags;
    mnemonic = compiler_tab[opcode].mnemonic;
    
    if ((!mnemonic[1]) || (!mnemonic[2])) {
      sprintf(buffer, branch_types[((opcode & 0x0f00)>>8)], mnemonic);
      mnemonic = buffer;
    } else {
      sprintf(buffer, "%s", mnemonic);
    }

    if (flags & TEF_DST) {
      switch (flags & TEF_DST_SIZE) {
      case TEF_DST_BYTE:
	strcat(buffer, ".B\t");
	break;
      case TEF_DST_WORD:
	strcat(buffer, ".W\t");
	break;
      case TEF_DST_LONG:
	strcat(buffer, ".L\t");
	break;
      default:
	strcat(buffer, "\t");
	break;
      }
    } else {
      strcat(buffer, "\t");
    }

    if (flags & TEF_MOVEM) {
      regs = mem[pc++];
    }
    if (flags & TEF_SRC) {
      if (flags & TEF_SRC_MOVEM) {
	if ((flags & 0x0038) == 0x0020) {
	  int i;
	  ULONG regs2 = 0;

	  /* Other direction in predecrement mode */
	  for (i=0; i<32; i++) {
	    regs2 <<= 1;
	    regs2 |= regs & 1;
	    regs >>= 1;
	  }
	  regs = regs2;
	}

	print_multiple_registers(buffer, regs);

      } else {
	ULONG newflags = ((flags & (TEF_SRC_MASK | TEF_SRC_SIZE))>>TEF_SRC_SHIFT);

	disass_ea(buffer, &pc, newflags, oldpc*2);
      }
    } else if (flags & TEF_SRC_QUICK8) {
      if (flags & 0x0010000) {
	sprintf(strchr(buffer, '\0'), "#-0x%02lx, ", 0x0100 - ((flags & 0xfe00)>>9));
      } else {
	sprintf(strchr(buffer, '\0'), "#0x%02lx, ", ((flags & 0xfe00)>>9));
      }
    }

    if ((flags & TEF_SRC) && (flags & TEF_DST)) {
      strcat(buffer, ", ");
    }

    if (flags & TEF_DST) {
      if (flags & TEF_DST_MOVEM) {
	print_multiple_registers(buffer, regs);
      } else {
	disass_ea(buffer, &pc, flags, oldpc*2);
      }
    } else if (!(flags & ~TEF_TERMINATE)) {
      if ((opcode & 0xf000) == 0x6000) {
	/* Bcc */
	if (opcode & 0x0080) {
	  sprintf(strchr(buffer, '\0'), "(0x%08lx, PC)",
		  (oldpc<<1) + (opcode & 0x00ff) - 0x00fe);
	} else {
	  sprintf(strchr(buffer, '\0'), "(0x%08lx, PC)",
		 (oldpc<<1) + (opcode & 0x00ff) + 2);
	}
      }
    }
    printf("%08lx  ", (oldpc<<1));
    for (i=0; i < (pc - oldpc); i++) {
      printf("%04x ", mem[oldpc + i]);
    }
    for (; i < 4; i++) {
      printf("     ");
    }
    printf("%s\n", buffer);
  }
  printf("End of disassembly of 0x%08lx to 0x%08lx\n", start, end);
}


