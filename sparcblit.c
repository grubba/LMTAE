#include "blitter.h"

#include <stdio.h>
#include <thread.h>
#include <synch.h>
#include <sys/mman.h>

static sparc_instr *code_space=NULL;

static mutex_t dmaconr_lock, blitter_starter;
static cond_t blitter_state;
static sema_t blitter_go;
static thread_t blitter_thread;

#define BLITADDR(x) ((u16*)(((x)&0x1ffffe)+chipmem))

/* Shift:
 *
 * Ascending:                   shift
 *             |               |...|
 * ppppppppppppppppcccccccccccccccc	ror
 *
 *
 * Descending:
 * |...|               |
 * ccccccccccccccccpppppppppppppppp	rol
 *
 */


/* i0-i5 (input) */
/* g1-g4 */
/* o0-o5, o7 */
/* l0-l7 */


/*
 * ToDo:
 *
 * Line mode
 *
 */


/*
 * i0 = dmaA  i1 = dmaB  i2 = dmaC  i3 = dmaD  i4 = fill carry i5 = vert cnt
 * o0 = srcA  o1 = srcB  o2 = srcC  o3 = dest  o4 = zero test  o5 = horiz cnt
 * l0 = oldA  l1 = oldB
 * g2, g3 = temp (minterm)
 */


static sparc_instr *mkblit1w(sparc_instr *code, sparc_instr *hloop,
	u16 bltcon0, u16 bltcon1, u16 amask)
{
  sparc_instr *mintermcode;

  if(((bltcon0>>4)^bltcon0)&0xf) {

    /* Get source A */

    if(bltcon0 & 0x0800) {
      *code++ = 0xd0160000; /* lduh [%i0],%o0 */
      if(bltcon1 & 2)
	*code++ = 0xb0063ffe; /* add -2,%i0,%i0 */
      else
	*code++ = 0xb0062002; /* add 2,%i0,%i0 */
    } else {
      *code++ = 0x90100018; /* mov %i0,%o0 */
    }

    /* Mask source A */

    if(amask!=0xffff) {
      if(((s16)amask)>=-4096 && ((s16)amask)<=4095)
	*code++ = 0x900a2000|(amask&0x1fff); /* and %o0,amask,%o0 */
      else {
	*code++ = 0x15000000|(amask>>10); /* sethi %hi(amask),%o2 */
	if(amask&0x3ff)
	  *code++ = 0x9412a000|(amask&0x3ff); /* or %o2,%lo(amask),%o2 */
	*code++ = 0x900a000a; /* and %o0,%o2,%o0 */
      }
    }

  } else if(bltcon0 & 0x0800) {

    /* Just fake the DMA transfer */

    if(bltcon1 & 2)
      *code++ = 0xb0063ffe; /* add -2,%i0,%i0 */
    else
      *code++ = 0xb0062002; /* add 2,%i0,%i0 */
  }

  /* Preshift source A */

  if(bltcon0&0xf000)
    if(bltcon1 & 2) {
      *code++ = 0x852a2010; /* sll %o0,16,%g2 */
      *code++ = 0x84108010; /* or  %g2,%l0,%g2 */
      *code++ = 0xa0120000; /* mov %o0,%l0 */
      *code++ = 0x9130a000|(16-(bltcon0>>12)); /* srl %g2,16-n,%o0 */
    } else {    
      *code++ = 0x90120010; /* or %o0,%l0,%o0 */
      *code++ = 0xa12a2010; /* sll %o0,16,%l0 */
      *code++ = 0x91322000|(bltcon0>>12); /* srl %o0,n,%o0 */
    }

  /* Get source B */

  if(bltcon0 & 0x0400) {
    if(((bltcon0>>2)^bltcon0)&0x33)
      *code++ = 0xd2164000; /* lduh [%i1],%o1 */
    if(bltcon1 & 2)
      *code++ = 0xb2067ffe; /* add -2,%i1,%i1 */
    else
      *code++ = 0xb2066002; /* add 2,%i1,%i1 */
  } else {
    if(((bltcon0>>2)^bltcon0)&0x33)
      *code++ = 0x92100019; /* mov %i1,%o1 */
  }

  /* Preshift source B */

  if(bltcon1&0xf000)
    if(bltcon1 & 2) {
      *code++ = 0x852a6010; /* sll %o1,16,%g2 */
      *code++ = 0x84108011; /* or  %g2,%l1,%g2 */
      *code++ = 0xa2124000; /* mov %o1,%l1 */
      *code++ = 0x9330a000|(16-(bltcon1>>12)); /* srl %g2,16-n,%o1 */
    } else {    
      *code++ = 0x92124011; /* or %o1,%l1,%o1 */
      *code++ = 0xa32a6010; /* sll %o1,16,%l1 */
      *code++ = 0x93326000|(bltcon1>>12); /* srl %o1,n,%o1 */
    }

  /* Get source C */

  if(bltcon0 & 0x0200) {
    if(((bltcon0>>1)^bltcon0)&0x55)
      *code++ = 0xd4168000; /* lduh [%i2],%o2 */
    if(bltcon1 & 2)
      *code++ = 0xb406bffe; /* add -2,%i2,%i2 */
    else
      *code++ = 0xb406a002; /* add 2,%i2,%i2 */
  } else {
    if(((bltcon0>>1)^bltcon0)&0x55)
      *code++ = 0x9410001a; /* mov %i2,%o2 */
  }

  /* Build %o3 from %o0-%o2 using %g2-%g3 as scratch registers */

  mintermcode=minterms[bltcon0&0xff];
  while((*code++=*mintermcode++));
  --code;

  /* Do fill */

  if(bltcon1 & 0x0018) {
    *code++ = 0x9010200f; /* mov 15,%o0 */
    *code++ = 0x92102001; /* mov 1,%o1 */
    *code++ = 0x940a400b; /* and %o1,%o3,%o2 */
    if(bltcon1 & 0x0008)
      *code++ = 0x9617000b; /* or %i4,%o3,%o3 */
    else
      *code++ = 0x961f000b; /* xor %i4,%o3,%o3 */
    *code++ = 0xb81a801c; /* xor %o2,%i4,%i4 */
    *code++ = 0x932a6001; /* sll %o1,1,%o1 */
    *code++ = 0x90823fff; /* addcc %o0,-1,%o0 */
    *code++ = 0x1cbffffb; /* bpos fill_loop */
    *code++ = 0xb92f2001; /* sll %i4,1,%i4 */
    *code++ = 0xb9372010; /* srl %i4,16,%i4 */
  }

  /* Save result */

  if(bltcon0 & 0x0100) {
    *code++ = 0xd636c000; /* sth %o3,[%i3] */
    if(bltcon1 & 2)
      *code++ = 0xb606fffe; /* add -2,%i3,%i3 */
    else
      *code++ = 0xb606e002; /* add 2,%i3,%i3 */
  }

  /* Do loop */

  if(hloop) {
    *code++ = 0x9a837fff; /* addcc %o5,-1,%o5 */
    *code++ = 0x1cbfffff&(hloop-code+1); /* bpos hloop */
  }

  /* Check zeros */

  *code++ = 0x9813000b; /* or %o4,%o3,%o4 */

  return code;
}

void doblit_sync()
{
  u16 (*start)(u16*, u16*, u16*, u16*);
  sparc_instr *code=code_space, *vloop;
  u16 bltadat, bltbdat, bltcdat;
  u32 bltapt, bltbpt, bltcpt, bltdpt;
  s16 bltamod, bltbmod, bltcmod, bltdmod;
  u16 bltafwm, bltalwm, bltsize, bltcon0, bltcon1;
  int width, height;

  start = (u16 (*)(u16*, u16*, u16*, u16*))code;

  mutex_lock(&dmaconr_lock);

  DMACONR |= 0x4000;

  bltadat=BLTADAT; bltbdat=BLTBDAT; bltcdat=BLTCDAT;
  bltapt=BLTAPT; bltbpt=BLTBPT; bltcpt=BLTCPT; bltdpt=BLTDPT;
  bltamod=BLTAMOD; bltbmod=BLTBMOD; bltcmod=BLTCMOD; bltdmod=BLTDMOD;
  bltafwm=BLTAFWM; bltalwm=BLTALWM;
  bltcon0=BLTCON0; bltcon1=BLTCON1; bltsize=BLTSIZE;

  __asm("flush %g0");

  cond_broadcast(&blitter_state);
  mutex_unlock(&dmaconr_lock);

  *code++ = 0x9de3bf90; /* save %sp,-112,%sp */
  *code++ = 0x98100000; /* clr %o4 */

  if(!(width = (bltsize&0x3f))) width=64;
  if(!(height = (bltsize>>6))) height=1024;

  if(bltcon0&0xf000)
    *code++ = 0xa0100000; /* clr %l0 */

  if(bltcon1&0xf000)
    *code++ = 0xa2100000; /* clr %l1 */

  *code++ = 0xba102000|(height-1); /* mov height-1,%i5 */
  vloop = code;

  if(bltcon1 & 0x18)
    *code++ = 0xb8102000|((bltcon1>>2)&1); /* mov blitfc,%i4 */

  if(width==1)
    code = mkblit1w(code, NULL, bltcon0, bltcon1, bltafwm&bltalwm);
  else {
    code = mkblit1w(code, NULL, bltcon0, bltcon1, bltafwm);
    if(width>3) {
      *code++ = 0x9a102000|(width-1); /* mov width-1,%o5 */
      code = mkblit1w(code, code, bltcon0, bltcon1, ~0);
    } else if(width==3)
      code = mkblit1w(code, NULL, bltcon0, bltcon1, ~0);
    code = mkblit1w(code, NULL, bltcon0, bltcon1, bltalwm);
  }

  /* Add source A modulo */

  if(bltamod && (bltcon0 & 0x0800))
    if(bltamod>=-4096 && bltamod<=4095)
      *code++ = 0xb0062000|(bltamod&0x1fff); /* add %i0,bltamod,%i0 */
    else {
      *code++ = 0x11000000|(bltamod>>10); /* sethi %hi(bltamod),%o0 */
      if(bltamod&0x3ff)
        *code++ = 0x90122000|(bltamod&0x3ff); /* or %o0,%lo(bltamod),%o0 */
      *code++ = 0xb0060008; /* add %i0,%o0,%i0 */
    }

  /* Add source B modulo */

  if(bltbmod && (bltcon0 & 0x0400))
    if(bltbmod>=-4096 && bltbmod<=4095)
      *code++ = 0xb2066000|(bltbmod&0x1fff); /* add %i1,bltbmod,%i1 */
    else {
      *code++ = 0x11000000|(bltbmod>>10); /* sethi %hi(bltbmod),%o0 */
      if(bltbmod&0x3ff)
        *code++ = 0x90122000|(bltbmod&0x3ff); /* or %o0,%lo(bltbmod),%o0 */
      *code++ = 0xb2064008; /* add %i1,%o0,%i1 */
    }

  /* Add source C modulo */

  if(bltcmod && (bltcon0 & 0x0200))
    if(bltcmod>=-4096 && bltcmod<=4095)
      *code++ = 0xb406a000|(bltcmod&0x1fff); /* add %i2,bltcmod,%i2 */
    else {
      *code++ = 0x11000000|(bltcmod>>10); /* sethi %hi(bltcmod),%o0 */
      if(bltcmod&0x3ff)
        *code++ = 0x90122000|(bltcmod&0x3ff); /* or %o0,%lo(bltcmod),%o0 */
      *code++ = 0xb4068008; /* add %i2,%o0,%i2 */
    }

  /* Add destination modulo */

  if(bltdmod && (bltcon0 & 0x0100))
    if(bltdmod>=-4096 && bltdmod<=4095)
      *code++ = 0xb606e000|(bltdmod&0x1fff); /* add %i3,bltdmod,%i3 */
    else {
      *code++ = 0x11000000|(bltdmod>>10); /* sethi %hi(bltdmod),%o0 */
      if(bltdmod&0x3ff)
        *code++ = 0x90122000|(bltdmod&0x3ff); /* or %o0,%lo(bltdmod),%o0 */
      *code++ = 0xb606c008; /* add %i3,%o0,%i3 */
    }


  *code++ = 0xba877fff; /* addcc %i5,-1,%i5 */
  *code++ = 0x1cbfffff&(vloop-code+1); /* bpos vloop */
  *code++ = 0x01000000; /* nop */

  *code++ = 0xb010000c; /* mov %o4,%i0 */
  *code++ = 0x81c7e008; /* ret */
  *code   = 0x81e80000; /* restore */

  if(start(((bltcon0 & 0x0800)? BLITADDR(bltapt):(u16*)(u32)bltadat),
	   ((bltcon0 & 0x0400)? BLITADDR(bltbpt):(u16*)(u32)bltbdat),
	   ((bltcon0 & 0x0200)? BLITADDR(bltcpt):(u16*)(u32)bltcdat),
	   BLITADDR(bltdpt))) {
    mutex_lock(&dmaconr_lock);
    DMACONR &= ~0x6000;
  } else {
    mutex_lock(&dmaconr_lock);
    DMACONR |=  0x2000;
    DMACONR &= ~0x4000;
  }
  cond_broadcast(&blitter_state);
  mutex_unlock(&dmaconr_lock);
}

void doblit_async()
{
  mutex_lock(&blitter_starter);
  mutex_lock(&dmaconr_lock);
  while(DMACONR&0x4000) cond_wait(&blitter_state, &dmaconr_lock);
  sema_post(&blitter_go);
  cond_wait(&blitter_state, &dmaconr_lock);
  mutex_unlock(&dmaconr_lock);
  mutex_unlock(&blitter_starter);
}

static void *blit_server(void *dummy)
{
  for(;;) {
    sema_wait(&blitter_go);
    doblit_sync();
  }
}

void init_blitter()
{
  code_space=(sparc_instr *)mmap(NULL, 4096, PROT_READ|PROT_WRITE|PROT_EXEC,
				 MAP_PRIVATE, devzero, 0);
  if(code_space==MAP_FAILED) {
    perror("init_blitter:mmap");
    exit(1);
  }
  if(mutex_init(&dmaconr_lock, USYNC_THREAD, 0)<0 ||
     mutex_init(&blitter_starter, USYNC_THREAD, 0)<0 ||
     cond_init(&blitter_state, USYNC_THREAD, 0)<0 ||
     sema_init(&blitter_go, 0, USYNC_THREAD, 0)<0) {
    perror("init_blitter");
    exit(1);
  }
  if(thr_create(NULL, thr_min_stack()+0x1000, blit_server, NULL,
		THR_BOUND, &blitter_thread)<0) {
    perror("init_blitter:thr_create");
    exit(1);
  }
}

void await_blitter()
{
  mutex_lock(&dmaconr_lock);
  while(DMACONR&0x4000) cond_wait(&blitter_state, &dmaconr_lock);
  mutex_unlock(&dmaconr_lock);
}
