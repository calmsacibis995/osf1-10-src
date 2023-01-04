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
 *	@(#)$RCSfile: mbuf.h,v $ $Revision: 4.3.17.8 $ (DEC) $Date: 1993/12/15 22:11:58 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
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
 * Copyright (c) 1982, 1986, 1988 Regents of the University of California.
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
 *	Base:	mbuf.h	7.12 (Berkeley) 9/4/89
 *	Merged: mbuf.h	7.13 (Berkeley) 6/28/90
 *
 */

#ifndef	_SYS_MBUF_H_
#define	_SYS_MBUF_H_

#include <sys/param.h>

#if	defined(_KERNEL) && !defined(_NET_GLOBALS_H_)
#include "net/net_globals.h"
#endif

#if	defined(_KERNEL) && !defined(M_WAITOK)
#include "net/net_malloc.h"
#endif

#if	!defined(MCLBYTES)
/* Compat for now */
#ifdef __alpha
#define	MSIZE		256	/* size of an mbuf */
#else
#define	MSIZE		128	/* size of an mbuf */
#endif /* __alpha */
#define MCLBYTES	CLBYTES
#define MCL2KBYTES	2048
#define MAPPED_MBUFS	/* safe */
#endif
#if	defined(_KERNEL) && !defined(NMBCLUSTERS) && !defined(__alpha)
#include <mbclusters.h>
#endif

/*
 * Mbufs are of a single size, MSIZE, which includes overhead.  
 * An mbuf may add a single "mbuf cluster" of size MCLBYTES (from CLBYTES 
 * in sys/param.h), which has no additional overhead and is used instead 
 * of the internal data area; this is usually done when
 * at least MINCLSIZE of data must be stored.
 */

#define	MLEN		(MSIZE - sizeof(struct m_hdr))	/* normal data len */
#define	MHLEN		(MLEN - sizeof(struct pkthdr))	/* data len w/pkthdr */

#define	MINCLSIZE	(MHLEN + MLEN)	/* smallest amount to put in cluster */
#define	M_MAXCOMPRESS	(MHLEN / 2)	/* max amount to copy for compression */
#define MLENGTH(m)	((m)->m_flags & M_PKTHDR ? (m)->m_pkthdr.len : m_length(m))

/* forward declarations as required for C++ */
#ifdef __cplusplus
struct mbuf;
struct m_hdr;
#endif

/*
 * Macros for type conversion
 * mtod(m,t) -	convert mbuf pointer to data pointer of correct type
 * dtom(x) -	convert data pointer within mbuf to mbuf pointer (XXX)
 */
#define mtod(m,t)	((t)((m)->m_data))
#define	dtom(x)		((struct mbuf *)((u_long)(x) & ~(MSIZE-1)))

/* header at beginning of each mbuf: */
struct m_hdr {
	struct	mbuf *mh_next;		/* next buffer in chain */
	struct	mbuf *mh_nextpkt;	/* next chain in queue/record */
	caddr_t	mh_data;		/* location of data */
	int	mh_len;			/* amount of data in this mbuf */
	int	mh_type;		/* type of data in this mbuf */
	int	mh_flags;		/* flags; see below */
#ifdef __alpha
	union {
		caddr_t	*mun1_ipq[4];	/* ipasfrag chain pointers */
		caddr_t	*mun1_ihp[4];	/* protocol sequence chain */
	} mh_un1;
#define	m_ipq		m_hdr.mh_un1.mun1_ipq
#define	m_ihp		m_hdr.mh_un1.mun1_ihp
#endif
};

/* record/packet header in first mbuf of chain; valid if M_PKTHDR set */
struct	pkthdr {
	int	len;		/* total packet length */
	struct	ifnet *rcvif;	/* rcv interface */
};

/* description of external storage mapped into mbuf, valid if M_EXT set */
struct m_ext {
	caddr_t	ext_buf;		/* start of buffer */
	void	(*ext_free)();		/* free routine if not the usual */
	u_int	ext_size;		/* size of buffer, for ext_free */
	caddr_t	ext_arg;		/* additional ext_free argument */
	struct	ext_refq {		/* reference list */
		struct ext_refq *forw, *back;
	} ext_ref;
};

struct mbuf {
	struct	m_hdr m_hdr;
	union {
		struct {
			struct	pkthdr MH_pkthdr;	/* M_PKTHDR set */
			union {
				struct	m_ext MH_ext;	/* M_EXT set */
				char	MH_databuf[MHLEN];
			} MH_dat;
		} MH;
		char	M_databuf[MLEN];		/* !M_PKTHDR, !M_EXT */
	} M_dat;
};

#define	m_next		m_hdr.mh_next
#define	m_len		m_hdr.mh_len
#define	m_data		m_hdr.mh_data
#define	m_type		m_hdr.mh_type
#define	m_flags		m_hdr.mh_flags
#define	m_nextpkt	m_hdr.mh_nextpkt
#define	m_act		m_nextpkt
#define	m_pkthdr	M_dat.MH.MH_pkthdr
#define	m_ext		M_dat.MH.MH_dat.MH_ext
#define	m_pktdat	M_dat.MH.MH_dat.MH_databuf
#define	m_dat		M_dat.M_databuf

/* mbuf flags */
#define	M_EXT		0x0001	/* has associated external storage */
#define	M_PKTHDR	0x0002	/* start of record */
#define	M_EOR		0x0004	/* end of record */
#define	M_FASTFREE	0x0008	/* free external storage asap */

/* mbuf pkthdr flags, also in m_flags */
#define	M_BCAST		0x0100	/* send/received as link-level broadcast */
#define	M_MCAST		0x0200	/* send/received as link-level multicast */
#define	M_WCARD		0x0400	/* received as network-level broadcast */

/* does mbuf hold a broadcast packet? */
#define m_broadcast(m)	((m)->m_flags & (M_BCAST|M_MCAST|M_WCARD))

/* flags copied when copying m_pkthdr */
#define	M_COPYFLAGS	(M_PKTHDR|M_EOR|M_BCAST|M_MCAST|M_WCARD)

/* mbuf types */
#define	MT_FREE		0	/* should be on free list */
#define	MT_DATA		1	/* dynamic (data) allocation */
#define	MT_HEADER	2	/* packet header */
#define	MT_SOCKET	3	/* socket structure */
#define	MT_PCB		4	/* protocol control block */
#define	MT_RTABLE	5	/* routing tables */
#define	MT_HTABLE	6	/* IMP host tables */
#define	MT_ATABLE	7	/* address resolution tables */
#define	MT_SONAME	8	/* socket name */
#define	MT_SOOPTS	10	/* socket options */
#define	MT_FTABLE	11	/* fragment reassembly header */
#define	MT_RIGHTS	12	/* access rights */
#define	MT_IFADDR	13	/* interface address */
#define MT_CONTROL	14	/* extra-data protocol message */
#define MT_OOBDATA	15	/* expedited data  */
#define MT_MAX		16	/* e.g. */

/* flags to m_get/MGET */
#define	M_DONTWAIT	M_NOWAIT
#define	M_WAIT		M_WAITOK

/*
 * mbuf allocation/deallocation macros:
 *
 *	MGET(struct mbuf *m, int how, int type)
 * allocates an mbuf and initializes it to contain internal data.
 *
 *	MGETHDR(struct mbuf *m, int how, int type)
 * allocates an mbuf and initializes it to contain a packet header
 * and internal data.
 */

#define	MGET(m, how, type) { \
	MALLOC((m), struct mbuf *, MSIZE, M_MBUF, (how)); \
	if (m) { \
		MBSTAT((mbstat.m_mtypes[type]++, mbstat.m_mbufs++)); \
		(m)->m_next = (m)->m_nextpkt = 0; \
		(m)->m_type = (type); \
		(m)->m_data = (m)->m_dat; \
		(m)->m_flags = 0; \
	} else \
		(m) = m_retry((how), (type)); \
}

#define	MGETHDR(m, how, type) { \
	MALLOC((m), struct mbuf *, MSIZE, M_MBUF, (how)); \
	if (m) { \
		MBSTAT((mbstat.m_mtypes[type]++, mbstat.m_mbufs++)); \
		(m)->m_next = (m)->m_nextpkt = 0; \
		(m)->m_type = (type); \
		(m)->m_data = (m)->m_pktdat; \
		(m)->m_flags = M_PKTHDR; \
	} else \
		(m) = m_retryhdr((how), (type)); \
}

/*
 * Mbuf cluster macros.
 * MCLALLOC(caddr_t p, int how) allocates an mbuf cluster.
 * MCLGET adds such clusters to a normal mbuf;
 * the flag M_EXT is set upon success.
 * MCLFREE unconditionally frees a cluster allocated by MCLALLOC.
 */

#define	MCLALLOC(p, how) \
	  p = m_clalloc(1, (how));

#define MCLFREE(p) { \
	MBSTAT(--mbstat.m_clusters); \
	FREE((p), M_CLUSTER); \
}

#define	MCLGET(m, how) { \
	(void) m_clalloc8((m), (how)); \
}

#define	MCLGET2(m, how) { \
	(void) m_clalloc2((m), (how)); \
}

#define MCLREFERENCED(m) \
	((m)->m_ext.ext_ref.forw != &((m)->m_ext.ext_ref))

/*
 * MFREE(struct mbuf *m, struct mbuf *n)
 * Free a single mbuf and associated external storage.
 * Place the successor, if any, in n.
 */
#define	MFREE(m, n)	(n) = m_free(m)

/*
 * Copy mbuf pkthdr from from to to.
 * from must have M_PKTHDR set, and to must be empty.
 */
#define	M_COPY_PKTHDR(to, from) { \
	(to)->m_pkthdr = (from)->m_pkthdr; \
	(to)->m_flags = (from)->m_flags & M_COPYFLAGS; \
	(to)->m_data = (to)->m_pktdat; \
}

/*
 * Set the m_data pointer of a newly-allocated mbuf (m_get/MGET) to place
 * an object of the specified size at the end of the mbuf, longword aligned.
 */
#define	M_ALIGN(m, len) \
	{ (m)->m_data += (MLEN - (len)) &~ (sizeof(long) - 1); }
/*
 * As above, for mbufs allocated with m_gethdr/MGETHDR
 * or initialized by M_COPY_PKTHDR.
 */
#define	MH_ALIGN(m, len) \
	{ (m)->m_data += (MHLEN - (len)) &~ (sizeof(long) - 1); }

/*
 * Compute the amount of space available
 * before the current start of data in an mbuf.
 * Subroutine - data not available if certain references.
 */
#define	M_LEADINGSPACE(m)	m_leadingspace(m)

/*
 * Compute the amount of space available
 * after the end of data in an mbuf.
 * Subroutine - data not available if certain references.
 */
#define	M_TRAILINGSPACE(m)	m_trailingspace(m)

/*
 * Arrange to prepend space of size plen to mbuf m.
 * If a new mbuf must be allocated, how specifies whether to wait.
 * If how is M_DONTWAIT and allocation fails, the original mbuf chain
 * is freed and m is set to NULL.
 */
#define	M_PREPEND(m, plen, how) { \
	if (M_LEADINGSPACE(m) >= (plen)) { \
		(m)->m_data -= (plen); \
		(m)->m_len += (plen); \
	} else \
		(m) = m_prepend((m), (plen), (how)); \
	if ((m) && (m)->m_flags & M_PKTHDR) \
		(m)->m_pkthdr.len += (plen); \
}

/* change mbuf to new type */
#define MCHTYPE(m, t) { \
	MBSTAT((mbstat.m_mtypes[(m)->m_type]--, mbstat.m_mtypes[t]++)); \
	(m)->m_type = t;\
}

/* length to m_copy to copy all */
#define	M_COPYALL	1000000000

/* compatiblity with 4.3 */
#define  m_copy(m, o, l)	m_copym((m), (o), (l), M_DONTWAIT)

/*
 * Mbuf statistics.
 */
struct mbstat {
	u_long	m_mbufs;	/* mbufs obtained from page pool */
	u_long	m_clusters;	/* clusters obtained from page pool */
	u_long	m_mfree;	/* free mbufs - UNUSED */
	u_long	m_clfree;	/* free clusters - UNUSED */
	u_long	m_drops;	/* times failed to find space */
	u_long	m_wait;		/* times waited for space - UNUSED */
	u_long	m_drain;	/* times drained protocols for space */
	u_int	m_mtypes[MT_MAX];	/* type specific mbuf allocations */
};

#ifdef	_KERNEL
#if	NETSYNC_LOCK
extern	simple_lock_data_t mbuf_slock;
#define MBUF_LOCKINIT()	simple_lock_init(&mbuf_slock)
#define MBUF_LOCK()	simple_lock(&mbuf_slock)
#define MBUF_UNLOCK()	simple_unlock(&mbuf_slock)
#define MBSTAT(x)	{int _ns = splimp(); MBUF_LOCK(); \
			  x; MBUF_UNLOCK(); splx(_ns); }
#else
#define MBUF_LOCKINIT()
#define MBUF_LOCK()
#define MBUF_UNLOCK()
#define MBSTAT(x)	x
#endif
extern struct	mbstat mbstat;		/* statistics */
extern int	max_linkhdr;		/* largest link-level header */
extern int	max_protohdr;		/* largest protocol header */
extern int	max_hdr;		/* largest link+protocol header */
extern int	max_datalen;		/* MHLEN - max_hdr */
#endif
#endif
