/*
 * $Id: hardfile.c,v 1.1 1996/07/12 13:09:09 marcus Exp $
 *
 * $Log: $
 */

/*
 * Includes
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "common.h"
#include "recomp.h"

int numunits, *fds;

static struct {
  u16 unit, cntrl;
  u32 addr, offs, len;
} board_reg;

static int errno2ioerr(int e)
{
  switch(e) {
  case EBUSY:
    return -6;
  case EACCES:
    return 28;
  case ENOMEM:
    return 31;
  default:
    return 20;
  }
}

static void do_command()
{
  int r, fd, u=board_reg.unit;

  if(u<0 || u>=numunits) {
    board_reg.len=0;
    board_reg.cntrl=32;
    return;
  }
  fd=fds[u];
  lseek(fd, board_reg.offs, SEEK_SET);
  if(board_reg.cntrl==2)
    r=read(fd, board_reg.addr+memory, board_reg.len);
  else
    r=write(fd, board_reg.addr+memory, board_reg.len);
  if(r>=0) {
    board_reg.addr+=r;
    board_reg.offs+=r;
    board_reg.len=r;
    board_reg.cntrl=0;
  } else {
    board_reg.len=0;
    board_reg.cntrl=errno2ioerr(errno);
  }
}

void board_writeword(u32 offset, u16 value)
{
  offset-=128;
  if(offset>=0 && offset<sizeof(board_reg)) {
    *(u16*)((u8*)&board_reg+offset)=value;
    if(offset==2)
      do_command();
  }
}

int board_readword(u32 offset, u16 *value)
{
  offset-=128;
  if(offset>=0 && offset<sizeof(board_reg)) {
    *value=*(u16*)((u8*)&board_reg+offset);
    return 1;
  }
  return 0;
}

void board_reset()
{
  board_reg.len=numunits;
}

void board_init()
{
  static int foo[1];

  numunits=1;
  fds=foo;
  if((fds[1]=open("hardfile0", O_RDWR))<0)
    perror("hardfile0");
}
