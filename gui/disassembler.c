/*
 * $Id: disassembler.c,v 1.1 1996/07/11 15:37:24 grubba Exp $
 *
 * User Interface for the M68000 to Sparc recompiler
 *
 * $Log$
 *
 */

/*
 * Includes
 */

#include <thread.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>

#include <stdio.h>
#include <memory.h>
#include <malloc.h>

#include "../recomp.h"
#include "../m68k.h"
#include "../gui.h"

/*
 * Functions
 */

void *disassembler_main(void *arg)
{
  /* 352x133+66+621 */

  return(arg);
}

int start_disassembler(void)
{
  thread_t disass_thread;

  return(thr_create(NULL, 0, disassembler_main, NULL,
		    THR_DETACHED | THR_DAEMON, &disass_thread));
}


