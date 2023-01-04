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
static char *rcsid = "@(#)$RCSfile: uipc_mbuf.c,v $ $Revision: 4.3.13.6 $ (DEC) $Date: 1993/10/13 20:03:31 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
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
 *	Base:	uipc_mbuf.c	7.12 (Berkeley) 9/26/89
 *	Merged: uipc_mbuf.c	7.16 (Berkeley) 6/28/90
 */

/*
 * uipc_mbuf.c
 *
 *      Modification History:
 *
 * 04-Nov-91	Fred Canter
 *	Make nmbclusters an extern (value set in param.c).
 *
 * 30-May-91	Ajay Kachrani
 *	OSF1.0.1 supplied fixes to mbufs performence and correct ptr bug in m_adj
 *
 * 14-Jan-91   David Metsky
 *      osf supplied fix to interrupt mask restore
 *
 */


#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/time.h"
#include "sys/kernel.h"

#include "sys/mbuf.h"
#include "sys/protosw.h"

#include "net/net_malloc.h"
#include "net/netisr.h"
#include <sys/stream.h>
#include "streams.h"

struct	mbuf *mfreelater;	/* mbuf deallocation list */
int	max_linkhdr;		/* largest link-level header */
int	max_protohdr;		/* largest protocol header */
int	max_hdr;		/* largest link+protocol header */
int	max_datalen;		/* MHLEN - max_hdr */
struct	mbstat mbstat;		/* statistics */
#ifdef  __alpha
int	mclbytes = 8192;	/* possibly interesting value */
#else
int	mclbytes = MCLBYTES;	/* possibly interesting value */
#endif
#if	NETSYNC_LOCK
simple_lock_data_t	mbuf_slock;
LOCK_ASSERTL_DECL
#endif

void
mbinit()
{
	int s, m;

	MBUF_LOCKINIT();
	/* Initialize isr stuff - here because of netisr_add and
	 * because some systems need mbufs before netinit. */
	NETISR_LOCKINIT();
	bzero((caddr_t)softnet_intr, sizeof softnet_intr);

	netisr_add(NETISR_MB, mbufintr,
			(struct ifqueue *)NULL, (struct domain *)NULL);
}

/* ARGSUSED */
caddr_t
m_clalloc(ncl, canwait)
	register int ncl;
	int canwait;
{
	register caddr_t mcl;

	MALLOC(mcl, caddr_t, ncl * MCLBYTES, M_CLUSTER, canwait);
	if (mcl == 0 && canwait != M_DONTWAIT) {
		pfreclaim();
		MBSTAT(mbstat.m_drain++); 
		MALLOC(mcl, caddr_t, ncl * MCLBYTES, M_CLUSTER, M_WAITOK);
	}
	MBSTAT(mcl ? mbstat.m_clusters++ : mbstat.m_drops++);
	return mcl;
}

caddr_t
m_clalloc2(m, canwait)
	register struct mbuf *m;
	int canwait;
{
	register caddr_t mcl;

	MALLOC(mcl, caddr_t, MCL2KBYTES, M_CLUSTER, canwait);
	if (mcl == 0 && canwait != M_DONTWAIT) {
		pfreclaim();
		MBSTAT(mbstat.m_drain++); 
		MALLOC(mcl, caddr_t, MCL2KBYTES, M_CLUSTER, M_WAITOK);
	}
	if (m && mcl) {
		m->m_data = m->m_ext.ext_buf = mcl;
		m->m_flags |= M_EXT;
		m->m_ext.ext_size = MCL2KBYTES;
		m->m_ext.ext_free = 0;
		m->m_ext.ext_arg  = 0;
		m->m_ext.ext_ref.forw = m->m_ext.ext_ref.back =
			&m->m_ext.ext_ref;
	}
	MBSTAT(mcl ? mbstat.m_clusters++ : mbstat.m_drops++);
	return mcl;
}

caddr_t
m_clalloc8(m, canwait)
	register struct mbuf *m;
	int canwait;
{
	register caddr_t mcl;

	MALLOC(mcl, caddr_t, MCLBYTES, M_CLUSTER, canwait);
	if (mcl == 0 && canwait != M_DONTWAIT) {
		pfreclaim();
		MBSTAT(mbstat.m_drain++); 
		MALLOC(mcl, caddr_t, MCLBYTES, M_CLUSTER, M_WAITOK);
	}
	if (m && mcl) {
		m->m_data = m->m_ext.ext_buf = mcl;
		m->m_flags |= M_EXT;
		m->m_ext.ext_size = MCLBYTES;
		m->m_ext.ext_free = 0;
		m->m_ext.ext_arg  = 0;
		m->m_ext.ext_ref.forw = m->m_ext.ext_ref.back =
			&m->m_ext.ext_ref;
	}
		
	MBSTAT(mcl ? mbstat.m_clusters++ : mbstat.m_drops++);
	return mcl;
}

/*
 * The mbuf "isr". Check cluster allocation and free any deferred
 * m_ext mbufs. Both done here to avoid calls from interrupt level.
 */
void
mbufintr()
{
	register struct mbuf *m;
	int s = splimp();

	MBUF_LOCK();
	m = mfreelater;
	mfreelater = NULL;
	MBUF_UNLOCK();
	splx(s);
	while (m) {
		if (m->m_flags & M_EXT) {
			if (MCLREFERENCED(m) || !m->m_ext.ext_free)
				panic("mfreelater");
			(*(m->m_ext.ext_free))(m->m_ext.ext_buf,
			    m->m_ext.ext_size, m->m_ext.ext_arg);
			m->m_flags &= ~M_EXT;
		}
		m = m_free(m);
	}
#if	STREAMS
	{
	extern void ext_freeb(void);
	ext_freeb();
	}
#endif
}

/*
 * When MGET fails, ask protocols to free space when short of memory,
 * then re-attempt to allocate an mbuf.
 */

struct mbuf *
m_retry(canwait, type)
	int canwait, type;
{
	register struct mbuf *m = 0;

#define	m_retry(m, t)	0
	if (canwait != M_DONTWAIT) {
		pfreclaim();
		MBSTAT(mbstat.m_drain++);
		MGET(m, canwait, type);
	}
	if (m == 0)
		MBSTAT(mbstat.m_drops++);
	return m;
#undef	m_retry
}

/*
 * As above; retry an MGETHDR.
 */
struct mbuf *
m_retryhdr(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	if (m = m_retry(canwait, type)) {
		m->m_flags |= M_PKTHDR;
		m->m_data = m->m_pktdat;
	}
	return (m);
}

/*
 * Space allocation routines.
 * These are also available as macros
 * for critical paths.
 */
struct mbuf *
m_get(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	MGET(m, canwait, type);
	return (m);
}

struct mbuf *
m_gethdr(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	MGETHDR(m, canwait, type);
	return (m);
}

struct mbuf *
m_getclr(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	MGET(m, canwait, type);
	if (m)
		bzero(mtod(m, caddr_t), MLEN);
	return (m);
}

/*
 * Free an allocated mbuf, freeing associated cluster if present.
 * If cluster requires special action, place whole mbuf on mfreelater
 * and schedule later freeing (so as not to free from interrupt level).
 */
struct mbuf *
m_free(m)
	struct mbuf *m;
{
	struct mbuf *n = m->m_next;
	int s;

	if (m->m_type == MT_FREE)
		panic("freeing free mbuf");
	s = splimp();
	MBUF_LOCK();
	if (m->m_flags & M_EXT) {
		if (MCLREFERENCED(m)) {		/* Unlink with lock held */
			remque(&m->m_ext.ext_ref);
		} else if (m->m_ext.ext_free == NULL) {
			FREE(m->m_ext.ext_buf, M_CLUSTER);
			--mbstat.m_clusters;
		} else {
			if (m->m_flags & M_FASTFREE) {
				(*(m->m_ext.ext_free))(m->m_ext.ext_buf,
						       m->m_ext.ext_size,
						       m->m_ext.ext_arg);
				m->m_flags &= ~M_EXT;
			} else {
				m->m_next = mfreelater;
				mfreelater = m;
				MBUF_UNLOCK();
				schednetisr(NETISR_MB);
				splx(s);
				return n;
			}
		}
	}

	mbstat.m_mbufs--;
	mbstat.m_mtypes[m->m_type]--;
	MBUF_UNLOCK();
	splx(s);
	m->m_type = MT_FREE;
	FREE(m, M_MBUF);
	return (n);
}

void
m_freem(m)
	register struct mbuf *m;
{
	while (m)
		m = m_free(m);
}

/*
 * Mbuffer utility routines.
 */

/*
 * Compute the amount of space available
 * before the current start of data in an mbuf.
 */
m_leadingspace(m)
register struct mbuf *m;
{
	if (m->m_flags & M_EXT) {
		if (MCLREFERENCED(m))
			return 0;
		return (m->m_data - m->m_ext.ext_buf);
	}
	if (m->m_flags & M_PKTHDR)
		return (m->m_data - m->m_pktdat);
	return (m->m_data - m->m_dat);
}

/*
 * Compute the amount of space available
 * after the end of data in an mbuf.
 */
m_trailingspace(m)
register struct mbuf *m;
{
	if (m->m_flags & M_EXT) {
		if (MCLREFERENCED(m))
			return 0;
		return (m->m_ext.ext_buf + m->m_ext.ext_size -
			(m->m_data + m->m_len));
	}
	return (&m->m_dat[MLEN] - (m->m_data + m->m_len));
}

/*
 * Lesser-used path for M_PREPEND:
 * allocate new mbuf to prepend to chain,
 * copy junk along.
 */
struct mbuf *
m_prepend(m, len, how)
	register struct mbuf *m;
	int len, how;
{
	struct mbuf *mn;

	MGET(mn, how, m->m_type);
	if (mn == (struct mbuf *)NULL) {
		m_freem(m);
		return ((struct mbuf *)NULL);
	}
	if (m->m_flags & M_PKTHDR) {
		M_COPY_PKTHDR(mn, m);
		m->m_flags &= ~M_PKTHDR;
	}
	mn->m_next = m;
	m = mn;
	if (len < MHLEN)
		MH_ALIGN(m, len);
	m->m_len = len;
	return (m);
}

/*
 * Make a copy of an mbuf chain starting "off0" bytes from the beginning,
 * continuing for "len" bytes.  If len is M_COPYALL, copy to end of mbuf.
 * The wait parameter is a choice of M_WAIT/M_DONTWAIT from caller.
 */
struct mbuf *
m_copym(m, off0, len, wait)
	register struct mbuf *m;
	int off0, wait;
	register int len;
{
	register struct mbuf *n, **np;
	register int off = off0;
	struct mbuf *top;
	int copyhdr = 0;

	if (off < 0 || len < 0)
		panic("m_copym sanity");
	if (off == 0 && m->m_flags & M_PKTHDR)
		copyhdr = 1;
	while (off > 0) {
		if (m == 0)
			panic("m_copym offset");
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	np = &top;
	top = 0;
	while (len > 0) {
		if (m == 0) {
			if (len != M_COPYALL)
				panic("m_copym length");
			break;
		}
		MGET(n, wait, m->m_type);
		*np = n;
		if (n == 0)
			goto nospace;
		if (copyhdr) {
			M_COPY_PKTHDR(n, m);
			if (len == M_COPYALL)
				n->m_pkthdr.len -= off0;
			else
				n->m_pkthdr.len = len;
			copyhdr = 0;
		}
		n->m_len = MIN(len, m->m_len - off);
		if (m->m_flags & M_EXT) {
			int s = splimp();
			MBUF_LOCK();
			n->m_ext = m->m_ext;
			insque(&n->m_ext.ext_ref, &m->m_ext.ext_ref);
			MBUF_UNLOCK();
			splx(s);
			n->m_data = m->m_data + off;
			n->m_flags |= M_EXT;
		} else
			bcopy(mtod(m, caddr_t)+off, mtod(n, caddr_t),
			    (unsigned)n->m_len);
		if (len != M_COPYALL)
			len -= n->m_len;
		off = 0;
		m = m->m_next;
		np = &n->m_next;
	}
	return (top);
nospace:
	m_freem(top);
	return (0);
}

/*
 * Copy data from an mbuf chain starting "off" bytes from the beginning,
 * continuing for "len" bytes, into the indicated buffer.
 */
void
m_copydata(m, off, len, cp)
	register struct mbuf *m;
	register int off;
	register int len;
	caddr_t cp;
{
	register unsigned count;

	if (off < 0 || len < 0)
		panic("m_copydata sanity");
	while (off > 0) {
		if (m == 0)
			panic("m_copydata offset");
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	while (len > 0) {
		if (m == 0)
			panic("m_copydata length");
		count = MIN(m->m_len - off, len);
		bcopy(mtod(m, caddr_t) + off, cp, count);
		len -= count;
		cp += count;
		off = 0;
		m = m->m_next;
	}
}

/*
 * Concatenate mbuf chain n to m.
 * Both chains must be of the same type (e.g. MT_DATA).
 * Any m_pkthdr is not updated.
 */
void
m_cat(m, n)
	register struct mbuf *m, *n;
{
	while (m->m_next)
		m = m->m_next;
	while (n) {
		if (m->m_flags & M_EXT ||
		    m->m_data + m->m_len + n->m_len >= &m->m_dat[MLEN]) {
			/* join the two chains, removing zero lengths */
			if (n->m_len == 0)
				m->m_next = m_free(n);
			else
				m->m_next = n;
			return;
		}
		/* splat the data from one into the other */
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t) + m->m_len,
		    (u_int)n->m_len);
		m->m_len += n->m_len;
		n = m_free(n);
	}
}

void
m_adj(mp, req_len)
	struct mbuf *mp;
	int req_len;
{
	register int len = req_len;
	register struct mbuf *m;
	register count;

	if ((m = mp) == NULL)
		return;
	if (len >= 0) {
		/*
		 * Trim from head.
		 */
		while (m != NULL && len > 0) {
			if (m->m_len <= len) {
				len -= m->m_len;
				m->m_len = 0;
				m = m->m_next;
			} else {
				m->m_len -= len;
				m->m_data += len;
				len = 0;
			}
		}
		if ((m = mp)->m_flags & M_PKTHDR)
			m->m_pkthdr.len -= (req_len - len);
	} else {
		/*
		 * Trim from tail.  Scan the mbuf chain,
		 * calculating its length and finding the last mbuf.
		 * If the adjustment only affects this mbuf, then just
		 * adjust and return.  Otherwise, rescan and truncate
		 * after the remaining size.
		 */
		len = -len;
		count = 0;
		for (;;) {
			count += m->m_len;
			if (m->m_next == (struct mbuf *)0)
				break;
			m = m->m_next;
		}
		if (m->m_len >= len) {
			m->m_len -= len;
			if ((m = mp)->m_flags & M_PKTHDR)
				m->m_pkthdr.len -= len;
			return;
		}
		count -= len;
		if (count < 0)
			count = 0;
		/*
		 * Correct length for chain is "count".
		 * Find the mbuf with last data, adjust its length,
		 * and toss data from remaining mbufs on chain.
		 */
		if ((m = mp)->m_flags & M_PKTHDR)
			m->m_pkthdr.len = count;
		for (; m; m = m->m_next) {
			if (m->m_len >= count) {
				m->m_len = count;
				break;
			}
			count -= m->m_len;
		}
		while (m = m->m_next)
			m->m_len = 0;
	}
}

/*
 * Rearange an mbuf chain so that len bytes are contiguous
 * and in the data area of an mbuf (so that mtod and dtom
 * will work for a structure of size len).  Returns the resulting
 * mbuf chain on success, frees it and returns null on failure.
 * If there is room, it will add up to max_protohdr-len extra bytes to the
 * contiguous region in an attempt to avoid being called next time.
 */
struct mbuf *
m_pullup(n, len)
	register struct mbuf *n;
	int len;
{
	register struct mbuf *m;
	register int count;
	int space;

	/*
	 * If first mbuf has no cluster, and has room for len bytes
	 * without shifting current data, pullup into it,
	 * otherwise allocate a new mbuf to prepend to the chain.
	 */
	if ((n->m_flags & M_EXT) == 0 &&
	    n->m_data + len < &n->m_dat[MLEN] && n->m_next) {
		if (n->m_len >= len)
			return (n);
		m = n;
		n = n->m_next;
		len -= m->m_len;
	} else {
		if (len > MHLEN)
			goto bad;
		MGET(m, M_DONTWAIT, n->m_type);
		if (m == 0)
			goto bad;
		m->m_len = 0;
		if (n->m_flags & M_PKTHDR) {
			M_COPY_PKTHDR(m, n);
			n->m_flags &= ~M_PKTHDR;
		}
	}
	space = &m->m_dat[MLEN] - (m->m_data + m->m_len);
	do {
		count = min(min(max(len, max_protohdr), space), n->m_len);
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t) + m->m_len,
		  (unsigned)count);
		len -= count;
		m->m_len += count;
		n->m_len -= count;
		space -= count;
		if (n->m_len)
			n->m_data += count;
		else
			n = m_free(n);
	} while (len > 0 && n);
	if (len > 0) {
		(void) m_free(m);
		goto bad;
	}
	m->m_next = n;
	return (m);
bad:
	m_freem(n);
	return (0);
}
/*
 * Same as m_pullup, except that a maximum of len bytes will be 
 * pulled up.
 */
struct mbuf *
m_pullup_exact(n, len)
	register struct mbuf *n;
	int len;
{
	register struct mbuf *m;
	register int count;
	int space;

	/*
	 * If first mbuf has no cluster, and has room for len bytes
	 * without shifting current data, pullup into it,
	 * otherwise allocate a new mbuf to prepend to the chain.
	 */
	if ((n->m_flags & M_EXT) == 0 &&
	    n->m_data + len < &n->m_dat[MLEN] && n->m_next) {
		if (n->m_len >= len)
			return (n);
		m = n;
		n = n->m_next;
		len -= m->m_len;
	} else {
		if (len > MHLEN)
			goto bad;
		MGET(m, M_DONTWAIT, n->m_type);
		if (m == 0)
			goto bad;
		m->m_len = 0;
		if (n->m_flags & M_PKTHDR) {
			M_COPY_PKTHDR(m, n);
			n->m_flags &= ~M_PKTHDR;
		}
	}
	space = &m->m_dat[MLEN] - (m->m_data + m->m_len);
	do {
		count = min(min(len, space), n->m_len);
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t) + m->m_len,
		  (unsigned)count);
		len -= count;
		m->m_len += count;
		n->m_len -= count;
		space -= count;
		if (n->m_len)
			n->m_data += count;
		else
			n = m_free(n);
	} while (len > 0 && n);
	if (len > 0) {
		(void) m_free(m);
		goto bad;
	}
	m->m_next = n;
	return (m);
bad:
	m_freem(n);
	return (0);
}

/*
 * get the length of an mbuf chain.
 */
int
m_length(m)
	register struct mbuf *m;
{
	register int len = 0;

	if (m) do {
		len += m->m_len;
	} while (m = m->m_next);

	return(len);
}

#if	STREAMS

/* Mbuf <-> Mblk conversion routines */
/*
 * Copy mbufs to and from mblks. Note these structures are very
 * similar but do not _quite_ line up.
 * A semi-complete list:
 *	MBUF		MBLK
 *	m_data		b_rptr
 *	m_len		b_wptr-b_rptr
 *	m_data+m_len	b_wptr
 *	m_next		b_cont
 *	m_nextpkt	b_next
 *	m_ext		mh_dblk
 *
 *	m_get()		allocb()/allocbi()
 *	m_freem()	freeb()
 *
 * When copying, for now we simply wrap each buffer in the chain
 * inside a buffer of the destination. This is to 1) avoid bcopies,
 * 2) simplify, 3) support only the necessary stuff, and could well
 * be changed.
 */


/*ARGSUSED*/
static void
m_freeb(p, size, bp)
	caddr_t p, bp;
	int size;
{
	(void) freeb((mblk_t *)bp);
}

/*ARGSUSED*/
static void
m_freea(p, q)
	char *p, *q;
{
	(void) m_free((struct mbuf *)p);
}

mblk_t *
mbuf_to_mblk(m, pri)
	struct mbuf *m;
	int pri;
{
	mblk_t *bp, *top = 0, **bpp = &top;
	struct mbuf *nm;

	while (m) {
		int front = m_leadingspace(m);
		int back  = m_trailingspace(m);
		bp = allocbi(front + m->m_len + back, pri,
			m_freea, (char *)m, (uchar *)(m->m_data - front));
		if (bp == 0) {
			while (top) {
				++top->b_datap->db_ref;
				bp = top->b_cont;
				(void) freeb(top);
				top = bp;
			}
			break;
		}
		bp->b_rptr = (unsigned char *)m->m_data;
		bp->b_wptr = (unsigned char *)(m->m_data + m->m_len);
		*bpp = bp;
		bpp = &bp->b_cont;
		nm = m->m_next;
		/*m->m_next = 0;*/
		m = nm;
	}
	return top;
}

struct mbuf *
mblk_to_mbuf(bp, canwait)
	mblk_t *bp;
	int canwait;
{
	struct mbuf *m, *top = 0;
	struct mbuf **mp = &top;
	mblk_t *nbp;
	int len = 0;

	while (bp) {
		if (top == 0)
			m = m_gethdr(canwait, MT_DATA);
		else
			m = m_get(canwait, MT_DATA);
		if (m == 0) {
			while (top) {
				top->m_flags &= ~M_EXT;
				top = m_free(top);
			}
			break;
		}
		m->m_data = (caddr_t)bp->b_rptr;
		m->m_len = bp->b_wptr - bp->b_rptr;
		m->m_flags |= M_EXT;
		if (bp->b_datap->db_ref > 1) {		/* referenced */
			m->m_ext.ext_buf = m->m_data;
			m->m_ext.ext_size = m->m_len;
		} else {				/* available */
			m->m_ext.ext_buf = (caddr_t)bp->b_datap->db_base;
			m->m_ext.ext_size =
				bp->b_datap->db_lim - bp->b_datap->db_base;
		}
		m->m_ext.ext_free = m_freeb;
		m->m_ext.ext_arg = (caddr_t)bp;
		m->m_ext.ext_ref.forw = m->m_ext.ext_ref.back = 
			&m->m_ext.ext_ref;
		*mp = m;
		mp = &m->m_next;
		len += m->m_len;
		nbp = bp->b_cont;
		/*bp->b_cont = 0;*/
		bp = nbp;
	}
	if (top) {
		top->m_pkthdr.len = len;
		top->m_pkthdr.rcvif = 0;
 		/*
 		 * If we have only one mbuf and len is < MHLEN
 		 * then we copy the data and free the mblk right here
 		 * This also helps in colapsing the long mbuf chain in
 		 * lower level code.
 		 */
 		if ((top->m_next == NULL) && (len <= MHLEN)) {
 
 			mblk_t	*obp =  (mblk_t *)top->m_ext.ext_arg;
 
 			/* copy the data right into the mbuf */
 			m->m_flags &= ~M_EXT;
 
 			/* 1 figures often, so optimize it */
 			if (len==1)
 				*top->m_pktdat = *top->m_data;
 			else
 				bcopy(top->m_data, top->m_pktdat, len);
 
 			/* adjust the pointer */
 			top->m_data = top->m_pktdat;
 
 			/* Now free up the mblk right here */
 			freeb(obp);
 		}
	}
	return top;
}

#endif	/* STREAMS */
