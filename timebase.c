/*
 * $Id: timebase.c,v 1.2 1998/02/11 00:06:07 marcus Exp $
 *
 * Time functions
 *
 * $Log: timebase.c,v $
 * Revision 1.1  1998/02/10 17:20:52  marcus
 * Synchronized raster counter.
 *
 *
 */

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <thread.h>

#include "types.h"
#include "recomp.h"
#include "interrupt.h"


mutex_t pixel_clock_lock;

static U64 hrepoch, sysepoch, fieldepoch;
static int pal, long_field;

#define CLOCKRATE ((pal)?9379:15909)
#define FREQ ((pal)?7093790:7159090)
#define HRTOSYS(n) ((n)*7/1000+(n)*CLOCKRATE/100000000)

U64 read_system_clock()
{
  U64 res, t;
  t = gethrtime()-hrepoch;
  res = HRTOSYS(t)+sysepoch;
  return res;
}

U64 read_e_clock()
{
  return read_system_clock()/10;
}

void pixel_timebase(int pal, int lace,
		    U32 *lof, U32 *vpos, U32 *hpos, U64 *nextvbr)
{
  U64 pixelpos, fieldlen;
  mutex_lock(&pixel_clock_lock);
  if(!lace)
    long_field = 1;
  
  pixelpos = read_system_clock()-fieldepoch;
  for(;;) {
    if(long_field)
      fieldlen = (pal? 455*313 : 455*263);
    else
      fieldlen = (pal? 455*312 : 455*262);
    if(pixelpos<fieldlen)
      break;
    pixelpos -= fieldlen;
    fieldepoch += fieldlen;
    if(lace)
      long_field = !long_field;
  }
  if(lof)
    *lof = long_field;
  if(vpos || hpos) {
    U32 pos = pixelpos;

    *vpos = pos/456;
    *hpos = pos%456;
  }
  if(nextvbr)
    *nextvbr = fieldepoch + fieldlen;
  mutex_unlock(&pixel_clock_lock);
}

void wait_until_sysclock(U64 c)
{
  struct timespec request;
  U32 freq = FREQ;
  do {
    U64 dur, now = read_system_clock();
    if(now >= c)
      return;
    dur = c - now;
    if(dur>=freq) {
      request.tv_sec = dur/freq;
      dur -= freq;
    } else
      request.tv_sec = 0;
    request.tv_nsec = (long)(dur*1000000000LL/FREQ);
  } while(nanosleep(&request, NULL));
}

void set_pal_ntsc_timebase(int newpal)
{
  if(newpal != pal) {
    U64 newepoch = gethrtime();
    U64 drift = HRTOSYS(newepoch-hrepoch);
    hrepoch = newepoch;
    sysepoch += drift;
  }
}

void do_vblank()
{
  mutex_lock(&irq_ctrl_lock);
  ((U16 *)memory)[0xdff01e>>1] |= 0x20;
  cond_signal(&irq_ctrl_signal);
  mutex_unlock(&irq_ctrl_lock);
}

thread_t timer_thread;

void *timer_entry(void *arg)
{
  U64 next_vblank = 0;
  for(;;) {
    U64 c = read_system_clock();
    if(c>=next_vblank) {
      pixel_timebase(memory[0xdff1dd]&0x20, memory[0xdff001]&0x02,
		     NULL, NULL, NULL, &next_vblank);
      do_vblank();
    }
    wait_until_sysclock(next_vblank);
  }
}

void init_timebase()
{
  mutex_init(&pixel_clock_lock, USYNC_THREAD, NULL);
  pal = 1;
  long_field = 1;
  hrepoch = gethrtime();
  sysepoch = fieldepoch = 0;
  thr_create(NULL, 65536, timer_entry, NULL,
	     THR_BOUND | THR_DETACHED | THR_DAEMON, &timer_thread);
  thr_setprio(timer_thread, 127);
}
