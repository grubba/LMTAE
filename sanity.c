/*
 * $Id: sanity.c,v 1.4 1996/07/13 19:37:51 grubba Exp $
 *
 * Sanity checks for the emulated computer.
 *
 * $Log: sanity.c,v $
 * Revision 1.3  1996/07/13 19:32:17  grubba
 * Now defaults to very little debuginfo.
 * Added (un|set)patch().
 * Patches added to MakeLibrary(), MakeFunctions(), Abort() and AddLibrary().
 *
 * Revision 1.2  1996/07/01 00:13:56  grubba
 * It is now possible to compile from scratch.
 *
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
#include "setpatch.h"

/*
 * Defines
 */

#define _ULONG(x)	(((x) & 2)?((_USHORT(x)<<16)|_USHORT((x)+2)):(*((ULONG *)(memory + (x)))))
#define _USHORT(x)	(*((USHORT *)(memory + (x))))

/*
 * Globals
 */

extern unsigned char *memory;

/*
 * Static globals
 */

static ULONG (*old_makelib)(struct m_registers *, void *);
static ULONG (*old_makefunc)(struct m_registers *, void *);
static ULONG (*old_alert)(struct m_registers *, void *);
static ULONG (*old_addlib)(struct m_registers *, void *);

/*
 * Functions
 */

/*
 * Patches
 */

static ULONG makelib_patch(struct m_registers *regs, void *mem)
{
  printf("MakeLibrary(0x%08lx, 0x%08lx, 0x%08lx, 0x%08lx, 0x%08lx)\n",
	 regs->a0, regs->a1, regs->a2, regs->d0, regs->d1);

  return (old_makelib(regs, mem));
}

static ULONG makefunc_patch(struct m_registers *regs, void *mem)
{
  printf("MakeFunctions(0x%08lx, 0x%08lx, 0x%08lx)\n",
	 regs->a0, regs->a1, regs->a2);

  return(old_makefunc(regs, mem));
}

static ULONG alert_patch(struct m_registers *regs, void *mem)
{
  printf("Alert(0x%08lx)\n", regs->d7);

  return(old_alert(regs, mem));
}

static ULONG addlib_patch(struct m_registers *regs, void *mem)
{
  ULONG name;

  printf("AddLibrary(0x%08lx)\n", regs->a1);

  name = _ULONG(regs->a1 + 0x0010);

  printf("Name: 0x%08lx, \"%s\"\n", name, memory + name);

  return (old_addlib(regs, mem));
}

/*
 * Sanity functions
 */

void patch_SysBase(ULONG SysBase)
{
#ifdef DEBUG
  printf("Patching SysBase...\n");
#endif /* DEBUG */
  old_makelib = setpatch(_ULONG(2 + SysBase - 0x0054), makelib_patch);
  old_makefunc = setpatch(_ULONG(2 + SysBase - 0x005a), makefunc_patch);
  old_alert = setpatch(_ULONG(2 + SysBase - 0x006c), alert_patch);
  old_addlib = setpatch(_ULONG(2 + SysBase - 0x018c), addlib_patch);
#ifdef DEBUG
  printf("SysBase patched\n");
#endif /* DEBUG */
}

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
      if (state != 1) {
	sprintf(Message, "Odd SysBase (0x%08lx)!\n", SysBase);
	state = 1;
      }
    } else {
      ULONG ChkBase;
      _SysBase = (struct ExecBase *)(memory + SysBase);
      ChkBase = ((*((USHORT *)(&_SysBase->ChkBase)))<<16) |
	(*(((USHORT *)(&_SysBase->ChkBase))+1));
      if (ChkBase && (ChkBase != ~SysBase)) {
	if (state != 2) {
	  sprintf(Message, "Broken ChkBase (0x%08lx != 0x%08lx)\n",
		  ChkBase, ~SysBase);
	  state = 2;
	}
      } else {
	if (state != 3) {
	  sprintf(Message, "SysBase at 0x%08lx\n", SysBase);
	  state = 3;
	}
      }
    }
  }

  if (state != oldstate) {
    if (count) {
      fprintf(stderr, "SANITY: Last messages repeated %d times\n", count);
    }
    fprintf(stderr, "SANITY: %s", Message);
    count = 0;
  } else if ((state == 3) && (count == 0x0200)){
    /* FIXME: Adhoc */
    patch_SysBase(SysBase);
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
