/*
 * $Id: codeinfo.h,v 1.2 1996/07/01 19:16:40 grubba Exp $
 *
 * Functions for handling segments of code.
 *
 * $Log: codeinfo.h,v $
 * Revision 1.1.1.1  1996/06/30 23:51:51  grubba
 * Entry into CVS
 *
 * Revision 1.4  1996/06/19 11:08:25  grubba
 * Some minor bugs fixed.
 *
 * Revision 1.3  1996/06/01 09:31:29  grubba
 *  Major changes.
 * Now generates output split into several files.
 * Now generates gasp assembler preprocessor output.
 * Now uses templates to build instructions.
 *
 * Revision 1.2  1996/05/11 17:23:27  grubba
 * parent pointer added to the code_info to simplify deletion, it's hard enough anyway.
 *
 * Revision 1.1  1996/05/08 07:24:22  grubba
 * Initial revision
 *
 */

#ifndef CODEINFO_H
#define CODEINFO_H

/*
 * Structures
 */

/* mmin field also holds the color BLACK/RED */

struct seg_info {
  struct seg_info *parent, *left, *right;
  ULONG mmin, maddr, mend, mmax;
  struct code_info *codeinfo;
};

#define SEG_COLOR_RED 0x80000000

struct code_info {
  struct code_info *next;
  ULONG maddr;
  ULONG flags;
  ULONG (*code)(struct m_registers *, void *);	/* Returns the next PC */
  ULONG *codeend;
  ULONG checksum;
};

/*
 * Prototypes
 */

struct seg_info *find_seg(struct seg_info *root, ULONG maddr);
struct seg_info *insert_seg(struct seg_info **root, ULONG maddr, ULONG mend);
void dump_seg_tree(struct seg_info *root);

struct code_info *find_ci(struct code_info **head, ULONG maddr);

#endif
