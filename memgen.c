/*
 * $Id: memgen.c,v 1.1.1.1 1996/06/30 23:51:53 grubba Exp $
 *
 * Compilergenerator. Generates a compiler from M68000 to Sparc binary code.
 * Memory access part.
 * Based on compgen 1.5.
 *
 * $Log: memgen.c,v $
 * Revision 1.1  1996/06/01 09:31:29  grubba
 * Initial revision
 *
 *
 */

/* TODO:
 *
 *  * Probaby need to generate emit_load_ea_%02x() to reduce code-size.
 *
 *  * Most op-codes still not implemented.
 *
 *  * The generated code-size (~16M) needs to be much reduced.
 */

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>

#include "recomp.h"
#include "m68k.h"
#include "sparc.h"

/*
 * Prototypes
 */

int comp_supervisor(FILE *, USHORT, const char *);
int comp_clobber(FILE *, UBYTE, UBYTE, UBYTE);


/*
 * Functions
 */

/*
 * Some help functions
 */

int comp_supervisor(FILE *fp, USHORT opcode, const char *mnemonic)
{
  fprintf(fp, "/* FIXME: Supervisor mode only instruction */\n");
  return(0);
}

int comp_clobber(FILE *fp, UBYTE sdst, UBYTE sval, UBYTE size)
{
  switch(size) {
  case 0: /* BYTE */
    fprintf(fp, "emit_clobber_byte(code, 0x%02x, 0x%02x);\n", sdst, sval);
    break;
  case 1: /* SHORT */
    fprintf(fp, "emit_clobber_short(code, 0x%02x, 0x%02x);\n", sdst, sval);
    break;
  case 2: /* LONG */
    fprintf(fp, "emit_clobber(code, 0x%02x, 0x%02x);\n", sdst, sval);
    break;
  default: /* ERROR */
    fprintf(fp, "/* Bad clobber size %d, dst=0x%02x, val=0x%02x */\n",
	    size, sdst, sval);
    break;
  }
  return(0);
}

int comp_load_ea(FILE *fp, UBYTE size, UBYTE mode, UBYTE mreg)
{
  switch(mode) {
  case 0x02:	/* (An) */
    fprintf(fp, "S_LD(S_I0, 0x%02x, eareg);\n", (M_A0 + mreg)<<2);
    break;
  case 0x03:	/* (An)+ */
    fprintf(fp, "S_LD(S_I0, 0x%02x, eareg);\n", (M_A0 + mreg)<<2);
    if ((mreg == 7) && (!size)) {
      size = 1;
    }
    fprintf(fp, "S_ADDI(0x%02x, eareg, S_O0);\n", 1<<size);
    fprintf(fp, "S_STI(S_O0, S_I0, 0x%02x);\n", (M_A0 + mreg)<<2);
    break;
  case 0x04:	/* -(An) */
    fprintf(fp, "S_LD(S_I0, 0x%02x, eareg);\n", (M_A0 + mreg)<<2);
    if ((mreg == 7) && (!size)) {
      size = 1;
    }
    fprintf(fp, "S_SUBI(0x%02x, eareg, eareg);\n", 1<<size);
    fprintf(fp, "S_STI(eareg, S_I0, 0x%02x);\n", (M_A0 + mreg)<<2);
    break;
  case 0x05:	/* (d16, An) */
    fprintf(fp, "{\nSHORT off = mem[(*pc)++];\n");
    fprintf(fp, "S_LD(S_I0, 0x%02x, eareg);\n", (M_A0 + mreg)<<2);
    fprintf(fp, "if ((off & 0xf000) && ((off & 0xf000) != 0xf000)) {\n");
    fprintf(fp, "S_SETHI(off, S_O0);\n");
    fprintf(fp, "S_ADD(S_O0, eareg, eareg);\n");
    fprintf(fp, "S_ADDI(off & 0x0fff, eareg, eareg);\n");
    fprintf(fp, "} else {\n");
    fprintf(fp, "S_ADDI(off & 0x1fff, eareg, eareg);\n");
    fprintf(fp, "}\n");
    fprintf(fp, "}\n");
    break;
  case 0x06:	/* (d8, An, Xn) */
    fprintf(fp, "{\n");
    fprintf(fp, "USHORT format = mem[(*pc)++];\n");
    fprintf(fp, "if (format & 0x0100) {\n");	/* M68020+? */
    fprintf(fp, "emit_exception(code, *pc - 1, VEC_ILL_INSTR);\n");
    fprintf(fp, "return;/* FIXME: */\n");
    fprintf(fp, "}\n");
    /* Fetch address register */
    fprintf(fp, "S_LD(S_I0, 0x%02x, eareg);\n", (M_A0 + mreg)<<2);
    /* Displacement */
    fprintf(fp, "if (format & 0x0080) {\n");
    fprintf(fp, "S_ADDI(0x1f00 | (format & 0x00ff), eareg, eareg);\n");
    fprintf(fp, "} else {\n");
    fprintf(fp, "S_ADDI((format & 0x007f), eareg, eareg);\n");
    fprintf(fp, "}\n");
    /* Index */
    fprintf(fp, "if (format & 0x0800) {\n");
    fprintf(fp, "S_LDH(S_I0, ((format & 0xf000)>>10)+2, S_O0);\n");
    fprintf(fp, "} else {\n");
    fprintf(fp, "S_LD(S_I0, ((format & 0xf000)>>10), S_O0);\n");
    fprintf(fp, "}\n");
    /* Scale */
    fprintf(fp, "if (format & 0x0600) {\n");
    fprintf(fp, "S_SLLI(S_O0, ((format & 0x0600)>>9), S_O0);\n");
    fprintf(fp, "}\n");
    /* Add on scaled index */
    fprintf(fp, "S_ADD(S_O0, eareg, eareg);\n");
    fprintf(fp, "}\n");
    break;
  case 0x07:	/* Other modes */
    switch(mreg) {
    case 0x00:	/* (d16).W */
      fprintf(fp, "{\nSHORT addr = mem[(*pc)++];\n");
      fprintf(fp, "if ((addr & 0xf000) && ((addr & 0xf000) != 0xf000)) {\n");
      fprintf(fp, "S_SETHI(addr, eareg);\n");
      fprintf(fp, "S_ORI(addr & 0xfff, eareg, eareg);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_MOVI(addr & 0x1fff, eareg);\n");
      fprintf(fp, "}\n");
      fprintf(fp, "}\n");
      break;
    case 0x01:	/* (d32).L */
      fprintf(fp, "{\nULONG addr = mem[(*pc)++];\n");
      fprintf(fp, "addr = (addr << 16) | mem[(*pc)++];\n");
      fprintf(fp, "if ((addr & 0xfffff000) && ((addr & 0xfffff000) != 0xfffff000)) {\n");
      fprintf(fp, "S_SETHI(addr, eareg);\n");
      fprintf(fp, "S_ORI(addr & 0xfff, eareg, eareg);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_MOVI(addr & 0x1fff, eareg);\n");
      fprintf(fp, "}\n");
      fprintf(fp, "}\n");
      break;
    case 0x02:	/* (d16, PC) */
      fprintf(fp, "{\nULONG addr = (*pc)<<1;\n");
      fprintf(fp, "addr += ((SHORT *)mem)[(*pc)++];\n");
      fprintf(fp, "if ((addr & 0xfffff000) && ((addr & 0xfffff000) != 0xfffff000)) {\n");
      fprintf(fp, "S_SETHI(addr, eareg);\n");
      fprintf(fp, "S_ORI(addr & 0xfff, eareg, eareg);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_MOVI(addr & 0x1fff, eareg);\n");
      fprintf(fp, "}\n");
      fprintf(fp, "}\n");
      break;
    case 0x03:	/* (d8, PC, Xn) */
      fprintf(fp, "{\n");
      fprintf(fp, "ULONG addr = (*pc)<<1;\n");
      fprintf(fp, "USHORT format = mem[(*pc)++];\n");
      fprintf(fp, "if (format & 0x0100) {\n");	/* M68020+? */
      fprintf(fp, "emit_exception(code, *pc - 1, VEC_ILL_INSTR)\n");
      fprintf(fp, "return(1);/* FIXME: */\n");
      fprintf(fp, "}\n");
      /* Displacement */
      fprintf(fp, "if (format & 0x0080) {\n");
      fprintf(fp, "addr += 0xffffff00 | (format & 0xff);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "addr += (format & 0x7f);\n");
      fprintf(fp, "}\n");
      /* Index */
      fprintf(fp, "if (format & 0x0800) {\n");
      fprintf(fp, "S_LDH(S_I0, ((format & 0xf000)>>10)+2, S_O0);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_LD(S_I0, ((format & 0xf000)>>10), S_O0);\n");
      fprintf(fp, "}\n");
      /* Scale */
      fprintf(fp, "if (format & 0x0600) {\n");
      fprintf(fp, "S_SLLI(S_O0, ((format & 0x0600)>>9), S_O0);\n");
      fprintf(fp, "}\n");
      fprintf(fp, "if ((addr & 0xfffff000) && ((addr & 0xfffff000) != 0xfffff000)) {\n");
      fprintf(fp, "S_SETHI(addr, eareg);\n");
      fprintf(fp, "S_ORI(addr & 0xfff, eareg, eareg);\n");
      fprintf(fp, "S_ADD(S_O0, eareg, eareg);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_ADDI(addr & 0x1fff, S_O0, eareg);\n");
      fprintf(fp, "}\n");
      fprintf(fp, "}\n");
      break;
    default:
      fprintf(fp, "/* Unknown addressing mode 0x%02x, mreg 0x%02x */\n", mode, mreg);
      break;
    }
    break;
  default:
    fprintf(fp, "/* Unknown addressing mode 0x%02x, mreg 0x%02x */\n", mode, mreg);
    break;
  }
  return(0);
}

int comp_load(FILE *fp, UBYTE size, UBYTE mode, UBYTE mreg)
{
  if ((mode & 0xfe) && (!((mode == 0x07) && (mreg == 0x04)))) {
    /* Memory access */
    if (mode == 2) {
      /* Inline optimization */
      fprintf(fp, "UBYTE eareg=S_L5;\n");
      comp_load_ea(fp, size, mode, mreg);
    } else {
      fprintf(fp, "emit_load_ea_%02x(pc, code, mem, S_L5);\n",
	      (size << 6) | (mode << 3) | (mreg));
    }
    fprintf(fp, "S_CMP(S_L5, S_I4);\n");
    fprintf(fp, "S_BCC(((ULONG)code)+0x18);\n");	/* To memory access ***** */
    /* Hw */
    fprintf(fp, "S_MOV(sreg, S_O1);\n");	/* Before-value */
    switch(size) {
    case 0: /* BYTE */
      fprintf(fp, "S_CALL(read_hw_byte);\n");
      break;
    case 1: /* SHORT */
      fprintf(fp, "S_CALL(read_hw_short);\n");
      break;
    case 2:
      fprintf(fp, "S_CALL(read_hw_long);\n");
      break;
    default:
      fprintf(fp, "/* Unknown size: %d, mode 0x%02x, reg 0x%02x */\n",
	      size, mode, mreg);
      fprintf(fp, "S_NOP;\n");
      break;
    }
    fprintf(fp, "S_MOV(S_L5, S_O0);\n");		/* maddr */
    if (size == 2) {
      fprintf(fp, "S_B(((ULONG)code)+0x18);\n");	/* to end **** */
    } else {
      fprintf(fp, "S_B(((ULONG)code)+0x08);\n");	/* to end **** */
    }
    fprintf(fp, "S_MOV(S_O0, sreg);\n");		/* result */
    
    /* Memory access */
    switch(size) {
    case 0: /* BYTE */
      fprintf(fp, "S_LDUB(S_L5, S_I1, sreg);\n");
      break;
    case 1: /* SHORT */
      fprintf(fp, "S_LDUH(S_L5, S_I1, sreg);\n");
      break;
    case 2: /* LONG */
      fprintf(fp, "S_LDUH(S_L5, S_I1, sreg);\n");
      fprintf(fp, "S_SLLI(sreg, 0x10, sreg);\n");
      fprintf(fp, "S_ADDI(0x02, S_L5, S_O0);\n");
      fprintf(fp, "S_LDUH(S_O0, S_I1, S_O0);\n");
      fprintf(fp, "S_OR(S_O0, sreg, sreg);\n");
      break;
    default:
      fprintf(fp, "/* Unknown size: %d, mode 0x%02x, mreg 0x%02x */\n",
	      size, mode, mreg);
      fprintf(fp, "S_NOP;\n");
      break;
    }
  } else if (mode == 0x07) {
    /* Immediate */
    switch(size) {
    case 0:	/* BYTE */
      fprintf(fp, "{\nUSHORT val = mem[(*pc)++];\n");
      fprintf(fp, "if (val & 0x0080) {\n");
      fprintf(fp, "S_MOVI((0x1f00 | (val & 0xff)), sreg);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_MOVI((val & 0x7f), sreg);\n");
      fprintf(fp, "}\n}\n");
      break;
    case 1:	/* WORD */
      fprintf(fp, "{\nULONG val = ((SHORT *)mem)[(*pc)++];\n");
      fprintf(fp, "if ((val & 0xfffff000) && ((val & 0xfffff000) != 0xfffff000)) {\n");
      fprintf(fp, "S_SETHI(val, sreg);\n");
      fprintf(fp, "S_ORI((val & 0xfff), sreg, sreg);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_MOVI((val & 0x1fff), sreg);\n");
      fprintf(fp, "}\n}\n");
      break;
    case 2:	/* LONG */
      fprintf(fp, "{\nULONG val = mem[(*pc)++];\n");
      fprintf(fp, "val = (val << 16) | mem[(*pc)++];\n");
      fprintf(fp, "if ((val & 0xfffff000) && ((val & 0xfffff000) != 0xfffff000)) {\n");
      fprintf(fp, "S_SETHI(val, sreg);\n");
      fprintf(fp, "S_ORI((val & 0xfff), sreg, sreg);\n");
      fprintf(fp, "} else {\n");
      fprintf(fp, "S_MOVI((val & 0x1fff), sreg);\n");
      fprintf(fp, "}\n}\n");
      break;
    default:
      /* ERROR */
      fprintf(fp, "/* Unknown size %d, mode 0x%02x, mreg 0x%02x */\n",
	      size, mode, mreg);
      break;
    }
  } else {
    /* Register access */
    switch(size) {
    case 0: /* BYTE */
      fprintf(fp, "S_LDUBI(S_I0, 0x%02x, sreg);\n", ((M_A0*mode + mreg) << 2) + 3);
      break;
    case 1: /* SHORT */
      fprintf(fp, "S_LDUHI(S_I0, 0x%02x, sreg);\n", ((M_A0*mode + mreg) << 2) + 2);
      break;
    case 2: /* LONG */
      fprintf(fp, "S_LDI(S_I0, 0x%02x, sreg);\n", ((M_A0*mode + mreg)<<2));
      break;
    default:
      fprintf(fp, "/* Unknown size: %d, mode 0x%02x, mreg 0x%02x */\n",
	      size, mode, mreg);
      break;
    }
  }
  return(0);
}

const char *memgen_head =
"/* This file was generated by memgen\n"
" *\n"
" * DO NOT EDIT!\n"
" *\n"
" * Generated by $Id: memgen.c,v 1.1.1.1 1996/06/30 23:51:53 grubba Exp $\n"
" *\n"
" * $Author: grubba $\n"
" */\n"
"\n"
"/*\n"
" * Includes\n"
" */\n"
"\n"
"#include \"recomp.h\"\n"
"#include \"sparc.h\"\n"
"#include \"m68k.h\"\n"
"#include \"codeinfo.h\"\n"
"\n"
"#include <stdio.h>\n"
"\n";

/*
 * The main memgen generator loop
 */

void make_memaccess(FILE *fp)
{
  ULONG opcode = 0;

  fprintf(fp, memgen_head);
#ifdef DEBUG
  fprintf(fp,
	  "#ifndef DEBUG\n"
	  "#define DEBUG\n"
	  "#endif\n\n");
#endif /* DEBUG */
#ifndef NDEBUG
  fprintf(fp, "/*\n * Debug functions\n */\n\n");
  fprintf(fp, "#ifdef DEBUG\n");
  fprintf(fp, "#define KPRINT(x) fprintf(stderr, x)\n");
  fprintf(fp, "#else\n");
  fprintf(fp, "#define KPRINT(x)\n");
  fprintf(fp, "#endif\n\n");
#endif /* NDEBUG */
  fprintf(fp, "/*\n * Help functions\n */\n\n");
  /* opcodes over 0xc0 are size 3 */
  for (opcode = 0; opcode < 0x00c0; opcode++) {
    if (((opcode & 0x0038) != 0x0038) || ((opcode & 0x0007) <= 0x0004)) {
      if ((opcode & 0x0030) && ((opcode & 0x003f) != 0x003c)) {
	fprintf(fp, "static void emit_load_ea_%02x(ULONG *pc, ULONG **code, "
		"USHORT *mem, UBYTE eareg)\n", (unsigned int)opcode);
	fprintf(fp, "{\n");
	comp_load_ea(fp, (opcode & 0xc0)>>6, (opcode & 0x38)>>3, (opcode & 0x07));
	fprintf(fp, "}\n\n");
      }
      fprintf(fp, "static void emit_load_%02x(ULONG *pc, ULONG **code, USHORT *mem, "
	      "UBYTE sreg)\n",
	      (unsigned int)opcode);
      fprintf(fp, "{\n");
      comp_load(fp, (opcode & 0xc0)>>6, (opcode & 0x38)>>3, (opcode & 0x07));
      fprintf(fp, "}\n\n");
    }
  }

  fprintf(fp, "\n/*\n * Compilerfunction lookup table\n */\n\n");
  fprintf(fp, "static int (*comp_tab)(ULONG *, ULONG **, USHORT *)[] = {\n");
  for (opcode=0; opcode < 0x00010000; opcode++) {
    fprintf(fp, "  comp_%04x,\n", (unsigned int)opcode);
  }

  fprintf(fp, "};\n");
  fprintf(fp, "\n/*\n * Main compilation function\n */\n\n");

  fprintf(fp, 

}


int main(int argc, char **argv)
{
  make_memaccess(stdout);
  exit(0);
}

