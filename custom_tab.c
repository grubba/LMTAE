/*
 * $Id: custom_tab.c,v 1.3 1998/02/10 01:02:00 marcus Exp $
 *
 * Tables for the customchip address decoding.
 *
 * $Log: custom_tab.c,v $
 * Revision 1.2  1996/07/21 16:16:13  grubba
 * custom_write_intena() and custom_write_intreq() moved to interrupt.[ch].
 * Serialport emulation on stdin/stdout added.
 * Custom registers that are write-only are now declared as such.
 *
 * Revision 1.1  1996/07/19 16:46:12  grubba
 * Cleaned up interrupt handling.
 * Cleaned up custom chip emulation.
 * INTENA/INTREQ should work.
 *
 *
 */

/*
 * Tables
 */

static S16 (*custom_read_tab[])(U32 reg) = {
  NULL,			/* BLTDDAT */
  NULL,			/* DMACONR */
  NULL,			/* VPOSR */
  custom_read_vhposr,	/* VHPOSR */
  NULL,			/* DSKDATR */
  NULL,			/* JOY0DAT */
  NULL,			/* JOY1DAT */
  NULL,			/* CLXDAT */
  NULL,			/* ADKCONR */
  NULL,			/* POT0DAT */
  NULL,			/* POT1DAT */
  NULL,			/* POTGOR */
  custom_read_serdatr,	/* SERDATR */
  NULL,			/* DSKBYTR */
  NULL,			/* INTENAR */
  NULL,			/* INTREQR */

  custom_write_only,	/* DSKPTH */
  custom_write_only,	/* DSKPTL */
  custom_write_only,	/* DSKLEN */
  custom_write_only,	/* DSKDAT */
  custom_write_only,	/* REFPTR */
  custom_write_only,	/* VPOSW */
  custom_write_only,	/* VHPOSW */
  custom_write_only,	/* COPCON */
  custom_write_only,	/* SERDAT */
  custom_write_only,	/* SERPER */
  custom_write_only,	/* POTGO */
  custom_write_only,	/* JOYTEST */
  custom_write_only,	/* STREQU */
  custom_write_only,	/* STRVBL */
  custom_write_only,	/* STRHOR */
  custom_write_only,	/* STRLONG */

  custom_write_only,	/* BLTCON0 */
  custom_write_only,	/* BLTCON1 */
  custom_write_only,	/* BLTAFWM */
  custom_write_only,	/* BLTALWM */
  custom_write_only,	/* BLTCPTH */
  custom_write_only,	/* BLTCPTL */
  custom_write_only,	/* BLTBPTH */
  custom_write_only,	/* BLTBPTL */
  custom_write_only,	/* BLTAPTH */
  custom_write_only,	/* BLTAPTL */
  custom_write_only,	/* BLTDPTH */
  custom_write_only,	/* BLTDPTL */
  custom_write_only,	/* BLTSIZE */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */
  
  custom_write_only,	/* BLTCMOD */
  custom_write_only,	/* BLTBMOD */
  custom_write_only,	/* BLTAMOD */
  custom_write_only,	/* BLTDMOD */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* BLTCDAT */
  custom_write_only,	/* BLTBDAT */
  custom_write_only,	/* BLTADAT */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */
  custom_read_deniseid,	/* DENISEID */
  custom_write_only,	/* DSKSYNC */

  custom_write_only,	/* COP1LCH */
  custom_write_only,	/* COP1LCL */
  custom_write_only,	/* COP2LCH */
  custom_write_only,	/* COP2LCL */
  custom_write_only,	/* COPJMP1 */
  custom_write_only,	/* COPJMP2 */
  custom_write_only,	/* COPINS */
  custom_write_only,	/* DIWSTRT */
  custom_write_only,	/* DIWSTOP */
  custom_write_only,	/* DDFSTRT */
  custom_write_only,	/* DDFSTOP */
  custom_write_only,	/* DMACON */
  custom_write_only,	/* CLXCON */
  custom_write_only,	/* INTENA */
  custom_write_only,	/* INTREQ */
  custom_write_only,	/* ADKCON */
  
  custom_write_only,	/* AUD0LCH */
  custom_write_only,	/* AUD0LCL */
  custom_write_only,	/* AUD0LEN */
  custom_write_only,	/* AUD0PER */
  custom_write_only,	/* AUD0VOL */
  custom_write_only,	/* AUD0DAT */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* AUD1LCH */
  custom_write_only,	/* AUD1LCL */
  custom_write_only,	/* AUD1LEN */
  custom_write_only,	/* AUD1PER */
  custom_write_only,	/* AUD1VOL */
  custom_write_only,	/* AUD1DAT */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */

  custom_write_only,	/* AUD2LCH */
  custom_write_only,	/* AUD2LCL */
  custom_write_only,	/* AUD2LEN */
  custom_write_only,	/* AUD2PER */
  custom_write_only,	/* AUD2VOL */
  custom_write_only,	/* AUD2DAT */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* AUD3LCH */
  custom_write_only,	/* AUD3LCL */
  custom_write_only,	/* AUD3LEN */
  custom_write_only,	/* AUD3PER */
  custom_write_only,	/* AUD3VOL */
  custom_write_only,	/* AUD3DAT */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */

  custom_write_only,	/* BPL1PTH */
  custom_write_only,	/* BPL1PTL */
  custom_write_only,	/* BPL2PTH */
  custom_write_only,	/* BPL2PTL */
  custom_write_only,	/* BPL3PTH */
  custom_write_only,	/* BPL3PTL */
  custom_write_only,	/* BPL4PTH */
  custom_write_only,	/* BPL4PTL */
  custom_write_only,	/* BPL5PTH */
  custom_write_only,	/* BPL5PTL */
  custom_write_only,	/* BPL6PTH */
  custom_write_only,	/* BPL6PTL */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */

  custom_write_only,	/* BPLCON0 */
  custom_write_only,	/* BPLCON1 */
  custom_write_only,	/* BPLCON2 */
  custom_write_only,	/* zilch */
  custom_write_only,	/* BPL1MOD */
  custom_write_only,	/* BPL2MOD */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* BPL1DAT */
  custom_write_only,	/* BPL2DAT */
  custom_write_only,	/* BPL3DAT */
  custom_write_only,	/* BPL4DAT */
  custom_write_only,	/* BPL5DAT */
  custom_write_only,	/* BPL6DAT */
  custom_write_only,	/* UNUSED */
  custom_write_only,	/* UNUSED */

  custom_write_only,	/* SPR0PTH */
  custom_write_only,	/* SPR0PTL */
  custom_write_only,	/* SPR1PTH */
  custom_write_only,	/* SPR1PTL */
  custom_write_only,	/* SPR2PTH */
  custom_write_only,	/* SPR2PTL */
  custom_write_only,	/* SPR3PTH */
  custom_write_only,	/* SPR3PTL */
  custom_write_only,	/* SPR4PTH */
  custom_write_only,	/* SPR4PTL */
  custom_write_only,	/* SPR5PTH */
  custom_write_only,	/* SPR5PTL */
  custom_write_only,	/* SPR6PTH */
  custom_write_only,	/* SPR6PTL */
  custom_write_only,	/* SPR7PTH */
  custom_write_only,	/* SPR7PTL */

  custom_write_only,	/* SPR0POS */
  custom_write_only,	/* SPR0CTL */
  custom_write_only,	/* SPR0DATA */
  custom_write_only,	/* SPR0DATB */
  custom_write_only,	/* SPR1POS */
  custom_write_only,	/* SPR1CTL */
  custom_write_only,	/* SPR1DATA */
  custom_write_only,	/* SPR1DATB */
  custom_write_only,	/* SPR2POS */
  custom_write_only,	/* SPR2CTL */
  custom_write_only,	/* SPR2DATA */
  custom_write_only,	/* SPR2DATB */
  custom_write_only,	/* SPR3POS */
  custom_write_only,	/* SPR3CTL */
  custom_write_only,	/* SPR3DATA */
  custom_write_only,	/* SPR3DATB */

  custom_write_only,	/* SPR4POS */
  custom_write_only,	/* SPR4CTL */
  custom_write_only,	/* SPR4DATA */
  custom_write_only,	/* SPR4DATB */
  custom_write_only,	/* SPR5POS */
  custom_write_only,	/* SPR5CTL */
  custom_write_only,	/* SPR5DATA */
  custom_write_only,	/* SPR5DATB */
  custom_write_only,	/* SPR6POS */
  custom_write_only,	/* SPR6CTL */
  custom_write_only,	/* SPR6DATA */
  custom_write_only,	/* SPR6DATB */
  custom_write_only,	/* SPR7POS */
  custom_write_only,	/* SPR7CTL */
  custom_write_only,	/* SPR7DATA */
  custom_write_only,	/* SPR7DATB */

  custom_write_only,	/* COLOR0 */
  custom_write_only,	/* COLOR1 */
  custom_write_only,	/* COLOR2 */
  custom_write_only,	/* COLOR3 */
  custom_write_only,	/* COLOR4 */
  custom_write_only,	/* COLOR5 */
  custom_write_only,	/* COLOR6 */
  custom_write_only,	/* COLOR7 */
  custom_write_only,	/* COLOR8 */
  custom_write_only,	/* COLOR9 */
  custom_write_only,	/* COLOR10 */
  custom_write_only,	/* COLOR11 */
  custom_write_only,	/* COLOR12 */
  custom_write_only,	/* COLOR13 */
  custom_write_only,	/* COLOR14 */
  custom_write_only,	/* COLOR15 */

  custom_write_only,	/* COLOR16 */
  custom_write_only,	/* COLOR17 */
  custom_write_only,	/* COLOR18 */
  custom_write_only,	/* COLOR19 */
  custom_write_only,	/* COLOR20 */
  custom_write_only,	/* COLOR21 */
  custom_write_only,	/* COLOR22 */
  custom_write_only,	/* COLOR23 */
  custom_write_only,	/* COLOR24 */
  custom_write_only,	/* COLOR25 */
  custom_write_only,	/* COLOR26 */
  custom_write_only,	/* COLOR27 */
  custom_write_only,	/* COLOR28 */
  custom_write_only,	/* COLOR29 */
  custom_write_only,	/* COLOR30 */
  custom_write_only,	/* COLOR31 */
};

static void (*custom_write_tab[])(U32 reg, U16 val) = {
  NULL,			/* BLTDDAT */
  custom_read_only,	/* DMACONR */
  custom_read_only,	/* VPOSR */
  custom_read_only,	/* VHPOSR */
  custom_read_only,	/* DSKDATR */
  NULL,			/* JOY0DAT */
  NULL,			/* JOY1DAT */
  NULL,			/* CLXDAT */
  custom_read_only,	/* ADKCONR */
  NULL,			/* POT0DAT */
  NULL,			/* POT1DAT */
  NULL,			/* POTGOR */
  custom_read_only,	/* SERDATR */
  custom_read_only,	/* DSKBYTR */
  custom_read_only,	/* INTENAR */
  custom_read_only,	/* INTREQR */

  NULL,			/* DSKPTH */
  NULL,			/* DSKPTL */
  NULL,			/* DSKLEN */
  NULL,			/* DSKDAT */
  NULL,			/* REFPTR */
  NULL,			/* VPOSW */
  NULL,			/* VHPOSW */
  NULL,			/* COPCON */
  custom_write_serdat,	/* SERDAT */
  NULL,			/* SERPER */
  NULL,			/* POTGO */
  NULL,			/* JOYTEST */
  NULL,			/* STREQU */
  NULL,			/* STRVBL */
  NULL,			/* STRHOR */
  NULL,			/* STRLONG */

  NULL,			/* BLTCON0 */
  NULL,			/* BLTCON1 */
  NULL,			/* BLTAFWM */
  NULL,			/* BLTALWM */
  NULL,			/* BLTCPTH */
  NULL,			/* BLTCPTL */
  NULL,			/* BLTBPTH */
  NULL,			/* BLTBPTL */
  NULL,			/* BLTAPTH */
  NULL,			/* BLTAPTL */
  NULL,			/* BLTDPTH */
  NULL,			/* BLTDPTL */
  NULL,			/* BLTSIZE */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */
  
  NULL,			/* BLTCMOD */
  NULL,			/* BLTBMOD */
  NULL,			/* BLTAMOD */
  NULL,			/* BLTDMOD */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */
  NULL,			/* BLTCDAT */
  NULL,			/* BLTBDAT */
  NULL,			/* BLTADAT */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* DENSISEID */
  NULL,			/* DSKSYNC */

  NULL,			/* COP1LCH */
  NULL,			/* COP1LCL */
  NULL,			/* COP2LCH */
  NULL,			/* COP2LCL */
  NULL,			/* COPJMP1 */
  NULL,			/* COPJMP2 */
  NULL,			/* COPINS */
  NULL,			/* DIWSTRT */
  NULL,			/* DIWSTOP */
  NULL,			/* DDFSTRT */
  NULL,			/* DDFSTOP */
  NULL,			/* DMACON */
  NULL,			/* CLXCON */
  custom_write_intena,	/* INTENA */
  custom_write_intreq,	/* INTREQ */
  NULL,			/* ADKCON */
  
  NULL,			/* AUD0LCH */
  NULL,			/* AUD0LCL */
  NULL,			/* AUD0LEN */
  NULL,			/* AUD0PER */
  NULL,			/* AUD0VOL */
  NULL,			/* AUD0DAT */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */
  NULL,			/* AUD1LCH */
  NULL,			/* AUD1LCL */
  NULL,			/* AUD1LEN */
  NULL,			/* AUD1PER */
  NULL,			/* AUD1VOL */
  NULL,			/* AUD1DAT */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */

  NULL,			/* AUD2LCH */
  NULL,			/* AUD2LCL */
  NULL,			/* AUD2LEN */
  NULL,			/* AUD2PER */
  NULL,			/* AUD2VOL */
  NULL,			/* AUD2DAT */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */
  NULL,			/* AUD3LCH */
  NULL,			/* AUD3LCL */
  NULL,			/* AUD3LEN */
  NULL,			/* AUD3PER */
  NULL,			/* AUD3VOL */
  NULL,			/* AUD3DAT */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */

  NULL,			/* BPL1PTH */
  NULL,			/* BPL1PTL */
  NULL,			/* BPL2PTH */
  NULL,			/* BPL2PTL */
  NULL,			/* BPL3PTH */
  NULL,			/* BPL3PTL */
  NULL,			/* BPL4PTH */
  NULL,			/* BPL4PTL */
  NULL,			/* BPL5PTH */
  NULL,			/* BPL5PTL */
  NULL,			/* BPL6PTH */
  NULL,			/* BPL6PTL */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */

  NULL,			/* BPLCON0 */
  NULL,			/* BPLCON1 */
  NULL,			/* BPLCON2 */
  custom_read_only,	/* zilch */
  NULL,			/* BPL1MOD */
  NULL,			/* BPL2MOD */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */
  NULL,			/* BPL1DAT */
  NULL,			/* BPL2DAT */
  NULL,			/* BPL3DAT */
  NULL,			/* BPL4DAT */
  NULL,			/* BPL5DAT */
  NULL,			/* BPL6DAT */
  custom_read_only,	/* UNUSED */
  custom_read_only,	/* UNUSED */

  NULL,			/* SPR0PTH */
  NULL,			/* SPR0PTL */
  NULL,			/* SPR1PTH */
  NULL,			/* SPR1PTL */
  NULL,			/* SPR2PTH */
  NULL,			/* SPR2PTL */
  NULL,			/* SPR3PTH */
  NULL,			/* SPR3PTL */
  NULL,			/* SPR4PTH */
  NULL,			/* SPR4PTL */
  NULL,			/* SPR5PTH */
  NULL,			/* SPR5PTL */
  NULL,			/* SPR6PTH */
  NULL,			/* SPR6PTL */
  NULL,			/* SPR7PTH */
  NULL,			/* SPR7PTL */

  NULL,			/* SPR0POS */
  NULL,			/* SPR0CTL */
  NULL,			/* SPR0DATA */
  NULL,			/* SPR0DATB */
  NULL,			/* SPR1POS */
  NULL,			/* SPR1CTL */
  NULL,			/* SPR1DATA */
  NULL,			/* SPR1DATB */
  NULL,			/* SPR2POS */
  NULL,			/* SPR2CTL */
  NULL,			/* SPR2DATA */
  NULL,			/* SPR2DATB */
  NULL,			/* SPR3POS */
  NULL,			/* SPR3CTL */
  NULL,			/* SPR3DATA */
  NULL,			/* SPR3DATB */

  NULL,			/* SPR4POS */
  NULL,			/* SPR4CTL */
  NULL,			/* SPR4DATA */
  NULL,			/* SPR4DATB */
  NULL,			/* SPR5POS */
  NULL,			/* SPR5CTL */
  NULL,			/* SPR5DATA */
  NULL,			/* SPR5DATB */
  NULL,			/* SPR6POS */
  NULL,			/* SPR6CTL */
  NULL,			/* SPR6DATA */
  NULL,			/* SPR6DATB */
  NULL,			/* SPR7POS */
  NULL,			/* SPR7CTL */
  NULL,			/* SPR7DATA */
  NULL,			/* SPR7DATB */

  NULL,			/* COLOR0 */
  NULL,			/* COLOR1 */
  NULL,			/* COLOR2 */
  NULL,			/* COLOR3 */
  NULL,			/* COLOR4 */
  NULL,			/* COLOR5 */
  NULL,			/* COLOR6 */
  NULL,			/* COLOR7 */
  NULL,			/* COLOR8 */
  NULL,			/* COLOR9 */
  NULL,			/* COLOR10 */
  NULL,			/* COLOR11 */
  NULL,			/* COLOR12 */
  NULL,			/* COLOR13 */
  NULL,			/* COLOR14 */
  NULL,			/* COLOR15 */

  NULL,			/* COLOR16 */
  NULL,			/* COLOR17 */
  NULL,			/* COLOR18 */
  NULL,			/* COLOR19 */
  NULL,			/* COLOR20 */
  NULL,			/* COLOR21 */
  NULL,			/* COLOR22 */
  NULL,			/* COLOR23 */
  NULL,			/* COLOR24 */
  NULL,			/* COLOR25 */
  NULL,			/* COLOR26 */
  NULL,			/* COLOR27 */
  NULL,			/* COLOR28 */
  NULL,			/* COLOR29 */
  NULL,			/* COLOR30 */
  NULL,			/* COLOR31 */
};
