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
 *	@(#)$RCSfile: if.h,v $ $Revision: 4.3.14.10 $ (DEC) $Date: 1993/11/10 14:33:38 $
 */

/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	Base:	if.h	7.6 (Berkeley) 9/20/89
 *	Merged: if.h	7.9 (Berkeley) 6/28/90
 */
/*
 *	Revision History:
 *
 * 5-June-91	Heather Gray
 *	OSF 1.0.1 patch
 *
 */

/*
 * Structures defining a network interface, providing a packet
 * transport mechanism (ala level 0 of the PUP protocols).
 *
 * Each interface accepts output datagrams of a specified maximum
 * length, and provides higher level routines with input datagrams
 * received from its medium.
 *
 * Output occurs when the routine if_output is called, with four parameters:
 *	(*ifp->if_output)(ifp, m, dst, ro)
 * Here m is the mbuf chain to be sent and dst is the destination address.
 * The output routine encapsulates the supplied datagram if necessary,
 * and then transmits it on its medium.
 *
 * On input, each interface unwraps the data received by it, and either
 * places it on the input queue of a internetwork datagram routine
 * and posts the associated software interrupt, or passes the datagram to a raw
 * packet input routine.
 *
 * Routines exist for locating interfaces by their addresses
 * or for locating a interface on a certain network, as well as more general
 * routing and gateway routines maintaining information used to locate
 * interfaces.  These routines live in the files if.c and route.c
 */

#ifndef _NET_IF_H_
#define _NET_IF_H_

#if	defined(_KERNEL) && !defined(_NET_GLOBALS_H_)
#include "net/net_globals.h"
#endif

#ifdef	_KERNEL
#include "netinet/if_trnstat.h"
#ifndef	_TIME_
#include "sys/time.h"
#endif
#else
#include <netinet/if_trnstat.h>
#ifndef	_TIME_
#include <sys/time.h>
#endif
#endif

/*
 * Structure defining a queue for a network interface.
 */

struct ifnet {
	struct	ifnet *if_next;		/* the next structure in the list */
	char	*if_name;		/* name, e.g. ``en'' or ``lo'' */
	char	*if_version;		/* The version string.		   */
	struct	sockaddr if_addr;	/* address of interface */
	int	if_flags;		/* up/down, broadcast, etc. */
	short	if_unit;		/* sub-unit for lower level driver */
	short	if_mtu;			/* maximum IP transmission unit */
	short	if_mediamtu;		/* maximum MTU of the media */
	short	if_timer;		/* time 'til if_watchdog called */
	int	if_metric;		/* routing metric (external only) */
	struct	ifaddr *if_addrlist;	/* linked list of addresses per if */
	struct	ifmulti *if_multiaddrs; /* list of multicast addrs */
	int	if_multicnt;		/* number of multicast addrs in list */	
	int	if_allmulticnt;		/* number of allmulti requests */	
/* procedure handles */
	int	(*if_init)();		/* init routine */
	int	(*if_output)();		/* output routine (enqueue) */
	int	(*if_start)();		/* initiate output routine */
	int	(*if_done)();		/* output complete routine */
	int	(*if_ioctl)();		/* ioctl routine */
	int	(*if_reset)();		/* bus reset routine */
	int	(*if_watchdog)();	/* timer routine */
/* generic interface statistics */
	int	if_ipackets;		/* packets received on interface */
	int	if_ierrors;		/* input errors on interface */
	int	if_opackets;		/* packets sent on interface */
	int	if_oerrors;		/* output errors on interface */
	int	if_collisions;		/* collisions on csma interfaces */
	int	if_sysid_type;		/* MOP SYSID device code */
/* SNMP statistics */
	struct	timeval if_lastchange;	/* last updated */
	int	if_ibytes;		/* total number of octets received */
	int	if_obytes;		/* total number of octets sent */
	int	if_imcasts;		/* packets received via multicast */
	int	if_omcasts;		/* packets sent via multicast */
	int	if_iqdrops;		/* dropped on input, this interface */
	int	if_noproto;		/* destined for unsupported protocol */
	int	if_baudrate;		/* linespeed */
/* end statistics */
	u_char	if_type;		/* ethernet, tokenring, etc */
	u_char	if_addrlen;		/* media address length */
	u_char	if_hdrlen;		/* media header length */
	u_char	if_index;		/* numeric abbreviation for this if  */
/* For future expansion */
	int	futureuse_4;		/* to be used for future expansion */
	int	futureuse_3;		/* to be used for future expansion */
	int	futureuse_2;		/* to be used for future expansion */
	int	futureuse_1;		/* to be used for future expansion */
	struct	ifqueue {
		struct	mbuf *ifq_head;
		struct	mbuf *ifq_tail;
		int	ifq_len;
		int	ifq_maxlen;
		int	ifq_drops;
#if	!defined(_KERNEL) || !NETSYNC_LOCK
	} if_snd;			/* output queue */
#else	/* Locks go last to make structure appear to have constant size */
		simple_lock_data_t	ifq_slock;
	} if_snd;			/* output queue */
#ifdef	LOCK_NETSTATS
	simple_lock_data_t	if_slock;	/* statistics lock */
#endif
#endif
};
#define	IFF_UP		0x1		/* interface is up */
#define	IFF_BROADCAST	0x2		/* broadcast address valid */
#define	IFF_DEBUG	0x4		/* turn on debugging */
#define	IFF_LOOPBACK	0x8		/* is a loopback net */
#define	IFF_POINTOPOINT	0x10		/* interface is point-to-point link */
#define	IFF_NOTRAILERS	0x20		/* avoid use of trailers */
#define	IFF_RUNNING	0x40		/* resources allocated */
#define	IFF_NOARP	0x80		/* no address resolution protocol */
#define	IFF_PROMISC	0x100		/* receive all packets */
#define	IFF_ALLMULTI	0x200		/* receive all multicast packets */
#define IFF_MULTICAST	0x400		/* supports multicast */
#define	IFF_SIMPLEX	0x800		/* can't hear own transmissions */
#define	IFF_OACTIVE	0x1000		/* transmission in progress */
#define IFF_PFCOPYALL   0x2000          /* pfilt gets packets to this host */

/* Device-specific flags */
#define IFF_D1		0x8000
#define IFF_D2		0x4000
#define IFF_SNAP	IFF_D1		/* Ethernet driver outputs SNAP hdr */

/* flags set internally only: */
#define	IFF_CANTCHANGE	\
	(IFF_BROADCAST|IFF_POINTOPOINT|IFF_SIMPLEX|IFF_RUNNING|\
	IFF_OACTIVE|IFF_MULTICAST)

/* interface types for benefit of parsing media address headers */
#define IFT_OTHER	0x1		/* none of the following */
#define IFT_1822	0x2		/* old-style arpanet imp */
#define IFT_HDH1822	0x3		/* HDH arpanet imp */
#define IFT_X25DDN	0x4		/* x25 to imp */
#define IFT_X25		0x5		/* PDN X25 interface */
#define	IFT_ETHER	0x6		/* Ethernet I or II */
#define	IFT_ISO88023	0x7		/* CMSA CD */
#define	IFT_ISO88024	0x8		/* Token Bus */
#define	IFT_ISO88025	0x9		/* Token Ring */
#define	IFT_ISO88026	0xa		/* MAN */
#define	IFT_STARLAN	0xb
#define	IFT_P10		0xc		/* Proteon 10MBit ring */
#define	IFT_P80		0xd		/* Proteon 10MBit ring */
#define IFT_HY		0xe		/* Hyperchannel */
#define IFT_FDDI	0xf
#define IFT_LAPB	0x10
#define IFT_SDLC	0x11
#define IFT_T1		0x12
#define IFT_CEPT	0x13
#define IFT_ISDNBASIC	0x14
#define IFT_ISDNPRIMARY	0x15

#if	NETSYNC_LOCK
/*
 * Multiprocessor queue locking.
 *
 * Note that the IF_QFULL and IF_DROP macros become racy in an mp environment.
 * The exact number of ifq_drops probably isn't important; on the other hand,
 * it is possible that an unlocked ifq could grow larger than its declared
 * ifq_maxlen as processors race between IF_QFULL and IF_ENQUEUE.  However,
 * it is still ABSOLUTELY NECESSARY that modification of ifq_len be locked!
 *
 * Note also that any macros which take the IFQ_LOCK must be done at splimp.
 *
 */
#define IFQ_LOCKINIT(ifq)	simple_lock_init(&((ifq)->ifq_slock))
#define IFQ_LOCK(ifq)		simple_lock(&((ifq)->ifq_slock))
#define IFQ_UNLOCK(ifq)		simple_unlock(&((ifq)->ifq_slock))
#else
#define IFQ_LOCKINIT(ifq)
#define IFQ_LOCK(ifq)
#define IFQ_UNLOCK(ifq)
#endif

/*
 * Output queues (ifp->if_snd) and internetwork datagram level (pup level 1)
 * input routines have queues of messages stored on ifqueue structures
 * (defined above).  Entries are added to and deleted from these structures
 * by these macros, which should be called with ipl raised to splimp().
 */
#define	IF_QFULL(ifq)		((ifq)->ifq_len >= (ifq)->ifq_maxlen)
#define	IF_DROP(ifq)		((ifq)->ifq_drops++)
#define	IF_ENQUEUE_NOLOCK(ifq, m) { \
	(m)->m_nextpkt = 0; \
	if ((ifq)->ifq_tail == 0) \
		(ifq)->ifq_head = m; \
	else \
		(ifq)->ifq_tail->m_nextpkt = m; \
	(ifq)->ifq_tail = m; \
	(ifq)->ifq_len++; \
}
#define	IF_ENQUEUE(ifq, m) { \
	IFQ_LOCK(ifq); \
	IF_ENQUEUE_NOLOCK(ifq, m); \
	IFQ_UNLOCK(ifq); \
}
#define	IF_PREPEND_NOLOCK(ifq, m) { \
	(m)->m_nextpkt = (ifq)->ifq_head; \
	if ((ifq)->ifq_tail == 0) \
		(ifq)->ifq_tail = (m); \
	(ifq)->ifq_head = (m); \
	(ifq)->ifq_len++; \
}
#define	IF_PREPEND(ifq, m) { \
	IFQ_LOCK(ifq); \
	IF_PREPEND_NOLOCK(ifq, m); \
	IFQ_UNLOCK(ifq); \
}
#define	IF_DEQUEUE_NOLOCK(ifq, m) { \
	(m) = (ifq)->ifq_head; \
	if (m) { \
		if (((ifq)->ifq_head = (m)->m_nextpkt) == 0) \
			(ifq)->ifq_tail = 0; \
		(m)->m_nextpkt = 0; \
		(ifq)->ifq_len--; \
	} \
}
#define	IF_DEQUEUE(ifq, m) { \
	IFQ_LOCK(ifq); \
	IF_DEQUEUE_NOLOCK(ifq, m); \
	IFQ_UNLOCK(ifq); \
}

#define	IFQ_MAXLEN	512
#define	IFNET_SLOWHZ	1		/* granularity is 1 second */

/*
 * The ifaddr structure contains information about one address
 * of an interface.  They are maintained by the different address families,
 * are allocated and attached when an address is set, and are linked
 * together so all addresses for an interface can be located.
 */
struct ifaddr {
	struct	sockaddr *ifa_addr;	/* address of interface */
	struct	sockaddr *ifa_dstaddr;	/* other end of p-to-p link */
#define	ifa_broadaddr	ifa_dstaddr	/* broadcast address interface */
	struct	sockaddr *ifa_netmask;	/* used to determine subnet */
	struct	ifnet *ifa_ifp;		/* back-pointer to interface */
	struct	ifaddr *ifa_next;	/* next address for interface */
	void	(*ifa_rtrequest)();	/* check or clean routes (+ or -)'d */
	struct	rtentry *ifa_rt;	/* ??? for ROUTETOIF */
	u_short	ifa_flags;		/* mostly rt_flags for cloning */
	u_short	ifa_llinfolen;		/* extra to malloc for link info */
};
#define IFA_ROUTE	RTF_UP		/* route installed */
/*
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with ifr_name.  The
 * remainder may be interface specific.
 */
struct	ifreq {
#define	IFNAMSIZ	16
	char	ifr_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	union {
		struct	sockaddr ifru_addr;
		struct	sockaddr ifru_dstaddr;
		struct	sockaddr ifru_broadaddr;
		int	ifru_flags;
		int	ifru_metric;
		caddr_t	ifru_data;
		int	ifru_value;		/* any generic value */
	} ifr_ifru;
#define	ifr_addr	ifr_ifru.ifru_addr	/* address */
#define	ifr_dstaddr	ifr_ifru.ifru_dstaddr	/* other end of p-to-p link */
#define	ifr_broadaddr	ifr_ifru.ifru_broadaddr	/* broadcast address */
#define	ifr_flags	ifr_ifru.ifru_flags	/* flags */
#define	ifr_metric	ifr_ifru.ifru_metric	/* metric */
#define	ifr_data	ifr_ifru.ifru_data	/* for use by interface */
#define	ifr_value	ifr_ifru.ifru_value	/* for use by interface */
};

/*
 * structure used to query de and qe for physical addresses
 */
struct ifdevea {
        char    ifr_name[IFNAMSIZ];             /* if name, e.g. "en0" */
        u_char default_pa[6];                   /* default hardware address */
        u_char current_pa[6];                   /* current physical address */
};

struct ifaliasreq {
	char	ifra_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	struct	sockaddr ifra_addr;
	struct	sockaddr ifra_broadaddr;
	struct	sockaddr ifra_mask;
};

/*
 * Structure used in SIOCGIFCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible).
 */
struct	ifconf {
	int	ifc_len;		/* size of associated buffer */
	union {
		caddr_t	ifcu_buf;
		struct	ifreq *ifcu_req;
	} ifc_ifcu;
#define	ifc_buf	ifc_ifcu.ifcu_buf	/* buffer address */
#define	ifc_req	ifc_ifcu.ifcu_req	/* array of structures returned */
};

/*
 * Interface multicast address structure.  There is one of these for each
 * multicast address or range of multicast addresses that we are supposed
 * to listen to on a particular interface.  They are kept in a linked list,
 * rooted in the interfaces's ifnet structure. 
 */
struct ifmulti {
	u_char	ifm_addrlo[6];		/* low  or only address of range */
	u_char	ifm_addrhi[6];		/* high or only address of range */
	struct	ifnet *ifm_ifnet;	/* back pointer to ifnet */
	u_int	ifm_refcount;		/* no. claims to this addr/range */
	struct	ifmulti *ifm_next;	/* ptr to next ifmulti */
};

#ifdef _KERNEL
/*
 * Minimum MTU size for IP: Used by drivers when setting ipmtu.
 */
#define	IP_MINMTU	68

/*
 * Structure used by macros below to remember position when stepping through
 * all of the ifmulti records.
  */
struct if_multistep {
	struct ifmulti  *if_ifm;
};

/*
 * Macro for looking up the ifmulti record for a given range of interface
 * multicast addresses connected to a given ifnet structure.  If no matching
 * record is found, "ifm" returns NULL.
 */
#define IF_LOOKUP_MULTI(addrlo, addrhi, ifp, ifm) \
	/* u_char addrlo[6]; */ \
	/* u_char addrhi[6]; */ \
	/* struct ifnet *ifp; */ \
	/* struct ifmulti *ifm; */ \
{ \
	for ((ifm) = (ifp)->if_multiaddrs; \
	    (ifm) != NULL && \
	    (bcmp((ifm)->ifm_addrlo, (addrlo), 6) != 0 || \
	     bcmp((ifm)->ifm_addrhi, (addrhi), 6) != 0); \
		(ifm) = (ifm)->ifm_next); \
}

/*
 * Macro to step through all of the ifmulti records, one at a time.
 * The current position is remembered in "step", which the caller must
 * provide.  IF_FIRST_MULTI(), below, must be called to initialize "step"
 * and get the first record.  Both macros return a NULL "ifm" when there
 * are no remaining records.
 */
#define IF_NEXT_MULTI(step, ifm) \
	/* struct if_multistep step; */  \
	/* struct ifmulti *ifm; */  \
{ \
	if (((ifm) = (step).if_ifm) != NULL) \
		(step).if_ifm = (ifm)->ifm_next; \
}

#define IF_FIRST_MULTI(step, ifp, ifm) \
	/* struct if_multistep step; */ \
	/* struct ifnet *ifp; */ \
	/* struct ifmulti *ifm; */ \
{ \
	(step).if_ifm = (ifp)->if_multiaddrs; \
	IF_NEXT_MULTI((step), (ifm)); \
}
#endif /* _KERNEL */

/*
 * Structure for EEPROM downline upgrades. (Supported by FDDI adapters)
 */
struct ifeeprom {
        char    ife_name[IFNAMSIZ];             /* if name, e.g. "fza0" */
        u_char  ife_data[64];                   /* block of EEPROM data */
        u_int  ife_offset;                     /* byte offset from base */
        u_int  ife_blklen;                     /* len of EEPROM block */
        u_int  ife_lastblk;                    /* True if last block */
};
#define IFE_NOTLAST     0x0     /* Intermediary block of EEPROM image */
#define IFE_LASTBLOCK   0x1     /* Last block of EEPROM image */
#define IFE_SUCCESS     0x0     /* Block of EEPROM successfully written */
#define IFE_RETRY       0x1     /* Retry last block written */
#define IFE_FAIL        0x2     /* Fail entire EEPROM image write sequence */

/*
 * Structure for set adapter's characteristics
 */
struct ifchar {
	char	ifc_name[IFNAMSIZ];
	u_int	ifc_treq;
	u_int	ifc_tvx;
	u_int	ifc_lem;
	u_int  ifc_rtoken;
	u_int  ifc_ring_purger;
	u_int  ifc_cnt_interval;	/* Not applicable to DEFZA */
#define FDX_ENB		1
#define FDX_DIS		2
	u_short	ifc_full_duplex_mode;	/* Not applicable to DEFZA */
};

/*
 * interface statistics structures
 */
struct estat {				/* Ethernet interface statistics */
	u_short	est_seconds;		/* seconds since last zeroed */
	u_int	est_bytercvd;		/* bytes received */
	u_int	est_bytesent;		/* bytes sent */
	u_int	est_blokrcvd;		/* data blocks received */
	u_int	est_bloksent;		/* data blocks sent */
	u_int	est_mbytercvd;		/* multicast bytes received */
	u_int	est_mblokrcvd;		/* multicast blocks received */
	u_int	est_deferred;		/* blocks sent, initially deferred */
	u_int	est_single;		/* blocks sent, single collision */
	u_int	est_multiple;		/* blocks sent, multiple collisions */
	u_short	est_sendfail_bm;	/*	0 - Excessive collisions */
					/*	1 - Carrier check failed */
					/*	2 - Short circuit */
					/*	3 - Open circuit */
					/*	4 - Frame too long */
					/*	5 - Remote failure to defer */
	u_short	est_sendfail;		/* send failures: (bit map)*/
	u_short	est_collis;		/* Collision detect check failure */
	u_short	est_recvfail_bm;	/*	0 - Block check error */
					/*	1 - Framing error */
					/*	2 - Frame too long */
	u_short	est_recvfail;		/* receive failure: (bit map) */
	u_short	est_unrecog;		/* unrecognized frame destination */
	u_short	est_overrun;		/* data overrun */
	u_short	est_sysbuf;		/* system buffer unavailable */
	u_short	est_userbuf;		/* user buffer unavailable */
	u_int	est_mbytesent;		/* multicast bytes sent */
	u_int	est_mbloksent;		/* multicast blocks sent */
};

struct dstat {				/* DDCMP pt-to-pt interface statistics */
	u_short	dst_seconds;		/* seconds since last zeroed */
	u_int	dst_bytercvd;		/* bytes received */
	u_int	dst_bytesent;		/* bytes sent */
	u_int	dst_blockrcvd;		/* data blocks received */
	u_int	dst_blocksent;		/* data blocks sent */
	u_short	dst_inbound_bm;		/*	0 - NAKs sent, header crc */
					/*	1 - NAKs sent, data crc */
					/*	2 - NAKs sent, REP response */
	u_char	dst_inbound;		/* data errors inbound: (bit map) */
	u_short	dst_outbound_bm;	/*	0 - NAKs rcvd, header crc */
					/*	1 - NAKs rcvd, data crc */
					/*	2 - NAKs rcvd, REP response */
	u_char	dst_outbound;		/* data errors outbound: (bit map) */
	u_char	dst_remotetmo;		/* remote reply timeouts */
	u_char	dst_localtmo;		/* local reply timeouts */
	u_short	dst_remotebuf_bm;	/*	0 - NAKs rcvd, buffer unavailable */
					/*	1 - NAKs rcvd, buffer too small */
	u_char	dst_remotebuf;		/* remote buffer errors: (bit map) */
	u_short	dst_localbuf_bm;	/*	0 - NAKs sent, buffer unavailable */
					/*	1 - NAKs sent, buffer too small */
	u_char	dst_localbuf;		/* local buffer errors: (bit map) */
	u_char	dst_select;		/* selection intervals elapsed */
	u_short	dst_selecttmo_bm;	/*	0 - No reply to select */
					/*	1 - Incomplete reply to select */
	u_char	dst_selecttmo;		/* selection timeouts: (bit map) */
	u_short	dst_remotesta_bm;	/*	0 - NAKs rcvd, receive overrun */
					/*	1 - NAKs sent, header format */
					/*	2 - Select address errors */
					/*	3 - Streaming tributaries */
	u_char	dst_remotesta;		/* remote station errors: (bit map) */
	u_short	dst_localsta_bm;	/*	0 - NAKs sent, receive overrun */
					/*	1 - Receive overrun, NAK not sent */
					/*	2 - Transmit underruns */
					/*	3 - NAKs rcvd, header format */
	u_char	dst_localsta;		/* local station errors: (bit map) */
};


struct fstat {
        u_short fst_second;             /* seconds since last zeroed */
	u_int	fst_frame;		/* total number of frames seen */ 
	u_int	fst_error;		/* MAC counter frame error */
	u_int 	fst_lost;		/* MAC counter frame count */
        u_int   fst_bytercvd;           /* bytes received */
        u_int   fst_bytesent;           /* bytes sent */
        u_int   fst_pdurcvd;            /* data blocks received */
        u_int   fst_pdusent;            /* data blocks sent */
        u_int   fst_mbytercvd;          /* multicast bytes received */
	u_int   fst_mpdurcvd;           /* multicast blocks received */
        u_int   fst_mbytesent;          /* multicast bytes sent */
        u_int   fst_mpdusent;           /* multicast blocks sent */
	u_short	fst_underrun;		/* transmit underrun error */
        u_short fst_sendfail;           /* sent failures: (bit map)*/
        u_short	fst_fcserror;		/* FCS check failure */
	u_short	fst_fseerror;		/* frame status error */
	u_short	fst_pdualig;		/* frame alignment error */
	u_short	fst_pdulen;		/* frame length error */
	u_short	fst_pduunrecog;		/* frame unrecognized */
	u_short fst_mpduunrecog;	/* multicast frame unrecognized */
        u_short fst_overrun;            /* data overrun */
        u_short fst_sysbuf;             /* system buffer unavailable */
        u_short fst_userbuf;            /* user buffer unavailable */
	u_short fst_ringinit;		/* other station ring reinit. intiated*/
	u_short fst_ringinitrcv;	/* ring reinitialization initiated */
	u_short fst_ringbeacon;		/* ring beacon process initiated */
	u_short fst_ringbeaconrecv;	/* ring beacon process recv */
	u_short fst_duptoken;		/* duplicat token detected */
	u_short fst_dupaddfail;		/* duplicate address test failures */
	u_short	fst_ringpurge;		/* ring purge errors */
	u_short fst_bridgestrip;	/* bridge strip errors */
	u_short fst_traceinit;		/* traces initiated */
	u_short fst_tracerecv;		/* traces received */
	u_short fst_lem_rej;		/* LEM reject count */
	u_short fst_lem_events;		/* LEM events count */
	u_short fst_lct_rej;		/* LCT reject count */
	u_short fst_tne_exp_rej;	/* TNE expired reject count */
	u_short fst_connection;		/* Completed Connection count */
	u_short fst_ebf_error;		/* Elasticity Buffer Errors */
};


/* 
 * FDDI MIB structures 
 */

/* SMT group */

struct fddismt {
	short   smt_number;             /* SMT number */
	short	smt_index;		/* SMT FDDI SMT index */	
	u_char	smt_stationid[8];	/* FDDI station id */ 
	u_short	smt_opversionid;	/* operation version id */
	u_short	smt_hiversionid;	/* highest version id */
	u_short	smt_loversionid;	/* lowest version id */
	short 	smt_macct;		/* number of MACs */
	short	smt_nonmasterct;	/* number of non master port */
	short	smt_masterct;		/* number of master port */
	short   smt_pathsavail;		/* available path type */
	short	smt_configcap;		/* configuration capabilities */
	short	smt_configpolicy;	/* configuration policy */
	u_short	smt_connectpolicy;	/* connection policy */
	u_short	smt_timenotify;		/* neighbor notification prot. time*/
	short	smt_statusreport;	/* status reporting protocol */
	short	smt_ecmstate;		/* state of ECM state machine */
	short	smt_cfstate;		/* CF_state */
	short	smt_holdstate;		/* current state of Hold function */
	short	smt_remotedisconn;	/* remotely disconnection flag */ 
	u_char	smt_msgtimestamp[8];	/* timestamp for SMT frames */
	u_char	smt_trantimestamp[8];	/* timestamp for last event */
	u_char	smt_setinfo[12];	/* station last parameter change */
	u_char	smt_lastsetid[8];	/* Last station ID change */
	short	smt_action;		/* SMT station action */
}; 	
		
struct fstatus {
	short	led_state;		/* LED State */
	short   rmt_state;		/* RMT state */
	short   link_state;		/* LINK state */
	short   dup_add_test;		/* duplicate address test */
	short   ring_purge_state;	/* ring purge state */
	u_int  neg_trt;		/* Negotiated TRT */
	u_char  upstream[6];		/* Upstream Neighbor */
	u_char  downstream[6];		/* downstream Neighbor */
	u_char	una_timed_out;		/* una timed out flag */
	u_char 	frame_strip_mode;	/* frame strip mode */
	u_char	claim_token_mode;	/* claim token yield mode */
	u_char 	phy_state;		/* Physical state */
	u_char	neighbor_phy_type;	/* Neighborshort */		   
	u_char  rej_reason;		/* reject reason */
	u_int	phy_link_error;		/* phy link error estimate */
	u_char	ring_error;		/* ring error reasons */
	u_int	t_req;			/* TRT request */
	u_int  tvx;			/* TVX value */
	u_int  t_max;			/* TRT maximum */
        u_int  lem_threshold;          /* lem threshold */
	u_int	rtoken_timeout;		/* restricted token timeout */
	u_char  mla[6];			/* station physical address */
	u_char  fw_rev[4];		/* firmware revision */
	u_char  phy_rev[4];		/* ROM revision */	
	u_char	pmd_type;		/* PMD type */
	u_char  dir_beacon[6];		/* Last Direct Beacon address */
	short	smt_version;		/* SMT version */
	short   state;			/* Adapter State */
	u_char	dir_beacon_una[6];	/* UNA from last direct beacon addr. */
	short	cnt_interval;		/* Interval to get counter updates */
	short	full_duplex_mode;	/* enabling/disabling duplex mode */
	short	full_duplex_state;	/* state, if enabled */
};	

	
/* MAC group */

struct fddimac {
	short   mac_number;             /* MAC number */
	short   mac_smt_index;		/* MAC SMT index */
	short   mac_index;		/* MAC index */
	short	mac_fsc;		/* MAC frame status capabilities */
	u_int	mac_gltmax;		/* Greastest lower bound of T_max */
	u_int	mac_gltvx;		/* Greastest lower bound of TVX */
	short	mac_paths;		/* path types available */
	short	mac_current;		/* association of the MAC with path*/
	u_char	mac_upstream[6];	/* upstream neighbor */
	u_char	mac_oldupstream[6];	/* previous upstream neighbor */
	short	mac_dupaddrtest;	/* duplicate address test */
	short   mac_pathsreq;		/* paths requested */
	short	mac_downstreamtype;	/* downstream PC-type */
	u_char	mac_smtaddress[6];	/* MAC address for SMT */
	u_int	mac_treq;		/* TRT time */
	u_int	mac_tneg;		/* Neg. TRT time */
	u_int	mac_tmax;		/* max. TRT time */
	u_int	mac_tvx;		/* TVX value */
	u_int	mac_tmin;		/* min. TRT time */
	short	mac_framestatus;	/* current frame status */
	int	mac_counter;		/* frame counters */
	int	mac_error;		/* frame error counters */
	int	mac_lost;		/* frame lost counter */
	short	mac_rmtstate;		/* Ring Management state */
	short	mac_dupaddr;		/* duplicate address flag */
	short	mac_condition;		/* MAC condition */
	short   mac_action;		/* MAC action */
	short	mac_updupaddr;		/* upstream duplicate address flag */
	short	mac_frame_error_thresh;	/* frame error threshold */
	short 	mac_frame_error_ratio;	/* frame error ratio */
	short	mac_chip_set;		/* the chip set used */
};

/* PATH group */

struct fddipath {
	short	path_configindex;	/* path configuration index */
	short	path_type;		
	short	path_portorder;			
	u_int	path_sba;		/*synchronous bandwidth allocation*/ 
	short	path_sbaoverhead;	/* SBA overhead */
	short	path_status;		/* path status */
};

/* PORT group */

struct fddiport {
	short   port_number;            /* port number */
	short	port_smt_index;		/* port SMT index */
	short	port_index;		/* port index */
	short	port_pctype;		/* value of the port's PC_type */
	short	port_pcneighbor;	/* PC_neighbor of remote port*/
	short   port_connpolicy;	/* connection policies */
	short	port_remoteind;		/* remote MAC indicated */
	short 	port_CEstate;		/* Current Configuration state */
	short   port_pathreq;		/* path request */
	u_short port_placement;		/* upstream MAC */
	short	port_availpaths;	/* available paths */
	u_int	port_looptime;		/* time for MAC loval loop */
	u_int	port_TBmax;		/* TB_max */
	short	port_BSflag;		/* the Break state, BF_flag */
	u_int	port_LCTfail;		/* counter for Link confidence test */
	short	port_LerrEst;		/* Link error estimate */
	u_int	port_Lemreject;		/* Link reject count */
	u_int	port_Lem;		/* Link error monitor count */
	short	port_baseLerEst;	/* value of port Ler Estimate */
	u_int	port_baseLerrej;	/* Ler reject count */
	u_int	port_baseLer;		/* Ler count */
	u_char	port_baseLerTime[8];	/* Ler timestamp */	
	short   port_Lercutoff;		/* error rate cut off limit */
	short	port_alarm;		/* error rate cause alarm generate*/
	short	port_connectstate;	/* connect state */
	short   port_PCMstate;		/* PCM state */
	short	port_PCwithhold;	/* PC_withhold */
	short	port_Lercondition;	/* true if Ler-Est <= Ler_alarm */
	short   port_action;		/* PORT action */
	short	port_chip_set;		/* PORT chip set */
};

/* Attachment group */

struct	fddiatta {
	short	atta_number;            /* attachment number */
	short 	atta_smt_index;		/* attachment SMT index */
	short 	atta_index;		/* attachment index */
	short	atta_class;		/* attachment class */
	short	atta_bypass;		/* attachment optical bypass */	
	short	atta_IMaxExpiration;	/* attachment I_Max Expiration */	
	short	atta_InsertedStatus;	/* Inserted status */	
	short	atta_InsertPolicy;	/* Insert policy */	
};

		/* Information here is conformant to SMT 7.2 */

    /* SMT GROUP */
struct smtmib_smt {

	u_char      smt_station_id[8];
	u_int       smt_op_version_id;
	u_int       smt_hi_version_id;
	u_int       smt_lo_version_id;
	u_char      smt_user_data[32];
	u_int       smt_mib_version_id;
	u_int       smt_mac_ct;
	u_int       smt_non_master_ct;
	u_int       smt_master_ct;
	u_int       smt_available_paths;
	u_int       smt_config_capabilities;
	u_int       smt_config_policy;
	u_int       smt_connection_policy;
	u_int       smt_t_notify;
	u_int       smt_stat_rpt_policy;
	u_int       smt_trace_max_expiration;
	u_int       smt_bypass_present;
	u_int       smt_ecm_state;
	u_int       smt_cf_state;
	u_int       smt_remote_disconnect_flag;
	u_int	    smt_station_status;
	u_int       smt_peer_wrap_flag;
#ifdef __alpha
	u_long      smt_msg_time_stamp;	/* 64 bit counter */
	u_long      smt_transition_time_stamp; /* 64 bit counter */
#else
	u_int       smt_msg_time_stamp_ms;	/* 64 bit counter */
	u_int       smt_msg_time_stamp_ls;
	u_int       smt_transition_time_stamp_ms; /* 64 bit counter */
	u_int       smt_transition_time_stamp_ls;
#endif /* __alpha */
};

	/* MAC GROUP */
struct smtmib_mac {
	u_int       mac_frame_status_functions;
	u_int       mac_t_max_capability;
	u_int       mac_tvx_capability;
	u_int       mac_available_paths;
	u_int       mac_current_path;
	u_char      mac_upstream_nbr[8];
	u_char      mac_downstream_nbr[8];
	u_char      mac_old_upstream_nbr[8];
	u_char      mac_old_downstream_nbr[8];
	u_int       mac_dup_address_test;
        u_int       mac_requested_paths;
      	u_int       mac_downstream_port_type;
	u_char      mac_smt_address[8];
    	u_int       mac_t_req;
	u_int       mac_t_neg;
	u_int       mac_t_max;
	u_int       mac_tvx_value;
	u_int       mac_frame_error_threshold;
	u_int       mac_frame_error_ratio;
	u_int       mac_rmt_state;
	u_int       mac_da_flag;
	u_int       mac_unda_flag;
	u_int       mac_frame_error_flag;
	u_int       mac_ma_unitdata_available;
	u_int       mac_hw_present;
	u_int       mac_ma_unitdata_enable;
};

	/* PATH GROUP */

struct smtmib_path {
	u_char      path_configuration[32];
	u_int       path_tvx_lower_bound;
	u_int       path_t_max_lower_bound;
	u_int       path_max_t_req;
};

	/* PORT GROUP */
struct smtmib_port {
	u_int       port_my_type[2];
	u_int       port_neighbor_type[2];
	u_int       port_connection_policies[2];
	u_int       port_mac_indicated[2];
	u_int       port_current_path[2];
	u_int       port_requested_paths[2];
	u_int       port_mac_placement[2];
	u_int       port_available_paths[2];
	u_int       port_pmd_class[2];
	u_int       port_connection_capabilities[2];
	u_int       port_bs_flag[2];
	u_int       port_ler_estimate[2];
	u_int       port_ler_cutoff[2];
	u_int       port_ler_alarm[2];
	u_int       port_connect_state[2];
	u_int       port_pcm_state[2];
	u_int       port_pc_withhold[2];
	u_int       port_ler_flag[2];
	u_int       port_hardware_present[2];
};

struct decext_mib {
	/* SMT GROUP */
	u_int 	esmt_station_type;

	/* MAC GROUP */
	u_int	emac_link_state;
	u_int	emac_ring_purger_state;
	u_int	emac_ring_purger_enable;
	u_int	emac_frame_strip_mode;
	u_int	emac_ring_error_reason;
	u_int	emac_up_nbr_dup_addr_flag;
	u_int	emac_restricted_token_timeout;

	/* PORT GROUP */
	u_int	eport_pmd_type[2];
	u_int	eport_phy_state[2];
	u_int	eport_reject_reason[2];

	/* FDX (Full-Duplex) GROUP */
	u_int	efdx_enable;        /* Valid only in SMT 7.2 */
	u_int	efdx_op;            /* Valid only in SMT 7.2 */
	u_int	efdx_state;         /* Valid only in SMT 7.2 */
};




#define CTR_ETHER 0			/* Ethernet interface */
#define CTR_DDCMP 1			/* DDCMP pt-to-pt interface */
#define CTR_FDDI 2			/* FDDI interface */
#define FDDIMIB_SMT	3		/* FDDI MIB SMT group */
#define FDDIMIB_MAC	4		/* FDDI MIB MAC group */
#define FDDIMIB_PATH	5		/* FDDI MIB PATH group */
#define FDDIMIB_PORT	6		/* FDDI MIB PORT group */
#define FDDIMIB_ATTA	7		/* FDDI MIB Attatchment Group */
#define FDDI_STATUS	8		/* FDDI status */
#define FDDISMT_MIB_SMT	9		/* FDDI SMT SMT MIB values */
#define FDDISMT_MIB_MAC	10		/* FDDI SMT MAC values */
#define FDDISMT_MIB_PATH	11	/* FDDI SMT PATH values */
#define FDDISMT_MIB_PORT	12	/* FDDI SMT PORT values */
#define FDDIDECEXT_MIB		13	/* FDDI DEC Extended MIB values */
#define CTR_TRN         	14	/* Token ring counters */
#define TRN_CHAR		15	/* Token ring characteristics */
#define TRN_MIB_ENTRY   	16	/* Token ring mib entry */
#define TRN_MIB_STAT_ENTRY  	17	/* Token ring mib status entry */
#define TRN_MIB_TIMER_ENTRY 	18	/* Token ring mib timer entry */

#define ctr_ether ctr_ctrs.ctrc_ether
#define ctr_ddcmp ctr_ctrs.ctrc_ddcmp
#define ctr_fddi  ctr_ctrs.ctrc_fddi
#define sts_fddi  ctr_ctrs.status_fddi
#define fmib_smt  ctr_ctrs.smt_fddi
#define fmib_mac  ctr_ctrs.mac_fddi
#define fmib_path  ctr_ctrs.path_fddi
#define fmib_port  ctr_ctrs.port_fddi
#define fmib_atta  ctr_ctrs.atta_fddi
#define smib_smt  ctr_ctrs.smt_smt
#define smib_mac  ctr_ctrs.smt_mac
#define smib_port  ctr_ctrs.smt_port
#define smib_path  ctr_ctrs.smt_path
#define decmib_ext ctr_ctrs.dec_ext

#define CTR_HDRCRC	0		/* header crc bit index */
#define CTR_DATCRC	1		/* data crc bit index */
#define CTR_BUFUNAVAIL	0		/* buffer unavailable bit index */

/*
 * Interface counter ioctl request
 */
struct ctrreq {
	char    ctr_name[IFNAMSIZ];     /* if name */
	        char    ctr_type;               /* type of interface */
        union {
                struct estat ctrc_ether;/* ethernet counters */
                struct dstat ctrc_ddcmp;/* DDCMP pt-to-pt counters */
                struct fstat ctrc_fddi; /* FDDI counters */
                struct fstatus status_fddi; /* FDDI stsatus */
                struct fddismt smt_fddi;/* fddi SMT attributes */
                struct fddimac mac_fddi;/* fddi MAC attributes */
                struct fddipath path_fddi; /* fddi PATH attributes */
                struct fddiport port_fddi; /* fddi PORT attributes */
                struct fddiatta atta_fddi; /* fddi attatch attributes */
		struct smtmib_smt	smt_smt;   /* smt mib attributes */
		struct smtmib_mac	smt_mac;   /* smt mib attributes */
		struct smtmib_path	smt_path;  /* smt mib attributes */
		struct smtmib_port	smt_port;  /* smt mib attributes */
		struct decext_mib	dec_ext;   /* dec extended mib */
		struct trnchar  trnchar;/* Token ring characteristics */
                struct trncount trncount;/* Token ring counters       */
                struct dot5Entry dot5Entry;/* Token ring MIB counters */
                struct dot5StatsEntry dot5StatsEntry;/*Token ring MIB stats */
                struct dot5TimerEntry dot5TimerEntry;/*Token ring MIB Timers */
        } ctr_ctrs;
};

union mac_addr {
    u_short saddr[3];
    u_char  caddr[6];
};

#ifdef	_KERNEL
#include "net/if_arp.h"
extern struct	ifnet *ifnet;
#else
#include <net/if_arp.h>
#endif

#endif
