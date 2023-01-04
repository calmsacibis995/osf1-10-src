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
 *
 * derived from msiscs.h	2.7	(ULTRIX)	10/12/89
 */
/*
 *   Facility:	Systems Communication Architecture
 *		Mayfair Storage Interconnect Port Driver
 *
 *   Abstract:	This module contains Mayfair Storage Interconnect Port
 *		Driver( MSI ) constants and data structure definitions
 *		visible to SCS.
 *
 *   Creator:	Todd M. Katz	Creation Date:	December 06, 1988
 *
 *   Modification History:
 *
 *   16-Jun-1989	Pete Keilty
 *	Changes smp locks to type of struct lock_t.
 */

#ifndef _MSISCS_H_
#define _MSISCS_H_

/* MSI Data Structure Definitions.
 */
typedef struct _msi_dmapinfo {		/* Double Mapping Buffer Information */
    u_long		protopte;	/* Proto-pte( only PFN is missing )  */
/*  struct pte		*dmap_bpteaddr;	 Double mapping buffer sva pte ptr */
    char		*dmap_baddr;	/* Double mapping buffer address     */
    union	{			/* First overlaid field		     */
	u_char		*saddr;		/*  Target/Source segment address    */
	u_int		sboff;		/*  Target/Source segment byte offset*/
    } un1;
#define	Saddr	un1.saddr
#define	Sboff	un1.sboff
    u_int		ssize;		/* Number of bytes in segment	     */
} MSI_DMAPINFO;

typedef	struct _msi_portid {		/* Local/Remote MSI Port ID Info     */
    u_short		port_type[ 2 ];	/* Port type			     */
    struct _msirpi	portinfo;	/* Port information		     */
    u_short		mbz[ 10 ];	/* MBZ				     */
} MSI_PORTID;

typedef	struct _msi_pportinfo {		/* Per-DSSI Port Information	     */
    struct _msibq xretryq;		/* MSIB transmit retry queue	     */
    struct		{		/* Remote port status flags	     */
	u_int	path		:  1;	/*  Path exists			     */
	u_int	vc		:  1;	/*  Virtual circuit enabled	     */
	u_int	dip		:  1;	/*  Transmit delaying in progress    */
	u_int			: 29;
    } rpstatus;
					/* Remote port status flag bit masks */
#define	MSI_RPPATH	0x00000001	/*  ( used when performance matters )*/
#define	MSI_RPVC	0x00000002
#define	MSI_RPDIP	0x00000004
    u_int	  xretry_timer;		/* Xmt retry timer( 10 msec units )  */
    u_int	  xretrys;		/* Current transmit retry attempt    */
    u_int	  xseqno;		/* Next transmit sequence number     */
					/*  ( Occupies bits: 9-11 )	     */
    u_int	  rseqno;		/* Next expected receive seq number  */
					/*  ( Occupies bits: 9-11 )	     */
} MSI_PPORTINFO;

/* Field				     Lock
 * ---------------			--------------
 * comqh				COMQH
 * comql				COMQL
 * mfreeq				MFREEQ
 * dfreeq				DFREEQ
 * rbusy				RFP
 * xbusy				XFP
 * xfree				XFP
 * lpstatus.active			XFP/RFP - need both to modify
 * lpstatus.timer			XFP
 * rdmap				RFP
 * xdmap				XFP
 * perport.xretryq			XFP
 * perport.rpstatus.path		XFP
 * perport.rpstatus.vc			XFP/RFP - need both to modify
 * perport.rpstatus.dip			XFP
 * perport.xretry_timer			XFP
 * perport.xretrys			XFP
 * perport.xseqno			XFP
 * perport.rseqno			RFP
 *
 * All other volatile fields/bits controlled by standard PCCB lock.
 *
 * Lock Hierarchy: PCCB -> PB -> RFP -> XFP -> { COMQH,COMQL,DFREEQ,MFREEQ }
 */
#ifndef vu_short
#define vu_short volatile u_short
#endif

typedef	struct _msipccb	{		/* MSI Specific Fields of PCCB	     */
    struct _msibq	comqh;		/* MSIB high priority command queue  */
    struct _msibq	comql;		/* MSIB low priority command queue   */
    struct _msibq	mfreeq;		/* MSIB message free queue	     */
    struct _msibq	dfreeq;		/* MSIB datagram free queue	     */
    struct _siibq	*rbusy;		/* First rcv-in-progress SIIBUF ptr  */
    struct _siibq	*xbusy;		/* First xmt-in-progress SIIBUF ptr  */
    struct _siibq	*xfree;		/* First free transmit SIIBUF pointer*/
    u_char		*siibuffer;	/* SII 128K RAM buffer address	     */
    u_char		*siiregs;	/* SII registers base address	     */
    u_int		randomseed;	/* Random number generator seed	     */
    u_int		pkt_size;	/* Size of port command	packet	     */
    u_int		msg_ovhd;	/* Size of message overhead	     */
    u_int		dg_ovhd;	/* Size of datagram overhead	     */
    u_short		retdat_ovhd;	/* Size of RETDAT overhead	     */
    u_short		lretdat_cssize;	/* Size of local RETDAT comp section */
    struct		{		/* MSI register pointers	     */
	vu_short	*msicsr;	/*  Control/Status register	     */
	vu_short	*msidscr;	/*  DSSI control register	     */
	vu_short	*msidssr;	/*  DSSI status register	     */
	vu_short	*msiidr;	/*  ID register			     */
	vu_short	*msitr;		/*  Timeout register		     */
	vu_short	*msitlp;	/*  Target list pointer register     */
	vu_short	*msiilp;	/*  Initiator list pointer register  */
	vu_short	*msidcr;	/*  Diagnostic control register	     */
	vu_short	*msicomm;	/*  SII command register	     */
	vu_short	*msidstat;	/*  Data transfer status register    */
	vu_short        *msiisr3;       /*  Main control diagnostic register */
    } siiregptrs;
    struct		{		/* Local port status flags	     */
	u_int	init		:  1;	/*  First time initialization 	     */
	u_int	active		:  1;	/*  Port active			     */
	u_int	timer		:  1;	/*  Retry delay timer active 	     */
	u_int	xfork		:  1;	/*  Transmit Fork Process scheduled  */
	u_int	rfork		:  1;	/*  Receive Fork Process scheduled   */
	u_int   optlpcinfo	:  1;	/*  Opt local port crash info flag   */
	u_int			: 26;
    } lpstatus;
					/* Local port status flag bit masks  */
#define	MSI_ACTIVE	0x00000002	/*  ( used when performance matters )*/
#define	MSI_XFORK	0x00000008
#define	MSI_RFORK	0x00000010
    struct		{		/* Optional local port crash info    */
	struct _msih	*pkth;		/*  Address of MSI packet	     */
	u_int		pktsize;	/*  Size of MSI packet		     */
 	u_int		pport_addr;	/*  Packet remote port station addr  */
    } lpcinfo;
    union		{		/* Optional error logging information*/
	u_int		portnum;	/*  Remote port station address	     */
    } errlogopt;
    u_short		min_msg_size;	/* Minimum message size		     */
    u_short		max_msg_size;	/* Maximum message size		     */
    u_short		min_dg_size;	/* Minimum datagram size	     */
    u_short		max_dg_size;	/* Maximum datagram size	     */
    u_short		min_id_size;	/* Minimum ID size		     */
    u_short		max_id_size;	/* Maximum ID size		     */
    u_short		min_idreq_size;	/* Minimum IDREQ size		     */
    u_short		max_idreq_size;	/* Maximum IDREQ size		     */
    u_short		min_sntdat_size;/* Minimum SNTDAT size		     */
    u_short		max_sntdat_size;/* Maximum SNTDAT size		     */
    u_short		min_datreq_size;/* Minimum DATREQ{0,1,2} size	     */
    u_short		max_datreq_size;/* Maximum DATREQ{0,1,2} size	     */
    u_short		save_dssr;	/* Cached DSSI status register	     */
    u_short		save_dstat;	/* Cached data transfer status reg   */
    struct _msi_dmapinfo rdmap;		/* Receive Fork Process dmap buf info*/
    struct _msi_dmapinfo xdmap;		/* Transmit Fork Process dmap bufinfo*/
    struct kschedblk	rforkb;		/* Receive Fork Process fork block   */
    struct kschedblk	xforkb;		/* Transmit Fork Process fork block  */
    struct slock	comqh_lk;	/* Command queue high lock structure */
    struct slock	comql_lk;	/* Command queue low lock structure  */
    struct slock	dfreeq_lk;	/* Datagram free queue lock structure*/
    struct slock	mfreeq_lk;	/* Message free queue lock structure */
    struct slock	rfp_lk;		/* Receive Fork Process lock struct  */
    struct slock	xfp_lk;		/* Transmit Fork Process lock struct */
    struct _msi_portid	lpidinfo;	/* Local port identification info    */
    struct _msi_pportinfo perport[ MSI_MAXNUM_PORT ]; /* Per-DSSI port info  */
} MSIPCCB;

#endif
