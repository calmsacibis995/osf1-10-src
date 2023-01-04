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
static char *rcsid = "@(#)$RCSfile: nfs_subr.c,v $ $Revision: 1.1.17.5 $ (DEC) $Date: 1993/07/07 17:33:25 $";
#endif
/*	@(#)nfs_subr.c	1.13 90/07/02 NFSSRC4.1 from 2.118 90/02/01 SMI 	*/

/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/mode.h>
#include <sys/uio.h>
#include <rpc/rpc.h>

#ifdef	TRACE
#ifndef	NFSSERVER
#define	NFSSERVER
#include <nfs/nfs.h>
#undef	NFSSERVER
#else	NFSSERVER
#include <nfs/nfs.h>
#endif	NFSSERVER
#else	TRACE
#include <nfs/nfs.h>
#endif	TRACE
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/domain.h>

#include <nfs/nfs_clnt.h>
#include <nfs/rnode.h>

/*
 * Move to conf/param.h
 */
#define MAXCLIENTS      5
u_int   nchtable = 2;           /* Target number of free client handles */
u_int   nunixauthtab = MAXCLIENTS;

/*
 * Client handle used to keep the pagedaemon from deadlocking waiting for mem.
 * XXX - bugid 1026717
 */
struct chtab  *ch_pagedaemon = (struct chtab *)NULL;


extern struct vnodeops nfs_vnodeops;
static struct rnode *rfind();

extern int nrnode;		/* max rnodes to alloc, set in machdep.c */

extern char *rfsnames[];
extern int sys_v_mode;

/*
 * Client side utilities
 */

/*
 * client side statistics
 */
struct {
	u_int	nclsleeps;		/* client handle waits */
	u_int	nclgets;		/* client handle gets */
	u_int	ncalls;			/* client requests */
	u_int	nbadcalls;		/* rpc failures */
	u_int	reqs[32];		/* count of each request */
} clstat;

struct chtab {
	struct chtab *ch_next, *ch_prev;
	u_int	ch_timesused;
	bool_t	ch_inuse;
	bool_t  ch_int;
	struct proc *ch_proc;
	struct socket *ch_so;
	CLIENT	*ch_client;
} chtabhead = { &chtabhead, &chtabhead, 0, 0, 0, NULL, NULL, NULL };

extern u_int nchtable;
u_int nchtotal;
u_int nchfree;

int	nfs_client_thread_init=0;

#define NFS_HZ	3
#define NFSINT_SIGMASK  (sigmask(SIGHUP)|sigmask(SIGINT)|sigmask(SIGQUIT)| \
                         sigmask(SIGKILL)|sigmask(SIGTERM))

static void rp_addhash(struct rnode *);

#ifdef notdef
u_int authdes_win = (60*60);	/* one hour -- should be mount option */

struct desauthent {
	struct mntinfo *da_mi;
	uid_t da_uid;
	short da_inuse;
	AUTH *da_auth;
} *desauthtab = NULL;
extern u_int ndesauthtab;
int nextdesvictim;
#endif

struct unixauthent {
	short ua_inuse;
	AUTH *ua_auth;
} *unixauthtab = NULL;
extern u_int nunixauthtab;
int nextunixvictim;

AUTH *
authget(mi, cr)
	struct mntinfo *mi;
	struct ucred *cr;
{
	int i;
	AUTH *auth;
	register struct unixauthent *ua;
	register struct desauthent *da;
	int authflavor;
	struct ucred *savecred;

	authflavor = mi->mi_authflavor;
	for (;;) switch (authflavor) {
	case AUTH_NONE:
		/*
		 * XXX: should do real AUTH_NONE, instead of AUTH_UNIX
		 */
	case AUTH_UNIX:
		if (unixauthtab == NULL) {
		/* XXX this should be in some initialization code */
			unixauthtab = (struct unixauthent *)
			    kmem_alloc(
				nunixauthtab * sizeof (struct unixauthent));
			bzero((caddr_t)unixauthtab, nunixauthtab * sizeof (struct unixauthent));
		}
		i = nunixauthtab;
		do {
			ua = &unixauthtab[nextunixvictim++];
			nextunixvictim %= nunixauthtab;
		} while (ua->ua_inuse && --i > 0);

		if (ua->ua_inuse) {
			/* overflow of unix auths */
			return (authkern_create());
		}

		if (ua->ua_auth == NULL) {
			ua->ua_auth = authkern_create();
		}
		ua->ua_inuse = 1;
		return (ua->ua_auth);
#ifdef notdef
	case AUTH_DES:
		if (desauthtab == NULL) {
		/* XXX this should be in some initialization code */
			desauthtab = (struct desauthent *)
			    kmem_zalloc(
				ndesauthtab * sizeof (struct desauthent));
		}
		for (da = desauthtab; da < &desauthtab[ndesauthtab]; da++) {
			if (da->da_mi == mi && da->da_uid == cr->cr_uid &&
			    !da->da_inuse && da->da_auth != NULL) {
				da->da_inuse = 1;
				return (da->da_auth);
			}
		}

		savecred = u.u_cred;
		u.u_cred = cr;
		auth = authdes_create(mi->mi_netname, authdes_win,
				&mi->mi_addr, (des_block *)NULL);
		u.u_cred = savecred;

		if (auth == NULL) {
			printf("authget: authdes_create failure\n");
			authflavor = AUTH_UNIX;
			continue;
		}

		i = ndesauthtab;
		do {
			da = &desauthtab[nextdesvictim++];
			nextdesvictim %= ndesauthtab;
		} while (da->da_inuse && --i > 0);

		if (da->da_inuse) {
			/* overflow of des auths */
			return (auth);
		}

		if (da->da_auth != NULL) {
			auth_destroy(da->da_auth);	/* should reuse!!! */
		}

		da->da_auth = auth;
		da->da_inuse = 1;
		da->da_uid = cr->cr_uid;
		da->da_mi = mi;
		return (da->da_auth);
#endif
	default:
		/*
		 * auth create must have failed, try AUTH_NONE
		 * (this relies on AUTH_NONE never failing)
		 */
		printf("authget: unknown authflavor %d\n", authflavor);
		authflavor = AUTH_NONE;
	}
}

void
authfree(auth)
	AUTH *auth;
{
	register struct unixauthent *ua;
	register struct desauthent *da;

	switch (auth->ah_cred.oa_flavor) {
	case AUTH_NONE: /* XXX: do real AUTH_NONE */
	case AUTH_UNIX:
		for (ua = unixauthtab; ua < &unixauthtab[nunixauthtab]; ua++) {
			if (ua->ua_auth == auth) {
				ua->ua_inuse = 0;
				return;
			}
		}
		auth_destroy(auth);	/* was overflow */
		break;
#ifdef notdef
	case AUTH_DES:
		for (da = desauthtab; da < &desauthtab[ndesauthtab]; da++) {
			if (da->da_auth == auth) {
				da->da_inuse = 0;
				return;
			}
		}
		auth_destroy(auth);	/* was overflow */
		break;
#endif
	default:
		printf("authfree: unknown authflavor %d\n", auth->ah_cred.oa_flavor);
		break;
	}
}


CLIENT *
clget(mi, cred)
	struct mntinfo *mi;
	struct ucred *cred;
{
	register struct chtab *ch;
	int retrans;

	/*
	 * If soft mount and server is down just try once
	 * Interruptable hard mounts get counted at this level.
	 */
	if ((!mi->mi_hard && mi->mi_down) || (mi->mi_int && mi->mi_hard)) {
		retrans = 1;
	} else {
		retrans = mi->mi_retrans;
	}

	/*
	 * If the pagedaemon wants a handle, then give it its pre-allocated
	 * handle.  If the pagedaemon gets here first and there isn't one yet,
	 * then it has to take its chances... XXX
	 */
	if ((u.u_procp == &proc[2]) && (ch_pagedaemon != (struct chtab *)NULL))
{
		ch = ch_pagedaemon;
		if (ch->ch_inuse)
			panic("clget:  pagedaemon needs > 1 cl handle!!\n");
		ch->ch_inuse = TRUE;
		if (ch->ch_client == (CLIENT *)NULL)
			panic("clget:  null client structure\n");
		clntkudp_init(ch->ch_client, &mi->mi_addr, retrans, cred);
		goto out;
	}

	/*
	 * Find an unused handle
	 */
	clstat.nclgets++;
	for (ch = chtabhead.ch_next; ch != &chtabhead; ch = ch->ch_next) {
		if (!ch->ch_inuse) {
			ch->ch_inuse = TRUE;
			clntkudp_init(ch->ch_client, &mi->mi_addr,
					retrans, cred);
			nchfree--;
			goto out;
		}
	}

again:

	/*
	 *  If we get here, we need to make a new handle
	 */
	ch = (struct chtab *)kmem_alloc( sizeof (struct chtab));
	bzero((caddr_t)ch, sizeof (struct chtab));
	/*
	 * First allocation:  stash one away for the pagedaemon
	 */
	if (ch_pagedaemon == (struct chtab *)NULL) {
		ch_pagedaemon = ch;
		ch->ch_client = clntkudp_create(&mi->mi_addr, NFS_PROGRAM,
					NFS_VERSION, retrans, cred);
		if (ch->ch_client == NULL) {
			panic("clget: null client");
		}
		auth_destroy(ch->ch_client->cl_auth); /* XXX */
		if (u.u_procp != &proc[2]) {
			ch->ch_inuse = FALSE;	/* pagedaemon will use later */
			goto again;
		} else {
			ch->ch_inuse = TRUE;	/* pagedaemon will use now */
			goto out;
		}
	}

	ch->ch_inuse = TRUE;
	ch->ch_next = chtabhead.ch_next;
	ch->ch_prev = &chtabhead;
	chtabhead.ch_next->ch_prev = ch;
	chtabhead.ch_next = ch;
	ch->ch_client = clntkudp_create(&mi->mi_addr, NFS_PROGRAM, NFS_VERSION,
					retrans, cred);
        ch->ch_int = mi->mi_int;
	if (ch->ch_client == NULL) {
		panic("clget: null client");
	}
	auth_destroy(ch->ch_client->cl_auth); /* XXX */
	nchtotal++;
out:
	ch->ch_proc = u.u_procp;
	ch->ch_client->cl_auth = authget(mi, cred);
	if (ch->ch_client->cl_auth == NULL)
		panic("clget: null auth");
	ch->ch_timesused++;

	if (!nfs_client_thread_init) 
		nfs_thread_init();

	return (ch->ch_client);

}

void
clfree(cl)
	CLIENT *cl;
{
	register struct chtab *ch;

	authfree(cl->cl_auth);
	cl->cl_auth = NULL;
	/*
	 * If we're deallocating the pagedaemon's allotted handle, just hang
	 * onto it instead.
	 */
	if (ch_pagedaemon != (struct chtab *)NULL) {
		if (cl == ch_pagedaemon->ch_client) {
			ch_pagedaemon->ch_inuse = FALSE;
			if (u.u_procp != &proc[2])
				panic("clfree: paged doesn't own its handle\n");
			return;
		}
	}
	for (ch = chtabhead.ch_next; ch != &chtabhead; ch = ch->ch_next) {
		if (ch->ch_client == cl) {
			if (nchfree >= nchtable) {
				/*
				*  We have enough free client handles,  so
				*  we can simply pop this one back into the
				*  heap where someone else can use the space.
				*/
				ch->ch_prev->ch_next = ch->ch_next;
				ch->ch_next->ch_prev = ch->ch_prev;
				CLNT_DESTROY(cl);
				kmem_free((caddr_t)ch, sizeof (struct chtab));
				nchtotal--;
			} else {
				/*
				*  We don't have enough free client handles,
				*  so leave this one in the list.
				*/
				ch->ch_inuse = FALSE;
				nchfree++;
			}
			return;
		}
	}
	panic("clfree: can't find client\n");
}

/*
 * This table maps from NFS protocol number into call type.
 * Zero means a "Lookup" type call
 * One  means a "Read" type call
 * Two  means a "Write" type call
 * This is used to select a default time-out as given below.
 */
static char call_type[] = {
	0, 0, 1, 0, 0, 0, 1,
	0, 2, 2, 2, 2, 2, 2,
	2, 2, 1, 0 };

/*
 * Minimum time-out values indexed by call type
 * These units are in "eights" of a second to avoid multiplies
 */
static unsigned int minimum_timeo[] = {
	6, 7, 10 };

/*
 * Similar table, but to determine which timer to use
 * (only real reads and writes!)
 */
static char timer_type[] = {
	0, 0, 0, 0, 0, 0, 1,
	0, 2, 0, 0, 0, 0, 0,
	0, 0, 1, 0 };

/*
 * Back off for retransmission timeout, MAXTIMO is in hz of a sec
 */
#define	MAXTIMO	20*hz
#define	backoff(tim)	((((tim) << 1) > MAXTIMO) ? MAXTIMO : ((tim) << 1))

#define	MIN_NFS_TSIZE 512	/* minimum "chunk" of NFS IO */
#define	REDUCE_NFS_TIME (hz/2)	/* rtxcur we try to keep under */
#define	INCREASE_NFS_TIME (hz/3*8) /* srtt we try to keep under (scaled*8) */
/*
 * Function called when the RPC package notices that we are
 * re-transmitting, or when we get a response without retransmissions.
 */
void
nfs_feedback(flag, which, mi)
	int flag;
	int which;
	register struct mntinfo *mi;
{
	int kind = timer_type[which];

	if (flag == FEEDBACK_REXMIT1) {
		/*
		 * The this timer check appears to limit buffer shrinkage only
		 * when two retransmits happen in a row.  On the first call,
		 * rt_rtxcur is the value derived from round trip timings.  On
		 * the second call, it is "backoff(timohz)" and will be more
		 * than twice "REDUCE_NFS_TIME".  I (werme) am preserving this
		 * behaviour solely because it allows the net to occasionally
		 * drop a packet without reducing the buffer size.
		 */
		if (mi->mi_timers[NFS_CALLTYPES].rt_rtxcur != 0 &&
		    mi->mi_timers[NFS_CALLTYPES].rt_rtxcur < REDUCE_NFS_TIME)
			return;
		if (kind==1 && mi->mi_curread > MIN_NFS_TSIZE) {
			mi->mi_curread /= 2;
			if (mi->mi_curread < MIN_NFS_TSIZE)
				mi->mi_curread = MIN_NFS_TSIZE;
		}
		if (kind==2 && mi->mi_curwrite > MIN_NFS_TSIZE) {
			mi->mi_curwrite /= 2;
			if (mi->mi_curwrite < MIN_NFS_TSIZE)
				mi->mi_curwrite = MIN_NFS_TSIZE;
		}
	} else if (flag == FEEDBACK_OK) {
		if (kind == 0) return;
#ifdef notdef
		/*
		 * This timer check appears to be totally bogus.  It disallows
		 * buffer shrinkage on slow networks to ever be undone.  So,
		 * in a case where big buffers are vital for any kind of
		 * decent performance, a few retransmits will force buffer
		 * sizes down and this would keep them down.  Blech!
		 */
		if (mi->mi_timers[kind].rt_srtt >= INCREASE_NFS_TIME)
			return;
#endif
		if (kind==1) {
			if (mi->mi_curread >= mi->mi_tsize)
				return;
			mi->mi_curread +=  MIN_NFS_TSIZE;
			if (mi->mi_curread > mi->mi_tsize/2)
				mi->mi_curread = mi->mi_tsize;
		}
		if (kind==2) {
			if (mi->mi_curwrite >= mi->mi_stsize)
				return;
			mi->mi_curwrite += MIN_NFS_TSIZE;
			if (mi->mi_curwrite > mi->mi_stsize/2)
				mi->mi_curwrite = mi->mi_stsize;
		}
	}
}

int
rfscall(mi, which, xdrargs, argsp, xdrres, resp, cred)
	register struct mntinfo *mi;
	int	 which;
	xdrproc_t xdrargs;
	caddr_t	argsp;
	xdrproc_t xdrres;
	caddr_t	resp;
	struct ucred *cred;
{
	CLIENT *client;
	register enum clnt_stat status;
	struct rpc_err rpcerr;
	struct timeval wait;
	struct ucred *newcred;
	int timeo;		/* in units of HZ, 20ms. */
	int user_told, count;
	bool_t tryagain;
	u_int xid;

	clstat.ncalls++;
	clstat.reqs[which]++;

	rpcerr.re_errno = 0;
	rpcerr.re_status = RPC_SUCCESS;
	newcred = NULL;
	user_told = 0;
retry:
	client = clget(mi, cred);
	xid = alloc_xid();
	timeo = clntkudp_settimers(client,
	    &(mi->mi_timers[timer_type[which]]),
	    &(mi->mi_timers[NFS_CALLTYPES]),
	    (minimum_timeo[call_type[which]]*hz)>>3,
	    mi->mi_dynamic ? nfs_feedback : (void (*)()) 0, (caddr_t)mi, xid);


	/*
	 * If hard mounted fs, retry call forever unless hard error occurs.
	 * If interruptable, need to count retransmissions at this level.
	 */
	if (mi->mi_int && mi->mi_hard)
		count = mi->mi_retrans;
	else
		count = 1;
	do {
		tryagain = FALSE;

		wait.tv_sec = timeo / hz;
		wait.tv_usec = 1000000/hz * (timeo % hz);
		status = CLNT_CALL(client, which, xdrargs, argsp,
		    xdrres, resp, wait);
		switch (status) {
		case RPC_SUCCESS:
			break;

		/*
		 * Unrecoverable errors: give up immediately
		 */
		case RPC_AUTHERROR:
		case RPC_CANTENCODEARGS:
		case RPC_CANTDECODERES:
		case RPC_VERSMISMATCH:
		case RPC_PROGVERSMISMATCH:
		case RPC_PROGUNAVAIL:
		case RPC_PROCUNAVAIL:
		case RPC_CANTDECODEARGS:
			break;

		default:
			if (status == RPC_INTR) {
				tryagain = (bool_t)(mi->mi_hard && !mi->mi_int);
				if (tryagain) 
					continue;
				rpcerr.re_status = RPC_INTR;
				rpcerr.re_errno = EINTR;
			} else 
				tryagain = (bool_t)mi->mi_hard; 

			if (tryagain) {
				timeo = backoff(timeo);
				if (--count > 0 && timeo < hz*15)
					continue;
				if (!mi->mi_printed) {
					mi->mi_printed = 1;
	aprintf("NFS server %s not responding still trying\n", mi->mi_hostname);
				}
				if (!user_told && (u.u_procp->p_flag & SCTTY)) {
					user_told = 1;
	uprintf("NFS server %s not responding still trying\n", mi->mi_hostname);
				}
				if (mi->mi_dynamic) {
					/*
					 * If dynamic retransmission is on,
					 * return back to the vnode ops level
					 * on read or write calls
					 */
					if (timer_type[which] != 0) {
						clfree(client);
						if (newcred) {
							crfree(newcred);
						}
						return (ENFS_TRYAGAIN);
					}
				}
			}
		}
	} while (tryagain);

	if (status != RPC_SUCCESS) {
		clstat.nbadcalls++;
		mi->mi_down = 1;
		if (status != RPC_INTR) {
			CLNT_GETERR(client, &rpcerr);
				aprintf("NFS %s failed for server %s: %s\n",
				rfsnames[which], mi->mi_hostname,
				clnt_sperrno(status));
			if (u.u_procp->p_flag & SCTTY) {
				uprintf("NFS %s failed for server %s: %s\n",
					rfsnames[which], mi->mi_hostname,
					clnt_sperrno(status));
			}
		}
	} else if (resp && *(int *)resp == EACCES &&
	    newcred == NULL && cred->cr_uid == 0 && cred->cr_ruid != 0) {
		/*
		 * Boy is this a kludge!  If the reply status is EACCES
		 * it may be because we are root (no root net access).
		 * Check the real uid, if it isn't root make that
		 * the uid instead and retry the call.
		 */
		newcred = crdup(cred);
		cred = newcred;
		cred->cr_uid = cred->cr_ruid;
		clfree(client);
		goto retry;
	} else if (mi->mi_hard) {
		if (mi->mi_printed) {
			aprintf("NFS server %s ok\n", mi->mi_hostname);
			mi->mi_printed = 0;
		}
		if (user_told && (u.u_procp->p_flag & SCTTY)) {
			uprintf("NFS server %s ok\n", mi->mi_hostname);
		}
	} else {
		mi->mi_down = 0;
	}

	clfree(client);
	if (newcred) {
		crfree(newcred);
	}
	/*
	 *      This should never happen, but we suspect something is
	 *      garbaging packets, so ...
	 */
	if (rpcerr.re_status != RPC_SUCCESS && rpcerr.re_errno == 0) {
		printf("rfscall: re_status %d, re_errno 0\n", rpcerr.re_status);
		panic("rfscall");
		/* NOTREACHED */
	}
	return (rpcerr.re_errno);
}

nfs_thread_init()
{
	extern  int nfs_timer();

	nfs_client_thread_init++;
	net_threadstart(nfs_timer, 0);
}

int
nfs_timer()
{
	int error;
	DOMAIN_FUNNEL_DECL(f)
	struct chtab *ch;	
	struct socket *so;
	struct proc   *p;
	struct cku_private *cku;


	for (ch = chtabhead.ch_next; ch != &chtabhead; ch = ch->ch_next) {

		if (ch->ch_inuse != TRUE)
			continue;

		if (ch->ch_int != TRUE)
			continue;

		if (ch->ch_client == NULL)
			continue;

		cku = (struct cku_private *) ch->ch_client->cl_private;
		so = cku->cku_sock;
		p = ch->ch_proc;

		DOMAIN_FUNNEL(sodomain(so), f);
		SOCKET_LOCK(so);
		if (p && p->p_sig &&
		    (((p->p_sig &~ p->p_sigmask) &~ p->p_sigignore) & NFSINT_SIGMASK)) {
			so->so_error = EINTR;
			sorwakeup(so);
		}
		SOCKET_UNLOCK(so);
		DOMAIN_UNFUNNEL(f);
		if (ch->ch_next == ch)
			break;
	}

	return (hz/NFS_HZ);
}

vattr_to_sattr(vap, sa)
	register struct vattr *vap;
	register struct nfssattr *sa;
{

#ifdef OLD
	sa->sa_mode = vap->va_mode;
	sa->sa_uid = (int)vap->va_uid;
	sa->sa_gid = (int)vap->va_gid;
#else
        if (vap->va_mode == (unsigned short) -1)
                sa->sa_mode = (unsigned int) -1;
        else
                sa->sa_mode = vap->va_mode;

        if (vap->va_uid == (uid_t) -1)
                sa->sa_uid = (unsigned int) -1;
        else
                sa->sa_uid = vap->va_uid;

        if (vap->va_gid == (gid_t) -1)
                sa->sa_gid = (unsigned int) -1;
        else
                sa->sa_gid = vap->va_gid;

#endif
	sa->sa_size = vap->va_size;
	sa->sa_atime.tv_sec  = vap->va_atime.tv_sec;
	sa->sa_atime.tv_usec = vap->va_atime.tv_usec;
	sa->sa_mtime.tv_sec  = vap->va_mtime.tv_sec;
	sa->sa_mtime.tv_usec = vap->va_mtime.tv_usec;
}

setdiropargs(da, ndp)
	struct nfsdiropargs *da;
	struct nameidata *ndp;
{
	da->da_fhandle = *vtofh(ndp->ni_dvp);
	if ((ndp->ni_nameiop & HASBUF) ^ HASBUF) {
		da->da_name = ndp->ni_dent.d_name;
		da->da_len = ndp->ni_dent.d_namlen;
	} else {
		da->da_name = ndp->ni_ptr;
		da->da_len = ndp->ni_namelen;
	}
}

int
setdirgid(dvp)
	struct vnode *dvp;
{

	/*
	 * To determine the expected group-id of the created file:
	 * Sun algorithm:
	 *  1)	If the filesystem was not mounted with the Old-BSD-compatible
	 *	GRPID option, and the directory's set-gid bit is clear,
	 *	then use the process's gid.
	 *  2)	Otherwise, set the group-id to the gid of the parent directory.
	 *
	 * DEC algorithm: The same, except that we test global variable
	 * sys_v_mode instead of the GRPID option which we didn't implement.
	 */
	if (sys_v_mode && !(vtor(dvp)->r_attr.va_mode & S_ISGID))
		return ((int)u.u_gid);
	else
		return ((int)vtor(dvp)->r_attr.va_gid);
}

u_int
setdirmode(dvp, om)
	struct vnode *dvp;
	u_int om;
{

	/*
	 * Modify the expected mode (om) so that the set-gid bit matches
	 * that of the parent directory (dvp).
	 */
	om &= ~S_ISGID;
	if (vtor(dvp)->r_attr.va_mode & S_ISGID)
		om |= S_ISGID;
	return (om);
}

struct rnode *rpfreelist = NULL;
int rnhash;

/*
 * Return a vnode for the given fhandle.
 * If no rnode exists for this fhandle create one and put it
 * in a table hashed by fh_fsid and fs_fid.  If the rnode for
 * this fhandle is already in the table return it (ref count is
 * incremented by rfind.  The rnode will be flushed from the
 * table when nfs_inactive calls runsave.
 */
struct vnode *
makenfsnode(fh, attr, mp, vap)
	nfsv2fh_t *fh;
	struct nfsfattr *attr;
	struct mount *mp;
	struct vattr *vap;
{
	register struct rnode *rp;
	struct vnode *vp = (struct vnode *)0;
	char newnode = 0;

	if ((rp = rfind(fh, mp)) == NULL) {
		register struct rnode *trp;

		if (getnewvnode(VT_NFS, &nfs_vnodeops, &vp))
			return((struct vnode *)0);
		bzero((caddr_t)(rp = vtor(vp)), sizeof (*rp));
		rp->r_fh = *fh;
		rp->r_vnode = vp;
		vp->v_mount = mp;
		rtov(rp)->v_mount = mp;
		if ((trp = rfind(fh, mp)) != NULL) {
			vrele(vp);
			rp = trp;
			vp = rtov(rp);
		} else {
			rp_addhash(rp);
			newnode++;
		}
	} else 
		vp = rtov(rp);

	if (attr) {
		extern struct vnodeops spec_nfsv2nodeops, fifo_nfsv2nodeops;

		ASSERT(vp != (struct vnode *)0);
		switch (vp->v_type = n2v_type(attr)) {
		case VCHR :
		case VBLK :
			vp->v_op = &spec_nfsv2nodeops;
			if (specalloc(vp, n2v_rdev(attr))) {
				vrele(vp);
				return((struct vnode *)0);
			}
			break;
		case VSOCK :
			if (vap && vap->va_socket) /* create */
				vp->v_socket = (struct socket *)vap->va_socket;
			break;
		case VFIFO :
			vp->v_op = &fifo_nfsv2nodeops;
			break;
		default :
			vp->v_op = &nfs_vnodeops;
			break;
		}
		if (!newnode)
			nfs_cache_check(vp, attr->na_mtime, attr->na_size);
		nfs_attrcache(vp, attr);
	}
	if (newnode)
		insmntque(vp, mp);
	return(vp);
}

/*
 * Rnode lookup stuff.
 * These routines maintain a table of rnodes hashed by fhandle so
 * that the rnode for an fhandle can be found if it already exists.
 * NOTE: RTABLESIZE must be a power of 2 for rtablehash to work!
 */

#define	RTABLESIZE	64
#define	rtablehash(fh) \
    ((fh->fh_bytes[2] ^ fh->fh_bytes[5] ^ fh->fh_bytes[15]) & (RTABLESIZE-1))

struct rnode *rtable[RTABLESIZE];

/*
 * Put a rnode in the hash table
 */
static void
rp_addhash(rp)
	struct rnode *rp;
{

	rp->r_hash = rtable[rtablehash(rtofh(rp))];
	rtable[rtablehash(rtofh(rp))] = rp;
	rnhash++;
	vtomi(rtov(rp))->mi_refct++;
}

/*
 * Remove a rnode from the hash table
 */
void
rp_rmhash(rp)
	struct rnode *rp;
{
	register struct rnode *rt;
	register struct rnode *rtprev = NULL;

	rt = rtable[rtablehash(rtofh(rp))];
	while (rt != NULL) {
		if (rt == rp) {
			if (rtprev == NULL) {
				rtable[rtablehash(rtofh(rp))] = rt->r_hash;
			} else {
				rtprev->r_hash = rt->r_hash;
			}
			rnhash--;
			vtomi(rtov(rp))->mi_refct--;
			return;
		}
		rtprev = rt;
		rt = rt->r_hash;
	}
}

/*
 * free resource for rnode
 */
rinactive(rp)
	struct rnode *rp;
{
	if (rp->r_cred) {
		crfree(rp->r_cred);
		rp->r_cred = NULL;
	}
}

/*
 * Put an rnode on the free list.
 * The rnode has already been removed from the hash table.
 * If there are no pages on the vnode remove inactivate it,
 * otherwise put it back in the hash table so it can be reused
 * and the vnode pages don't go away.
 */
rfree(rp)
	register struct rnode *rp;
{
	rinactive(rp);
}

/*
 * Lookup a rnode by fhandle.
 */
static struct rnode *
rfind(fh, mp)
	nfsv2fh_t *fh;
	struct mount *mp;
{
	register struct rnode *rt;

 again:
	rt = rtable[rtablehash(fh)];
	while (rt != NULL) {
		if (((rt->r_flags & RFREEING) == 0) &&
		    bcmp((caddr_t)rtofh(rt), (caddr_t)fh, NFS_FHSIZE) == 0 &&
		    mp == rtov(rt)->v_mount) {
			if (vget(rtov(rt)))
				goto again; /* Slept, list may have changed */
			else
				return (rt);
		}
		rt = rt->r_hash;
	}
	return (NULL);
}

/*ARGSUSED*/
rinval(mp)
	struct mount *mp;
{
	register struct rnode **rpp, *rp, *rhash;
	register struct vnode *vp;

	for (rpp = rtable; rpp < &rtable[RTABLESIZE]; rpp++) {
		for (rp = *rpp; rp != (struct rnode *) NULL; rp = rhash) {
			/*
			 *	We must save the previous hash value since
			 *	rp_addhash will change it.
			 */
			rhash = rp->r_hash;
			vp = rtov(rp);
			if (vp->v_mount == mp) {
				rp_rmhash(rp);
				cache_purge(vp);
				/*
				 *	If ours is not the only reference to
				 *	this rnode, put it back in the rtable.
				 */
				if (vp->v_usecount > 0)
					rp_addhash(rp);
			}
		}
	} 
}

newname(news)
	char *news;
{
	register char *s;
	register u_int id;
	static u_int newnum = 0;
	extern char *strcpy();

	if (newnum == 0) {
		newnum = time.tv_sec & 0xffff;
	}

	s = strcpy(news, ".nfs");
	id = newnum++;
	while (id != 0) {
		*s++ = "0123456789ABCDEF"[id & 0x0f];
		id >>= 4;
	}
	*s = '\0';
}

rlock(rp)
	register struct rnode *rp;
{

	RLOCK(rp);
}

runlock(rp)
	register struct rnode *rp;
{

	RUNLOCK(rp);
}

#define NFS_SMALL       1
#define NFS_MEDIUM      2
#define NFS_LARGE       3
#define NFS_XLARGE      4


nfs_system_size()
{
        extern int maxusers;

        if (maxusers < 32)
                return (NFS_SMALL);

        if (maxusers < 128)
                return (NFS_MEDIUM);

        if (maxusers < 256)
                return (NFS_LARGE);

        return (NFS_XLARGE);
}

struct kexport *exported;

/*
 * Exportfs system call
 * TODO: make smp safe
 */
nfs_exportfs(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	register struct args {
		short option;	/* 1=create, 2=delete, 3=read */
		u_int *cookie;
		struct exportfsdata *buf;
	} *uap = (struct args *)args;
	register struct vnode *vp=NULL;
	register int error;
	register struct nameidata *ndp = &u.u_nd;
	struct kexport *ep, **tail, *tmp, *tmp2;
	struct exportfsdata *e;
	int cookie, i, allocsize;


#ifdef notdef
printf("exportfs: option=%x\n", uap->option);
#endif

#if SEC_BASE
	/*
	 * Must have the mount privilege.
	 */
	if (!privileged(SEC_MOUNT, EPERM))
		return (EPERM);
#else
	if (error = suser(u.u_cred, &u.u_acflag))
		return (error);
#endif

	e = (struct exportfsdata *) kalloc(sizeof(struct exportfsdata));
	if (e == NULL) 
		return(EIO);
	if (uap->option != EXPORTFS_READ) {
		if (uap->buf != NULL) {
			error = copyin((caddr_t)uap->buf, (caddr_t)e,
					   sizeof (struct exportfsdata));
			if (error) {
				kfree(e, sizeof (struct exportfsdata));
				return(error);
			}
			
		}
		else {
			kfree(e, sizeof (struct exportfsdata));
			return(EINVAL);
		}
	}
	switch (uap->option) {

	case EXPORTFS_CREATE:


		ndp->ni_dirp = (caddr_t) kalloc(MAXPATHLEN);
		if (ndp->ni_dirp == NULL) {
			kfree(e, sizeof (struct exportfsdata));
			return (EIO);
		}

		bcopy(e->e_path, ndp->ni_dirp, MAXPATHLEN);

		ndp->ni_nameiop = LOOKUP | FOLLOW;
		ndp->ni_segflg = UIO_SYSSPACE;
		error = namei(ndp);
		if (error) {
			kfree(e, sizeof (struct exportfsdata));
			return(error);
		}
		kfree(ndp->ni_dirp, MAXPATHLEN);
		vp = ndp->ni_vp;
		if (vp == NULL) {
			/* Should this be ENOENT ? */
			kfree(e, sizeof (struct exportfsdata));
			/* DO I have to vrele anything ? */
			return(ENOENT);
		}

		/* Only check for NFS fs type for now */
		if (vp->v_mount->m_stat.f_type == MOUNT_NFS) {
			vrele(vp);
			kfree(e, sizeof (struct exportfsdata));
			return(EREMOTE); /* can't export NFS fs */
		}

		ep = (struct kexport *) kalloc(EXPORT_SIZE);
		if (ep == NULL) {
			vrele(vp);
			kfree(e, sizeof (struct exportfsdata));
			return(EIO);
		}
		ep->e_pathlen = strlen(e->e_path)+1;
		ep->e_path = kalloc(ep->e_pathlen);
		if (ep->e_path == NULL) {
			vrele(vp);
			kfree(e, sizeof (struct exportfsdata));
			kfree(ep, EXPORT_SIZE);
			return(EIO);
		}
		bcopy(e->e_path, ep->e_path, ep->e_pathlen);

		ep->e_fsid = vp->v_mount->m_stat.f_fsid;
		VFS_VPTOFH(vp, &ep->e_fid, error);

		if (error) {
			vrele(vp);
			kfree(e, sizeof (struct exportfsdata));
			exportfree(ep);
			return(error);
		}

		/*
		 * Kalloc space for any root addrs and write addrs and 
		 * load them from the exportfsdata array.
		 */
		ep->e_rootaddrs.naddrs = e->e_rootaddrs.naddrs;
		allocsize = ep->e_rootaddrs.naddrs * sizeof(struct sockaddr);
		if (allocsize > 0) {
        		ep->e_rootaddrs.addrvec = 
				(struct sockaddr *) kalloc(allocsize);
			if (ep->e_rootaddrs.addrvec == NULL) {
				vrele(vp);
				kfree(e, sizeof (struct exportfsdata));
				exportfree(ep);
				return(EIO);
			}
			bcopy(e->e_rootaddrs.addrvec, ep->e_rootaddrs.addrvec,
				allocsize);
		} else {
			ep->e_rootaddrs.addrvec = NULL;
		}

		ep->e_writeaddrs.naddrs = e->e_writeaddrs.naddrs;
		allocsize = ep->e_writeaddrs.naddrs * sizeof(struct sockaddr);
		if (allocsize > 0) {
        		ep->e_writeaddrs.addrvec = 
				(struct sockaddr *) kalloc(allocsize);
			if (ep->e_writeaddrs.addrvec == NULL) {
				vrele(vp);
				kfree(e, sizeof (struct exportfsdata));
				exportfree(ep);
				return(EIO);
			}
			bcopy(e->e_writeaddrs.addrvec, ep->e_writeaddrs.addrvec,
				allocsize);
		} else {
			ep->e_writeaddrs.addrvec = NULL;
		}
		
	     	/* Set root and anon uid mapping */
		ep->e_rootmap = e->e_rootmap;
		ep->e_anon = e->e_anon;
		ep->e_flags = e->e_flags | M_EXPORTED;

		/* the kernel does not use these but user level programs
		 * i.e. the mountd, expect them
		 */
		ep->e_dev = e->e_dev;
		ep->e_ino = e->e_ino;
		ep->e_gen = e->e_gen;

		kfree(e, sizeof (struct exportfsdata));

		/*
		 * Commit the new information to the export list, making
		 * sure to delete the old entry for the fs, if one exists.
		 */
#ifdef notdef
		smp_lock(&lk_gnode, LK_RETRY);
#endif
		tail = &exported;
		tmp=NULL;
		while (*tail != NULL) {
			if ( fsid_equal(&(*tail)->e_fsid, &ep->e_fsid) &&
			     fid_equal(&(*tail)->e_fid, &ep->e_fid)) {
 
				(*tail)->e_rootmap = ep->e_rootmap;
				(*tail)->e_anon = ep->e_anon;
				(*tail)->e_flags = ep->e_flags;

				(*tail)->e_rootaddrs.naddrs = 
					ep->e_rootaddrs.naddrs;
				(*tail)->e_rootaddrs.addrvec = 
					ep->e_rootaddrs.addrvec;
				ep->e_rootaddrs.naddrs = 0;

				(*tail)->e_writeaddrs.naddrs = 
					ep->e_writeaddrs.naddrs;
				(*tail)->e_writeaddrs.addrvec = 
					ep->e_writeaddrs.addrvec;
				ep->e_writeaddrs.naddrs = 0;
#ifdef notdef
				smp_unlock(&lk_gnode);
#endif
				exportfree(ep);
				vrele(vp);
				return(0);

			} else {
				tmp=(*tail);
				tail = &(*tail)->e_next;
			}
		}
		ep->e_next = exported;
		exported = ep;
#ifdef notdef
		smp_unlock(&lk_gnode);
#endif
		vrele(vp);
		return(0);


	case EXPORTFS_REMOVE:

#ifdef notdef
		smp_lock(&lk_gnode, LK_RETRY);
#endif
		tail = &exported;
		tmp=NULL;
		while (*tail != NULL) {
			if (strcmp((*tail)->e_path,e->e_path) == 0) {
				if (tmp != NULL) {
					tmp2 = *tail;
					tmp->e_next = (*tail)->e_next;
#ifdef notdef
					smp_unlock(&lk_gnode);
#endif
					kfree(e, sizeof (struct exportfsdata));
					exportfree(tmp2);
					return (0);
				}
				else {
					tmp = *tail;
					exported = (*tail)->e_next;
#ifdef notdef
					smp_unlock(&lk_gnode);
#endif
					kfree(e, sizeof (struct exportfsdata));
					exportfree(tmp);
					return (0);
				}
			} else {
				tmp=(*tail);
				tail = &(*tail)->e_next;
			}
		}
#ifdef notdef
		smp_unlock(&lk_gnode);
#endif
		kfree(e, sizeof (struct exportfsdata));
		return(ENOENT);

	case EXPORTFS_READ:

		if (error = copyin(uap->cookie, &cookie, 
				       sizeof(cookie))) {
			kfree(e, sizeof (struct exportfsdata));
			return(error);
		}
			
		if (exported == NULL) {
			kfree(e, sizeof (struct exportfsdata));
			return(ENOENT);
		}
		if (cookie < 0) {
			kfree(e, sizeof (struct exportfsdata));
	                return(EINVAL);
		}
		i=0;

#ifdef notdef
		smp_lock(&lk_gnode, LK_RETRY);
#endif
		ep = exported;
		while (ep != NULL) {
			if (i++ == cookie) {
				e->e_flags = ep->e_flags;
				e->e_rootmap = ep->e_rootmap;
				e->e_anon = ep->e_anon;
				bcopy(ep->e_path, e->e_path, ep->e_pathlen);
				e->e_dev = ep->e_dev;
				e->e_ino = ep->e_ino;
				e->e_gen = ep->e_gen;

				/* Fill root and write addr arrays */
				e->e_rootaddrs.naddrs = ep->e_rootaddrs.naddrs;
				bcopy(ep->e_rootaddrs.addrvec,
				      e->e_rootaddrs.addrvec,
				      e->e_rootaddrs.naddrs * 
					sizeof(struct sockaddr));
				e->e_writeaddrs.naddrs= ep->e_writeaddrs.naddrs;
				bcopy(ep->e_writeaddrs.addrvec,
				      e->e_writeaddrs.addrvec,
				      e->e_writeaddrs.naddrs * 
					sizeof(struct sockaddr));

				if (ep->e_next == NULL)
					e->e_more = 0;
				else
					e->e_more = 1;
				break;
			} else {
				ep = ep->e_next;
			}
		}
#ifdef notdef
		smp_unlock(&lk_gnode);
#endif

		cookie=i;
		copyout(&cookie, uap->cookie, sizeof(cookie));

		copyout((caddr_t)e, (caddr_t) uap->buf, 
			sizeof(struct exportfsdata));

			kfree(e, sizeof (struct exportfsdata));
		return(0);
	
	default:
		return(EINVAL);
		break;
	}
	return EINVAL;		/* Shaddup, lint! */
}


/*
 * Free an entire export list node
 */
exportfree(exi)
	struct kexport *exi;
{
	kfree(exi->e_path, exi->e_pathlen);
	if (exi->e_rootaddrs.naddrs)
		kfree(exi->e_rootaddrs.addrvec, exi->e_rootaddrs.naddrs *
			sizeof(struct sockaddr));
	if (exi->e_writeaddrs.naddrs)
		kfree(exi->e_writeaddrs.addrvec, exi->e_writeaddrs.naddrs *
			sizeof(struct sockaddr));
	kfree(exi, EXPORT_SIZE);
}

#ifdef CJXXX
/*
 * Find the export entry that corresponds to a file handle
 * SMP:  need export list locked.
 */
void
fhtoexport(fhp, xpd)
	nfsv2fh_t *fhp;
	struct exportfsdata *xpd;
{
	register struct kexport *ep;

	ep = exported;
	xpd->e_flags = 0;
	while (ep != NULL) {
#ifdef NFSDEBUG
		printf("fhtoexport: looking for match: %s\n", ep->e_path);
		fsid_print(&ep->e_fsid);
		fsid_print(&fhp->fh_fsid);
		fid_print(&ep->e_fid);
		fid_print(&fhp->fh_efid);
#endif
	    if ( /* export entry id matches fhandle export id */
		fsid_equal(&ep->e_fsid, &fhp->fh_fsid)  &&
		fid_equal(&ep->e_fid, &fhp->fh_efid) )  {
			xpd->e_flags = ep->e_flags;
			xpd->e_rootmap = ep->e_rootmap;
			xpd->e_anon = ep->e_anon;
			xpd->e_rootaddrs.naddrs = ep->e_rootaddrs.naddrs;
			bcopy(ep->e_rootaddrs.addrvec, xpd->e_rootaddrs.addrvec,
			      ep->e_rootaddrs.naddrs * sizeof(struct sockaddr));
			xpd->e_writeaddrs.naddrs = ep->e_writeaddrs.naddrs;
			bcopy(ep->e_writeaddrs.addrvec, 
			      xpd->e_writeaddrs.addrvec,
			      ep->e_writeaddrs.naddrs * 
				sizeof(struct sockaddr));
			fid_copy(&ep->e_fid, &xpd->x_fid);
			return;
	    } else 
		ep = ep->e_next;
	}
#ifdef NFSDEBUG
	printf("no match found\n");
#endif
	return;
}
#endif /* CJXXX */
