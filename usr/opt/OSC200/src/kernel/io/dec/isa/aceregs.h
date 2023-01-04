/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*
 * @(#)$RCSfile: aceregs.h,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/07/14 18:16:58 $
 */

/*
 * Mods:
 *
 * 09-01-92 - Tim - comments & cleanup.
 */

#ifndef _ACEREGS_H_ 
#define _ACEREGS_H_ 

/* following specific to apc console device, on board combo chip
 *       serial lines for the ALPHA PC
 */
#define NACELINE 2
#define ACELINEMASK 1		/* 1 bit represents 2 lines */
#define CONSOLEMAJOR 0
#define ACE_CONSOLE_UNIT 0	/* assumes the console is on line 0 */
#define ACE_KDEBUG_UNIT  1	/* assumes that if kdebug is running it 
				 * will be on this line (not the console) */

#define ACE_COMMA_BASE 0x3f8
#define ACE_COMMB_BASE 0x2f8

struct ace_saved_reg{
  int lcr;
  int dllsb;
  int dlmsb;
  int ie;
};

/* Driver and data specific structure */
struct	ace_softc {
  u_long ace_regs[NACELINE];
  struct ace_saved_reg ace_saved_regs[NACELINE];
  long ace_flags[NACELINE];       /* Flags (one per line)		*/
  long ace_category_flags[NACELINE]; /* Category flags (one per line)*/
  u_long ace_softcnt[NACELINE];	/* Soft error count total	*/
  u_long ace_hardcnt[NACELINE];	/* Hard error count total	*/
  char ace_device[DEV_SIZE][NACELINE]; /* Device type string	*/
};

/* register offset definitions */
#define ACE_RBR    0x00
#define ACE_THR    0x00
#define ACE_IE     0x01
#define ACE_IIR    0x02
#define ACE_LCR    0x03
#define ACE_MCR    0x04
#define ACE_LSR    0x05
#define ACE_MSR    0x06
#define ACE_SCR    0x07
#define ACE_DLLSB  0x00
#define ACE_DLMSB  0x01


/*
 * modem control register.  See page 6-10 of vti spec. 
 * Note: according to the spec they are reverse logic (ie if the bit
 * is "1" the lead is NOT asserted.  However in practice I found that
 * to be the oposite.  I'm not sure if the spec is wrong or if the leads
 * are getting inverted somewhere.
 */
#define ACE_MCR_DTR	0x01
#define ACE_MCR_RTS	0x02
#define ACE_MCR_OUT2	0x08	/* Killer bit - always leave set! */

/* modem status register defs 
 * See page 6-9 of the vti spec for this register.
 * Note: according to the spec they are reverse logic (ie if the bit
 * is "1" the lead is NOT asserted.  However in practice I found that
 * to be the oposite.  I'm not sure if the spec is wrong or if the leads
 * are getting inverted somewhere.
 */
#define ACE_MSR_CTS     0x10
#define ACE_MSR_DSR     0x20
#define ACE_MSR_CD      0x80
#define ACE_XMIT_BITS	    (ACE_MSR_CTS|ACE_MSR_DSR|ACE_MSR_CD)

/* interrupt enable defines */
#define ACE_IE_RDINT     0x01
#define ACE_IE_THREINT   0x02
#define ACE_IE_RLSINT    0x04
#define ACE_IE_MSINT     0x08

/* line status register defs */
#define ACE_LSR_DR       0x01
#define ACE_LSR_OE       0x02
#define ACE_LSR_PE       0x04
#define ACE_LSR_FE       0x08
#define ACE_LSR_BI       0x10
#define ACE_LSR_THRE     0x20
#define ACE_LSR_TEMT     0x40


/* line control register defs */
#define ACE_LCR_5DATA     0x00
#define ACE_LCR_6DATA     0x01
#define ACE_LCR_7DATA     0x02
#define ACE_LCR_8DATA     0x03
#define ACE_LCR_ONESTPBIT 0x0
#define ACE_LCR_TWOSTPBIT 0x04
#define ACE_LCR_PENABLE   0x08
#define ACE_LCR_EPAR      0x10
#define ACE_LCR_BRKEN     0x40
#define ACE_LCR_DLAB      0x80


/* interrupt id register defs */
#define ACE_INT_BIT     0x01
#define ACE_RLS_INT     0x06
#define ACE_RDA_INT     0x04
#define ACE_THRE_INT    0x02
#define ACE_MS_INT      0x00

/*
 * The baud rate is specified as a 16-bit divisor of an input clock
 * frequency.  Since the uart regs are 8-bits in length it takes 2
 * registers to represent this 16-bit divisor.  Thats why there is a
 * hi and lo register used to represent the 16-bit divisor.
 *
 * The numbers for the divisor are based on a 1.8432 MHz clock.
 * Refer to the VTI spec for the VL16C452 for these numbers.  This can
 * be found in the "VLSI Computer Products Data Manual" page 6-61 table 5.
 */

#define ACE_B50_LO      0x00		/* 2304 */
#define ACE_B50_HI      0x09

#define ACE_B75_LO      0x00
#define ACE_B75_HI      0x06

#define ACE_B110_LO     0x17
#define ACE_B110_HI     0x04

#define ACE_B134_5_LO   0x59
#define ACE_B134_5_HI   0x3

#define ACE_B150_LO      0x00
#define ACE_B150_HI      0x03

#define ACE_B300_LO      0x80
#define ACE_B300_HI      0x01

#define ACE_B600_LO      0xc0
#define ACE_B600_HI      0

#define ACE_B1200_LO     0x60
#define ACE_B1200_HI     0

#define ACE_B1800_LO     0x40
#define ACE_B1800_HI     0

#define ACE_B2000_LO     0x3a
#define ACE_B2000_HI     0

#define ACE_B2400_LO     0x30
#define ACE_B2400_HI     0

#define ACE_B3600_LO     0x20
#define ACE_B3600_HI     0

#define ACE_B4800_LO     0x18
#define ACE_B4800_HI     0

#define ACE_B7200_LO     0x10
#define ACE_B7200_HI     0

#define ACE_B9600_LO     0xc
#define ACE_B9600_HI     0

#define ACE_B19200_LO    6
#define ACE_B19200_HI    0

#define ACE_B38400_LO    3
#define ACE_B38400_HI    0

#define ACE_B56000_LO    2
#define ACE_B56000_HI    0

#endif
