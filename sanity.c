/*
 * $Id: sanity.c,v 1.8 1996/07/17 16:01:54 grubba Exp $
 *
 * Sanity checks for the emulated computer.
 *
 * $Log: sanity.c,v $
 * Revision 1.7  1996/07/17 00:23:39  grubba
 * Added InitResident() to the list of patched functions.
 *
 * Revision 1.6  1996/07/15 02:49:09  grubba
 * addlib_patch() used the wrong offset.
 *
 * Revision 1.5  1996/07/14 21:44:26  grubba
 * Added support for adding hardware dynamically.
 * Added CIAA time of day clock (50Hz).
 * Moved some debug output from stderr to stdout.
 *
 * Revision 1.4  1996/07/13 19:37:51  grubba
 * typo fix.
 *
 * Revision 1.3  1996/07/13 19:32:17  grubba
 * Now defaults to very little debuginfo.
 * Added (un|set)patch().
 * Patches added to MakeLibrary(), MakeFunctions(), Alert() and AddLibrary().
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

#define _U32(x)	(((x) & 2)?((_U16(x)<<16)|_U16((x)+2)):(*((U32 *)(memory + (x)))))
#define _U16(x)	(*((U16 *)(memory + (x))))

/*
 * Globals
 */

extern unsigned char *memory;

/*
 * Static globals
 */

static U32 (*old_makelib)(struct m_registers *, void *);
static U32 (*old_makefunc)(struct m_registers *, void *);
static U32 (*old_alert)(struct m_registers *, void *);
static U32 (*old_addlib)(struct m_registers *, void *);
static U32 (*old_allocmem)(struct m_registers *, void *);
static U32 (*old_permit)(struct m_registers *, void *);
static U32 (*old_initresident)(struct m_registers *, void *);

/*
 * Functions
 */

/*
 * Patches
 */

static U32 makelib_patch(struct m_registers *regs, void *mem)
{
  printf("MakeLibrary(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x)\n",
	 regs->a0, regs->a1, regs->a2, regs->d0, regs->d1);

  return (old_makelib(regs, mem));
}

static U32 makefunc_patch(struct m_registers *regs, void *mem)
{
  printf("MakeFunctions(0x%08x, 0x%08x, 0x%08x)\n",
	 regs->a0, regs->a1, regs->a2);

  return(old_makefunc(regs, mem));
}

static U32 alert_patch(struct m_registers *regs, void *mem)
{
  printf("Alert(0x%08x)\n", regs->d7);

  return(old_alert(regs, mem));
}

static U32 addlib_patch(struct m_registers *regs, void *mem)
{
  U32 name;

  printf("AddLibrary(0x%08x)\n", regs->a1);
 
  name = _U32(regs->a1 + 0x000a);

  printf("Name: 0x%08x, \"%s\"\n", name, memory + name);

  name = _U32(regs->a1 + 0x0018);

  printf("ID:   0x%08x, \"%s\"\n", name, memory + name);

  return (old_addlib(regs, mem));
}

static U32 allocmem_patch(struct m_registers *regs, void *mem)
{
  printf("AllocMem(0x%08x, 0x%08x)\n",
	 regs->d0, regs->d1);

  return (old_allocmem(regs, mem));
}

static U32 permit_patch(struct m_registers *regs, void *mem)
{
  printf("Permit()\n");

  return (old_permit(regs, mem));
}

static U32 initresident_patch(struct m_registers *regs, void *mem)
{
  printf("InitResident(0x%08x, 0x%08x)\n", regs->a1, regs->d1);

  return (old_initresident(regs, mem));
}

/*
 * Sanity functions
 */

void patch_SysBase(U32 SysBase)
{
#ifdef DEBUG
  printf("Patching SysBase...\n");
#endif /* DEBUG */
  old_makelib = setpatch(_U32(2 + SysBase - 0x0054), makelib_patch);
  old_makefunc = setpatch(_U32(2 + SysBase - 0x005a), makefunc_patch);
  old_alert = setpatch(_U32(2 + SysBase - 0x006c), alert_patch);
  old_addlib = setpatch(_U32(2 + SysBase - 0x018c), addlib_patch);
  old_allocmem = setpatch(_U32(2 + SysBase - 0x00c6), allocmem_patch);
  old_permit = setpatch(_U32(2 + SysBase - 0x008a), permit_patch);
  old_initresident = setpatch(_U32(2 + SysBase - 0x0066), initresident_patch);
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
  U32 SysBase = *((U32 *)(memory + 4));
  int oldstate = state;
  static meminfogiven = 0;

  if (*((U32 *)memory) == 0x48454c50) {
    printf("HELP\n");
  }

  if (SysBase) {
    if (SysBase & 1) {
      if (state != 1) {
	sprintf(Message, "Odd SysBase (0x%08x)!\n", SysBase);
	state = 1;
      }
    } else {
      U32 ChkBase;
      _SysBase = (struct ExecBase *)(memory + SysBase);
      ChkBase = ((*((U16 *)(&_SysBase->ChkBase)))<<16) |
	(*(((U16 *)(&_SysBase->ChkBase))+1));
      if (ChkBase && (ChkBase != ~SysBase)) {
	if (state != 2) {
	  sprintf(Message, "Broken ChkBase (0x%08x != 0x%08x)\n",
		  ChkBase, ~SysBase);
	  state = 2;
	}
      } else {
	if (state != 3) {
	  sprintf(Message, "SysBase at 0x%08x\n", SysBase);
	  state = 3;
	}
      }
    }
  }

  if (state != oldstate) {
    if (count) {
      printf("SANITY: Last messages repeated %d times\n", count);
    }
    printf("SANITY: %s", Message);
    count = 0;
  } else if ((state == 3) && (count == 0x0200)){
    /* FIXME: Adhoc */
    patch_SysBase(SysBase);
  }
  count++;

  if ((!meminfogiven) && (_U16(0x040e))) {
    meminfogiven = 1;
    if (_U32(0x040a)) {
      printf("SANITY: Memory found: type:0x%04x Free:0x%08x Range:0x%08x - 0x%08x First free block:0x%08x Name:\"%s\"\n",
	      _U16(0x040e), _U32(0x041c), _U32(0x0414),
	      _U32(0x0418), _U32(0x0410), memory + _U32(0x040a));
    } else {
      printf("SANITY: Memory found: type:0x%04x Free:0x%08x Range:0x%08x - 0x%08x First free block:0x%08x No name\n",
	      _U16(0x040e), _U32(0x041c), _U32(0x0414),
	      _U32(0x0418), _U32(0x0410));
    }
  }
}

void breakme(void)
{
}
