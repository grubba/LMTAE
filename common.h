
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed long s32;
typedef unsigned long u32;

extern volatile u8 *chipmem;
extern volatile u16 *custom;

extern int devzero;

#define REGACCESS(x) (custom[(x)>>1])
#define DREGACCESS(x) (*(u32*)&(REGACCESS(x)))

#define DMACONR REGACCESS(0x002)
#define BLTCON0 REGACCESS(0x040)
#define BLTCON1 REGACCESS(0x042)
#define BLTAFWM REGACCESS(0x044)
#define BLTALWM REGACCESS(0x046)
#define BLTCPT  DREGACCESS(0x048)
#define BLTBPT  DREGACCESS(0x04c)
#define BLTAPT  DREGACCESS(0x050)
#define BLTDPT  DREGACCESS(0x054)
#define BLTSIZE REGACCESS(0x058)
#define BLTCMOD REGACCESS(0x060)
#define BLTBMOD REGACCESS(0x062)
#define BLTAMOD REGACCESS(0x064)
#define BLTDMOD REGACCESS(0x066)
#define BLTCDAT REGACCESS(0x070)
#define BLTBDAT REGACCESS(0x072)
#define BLTADAT REGACCESS(0x074)
