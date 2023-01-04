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
static char *rcsid = "@(#)$RCSfile: subr_kudp.c,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 93/02/15 15:03:19 $";
#endif
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)subr_kudp.c	1.4 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.26 88/02/08 
 */


/*
 * subr_kudp.c
 * Subroutines to do UDP/IP sendto and recvfrom in the kernel
 */
#include <rpc/rpc.h>
#include <sys/mbuf.h>

/*
 * General kernel udp stuff.
 * The routines below are used by both the client and the server side
 * rpc code.
 */
int recvdebug=0;
int rpc_empty_mbufs=0;
/*
 * Kernel recvfrom.
 * Pull address mbuf and data mbuf chain off socket receive queue.
 */
struct mbuf *
ku_recvfrom(so, from)
	struct socket *so;
	struct sockaddr_in *from;
{
	register struct mbuf	*m,*mp;
	register struct mbuf	*m0;
	struct mbuf		*nextrecord;
	register struct sockbuf	*sb = &so->so_rcv;
	register int		len = 0;
	int			s;

#ifdef	KTRACE
	kern_trace(10,sb->sb_cc, 0, 0);
#endif  /* KTRACE */

	s=splnet();
	m = so->so_rcv.sb_mb;
	if (m == 0)
		panic("ku_recvfrom - empty sockbuf");
	nextrecord = m->m_nextpkt;

	/* Save sender's address */
	if (m->m_type != MT_SONAME)
		panic("ku_recvfrom - not SO_NAME");

	sbfree(&so->so_rcv, m);
	*from = *mtod(m, struct sockaddr_in *);
	so->so_rcv.sb_mb = m->m_next;
	m->m_next = NULL;
	m_free(m);

	m = so->so_rcv.sb_mb;

	/* Drop control mbuf's */
	while (m && m->m_type != MT_DATA && m->m_type != MT_HEADER) {
		if (m->m_type == MT_RIGHTS)
			panic("ku_recvfrom - strange packet type");
		sbfree(&so->so_rcv, m);
		MFREE(m, so->so_rcv.sb_mb);
		m = so->so_rcv.sb_mb;
	}
	/* Dequeue packet from sockbuf */
	m0 = m;
	while (m) {
		if (m->m_type != MT_DATA && m->m_type != MT_HEADER)
			panic("nfs_dgreceive 3");
		sbfree(&so->so_rcv, m);
		mp = m;
		m = so->so_rcv.sb_mb = m->m_next;
		if ((m != NULL) && (m->m_len == 0)) {
			rpc_empty_mbufs++;
/*			printf("ku_recvfrom: dropping empty mbuf\n"); */
			mp->m_next = m->m_next;
			sbfree(&so->so_rcv, m);
			m_free(m);
			m = so->so_rcv.sb_mb = mp->m_next;
		}
	}
	so->so_rcv.sb_mb = nextrecord;
	splx(s);
	return(m0);
}

int Slow_Sendtries = 0;
int Slow_Sendok = 0;
extern int rrokdebug;
/*
 * Kernel sendto.
 * Set addr and send off via UDP.
 * Use version in kudp_fastsend.c if possible.
 */
int
ku_slow_sendto_mbuf(so, m, addr)
	struct socket *so;
	struct mbuf *m;
	struct sockaddr_in *addr;
{
	register struct inpcb *inp = sotoinpcb(so);
	struct mbuf *nam,*m2;
	int error,len=0;

	Slow_Sendtries++;

	m->m_flags |= M_PKTHDR;
	/*
	 * Calculate packet length.
	 */
	m2 = m;
	while(m2) {
		if (rrokdebug)
			printf("ku_slow_sendto_mbuf: m %x len %d\n",m2,m2->m_len);
		len += m2->m_len;
		m2=m2->m_next;
	}
	m->m_pkthdr.len = len;
	m->m_pkthdr.rcvif = (struct ifnet *) NULL;

	/*
	 * Copy the address into a mbuf
	 */
	nam = m_get(M_DONTWAIT, MT_SONAME);
	if (nam == (struct mbuf *)0) {
		printf("ku_slow_sendto_mbuf: Can't get addr mbuf\n");
		m_freem(m);
		return (ENOBUFS);
	}
	nam->m_len = sizeof(*addr);
	bcopy((caddr_t)addr,  mtod(nam, caddr_t), sizeof(*addr));

	error = udp_output(inp, m, nam, NULL);

	m_free(nam);

	if (!error)
		Slow_Sendok++;

	return (error);
}

/*
 * DWS: move this some place else??
 */
struct mbuf *
mclgetx(fun, arg, addr, len, wait)
	void (*fun)();
	caddr_t arg; 
	caddr_t addr;
	int len;
	int wait;
{
	register struct mbuf *m = (struct mbuf *)0;

	MGET(m, wait, MT_DATA);
	if (m) {
		m->m_flags |= (M_EXT|M_FASTFREE);
		m->m_data = m->m_ext.ext_buf = addr;
		m->m_len = m->m_ext.ext_size = len;
		m->m_ext.ext_free = fun;
		m->m_ext.ext_arg  = arg;
		m->m_ext.ext_ref.forw = m->m_ext.ext_ref.back =
			&m->m_ext.ext_ref;
	}

	return (m);
}
