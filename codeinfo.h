/*
 * $Id: codeinfo.h,v 1.5 1996/08/11 17:36:08 grubba Exp $
 *
 * Functions for handling segments of code.
 *
 * $Log: codeinfo.h,v $
 * Revision 1.4  1996/07/17 16:01:09  grubba
 * Changed from {U,}{LONG,WORD,BYTE} to [SU]{8,16,32}.
 * Hopefully all places got patched.
 *
 * Revision 1.3  1996/07/13 19:32:23  grubba
 * Now defaults to very little debuginfo.
 * Added (un|set)patch().
 * Patches added to MakeLibrary(), MakeFunctions(), Abort() and AddLibrary().
 *
 * Revision 1.2  1996/07/01 19:16:40  grubba
 * Implemented ASL and ASR.
 * Changed semantics for new_codeinfo(), it doesn't allocate space for the code.
 * Added PeepHoleOptimize(). At the moment it just mallocs and copies the code.
 * Removed some warnings.
 *
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
 * Includes
 */

#ifndef TYPES_H
#include "types.h"
#endif /* TYPES_H */

/*
 * Globals
 */

extern struct seg_info *code_tree;

/*
 * Structures
 */

/* mmin field also holds the color BLACK/RED */

struct seg_info {
  struct seg_info *parent, *left, *right;
  U32 mmin, maddr, mend, mmax;
  struct code_info *codeinfo;
};

#define SEG_COLOR_RED 0x80000000

struct code_info {
  struct code_info *next;
  U32 maddr;
  U32 flags;
  U32 num_opcodes;
  U64 time_run;
  U32 (*code)(struct m_registers *, void *);	/* Returns the next PC */
  U32 *codeend;
  U32 checksum;
};

#define CIF_LOCKED	1
#define CIF_PATCHED	2

/*
 * Prototypes
 */

struct seg_info *find_seg(struct seg_info *root, U32 maddr);
struct seg_info *insert_seg(struct seg_info **root, U32 maddr, U32 mend);
void dump_seg_tree(struct seg_info *root);

struct code_info *find_ci(struct code_info **head, U32 maddr);

#endif /* CODEINFO_H */
