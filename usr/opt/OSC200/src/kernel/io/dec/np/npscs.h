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
 *	@(#)npscs.h	5.2	(ULTRIX)	10/16/91
 */

/*
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect N_PORT Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect N_PORT Port Driver
 *		constants and data structure definitions visible to SCS.
 *
 *   Creator:	Peter Keilty	Creation Date:	July 1, 1991
 *	        This file derived from Todd Katz CI port driver.
 *
 *   Modification History:
 *
 *   31-Oct-1991	Peter Keilty
 *	Ported to OFS/1
 *
 *   23-Oct-1991	Brian Nadeau
 *	Updates/bug fixes
 *
 */

#ifndef _NPSCS_H_
#define _NPSCS_H_

/* CI Constants.
 */
#define	LBDSIZE			48	/* Size of loopback data	     */

/* N_PORT Data Structure Definitions.
 */
typedef struct _regptrs {		/* Port Control Register Pointers     */
					/* For Single Channel Adapters 	      */
    volatile unsigned long *amcsr;	/* Adap maintenance cntl & status reg */
    volatile unsigned long *csr;	/* Channel status register	      */
    volatile unsigned long *cesr;	/* Channel error status register      */
    volatile unsigned long *cfar;	/* Channel failing address register   */
    volatile unsigned long *abbr;	/* Adapter block base register	      */
    volatile unsigned long *ccq0ir;	/* Channel command que 0 control reg  */
    volatile unsigned long *ccq1ir;	/* Channel command que 1 control reg  */
    volatile unsigned long *ccq2ir;	/* Channel command que 2 control reg  */
    volatile unsigned long *adfqir;	/* Channel dg free queue control reg  */
    volatile unsigned long *amfqir;	/* Channel msg free queue control reg */
    volatile unsigned long *csrcr;	/* Channel status release control reg */
    volatile unsigned long *cecr;	/* Channel enable control register    */
    volatile unsigned long *cicr;	/* Channel initialization control reg */
    volatile unsigned long *amtcr;	/* Adap maintenance timer control reg */
    volatile unsigned long *amtecr;	/* Adapter maint/sanity expiration reg*/
    volatile unsigned long *aitcr;	/* Adapter intr holdoff timer ctrl reg*/
} REGPTRS;

typedef	struct _nppccb	{		/* CI Specific Fields of PCCB	     */
    struct _npadap	*npadap;	/* Adapter Interface Block pointer   */
    struct _isr		*ciisr;		/* Interupt Service  Block pointer   */
    struct _regptrs	regptrs;	/* control register pointers    */
    void		( *disable_port )();/* Disable a local CI port	     */
    u_long		( *start_port )();  /* Start a local CI port	     */
    u_long		( *load_ucode )();  /* Load fn microcode( optional ) */
    struct _npq		*dccq2;		/* Channel command queue 2	     */
    struct _npq		*dccq1;		/* Channel command queue 1    	     */
    struct _npq		*dccq0;		/* Channel command queue 0    	     */
    u_int		c_idx;		/* Per channel index		     */
    struct _npbq	binuseq;	/* Buffers inuse by this channel     */
    struct	{			/* Local port status flags	     */
	u_int	init		:  1;	/*  First time initialization 	     */
	u_int	power		:  1;	/*  Port has power		     */
	u_int	mapped		:  1;	/*  Adapter space is mapped	     */
	u_int	mtimer		:  1;	/*  Maintenance timer is operational */
	u_int	connectivity	:  1;	/*  Port connectivity established    */
	u_int	onboard		:  1;	/*  Port microcode is onboard	     */
	u_int	adapt		:  1;	/*  This PCCB "ONLY" touchs adapter  */
	u_int	isrfork		:  1;	/*  Isr fork process scheduled       */
	u_int			: 24;
    } lpstatus;
#define	NP_ISRFORK		0x00000080
    struct kschedblk	isrforkb;	/* Isr thread fork process block     */
    u_int		pgsize;		/* System page size 4k or 8k	     */
    u_int		typ0_max;	/* Type 0 ptr max		     */
    u_int		typ1_max;	/* Type 1 ptr max		     */
    struct _mrltab	*mrltab;	/* Microcode revision level table ptr*/
    u_int		lbcrc;		/* Loopback CRC			     */
    u_short		pkt_size;	/* Size of port command packet	     */
    u_short		reinit_tries;	/* Number consecutive re-inits left  */
    u_int		pkt_mult;	/* Port packet data multiple	     */
    struct	{			/* Loopback status flags	     */
	u_char	cable0_prev	:  1;	/*  Cable 0 prev status( Bad == 1 )  */
	u_char	cable0_curr	:  1;	/*  Cable 0 current status( Bad == 1)*/
	u_char	cable0_test	:  1;	/*  Cable 0 loopback tested	     */
	u_char	cable1_prev	:  1;	/*  Cable 1 prev status( Bad == 1 )  */
	u_char	cable1_curr	:  1;	/*  Cable 1 current status( Bad == 1)*/
	u_char	cable1_test	:  1;	/*  Cable 1 loopback tested	     */
	u_char			:  2;
    } lbstatus;
    u_char		interconnect;	/* Interconnect type	 	     */
    u_char		fn_level;	/* Functional ucode revision level   */
    u_char		rom_level;	/* PROM/Self-test ucode rev level    */
    u_char		lbdata[ LBDSIZE ];/* Loopback data		     */
    union ci_dattnopt	devattn;	/* Device attention information	     */
					/* Family/port specific information  */
    u_char		dg_cache;	/* Size of datagram cache	     */
    u_char		msg_cache;	/* Size of message cache	     */
    u_char		max_fn_level;	/* Max functional ucode rev level    */
    u_char		max_rom_level;	/* Max PROM/Self-test ucode rev lev  */
    struct bus		*bus;		/* Bus structure pointer             */
} NPPCCB;

typedef struct _nppb	{		/* N_PORT CI Specific Fields of PB   */
    struct _npbh *scpkt;		/* Set circuit off command packet    */
    struct _npbh *invtcpkt;		/* Invalidate translation cache pkt  */
    struct _npbh *purgpkt;		/* Purge channel queues pkt	     */
    struct	{			/* Path status flags		     */
	u_int	cable0		:  1;	/*  Cable 0 status( Bad == 1 )	     */
	u_int	cable1		:  1;	/*  Cable 1 status( Bad == 1 )	     */
	u_int	cables_crossed	:  1;	/*  Cables crossed		     */
	u_int			: 29;
    } pstatus;
} NPPB;


#endif
