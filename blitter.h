#include "common.h"

typedef u32 sparc_instr;

extern sparc_instr *minterms[];

#define NULL ((void*)0l)

extern void doblit_sync(void);
extern void doblit_async(void);
extern void init_blitter(void);
extern void await_blitter(void);
