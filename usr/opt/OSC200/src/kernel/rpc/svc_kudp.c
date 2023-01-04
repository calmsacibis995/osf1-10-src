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
static char *rcsid = "@(#)$RCSfile: svc_kudp.c,v $ $Revision: 1.1.10.3 $ (DEC) $Date: 1993/08/02 14:55:22 $";
#endif
#ifdef	KERNEL
#ifndef lint
static char sccsid[] = 	"@(#)svc_kudp.c	1.3 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.28 88/02/08
 */


/*
 * svc_kudp.c,
 * Server side for UDP/IP based RPC in the kernel.
 */
#include <rpc/rpc.h>
#include <sys/mbuf.h>

#define	rpc_buffer(xprt) ((xprt)->xp_p1)

#ifdef	DEBUG
extern int rrokdebug;
int svcdupdebug = 0;
#define DPRINTF(c, args)	if(c) printf args
#else
#define DPRINTF(c, args)
#endif


/*
 * Routines exported through ops vector.
 */
bool_t		svckudp_recv();
bool_t		svckudp_send();
enum xprt_stat	svckudp_stat();
bool_t		svckudp_getargs();
bool_t		svckudp_freeargs();
void		svckudp_destroy();

/*
 * Server transport operations vector.
 */
struct xp_ops svckudp_op = {
	svckudp_recv,		/* Get requests */
	svckudp_stat,		/* Return status */
	svckudp_getargs,	/* Deserialize arguments */
	svckudp_send,		/* Send reply */
	svckudp_freeargs,	/* Free argument data space */
	svckudp_destroy		/* Destroy transport handle */
};


struct mbuf	*ku_recvfrom();
void		xdrmbuf_init();

/*
 * Transport private data.
 * Kept in xprt->xp_p2.
 */
struct udp_data {
	int	ud_flags;			/* flag bits, see below */
	u_int 	ud_xid;				/* id */
	struct	mbuf *ud_inmbuf;		/* input mbuf chain */
	XDR	ud_xdrin;			/* input xdr stream */
	XDR	ud_xdrout;			/* output xdr stream */
	char	ud_verfbody[MAX_AUTH_BYTES];	/* verifier */
};


/*
 * Flags
 */
#define	UD_BUSY		0x001		/* buffer is busy */
#define	UD_WANTED	0x002		/* buffer wanted */

/*
 * Server statistics
 */
struct {
	int	rscalls;
	int	rsbadcalls;
	int	rsnullrecv;
	int	rsbadlen;
	int	rsxdrcall;
} rsstat;

/*
 * Create a transport record.
 * The transport record, output buffer, and private data structure
 * are allocated.  The output buffer is serialized into using xdrmem.
 * There is one transport record per user process which implements a
 * set of services.
 */
SVCXPRT *
svckudp_create(sock, port)
	struct socket	*sock;
	u_short		 port;
{
	register SVCXPRT	 *xprt;
	register struct udp_data *ud;

	xprt = (SVCXPRT *)kmem_alloc((u_int)sizeof(SVCXPRT));
	rpc_buffer(xprt) = (caddr_t)kmem_alloc((u_int)UDPMSGSIZE);
	ud = (struct udp_data *)kmem_alloc((u_int)sizeof(struct udp_data));
	bzero((caddr_t)ud, sizeof(*ud));
	xprt->xp_addrlen = 0;
	xprt->xp_p2 = (caddr_t)ud;
	xprt->xp_p3 = NULL;
	xprt->xp_verf.oa_base = ud->ud_verfbody;
	xprt->xp_ops = &svckudp_op;
	xprt->xp_port = port;
	xprt->xp_sock = sock;
	xprt_register(xprt);
	return (xprt);
}
 
/*
 * Destroy a transport record.
 * Frees the space allocated for a transport record.
 */
void
svckudp_destroy(xprt)
	register SVCXPRT   *xprt;
{
	register struct udp_data *ud = (struct udp_data *)xprt->xp_p2;
	int						  s;

	/*
	 * Make sure that the buffer is not in use before freeing it.
	 */
	s = splimp();
	while (ud->ud_flags & UD_BUSY) {
		ud->ud_flags |= UD_WANTED;
		(void) sleep((caddr_t)ud, PZERO-2);
	}
	(void) splx(s);

	if (ud->ud_inmbuf) {
		m_freem(ud->ud_inmbuf);
	}
	kmem_free((caddr_t)ud, (u_int)sizeof(struct udp_data));
	kmem_free((caddr_t)rpc_buffer(xprt), (u_int)UDPMSGSIZE);
	kmem_free((caddr_t)xprt, (u_int)sizeof(SVCXPRT));
}

/*
 * Receive rpc requests.
 * Pulls a request in off the socket, checks if the packet is intact,
 * and deserializes the call packet.
 */

bool_t
svckudp_recv(xprt, msg)
	register SVCXPRT	 *xprt;
	struct rpc_msg		 *msg;
{
	register struct udp_data *ud = (struct udp_data *)xprt->xp_p2;
	register XDR	 *xdrs = &(ud->ud_xdrin);
	register struct mbuf	 *m;
	int			  s;

	rsstat.rscalls++;
	m = ku_recvfrom(xprt->xp_sock, &(xprt->xp_raddr));
	if (m == NULL) {
		rsstat.rsnullrecv++;
		return (FALSE);
	}
	/*
	 * EJWXXX: delete the printf and restore the goto when m_pkthdr is
	 * trustworthy.  Currently m_pkthdr reflects the length of the first
	 * IP fragment.
	 */
	if (m->m_pkthdr.len < 8 * BYTES_PER_XDR_UNIT) {
		printf("svckudp_recv: bad length: m 0x%08x, m_flags %x, \n\
    m_len %d, m_pkthdr.len %d\n", m, m->m_flags, m->m_len, m->m_pkthdr.len);
		rsstat.rsbadlen++;
/* EJWXXX	goto bad; */
	}
	xdrmbuf_init(&ud->ud_xdrin, m, XDR_DECODE);
	if (! xdr_callmsg(xdrs, msg)) {
		rsstat.rsxdrcall++;
		goto bad;
	}
#ifdef	KTRACE
	kern_trace(9, msg->rm_xid, 0, 0);
#endif  /* KTRACE */
	ud->ud_xid = msg->rm_xid;
	ud->ud_inmbuf = m;
	xprt->xp_ifp = ifa_ifwithnet(&xprt->xp_raddr);
	return (TRUE);

bad:
	m_freem(m);
	ud->ud_inmbuf = NULL;
	rsstat.rsbadcalls++;
	return (FALSE);
}


static
buffree(addr, size, ud)
	caddr_t addr;
	u_int	size;
	register struct udp_data *ud;
{
	ud->ud_flags &= ~UD_BUSY;
	if (ud->ud_flags & UD_WANTED) {
		ud->ud_flags &= ~UD_WANTED;
		wakeup((caddr_t)ud);
	}
}

/*
 * Send rpc reply.
 * Serialize the reply packet into the output buffer then
 * call ku_sendto to make an mbuf out of it and send it.
 */
long svcsendwait = 0;
bool_t
/* ARGSUSED */
svckudp_send(xprt, msg)
	register SVCXPRT *xprt; 
	struct rpc_msg *msg; 
{
	register struct udp_data *ud = (struct udp_data *)xprt->xp_p2;
	register XDR *xdrs = &(ud->ud_xdrout);
	register int slen;
	register int stat = FALSE;
	int s;
	struct mbuf *m, *mclgetx();

	s = splimp();
	while (ud->ud_flags & UD_BUSY) {
		++svcsendwait;
		ud->ud_flags |= UD_WANTED;
		(void) sleep((caddr_t)ud, PZERO-2);
	}
	ud->ud_flags |= UD_BUSY;
	(void) splx(s);
	m = mclgetx(buffree, (int)ud, rpc_buffer(xprt), UDPMSGSIZE, M_WAIT);
	if (m == NULL) {
		buffree(rpc_buffer(xprt), UDPMSGSIZE, ud);
		printf("svckudp_send: mclgetx failed\n");
		return (stat);
		/* CJXXX - who does vop_brelse?? */
	}

	xdrmbuf_init(&ud->ud_xdrout, m, XDR_ENCODE);
	msg->rm_xid = ud->ud_xid;
	if (xdr_replymsg(xdrs, msg)) {
		slen = (int)XDR_GETPOS(xdrs);
		if (m->m_next == 0) {		/* XXX */
			m->m_len = slen;
		}
		if (!ku_sendto_mbuf(xprt->xp_sock, m, &xprt->xp_raddr))
			stat = TRUE;
	} else {
		printf("svckudp_send: xdr_replymsg failed\n");
		m_freem(m);
	}
	/*
	 * This is completely disgusting.  If public is set it is
	 * a pointer to a structure whose first field is the address
	 * of the function to free that structure and any related
	 * stuff.  (see rrokfree in nfs_xdr.c).
	 */
	if (xdrs->x_public) {
		(**((int (**)())xdrs->x_public))(xdrs->x_public);
	}

	return (stat);
}

/*
 * Return transport status.
 */
/*ARGSUSED*/
enum xprt_stat
svckudp_stat(xprt)
	SVCXPRT *xprt;
{

	return (XPRT_IDLE); 
}

/*
 * Deserialize arguments.
 */
bool_t
svckudp_getargs(xprt, xdr_args, args_ptr)
	SVCXPRT	*xprt;
	xdrproc_t	 xdr_args;
	caddr_t		 args_ptr;
{

	return ((*xdr_args)(&(((struct udp_data *)(xprt->xp_p2))->ud_xdrin), args_ptr));
}

bool_t
svckudp_freeargs(xprt, xdr_args, args_ptr)
	SVCXPRT	*xprt;
	xdrproc_t	 xdr_args;
	caddr_t		 args_ptr;
{
	register XDR *xdrs =
	    &(((struct udp_data *)(xprt->xp_p2))->ud_xdrin);
	register struct udp_data *ud = (struct udp_data *)xprt->xp_p2;

	if (ud->ud_inmbuf) {
		m_freem(ud->ud_inmbuf);
	}
	ud->ud_inmbuf = (struct mbuf *)0;
	if (args_ptr) {
		xdrs->x_op = XDR_FREE;
		return ((*xdr_args)(xdrs, args_ptr));
	} else {
		return (TRUE);
	}
}

/*
 * The dup cacheing routines below provide a cache of recent
 * transaction id's.
 */

/* CJXXX */
/*
 * kernel-based RPC server duplicate transaction cache flag values
 */
#define	NDUP_NEW			0x0	/* new request */
#define	NDUP_INPROGRESS		0x01	/* request already going */
#define	NDUP_DONE		0x02	/* request done */
#define	NDUP_DROP		0x03	/* request done */

svckudp_dup();
void svckudp_dupdone();
static void unhash();

int kudp_usedrc = 1;

struct dupreq {
	u_int		dr_xid;		/* unique transaction ID */
	u_int		dr_addr;	/* client address */
	u_int		dr_vers;	/* version of prog (assumes NFS) */
	u_int		dr_proc;	/* proc within prog, vers */
	int		dr_status;	/* request status	*/
	caddr_t		dr_resp;	/* cached response	*/
	struct dupreq	*dr_next;	/* linked list of all entries */
	struct dupreq	*dr_chain;	/* hash chain */
};

struct dupreq **drhashtbl; /* array of heads of hash lists */
int drhashsz;		   /* number of hash lists */
int drhashszminus1;	   /* hash modulus */

#define MINDUPREQS	512
#define	MAXDUPREQS	4096

/*
 * dupreq_stats.ndupreqs is the number of cached items.  It is set
 * based on "system size". It should be large enough to hold
 * transaction history long enough so that a given entry is still
 * around for a few retransmissions of that transaction.
 */

struct {
	int	ndupreqs;	/* size of cache */
	int	checks;		/* number of checks */
	int	missing;	/* cache wraparound */
	int	inprogress;	/* busy duplicates */
	int	done;		/* completed duplicates */
} dupreq_stats;

/*
 * cache support functions:
 *	xid hash function (the remainder function samples all 32 bits of xid)
 *	xid 		  -> head of hash list
 *	duplicate request -> head of hash list
 */

#define	XIDHASH(xid)	((xid) % drhashszminus1)
#define XIDTOLIST(xid) 	((struct dupreq *)(drhashtbl[XIDHASH(xid)]))
#define	REQTOLIST(dr)	XIDTOLIST((dr)->dr_xid)

/* SMP lock for dupreq hash list and dupreq event counters */
/* struct	lock_t	lk_rpcdupreq; */

/* routine to compare dup cache entries */
#define NOTDUP(dr, xid, vers, proc, addr) (dr->dr_xid != xid || \
			      dr->dr_vers != vers || \
			      dr->dr_proc != proc || \
			      dr->dr_addr != addr)

/*
 * drmru points to the head of a circular linked list in lru order.
 * drmru->dr_next == least recently entered (i.e. oldest) entry.
 * entries are not moved on this list when they are modified.
 */
struct dupreq *dupreqcache, *drmru;

extern int ressize;

int
svckudp_dup(xid, proc, vers, addr, res, size)
	u_int xid, proc, vers, addr;
	register caddr_t res;
	register int size;
{
	register struct dupreq *dr;
	register struct dupreq **dt;
	extern int prattached;

	if (!kudp_usedrc)
		return (0);

	DPRINTF(svcdupdebug, ("svckudp_dupbusy: xid 0x%x\n", xid));

	/* smp_lock(&lk_rpcdupreq, LK_RETRY); */

	/* First time through, allocate and init hash list and cache area */
	if (!dupreqcache) {
		int i, dupcache_max;

		dupcache_max = 512 * nfs_system_size();
		/* Prestoserve allows a server to cycle many more requests */
		if (prattached)		/* Prestoserve is present */
			dupcache_max *= 2;
		dupcache_max = MAX(MINDUPREQS, dupcache_max);
		dupcache_max = MIN(MAXDUPREQS, dupcache_max);
		drhashsz = dupcache_max / 16;
		drhashszminus1 = drhashsz - 1;
		/* smp_unlock(&lk_rpcdupreq); */

		/*
		 * We used to assume kmem_alloc will block
		 * until success.  We give up the
		 * lock in case we block.  Since kalloc may not block
		 * we check and bail out if we can not grab memory.
		 */
		(char *)dr = kalloc(sizeof(*dr) * dupcache_max);
		if (dr == NULL) {
			/* smp_unlock(&lk_rpcdupreq); */
			printf("Can't allocate NFS server request cache1!\n");
			kudp_usedrc = 0;
			return (0);
		}
		bzero((caddr_t)dr, sizeof(*dr) * dupcache_max);

		(char *)dt = kalloc(sizeof(struct dupreq *) * drhashsz);
		if (dt == NULL) {
			/* smp_unlock(&lk_rpcdupreq); */
			kfree(dr, sizeof(*dr) * dupcache_max);
			printf("Can't allocate NFS server request cache2!\n");
			kudp_usedrc = 0;
			return (0);
		}
		bzero((caddr_t)dt, sizeof(struct dupreq *) * drhashsz);

		/* smp_lock(&lk_rpcdupreq, LK_RETRY); */
		if (!dupreqcache) { /* we got it first */
			for (i = 0; i < dupcache_max; i++) {
				dr[i].dr_next = &(dr[i + 1]);
				if (ressize)
					dr[i].dr_resp = kalloc(ressize);
				if (!ressize || dr[i].dr_resp == NULL)
					panic("svckudp_dupbusy: can't kalloc");
			}
			dr[dupcache_max - 1].dr_next = dr;
			dupreqcache = dr;
			dupreq_stats.ndupreqs = dupcache_max;
			drmru = dr;
			drhashtbl = dt;
		}
		else  { /* someone beat us to it */
			kfree(dr, sizeof(*dr) * dupcache_max);
			kfree(dt, sizeof(struct dupreq *) * drhashsz);
		}
	}

	dupreq_stats.checks++;
	dr = XIDTOLIST(xid); 
	while (dr != NULL) { 
		if (NOTDUP(dr, xid, vers, proc, addr)) {
			dr = dr->dr_chain;
			continue;
		}

		DPRINTF(svcdupdebug,
			("svckudp_dup: dupreq xid 0x%x status 0x%x\n",
			xid, dr->dr_status));

		switch (dr->dr_status) {
		      case NDUP_INPROGRESS:
			dupreq_stats.inprogress++;
			break;
		      case NDUP_DONE:
			bcopy(dr->dr_resp, res, size);
			dupreq_stats.done++;
			break;
		      default:
			printf("bad dr_status 0x%x\n", dr->dr_status); 
			panic ("svckudp_dup 1");
		}
		return (dr->dr_status);
	}

	DPRINTF(svcdupdebug,
		("svckudp_dupbusy: enter xid 0x%x\n", xid));
	dr = drmru->dr_next;
	unhash(dr);
	drmru = dr;
	dr->dr_xid = xid;
	dr->dr_proc = proc;
	dr->dr_vers = vers;
	dr->dr_addr = addr;
	dr->dr_status = NDUP_INPROGRESS;
	dr->dr_chain = REQTOLIST(dr);
	REQTOLIST(dr) = dr;

	/* smp_unlock(&lk_rpcdupreq); */
 	return (NDUP_NEW);
}

void
svckudp_dupdone(xid, proc, vers, addr, res, size, status)
	u_int xid, proc, vers, addr;
	register caddr_t res;
	register int	size;
	int status;
{
	register struct dupreq *dr;
	 
	if (!kudp_usedrc)
		return;

	DPRINTF(svcdupdebug, ("svckudp_dupdone: xid 0x%x\n", xid));

	/* smp_lock(&lk_rpcdupreq, LK_RETRY); */
	dr = XIDTOLIST(xid); 
	while (dr != NULL) { 
		if (NOTDUP(dr, xid, vers, proc, addr)) {
			dr = dr->dr_chain;
			continue;
 		}
		if (!res || !size) {
			printf("dupdone: bad copy 0x%x %d\n", res, size);
			panic("svckudp_dupdone");
		}
		if (size > ressize || !dr->dr_resp) {
			printf("size %d max %d resp 0x%x\n",
			       size, ressize, dr->dr_resp);
			panic("svckudp_dupdone");
		}
		bcopy(res, dr->dr_resp, (u_int)size);
		dr->dr_status = status;
		/* smp_unlock(&lk_rpcdupreq); */
 		return;
	}

	DPRINTF(svcdupdebug, ("svckudp_dupdone: xid 0x%x NOTFOUND\n", xid));
	dupreq_stats.missing++;	/* cache wraparound */
	/* smp_unlock(&lk_rpcdupreq); */
	return;

}

static void
unhash(dr)
	struct dupreq *dr;
{
	struct dupreq *drt;
	struct dupreq *drtprev = NULL;
	 
	/*
	 * NB:
	 *	This routine must only be called while holding
	 *	the lk_rpcdupreq lock.
	 */

	drt = REQTOLIST(dr); 
	while (drt != NULL) { 
		if (drt == dr) { 
			if (drtprev == NULL) {
				REQTOLIST(dr) = drt->dr_chain;
			} else {
				drtprev->dr_chain = drt->dr_chain;
			}
			return; 
		}	
		drtprev = drt;
		drt = drt->dr_chain;
	}	
}

#endif	/* KERNEL */
