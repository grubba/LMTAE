/*
 * $Id: zorro.c,v 1.1 1996/07/11 23:02:08 marcus Exp $
 *
 * $Log: $
 *
 */


/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <dlfcn.h>

#include "common.h"
#include "recomp.h"


/*
 * Some hardcoded ZorroII characteristical values
 */

#define Z2_MINSLOT 0xe8
#define Z2_MAXSLOT 0xef
#define Z2_CONFSLOT 0xe8

#define SLOT2MEM(x) ((x)<<16)
#define MEM2SLOT(x) ((x)>>16)

/*
 * A struct describing a single AutoConfig(TM) board, memory or otherwise
 */

static struct board {
  struct board *next;
  u8 type, product;
  int number;
  char *name;
  int numslots, mappedslots;
  int baseslot;
  void *mapped_at;
  int rom_fd;
  void *driver;
  void (*func_reset)();
  void (*func_setaddr)(void *);
  void (*func_writebyte)(u32, u8);
  void (*func_writeword)(u32, u16);
  int (*func_readbyte)(u32, u8 *);
  int (*func_readword)(u32, u16 *);
} *boards=NULL, *lastboard=NULL, *z2map[Z2_MAXSLOT-Z2_MINSLOT+1];


/*
 * Level of verbosity for Zorro emulation
 */

int zorro_verbose=1;

/*
 * ZorroII emulation functions
 */

static void make_ramboard(struct board *b)
{
  static u8 exprom[] = { 0, 0, 0x07, 0xdb, 0, 0,
			 0, 1, 0, 0, 0, 0, 0, 0 };
  u16 *p=b->mapped_at;
  int i;

  *p++=(b->type&0xf0)<<8;
  *p++=(b->type&0x0f)<<12;
  *p++=((~b->product)&0xf0)<<8;
  *p++=((~b->product)&0x0f)<<12;
  for(i=0; i<sizeof(exprom); i++) {
    *p++=((~exprom[i])&0xf0)<<8;
    *p++=((~exprom[i])&0x0f)<<12;
  }
}

static void mapboard(struct board *b, int slot);

static void unmapboard(struct board *b)
{
  if(b->mapped_at) {
    int s;
    if(b->func_setaddr)
      b->func_setaddr(NULL);
    if(b->baseslot>=Z2_MINSLOT && b->baseslot<=Z2_MAXSLOT)
      for(s=0; s<b->numslots; s++)
        z2map[s+b->baseslot-Z2_MINSLOT]=NULL;
    if(mmap((caddr_t)b->mapped_at, SLOT2MEM(b->mappedslots),
	    PROT_READ, MAP_PRIVATE, devzero, 0)==MAP_FAILED)
      perror("Zorro board unmap");
    b->mapped_at=NULL;
    if(b->baseslot==Z2_CONFSLOT && b->next)
      mapboard(b->next, Z2_CONFSLOT);
    b->baseslot=0;
  }
}

static void mapboard(struct board *b, int slot)
{
  int s;

  unmapboard(b);
  b->mappedslots=b->numslots;
  if(slot==Z2_CONFSLOT)
    b->mappedslots=1;
  b->baseslot=slot;
  b->mapped_at=mmap((caddr_t)(memory+SLOT2MEM(slot)), SLOT2MEM(b->mappedslots),
		    (b->rom_fd==devzero?(PROT_READ|PROT_WRITE):PROT_READ),
		    MAP_FIXED|MAP_PRIVATE, b->rom_fd, 0);
  if(b->mapped_at==MAP_FAILED) {
    perror("Zorro board mmap");
    b->baseslot=0;
    b->mappedslots=0;
    b->mapped_at=NULL;
    return;
  } 
  if(b->rom_fd==devzero && slot==Z2_CONFSLOT)
     make_ramboard(b);
  if(slot>=Z2_MINSLOT && slot<=Z2_MAXSLOT)
    for(s=0; s<b->mappedslots; s++) {
      if(z2map[s+b->baseslot-Z2_MINSLOT])
	unmapboard(z2map[s+b->baseslot-Z2_MINSLOT]);
      z2map[s+b->baseslot-Z2_MINSLOT]=b;
    }
  if(b->func_setaddr)
    b->func_setaddr(b->mapped_at);
  if(zorro_verbose)
    fprintf(stderr, "Board %d (%s) mapped at %08x\n",
		b->number, b->name, SLOT2MEM(slot));
}

void zorro_reset(u32 base)
{
  struct board *b;

  for(b=boards; b; b=b->next) {
    if(zorro_verbose)
      fprintf(stderr, "Resetting board %d (%s)\n",
	      b->number, b->name);
    if(b->mapped_at)
      unmapboard(b);
    if(b->func_reset)
      b->func_reset();
  }
  if(boards)
    mapboard(boards, Z2_CONFSLOT);
}

void zorro_writebyte(u32 addr, u8 value, u32 base)
{
  int slot=MEM2SLOT(addr);
  struct board *b;

  if((slot>=Z2_MINSLOT && slot<=Z2_MAXSLOT) &&
	(b=z2map[slot-Z2_MINSLOT])) {
    u32 offs=addr-SLOT2MEM(slot);
    if(offs<128) {
      if((offs & 3) || offs<64)
	return;
      switch((offs-64)>>2) {
	case 2:
	  mapboard(b, value);
	  break;
	case 3:
	  if(zorro_verbose)
	    fprintf(stderr, "Shutting up board %d (%s)\n",
			b->number, b->name);
	  unmapboard(b);
	  break;
	default:
	  if(zorro_verbose)
	    fprintf(stderr, "Board %d (%s): write to ExpansionControl "
			"register %ld: %02x\n", b->number, b->name,
			(offs-64)>>2, value);
      }
    } else if(b->func_writebyte)
      b->func_writebyte(offs, value);
  }
}

void zorro_writeword(u32 addr, u16 value, u32 base)
{
  int slot=MEM2SLOT(addr);
  struct board *b;

  if(!(addr&1) && (slot>=Z2_MINSLOT && slot<=Z2_MAXSLOT) &&
     (b=z2map[slot-Z2_MINSLOT])) {
    u32 offs=addr-SLOT2MEM(slot);
    if(b->func_writebyte)
      b->func_writebyte(offs, value);
  }
}

void zorro_writelong(u32 addr, u32 value, u32 base)
{
  zorro_writeword(addr, value>>16, base);
  zorro_writeword(addr+2, value&0xffff, base);
}

u8 zorro_readbyte(u32 addr, u32 base)
{
  int slot=MEM2SLOT(addr);
  struct board *b;

  if((slot>=Z2_MINSLOT && slot<=Z2_MAXSLOT) &&
     (b=z2map[slot-Z2_MINSLOT])) {
    u32 offs=addr-SLOT2MEM(slot);
    u8 value;
    if(b->func_readbyte && b->func_readbyte(offs, &value))
      return value;
    return *(u8 *)(memory+addr);
  }
  return 0;
}

u16 zorro_readword(u32 addr, u32 base)
{
  int slot=MEM2SLOT(addr);
  struct board *b;

  if(!(addr&1) && (slot>=Z2_MINSLOT && slot<=Z2_MAXSLOT) &&
     (b=z2map[slot-Z2_MINSLOT])) {
    u32 offs=addr-SLOT2MEM(slot);
    u16 value;
    if(b->func_readword && b->func_readword(offs, &value))
      return value;
    return *(u16 *)(memory+addr);
  }
  return 0;
}

u32 zorro_readlong(u32 addr, u32 base)
{
  return (zorro_readword(addr, base)<<16)|zorro_readword(addr+2, base);
}

static struct board *createboard(char *name)
{
  static int serial=0;
  struct board *b;

  if((b=calloc(1, sizeof(struct board)))) {
    b->next=NULL;
    b->number=serial++;
    b->name=strdup(name);
    b->mapped_at=NULL;
    b->rom_fd=-1;
    b->driver=NULL;
    b->func_reset=NULL;
    b->func_setaddr=NULL;
    b->func_writebyte=NULL;
    b->func_writeword=NULL;
    b->func_readbyte=NULL;
    b->func_readword=NULL;
    if(lastboard)
      lastboard->next=b;
    else
      boards=b;
    lastboard=b;
  }
  return b;
}

static int sizemap[8]={ 128, 1, 2, 4, 8, 16, 32, 64 };

void zorro_addram(int megs)
{
  struct board *b;
  char name[16];

  sprintf(name, "%dmeg RAM", megs);
  if((b=createboard(name))) {
    int i;
    b->type=0xe0;  /* ZorroII, memory board, no diag vector, not chained */
    b->product=megs;
    b->numslots=megs<<4;
    for(i=7; i; --i)
      if(sizemap[i]==b->numslots)
	b->type|=i;
    b->rom_fd=devzero;
  }
}

void zorro_addboard(char *name, int fd, void *driver)
{
  struct board *b;

  if((b=createboard(name))) {
    if((b->rom_fd=fd)>=0) {
      u16 hdr[4];
      read(fd, hdr, sizeof(hdr));
      b->type=((hdr[0]>>8)&0xf0)|(hdr[1]>>12);
      b->product=~(((hdr[2]>>8)&0xf0)|(hdr[3]>>12));
      b->numslots=sizemap[b->type&7];
    }
    if((b->driver=driver)) {
      void (*init_func)();
      b->func_reset=(void (*)())dlsym(driver, "board_reset");
      b->func_setaddr=(void (*)(void *))dlsym(driver, "board_setaddr");
      b->func_writebyte=(void (*)(u32, u8))dlsym(driver, "board_writebyte");
      b->func_writeword=(void (*)(u32, u16))dlsym(driver, "board_writeword");
      b->func_readbyte=(int (*)(u32, u8 *))dlsym(driver, "board_readbyte");
      b->func_readword=(int (*)(u32, u16 *))dlsym(driver, "board_readword");
      if((init_func=(void (*)())dlsym(driver, "board_init")))
	init_func();
    }
  }
}

