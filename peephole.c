/*
 * $Id: peephole.c,v 1.2 1996/07/17 16:01:39 grubba Exp $
 *
 * Peephole optimizer for the M68000 to Sparc recompiler.
 *
 * $Log: peephole.c,v $
 * Revision 1.1  1996/07/01 19:16:53  grubba
 * Implemented ASL and ASR.
 * Changed semantics for new_codeinfo(), it doesn't allocate space for the code.
 * Added PeepHoleOptimize(). At the moment it just mallocs and copies the code.
 * Removed some warnings.
 *
 *
 */

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "types.h"

#include "m68k.h"
#include "codeinfo.h"
#include "peephole.h"

/*
 * Structures and Defines
 */

struct s_register {
  unsigned mode, value;
};

#define RM_UNKNOWN	0
#define RM_SET_VAL	1
#define RM_SET_REG	2

/*
 * Functions
 */

static void PeepHoleOptimize2(struct code_info *code, U32 *start, U32 *end)
{
  static struct s_register registers[32];
  int i;
  U32 *scan = start;
  U32 *scratch = end;

  memset(registers, 0, sizeof(registers));

  while (scan < end) {
    switch (*scan & 0xc0000000) {
    case 0x00000000:	/* IMM 22 */
      switch (*scan & 0x01c00000) {
      case 0x01000000:	/* sethi */
	break;
      case 0x00800000:	/* bxx */
	break;
      default:
	fprintf(stderr, "PeepHoleOptimize2() Unknown sparc opcode 0x%08x\n", *scan);
	abort();
	break;
      }
      break;
    case 0x40000000:	/* UNKNOWN */
      fprintf(stderr, "PeepHoleOptimize2() Unknown sparc opcode 0x%08x\n", *scan);
      abort();
      break;
    case 0x80000000:	/* IMM 12 or register */
      switch (*scan & 0x01f80000) {

      }
      if (!(*scan & 0x0007c000)) {
      }
      break;
    case 0xc0000000:	/* Memory access */
      switch (*scan & 0x01f80000) {
      case 0x00200000:	/* st */
      case 0x00280000:	/* stb */
      case 0x00300000:	/* sth */
	/* The registers value is kept */
	break;
      default:		/* ld and others */
	if (*scan & 0x3e000000) {
	  registers[(*scan & 0x3e000000)>>25].mode = RM_UNKNOWN;
	}
	break;
      }
      break;
    }
    scan++;
  }
}

void PeepHoleOptimize(struct code_info *ci, U32 *start, U32 *end)
{
  U32 size = ((U32)end) - ((U32)start);

  if (!(ci->code = (U32 (*)(struct m_registers *, void *))malloc(size))) {
    fprintf(stderr, "Out of memory in PeepHoleOptimize()\n");
    abort();
  }
  memcpy((void *)ci->code, start, size);
  ci->codeend = (U32 *)(((U32)ci->code) + size);
}
