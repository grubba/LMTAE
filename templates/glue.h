/*
 * $Id: glue.h,v 1.2 1996/07/17 16:02:50 grubba Exp $
 *
 * Headerfile for the M68000 to Sparc recompiler.
 *
 * $Log: glue.h,v $
 * Revision 1.1.1.1  1996/06/30 23:51:55  grubba
 * Entry into CVS
 *
 * Revision 1.1  1996/06/01 09:31:29  grubba
 * Initial revision
 *
 *
 */

#ifndef TEMPLATES_GLUE_H
#define TEMPLATES_GLUE_H

/*
 * Assembler glue template code
 */

extern U32 s_pre_amble[];
extern U32 s_raise_exception[];
extern U32 s_clobber[];
extern U32 s_clobber_short[];
extern U32 s_clobber_byte[];
extern U32 s_supervisor[];

#endif /* TEMPLATES_GLUE_H */
