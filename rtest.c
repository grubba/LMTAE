/*
 * $Id: rtest.c,v 1.4 1996/07/17 19:16:32 grubba Exp $
 *
 * Test rpogram for the M68000 to Sparc recompiler.
 *
 * $Log: rtest.c,v $
 * Revision 1.3  1996/07/17 16:01:51  grubba
 * Changed from {U,}{LONG,WORD,BYTE} to [SU]{8,16,32}.
 * Hopefully all places got patched.
 *
 * Revision 1.2  1996/07/13 19:32:19  grubba
 * Now defaults to very little debuginfo.
 * Added (un|set)patch().
 * Patches added to MakeLibrary(), MakeFunctions(), Abort() and AddLibrary().
 *
 * Revision 1.1.1.1  1996/06/30 23:51:53  grubba
 * Entry into CVS
 *
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

#include "types.h"

#include "recomp.h"
#include "codeinfo.h"

/*
 * Globals
 */

unsigned char *memory = NULL;

U32 debuglevel = ~0;

/*
 * Stubs
 */

void reset_hw(void){}
U32 s_stop(struct m_registers *regs, U8 *mem, U32 nextpc){}
int load_hw_byte(){}
int load_hw_short(){}
int load_hw(){}
void store_hw_byte(U32 maddr, U8 val){}
void store_hw_short(U32 maddr, U16 val){}
void store_hw(U32 maddr, U32 val){}
U32 clobber_code_byte(U32 maddr, U8 val){}
U32 clobber_code_short(U32 maddr, U16 val){}
void clobber_code(U32 maddr, U32 val){}

U32 raise_exception(struct m_registers *regs, U16 *mem, U32 vec){return 0;}

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
	      U32 mend;
	      U32 *dump;

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
