/*
 * $Id: timebase.c,v 1.1 1998/02/10 17:20:52 marcus Exp $
 *
 * Time functions
 *
 * $Log$
 *
 */

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <thread.h>

#include "types.h"


mutex_t sys_clock_lock;
mutex_t pixel_clock_lock;

static U64 hrepoch, sysepoch, fieldepoch, pixelpos;
static int pal, long_frame;

#define CLOCKRATE ((pal)?9379:15909)
#define HRTOSYS(n) ((n)*7/1000+(n)*CLOCKRATE/100000000)

U64 read_system_clock()
{
  U64 res, t;
  mutex_lock(&sys_clock_lock);
  t = gethrtime()-hrepoch;
  res = HRTOSYS(t)+sysepoch;
  mutex_unlock(&sys_clock_lock);
  return res;
}

U64 read_e_clock()
{
  return read_system_clock()/10;
}

void pixel_timebase(int pal, int lace,
		    U32 *lof, U32 *vpos, U32 *hpos, U64 *nextvbr)
{
  U64 fieldlen;
  mutex_lock(&pixel_clock_lock);
  if(!lace)
    long_frame = 1;
  
  pixelpos += read_system_clock()-fieldepoch;
  for(;;) {
    if(long_frame)
      fieldlen = (pal? 455*313 : 455*263);
    else
      fieldlen = (pal? 455*312 : 455*262);
    if(pixelpos<fieldlen)
      break;
    pixelpos -= fieldlen;
    if(lace)
      long_frame = !long_frame;
  }
  if(lof)
    *lof = long_frame;
  if(vpos || hpos) {
    U32 pos = pixelpos;

    *vpos = pos/456;
    *hpos = pos%456;
  }
  if(nextvbr)
    *nextvbr = fieldepoch + fieldlen - pixelpos;
  mutex_unlock(&pixel_clock_lock);
}

void set_pal_ntsc_timebase(int newpal)
{
  mutex_lock(&sys_clock_lock);
  if(newpal != pal) {
    U64 newepoch = gethrtime();
    U64 drift = HRTOSYS(newepoch-hrepoch);
    hrepoch = newepoch;
    sysepoch += drift;
  }
  mutex_unlock(&sys_clock_lock);
}

void init_timebase()
{
  mutex_init(&sys_clock_lock, USYNC_THREAD, NULL);
  mutex_init(&pixel_clock_lock, USYNC_THREAD, NULL);
  pal = 1;
  long_frame = 1;
  hrepoch = gethrtime();
  sysepoch = fieldepoch = 0;
  pixelpos = 0;
}
