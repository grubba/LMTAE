/*
 * $Id: hardware.h,v 1.1 1996/07/17 16:01:34 grubba Exp $
 *
 * Hardware emulation
 *
 * $Log$
 *
 */

#ifndef HARDWARE_H
#define HARDWARE_H

/*
 * Includes
 */

#ifndef TYPES_H
#include "types.h"
#endif /* TYPES_H */

/*
 * Prototypes
 */

void add_hw(U32 start, U32 size, const char *name,
	    U32 (*read_long)(U32, U32),
	    U32 (*read_word)(U32, U32),
	    U32 (*read_byte)(U32, U32),
	    void (*write_long)(U32, U32, U32),
	    void (*write_word)(U32, U32, U32),
	    void (*write_byte)(U32, U32, U32),
	    void (*reset)(U32));
void init_hardware(void);
U32 read_bad(U32, U32);
void write_bad(U32, U32, U32);

#endif /* HARDWARE_H */
