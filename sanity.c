/*
 * $Id: sanity.c,v 1.2 1996/07/01 00:13:56 grubba Exp $
 *
 * Sanity checks for the emulated computer.
 *
 * $Log: sanity.c,v $
 * Revision 1.1.1.1  1996/06/30 23:51:50  grubba
 * Entry into CVS
 *
 * Revision 1.1  1996/06/30 23:27:11  grubba
 * Initial revision
 *
 *
 */

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>

#include "exec/execbase.h"

#include "m68k.h"
#include "codeinfo.h"

/*
 * Defines
 */

#define _ULONG(x)	((x & 2)?((_USHORT(x)<<16)|_USHORT(x+2)):(*((ULONG *)(memory + x))))
#define _USHORT(x)	(*((USHORT *)(memory + x)))

/*
 * Functions
 */

void check_sanity(struct code_info *ci, struct m_registers *regs, unsigned char *memory)
{
  static state = 0;
  static count = 0;
  static char Message[2048];
  static struct ExecBase *_SysBase = NULL;
  ULONG SysBase = *((ULONG *)(memory + 4));
  int oldstate = state;
  static meminfogiven = 0;

  if (*((ULONG *)memory) == 0x48454c50) {
    fprintf(stderr, "HELP\n");
  }

  if (SysBase) {
    if (SysBase & 1) {
      sprintf(Message, "Odd SysBase (0x%08lx)!\n", SysBase);
      state = 1;
    } else {
      ULONG ChkBase;
      _SysBase = (struct ExecBase *)(memory + SysBase);
      ChkBase = ((*((USHORT *)(&_SysBase->ChkBase)))<<16) |
	(*(((USHORT *)(&_SysBase->ChkBase))+1));
      if (ChkBase && (ChkBase != ~SysBase)) {
	sprintf(Message, "Broken ChkBase (0x%08lx != 0x%08lx)\n",
		ChkBase, ~SysBase);
	state = 2;
      } else {
	sprintf(Message, "SysBase at 0x%08lx\n", SysBase);
	state = 3;
      }
    }
  }

  if (state != oldstate) {
    if (count) {
      fprintf(stderr, "SANITY: Last messages repeated %d times\n", count);
    }
    fprintf(stderr, "SANITY: %s", Message);
    count = 0;
  }
  count++;

  if ((!meminfogiven) && (_USHORT(0x040e))) {
    meminfogiven = 1;
    if (_ULONG(0x040a)) {
      fprintf(stderr, "SANITY: Memory found: type:0x%04x Free:0x%08lx Range:0x%08lx - 0x%08lx First free block:0x%08lx Name:\"%s\"\n",
	      _USHORT(0x040e), _ULONG(0x041c), _ULONG(0x0414),
	      _ULONG(0x0418), _ULONG(0x0410), memory + _ULONG(0x040a));
    } else {
      fprintf(stderr, "SANITY: Memory found: type:0x%04x Free:0x%08lx Range:0x%08lx - 0x%08lx First free block:0x%08lx No name\n",
	      _USHORT(0x040e), _ULONG(0x041c), _ULONG(0x0414),
	      _ULONG(0x0418), _ULONG(0x0410));
    }
  }
}

void breakme(void)
{
}
