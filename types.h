/*
 * $Id: types.h,v 1.2 1996/08/11 17:36:22 grubba Exp $
 *
 * Common types used by the LMTAE project
 *
 * $Log: types.h,v $
 * Revision 1.1  1996/07/17 16:02:07  grubba
 * Changed from {U,}{LONG,WORD,BYTE} to [SU]{8,16,32}.
 * Hopefully all places got patched.
 *
 *
 */

#ifndef TYPES_H
#define TYPES_H

typedef signed char		S8;
typedef unsigned char		U8;
typedef signed short		S16;
typedef unsigned short		U16;
typedef signed			S32;
typedef unsigned		U32;
/* It should be possible to use __extension__ here */
typedef signed long long	S64;
typedef unsigned long long	U64;

#define s8	S8
#define u8	U8
#define s16	S16
#define u16	U16
#define s32	S32
#define u32	U32
#define s64	S64
#define u64	U64

#endif
