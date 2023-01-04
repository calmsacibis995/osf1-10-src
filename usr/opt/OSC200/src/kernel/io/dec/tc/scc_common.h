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
 * @(#)$RCSfile: scc_common.h,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/14 18:17:46 $
 */
/*
 *
 * scc_common.h
 *
 * SCC SLU async driver and sync driver
 *
 */

/* SCC driver interrupt switch tables */

/*
 *  The SCC interrutps, described in the SIR, are called via these entry points.
 */
#ifndef _SCC_COMMON_H_
#define _SCC_COMMON_H_

struct SIR_Interrupt_sw {
    int	(*RxOverrun)();		/* DMA Receive Overrun Interrupt     */
    int (*RxHalfPage)();	/* DMA Receive Half Page Interrupt   */
    int (*TxReadError)();	/* DMA Transmit Read Error Interrupt */
    int (*TxPageEnd)();		/* DMA Transmit Page End Interrupt   */
    int (*SCC)();		/* SCC Interrupt 		     */
};

/*
 *  The specific SCC ISRs are called via these entry points.
 */
struct SCC_Interrupt_sw {
    int (*RxSpecCond)();	/* Receive Special Condition Interrupt */
    int (*Transmit)();		/* Transmit Interrupt 		       */
    int (*ExtStatus)();		/* External Status Interrupt 	       */
};

/* Saved registers */
struct scc_saved_reg
{
    u_char      rr0;
    u_char      wr1;
    u_char      wr2;
    u_char      wr3;
    u_char      wr4;
    u_char      wr5;
    u_char      wr6;
    u_char      wr7;
    u_char      wr8;
    u_char      wr9;
    u_char      wr10;
    u_char      wr11;
    u_char      wr12;
    u_char      wr13;
    u_char      wr14;
    u_char      wr15;
};

#define NSCC_DEV        2
#define SCC_CHA		0
#define SCC_CHB		1

#endif
