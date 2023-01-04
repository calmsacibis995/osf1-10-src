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
 * @(#)$RCSfile: npadapter.h,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/13 17:47:23 $
 */
/*
 * derived from npadapter.h	5.2	(ULTRIX)	10/16/91
 */
/*
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect N_PORT Port Driver
 *
 *   Abstract:	This module contains Computer Interconnect N_PORT
 *		Port Driver constants, data structure definitions, 
 *		and macros required to link N_PORT CI ports and their 
 *		driver with lower level machine specific portions of the 
 *		Ultrix kernel.
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
 *	Updates/bug fixes.
 *
 */

#ifndef _NPADAPTER_H_
#define _NPADAPTER_H_
/*
 * Constants.
 */
#ifdef __alpha
#define	NP_ADAPSIZE		16	/* Size adapter I/O space( pages )   */
#else
#define	NP_ADAPSIZE		32	/* Size adapter I/O space( pages )   */
#endif /* __alpha */

/* Data Structures.
 *
 * Adapter Interface Block Definition.
 *
 */

typedef struct _npq	{		/* N_PORT Queues  		     */
    PTR		head_ptr;		/* Queue header pointer		     */
    PTR		tail_ptr;		/* Queue tail pointer		     */
} NPQ;

typedef struct _npccq	{		/* N_PORT Channel Command Queue      */
    u_int  	resrv[4];
    NPQ	   	dccq2;			/* Driver to channel command queue 2 */
    NPQ	   	dccq1;			/* Driver to channel command queue 1 */
    NPQ	   	dccq0;			/* Driver to channel command queue 0 */
    NPQ	   	cccq3;			/* Channel to channel command que 3  */
    NPQ	   	cccq2;			/* Channel to channel command que 2  */
    NPQ	   	cccq1;			/* Channel to channel command que 1  */
    NPQ	   	cccq0;			/* Channel to channel command que 0  */
} NPCCQ;

typedef struct _npcpb	{		/* N_PORT Channel Parameter Block    */
    u_short	xp_addr;		/* Xport_entity address		     */
    u_char	max_pgrp;		/* Max. P_GRP value supported	     */
    u_char	max_mem;		/* Max. MEM value supported	     */
    u_int	: 32;			/* SBZ				     */
    union {
        struct 	{
    	    u_short	dlink_typ;	/* Datalink type - 0 ci              */
	    u_char	dlink_addr;	/* Datalink address                  */
	    u_char 	arb_slot;	/* Arbiration slot		     */
	    u_char	arb_slotsiz;	/* Arbiration slot size 	     */
	    u_char	arb_modulus;	/* Arbiration modulus 		     */
	    u_char	arb_mode;	/* Arbiration mode - 0 star; 1 switch*/
	    u_char		:  8;	/* SBZ				     */
        } ci;
        struct 	{
    	    u_short	dlink_typ;	/* Datalink type - 1 dssi	     */
	    u_char	dlink_addr;	/* Datalink address		     */
	    u_char		:  8;	/* SBZ				     */
	    u_int		: 32;	/* SBZ				     */
        } dssi;
    } type;
    u_int	hold[4];		/* Implementation Specific           */
} NPCPB;

typedef struct _npapb	{		/* N_PORT Adapter Parameter Block    */
    u_short	vcdt_len;		/* VC descriptor table size	     */
    u_short	ibuf_len;		/* Internal buffer size		     */
    u_short	ramp;			/* Requested adpater memory pointer  */
    u_short	media		:  4;	/* 0 - CI, 1 - DSSI		     */
    u_short	rdp		:  1;	/* RDP supported when set	     */
    u_short	es		:  1; 	/* Explicit state supported when set */
    u_short	ea		:  1;	/* Explicited address supported	     */
    u_short	sdp		:  1;	/* Short data page supported	     */
    u_short	sic		:  1;	/* Single interrupt completion	     */
    u_short			:  7;	/* SBZ				     */
    u_int	hold[2];		/* Implementation Specific           */
    u_int	sbz[4];			/* SBZ				     */
} NPAPB;

typedef struct _npab	{		/* N_PORT Adapter Block  	     */
    NPCCQ	chnl_cmdq[16];		/* Channel command queues - c_idx    */
    NPQ		adrq;			/* Adapter-driver response queue     */
    NPQ		dadfq;			/* Driver-Adapter Dg free queue      */
    NPQ		addfq;			/* Adapter-driver Dg free queue      */
    NPQ		damfq;			/* Driver-adapter Msg free queue     */
    NPQ		admfq;			/* Adapter-driver Msg free queue     */
    NPQ		aadfq;			/* Adapter-adapter Dg free queue     */
    NPQ		aamfq;			/* Adapter-adapter Msg free queue    */
    PTR		bdlt_base;		/* Physical address of BDLT	     */
    u_int	bdlt_len;		/* Lenght of BDLT in bytes	     */
    u_int	keep_alive;		/* Maintenance/sanity timer period   */
    u_int	dqe_len;		/* Datagram q_buffer length in bytes */
    u_int	mqe_len;		/* Message q_buffer length in bytes  */
    u_int	intr_holdoff[2];	/* Completion intr holdoff timer cntl*/
    PTR		ampb_base;		/* Physical address of AMPB	     */
    u_int	ampb_len;		/* Length of AMPB in bytes	     */
    u_int	resrv1[1];		/* Resserved area		     */
    u_int	drv_param;		/* Driver parameters		     */
    u_int	resrv2[7];		/* Resserved area		     */
    NPAPB	adap_pb;		/* Adapter parameter block	     */
    NPCPB	chnl_pb[16];		/* Channel parameter block - c_idx   */
} NPAB;

typedef	struct _npadap	{		/* N_PORT CI Adapter Interface Block */
    NPAB	   npab;		/* N_PORT adapter block structure    */
    void	   ( *arsp )(); 	/* Adapter Rsp ISR address   	     */
    void	   ( *amisc )(); 	/* Adapter Misc ISR address  	     */
    void	   ( *isr[16] )(); 	/* Channels Misc ISR address 	     */
    struct _pccb   *pccb[16];		/* PCCB pointers		     */
    void	   ( *mapped_isr )();	/* Mapped port ISR addr( OPTIONAL )  */
#ifdef __alpha
    volatile struct xmi_reg *phyaddr;	/* Adapter I/O space physical address*/
    volatile struct xmi_reg *viraddr;	/* Adapter I/O space virtual address */
#else
    unsigned char  *phyaddr;		/* Adapter I/O space physical address*/
    unsigned char  *viraddr;		/* Adapter I/O space virtual address */
    struct pte	   *iopte;		/* Adapter I/O space PTE pointer     */
#endif /* __alpha */
    unsigned short npages;		/* Size adapter I/O space( pages )   */
    unsigned char  icnum;		/* Interconnect number		     */
#define	Binum		icnum
#define	Sbinum		icnum
#define	Xminum		icnum
#define	Tcnum		icnum
    unsigned char  nexnum;		/* Nexus/Node number		     */
#define	Binode		nexnum
#define	Sbinexus	nexnum
#define	Xminode		nexnum
#define	Tcnode		nexnum
    struct	{
	u_int	reset   : 1;		/* Adapter reset needed	 	     */
	u_int	reset_ip: 1;		/* Adapter reset in progress 	     */
	u_int	pccb	: 1;		/* Adapter has control pccb	     */
	u_int		:28;
    } status;
    union	{			/* Interconnect dependent fields     */
	struct		{		/*  XMI only fields( CIXCD/CIKMF )   */
	    vu_long	*xdev;		/*   XMI device type register	     */
	    vu_long	*xber;		/*   XMI bus error register	     */
	    vu_long	*xfadrl;	/*   XMI failing address register low*/
	    vu_long	*xfadrh;	/*   XMI failing address register hi */
	} xmi;
	struct		{		/*  TC only fields( CITCA )   */
	    vu_long	*dev;		/*   TC device type register	     */
	    vu_long	*ber;		/*   TC bus error register	     */
	} tc;
    } ic;
    union	{			/* Port dependent fields     */
	struct		{		/*  Adapter only fields( CIMNA )     */
	    vu_long	*aidr;		/*   Port interrupt destination reg  */
	    vu_long	*amivr;		/*   Port vector register	     */
	    vu_long	*acivr;		/*   Port vector register	     */
	    u_int 	aid;		/*   Port int dst reg contents	     */
	    u_int 	amiv;	 	/*   Port vector register contents   */
	    u_int 	aciv;	 	/*   Port vector register contents   */
	    vu_long 	*pdcsr;		/*   Adapter diag cntrl status reg   */
	    vu_long 	*asnr;		/*   Adapter serial number register  */
	    vu_long 	*asubr;		/*   Adapter subsitute register  */
	} mna;
	struct		{		/*  Adapter only fields( CITCA )     */
	    vu_long 	*pdcsr;		/*   Adapter diag cntrl status reg   */
	    vu_long 	*asnr;		/*   Adapter serial number register  */
	} tca;
    } adap;
    struct _npbq	cfreeq;		/* Carrier free queue head	     */
    struct _npbq	cinuseq;	/* Carrier insue queue head	     */
    struct _npbq	dfreeq;		/* Dg free queue head	     */
    struct _npbq	mfreeq;		/* Msg free queue head	     */
} NPADAP;

typedef	struct _tca_reg	{		/* CI Turbochannel Adapter Regs	     */
    u_int	tca_dtype;
#define TC_DTYPE 	0xFFFF		/* TC device type mask */
#define TC_CITCA 	0x00CA		/* TC device type */
    u_int       tca_reserved;
    u_int	tca_ber;
} TCAREG;

#define	Mna_dev		Npadap->ic.xmi.xdev
#define	Mna_ber		Npadap->ic.xmi.xber
#define	Mna_fadrh	Npadap->ic.xmi.xfadrh
#define	Mna_fadrl	Npadap->ic.xmi.xfadrl
#define	Mna_aidr	Npadap->adap.mna.aidr
#define	Mna_amivr	Npadap->adap.mna.amivr
#define	Mna_acivr	Npadap->adap.mna.acivr
#define	Mna_aid		adap.mna.aid
#define	Mna_amiv	adap.mna.amiv
#define	Mna_aciv	adap.mna.aciv
#define Mna_pdcsr	Npadap->adap.mna.pdcsr
#define Mna_asnr	Npadap->adap.mna.asnr
#define Mna_asubr	Npadap->adap.mna.asubr
#define Tca_dev		Npadap->ic.tc.dev
#define Tca_ber		Npadap->ic.tc.ber
#define Tca_pdcsr	Npadap->adap.tca.pdcsr
#define Tca_asnr	Npadap->adap.tca.asnr

#endif
