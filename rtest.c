/*
 * $Id: rtest.c,v 1.1.1.1 1996/06/30 23:51:53 grubba Exp $
 *
 * Test rpogram for the M68000 to Sparc recompiler.
 *
 * $Log: rtest.c,v $
 * Revision 1.2  1996/06/19 11:08:25  grubba
 * *** empty log message ***
 *
 * Revision 1.1  1996/06/01 09:31:29  grubba
 * Initial revision
 *
 *
 */

/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "recomp.h"
#include "codeinfo.h"

/*
 * Globals
 */

unsigned char *memory = NULL;

/*
 * Stubs
 */

void reset_hw(void){}
int load_hw_byte(){}
int load_hw_short(){}
int load_hw(){}
void store_hw_byte(ULONG maddr, UBYTE val){}
void store_hw_short(ULONG maddr, USHORT val){}
void store_hw(ULONG maddr, ULONG val){}
ULONG clobber_code_byte(ULONG maddr, UBYTE val){}
ULONG clobber_code_short(ULONG maddr, USHORT val){}
void clobber_code(ULONG maddr, ULONG val){}

volatile void raise_exception(struct m_registers *regs, USHORT *mem, ULONG vec){}

/*
 * Functions
 */

int main(int argc, char **argv)
{
  int zfd = -1;
  int romfd = -1;
  unsigned char *rommem;

  if ((zfd = open("/dev/zero", O_RDONLY)) >= 0) {
    if ((memory = mmap(NULL, 16*1024*1024, PROT_READ|PROT_WRITE,
		       MAP_PRIVATE, zfd, 0))) {
      if ((romfd = open("./ROM.dump", O_RDONLY)) >= 0) {
	if ((rommem = mmap(memory + 0x00f80000, 512*1024, PROT_READ,
			   MAP_SHARED|MAP_FIXED, romfd, 0))) {
	  if (rommem == memory + 0x00f80000) {
	    struct code_info ci = {
	      NULL,
	      0xf800d2,
	      0,
	      NULL,
	      NULL,
	      0,
	    };
	    
	    if ((ci.code = (void *)calloc(4, 1024))) {
	      ULONG mend;
	      ULONG *dump;

	      printf("Compiling...\n");
	      mend = compile(&ci);

	      printf("Motorola code from 0x00f800d2 to 0x%08x\n", mend);
	      printf("Sparc code from 0x%08x to 0x%08x\n", ci.code, ci.codeend);
	      for (dump = (void *)ci.code; dump <= ci.codeend;) {
		int j;
		printf("%08x: ", dump);
		for (j = 0; j < 8; j++) {
		  printf("%08x ", *(dump++));
		}
		printf("\n");
	      }
	    }
	  } else {
	    fprintf(stderr, "Bad ROM-map\n");
	  }
	} else {
	  fprintf(stderr, "Couldn't mmap ./ROM.dump\n");
	}
      } else {
	fprintf(stderr, "Couldn't open ./ROM.dump\n");
      }
    } else {
      fprintf(stderr, "Couldn't mmap /dev/zero\n");
    }
  } else {
    fprintf(stderr, "Couldn't open /dev/zero\n");
  }
  return(0);
}
