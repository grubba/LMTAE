/*
 * $Id: citest.c,v 1.1.1.1 1996/06/30 23:51:51 grubba Exp $
 *
 * Codeinfo tester
 *
 * $Log: citest.c,v $
 * Revision 1.2  1996/06/19 11:08:25  grubba
 * Some minor bugs fixed.
 *
 * Revision 1.1  1996/05/11 17:20:01  grubba
 * Initial revision
 *
 */

/*
 * Includes
 */

#include <stdio.h>

#include "recomp.h"
#include "m68k.h"
#include "codeinfo.h"

/*
 * Functions
 */

int main(int argc, char **argv)
{
  struct seg_info *seg_tree = NULL;
  char buf[1024];

  dump_seg_tree(seg_tree);

  printf("<>");
  fflush(stdout);

  while (fgets(buf, 1023, stdin)) {
    ULONG start, end;
    
    if (buf[0] == 'h') {
      printf("a <start> <end>\tAdd the range from <start> to <end>\n");
      printf("d\t\tDump tree\n");
      printf("D <val>\t\tDelete the node with the range containing <val>\n");
      printf("f <start>\tFind the range containing <start>\n");
      printf("h\t\tHelp\n");
      printf("q\t\tQuit\n");
    } else if (buf[0] == 'd') {
      dump_seg_tree(seg_tree);
    } else if (buf[0] == 'q') {
      exit(0);
    } else if (2==sscanf(buf, "a %ld %ld\n", &start, &end)) {
      insert_seg(&seg_tree, start, end);
      dump_seg_tree(seg_tree);
    } else if (1==sscanf(buf, "f %ld\n", &start)) {
      dump_seg_tree(find_seg(seg_tree, start));
    } else if (1==sscanf(buf, "D %ld\n", &start)) {
      struct seg_info *seg = find_seg(seg_tree, start);
      if (seg) {
	destroy_seg(&seg_tree, seg);
      }
      dump_seg_tree(seg_tree);
    }

    printf("<>");
    fflush(stdout);
  }
  exit(0);
}



