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
 *	@(#)$RCSfile: adulnreg.h,v $ $Revision: 1.2.2.2 $ (DEC) $Date: 1992/04/03 12:47:30 $
 */

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 * Definitions for the ADU NI port driver.
 *
 * Modification History: adulnreg.h
 *
 * 15-Jan-91 -- Mark Parenti
 *	Created this file for Alpha support.
 */

/*
 * Protect the file from multiple includes.
 */
#ifndef __ADU_NI_HDR
#define __ADU_NI_HDR

#include <sys/types.h>
#include <hal/adudefs.h>
#include <hal/ka_adu.h>

/*
 * Ringing the doorbell involves writing arbitrary data to the doorbell reg.
 */
#define RING_NI_DOORBELL 				\
	mb();						\
	*adu_ni_regs.db = 1;				\
	mb();

/*
 * Macros are needed to set and clear interrupt bits.  This is necessary
 * bacause the register is write-only so a read-modify-write is not possible.
 */
#define	ENABLE_LN_INTERRUPT(val)			\
        adu_ln_intr_reg |= (val); 			\
	mb();						\
	*adu_ni_regs.icr = adu_ln_intr_reg;		\
	mb();

#define	DISABLE_LN_INTERRUPT(val)			\
        adu_ln_intr_reg &= ~(val); 			\
	mb();						\
	*adu_ni_regs.icr = adu_ln_intr_reg;		\
	mb();


struct	trans_entry{
	volatile long	pad[8];
};

struct	rcv_entry{
	volatile long	pad[8];
};

/* ADU Packet formats	*/

typedef struct	ln_trn_pkt {
	volatile int	flag;
	volatile int	status;
	volatile int	ncol;
	volatile int	pad[5];
	volatile long	bufp;
	volatile int	ntbuf;
	volatile int	pad1[5];
}TRN_PKT;

typedef struct	ln_init_pkt {
	volatile int	flag;
	volatile int	status;
	volatile long	pad[3];
	volatile long	rxmode;
	volatile char	rxaddr[8];
	volatile short	lamask[4];
	volatile long	pad1;
}INIT_PKT;

typedef struct	ln_rcv_pkt {
	volatile int	flag;
	volatile int	status;
	volatile int	nrbuf;
	volatile int	pad[5];
	volatile long	bufp;
	volatile long	pad1[3];
}RCV_PKT;

/*
 * Network INIT Block
 */
struct ln_initb {
        u_short ln_mode;                /* NIB_MODE mode word */
        u_short ln_sta_addr[3];         /* NIB_PADR station address */
        u_short ln_multi_mask[4];       /* NIB_LADRF Multicast addr ma sk*/
        u_short ln_rcvlist_lo,          /* NIB_RDRP Rcv Dsc Ring Ptr */
                ln_rcvlist_hi:8,        /* Rcv list hi addr */
                ln_rcvresv:5,           /* reserved */
                ln_rcvlen:3;            /* RLEN Rcv ring length **/
        u_short ln_xmtlist_lo,          /* NIB_TDRP Xmt Dsc Ring Ptr */
                ln_xmtlist_hi:8,        /* Xmt list hi addr */
                ln_xmtresv:5,           /* reserved */
                ln_xmtlen:3;            /* TLEN Xmt ring length */
};

/* ADU Ethernet Rings */

#define	ADU_LN_RING_ENTRIE 64

struct	ln_ring {
	volatile struct trans_entry	trans_ring[ADU_LN_RING_ENTRIE];
	volatile struct rcv_entry	rcv_ring[ADU_LN_RING_ENTRIE];
};

/*
 * LANCE command and status bits (CSR0)
 */
#define LN_INIT			0x0001	/* (Re)-Initialize		*/
#define LN_START		0x0002	/* Start operation		*/
#define LN_STOP			0x0004	/* Reset firmware (stop)	*/
#define LN_TDMD			0x0008	/* Transmit on demand		*/
#define LN_TXON			0x0010	/* Transmitter is enabled	*/
#define LN_RXON			0x0020	/* Receiver is enabled		*/
#define LN_INEA			0x0040	/* Interrupt enable		*/
#define LN_INTR			0x0080	/* Interrupt request		*/
#define LN_IDON 		0x0100	/* Initialization done		*/
#define LN_TINT			0x0200	/* Transmitter interrupt	*/
#define LN_RINT			0x0400	/* Receive interrupt		*/
#define LN_MERR			0x0800	/* Memory error			*/
#define LN_MISS			0x1000	/* Missed packet		*/
#define LN_CERR			0x2000	/* Collision error		*/
#define LN_BABL			0x4000	/* Transmit timeout err		*/
#define LN_ERR			0x8000	/* Error summary		*/

/*
 * LANCE CSR3 status word
 * (these should always be 0 for the LANCE hardware)
 */
#define LN_BCON			0x0001	/* Byte control 		*/
#define LN_ACON			0x0002	/* ALE control			*/
#define LN_BSWP			0x0004	/* Byte swap (for DMA)		*/

/*
 * LANCE CSR select
 */
#define LN_CSR0			0x0000	/* CSR 0			*/
#define LN_CSR1			0x0001	/* CSR 1			*/
#define LN_CSR2			0x0002	/* CSR 2			*/
#define LN_CSR3			0x0003	/* CSR 3			*/

/*
 * General constant definitions
 */
#define LNNOALLOC		0	/* No buffer allocation		*/
#define LNALLOC 		1	/* Allocate local RAM buffer	*/

#define	LN_WORD_ALIGN		0x01	/* Check for word alignment	*/
#define	LN_QUAD_ALIGN		0x07	/* Check for quad alignment	*/
#define	LN_BUF_SIZE		0x0600	/* 1536 Byte buffers for rings	*/
#define	LN_LRB_SIZE		0x20000	/* 128K byte addresses */

#define LN_CRC_INIT		1	/* initialize CRC table		*/

#define	LN_NONDMA_RCV		0	/* Non-DMA architectures	*/
#define LN_DMA_RCV		1	/* LANCE receieve DMA		*/


#endif /* __ADU_NI_HDR */
