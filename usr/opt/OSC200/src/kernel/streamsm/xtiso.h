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
 * @(#)$RCSfile: xtiso.h,v $ $Revision: 4.2.12.6 $ (DEC) $Date: 1993/10/18 21:38:03 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#ifndef _XTISO_H
#define _XTISO_H

#include <streamsm/xtiso_config.h>	/* struct xti_proto ...	*/
#include <streamsm/xti.h>

extern int somaxconn;


/* 
 * Connection Indication Structure
 */
struct xtiseq { 
	long    seq_no;			/* sequence number		*/
	int     seq_used;		/* used or not?			*/
	struct  socket *seq_so;		/* associated socket		*/
};

/* Statically defined */
#define XTI_MAXOPENS	(minor(~0) + 1)

/* Support for kernel configured variable max_accept_conn. */
#ifndef XTI_CONFIGURABLE_QLEN
#define XTI_MAXQLEN	8		/* Remainder queued in socket	*/
#endif

#if XTI_XPG4
struct xti_def_opts {
	unsigned long		xti_rcvbuf;
	unsigned long		xti_rcvlowat;
	unsigned long		xti_sndbuf;
	unsigned long		xti_sndlowat;
	struct linger		xti_linger;
	char			xti_debug;
	/* IP specific - union with other protocols later */
	char			ip_broadcast;
	char			ip_dontroute;
	char			ip_reuseaddr;
	struct mbuf *		ip_options;
	unsigned char		ip_tos;
	unsigned char		ip_ttl;
	char			udp_checksum;
	char			tcp_nodelay;
	unsigned long		tcp_maxseg;
	struct t_kpalive	tcp_keepalive;
};
#endif

/*
 * XTI-Socket Control Block
 */
struct xticb {
	int	 xti_state;		/* state of transport provider	*/
	long	 xti_flags;		/* internal flags, see below	*/
	dev_t    xti_minor;             /* minor device # 		*/

	struct   socket   *xti_so;	/* corresponding socket		*/
	int	 xti_sostate;		/* safe state of socket		*/
	int	 xti_soerror;		/* safe socket error		*/
	long	 xti_sorcount;		/* safe read-count of socket	*/
	long	 xti_sowspace;		/* safe write-space of socket	*/
	int	 xti_sosnap;		/* async state of socket	*/

	struct   socket   *xti_lso;	/* "saved" listening socket	*/
	int	 xti_maxqlen;
#ifdef XTI_CONFIGURABLE_QLEN
	struct   xtiseq    *xti_seq;		
#else
	struct   xtiseq    xti_seq[XTI_MAXQLEN];		
#endif
					/* outstanding conn ind seq #s	*/
	short	 xti_qlen;		/* # of conn ind. allowed	*/
        short	 xti_cindno;		/* # of outstanding conn ind	*/

	int 	 xti_seqcnt;		/* sequence counter 		*/
        int	 xti_pendind;		/* pending indication		*/
	long     xti_tsdu;
	long     xti_etsdu;

	queue_t  *xti_rq;		/* stream read queue ptr	*/
	queue_t  *xti_wq;		/* stream write queue ptr	*/

	mblk_t	 *xti_wndata;		/* pending normal data for write */
	mblk_t	 *xti_wexpdata;		/* pending expedited data for write */
	struct   mbuf *xti_wnam;	/* destination addr of unitdata	*/

	struct   mbuf *xti_rdata;	/* pending read-side data	*/
	struct   mbuf *xti_rnam;	/* source addr of unitdata      */
	int      xti_rflags;		/* pending read-side flags	*/
					/* xti error recovery		*/
	int	(*xti_pendcall)();
	int	xti_bufcallid;
	int	xti_tlierr;
	int	xti_unixerr;
	int	xti_errtype;

	struct   xtisocfg *xti_cfg;	/* link back to cfg head	*/
#define	xti_proto xti_cfg->xti_cfgproto	/* xtiproto for xticb		*/

#if	XTI_XPG4
	struct xti_def_opts xti_def_opts; /* options managed from app	*/
#endif
};

/*
 * XTI configuration block.
 */
struct xtisocfg {
	struct	xtisocfg *	xti_cfgnext;	/* more config structs	*/
	dev_t 			xti_cfgmajor;	/* major device # 	*/
	int			xti_cfgnopen;	/* # of current opens	*/
	struct	xtiproto	xti_cfgproto;	/* protocol information	*/
#ifndef XTI11
	u_char			xti_cfgminor[1];/* allocated minor devs */
						/* calculated at config time */
#endif
};

/* 
 * If the transport supports options management then extract the following 
 * It serves as a part of the t_info structure which can be changed through
 * t_optmgmt().
 */
struct topt_xtiinfo
{
    int	etsdulen;			/* expedited data len */
    int	connectlen;			/* connect user data len */
    int disconlen;			/* disconnect user data len */
};

/*
 * xti_proto_info uses the following defines
 */
#define XTI_NO_OPTS	0		/* There are no setopts/getopts */
#define XTI_NS		-2L		/* Not supported */

/*
 * Socket types
 */
#define XTI_NEWSOCK     1		/* New Socket, open		*/
#define XTI_NOSOCK	2		/* Socket disabled, no close 	*/
#define XTI_CLOSESOCK	3		/* Close indicator 		*/

/* 
 * Internal XTI Flags
 */
#define XTI_ACTIVE	0x0001		/* if set, this xticb is in use */
#define XTI_FATAL	0x0002          /* fatal condition occurred     */
#define XTI_DISCONMAIN  0x0004          /* disconnect the main conn.    */
#define XTI_PRIV        0x0008          /* created by privileged user   */
#define XTI_MORENDATA	0x0010          /* more normal data expected from TU */
#define XTI_MOREEXPDATA	0x0020		/* more expedited normal data
					   expected from TU */
#define XTI_FLOW	0x0040		/* flow control for normal data  */
#define XTI_SETEOR      0x0080          /* EOR flag for record-oriented  TP */
#define XTI_TIMEOUT	0x0200		/* timeout active 		*/
#define XTI_ISCLOSING	0x0400		/* close in progress		*/
#define XTI_OPTINIT	0x1000		/* options initialized		*/
#define XTI_OPTMODIFY	0x2000		/* options modified		*/

/* 
 * State for connection indications
 */
#define	XTIS_AVAILABLE	0		/* Available			*/
#define XTIS_AWAITING	1		/* Awaiting acceptance		*/
#define XTIS_ACTIVE	2		/* Connected/accepted		*/
#define XTIS_LOST	3		/* Connection lost before accept*/

/*
 * Disconnect reason codes
 */
#define XTID_TPINIT		1  /* Initiated by transport provider    */
#define XTID_REMWITHDRAW	2  /* Connect req withdrawn by remote    */
#define XTID_REMREJECT  	3  /* Connect req rejected by remote     */
#define XTID_REMINIT		4  /* Disconnect req Initiated by remote */

/*
 * UDERR IND error types
 */
#define XTIU_BADOPTSZ		1  /* Initiated by transport provider    */
#define XTIU_BADMSGSZ		2  /* Connect req withdrawn by remote    */
#define XTIU_NOOPT		1  /* UDP Option not supported           */

#define XTI_INFO_ID	5010	/* module/driver ID */

/*
 * Flow control parameters - big enough for socket's default
 * TCP window, with lowater set to refill promptly.
 */
#define XTI_INIT_HI	6400
#define XTI_INIT_LO	(XTI_INIT_HI - 512)

/*
 * Debugging
 */
#if	XTIDEBUG

#define XTIF_ALL	(~0)
#define XTIF_CONFIGURE	0x00000001	/* configuration  		*/
#define XTIF_OPEN	0x00000002	/* open routine  		*/
#define XTIF_WPUT	0x00000004	/* write put routine 		*/
#define XTIF_MISC	0x00000008	/* miscellaneous trace 		*/
#define XTIF_WSRV	0x00000010	/* write service routine	*/
#define XTIF_OUTPUT	0x00000020	/* xti output routine		*/
#define XTIF_INPUT	0x00000040	/* xti input routine		*/
#define XTIF_EVENTS	0x00000080	/* socket event tracing		*/
#define XTIF_INFO	0x00000100	/* info request			*/
#define XTIF_BINDING	0x00000200	/* binding-related tracing	*/
#define XTIF_CONNECT	0x00000400	/* connect/disconnect trace	*/
#define XTIF_DATA	0x00000800	/* data xfer trace		*/
#define XTIF_SEND	0x00001000	/* xti send routine trace	*/
#define XTIF_RECV	0x00002000	/* xti receive routine trace 	*/
#define XTIF_CLOSE	0x00004000	/* close-related trace		*/
#define XTIF_ERRORS	0x00008000	/* stream error trace		*/
#define XTIF_SOCKET	0x00010000	/* socket misc			*/
#define XTIF_CHECKPOINT	0x00020000	/* routine entry trace		*/
#define XTIF_SEND_FLOW	0x00040000	/* send flow control trace	*/
#define XTIF_STRLOG	0x00080000	/* enable strlog trace		*/
#define XTIF_OPTMGMT	0x00100000	/* optmgmt stuff 		*/
#define XTIF_BREAK	0x40000000	/* enable debugger on "panic"	*/
#define XTIF_PANIC	0x80000000	/* enable "real" panic		*/

extern int xtiDEBUG;

#define PUTNEXT(q,mp)	xti_putnext((q),(mp))
#define XTITRACE(f,v)	do { if (xtiDEBUG & (f)) { v } } while (0)
#define CHECKPOINT(v)	XTITRACE(XTIF_CHECKPOINT, \
			 printf( "\n xtiso: ========>> entered %s()...\n",(v));)

#else	/* !XTIDEBUG */

#define PUTNEXT(q,mp)	putnext((q),(mp))
#define XTITRACE(f,v)
#define CHECKPOINT(v)

#endif	/* !XTIDEBUG */

#endif /* _XTISO_H */
