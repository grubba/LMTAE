#include "blitter.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>

int devzero;
volatile u8 *chipmem;
volatile u16 *custom;

double timediff(struct timeval *a, struct timeval *b)
{
  return (a->tv_sec-b->tv_sec)+(a->tv_usec-b->tv_usec)/1000000.0;
}

void testit()
{
  int i;
  struct timeval start, stop;
  int startc, stopc;

  for(i=0; i<256; i++) chipmem[i]=i;

  BLTCON0 = 0x19c0;
  BLTCON1 = 0x1000;
  BLTAFWM = BLTALWM = 0xffff;
  BLTAPT = 0;
  BLTDPT = 0x4000;
  BLTAMOD = 6;
  BLTDMOD = 14;
  BLTBDAT = 0x04fe;
  BLTSIZE = (9<<6)|3;
  doblit_async();

  BLTCON0 = 0x090f;
  BLTCON1 = 0x0000;
  BLTAFWM = BLTALWM = 0xffff;
  BLTAPT = 0x4002;
  BLTDPT = 0x4002;
  BLTAMOD = 18;
  BLTDMOD = 18;
  BLTSIZE = (9<<6)|1;
  doblit_async();

  for(i=0; i<200; i++) {
    printf("%02x", chipmem[i+0x4000]);
    if(i%20==19)
      printf("\n");     
  }

  printf("16x16 pixels: "); fflush(stdout);
  gettimeofday(&start); startc=clock();
  for(i=0;i<5000;i++) {
    BLTCON0 = 0x09f0;
    BLTCON1 = 0x0000;
    BLTAFWM = BLTALWM = 0xffff;
    BLTAPT = 0;
    BLTDPT = 0x100000;
    BLTAMOD = 0;
    BLTDMOD = 0;
    BLTSIZE = (16<<6)|(16>>4);
    doblit_async();
  }
  await_blitter();
  gettimeofday(&stop); stopc=clock();
  printf("%f pixels/second\n", 16*16*5000/timediff(&stop,&start));
  printf("CPU: %f pixels/second\n", 16*16*5000/(1.0*(stopc-startc)/CLOCKS_PER_SEC));

  printf("80x80 pixels: "); fflush(stdout);
  gettimeofday(&start); startc=clock();
  for(i=0;i<1000;i++) {
    BLTCON0 = 0x09f0;
    BLTCON1 = 0x0000;
    BLTAFWM = BLTALWM = 0xffff;
    BLTAPT = 0;
    BLTDPT = 0x100000;
    BLTAMOD = 0;
    BLTDMOD = 0;
    BLTSIZE = (80<<6)|(80>>4);
    doblit_async();
  }
  await_blitter();
  gettimeofday(&stop); stopc=clock();
  printf("%f pixels/second\n", 80*80*1000/timediff(&stop,&start));
  printf("CPU: %f pixels/second\n", 80*80*1000/(1.0*(stopc-startc)/CLOCKS_PER_SEC));

  printf("400x400 pixels: "); fflush(stdout);
  gettimeofday(&start); startc=clock();
  for(i=0;i<100;i++) {
    BLTCON0 = 0x09f0;
    BLTCON1 = 0x0000;
    BLTAFWM = BLTALWM = 0xffff;
    BLTAPT = 0;
    BLTDPT = 0x100000;
    BLTAMOD = 0;
    BLTDMOD = 0;
    BLTSIZE = (400<<6)|(400>>4);
    doblit_async();
  }
  await_blitter();
  gettimeofday(&stop); stopc=clock();
  printf("%f pixels/second\n", 400*400*100/timediff(&stop,&start));
  printf("CPU: %f pixels/second\n", 400*400*100/(1.0*(stopc-startc)/CLOCKS_PER_SEC));
}

int main(int argc, char *argv[])
{
  if((devzero=open("/dev/zero", O_RDONLY))>=0) {
    if(MAP_FAILED==(chipmem=(u8*)mmap(NULL, 1<<24, PROT_NONE,
				      MAP_PRIVATE|MAP_NORESERVE, devzero, 0))){
      perror("mmap(addrspace)");
      exit(1);
    }
    if(MAP_FAILED==mmap((caddr_t)chipmem, 2<<20,
			PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED,
			devzero, 0)) {
      perror("mmap(chipmem)");
      exit(1);
    }
    if(MAP_FAILED==(custom=(u16*)mmap(((caddr_t)chipmem)+0xdfe000, 0x2000,
				      PROT_READ|PROT_WRITE,
				      MAP_PRIVATE|MAP_FIXED,
				      devzero, 0))) {
      perror("mmap(custom)");
      exit(1);
    }
    custom+=0x1000/2;
    init_blitter();

    testit();

    close(devzero);
  } else perror("/dev/zero");
  return 0;
}
