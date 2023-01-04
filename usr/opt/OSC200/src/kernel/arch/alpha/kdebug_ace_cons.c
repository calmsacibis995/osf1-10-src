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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: kdebug_ace_cons.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/09/27 14:18:11 $";
#endif

/*
 * Do the Jensen address swizzle.
 */
#define JENSEN_JSHIFT   9
#define JENSEN_VTI_BASE 0x1C0000000L

#define APC_READ(off, val)   val = inVti(regs+(off))
#define APC_WRITE(off, val)  outVti(regs+(off),(val))

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

/* line status register defs */
#define ACE_LSR_DR       0x01
#define ACE_LSR_OE       0x02
#define ACE_LSR_PE       0x04
#define ACE_LSR_FE       0x08
#define ACE_LSR_BI       0x10
#define ACE_LSR_THRE     0x20
#define ACE_LSR_TEMT     0x40

static unsigned long regs;

#include <machine/pmap.h>
#include <machine/machparam.h>

/* 
 *  inVti/outVti
 *
 *  These are the read/write routines for talking
 *  to the VL82C106 Combo Chip. 
 */
static char
inVti(
    unsigned long port)
{
    unsigned int *portp;

    portp = (unsigned int *)(JENSEN_VTI_BASE | (port << JENSEN_JSHIFT));

    return((unsigned char) *(unsigned int *)PHYS_TO_KSEG(portp));
}

static void
outVti(
    unsigned long port,
    unsigned char data)
{
    unsigned int *portp;

    portp = (unsigned int *)(JENSEN_VTI_BASE | (port << JENSEN_JSHIFT));

    *(unsigned int *)PHYS_TO_KSEG(portp) = (unsigned int) data;
    kdebug_mb();
}

long
kdebug_ace_init(
    unsigned long addr)
{
    regs = addr;
    return(0);
}

unsigned char
kdebug_ace_rx_read()
{
    unsigned char result;

    APC_READ(ACE_RBR, result);
    return(result);
}

long
kdebug_ace_tx_write(
    unsigned char c)
{
    APC_WRITE(ACE_THR, c);
}

long
kdebug_ace_tx_rdy()
{
    unsigned char state;

    APC_READ(ACE_LSR, state);
    return(state & ACE_LSR_THRE);
}

long
kdebug_ace_rx_rdy()
{
    unsigned char state;

    APC_READ(ACE_LSR, state);
    if (state & (ACE_LSR_PE | ACE_LSR_OE | ACE_LSR_FE)) {
	return(0);
    }
    return(state & ACE_LSR_DR);
}
