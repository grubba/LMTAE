#include <stdio.h>
#include <stdlib.h>

int srtest(int sr);

int main(int argc, char **argv)
{
  int i;

  for (i=0x00100000;i;i<<=1) {
    printf("0x%08x => 0x%04x\n", i, srtest(i));
  }
  exit(0);
}

