/*
 * $Id: glue.h,v 1.1.1.1 1996/06/30 23:51:55 grubba Exp $
 *
 * Headerfile for the M68000 to Sparc recompiler.
 *
 * $Log: glue.h,v $
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

extern ULONG s_pre_amble[];
extern ULONG s_raise_exception[];
extern ULONG s_clobber[];
extern ULONG s_clobber_short[];
extern ULONG s_clobber_byte[];
extern ULONG s_supervisor[];

#endif /* TEMPLATES_GLUE_H */
