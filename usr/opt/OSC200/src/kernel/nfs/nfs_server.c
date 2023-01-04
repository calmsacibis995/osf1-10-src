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
static char *rcsid = "@(#)$RCSfile: nfs_server.c,v $ $Revision: 1.1.32.11 $ (DEC) $Date: 1993/12/15 21:49:59 $";
#endif

/*
 * Replacement for OSF/NFS server code built from ONC/NFS + ULTRIX/NFS code.
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <dirent.h>
#include <sys/mbuf.h>
#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <nfs/nfssys.h>
#include <ufs/inode.h>
#include <ufs/dir.h>
#include <sys/presto.h>
#include <sys/prestoioctl.h>
#include <sys/fs_types.h>

#include <kern/zalloc.h>
#include <kern/mfs.h>

#include <sys/uio.h>
#include <sys/ucred.h>
#include <sys/namei.h>
#include <sys/errno.h>

#ifdef CJKTRACE
#include <sys/logsys.h>

long nfs_cjtrace = 0;
#endif /* CJKTRACE */

/*
 * Mask some of the security differences.
 */
#if	SEC_BASE
#define	PRIV_SUSER(cr,acflag,priv,code,retcode,error)			\
	error = !privileged(priv,code);   				\
	if (error)							\
		return (retcode);
#else
#define	PRIV_SUSER(cr,acflag,priv,code,retcode,error)			\
	if (error = suser((cr),(acflag)))				\
		return (error);
#define	SEC_REMOTE	0
#endif /* SEC_BASE */

/* struct nfsstats nfsstats;
vdecl_simple_lock_data(,nfs_stats_lock)
*/

nfs_init()
{
	nfs_syscall_config();
	return (0);
}

/* #define DEBUG */

#ifdef	DEBUG
extern int rrokdebug;
int nfsdebug = 0;
int nfsrefdebug = 0;
int nfsopdebug[RFS_NPROC];
/* 0=null, 1=getattr, 2=setattr, 3=unused, 4=lookup, 5=readlink, 6=read,
 * 7=unused, 8=write, 9=create, 10=remove, 11=rename, 12=link, 13=symlink,
 * 14=mkdir, 15=rmdir, 16=readdir, 17=fsstat
 */

long nfssvdebug = 0;
long nfs_sv_debug = 0;
long nfs_cl_debug = 0;
long nfs_xprt_debug = 0;
long nfs_mbuf_debug = 0;
long nfs_lmbuf_debug = 0;
long nfs_dup_debug = 0;
long nfs_order_debug = 0;

#define DPRINTF(c, args)	if(c) printf args
#else
#define DPRINTF(c, args)
#endif

/*
 * Perform access checking for vnodes obtained from file handles that would
 * refer to files already opened by a Unix client. You cannot just use
 * vn_writechk() and VOP_ACCESS():
 *     The owner is to be given access irrespective of mode bits so that
 *     processes that chmod after opening a file don't break. I don't like
 *     this because it opens a security hole, but since the nfs server opens
 *     a security hole the size of a barn door anyhow, what the heck.
 */
nfsrv_access(vp, flags, cred)
	struct vnode *vp;
	int flags;
	struct ucred *cred;
{
	struct vattr vattr;
	int error;
	if (flags & VWRITE) {
		DPRINTF(nfsdebug,("access(w): cruid %d\n",(uid_t)cred->cr_uid));
		VOP_GETATTR(vp, &vattr, cred, error);
		DPRINTF(nfsdebug, ("access(w): iuid %d\n", vattr.va_uid));
		if (error) {
			DPRINTF(nfsdebug,
				("access(w): getattr returns %d\n", error));
			return (error);
		}
		if ((uid_t)cred->cr_uid == (uid_t)vattr.va_uid)
			return (0);
	} else {
		VOP_GETATTR(vp, &vattr, cred, error);
		if (error) {
			DPRINTF(nfsdebug,
				("access(r): getattr returns %d\n", error));
			return (error);
		}
		if ((uid_t)cred->cr_uid == (uid_t)vattr.va_uid)
			return (0);
	}
	VOP_ACCESS(vp, flags, cred, error);
	DPRINTF(error && nfsdebug, ("access: access returns %d\n", error));
	return (error);
}

int nfs_throwaways[RFS_NPROC];
int nfs_nfsds = 0;		/* number of running NFS daemon processes */
int nfs_max_daemon = 0;		/* maximum NFS daemon number */

char *rfsnames[] = {
	"null", "getattr", "setattr", "unused", "lookup", "readlink", "read",
	"unused", "write", "create", "remove", "rename", "link", "symlink",
	"mkdir", "rmdir", "readdir", "fsstat" };

/*
 * rpc service program version range supported
 */
#define	VERSIONMIN	2
#define	VERSIONMAX	2

/*
 * Returns true if filesystem for a given vp is
 * (mounted read-only) or (vp is not a socket or a block 
 * or character device resident on the file system) and
 * (the filesystem is exported read-only).
 */

#define rdonly(vp, xpd, xprt) \
      ((vp->v_mount->m_flag & M_RDONLY) || \
      ((vp)->v_type != VSOCK && (vp)->v_type != VCHR && \
       (vp)->v_type != VBLK && (vp)->v_type != VFIFO) && \
      (((xpd)->e_flags & M_EXRDONLY) || \
       (((xpd)->e_flags & M_EXRDMOSTLY) && (!hostinlist((struct sockaddr *)\
                                 svc_getcaller(xprt), &(xpd)->e_writeaddrs)))))


struct file	*getsock();
void		svcerr_progvers();
int		rfs_dispatch();

extern struct kexport *exported;

/* struct lock_t lk_nfsargs; */
/* extern struct lock_t lk_gnode; */
/* extern struct lock_t lk_nfsstat; */
/* extern struct lock_t lk_nfsbiod; */	/* used for count of biods and nfsds */

struct {
        int     ncalls;         /* number of calls received */
        int     nbadcalls;      /* calls that failed */
        int     reqs[32];       /* count for each request */
} svstat;

int nfs_wakeupone = 1;

int nfs_socket_size = 0;

/* nfsd write synchronization stuff */
struct nfs_svattr {
	int	error;
	int	ref;
	struct	nfsfattr nattr;
};
#define N_NFS_SV_IOV 10
struct nfs_svinfo {
	pid_t			nfs_sv_pid;	/* pid of nfsd */
	int			nfs_sv_nfsd;	/* nfsd # */
	int			nfs_sv_which;	/* type of request */
	int			nfs_sv_state;	/* state that request is in */
	nfsv2fh_t *		nfs_sv_fh;	/* primary fh of interest */
	struct vnode	       *nfs_sv_vp;	/* primary vnode of interest */
	int			nfs_sv_start;	/* write start offset */
	int			nfs_sv_stop;	/* write stop offset */
	SVCXPRT		       *nfs_sv_xprt;	/* RPC xprt */
	struct iovec nfs_sv_iovp[N_NFS_SV_IOV]; /* iovecs for write mbufs */
};

struct nfs_svinfo nfs_sva[MAXNFSDS];

struct socket * nfs_sv_socket;
int	nfs_sv_cc_hist[32];
int	nfs_sv_hist_err;
int	nfs_sv_active;
int	nfs_sv_active_hist[MAXNFSDS];

#define PNFS_SV_HACK (u.u_spare[0])
#define PNFS_SV ((struct nfs_svinfo *)PNFS_SV_HACK)

#define NFSSV_IDLE	0	/* doing nothing */
#define NFSSV_STARTED	1	/* out of dispatch */
#define NFSSV_FHTOVP	2	/* going for vnode; maybe sleeping already */
#define NFSSV_DATA1	3	/* have vnode, doing data portion */
#define NFSSV_META	4	/* have vnode, doing meta-data portion */
#define NFSSV_SLEEP	5	/* have vnode, waiting before 2nd look for
				 *  a following request */ 
#define NFSSV_METADONE	6	/* have vnode, meta-data is done */
#define NFSSV_DATA2	7	/* have vnode, doing data portion */
#define NFSSV_DATA3	8	/* have vnode, doing data portion */
#define NFSSV_REPLY	9	/* no vnode, in dispatch sending reply */

/* NFS request struct */
struct nfs_req {
	struct nfs_req *nrq_next;
	u_int		nrq_xid;
	u_int		nrq_proc;
	u_int		nrq_vers;
	u_int		nrq_addr;
	struct rfsdisp *nrq_disp;
	struct ucred   *nrq_holdcred;
	SVCXPRT	       *nrq_xprt;
	caddr_t	        nrq_args;
	caddr_t	        nrq_res;
	int		nrq_busyxid;
};

/* NFS request struct free list */
struct nfs_req *nfs_reqfree = NULL;

int rfs_nreqs = 0;

/*
 * Get an NFS request struct; make one if necessary.
 * The returned struct is *always* cleared.
 * This should guarantee that next(list tail) should be NULL.
 */
#define RFSGETREQ(req) { \
	/* smp_lock(&lk_nfsargs, LK_RETRY); */ \
	if (nfs_reqfree) { \
		(req) = nfs_reqfree; \
		nfs_reqfree = nfs_reqfree->nrq_next; \
		/* smp_unlock(&lk_nfsargs); */ \
	} else { \
		/* smp_unlock(&lk_nfsargs); */ \
		(req) = (struct nfs_req *) \
			kmem_alloc(sizeof(struct nfs_req)); \
		++rfs_nreqs; \
	} \
	bzero((req), sizeof(struct nfs_req)); \
	}

/*
 * Add an NFS request struct to its free list.
 * NB: assumes that next(head) for an empty list is NULL.
 */
#define RFSPUTREQ(req) { \
	/* smp_lock(&lk_nfsargs, LK_RETRY); */ \
	(req)->nrq_next = nfs_reqfree; \
	nfs_reqfree = (req); \
	/* smp_unlock(&lk_nfsargs); */ \
	}

/* NFS write request struct */
struct nfs_writereq {
	struct nfs_writereq    *nwr_next;
	struct nfs_req 	       *nwr_nrq;
	nfsv2fh_t 	       *nwr_fh;		/* primary fh of interest */
	struct vnode	       *nwr_vp;		/* primary vnode of interest */
	int			nwr_start;	/* write start offset */
	int			nwr_stop;	/* write stop offset */
};

/* list of outstanding write requests */
struct nfs_writereq *nfs_activewrites = NULL;

#define ADDTOWRLIST(nwr) { \
	/* smp_lock(&lk_nfsargs, LK_RETRY); */ \
	(nwr)->nwr_next = nfs_activewrites; \
	nfs_activewrites = (nwr); \
	/* smp_unlock(&lk_nfsargs);*/ \
	}

#define REMOVEFROMWRLIST(nwr, prev) { \
	/* smp_lock(&lk_nfsargs, LK_RETRY); */ \
	(prev)->nwr_next = (nwr)->nwr_next; \
	/* smp_unlock(&lk_nfsargs); */ \
	}

/* NFS write struct request free list */
struct nfs_writereq *nfs_writefree = NULL;

int rfs_nwrites = 0;

/*
 * Get an NFS write struct; make one if necessary.
 * The returned struct is *always* cleared.
 * This should guarantee that next(list tail) should be NULL.
 */
#define RFSGETWRITE(nwr) { \
	/* smp_lock(&lk_nfsargs, LK_RETRY); */ \
	if (nfs_writefree) { \
		(nwr) = nfs_writefree; \
		nfs_writefree = nfs_writefree->nwr_next; \
		/* smp_unlock(&lk_nfsargs); */ \
	} else { \
		/* smp_unlock(&lk_nfsargs); */ \
		(nwr) = (struct nfs_writereq *) \
			kmem_alloc(sizeof(struct nfs_writereq)); \
		++rfs_nwrites; \
	} \
	bzero((nwr), sizeof(struct nfs_writereq)); \
	}

/*
 * Add an NFS request struct to its free list.
 * NB: assumes that next(head) for an empty list is NULL.
 */
#define RFSPUTWRITE(nwr) { \
	/* smp_lock(&lk_nfsargs, LK_RETRY); */ \
	(nwr)->nwr_next = nfs_writefree; \
	nfs_writefree = (nwr); \
	/* smp_unlock(&lk_nfsargs); */ \
	}

/* RPC stuff */

#define RPCHOLDXPRT -9999
#define RPCNOREPLY  -9998

extern void svckudp_dupsave();
extern svckudp_dup();

#define	REQTOXID(req)	((req)->rq_xid)
#define	REQTOADDR(req)	((req)->rq_xprt->xp_raddr.sin_addr.s_addr)

/* RPC xprt struct free list */
SVCXPRT *rfsxprtfree = NULL;

int rfs_nxprts = 0;

/*
 * Get an RPC xprt struct; make one if necessary.
 * The returned struct is *never* cleared; we manually
 * set the next field to NULL.
 * This should guarantee that next(list tail) should be NULL.
 */
#define RFSGETXPRT(so, xprt) { \
	u_int	vers; \
	/* smp_lock(&lk_nfsargs, LK_RETRY); */ \
	if (rfsxprtfree) { \
		(xprt) = rfsxprtfree; \
		rfsxprtfree = rfsxprtfree->xp_next; \
		/* smp_unlock(&lk_nfsargs); */ \
	} else { \
		/* smp_unlock(&lk_nfsargs); */ \
		(xprt) = svckudp_create((so), NFS_PORT); \
		for (vers = VERSIONMIN; vers <= VERSIONMAX; vers++) \
			(void) svc_register((xprt), NFS_PROGRAM, vers, \
					     rfs_dispatch, FALSE); \
		++rfs_nxprts; \
	} \
	(xprt)->xp_next = NULL; \
	}

/*
 * Add an RPC xprt struct to its free list.
 * NB: assumes that next(head) for an empty list is NULL.
 */
#define RFSPUTXPRT(rx) { \
	/* smp_lock(&lk_nfsargs, LK_RETRY); */ \
	(rx)->xp_next = rfsxprtfree; \
	rfsxprtfree = (rx); \
	/* smp_unlock(&lk_nfsargs); */ \
	}

/*
 * Free all RPC xprt struct free list entries.
 * Used for service shutdown.
 */
#define RFSFREEXPRT { \
	register SVCXPRT *rx, *next; \
	/* smp_lock(&lk_nfsargs, LK_RETRY); */ \
	next = rfsxprtfree; \
	while (next != NULL) { \
		rx = next; \
		next = rx->xp_next; \
		SVC_DESTROY(rx); \
	} \
	rfsxprtfree = next; \
	/* smp_unlock(&lk_nfsargs); */ \
	}

/*
 * NFS Server system call.
 * Does all of the work of running an NFS server.
 * sock is the fd of an open UDP socket.
 */

int nfs_sv_pages=128;
int nfs_sv_bytes=256000;
int nfs_soreserve_size = 65000;

nfs_svc(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{
	struct args {
		int s;
		u_int ormask;
		u_int matchbits;
	} *uap = (struct args *) args;
	u_int msk, mtch;
	struct ucred *cr;
	int	error;
	int	i, found_one, sb_max_save;
	struct 	nfs_svinfo *psv;

	struct socket   *so;
	struct file	*fp;
	u_int	vers;
	struct vnode	*rdir;
	struct vnode	*cdir;

	/*
	 * NOTE:
	 * This routine is called from nfssvc() in nfs_hooks.c
	 * with NFSD_LOCK() held.
	 */

	DPRINTF(nfssvdebug, ("nfs_svc: got here\n"));

	/*
	 * Must be super user
	 */
	PRIV_SUSER(u.u_cred, &u.u_acflag, SEC_REMOTE, 0, EPERM, error);
	fp = getsock(uap->s, &error);
	if (error != 0) {
		NFSD_UNLOCK();
		return (error);
	}

	NFSD_UNLOCK();
	so = (struct socket *)fp->f_data;
	SOCKET_LOCK(so);

	if (!nfs_sv_socket)
		nfs_sv_socket = so;	/* squirrel away for later use */

#ifdef	KTRACE
	kern_trace(325,so,0,0);
#endif  /* KTRACE */
	if (nfs_wakeupone)
		so->so_rcv.sb_flags |= SB_WAKEONE;
	SOCKET_UNLOCK(so);

	nfs_socket_size = nfs_system_size() * nfs_soreserve_size;
	sb_max_save = sb_max;
	sb_max = nfs_socket_size * 4;
	error = soreserve(so, nfs_socket_size, nfs_socket_size);
	sb_max = sb_max_save;
	if (error) {
		printf("nfsd: soreserve fails: %d\n", error);
		return (error);
	}

	DPRINTF(nfs_sv_debug,
		("nfs_svc: mbmax %d, hiwat %d\n", so->so_rcv.sb_mbmax,
		so->so_rcv.sb_hiwat));

	/*
	 * must copy credentials so others don't see changes
	 */
	cr = u.u_cred = crcopy(u.u_cred);
	msk = uap->ormask;
	mtch = uap->matchbits;

	/*
	 * Be sure that rdir (the server's root vnode) is set.
	 * Save the current directory and restore it again when
	 * the call terminates.
	 */
	rdir = u.u_rdir;
	cdir = u.u_cdir;
	DPRINTF(nfssvdebug,
		("nfs_svc: pid %d cdir 0x%x rdir 0x%x\n",
		u.u_procp->p_pid, u.u_cdir, u.u_rdir));

	if (!rdir) {
		u.u_rdir = u.u_cdir;
	}

	/*
	 * Try to discard pte's, mark not swappable.
	 */
	pmap_collect(vm_map_pmap(current_thread()->task->map));
	thread_swappable(current_thread(), FALSE);

	/* smp_lock(&lk_nfsbiod, LK_RETRY); */
	NFSD_LOCK();
	if (nfs_nfsds >= MAXNFSDS) {
		/* smp_unlock(&lk_nfsbiod); */
		NFSD_UNLOCK();
		printf("too many nfsds\n");
		return (EACCES);
	} else {
		++nfs_nfsds; /* increment running server count */
		found_one = 0;
		for (i = 0, psv = nfs_sva; i < nfs_max_daemon; i++, psv++) {
			if (psv->nfs_sv_pid > 0)
				continue;
			found_one++;
			break;
		}
		if (!found_one) {
			++nfs_max_daemon;
			++i;
		}
		if (nfs_max_daemon > nfs_nfsds)
			panic("NFS server array botch, entry");
		PNFS_SV_HACK = (vm_offset_t)psv;
		psv->nfs_sv_pid =
			u.u_procp->p_pid;

		psv->nfs_sv_nfsd = nfs_nfsds;
		
		/* smp_unlock(&lk_nfsbiod); */
		NFSD_UNLOCK();
	}

	RFSGETXPRT(so, psv->nfs_sv_xprt);

	DPRINTF(nfssvdebug, ("nfs_svc: start svc_run\n"));

	while (TRUE) {
		/* returns *now* - chet */
		error = svc_run(psv->nfs_sv_xprt);
		if (error != RPCHOLDXPRT)
			break;
		RFSGETXPRT(so, psv->nfs_sv_xprt);
	}

      done:

	DPRINTF(nfssvdebug,
		("nfsd %d is dying of %d\n", u.u_procp->p_pid, error));

	if (error == EINTR)
		error = 0;
	else
		printf("svc_run returns unexpected %d\n", error);

	/* shut the service down when the last daemon exits */
	/* smp_lock(&lk_nfsbiod, LK_RETRY); */
	NFSD_LOCK();
	psv->nfs_sv_pid = 0;
	i = nfs_max_daemon - 1;
	if (psv == nfs_sva + i) {
		struct 	nfs_svinfo *qsv;

		for (qsv = nfs_sva + i; qsv >= nfs_sva; --qsv) {
			if (qsv->nfs_sv_pid == 0)
				--nfs_max_daemon;
			else
				break;
		}
	}

	if (--nfs_nfsds <= 0) {
		for (vers = VERSIONMIN; vers <= VERSIONMAX; vers++) {
			svc_unregister(NFS_PROGRAM, vers);
		}
		/* smp_unlock(&lk_nfsbiod); */
		NFSD_UNLOCK();
		SVC_DESTROY(psv->nfs_sv_xprt);
		/* rfsfreexprt(); */
		RFSFREEXPRT;
		nfs_sv_socket = NULL;
	} else {
		/* smp_unlock(&lk_nfsbiod); */
		NFSD_UNLOCK();
		SVC_DESTROY(psv->nfs_sv_xprt);
	}

	FP_UNREF(fp);
	thread_swappable(current_thread(), TRUE);

	u.u_rdir = rdir;
	u.u_cdir = cdir;

	DPRINTF(nfssvdebug, ("nfs_svc: bye bye\n"));
	return (error);
}

/*
 * Global "constants" and defines used for stale file handle processing
 */
int nfs_stalefh_count = 0;

int nfs_n_nomount = 0;
int nfs_last_nomount = 0;

int nfs_nomount_thresh = 10;
int nfs_nomount_timeout = 600;

int nfs_noexport_thresh = 10;
int nfs_noexport_timeout = 600;

int nfs_stale_thresh = 10;
int nfs_stale_timeout = 600;

#define NFS_N_NOEXPORT(mp) ((mp)->m_nfs_errmsginfo.n_noexport)
#define NFS_LAST_NOEXPORT(mp) ((mp)->m_nfs_errmsginfo.last_noexport)
#define NFS_N_STALE(mp) ((mp)->m_nfs_errmsginfo.n_stalefh)
#define NFS_LAST_STALE(mp) ((mp)->m_nfs_errmsginfo.last_stalefh)

#ifdef	DEBUG
int nfs_ref_threshhold = 32;	/* CJXXX */
#endif /* DEBUG */

/*
 * nfsrv_fhtovp() - convert a fh to a vnode ptr
 * 	- look up fsid in mount list (if not found ret error)
 *	- check that it is exported
 *	- get vp by calling VFS_FHTOVP() macro
 *	- if cred->cr_uid == 0 set it to m_exroot
 */
struct vnode *
nfsrv_fhtovp(fh, xprt, which, xpd)
	nfsv2fh_t *fh;
	SVCXPRT *xprt;
	int	which;
	struct kexport *xpd;
{
	struct mount *mp;
	struct vnode *vp;
	int print = 1;
	int error;

	/*
	 * Check that the file system is mounted on and
	 * if so get a ref on it.
	 */
	mp = getvfs(&fh->fh_generic.fh_fsid);
	DPRINTF(nfsdebug, ("fhtovp: getvfs returns 0x%x\n", mp));

	if (!mp) {
		DPRINTF(nfsdebug, ("fhtovp: nomount stuff\n"));

		/*if ((rgp = fref(mp, fh->fh_generic.fh_fsid)) == NULL)
		  ULTRIX SMP*/
		/*
		 * These messages can be a pain - sometimes you want them,
		 * sometimes (most of the time) you don't.
		 * Print the first nfs_nomount_thresh, then keep a
		 * timestamp and throw the rest away until nfs_nomount_timeout
		 * seconds have passed; after that, start over with the
		 * same algorithm.
		 *
		 * If nfs_nomount_thresh is set (via adb or the like)
		 * to zero, then all messages will be printed.
		 *
		 * If nfs_nomount_thresh is positive, and
		 * nfs_nomount_timeout is set to zero,
		 * then no more than nfs_nomount_thresh messages will be
		 * printed.
		 */
		if (nfs_nomount_thresh) {
			++nfs_n_nomount;
			if (nfs_n_nomount == nfs_nomount_thresh) {
				nfs_last_nomount = time.tv_sec;
			}
			else if (nfs_n_nomount > nfs_nomount_thresh) {
				if (nfs_nomount_timeout &&
					time.tv_sec > nfs_last_nomount +
					nfs_nomount_timeout) {
						nfs_n_nomount = 1;
				}
				else {
					print = 0;
				}
			}
		}
		if (print) {
			printf("NFS server: fs(%d,%d) not mounted,",
			       major(fh->fh_generic.fh_fsid.val[0]),
			       minor(fh->fh_generic.fh_fsid.val[0]));
			printf(" %s, client address = %s\n",
			       rfsnames[which],
			       inet_ntoa(xprt->xp_raddr.sin_addr));
		}
		goto stale;
	}

	/*
	 * verify that the filesystem is exported
	 */
	if ((xpd == NULL) || (!(xpd->e_flags & M_EXPORTED))) {
		DPRINTF(nfsdebug, ("fhtovp: not exported stuff\n"));

		/*
		 * See comment above for `not mounted ' messages.
		 * A similar algorithm is used here.
		 */
		if (nfs_noexport_thresh) {
			++NFS_N_NOEXPORT(mp);
			if (NFS_N_NOEXPORT(mp) == nfs_noexport_thresh) {
				NFS_LAST_NOEXPORT(mp) =
					(u_int)time.tv_sec;
			}
			else if (NFS_N_NOEXPORT(mp) > nfs_noexport_thresh) {
				if (nfs_noexport_timeout &&
					(u_int)time.tv_sec >
					NFS_LAST_NOEXPORT(mp) +
					nfs_noexport_timeout) {
						NFS_N_NOEXPORT(mp) = 1;
				}
				else {
					print = 0;
				}
			}
		}
		if (print) {
			struct ufid *ufh;

			ufh = (struct ufid *)&fh->fh_generic.fh_fid;
			printf("NFS server: unexported fs(%d, %d) file %d,",
			       major(fh->fh_generic.fh_fsid.val[0]),
			       minor(fh->fh_generic.fh_fsid.val[0]),
			       ufh->ufid_ino);
			printf(" %s, client address = %s\n",
			       rfsnames[which],
			       inet_ntoa(xprt->xp_raddr.sin_addr));
		}
		/*grele(rgp); ULTRIX SMP*/
		goto stale;
	}

	VFS_FHTOVP(mp, &fh->fh_generic.fh_fid, &vp, error);
	if (error)  {
		DPRINTF(nfsdebug, ("fhtovp: stalefh stuff\n"));
		/*
		 * See comment above for `not mounted ' messages.
		 * A similar algorithm is used here.
		 */
		if (nfs_stale_thresh) {
			++NFS_N_STALE(mp);
			if (NFS_N_STALE(mp) == nfs_stale_thresh) {
				NFS_LAST_STALE(mp) = (u_int)time.tv_sec;
			}
			else if (NFS_N_STALE(mp) > nfs_stale_thresh) {
				if (nfs_stale_timeout &&
					(u_int) time.tv_sec >
					NFS_LAST_STALE(mp) +
					nfs_stale_timeout) {
						NFS_N_STALE(mp) = 1;
				}
				else {
					print = 0;
				}
			}
		}
		if (print) {
			struct ufid *ufh;

			ufh = (struct ufid *)&fh->fh_generic.fh_fid;
			printf("NFS server: stale file handle fs(%d,%d) file %d gen %d\n",
			       major(fh->fh_generic.fh_fsid.val[0]),
			       minor(fh->fh_generic.fh_fsid.val[0]), 
			       ufh->ufid_ino, ufh->ufid_gen);
			/*printf("             local gen %d nlink %d,",
				gp->g_gennum, gp->g_nlink);*/
			printf(" %s, client address = %s, errno %d\n",
			       rfsnames[which],
			       inet_ntoa(xprt->xp_raddr.sin_addr), error);
		}
		goto stale;
	}

	MOUNT_LOOKUP_DONE(mp);
#ifdef DEBUG
	if ((vp->v_usecount > nfs_ref_threshhold))
		printf("fhtovp *refs*: %s vp 0x%x type %d ref %d\n",
		       rfsnames[which], vp, vp->v_type, vp->v_usecount);
#endif /* DEBUG */
	DPRINTF(nfsdebug || nfsopdebug[which] || nfsrefdebug,
		("fhtovp: %s return vp 0x%x ref %d\n", rfsnames[which],
		       vp, (vp)?vp->v_usecount:666));
	return (vp);

stale:
	/* NULL return means fhandle not converted */
	if (mp)
		MOUNT_LOOKUP_DONE(mp);
	nfs_stalefh_count++;
	DPRINTF(nfsdebug, ("fhtovp: return NULL\n"));
	return (NULL);
}

/*
 * These are the interface routines for the server side of the
 * Networked File System.  See the NFS protocol specification
 * for a description of this interface.
 */

/*
 * Get file attributes.
 * Returns the current attributes of the file with the given fhandle.
 */
int
rfs_getattr(fhp, ns, nreq, xprt, xpd)
	nfsv2fh_t *fhp;
	struct nfsattrstat *ns;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	int error = 0;
	struct vnode *vp;
	struct vattr va;

	vp = nfsrv_fhtovp(fhp, xprt, RFS_GETATTR, xpd);
#ifdef	KTRACE
	kern_trace(301,vp,0,0);
#endif  /* KTRACE */
	DPRINTF(nfsdebug || nfsopdebug[RFS_GETATTR],
		("getattr: fhtovp returns 0x%x\n", vp));

	if (!vp) {
		ns->ns_status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_GETATTR],
			("getattr: return ESTALE %d\n", ESTALE));
		return (0);
	}

	/* From here on, all returns should be via `goto done' */

	VOP_GETATTR(vp, &va, u.u_cred, error);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_GETATTR],
			("getattr: vop_getattr returns %d\n", error));
		goto done;
	}

	vattr_to_nattr(&va, &ns->ns_attr);

      done:

	vrele(vp);	/* one for fhtovp */
	ns->ns_status = puterrno(error);
	DPRINTF(nfsdebug || nfsopdebug[RFS_GETATTR],
		("getattr: return %d\n", error));
	return (0);

}

/*
 * Set file attributes.
 * Sets the attributes of the file with the given fhandle.  Returns
 * the new attributes.
 */
int
rfs_setattr(args, ns, nreq, xprt, xpd)
	struct nfssaargs *args;
	struct nfsattrstat *ns;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	int error = 0;
	struct vnode *vp;
	struct vattr va;

	vp = nfsrv_fhtovp(&args->saa_fh, xprt, RFS_SETATTR, xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_SETATTR],
		("setattr: fhtovp returns 0x%x\n", vp));

#ifdef	KTRACE
	kern_trace(302,vp,0,0);
#endif  /* KTRACE */
	if (!vp) {
		ns->ns_status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_SETATTR],
			("setattr: return ESTALE %d\n", ESTALE));
		return (0);
	}

	/* From here on, all returns should be via `goto done' */

	if (rdonly(vp, xpd, xprt)) {
		error = EROFS;
		DPRINTF(nfsdebug || nfsopdebug[RFS_SETATTR],
			("setattr: readonly fs\n"));
		goto done;
	}

	error = nfsrv_access(vp, VWRITE, u.u_cred);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_SETATTR],
			("setattr: nfsrv_access failed %d\n", error));
		goto done;
	}

	sattr_to_vattr(&args->saa_sa, &va);
	/*
	 * There is a bug in the Sun client that puts 0xffff in the mode
	 * field of sattr when it should put in 0xffffffff. The u_short
	 * doesn't sign extend.
	 * --> check the low order 2 bytes for 0xffff
	 */
	if ((va.va_mode & 0xffff) == 0xffff)
		va.va_mode = VNOVAL;
	va.va_type = VNON;

	VOP_SETATTR(vp, &va, u.u_cred, error);
	if (error) {
		DPRINTF((nfsdebug || nfsopdebug[RFS_SETATTR]),
			("setattr: vop_setattr fails %d\n", error));
		goto done;
	}

	VOP_FSYNC(vp, FWRITE | FWRITE_METADATA, u.u_cred, MNT_WAIT, error);
	DPRINTF(error && (nfsdebug || nfsopdebug[RFS_SETATTR]),
			("setattr: vop_fsync returns %d\n", error));

	if (!error) {
		VOP_GETATTR(vp, &va, u.u_cred, error);
		if (error) {
			DPRINTF(nfsdebug || nfsopdebug[RFS_SETATTR],
				("write: vop_getattr2 failed %d\n", error));
			goto done;
		}
	}

      done:

	vrele(vp);	/* one for fhtovp */
	if (!error)
		vattr_to_nattr(&va, &ns->ns_attr);

	ns->ns_status = puterrno(error);
	DPRINTF(nfsdebug || nfsopdebug[RFS_SETATTR],
		("setattr: return %d\n", error));
	return (0);

}

/*
 * Directory lookup.
 * Returns an fhandle and file attributes for file name in a directory.
 */
int
rfs_lookup(da, dr, nreq, xprt, xpd)
	struct nfsdiropargs *da;
	struct  nfsdiropres *dr;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	int error = 0;
	struct vnode *dvp;
	struct vnode *vp;
	struct vattr va;
	char *name = da->da_name;
	struct nameidata *ndp = &u.u_nd;
	nfsv2fh_t *fhp;

	/* Disallow NULL paths */
	if ((da->da_name == (char *)NULL) || (*da->da_name == '\0')) {
		dr->dr_status = NFSERR_ACCES;
		DPRINTF(nfsdebug || nfsopdebug[RFS_LOOKUP],
			("lookup: null name\n", ESTALE));
		return (0);
	}

	dvp = nfsrv_fhtovp(&da->da_fhandle, xprt, RFS_LOOKUP, xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_LOOKUP],
		("lookup: fhtovp returns 0x%x\n", dvp));

#ifdef	KTRACE
	kern_trace(304,vp,0,0);
#endif  /* KTRACE */
	if (!dvp) {
		dr->dr_status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_LOOKUP],
			("lookup: return ESTALE %d\n", ESTALE));
		return (0);
	}

	/* From here on, all returns should be via `goto done' */

	/* Make sure that we are positioned at a directory */
	if (dvp->v_type != VDIR) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_LOOKUP],
			("lookup: attempt to lookup in non-directory\n"));
 		error = ENOTDIR;
		goto done;
	}

	/* Do not allow lookup beyond root */
	if (!strcmp(da->da_name, "..") &&
	    fid_equal(&da->da_fhandle.fh_generic.fh_fid,
		      &da->da_fhandle.fh_generic.fh_efid)) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_LOOKUP],
			("lookup: .. from root\n"));
		error = ENOENT;
		goto done;
	}

	ndp->ni_cdir = dvp;
	ndp->ni_cred = u.u_cred;
	ndp->ni_nameiop = LOOKUP|NOCROSSMOUNT;
	ndp->ni_segflg = UIO_SYSSPACE;
	ndp->ni_dirp = name;
	DPRINTF(nfsdebug || nfsopdebug[RFS_LOOKUP],
		("lookup: call namei (looking for: %s)\n", name));
	error = namei(ndp);
	vp = ndp->ni_vp;
	DPRINTF(nfsdebug || nfsopdebug[RFS_LOOKUP],
		("lookup: vop_lookup returns 0x%x %d\n", vp, error));
	if (error)
		goto done;

	/*gfs_unlock(dvp); ultrix SMP*/
	/*gfs_lock(vp); ultrix SMP*/

	/* From here on, all returns should be via `goto out' */

	VOP_GETATTR(vp, &va, u.u_cred, error);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_LOOKUP],
			("lookup: vop_getattr returns %d\n", error));
		goto out;
	}

	DPRINTF(nfsdebug || nfsopdebug[RFS_LOOKUP],
		("lookup: vop_getattr returns fsid %x file id %d\n",
		       va.va_fsid, va.va_fileid));

	vattr_to_nattr(&va, &dr->dr_attr);

	/*
	 * N.B.: there's a slim chance that this vnode has been vgone'd,
	 * in which case v_mount points to dead_mount.  If this is the
	 * case, then we can depend on an error from the
	 * VFS_VPTOFH call.
	 */
	fhp = &dr->dr_fhandle;
	fhp->fh_generic.fh_fsid = vp->v_mount->m_stat.f_fsid;
	VFS_VPTOFH(vp, (struct fid *)&fhp->fh_generic.fh_fid, error);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_LOOKUP],
			("lookup: vop_vptofh returns %d\n", error));
		goto out;
	}

	/*
	 *  Check to see if we can support this file system
	 */
	if (sizeof(fsid_t) + fhp->fh_generic.fh_fid.fid_len +
	    xpd->e_fid.fid_len > NFS_FHSIZE ) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_LOOKUP],
			("lookup: fhandle too big\n"));
		error = EREMOTE;
		goto out;
	}

	fid_copy(&xpd->e_fid, &fhp->fh_generic.fh_efid);
	/* gfs_unlock(vp); ultrix SMP*/

      out:

	vrele(vp);

      done:

	vrele(dvp);	/* one for fhtovp */
	dr->dr_status = puterrno(error);
	DPRINTF(nfsdebug || nfsopdebug[RFS_LOOKUP],
		("lookup: return %d\n", error));
	return (0);

}

/*
 * Read symbolic link.
 * Returns the string in the symbolic link at the given fhandle.
 */
int
rfs_readlink(fhp, rl, nreq, xprt, xpd)
	nfsv2fh_t *fhp;
	struct nfsrdlnres *rl;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	int error = 0;
	struct vnode *vp;
	struct iovec iov;
	struct uio uio;

	vp = nfsrv_fhtovp(fhp, xprt, RFS_READLINK, xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_READLINK],
		("readlink: fhtovp returns 0x%x\n", vp));

#ifdef	KTRACE
	kern_trace(305,vp,0,0);
#endif  /* KTRACE */
	if (!vp) {
		rl->rl_status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_READLINK],
			("readlink: return ESTALE %d\n", ESTALE));
		return (0);
	}

	/* From here on, all returns should be via `goto done' */

	/*
	 * Allocate data for pathname.  This will be freed by rfs_rlfree.
	 */
	rl->rl_data = (char *)kmem_alloc(MAXPATHLEN);

	/*
	 * Set up io vector to read sym link data
	 */
	iov.iov_base = rl->rl_data;
	iov.iov_len = NFS_MAXPATHLEN;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_rw = UIO_READ;
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_offset = 0;
	uio.uio_resid = NFS_MAXPATHLEN;

	/*
	 * read link
	 */
	if (vp->v_type != VLNK) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_READLINK],
			("readlink: not a symlink file\n"));
		error = EINVAL;
		goto done;
	}

	VOP_READLINK(vp, &uio, u.u_cred, error);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_READLINK],
			("readlink: vop_readlink returns %d\n", error));
		goto done;
	}

      done:

	vrele(vp);	/* one for fhtovp */
	if (error) {
		kmem_free((caddr_t)rl->rl_data, (u_int)NFS_MAXPATHLEN);
		rl->rl_count = 0;
		rl->rl_data = NULL;
	} else {
		rl->rl_count = NFS_MAXPATHLEN - uio.uio_resid;
	}

	rl->rl_status = puterrno(error);
	DPRINTF(nfsdebug || nfsopdebug[RFS_READLINK],
		("readlink: return %d\n", error));

	return (0);

}

/*
 * Free data allocated by rfs_readlink
 */
rfs_rlfree(rl)
	struct nfsrdlnres *rl;
{
#ifdef	KTRACE
	kern_trace(318,rl->rl_srok,rl->rl_count,rl->rl_data);
#endif  /* KTRACE */
	if (rl->rl_data) {
		kmem_free((caddr_t)rl->rl_data, (u_int)NFS_MAXPATHLEN);
	}
}

/*
 * Read data.
 * Returns some data read from the file at the given fhandle.
 */
int
rfs_read(ra, rr, nreq, xprt, xpd)
	struct nfsreadargs *ra;
	struct nfsrdresult *rr;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	struct nfs_svinfo *psv = (struct nfs_svinfo *)PNFS_SV;
	struct buf *bp;
	int error = 0;
	int bread = 0;
	struct vnode *vp;
	struct vattr va;
	struct iovec iov;
	struct uio uio;
	int offset, fsbsize;

	DPRINTF(rrokdebug, ("rfs_read rr 0x%x\n", rr));

	if (ra->ra_count > NFS_MAXDATA || ra->ra_count < 0) {
		error = EBADRPC;
		rr->rr_status = puterrno(error);
		DPRINTF(nfsdebug || nfsopdebug[RFS_READ],
			("read: return %d\n", error));
		return (0);
	}

	/* Do not allow any reads beyond 2GB */
        if ((int)ra->ra_offset >= 0x7fffffff) {
                error = EFBIG;
                rr->rr_status = puterrno(error);
                DPRINTF(nfsdebug || nfsopdebug[RFS_WRITE],
                        ("write: return %d\n", error));
                return (0);
        }

	/* Only reads upto 2GB are allowed */
        if ((int)ra->ra_offset + (int)ra->ra_count > 0x7fffffff)
                (int)ra->ra_count = 0x7fffffff - (int)ra->ra_offset;

	rr->rr_data = NULL;
	rr->rr_count = 0;
	rr->rr_bp = NULL;
	rr->rr_vp = NULL;
	psv->nfs_sv_state = NFSSV_FHTOVP;
	psv->nfs_sv_fh = &ra->ra_fhandle;

	vp = nfsrv_fhtovp(&ra->ra_fhandle, xprt, RFS_READ, xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_READ],
		("read: fhtovp returns 0x%x\n", vp));

#ifdef	KTRACE
	kern_trace(306,vp,ra->ra_offset,ra->ra_count);
#endif  /* KTRACE */
	if (!vp) {
		rr->rr_status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_READ],
			("read: return ESTALE %d\n", ESTALE));
		return (0);
	}

	/* From here on, all returns should be via `goto done' */

	psv->nfs_sv_vp = vp;
	psv->nfs_sv_state = NFSSV_DATA1;
	if (vp->v_type != VREG) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_READ],
			("read: not a regular file\n"));
		error = EINVAL;
		goto done;
	}

	/*
	 * This is a kludge to allow reading of files created
	 * with no read permission.  The owner of the file
	 * is always allowed to read it.
	 * Exec is the same as read over the net because
	 * of demand loading.
	 */
	error = nfsrv_access(vp, VREAD|VEXEC, u.u_cred);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_READ],
			("read: nfsrv_access failed %d\n", error));
		goto done;
	}

	VOP_GETATTR(vp, &va, u.u_cred, error);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_READ],
			("read: vop_getattr1 returns %d\n", error));
		goto done;
	}
	if (ra->ra_offset >= va.va_size || ra->ra_count == 0) {
		vattr_to_nattr(&va, &rr->rr_attr);
		goto done;			/* zero read or EOF */
	}

	/*
	 * Check whether we can do this with bread, which would
	 * save the copy through the uio.
	 */

	fsbsize = va.va_blocksize;
	offset = ra->ra_offset % fsbsize;

	/* make sure and fall through to VOP_READ if filesystem */
	/* does not support VOP_BREAD */
	if (offset + ra->ra_count <= fsbsize && offset % 4 == 0) {
		VOP_BREAD(vp, ra->ra_offset / fsbsize, &bp, u.u_cred, error);
		DPRINTF(rrokdebug,
			("rfs_read: err %d vp 0x%x bp 0x%x\n",
			error, vp, bp));
		if (error == 0) {
			++bread;
			rr->rr_data = bp->b_un.b_addr + offset;
			rr->rr_count = MIN(
			    (va.va_size - ra->ra_offset), ra->ra_count);
			rr->rr_bp = bp;
			rr->rr_vp = vp;

			VOP_GETATTR(vp, &va, u.u_cred, error);
			if (error) {
				DPRINTF(nfsdebug || nfsopdebug[RFS_READ],
				("read: vop_getattr2 returns %d\n", error));
				goto done;
			}

			vattr_to_nattr(&va, &rr->rr_attr);
			DPRINTF(rrokdebug,
				("rfs_read: data 0x%x cnt %d\n",
				rr->rr_data, rr->rr_count));
			goto done;
		} else { /* fall through */
			DPRINTF(nfsdebug || nfsopdebug[RFS_READ],
				("read: vop_bread failed %d\n", error));
		}
	}

	/*
	 * Allocate space for data.  This will be freed by rfs_rdfree()
	 * unless we lose a truncate race.
	 */
	rr->rr_data = kmem_alloc(ra->ra_count);
	/* NB: we cannot free a portion of this later */
	rr->rr_allocsize = ra->ra_count;

	/*
	 * Set up io vector
	 */
	iov.iov_base = rr->rr_data;
	iov.iov_len = ra->ra_count;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_rw = UIO_READ;
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_offset = ra->ra_offset;
	uio.uio_resid = ra->ra_count;
	/*
	 * for now we assume no append mode and ignore
	 * totcount (read ahead)
	 */

	VOP_READ(vp, &uio, 0, u.u_cred, error);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_READ],
			("read: vop_read returns %d\n", error));
		goto done;
	}

	VOP_GETATTR(vp, &va, u.u_cred, error);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_READ],
			("read: vop_getattr3 returns %d\n", error));
		goto done;
	}

	vattr_to_nattr(&va, &rr->rr_attr);
	rr->rr_count = ra->ra_count - uio.uio_resid;

      done:

	if (!bread) {	/* resources free via rrok route */
		vrele(vp);	/* one for fhtovp */
		if (rr->rr_data != NULL &&
		    (error || rr->rr_count == 0)) {
			/* read error or lost a truncate race */
			kmem_free(rr->rr_data, (u_int)rr->rr_allocsize);
			rr->rr_data = NULL;
			rr->rr_count = 0;
		}
	}

	rr->rr_status = puterrno(error);
	DPRINTF(nfsdebug || nfsopdebug[RFS_READ],
		("read: return %d, count = %d\n", error, rr->rr_count));
	return (0);

}

/*
 * Free data allocated by rfs_read.
 */
rfs_rdfree(rr)
	struct nfsrdresult *rr;
{
#ifdef	KTRACE
	kern_trace(319,rr->rr_data,rr->rr_count,rr->rr_allocsize);
#endif  /* KTRACE */
	DPRINTF(rrokdebug,
		("rfs_rdfree: bp 0x%x size %d\n", rr->rr_bp, rr->rr_allocsize));
	if (!rr->rr_bp && rr->rr_data) {
		if (rr->rr_count <= 0)
			panic("rfs_rdfree: bad count");
		kmem_free(rr->rr_data, (u_int)rr->rr_allocsize);
	}
}

/*
 * Write data to file.
 * Returns attributes of a file after writing some data to it.
 */

long nfs_write_gather = 1;
long nfs_write_mbufs = 1;
int nfs_mbuf_writes = 0;
int nfs_nonmbuf_writes = 0;

int
rfs_write(wa, ns, nreq, xprt, xpd)
	struct nfswriteargs *wa;
	struct nfsattrstat *ns;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	struct nfs_svinfo *psv = (struct nfs_svinfo *)PNFS_SV;
	struct vnode *vp;
	struct inode *ip;
	int error;

	DPRINTF(nfs_sv_debug,
		("rfs_write start: pid %d, offset %d\n", psv->nfs_sv_pid,
		wa->wa_offset));

	if (wa->wa_count > NFS_MAXDATA || wa->wa_count < 0) {
		error = EINVAL;
		ns->ns_status = puterrno(error);
		DPRINTF(nfsdebug || nfsopdebug[RFS_WRITE],
			("write: return %d\n", error));
		return (0);
	}

	/* Do not allow any writes beyond 2GB */
        if ((int)wa->wa_offset >= 0x7fffffff) {
                error = EFBIG;
                ns->ns_status = puterrno(error);
                DPRINTF(nfsdebug || nfsopdebug[RFS_WRITE],
                        ("write: return %d\n", error));
                return (0);
        }

	/* Only writes upto 2GB are allowed */
        if ((int)wa->wa_offset + (int)wa->wa_count > 0x7fffffff)
                (int)wa->wa_count = 0x7fffffff - (int)wa->wa_offset;

	psv->nfs_sv_state = NFSSV_FHTOVP;
	psv->nfs_sv_fh = &wa->wa_fhandle;
	if (wa->wa_count > 0) {
		psv->nfs_sv_start = (int)wa->wa_offset;
		psv->nfs_sv_stop = psv->nfs_sv_start + (int)wa->wa_count - 1;
	} else {
		psv->nfs_sv_start = -1;
		psv->nfs_sv_stop = -1;
	}

	/* There is a nasty problem in this routine if write gathering */
	/* is on, there are outstanding replies waiting for meta-data, */
	/* and this thread is the last. All error legs must fall all */
	/* the way through to the bottom so as to flush replies. */

#ifdef CJKTRACE
/*	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_W_FHTOVP_START);*/
#endif /* CJKTRACE */

	vp = nfsrv_fhtovp(&wa->wa_fhandle, xprt, RFS_WRITE, xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_WRITE],
		("write: fhtovp returns 0x%x\n", vp));
#ifdef CJKTRACE
/*	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_W_FHTOVP_DONE);*/
#endif /* CJKTRACE */

#ifdef	KTRACE
	kern_trace(308, vp, wa->wa_offset,wa->wa_count);
#endif  /* KTRACE */
	if (!vp) {
		ns->ns_status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_WRITE],
			("write: return ESTALE %d\n", ESTALE));
	}

	psv->nfs_sv_vp = vp;

	if (nfs_write_gather &&
	    (!vp || (vp->v_mount->m_stat.f_type == MOUNT_UFS) ||
			(vp->v_mount->m_stat.f_type == MOUNT_MSFS)))
		return (rfs_writeg(vp, wa, ns, nreq, xprt, xpd));
	else
		return (rfs_writeo(vp, wa, ns, nreq, xprt, xpd));

}

int nfs_long_chain = 0;

int
rfs_writeo(vp, wa, ns, nreq, xprt, xpd)
	struct vnode *vp;
	struct nfswriteargs *wa;
	struct nfsattrstat *ns;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	struct nfs_svinfo *psv = (struct nfs_svinfo *)PNFS_SV;
	int error = 0;
	struct vattr va;
	struct iovec iov;
	struct iovec *piov = NULL;
	int piovlen;
	struct uio uio;
	struct timeval duptime;
	int cc;
	struct inode *ip;

	if (!vp)
		return (0);	/* ESTALE */

	/* From here on, all returns should be via `goto done' */

	DPRINTF(nfs_sv_debug,
		("rfs_write(start): pid %d, vp 0x%x, off %d, cnt %d\n",
		psv->nfs_sv_pid, vp, wa->wa_offset, wa->wa_count));

#ifdef CJKTRACE
	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WO_START);
#endif /* CJKTRACE */

	if (rdonly(vp, xpd, xprt)) {
		error = EROFS;
		DPRINTF(nfsdebug || nfsopdebug[RFS_WRITE],
			("write: readonly fs\n"));
		goto done;
	}

	DPRINTF(nfs_sv_debug, ("rfs_write(2): pid %d\n", psv->nfs_sv_pid));

	if (vp->v_type != VREG) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_WRITE],
			("write: not a regular file\n"));
		error = EINVAL;
		goto done;
	}

	DPRINTF(nfs_sv_debug, ("rfs_write(3): pid %d\n", psv->nfs_sv_pid));

	error = nfsrv_access(vp, VWRITE, u.u_cred);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_WRITE],
			("write: nfsrv_access failed %d\n", error));
		goto done;
	}

	DPRINTF(nfs_sv_debug, ("rfs_write(4): pid %d\n", psv->nfs_sv_pid));

	if (wa->wa_data) {
		iov.iov_base = wa->wa_data;
		iov.iov_len = wa->wa_count;
		uio.uio_iov = &iov;
		uio.uio_iovcnt = 1;
		uio.uio_rw = UIO_WRITE;
		uio.uio_segflg = UIO_SYSSPACE;
		uio.uio_offset = wa->wa_offset;
		uio.uio_resid = wa->wa_count;
		++nfs_nonmbuf_writes;
	}
	else { 	/* mbuf hack */
		register struct mbuf *m;
		register int iovcnt;

		iovcnt = 0;
		DPRINTF(nfs_mbuf_debug, ("mbufs = "));
		for (m = (struct mbuf *)wa->wa_mbuf; m; m = m->m_next) {
			DPRINTF(nfs_mbuf_debug, ("%d ", m->m_len));
			iovcnt++;
		}
		DPRINTF(nfs_mbuf_debug, ("\n"));

		/*
		 * Use iovcnt to determine whether we have a long
		 * mbuf chain and need to kmem_alloc temp space.
		 * Use piov to point at either permanent ot temp space.
		 */
		if (iovcnt > N_NFS_SV_IOV) {
			++nfs_long_chain;
			DPRINTF(nfs_lmbuf_debug,
				("rfs_write: long chain %d\n", iovcnt));
			piovlen = sizeof(struct iovec) * iovcnt;
			piov = (struct iovec *)
				kmem_alloc(piovlen);
		} else
			piov = psv->nfs_sv_iovp;

		mbuf_to_iov((struct mbuf *)wa->wa_mbuf, piov);
		uio.uio_iov = piov;
		uio.uio_iovcnt = iovcnt;
		uio.uio_rw = UIO_WRITE;
		uio.uio_segflg = UIO_SYSSPACE;
		uio.uio_offset = wa->wa_offset;
		uio.uio_resid = wa->wa_count;
		if (iovcnt <= N_NFS_SV_IOV)
			piov = NULL;
		++nfs_mbuf_writes;
	}

	DPRINTF(nfs_sv_debug, ("rfs_write(6): pid %d\n", psv->nfs_sv_pid));

	/*
	 * for now we assume no append mode
	 */
	VOP_WRITE(vp, &uio, IO_SYNC, u.u_cred, error);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_WRITE],
			("write: vop_write returns %d\n", error));
		goto done;
	}

	DPRINTF(nfs_sv_debug, ("rfs_write(7): pid %d\n", psv->nfs_sv_pid));

	if (piov)
		kmem_free(piov, piovlen);

	cc = nfs_sv_socket->so_rcv.sb_cc;
	cc /= 8192;
	if (cc >= 0 && cc < 32)
		nfs_sv_cc_hist[cc]++;
	else
		nfs_sv_hist_err++;

	/*
	 * Get attributes again so we send the latest mod
	 * time to the client side for his cache.
	 */
	if (!error) {
		VOP_GETATTR(vp, &va, u.u_cred, error);
		if (error) {
			DPRINTF(nfsdebug || nfsopdebug[RFS_WRITE],
				("write: vop_getattr2 failed %d\n", error));
			goto done;
		}
	}

 	DPRINTF(nfs_sv_debug, ("rfs_write(8): pid %d\n", psv->nfs_sv_pid));

     done:

	vrele(vp);	/* one for fhtovp */
	DPRINTF(nfs_sv_debug, ("rfs_write(10): pid %d\n", psv->nfs_sv_pid));

	if (!error)
		vattr_to_nattr(&va, &ns->ns_attr);

	DPRINTF(nfs_sv_debug, ("rfs_write(11): pid %d\n", psv->nfs_sv_pid));

	ns->ns_status = puterrno(error);
	DPRINTF(nfsdebug || nfsopdebug[RFS_WRITE],
		("write: return %d\n", error));
#ifdef CJKTRACE
	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WO_DONE);
#endif /* CJKTRACE */
	return (0);

}

/* whether to do lbolt or not */
long nfs_presto_lbolt = 1;
long nfs_ufs_lbolt = 1;
/* how long lbolt should be in msec (n.b.: only used during initialization) */
long nfs_distant_lbolt = 8;
long nfs_fast_lbolt = 5;
long nfs_slow_lbolt = 8;
long nfs_unkn_lbolt = 8;
/* how long lbolt is in clock ticks (used from then on) */
long nfs_distant_ticks = 0;
long nfs_fast_ticks = 0;
long nfs_slow_ticks = 0;
long nfs_unkn_ticks = 0;

long nfs_did_ticks = 0;	/* nfs_init_lbolt has been called */

long nfs_domore_presto = 1;
long nfs_domore_ufs = 1;
int nfs_last_lbolt = 0;

struct {
       int more_nfsd;
       int emptysb;
       int more_presto_sb;
       int more_ufs_sb;
       int more_sleeper;
       int metadata;
       int failures;
       int clusters;
       int singles;
       int prestosingles;
       int presto_sleeps;
       int ufs_sleeps;
       int wakeups;
       int estale;
       int dupbusy;
       int outoforder;
       int early;
       int distant_lbolt;
       int fast_lbolt;
       int slow_lbolt;
       int unkn_lbolt;
} nfs_writes;

struct nfs_mw {
	int debug;
	int cmpdebug;
	int print_empties;
	int calls;
	int emptysb;
	int emptychn;
	int found_write;
	int partial;
	int badfh;
	int cmpfh;
	int hits;
	int miss;
} nfs_mw;

#define	GETLBOLT(n, xprt) { \
	if (!(xprt)->xp_ifp || !(xprt)->xp_ifp->ifa_ifp) { \
		 ++nfs_writes.distant_lbolt; \
		 *(n) = nfs_distant_ticks; \
	} else { \
		 switch ((xprt)->xp_ifp->ifa_ifp->if_type) { \
			case IFT_FDDI: \
			  ++nfs_writes.fast_lbolt; \
			  *(n) = nfs_fast_ticks; \
			  break; \
			case IFT_ETHER: \
			  ++nfs_writes.slow_lbolt; \
			  *(n) = nfs_slow_ticks; \
			  break; \
			default: \
			  ++nfs_writes.unkn_lbolt; \
			  *(n) = nfs_unkn_ticks; \
			  break; \
		  } \
	} \
	nfs_last_lbolt = *(n); \
}

void
nfs_init_lbolt()
{		
	extern int hz;
	int i;

	i = 1000000/hz; /* i is in us */

	if (!nfs_distant_ticks) {
		nfs_distant_ticks = (nfs_distant_lbolt * 1000) / i; 
		if (nfs_distant_ticks * i < nfs_distant_lbolt * 1000)
			++nfs_distant_ticks;
	}

	if (!nfs_fast_ticks) {
		nfs_fast_ticks = (nfs_fast_lbolt * 1000) / i; 
		if (nfs_fast_ticks * i < nfs_fast_lbolt * 1000)
			++nfs_fast_ticks;
	}

	if (!nfs_slow_ticks) {
		nfs_slow_ticks = (nfs_slow_lbolt * 1000) / i; 
		if (nfs_slow_ticks * i < nfs_slow_lbolt * 1000)
			++nfs_slow_ticks;
	}

	if (!nfs_unkn_ticks) {
		nfs_unkn_ticks = (nfs_unkn_lbolt * 1000) / i; 
		if (nfs_unkn_ticks * i < nfs_unkn_lbolt * 1000)
			++nfs_unkn_ticks;
	}
}

long nfs_last_offset = 0;
long nfs_cluster_max = 65536;

int
rfs_writeg(vp, wa, ns, nreq, xprt, xpd)
	struct vnode *vp;
	struct nfswriteargs *wa;
	struct nfsattrstat *ns;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	int error = 0;
	struct vattr va;
	struct iovec iov;
	struct iovec *piov = NULL;
	int piovlen;
	struct uio uio;

	struct nfs_svinfo *psv = (struct nfs_svinfo *)PNFS_SV;
	struct nfs_svinfo *qsv;
	int s;
	extern int prattached;
	int i, cc;
	int minoffset, maxoffset;
	int nfs_lbolt();

	int first = 1;
	int imgathering = 0;
	int dupwrite = 0;
	int estale = 0;
	int prestohere = 0;
	int dummy = 0;
	int writer = 0;
	int attr_error;
	int didmyreply = 0;
	struct nfs_writereq *wr, *mywr;
	struct nfs_writereq *prevwr, *savewr, *mywritelist;
	dev_t dev;
	int ticks;
	struct nfsattrstat tns;
	extern long prmetaonly;

	/*
	 * There is a nasty problem here if there are outstanding
	 * replies waiting for meta-data and this
	 * thread is the last; this thread must flush replies.
	 * All error legs must fall through far enough to flush active
	 * writes after discarding their write data.
	 */

#ifdef CJKTRACE
/*	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_START); */
#endif /* CJKTRACE */

	/* vnode is locked from here on out */
	if (vp) {
		VN_WRITE_LOCK(vp);
		DPRINTF(nfs_sv_debug,
			 ("rfs_write(gotlock1): pid %d, off %d\n",
			  psv->nfs_sv_pid, wa->wa_offset));

#ifdef CJKTRACE
		if (nfs_cjtrace)
			LOG_SYS_NFS(psv, NFS_WG_START);

/* 		if (nfs_cjtrace)
			LOG_SYS_NFS(psv, NFS_WG_GOTVP);*/
#endif /* CJKTRACE */
	}

	psv->nfs_sv_state = NFSSV_DATA1;

	/* Stale fhandle */
	if (!vp) {
		++estale;
		++nfs_writes.estale;
		goto metadata;
	}

	/* Read-only filesystem */
	if (rdonly(vp, xpd, xprt)) {
		error = EROFS;
		goto metadata;	/* can we have sleeping threads?? */
	}

	if (vp->v_type != VREG) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_WRITE],
			("write: not a regular file\n"));
		error = EISDIR;
		goto metadata;
	}

	error = nfsrv_access(vp, VWRITE, u.u_cred);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_WRITE],
			("write: nfsrv_access failed %d\n", error));
		goto metadata;
	}

	/*
	 * See if this is a duplicate of a write in-progress.
	 * If it is, then flush replies if "last".
	 */
	if (wa->wa_dupbusy) {
		++dupwrite;
		++nfs_writes.dupbusy;
		/*
		 * We may have won a race with
		 * the "real" write.
		 */
		DPRINTF(nfs_dup_debug,
			("dupbusy: pid %d, off %d\n", 
			       psv->nfs_sv_pid, wa->wa_offset));
		goto metadata;
	}

#ifdef CJKTRACE
/*	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_GA1_START);*/
#endif /* CJKTRACE */
	VOP_GETATTR(vp, &va, u.u_cred, error);
#ifdef CJKTRACE
/*	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_GA1_DONE);*/
#endif /* CJKTRACE */
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_WRITE],
			("write: vop_getattr1 failed %d\n", error));
		goto metadata;
	}

	if (wa->wa_data) {
		iov.iov_base = wa->wa_data;
		iov.iov_len = wa->wa_count;
		uio.uio_iov = &iov;
		uio.uio_iovcnt = 1;
		uio.uio_rw = UIO_WRITE;
		uio.uio_segflg = UIO_SYSSPACE;
		uio.uio_offset = wa->wa_offset;
		uio.uio_resid = wa->wa_count;
		++nfs_nonmbuf_writes;
	}
	else { 	/* mbuf hack */
		register struct mbuf *m;
		register int iovcnt;

		iovcnt = 0;
		DPRINTF(nfs_mbuf_debug, ("mbufs = "));
		for (m = (struct mbuf *)wa->wa_mbuf; m; m = m->m_next) {
			DPRINTF(nfs_mbuf_debug, ("%d ", m->m_len));
			iovcnt++;
		}
		DPRINTF(nfs_mbuf_debug, ("\n"));

		/*
		 * Use iovcnt to determine whether we have a long
		 * mbuf chain and need to kmem_alloc temp space.
		 * Use piov to point at either permanent ot temp space.
		 */
		if (iovcnt > N_NFS_SV_IOV) {
			++nfs_long_chain;
			DPRINTF(nfs_lmbuf_debug,
				("rfs_write: long chain %d\n", iovcnt));
			piovlen = sizeof(struct iovec) * iovcnt;
			piov = (struct iovec *)
				kmem_alloc(piovlen);
		} else
			piov = psv->nfs_sv_iovp;

		mbuf_to_iov((struct mbuf *)wa->wa_mbuf, piov);
		uio.uio_iov = piov;
		uio.uio_iovcnt = iovcnt;
		uio.uio_rw = UIO_WRITE;
		uio.uio_segflg = UIO_SYSSPACE;
		uio.uio_offset = wa->wa_offset;
		uio.uio_resid = wa->wa_count;
		if (iovcnt <= N_NFS_SV_IOV)
			piov = NULL;
		++nfs_mbuf_writes;
	}

	/* pass the data portion of the write on to ufs */
	dev = va.va_fsid; /* NB: assumes fsid is dev */
	if (prattached && prenabled(dev))
		++prestohere;

	if (uio.uio_offset < nfs_last_offset) {
		DPRINTF(nfs_order_debug,
			("rfs_write: outoforder: pid %d off %d lastoff %d\n",
			psv->nfs_sv_pid, uio.uio_offset, nfs_last_offset));
		++nfs_writes.outoforder;
	}
	nfs_last_offset = uio.uio_offset;
	DPRINTF(nfs_order_debug,
		("rfs_write: pid %d off %d lastoff %d\n",
		 psv->nfs_sv_pid, uio.uio_offset, nfs_last_offset));

	if (prestohere && !prmetaonly) {
		/* use sync i/o for data */
		/* error = VOP_RDWR(vp, &uio, UIO_WRITE,
				 IO_SYNCDATA, u.u_cred); */
#ifdef CJKTRACE
		if (nfs_cjtrace)
			LOG_SYS_NFS(psv, NFS_WG_WR1_START);
#endif /* CJKTRACE */
		VOP_WRITE(vp, &uio, IO_SYNC | IO_DATAONLY, u.u_cred, error);
#ifdef CJKTRACE
		if (nfs_cjtrace)
			LOG_SYS_NFS(psv, NFS_WG_WR1_DONE);
#endif /* CJKTRACE */
		DPRINTF(nfs_sv_debug && error,
			("rfs_write: pid %d syncdata err %d\n",
			psv->nfs_sv_pid, error));
		++nfs_writes.prestosingles;
	}
	else {
		/* use delay i/o for data */
		/* error = VOP_RDWR(vp, &uio, UIO_WRITE,
				 IO_DELAYDATA, u.u_cred); */
#ifdef CJKTRACE
		if (nfs_cjtrace)
			LOG_SYS_NFS(psv, NFS_WG_WR2_START);
#endif /* CJKTRACE */

		VOP_WRITE(vp, &uio, 0, u.u_cred, error);

#ifdef CJKTRACE
		if (nfs_cjtrace)
			LOG_SYS_NFS(psv, NFS_WG_WR2_DONE);
#endif /* CJKTRACE */
		DPRINTF(nfs_sv_debug && error,
			("rfs_write: pid %d delaydata err %d\n",
			psv->nfs_sv_pid, error));
	}

	if (piov)
		kmem_free(piov, piovlen);

	cc = nfs_sv_socket->so_rcv.sb_cc;
	cc /= 8192;
	if (cc >= 0 && cc < 32)
		nfs_sv_cc_hist[cc]++;
	else
		nfs_sv_hist_err++;

	/* Now, deal with the meta-data */

      metadata:

	DPRINTF(nfs_sv_debug,
		("rfs_write(metadata): pid %d, offset %d\n",
		 psv->nfs_sv_pid, wa->wa_offset));

	psv->nfs_sv_state = NFSSV_META;
	if (dupwrite || estale)	/* going through the motions */
		++dummy;

	/*
	 * See if another thread is writing this file
	 * behind me. If one is, let someone else do the
	 * meta-data.
	 *
	 * NB: we assume that either this thread has *no* vp (estale) or
	 * has vp locked and that threads "behind it" are waiting
	 * just after nfsrv_fhtovp().
	 *
	 * But first,
	 * if there is already an error, and there are outstanding writes
	 * on this fh, then start them up. If this is an estale thread,
	 * assume their vp is ref'd and can't go away, so grab and use it.
	 */
	/* smp_lock(&lk_nfsbiod, LK_RETRY); */
	if (dummy || error) {
		wr = nfs_activewrites;
		while (wr != NULL) {
			if (wr->nwr_fh != NULL &&
			    bcmp((char *)&wa->wa_fhandle,
				 (char *)wr->nwr_fh,
				 NFS_FHSIZE) == 0) {
				/* Someone has writes on this fh. */
				DPRINTF(nfs_dup_debug,
					("dup(2): pid %d, vp 0x%x off %d %s\n",
					psv->nfs_sv_pid, vp, wa->wa_offset,
					(dupwrite)?"dup":""));
				if (estale) {
					/* grab their vnode; */
					/* assume it is ref'd and */
					/* can't go away. */
					vp = wr->nwr_vp;
					psv->nfs_sv_vp = vp;
					/* smp_unlock(&lk_nfsbiod); */
					VN_WRITE_LOCK(vp);
				} else {
					/* smp_unlock(&lk_nfsbiod); */
				}
				goto writemeta; /* start 'em up */
			}
			wr = wr->nwr_next;
		}
	}

	/* If an error already, we have no interest in */
	/* threads behind us. */

	if (dummy || error) {
		DPRINTF(nfs_dup_debug,
			("dup(3): pid %d, vp 0x%x off %d %s\n",
			psv->nfs_sv_pid, vp, wa->wa_offset,
			(dupwrite)?"dup":""));
		/* No threads ahead of us; we're off the hook. */
		/* smp_unlock(&lk_nfsbiod); */
		goto single_thread_done;
	}

	/* There should be *no* dummy or error threads between here */
	/* and writemeta: */

#ifdef CJKTRACE
	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_LOOK1);
#endif /* CJKTRACE */

	/* First, see if there is a sleeper */
	/* smp_lock(&lk_nfsbiod, LK_RETRY); */
	for (i = 0, qsv = nfs_sva;
	     i < nfs_max_daemon;
	     i++, qsv++) {
		if (qsv != psv &&
		    qsv->nfs_sv_which == RFS_WRITE &&
		    qsv->nfs_sv_vp == vp &&
		    qsv->nfs_sv_state == NFSSV_SLEEP) {
			++nfs_writes.more_sleeper;
			/* give up vnode, then spin lock */
			VN_WRITE_UNLOCK(vp);
			/* smp_unlock(&lk_nfsbiod); */
			/* Add this request to list of active writes */
			RFSGETWRITE(mywr); /* has own protection */
			mywr->nwr_nrq = nreq;
			mywr->nwr_fh = &wa->wa_fhandle;
			mywr->nwr_vp = vp;
			mywr->nwr_start = psv->nfs_sv_start;
			mywr->nwr_stop = psv->nfs_sv_stop;
			ADDTOWRLIST(mywr); /* has own protection */
			DPRINTF(nfs_sv_debug,
				("rfs_write(datadone3): pid %d, off %d\n",
				 psv->nfs_sv_pid, wa->wa_offset));
#ifdef CJKTRACE
			if (nfs_cjtrace)
				LOG_SYS_NFS(psv, NFS_WG_DONE);
#endif /* CJKTRACE */
			
			GETLBOLT(&ticks, xprt);
			s = splclock();
			untimeout(nfs_lbolt, (caddr_t)qsv);
			timeout(nfs_lbolt, (caddr_t)qsv,
				ticks); 
			splx(s);
			return (RPCHOLDXPRT);
			
		}
	}
	
	/* Now, see what other daemons are up to. */
	for (i = 0, qsv = nfs_sva;
	     i < nfs_max_daemon;
	     i++, qsv++) {
		if (qsv != psv &&
		    qsv->nfs_sv_which == RFS_WRITE &&
		    qsv->nfs_sv_fh != NULL &&
		    bcmp((char *)&wa->wa_fhandle,
			 (char *)qsv->nfs_sv_fh,
			 NFS_FHSIZE) == 0) {
			/* We can't tell if a thread is necessarily behind */
			/* unless they are blocked in fhtovp(). */
			if (qsv->nfs_sv_state != NFSSV_FHTOVP)
				continue;
			nfs_writes.more_nfsd++;
			/* give up vnode, then spin lock */
			VN_WRITE_UNLOCK(vp);
			/* smp_unlock(&lk_nfsbiod); */
			/* Add this request to list of active writes */
			RFSGETWRITE(mywr); /* has own protection */
			mywr->nwr_nrq = nreq;
			mywr->nwr_fh = &wa->wa_fhandle;
			mywr->nwr_vp = vp;
			mywr->nwr_start = psv->nfs_sv_start;
			mywr->nwr_stop = psv->nfs_sv_stop;
			ADDTOWRLIST(mywr); /* has own protection */
			DPRINTF(nfs_sv_debug,
				("rfs_write(datadone1): pid %d, off %d\n",
				psv->nfs_sv_pid, wa->wa_offset));
#ifdef CJKTRACE
			if (nfs_cjtrace)
				LOG_SYS_NFS(psv, NFS_WG_DONE);
#endif /* CJKTRACE */
			return (RPCHOLDXPRT);
		}
	}

	/* We didn't find another thread behind us.
	 * Finally, see if there is another write request
	 * for this file on the socket buffer.
	 */
	if (prestohere) {
		if (!nfs_domore_presto)
			goto chksleep;
	} else {
		if (!nfs_domore_ufs)
			goto chksleep;
	}
		
	if (nfs_sv_socket->so_rcv.sb_mb == NULL)
		++nfs_writes.emptysb;
	else {
#ifdef CJKTRACE
		if (nfs_cjtrace)
			LOG_SYS_NFS(psv, NFS_WG_LOOK2);
#endif /* CJKTRACE */
		if (nfs_more_writes(&wa->wa_fhandle)) {
			if (prestohere)
				nfs_writes.more_presto_sb++;
			else 
				nfs_writes.more_ufs_sb++;
			/* give up vnode, then spin lock */
			VN_WRITE_UNLOCK(vp);
			/* smp_unlock(&lk_nfsbiod); */
			/* Add this request to list of active writes */
			RFSGETWRITE(mywr); /* has own protection */
			mywr->nwr_nrq = nreq;
			mywr->nwr_fh = &wa->wa_fhandle;
			mywr->nwr_vp = vp;
			mywr->nwr_start = psv->nfs_sv_start;
			mywr->nwr_stop = psv->nfs_sv_stop;
			ADDTOWRLIST(mywr); /* has own protection */
			DPRINTF(nfs_sv_debug,
				("rfs_write(datadone2): pid %d, off %d\n",
				 psv->nfs_sv_pid, wa->wa_offset));
#ifdef CJKTRACE
			if (nfs_cjtrace)
				LOG_SYS_NFS(psv, NFS_WG_DONE);
#endif /* CJKTRACE */
			return (RPCHOLDXPRT);
		}
	}
		
      chksleep:
	/* smp_unlock(&lk_nfsbiod); */
	
	/* Alas, looks like we're going to have to do meta-data ourselves. */
	/* Take a second peek if this was our first search. */
#ifdef CJKTRACE
	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_SLEEP);
#endif /* CJKTRACE */
	if (first) {
		int ticks;
		
		--first;
		if (!nfs_did_ticks) {
			nfs_init_lbolt();
			++nfs_did_ticks;
		}
		
		GETLBOLT(&ticks, xprt);
		
		if (prestohere) {
			if (nfs_presto_lbolt) {
				psv->nfs_sv_state = NFSSV_SLEEP;
				++nfs_writes.presto_sleeps;
				s = splclock();
				timeout(nfs_lbolt, (caddr_t)psv,
					ticks); 
				VN_WRITE_UNLOCK(vp); /* give it up */
				DPRINTF(nfs_sv_debug,
					("rfs_write(sleep1): pid %d\n",
					 psv->nfs_sv_pid));
				sleep((caddr_t)psv, PRIBIO - 1);
				/* ...zzz...zzz... */
				
				splx(s);
				psv->nfs_sv_state = NFSSV_FHTOVP;
				DPRINTF(nfs_sv_debug,
					("rfs_write(wakeup1): pid %d\n",
					 psv->nfs_sv_pid));
				VN_WRITE_LOCK(vp); /* get it again */
				DPRINTF(nfs_sv_debug,
					("rfs_write(gotlock2): pid %d, off %d\n",
					 psv->nfs_sv_pid, wa->wa_offset));
			}
			
#ifdef CJKTRACE
			if (nfs_cjtrace)
				LOG_SYS_NFS(psv, NFS_WG_WAKEUP1);
#endif /* CJKTRACE */
		} else {
			if (nfs_ufs_lbolt) {
				psv->nfs_sv_state = NFSSV_SLEEP;
				++nfs_writes.ufs_sleeps;
				s = splclock();
				timeout(nfs_lbolt, (caddr_t)psv,
					ticks); 
				VN_WRITE_UNLOCK(vp); /* give it up */
				DPRINTF(nfs_sv_debug,
					("rfs_write(sleep2): pid %d\n",
					 psv->nfs_sv_pid));
				sleep((caddr_t)psv, PRIBIO - 1);
				/* ...zzz...zzz... */
				
				splx(s);
				psv->nfs_sv_state = NFSSV_FHTOVP;
				DPRINTF(nfs_sv_debug,
					("rfs_write(wakeup2): pid %d\n",
					 psv->nfs_sv_pid));
				VN_WRITE_LOCK(vp); /* get it again */
				DPRINTF(nfs_sv_debug,
					("rfs_write(gotlock3): pid %d, off %d\n",
					 psv->nfs_sv_pid, wa->wa_offset));
			}
			
#ifdef CJKTRACE
			if (nfs_cjtrace)
				LOG_SYS_NFS(psv, NFS_WG_WAKEUP2);
#endif /* CJKTRACE */
		}
		goto metadata;
	}
	
      writemeta:

	/* We are a dummy cleaning up outstanding writes, an error leg,
	 * or there is no one behind us, so do metadata.
	 * NB: Should be nobody but meta-data writers between here and
	 * `multi_thread_done:'.
	 */
	++writer;
	DPRINTF(nfs_dup_debug && (dummy || error),
		("dup(4): pid %d, vp 0x%x off %d %s\n",
		psv->nfs_sv_pid, vp, wa->wa_offset,
		(dupwrite)?"dup":""));
	nfs_writes.metadata++;

	DPRINTF(nfs_sv_debug,
		("rfs_write(meta): pid %d, off %d list 0x%x mylist 0x%x\n",
		psv->nfs_sv_pid, wa->wa_offset, &nfs_activewrites,
		&mywritelist));

#ifdef CJKTRACE
	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_FSYNC1_START);
#endif /* CJKTRACE */
	/* error = ufs_syncmeta(vp, u.u_cred); */
	VOP_FSYNC(vp, FWRITE | FWRITE_METADATA, u.u_cred,
	          MNT_WAIT, error);
#ifdef CJKTRACE
	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_FSYNC1_DONE);
#endif /* CJKTRACE */

	psv->nfs_sv_state = NFSSV_METADONE;
	mywritelist = NULL;
	i = 0;

      gather:

	if (dummy || error || (prestohere && !prmetaonly)) {
		minoffset = -1;
		maxoffset = -1;
	} else {
		minoffset = psv->nfs_sv_start;
		maxoffset = psv->nfs_sv_stop;
	}

	/* smp_lock(&lk_nfsbiod, LK_RETRY); */
	wr = nfs_activewrites;
	prevwr = (struct nfs_writereq *)&nfs_activewrites;
	/*
	DPRINTF(nfs_sv_debug,
		("rfs_write(gather): wr 0x%x prevwr 0x%x\n", wr, prevwr));
	*/
	while (wr != NULL) {
		if (wr->nwr_vp == vp) {

			/*
			DPRINTF(nfs_sv_debug,
			("rfs_write(vpmatch): wr 0x%x prevwr 0x%x vp 0x%x\n",
				wr, prevwr, vp));
			*/

			if (!imgathering) {
				/* smp_unlock(&lk_nfsbiod); */
				++imgathering;
				/*
				 * Get attributes again so we send the latest
				 * mod time to the client side for its cache.
				 */
				VOP_GETATTR(vp, &va, u.u_cred, attr_error);
				vattr_to_nattr(&va, &ns->ns_attr);
 				goto gather;
			}
			/*
			DPRINTF(nfs_sv_debug,
				("rfs_write(remove): wr 0x%x prevwr 0x%x next 0x%x rq 0x%x\n",
				wr, prevwr, wr->nwr_next, wr->nwr_nrq));
			*/
			REMOVEFROMWRLIST(wr, prevwr); /* has own protection */
			savewr = wr->nwr_next;

			/* build list in ascending offset order */
			if (!mywritelist) {
				wr->nwr_next = mywritelist;
				mywritelist = wr;
			} else {
				struct nfs_writereq *swr, *prevswr;
				swr = mywritelist;
				prevswr = (struct nfs_writereq *)&mywritelist;
				while (swr != NULL) {
					if (swr->nwr_start < wr->nwr_start) {
						prevswr = swr;
						swr = swr->nwr_next;
						continue;
					}
					break;
				}
				wr->nwr_next = swr;
				prevswr->nwr_next = wr;
			}
			++i;
		        if (!prestohere || prmetaonly) {
				if (minoffset < 0 ||
				    wr->nwr_start < minoffset)
					minoffset = wr->nwr_start;
				if (maxoffset < 0 ||
				    wr->nwr_stop > maxoffset)
					maxoffset = wr->nwr_stop;
			}
			wr = savewr;
	        } else {
			/*
			DPRINTF(nfs_sv_debug,
				("rfs_write(no vpmatch): wr 0x%x prevwr 0x%x next 0x%x vp 0x%x\n",
				wr, prevwr, wr->nwr_next, vp));
			*/
			prevwr = wr;
			wr = wr->nwr_next;
		}
        }
	/* smp_unlock(&lk_nfsbiod); */

	DPRINTF(nfs_sv_debug,
		("rfs_write(gatherdone): gathering %d mylist 0x%x i %d\n",
		imgathering, mywritelist, i));

	/* If a lone thread, and there's been an error, bail out */
	if (!imgathering) {
		/* if failed already or duplicate or stale, we're outa here */
		if (dummy || error) {
			DPRINTF(1, ("rfswrite: %s, found no outstanding writes\n",
			       (dummy)?"dummy":"error"));
			goto single_thread_done;
		}
		nfs_writes.failures++;
	}

      syncdata:

#ifdef CJKTRACE
/*	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_SYNCDATA); */
#endif /* CJKTRACE */

	/* prestoized data has been written already */
	if (!prestohere || prmetaonly) {
		if (imgathering) {

/* CJXXX: look at getting rid of min and max offset */

			int error2;
			int tstart, tstop;

			psv->nfs_sv_state = NFSSV_DATA2;
			wr = mywritelist;
			tstart = -1;
			while (wr != NULL) {
				if (tstart < 0) {
					tstart = wr->nwr_start;
					tstop = wr->nwr_stop;
				}
				if (wr->nwr_stop > tstart + nfs_cluster_max ||
				    wr->nwr_next == NULL) {
					DPRINTF(nfs_cl_debug,
						("rfs_write: cluster %d %d\n",
						 tstart, tstop));
					VOP_SYNCDATA(vp, FWRITE, tstart,
						     (tstop - tstart) + 1,
						     u.u_cred, error2);
					if (error2) {
						if (!error)
							error = error2;
						if (!attr_error)
							attr_error = error2;
					}
					++nfs_writes.clusters;
					if (wr->nwr_next == NULL)
						break;
					tstart = -1;
					continue;
				}
				tstop = wr->nwr_stop;
				wr = wr->nwr_next;
			}

#ifdef CJKTRACE
			if (nfs_cjtrace)
				LOG_SYS_NFS(psv, NFS_WG_FSYNC2_DONE1);
#endif /* CJKTRACE */

			goto multi_thread_done;
		}

		/* No one else to worry about but me */
		if (dummy || error) {
			printf("rfs_write: how did this happen???\n");
			goto single_thread_done;
		}
		psv->nfs_sv_state = NFSSV_DATA3;
		DPRINTF(nfs_cl_debug,
			("rfs_write: single4 %d %d\n",
			(int)wa->wa_offset, (int)wa->wa_count));
		VOP_SYNCDATA(vp, FWRITE, minoffset,
			     (maxoffset - minoffset) + 1,
			     u.u_cred, error);

#ifdef CJKTRACE
		if (nfs_cjtrace)
			LOG_SYS_NFS(psv, NFS_WG_FSYNC2_DONE2);
#endif /* CJKTRACE */

		++nfs_writes.singles;
		goto single_thread_done;
	}

	if (!imgathering)
		goto single_thread_done;

	/* I am responsible for outstanding writes. */

      multi_thread_done:
#ifdef CJKTRACE
/*	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_MULTI_DONE);*/
#endif /* CJKTRACE */

	if (!error)
		error = attr_error;
	ns->ns_status = puterrno(error);
	tns = *ns;	/* save a copy of attributes */

	/*
	DPRINTF(nfs_sv_debug,
		("rfs_write(mtd): wr 0x%x mylist 0x%x next 0x%x\n",
		wr, mywritelist, mywritelist->nwr_next));
	*/
	wr = mywritelist;

	while (wr != NULL) {
		struct nfsattrstat *ns2;
		struct nfs_req *rq2;
		SVCXPRT *savexprt;

#ifdef CJKTRACE
		if (nfs_cjtrace)
			LOG_SYS_NFS(psv, NFS_WG_MULTI_DONE1);
#endif /* CJKTRACE */


		/*
		 * Preserve offset order of replies.
		 * See if my offset is less than cached offset.
		 * Send reply for mine if it is.
		 */
		if (!estale && !didmyreply &&
		    psv->nfs_sv_start < wr->nwr_start) {
			DPRINTF(nfs_sv_debug,
				("rfs_write(domine): off %d err %d %s\n",
				 psv->nfs_sv_start, error, (dupwrite)? "dup":""));
#ifdef CJKTRACE
/*	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_DIDMYREPLY);*/
#endif /* CJKTRACE */

			++didmyreply;
			++nfs_writes.early;
			/* NB: careful with nreq after this!! */
			rfs_sendreply(nreq, 0);
		}

		rq2 = (struct nfs_req *)wr->nwr_nrq;
		ns2 = (struct nfsattrstat *)rq2->nrq_res;
		*ns2 = tns;
		savexprt = rq2->nrq_xprt;

#ifdef CJKTRACE
/*		if (nfs_cjtrace)
			LOG_SYS_NFS(psv, NFS_WG_MULTI_DONE2);*/
#endif /* CJKTRACE */

		rfs_sendreply(rq2, 0);	/* does the RFSPUTREQ() */

#ifdef CJKTRACE
/*		if (nfs_cjtrace)
			LOG_SYS_NFS(psv, NFS_WG_MULTI_DONE3);*/
#endif /* CJKTRACE */

		RFSPUTXPRT(savexprt);
		vrele(wr->nwr_vp); /* one for each outstanding reply */
		DPRINTF(nfs_sv_debug,
			("rfs_write(while2done): off %d err %d %s\n",
			wr->nwr_start, error, (dupwrite)? "dup":""));
		savewr = wr;
		wr = wr->nwr_next;
		RFSPUTWRITE(savewr);

#ifdef CJKTRACE
/*		if (nfs_cjtrace)
			LOG_SYS_NFS(psv, NFS_WG_MULTI_DONE4);*/
#endif /* CJKTRACE */

	}
	goto out;

	/* Threads that get here need *no* synchronization w/others */

      single_thread_done:
#ifdef CJKTRACE
/*	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_SINGLE_DONE);*/
#endif /* CJKTRACE */

	if (!estale) {
		if (!error) {
			/*
			 * Get attributes again so we send the latest mod
			 * time to the client side for his cache.
			 */
			VOP_GETATTR(vp, &va, u.u_cred, error);
		}
		if (!error) {
			vattr_to_nattr(&va, &ns->ns_attr);
		}
		ns->ns_status = puterrno(error);
	}

      out:
#ifdef CJKTRACE
/*	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_OUT1);*/
#endif /* CJKTRACE */

	if (!didmyreply && !estale) {
		/* NB: careful with nreq after this!! */
		rfs_sendreply(nreq, 0);
		++didmyreply;
	}

#ifdef CJKTRACE
/*	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_OUT2);*/
#endif /* CJKTRACE */

	if (vp) {
		/* all but estales that did not grab a vnode */
		VN_WRITE_UNLOCK(vp);
		vrele(vp);

	}

	DPRINTF(nfs_sv_debug,
		("rfs_write(done): pid %d, off %d err %d %s\n",
		psv->nfs_sv_pid, psv->nfs_sv_start, error,
		(dupwrite)? "dup":""));

#ifdef CJKTRACE
	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_METADONE);
#endif /* CJKTRACE */

	if (didmyreply)
		return (RPCNOREPLY);
	else
		return (0);
}

static
mbuf_to_iov(m, iov)
	struct mbuf *m;
	struct iovec *iov;
{

	while (m) {
		iov->iov_base = mtod(m, caddr_t);
		iov->iov_len = m->m_len;
		iov++;
		m = m->m_next;
	}
}

int nfs_lbolt(psv)
	struct nfs_svinfo *psv;
{

#ifdef CJKTRACE
	if (nfs_cjtrace)
		LOG_SYS_NFS(psv, NFS_WG_LBOLT);
#endif /* CJKTRACE */
	wakeup(psv);
}

/*
 * This function looks down the socket queue, looking for write operations
 * on the specified filehandle.  Its disregard for locking is shameless.
 * its inattention to proper layering is equally disgraceful.
 */
int nfs_more_writes(fhp)
nfsv2fh_t	*fhp;
{
	struct mbuf *m;
	u_char *d;
	int proc,fh,count,credlen;
	int s, i;
#ifdef DEBUG
	u_char *dfhp;
#endif /* DEBUG */

	nfs_mw.calls++;

#ifdef DEBUG
	if (nfs_mw.debug) {
		int i;
		
		printf("nfs_mw: fh 0x%x: \n", fhp);
		for (i=0; i<32; i++)
			printf("%2x ",
			       (u_char)fhp->fh_bytes[i]);
		printf("\n");
	}
#endif /* DEBUG */

	s=splnet();
        SOCKET_LOCK(nfs_sv_socket);
        SOCKBUF_LOCK(&nfs_sv_socket->so_rcv);

	m = nfs_sv_socket->so_rcv.sb_mb;

	if (m == NULL) {
                SOCKBUF_UNLOCK(&nfs_sv_socket->so_rcv);
                SOCKET_UNLOCK(nfs_sv_socket);
		splx(s);
		nfs_mw.emptysb++;
		return (0);
	}

	do {
                count = 0;
                proc = 0x27;  /* proc is at offset 27 - magic */
                credlen = 0x2f; /* offset of cred length - more magic */
                DPRINTF (nfs_mw.debug, ("\nnfs_more_writes\n"));

#ifdef DEBUG
                if (nfs_mw.debug) {
                        int i;
			printf("\n1st mbuf m_len %d\n", m->m_len);
			if (m->m_len > 0) {
				d = mtod(m, u_char*);
				printf("     ");
				for (i = 0; i < 80 && i < m->m_len; i++) {
					printf("%2x ",(u_char)d[i]);
					if ((i % 32) == 31)
						printf("\n%4x ",i+1);
				}
			}
                        printf("\n");
                }
#endif

                while (proc > m->m_len) {
                        proc -= m->m_len;
                        count += m->m_len;
                        DPRINTF (nfs_mw.debug, 
		("step(1) to next mbuf %x (len %d)\n", count, m->m_len));
                        m = m->m_next;
                        if (m == NULL)
                                break;
                }

                if (m == NULL) {
                        DPRINTF (nfs_mw.print_empties,
                                ("nfs_more_writes: empty mbuf chain!\n"));
			nfs_mw.emptychn++;
                        continue;
                }

		d = mtod(m, u_char*);
#ifdef DEBUG
                if (nfs_mw.debug) {
                        int i;
                        printf("count %d %x len %d %x proc %d %x\n",
				count,count,m->m_len,m->m_len,proc,proc);
			printf("     ");
                        for (i = 0; i <= proc; i++) {
                                printf("%2x ",(u_char)d[i]);
                                if ((i % 32) == 31)
                                        printf("\n%4x ",i+1);
                        }
                        printf("\n");
                }
#endif
		if (d[proc] != RFS_WRITE)
			continue;

		nfs_mw.found_write++;

                fh = 0x28 + d[credlen - count]; /* yet more magic */

                DPRINTF (nfs_mw.debug,
	("proc=WRITE fh at 0x%x - credlen is %d\n",fh, d[credlen - count]));

                while (fh > m->m_len) {
                        fh -= m->m_len;
                        m = m->m_next;
                        DPRINTF (nfs_mw.debug,
		("step(2) to next mbuf fh=%x (len %d)\n",fh,m->m_len));
                        if (m == NULL)
                                break;
                }

                if (m == NULL) {
                        DPRINTF (nfs_mw.print_empties, 
			("nfs_more_writes: partial write datagram!\n"));
			nfs_mw.partial++;
                        continue;
                }
#ifdef DEBUG
                if (nfs_mw.debug) {
                        int i;
                        printf(" fh %d %x",fh,fh);
                        printf(" mlen %d\n",m->m_len);
			printf("     ");
                        for (i = 0; i < fh+80 && i < m->m_len; i++) {
                                printf("%2x ",(u_char)d[i]);
                                if ((i % 32) == 31)
                                        printf("\n%4x ",i+1);
                        }
                        printf("\n");
                }
#endif

                if (fh+NFS_FHSIZE > m->m_len) {
                        printf("filehandle spans mbufs\n");
			nfs_mw.badfh++;
                        continue;
                }

		d = mtod(m, u_char*);

#ifdef DEBUG
		dfhp=(u_char *)fhp;
		DPRINTF (nfs_mw.cmpdebug, ("fhp is %x %x %x %x\n",
			dfhp[0], dfhp[4], dfhp[8], dfhp[12]));
		
		DPRINTF (nfs_mw.cmpdebug, ("fh is %x %x %x %x\n",
			d[fh], d[fh+4], d[fh+8], d[fh+12]));
#endif /* DEBUG */
		
		nfs_mw.cmpfh++;
		if (bcmp(fhp, (u_char *)(d+fh), NFS_FHSIZE) == 0){
			splx(s);
			nfs_mw.hits++;
			return (1);
		}
	} while ((m = m->m_act) != NULL);

	nfs_mw.miss++;
	splx(s);
	return (0);

}

/*
 * Create a file.
 * Creates a file with given attributes and returns those attributes
 * and an fhandle for the new file.
 */
int
rfs_create(args, dr, nreq, xprt, xpd)
	struct nfscreatargs *args;
	struct  nfsdiropres *dr;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	int error = 0;
	struct vnode *vp;
	struct vnode *dvp;
	struct vattr va, dva, eva;
	char *name = args->ca_da.da_name;
	struct nameidata *ndp = &u.u_nd;
	nfsv2fh_t *fhp;

	DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
		("rfs_create: %s dfh fsid %x %x inode %d %d\n",
		name, args->ca_da.da_fhandle.fh_generic.fh_fsid.val[0],
		args->ca_da.da_fhandle.fh_generic.fh_fsid.val[1],
		args->ca_da.da_fhandle.fh_generic.fh_fid.fid_data[0],
		args->ca_da.da_fhandle.fh_generic.fh_fid.fid_data[1]));

	/*
	 *	Disallow NULL paths
	 */
	if ((name == (char *) NULL) || (*name == '\0')) {
		dr->dr_status = NFSERR_NOENT;
		return (0);
	}

	dvp = nfsrv_fhtovp(&args->ca_da.da_fhandle, xprt, RFS_CREATE, xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
		("create: fhtovp returns 0x%x\n", dvp));

#ifdef	KTRACE
	kern_trace(309,vp,0,0);
#endif  /* KTRACE */
	if (!dvp) {
		dr->dr_status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
			("create: return ESTALE %d\n", ESTALE));
		return (0);
	}

	/* From here on, all returns should be via `goto done' */

	if (rdonly(dvp, xpd, xprt)) {
		error = EROFS;
		DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
			("create: readonly fs\n"));
		goto done;
	}

	/* CJXXX: Is this really NFS's business?? */
	error = nfsrv_access(dvp, VWRITE, u.u_cred);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
			("create: access returns %d\n", error));
		goto done;
	}

	sattr_to_vattr(&args->ca_sa, &va);
	DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
		("create(1): type %d, mode 0x%x, size %d\n",
		       va.va_type, va.va_mode, va.va_size));

	/*
	 * This is a completely gross hack to make mknod
	 * work over the wire until we can wack the protocol
	 */
#define IFMT		0170000		/* type of file */
#define IFCHR		0020000		/* character special */
#define IFBLK		0060000		/* block special */
#define	IFSOCK		0140000		/* socket */
	if ((va.va_mode & IFMT) == IFCHR) {
		va.va_type = VCHR;
		if (va.va_size == (u_int)NFS_FIFO_DEV)
			va.va_type = VFIFO; /* xtra kludge for named pipe */
		else
			va.va_rdev = (dev_t)va.va_size;
		va.va_size = 0;
	} else if ((va.va_mode & IFMT) == IFBLK) {
		va.va_type = VBLK;
		va.va_rdev = (dev_t)va.va_size;
		va.va_size = 0;
	} else if ((va.va_mode & IFMT) == IFSOCK) {
		va.va_type = VSOCK;
	} else {
		va.va_type = VREG;
	}
	va.va_mode &= ~IFMT;

	DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
		("create(2): type %d, mode 0x%x, size %d\n",
		       va.va_type, va.va_mode, va.va_size));

	ndp->ni_cdir = dvp;
	ndp->ni_cred = u.u_cred;
	ndp->ni_nameiop = CREATE | WANTPARENT;
	ndp->ni_segflg = UIO_SYSSPACE;
	ndp->ni_dirp = name;
	DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
		("create: call namei (creating: %s)\n", name));
	error = namei(ndp);
	vp = ndp->ni_vp;
	DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
		("create: namei returns 0x%x %d\n", vp, error));
	if (error)
		goto done;

	DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
		("create: ref on dvp after namei %d\n",	dvp->v_usecount));

	if (!vp) { /* file doesn't exist - true create */
		if (va.va_type == VREG) {
			VOP_CREATE(ndp, &va, error);
			DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
				("create: back from vop_create (%d)\n", error));
			if (error)
				goto done;
		} else {
			VOP_MKNOD(ndp, &va, u.u_cred, error);
			DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
				("create: back from vop_mknod (%d)\n", error));
			if (error)
				goto done;
			/* 
			 * Because of a problem with the VFS interface,
			 * we need to reaquire the vp of the file we just
			 * created so that we can return attributes to the
			 * client.  The only means we have is namei. Sigh.
			 */
			ndp->ni_nameiop = LOOKUP | NOCROSSMOUNT;
			ndp->ni_segflg = UIO_SYSSPACE;
			ndp->ni_dirp = name;
			error = namei(ndp);
			if (error)
				goto done;
			
		}
		vp = ndp->ni_vp;

	/* From here on, all returns should be via `goto out' */

	} else { /* file exists, turn into over-write */
		/* VFS cruft */
		ndp->ni_vp = (struct vnode *) NULL;
		VOP_ABORTOP(ndp, error);
		vrele(dvp);

		/* make sure we have write permission on file */
		error = nfsrv_access(vp, VWRITE, u.u_cred);
		if (error) {
			DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
				("create: access(2) returns %d\n", error));
			goto out;
		}

		VOP_GETATTR(vp, &eva, u.u_cred, error);
		if (error) {
			DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
				("create: vop_getattr2 failed %d\n", error));
			goto out;
		}

		/* make sure it's a regular file */
		if (eva.va_type != VREG) {
			error = EEXIST;
			DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
				("create: special/dir exists%d\n"));
			goto out;
		}

		/* Clear out what can't be changed on an existing file */
		va.va_type = VNON;
		va.va_uid = (uid_t)VNOVAL;
		va.va_gid = (gid_t)VNOVAL;
		va.va_mode = (u_short)VNOVAL;
		/*
		 * Use the size supplied by the client.  Truncating to 0
		 * on open is a UNIX semantic.  If the client wants the
		 * size to be 0 it will set this in the attributes.
		 * va.va_size = 0;
		 */

		VOP_SETATTR(vp, &va, u.u_cred, error);
		if (error) {
			DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
				("create: vop_setattr returns %d\n", error));
#ifdef	DEBUG
       printf("   type 0x%x nlink 0x%x fsid 0x%x fileid 0x%x\n",
	      va.va_type, va.va_nlink, va.va_fsid, va.va_fileid);
       printf("   bs 0x%x rdev 0x%x bytes 0x%x gen 0x%x\n",
	      va.va_blocksize, va.va_rdev, va.va_bytes, va.va_gen);
#endif
			goto out;
		}
	}
	DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
		("create: ref on vp %d\n", vp->v_usecount));

	VOP_GETATTR(vp, &va, u.u_cred, error);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
			("create: vop_getattr3 failed %d\n", error));
		goto out;
	}
	DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
		("create: vop_getattr2 returns fsid %x file id %d\n",
		       va.va_fsid, va.va_fileid));

	vattr_to_nattr(&va, &dr->dr_attr);

	/*
	 * N.B.: there's a slim chance that this vnode has been vgone'd,
	 * in which case v_mount points to dead_mount.  If this is the
	 * case, then we can depend on an error from the VFS_VPTOFH call.
	 */
	fhp = &dr->dr_fhandle;
	fhp->fh_generic.fh_fsid = vp->v_mount->m_stat.f_fsid;
	VFS_VPTOFH(vp, (struct fid *)&fhp->fh_generic.fh_fid, error);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
			("create: vop_vptofh returns %d\n", error));
		goto out;
	}

	/*
	 *  Check to see if we can support this file system
	 */
	if ( sizeof(fsid_t) + fhp->fh_generic.fh_fid.fid_len +
	    xpd->e_fid.fid_len > NFS_FHSIZE ) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
			("create: fhandle too big\n"));
		error = EREMOTE;
		goto out;
	}

	fid_copy(&xpd->e_fid, &fhp->fh_generic.fh_efid);
	/* gfs_unlock(vp); ultrix SMP*/

      out:

	vrele(vp);

      done:

	if (!error) {
		VOP_GETATTR(dvp, &dva, u.u_cred, error);
		DPRINTF(error && (nfsdebug || nfsopdebug[RFS_CREATE]),
			("create: vop_getattr3 failed %d\n", error));
	}

	vrele(dvp);	/* one for fhtovp */

	dr->dr_status = puterrno(error);
	DPRINTF(nfsdebug || nfsopdebug[RFS_CREATE],
		("create: return %d\n", error));
	return (0);
}

/*
 * Remove a file.
 * Remove named file from parent directory.
 */
int
rfs_remove(da, status, nreq, xprt, xpd)
	struct nfsdiropargs *da;
	enum nfsstat *status;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	int error = 0, aerror;
	struct vnode *vp;
	struct vnode *dvp;
	struct vattr dva;
	char *name = da->da_name;
	struct nameidata *ndp = &u.u_nd;
	nfsv2fh_t *fhp;

	DPRINTF(nfsdebug || nfsopdebug[RFS_REMOVE],
		("rfs_remove: %s dfh fsid %x %x inode %d %d\n",
		name, da->da_fhandle.fh_generic.fh_fsid.val[0],
		da->da_fhandle.fh_generic.fh_fsid.val[1],
		da->da_fhandle.fh_generic.fh_fid.fid_data[0],
		da->da_fhandle.fh_generic.fh_fid.fid_data[1]));

	/*
	 *	Disallow NULL paths
	 */
	if ((name == (char *) NULL) || (*name == '\0')) {
		*status = NFSERR_NOENT;
		return (0);
	}

	dvp = nfsrv_fhtovp(&da->da_fhandle, xprt, RFS_REMOVE, xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_REMOVE],
		("remove: fhtovp returns 0x%x\n", vp));

#ifdef	KTRACE
	kern_trace(310,dvp,0,0);
#endif  /* KTRACE */
	if (!dvp) {
		*status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_REMOVE],
			("remove: return ESTALE %d\n", ESTALE));
		return (0);
	}

	/* From here on, all returns should be via `goto done' */

	if (rdonly(dvp, xpd, xprt)) {
		error = EROFS;
		DPRINTF(nfsdebug || nfsopdebug[RFS_REMOVE],
			("remove: readonly fs\n"));
		goto done;
	}

	error = nfsrv_access(dvp, VWRITE, u.u_cred);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_REMOVE],
			("remove: access returns %d\n", error));
		goto done;
	}

	ndp->ni_cdir = dvp;
	ndp->ni_cred = u.u_cred;
	ndp->ni_nameiop =  DELETE | WANTPARENT;
	ndp->ni_segflg = UIO_SYSSPACE;
	ndp->ni_dirp = name;
	DPRINTF(nfsdebug || nfsopdebug[RFS_REMOVE],
		("remove: call namei (removing: %s)\n", name));
	error = namei(ndp);
	vp = ndp->ni_vp;
	DPRINTF(nfsdebug || nfsopdebug[RFS_REMOVE],
		("remove: namei returns 0x%x %d\n", vp, error));
	if (error)
		goto done;

	DPRINTF(nfsdebug || nfsopdebug[RFS_REMOVE],
		("remove: ref on dvp after namei %d\n", dvp->v_usecount));

	/* From here on, all returns should be via `goto out' */

/*
 * NOTE:  may want to look into sharing more with unlink(), with something
 *	  like vn_remove.  The rest of this function is straight from
 *	  unlink.
 */

	if (vp->v_type == VDIR) {
		error = EISDIR;
		goto out;
	}

	/*
	 * Don't unlink a mounted file.
	 */
	if (vp->v_flag & VROOT) {
		error = EBUSY;
		goto out;
	}

      out:

	if (!error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_REMOVE],
			("remove:  (ref on dvp %d)", dvp->v_usecount));
		VOP_REMOVE(ndp, error);
		DPRINTF(nfsdebug || nfsopdebug[RFS_REMOVE],
			("  error %d (ref on dvp %d)\n", error,
			dvp->v_usecount));
	} else {
		DPRINTF(nfsdebug || nfsopdebug[RFS_REMOVE],
			("remove: error %d (ref on dvp %d)\n", error,
			       dvp->v_usecount));
		VOP_ABORTOP(ndp, aerror);
		vrele(dvp); /* one for namei */
		vrele(vp);
	}

      done:

	if (!error) {
		VOP_GETATTR(dvp, &dva, u.u_cred, error);
		DPRINTF(error && (nfsdebug || nfsopdebug[RFS_REMOVE]),
			("remove: vop_getattr2 failed %d\n", error));
	}

	vrele(dvp);	/* one for fhtovp */

	*status = puterrno(error);

	DPRINTF(nfsdebug || nfsopdebug[RFS_REMOVE],
		("remove: return %d (ref on dvp %d)\n", error,
		dvp->v_usecount));
	return (0);
}


/*
 * rename a file
 * Give a file (fname in fdvp) a new name (tname in tdvp).
 */
int
rfs_rename(args, status, nreq, xprt, xpd)
	struct nfsrnmargs *args;
	enum nfsstat *status;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	int error = 0, aerror;
	struct vnode *fdvp;
	struct vnode *tdvp;
	struct vnode *fvp;
	struct vnode *tvp;
	struct vattr va;
	char *fname = args->rna_from.da_name;
	char *tname = args->rna_to.da_name;
	struct nameidata *ndp = &u.u_nd;
	struct nameidata tond;
	struct utask_nd tutnd;
	struct timeval duptime;
	int stripslash;

	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rfs_rename: %s ffh %x %x %d -> %s tfh %x %x %d\n",
		fname,
		args->rna_from.da_fhandle.fh_generic.fh_fsid.val[0],
		args->rna_from.da_fhandle.fh_generic.fh_fsid.val[1],
		args->rna_from.da_len,
		tname,
		args->rna_to.da_fhandle.fh_generic.fh_fsid.val[0],
		args->rna_to.da_fhandle.fh_generic.fh_fsid.val[1],
		args->rna_to.da_len));

#ifdef	KTRACE
	kern_trace(311,0,0,0);
#endif  /* KTRACE */
	/*
	 *	Disallow NULL paths
	 */
	if ((args->rna_from.da_name == (char *) NULL) ||
	    (*args->rna_from.da_name == '\0') ||
	    (args->rna_to.da_name == (char *) NULL) ||
	    (*args->rna_to.da_name == '\0')) {
		*status = NFSERR_NOENT;
		return (0);
	}

	fdvp = nfsrv_fhtovp(&args->rna_from.da_fhandle, xprt, RFS_RENAME,
			      xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rename: fhtovp1 returns 0x%x %d\n", fdvp));

	if (!fdvp) {
		*status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
			("rename: return ESTALE %d\n", ESTALE));
		return (0);
	}

	tdvp = NULL;
	tond.ni_utnd = NULL; /* prevents ndrele on error paths before nddup */
	/* From here on, all returns should be via `goto done' */

	if (rdonly(fdvp, xpd, xprt)) {
		error = EROFS;
		DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
			("rename: readonly fs\n"));
		goto done;
	}

	error = nfsrv_access(fdvp, VWRITE, u.u_cred);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
			("rename: access1 returns %d\n", error));
		goto done;
	}

	tdvp = nfsrv_fhtovp(&args->rna_to.da_fhandle, xprt, RFS_RENAME, xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rename: fhtovp2 returns 0x%x\n", tdvp));

	if (!tdvp) {
		*status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
			("rename: return ESTALE2 %d\n", ESTALE));
		goto done;
	}

	if (rdonly(tdvp, xpd, xprt)) {
		error = EROFS;
		DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
			("rename: readonly fs\n"));
		goto done;
	}

	if (error = nfsrv_access(tdvp, VWRITE, u.u_cred)) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
			("rename: access2 returns %d\n", error));
		goto done;
	}

	/*
	 * XXX namei hack. We have to lookup the from name to see
	 * if it is a directory. Semantics of namei() require the
	 * STRIPSLASH flag set when operating on directories.
	 * Stolen from rename()
	 */
	fvp = NULL;
	ndp->ni_cdir = fdvp;
	ndp->ni_cred = u.u_cred;
	ndp->ni_nameiop =  LOOKUP;
	ndp->ni_segflg = UIO_SYSSPACE;
	ndp->ni_dirp = fname;
	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rename: call namei1 (lookup: %s)\n", fname));

	error = namei(ndp);
	fvp = ndp->ni_vp;
	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rename: namei1 lookup returns 0x%x %d\n", fvp, error));
	if (error)
		goto done;

	if (fvp->v_type == VDIR)
		stripslash = STRIPSLASH;
	else
		stripslash = 0;
	vrele(fvp);
			
	fvp = NULL;
	ndp->ni_cdir = fdvp;
	ndp->ni_cred = u.u_cred;
	ndp->ni_nameiop =  DELETE | WANTPARENT | stripslash;
	ndp->ni_segflg = UIO_SYSSPACE;
	ndp->ni_dirp = fname;
	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rename: call namei1 (renaming: %s)\n", fname));

	error = namei(ndp);
	fvp = ndp->ni_vp;
	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rename: namei1 returns 0x%x %d\n", fvp, error));
	if (error)
		goto done;
	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rename: (ref5 on fdvp %d, fvp %d)\n",
		fdvp->v_usecount, (fvp)?fvp->v_usecount:666));

/*
	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rename: ref on fdvp after namei %d\n", fdvp->v_usecount));
*/

	/* From here on, all returns should be via `goto out1' */

	tvp = NULL;
	tond.ni_utnd = &tutnd;
	nddup(ndp, &tond);  /* puts ref on cdir(fdvp), rdir, and cred) */
	tond.ni_cdir = tdvp; /* change ni_cdir, must reset before ndrele() */
	tond.ni_nameiop = RENAME | WANTPARENT | stripslash;
	tond.ni_segflg = UIO_SYSSPACE;
	tond.ni_dirp = tname;
	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rename: call namei2 (renaming: %s)\n", tname));
	error = namei(&tond);
	tvp = tond.ni_vp;
	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rename: namei2 returns 0x%x %d\n", tvp, error));
	if (error)
		goto out1; /* clean up after namei1 */

	/* From here on, all returns should be via `goto out2' */

	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rename: (ref6 on fdvp %d, fvp %d)\n",
		fdvp->v_usecount, (fvp)?fvp->v_usecount:666));
	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rename: (ref6 on tdvp %d, tvp %d)\n",
		tdvp->v_usecount, (tvp)?tvp->v_usecount:666));

	if (tvp) {
		if (fvp->v_type == VDIR && tvp->v_type != VDIR) {
			error = EISDIR;
			goto out2;
		}
		if (fvp->v_type != VDIR && tvp->v_type == VDIR) {
			error = ENOTDIR;
			goto out2;
		}
	}

	if (fvp->v_mount != tdvp->v_mount) {
		error = EXDEV;
		goto out2;
	}

	if (fvp == tdvp || fvp == tvp) {
		error = EINVAL;
		goto out2;
	}

	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rename: (ref on fdvp %d, tdvp %d)",
		fdvp->v_usecount, tdvp->v_usecount));
	VOP_RENAME(ndp, &tond, error);
#ifdef	DEBUG
	if (nfsdebug || nfsopdebug[RFS_RENAME]) {
		printf("rename: vop_rename returns %d\n", error);
		printf("rename: (ref on fdvp %d, tdvp %d)\n",
		       fdvp->v_usecount, tdvp->v_usecount);
	}
#endif
	goto done;

      out2:

	if (error) { /* clean up after namei2 */
		VOP_ABORTOP(&tond, aerror);
		vrele(tdvp);
		if (tvp)
			vrele(tvp);
	}

      out1:

	if (error) { /* clean up after namei1 */
		VOP_ABORTOP(ndp, aerror);
		vrele(fdvp);
		if (fvp)
			vrele(fvp);
	}

      done:

	if (!error) {
		VOP_GETATTR(fdvp, &va, u.u_cred, error);
		DPRINTF(error && (nfsdebug || nfsopdebug[RFS_RENAME]),
			("rename: vop_getattr2 failed %d\n", error));
	}

	vrele(fdvp);	/* one for fhtovp */
	if (tdvp)
		vrele(tdvp);	/* one for fhtovp */

	if (tond.ni_utnd) {
		tond.ni_cdir = fdvp; /* put back original vp VREF'd in nddup */
		ndrele(&tond);
	}
	*status = puterrno(error);

	DPRINTF(nfsdebug || nfsopdebug[RFS_RENAME],
		("rename: return %d (ref on fdvp %d, tdvp %d)\n", error,
		fdvp->v_usecount, tdvp->v_usecount));

	return (0);
}

/*
 * Link to a file.
 * Create a file (name in tdvp) which is a hard link to the given file (fvp).
 */
int
rfs_link(args, status, nreq, xprt, xpd)
	struct nfslinkargs *args;
	enum nfsstat *status;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	int error = 0, aerror;
	struct vnode *fvp;
	struct vnode *tdvp;
	struct vnode *newvp;
	struct vattr va;
	struct nameidata *ndp = &u.u_nd;
	register char *name = args->la_to.da_name;
	nfsv2fh_t *fhp;

#ifdef	KTRACE
	kern_trace(312,0,0,0);
#endif  /* KTRACE */
	DPRINTF(nfsdebug || nfsopdebug[RFS_LINK],
		("rfs_link: name %s dfh fsid %x %x inode %d %d\n",
		name, args->la_from.fh_generic.fh_fsid.val[0],
		args->la_from.fh_generic.fh_fsid.val[1],
		args->la_from.fh_generic.fh_fid.fid_data[0],
		args->la_from.fh_generic.fh_fid.fid_data[1]));

	/*
	 *	Disallow NULL paths
	 */
	if ((name == (char *) NULL) || (*name == '\0')) {
		*status = NFSERR_NOENT;
		return (0);
	}

	fvp = nfsrv_fhtovp(&args->la_from, xprt, RFS_LINK, xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_LINK],
		("link: fhtovp1 returns 0x%x %d\n", fvp, error));

	if (!fvp) {
		*status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_LINK],
			("link: return ESTALE1 %d\n", ESTALE));
		return (0);
	}

	tdvp = NULL;
	/* From here on, all returns should be via `goto done' */

	/* there is sort of an assumption here that tdvp is within */
	/* the same export domain as the fvp and that sfs code will */
	/* worry about cross filesystem links */

	tdvp = nfsrv_fhtovp(&args->la_to.da_fhandle, xprt, RFS_LINK, xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_LINK],
		("link: fhtovp2 returns 0x%x %d\n", tdvp, error));

	if (!tdvp) {
		*status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_LINK],
			("link: return ESTALE2 %d\n", ESTALE));
		goto done;
	}

	if (rdonly(tdvp, xpd, xprt)) {
		error = EROFS;
		DPRINTF(nfsdebug || nfsopdebug[RFS_LINK],
			("link: readonly to fs\n"));
		goto done;
	}

	error = nfsrv_access(tdvp, VWRITE, u.u_cred);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_LINK],
			("link: access returns %d\n", error));
		goto done;
	}

	newvp = NULL;
	ndp->ni_cdir = tdvp;
	ndp->ni_cred = u.u_cred;
	ndp->ni_nameiop =  CREATE | WANTPARENT;
	ndp->ni_segflg = UIO_SYSSPACE;
	ndp->ni_dirp = name;
	DPRINTF(nfsdebug || nfsopdebug[RFS_LINK],
		("link: call namei (creating: %s)\n", name));
	error = namei(ndp);
	newvp = ndp->ni_vp;
	DPRINTF(nfsdebug || nfsopdebug[RFS_LINK],
		("link: namei returns 0x%x %d\n", newvp, error));
        if (error)
		goto done;

	DPRINTF(nfsdebug || nfsopdebug[RFS_LINK],
		("link: ref on tdvp after namei %d\n", tdvp->v_usecount));

	/* From here on, all returns should be via `goto out' */

	if (newvp) {
		error = EEXIST;
		goto out;
	}

	/*
	 * N.B.:  If forceable unmount is ever working, it is possible that
	 *	  both vnodes could have been vgone'd, which would make
	 *	  their v_mount fields equivalent, but invalid.  We rely
	 *	  on an error from one of the VOP* functions to deal with
	 *	  this.
	 */
	if (fvp->v_mount != tdvp->v_mount)
		error = EXDEV;

      out:

	if (!error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_LINK],
			("link: (ref on fvp %d, tdvp %d)",
			fvp->v_usecount, tdvp->v_usecount));
		VOP_LINK(fvp, ndp, error);
#ifdef	DEBUG
		if (nfsdebug || nfsopdebug[RFS_LINK]) {
			printf("link: vop_link returns %d\n", error);
			printf("link: (ref on fvp %d, tdvp %d)\n",
			       fvp->v_usecount, tdvp->v_usecount);
		}
#endif
	} else {
		DPRINTF(nfsdebug || nfsopdebug[RFS_LINK],
			("link: error %d (ref on fvp %d, tdvp %d)\n",
			error, fvp->v_usecount, tdvp->v_usecount));
		VOP_ABORTOP(ndp, aerror);
		vrele(tdvp); /* one for namei */
		if (newvp) /* for safety */
			vrele(newvp);
	}

      done:

	if (!error) {
		VOP_GETATTR(fvp, &va, u.u_cred, error);
		DPRINTF(error && (nfsdebug || nfsopdebug[RFS_LINK]),
			("link: vop_getattr2 failed %d\n", error));
	}

	vrele(fvp);	/* one for fhtovp */
	if (tdvp)
		vrele(tdvp);	/* one for fhtovp */

	*status = puterrno(error);

#ifdef	DEBUG
	if (nfsdebug || nfsopdebug[RFS_LINK]) {
		printf("link: return %d (ref on fvp %d, tdvp %d)\n",
		       error, fvp->v_usecount, tdvp->v_usecount);
		printf("link: (ref on newvp %d)\n",
		       newvp?newvp->v_usecount:666);
	}
#endif

	return (0);

}

/*
 * Symbolically link to a file.
 * Create a file (fname in dvp) with the given attributes which is a
 * symbolic link to the given path name (tname).
 */
int
rfs_symlink(args, status, nreq, xprt, xpd)
	struct nfsslargs *args;
	enum nfsstat *status;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	int error = 0, aerror;
	struct vnode *dvp;
	struct vnode *vp;
	struct vattr va, dva;
	char *tname = args->sla_tnm;
	char *fname = args->sla_from.da_name;
	struct nameidata *ndp = &u.u_nd;
	nfsv2fh_t *fhp;

#ifdef	KTRACE
	kern_trace(313,0,0,0);
#endif  /* KTRACE */
	DPRINTF(nfsdebug || nfsopdebug[RFS_SYMLINK],
		("symlink: from %s to %s dfh fsid %x %x inode %d %d\n",
		fname, tname,
		args->sla_from.da_fhandle.fh_generic.fh_fsid.val[0],
		args->sla_from.da_fhandle.fh_generic.fh_fsid.val[1],
		args->sla_from.da_fhandle.fh_generic.fh_fid.fid_data[0],
		args->sla_from.da_fhandle.fh_generic.fh_fid.fid_data[1]));

	/*
	 *	Disallow NULL paths
	 */
	if ((fname == (char *) NULL) || (*fname == '\0')) {
		*status = NFSERR_NOENT;
		return (0);
	}

	if ((tname == (char *) NULL) || (*tname == '\0')) {
		*status = NFSERR_NOENT;
		return (0);
	}

	dvp = nfsrv_fhtovp(&args->sla_from.da_fhandle, xprt, RFS_SYMLINK, xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_SYMLINK],
		("symlink: fhtovp returns 0x%x\n", dvp));

	if (!dvp) {
		*status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_SYMLINK],
			("symlink: return ESTALE %d\n", ESTALE));
		return (0);
	}

	/* From here on, all returns should be via `goto done' */

	if (rdonly(dvp, xpd, xprt)) {
		error = EROFS;
		DPRINTF(nfsdebug || nfsopdebug[RFS_SYMLINK],
			("symlink: readonly fs\n"));
		goto done;
	}

	if (error = nfsrv_access(dvp, VWRITE, u.u_cred)) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_SYMLINK],
			("symlink: access returns %d\n", error));
		goto done;
	}

	sattr_to_vattr(&args->sla_sa, &va);
	va.va_mode = 0777;
	va.va_type = VLNK;

	DPRINTF(nfsdebug || nfsopdebug[RFS_SYMLINK],
		("symlink(1): type %d, mode 0x%x, size %d\n",
		       va.va_type, va.va_mode, va.va_size));

	ndp->ni_cdir = dvp;
	ndp->ni_cred = u.u_cred;
	ndp->ni_nameiop =  CREATE | WANTPARENT;
	ndp->ni_segflg = UIO_SYSSPACE;
	ndp->ni_dirp = fname;
	DPRINTF(nfsdebug || nfsopdebug[RFS_SYMLINK],
		("symlink: call namei (creating: %s)\n",fname));
	error = namei(ndp);
	vp = ndp->ni_vp;
	DPRINTF(nfsdebug || nfsopdebug[RFS_SYMLINK],
		("symlink: namei returns 0x%x %d\n", vp, error));
	if (error)
		goto done;

	DPRINTF(nfsdebug || nfsopdebug[RFS_SYMLINK],
		("symlink: ref on dvp after namei %d\n", dvp->v_usecount));

	/* From here on, all returns should be via `goto out' */

	if (vp) {
		error = EEXIST;
		goto out;
	}

      out:

	if (!error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_SYMLINK],
			("symlink:  (ref on dvp %d)", dvp->v_usecount));
		VOP_SYMLINK(ndp, &va, tname, error);
		DPRINTF(nfsdebug || nfsopdebug[RFS_SYMLINK],
			("symlink:  error %d (ref on dvp %d)\n", error,
			dvp->v_usecount));
	} else {
		DPRINTF(nfsdebug || nfsopdebug[RFS_SYMLINK],
			("symlink: error %d (ref on dvp %d)\n", error,
			dvp->v_usecount));
		VOP_ABORTOP(ndp, aerror);
		vrele(dvp); /* one for namei */
		vrele(vp);
	}

      done:

	if (!error) {
		VOP_GETATTR(dvp, &dva, u.u_cred, error);
		DPRINTF(error && (nfsdebug || nfsopdebug[RFS_SYMLINK]),
			("symlink: vop_getattr2 failed %d\n", error));
	}

	vrele(dvp);	/* one for fhtovp */

	*status = puterrno(error);

	DPRINTF(nfsdebug || nfsopdebug[RFS_SYMLINK],
		("symlink: return %d (ref on dvp %d)\n",error,dvp->v_usecount));
	return (0);

}

/*
 * Make a directory.
 * Create a directory with the given name, parent directory, and attributes.
 * Returns a file handle and attributes for the new directory.
 */
int
rfs_mkdir(args, dr, nreq, xprt, xpd)
	struct nfscreatargs *args;
	struct  nfsdiropres *dr;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	int error = 0;
	struct vnode *vp;
	struct vnode *dvp;
	struct vattr va, dva;
	char *name = args->ca_da.da_name;
	struct nameidata *ndp = &u.u_nd;
	nfsv2fh_t *fhp;
	struct timeval duptime;

#ifdef	KTRACE
	kern_trace(314,0,0,0);
#endif  /* KTRACE */
	DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
		("rfs_mkdir: %s dfh fsid %x %x inode %d %d\n",
		name, args->ca_da.da_fhandle.fh_generic.fh_fsid.val[0],
		args->ca_da.da_fhandle.fh_generic.fh_fsid.val[1],
		args->ca_da.da_fhandle.fh_generic.fh_fid.fid_data[0],
		args->ca_da.da_fhandle.fh_generic.fh_fid.fid_data[1]));

	/*
	 *	Disallow NULL paths
	 */
	if ((name == (char *) NULL) || (*name == '\0')) {
		dr->dr_status = NFSERR_NOENT;
		return (0);
	}

	dvp = nfsrv_fhtovp(&args->ca_da.da_fhandle, xprt, RFS_MKDIR, xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
		("mkdir: fhtovp returns 0x%x\n", vp));

	if (!dvp) {
		dr->dr_status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
			("mkdir: return ESTALE %d\n", ESTALE));
		return (0);
	}

	/* From here on, all returns should be via `goto done' */

	if (rdonly(dvp, xpd, xprt)) {
		error = EROFS;
		DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
			("mkdir: readonly fs\n"));
		goto done;
	}

	sattr_to_vattr(&args->ca_sa, &va);
	va.va_type = VDIR;
	DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
		("mkdir: type %d, mode 0x%x, size %d\n",
		       va.va_type, va.va_mode, va.va_size));

/* MPCXXX */
	error = nfsrv_access(dvp, VWRITE, u.u_cred);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
			("mkdir: access returns %d\n", error));
		goto done;
	}

	ndp->ni_cdir = dvp;
	ndp->ni_cred = u.u_cred;
	ndp->ni_nameiop =  CREATE | WANTPARENT | STRIPSLASH;
	ndp->ni_segflg = UIO_SYSSPACE;
	ndp->ni_dirp = name;

	DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
		("mkdir: call namei (creating: %s)\n", name));
	error = namei(ndp);
	vp = ndp->ni_vp;
	DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
		("mkdir: namei returns 0x%x %d\n", vp, error));

	if (error)
		goto done;

	if (!vp) {
		VOP_MKDIR(ndp, &va, error);
		DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
			("mkdir: back from vop_mkdir (%d)\n", error));
		if (error)
			goto done;
		vp = ndp->ni_vp;

	/* From here on, all returns should be via `goto out' */

	} else {
		VOP_ABORTOP(ndp, error);
		vrele(dvp);
		error = EEXIST;
		DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
			("mkdir: directory exists\n"));
		goto out;
	}

	VOP_GETATTR(vp, &va, u.u_cred, error);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
			("mkdir: vop_getattr2 failed %d\n", error));
		goto out;
	}
	DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
		("mkdir: vop_getattr2 returns fsid %x file id %d\n",
		va.va_fsid, va.va_fileid));

	vattr_to_nattr(&va, &dr->dr_attr);

	/*
	 * N.B.: there's a slim chance that this vnode has been vgone'd,
	 * in which case v_mount points to dead_mount.  If this is the
	 * case, then we can depend on an error from the VFS_VPTOFH call.
	 */
	fhp = &dr->dr_fhandle;
	fhp->fh_generic.fh_fsid = vp->v_mount->m_stat.f_fsid;
	VFS_VPTOFH(vp, (struct fid *)&fhp->fh_generic.fh_fid, error);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
			("mkdir: vop_vptofh returns %d\n", error));
		goto out;
	}

	/*
	 *  Check to see if we can support this file system
	 */
	if ( sizeof(fsid_t) + fhp->fh_generic.fh_fid.fid_len +  xpd->e_fid.fid_len >
	    NFS_FHSIZE ) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
			("mkdir: fhandle too big\n"));
		error = EREMOTE;
		goto out;
	}

	fid_copy(&xpd->e_fid, &fhp->fh_generic.fh_efid);
	/* gfs_unlock(vp); ultrix SMP*/

      out:

	vrele(vp);

      done:

	vrele(dvp);	/* one for fhtovp */

	dr->dr_status = puterrno(error);
	DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
		("mkdir: return %d\n", error));
	DPRINTF(nfsdebug || nfsopdebug[RFS_MKDIR],
		("mkdir: vp 0x%x ref %d dvp 0x%x ref %d\n",
		       vp, vp?vp->v_usecount:666,
		       dvp, dvp?dvp->v_usecount:666));
	return (0);


}

/*
 * Remove a directory.
 * Remove the given directory name from the given parent directory.
 */
int
rfs_rmdir(da, status, nreq, xprt, xpd)
	struct nfsdiropargs *da;
	enum nfsstat *status;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	int error = 0, aerror;
	struct vnode *vp;
	struct vnode *dvp;
	struct vattr dva;
	nfsv2fh_t *fhp;
	struct nameidata *ndp = &u.u_nd;
	char *name = da->da_name;

#ifdef	KTRACE
	kern_trace(315,0,0,0);
#endif  /* KTRACE */
	DPRINTF(nfsdebug || nfsopdebug[RFS_RMDIR],
		("rfs_rmdir: %s dfh fsid %x %x inode %d %d\n",
		name, da->da_fhandle.fh_generic.fh_fsid.val[0],
		da->da_fhandle.fh_generic.fh_fsid.val[1],
		da->da_fhandle.fh_generic.fh_fid.fid_data[0],
		da->da_fhandle.fh_generic.fh_fid.fid_data[1]));

	/*
	 *	Disallow NULL paths
	 */
	if ((name == (char *) NULL) || (*name == '\0')) {
		*status = NFSERR_NOENT;
		return (0);
	}

	dvp = nfsrv_fhtovp(&da->da_fhandle, xprt, RFS_RMDIR, xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_RMDIR],
		("rmdir: fhtovp returns 0x%x\n", vp));

	if (!dvp) {
		*status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_RMDIR],
			("rmdir: return ESTALE %d\n", ESTALE));
		return (0);
	}

	/* From here on, all returns should be via `goto done' */

	if (rdonly(dvp, xpd, xprt)) {
		error = EROFS;
		DPRINTF(nfsdebug || nfsopdebug[RFS_RMDIR],
			("rmdir: readonly fs\n"));
		goto done;
	}

	nfsrv_access(dvp, VWRITE, u.u_cred);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_RMDIR],
			("rmdir: access returns %d\n", error));
		goto done;
	}

	ndp->ni_cdir = dvp;
	ndp->ni_cred = u.u_cred;
	ndp->ni_nameiop =  DELETE | WANTPARENT | STRIPSLASH;
	ndp->ni_segflg = UIO_SYSSPACE;
	ndp->ni_dirp = name;
	DPRINTF(nfsdebug || nfsopdebug[RFS_RMDIR],
		("rmdir: call namei (removing: %s)\n", name));
	error = namei(ndp);
	vp = ndp->ni_vp;
	DPRINTF(nfsdebug || nfsopdebug[RFS_RMDIR],
		("rmdir: namei returns 0x%x %d\n", vp, error));
	if (error)
		goto done;

	/* From here on, all returns should be via `goto out' */

	if (vp->v_type != VDIR) {
		error = ENOTDIR;
		goto out;
	}

	/*
	 * No rmdir "." please.
	 */
	if (dvp == vp) {
		error = EINVAL;
		goto out;
	}

	/*
	 * Don't unlink a mounted file.
	 */
	if (vp->v_flag & VROOT) {
		error = EBUSY;
		goto out;
	}

      out:

	if (!error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_RMDIR],
			("rmdir:  (ref on dvp %d)", dvp->v_usecount));
		VOP_RMDIR(ndp, error);
		DPRINTF(nfsdebug || nfsopdebug[RFS_RMDIR],
			("  error %d (ref on dvp %d)\n", error,
			       dvp->v_usecount));
	} else {
		DPRINTF(nfsdebug || nfsopdebug[RFS_RMDIR],
			("remove: error %d (ref on dvp %d)\n", error,
			       dvp->v_usecount));
		VOP_ABORTOP(ndp, aerror);
		vrele(dvp); /* one for namei */
		vrele(vp);
	}

      done:

	vrele(dvp);	/* one for fhtovp */

	*status = puterrno(error);

	DPRINTF(nfsdebug || nfsopdebug[RFS_RMDIR],
		("rmdir: return %d (ref on dvp %d)\n", error, dvp->v_usecount));
	return (0);
}

#define	ROUND_UP(v,t) (((v) + (t) - 1) & ~((t) - 1))
/*
 * Several variables deserve documentation:
 *
 * rda->rda_count
 *	XDR data size the client wants back.  We treat this as the size of
 *	the directory buffer to use.  This doesn't make much sense, but
 *	there's no decent correlation between data buffer and serialized
 *	data.
 * rd->rd_origreqsize
 *	Only used in xdr_putrddirres to limit the data returned.  We limit
 *	this ourselves for no good reason.
 * rd->rd_entries
 *	Becomes a pointer to the first directory entry to return.
 * rd->rd_size
 *	Become data length we want to encode and return.
 * rd->rd_bufsize
 *	Becomes buffer length minus junk skipped at the beginning.  Only used
 *      to recompute the start of the buffer so we can call kmem_free.
 *
 * Ultimately, this code prepares a buffer that looks something like this:
 *
 * 	rd_entries
 *	     V
 * +--------------------------------------------------------------------+
 * |									|
 * +--------------------------------------------------------------------+
 * | - - - - - - - - - - - - - - rd_rda_count  - - - - - - - - - - - - -|
 *
 * | - - - - - - rd_origreqsize - - - - -|
 *
 * 	     | - - - - - - - - - rd_size - - - - - - - -|
 *
 * 	     | - - - - - - - - - - - rd_bufsize  - - - - - - - - - - - -|
 *
 */
int
rfs_readdir(rda, rd, nreq, xprt, xpd)
	struct nfsrddirargs *rda;
	struct nfsrddirres  *rd;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	int error = 0;
	struct vnode *vp;
	struct dirent *dp;
	struct iovec iov;
	struct uio uio;
	u_int offset;
	u_int skipped;
	int eofflag;

	vp = nfsrv_fhtovp(&rda->rda_fh, xprt, RFS_READDIR, xpd);
	DPRINTF(nfsdebug || nfsopdebug[RFS_READDIR],
		("readdir: fhtovp returns 0x%x\n", vp));

#ifdef	KTRACE
	kern_trace(316,vp,rda->rda_offset,rda->rda_count);
#endif  /* KTRACE */
	if (!vp) {
		rd->rd_status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_READDIR],
			("readdir: return ESTALE %d\n", ESTALE));
		return (0);
	}

	/* From here on, all returns should be via `goto done' */

	if (vp->v_type != VDIR) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_READDIR],
			("readdir: attempt to read non-directory\n"));
		error = ENOTDIR;
		goto done;
	}

	/*
	 * check cd access to dir.  we have to do this here because
	 * the opendir doesn't go over the wire.
	 */
	error = nfsrv_access(vp, VREAD, u.u_cred);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_READDIR],
			("readdir: nfsrv_access failed %d\n", error));
		goto done;
	}

	if (rda->rda_count == 0) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_READDIR],
			("readdir: read size 0\n"));
		goto done;
	}

	rda->rda_count = MIN(rda->rda_count, NFS_MAXREADDIR);
	rd->rd_origreqsize = rda->rda_count;
	rda->rda_count = ROUND_UP(rda->rda_count, DIRBLKSIZ);

	/*
	 * Allocate data for entries.  This will be freed by rfs_rddirfree.
	 */
	rd->rd_entries = (struct dirent *)
			kmem_alloc(rda->rda_count);
	rd->rd_bufsize = rda->rda_count;

nxtblk:

	rd->rd_offset = rda->rda_offset & ~(DIRBLKSIZ - 1);
	DPRINTF(nfsdebug || nfsopdebug[RFS_READDIR],
               ("readdir: rda_count %d rda_offset %d rd_offset %d \n",
                rda->rda_count, rda->rda_offset, rd->rd_offset));

	/*
	 * Set up io vector to read directory data
	 */
	iov.iov_base = (caddr_t)rd->rd_entries;
	iov.iov_len = rda->rda_count;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_rw = UIO_READ;
	uio.uio_offset = rd->rd_offset;
	uio.uio_resid = rda->rda_count;

	/*
	 * read directory
	 */
	DPRINTF(nfsdebug || nfsopdebug[RFS_READDIR],
		("readdir: do vop_readdir\n"));

	VOP_READDIR(vp, &uio, u.u_cred, &eofflag, error);

	DPRINTF(nfsdebug || nfsopdebug[RFS_READDIR],
		("readdir: vop_readdir uio_offset %ld\n", uio.uio_offset));
 
	/*
	 * Clean up
	 */
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_READDIR],
			("readdir: vop_readdir failed %d\n", error));
		rd->rd_size = 0;
		goto done;
	}

	/*
	 * set size and eof
	 */
	if (uio.uio_resid) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_READDIR],
			("readdir: resid %d, eof true\n", uio.uio_resid));
		rd->rd_size = rda->rda_count - uio.uio_resid;
		rd->rd_eof = TRUE;
	} else {
		DPRINTF(nfsdebug || nfsopdebug[RFS_READDIR],
			("readdir: resid %d, eof false\n", uio.uio_resid));
		rd->rd_size = rda->rda_count;
		rd->rd_eof = FALSE;
	}

	/*
	 * if client request was in the middle of a block
	 * or block begins with null entries skip entries
	 * til we are on a valid entry >= client's requested
	 * offset.
	 */
	dp = rd->rd_entries;
	offset = rd->rd_offset;
	skipped = 0;
	while ((skipped < rd->rd_size) &&
	    ((offset + dp->d_reclen <= rda->rda_offset) || (dp->d_ino == 0))) {
		skipped += dp->d_reclen;
		offset += dp->d_reclen;
		dp = (struct dirent *)((vm_offset_t)dp +
				       (vm_offset_t)dp->d_reclen);
	}
	/*
	 * Reset entries pointer
	 */
	if (skipped) {
		rd->rd_size -= skipped;
		if (rd->rd_size == 0 && !rd->rd_eof) {
			/*
			 * we have skipped a whole block, reset offset
			 * and read another block (unless eof)
			 */
			rda->rda_offset = offset;
			DPRINTF(nfsdebug || nfsopdebug[RFS_READDIR],
				("readdir: goto nxtblk\n"));
			goto nxtblk;
		}
		rd->rd_bufsize -= skipped;
		rd->rd_offset = offset;
		rd->rd_entries = dp;
	}
      done:
	vrele(vp);
	rd->rd_status = puterrno(error);
	DPRINTF(nfsdebug || nfsopdebug[RFS_READDIR],
		("readdir: return %d bytes skipped %d off %d code %d\n",
		 rd->rd_size, skipped, rd->rd_offset, error));
	return (0);

}

rfs_rddirfree(rd)
	struct nfsrddirres *rd;
{
	u_int buflen;

#ifdef	KTRACE
	kern_trace(320,rd->rd_entries,rd->rd_bufsize,0);
#endif  /* KTRACE */
	buflen = ROUND_UP(rd->rd_origreqsize, DIRBLKSIZ);
	if (rd->rd_entries)
	    if (buflen == rd->rd_bufsize)
		kmem_free((caddr_t)rd->rd_entries, (u_int)rd->rd_bufsize);
	    else {
		(vm_offset_t) rd->rd_entries -= 
		    (buflen - rd->rd_bufsize);
			kmem_free((caddr_t)rd->rd_entries, buflen);
	    }
}

rfs_statfs(fh, fs, nreq, xprt, xpd)
	nfsv2fh_t *fh;
	struct nfsstatfs *fs;
	struct nfs_req *nreq;
	SVCXPRT *xprt;
	struct kexport *xpd;
{
	int error = 0;
	struct vnode *vp;
	struct mount *mp;
	struct statfs *sf;

	vp = nfsrv_fhtovp(fh, xprt, RFS_STATFS, xpd);
#ifdef	KTRACE
	kern_trace(317,vp,0,0);
#endif  /* KTRACE */
	DPRINTF(nfsdebug || nfsopdebug[RFS_STATFS],
		("statfs: fhtovp returns 0x%x\n", vp));

	if (!vp) {
		fs->fs_status = NFSERR_STALE;
		DPRINTF(nfsdebug || nfsopdebug[RFS_STATFS],
			("statfs: return ESTALE %d\n", ESTALE));
		return (0);
	}

	/* From here on, all returns should be via `goto done' */

	mp = vp->v_mount;
	sf = &mp->m_stat;

	VFS_STATFS(mp, error);
	if (error) {
		DPRINTF(nfsdebug || nfsopdebug[RFS_STATFS],
			("statfs: vop_statfs returns %d\n", error));
		goto done;
	}

	fs->fs_tsize = NFS_MAXDATA;
	MOUNT_LOCK(mp);
	if (sf->f_fsize > 0x7fffffff)
		fs->fs_bsize = 0x7fffffff;
	else
		fs->fs_bsize = sf->f_fsize;
	if (sf->f_blocks > 0x7fffffff)
		fs->fs_blocks = 0x7fffffff;
	else
		fs->fs_blocks = sf->f_blocks;
	if (sf->f_bfree > 0x7fffffff)
		fs->fs_bfree = 0x7fffffff;
	else
		fs->fs_bfree = sf->f_bfree;
	if (sf->f_bavail > 0x7fffffff)
		fs->fs_bavail = 0x7fffffff;
	else
		fs->fs_bavail = sf->f_bavail;
	MOUNT_UNLOCK(mp);

      done:

	vrele(vp);
	fs->fs_status = puterrno(error);
	DPRINTF(nfsdebug || nfsopdebug[RFS_STATFS],
		("statfs: return %d\n", error));
	return (0);

}

/*ARGSUSED*/
rfs_null(argp, resp)
	caddr_t *argp;
	caddr_t *resp;
{
	/* do nothing */
#ifdef	KTRACE
	kern_trace(300,0,0,0);
#endif  /* KTRACE */
	return (0);
}

/*ARGSUSED*/
rfs_error(argp, resp)
	caddr_t *argp;
	int *resp;
{
	*resp = EOPNOTSUPP;
	return (0);
}

int
nullfree()
{
}

/*
 * rfs dispatch table
 * Indexed by version,proc
 */

struct rfsdisp {
	int	  (*dis_proc)();	/* proc to call */
	xdrproc_t dis_xdrargs;		/* xdr routine to get args */
	xdrproc_t dis_fastxdrargs;	/* `fast' xdr routine to get args */
	int	  dis_argsz;		/* sizeof args */
	xdrproc_t dis_xdrres;		/* xdr routine to put results */
	xdrproc_t dis_fastxdrres;	/* `fast' xdr routine to put results */
	int	  dis_ressz;		/* size of results */
	int	  (*dis_resfree)();	/* frees space allocated by proc */
	int	  dis_flags;		/* flags, see below */
	nfsv2fh_t *(*dis_getfh)();	/* returns the fhandle for the req */
	int	  dis_fhsize;		/* size of a file handle */
}; 

#define	RFS_IDEMPOTENT	0x1	/* idempotent or not */
#define	RFS_ALLOWANON	0x2	/* allow anonymous access */
#define	RFS_MAPRESP	0x4	/* use mapped response buffer */
#define	RFS_AVOIDWORK	0x8	/* do work avoidance for dups */

#define	KSTAT_STRLEN	31	/* 30 chars + NULL; must be 16 * n - 1 */
typedef struct kstat_named {
	char	name[KSTAT_STRLEN];	/* name of counter */
	ulong_t	count;
} kstat_named_t;

#define	KSTAT_DATA_CHAR		0
#define	KSTAT_DATA_LONG		1
#define	KSTAT_DATA_ULONG	2
#define	KSTAT_DATA_LONGLONG	3
#define	KSTAT_DATA_ULONGLONG	4
#define	KSTAT_DATA_FLOAT	5
#define	KSTAT_DATA_DOUBLE	6

kstat_named_t rfsproccnt_v2[] = {
	{"null"},
	{"getattr"},
	{"setattr"},
	{"unused"},
	{"lookup"},
	{"readlink"},
	{"read"},
	{"unused"},
	{"write"},
	{"create"},
	{"remove"},
	{"rename"},
	{"link"},
	{"symlink"},
	{"mkdir"},
	{"rmdir"},
	{"readdir"},
	{"fsstat"},
};

kstat_named_t *rfsproccnt_v2_ptr = rfsproccnt_v2;
ulong_t rfsproccnt_v2_ndata = sizeof (rfsproccnt_v2) / sizeof (kstat_named_t);

struct rfsdisp rfsdisptab_v2[] = {
	/*
	 * VERSION 2
	 */
	/* RFS_NULL = 0 */
	{rfs_null,
		 xdr_void, 0, 0,
		 xdr_void, 0, 0,
		 nullfree, RFS_IDEMPOTENT,
		 0, 0},
	/* RFS_GETATTR = 1 */
	{rfs_getattr,
		 xdr_fhandle, 0, sizeof(fhandle_t),
		 xdr_attrstat, 0, sizeof(struct nfsattrstat),
		 nullfree, RFS_IDEMPOTENT,
		 0, 0},
	/* RFS_SETATTR = 2 */
	{rfs_setattr,
		 xdr_saargs, 0, sizeof(struct nfssaargs),
		 xdr_attrstat, 0, sizeof(struct nfsattrstat),
		 nullfree, 0,
		 0, 0},
	/* RFS_ROOT = 3 *** NO LONGER SUPPORTED *** */
	{rfs_error,
		 xdr_void, 0, 0,
		 xdr_void, 0, 0,
		 nullfree, 0,
		 0, 0},
	/* RFS_LOOKUP = 4 */
	{rfs_lookup,
		 xdr_diropargs, 0, sizeof(struct nfsdiropargs),
		 xdr_diropres, 0, sizeof(struct nfsdiropres),
		 nullfree, RFS_IDEMPOTENT,
		 0, 0},
	/* RFS_READLINK = 5 */
	{rfs_readlink,
		 xdr_fhandle, 0, sizeof(fhandle_t),
		 xdr_rdlnres, 0, sizeof(struct nfsrdlnres),
		 rfs_rlfree, RFS_IDEMPOTENT,
		 0, 0},
	/* RFS_READ = 6 */
	{rfs_read,
		 xdr_readargs, 0, sizeof(struct nfsreadargs),
		 xdr_rdresult, 0, sizeof(struct nfsrdresult),
		 rfs_rdfree, RFS_IDEMPOTENT,
		 0, 0},
	/* RFS_WRITECACHE = 7 *** NO LONGER SUPPORTED *** */
	{rfs_error,
		 xdr_void, 0, 0,
		 xdr_void, 0, 0,
		 nullfree, 0,
		 0, 0},
	/* RFS_WRITE = 8 */
	{rfs_write,
		 xdr_writeargs, 0, sizeof(struct nfswriteargs),
		 xdr_attrstat, 0, sizeof(struct nfsattrstat),
		 nullfree, 0,
		 0, 0},
	/* RFS_CREATE = 9 */
	{rfs_create,
		 xdr_creatargs, 0, sizeof(struct nfscreatargs),
		 xdr_diropres, 0, sizeof(struct nfsdiropres),
		 nullfree, 0,
		 0, 0},
	/* RFS_REMOVE = 10 */
	{rfs_remove,
		 xdr_diropargs, 0, sizeof(struct nfsdiropargs),
		 xdr_enum, 0, sizeof(enum nfsstat),
		 nullfree, 0,
		 0, 0},
	/* RFS_RENAME = 11 */
	{rfs_rename,
		 xdr_rnmargs, 0, sizeof(struct nfsrnmargs),
		 xdr_enum, 0, sizeof(enum nfsstat),
		 nullfree, 0,
		 0, 0},
	/* RFS_LINK = 12 */
	{rfs_link,
		 xdr_linkargs, 0, sizeof(struct nfslinkargs),
		 xdr_enum, 0, sizeof(enum nfsstat),
		 nullfree, 0,
		 0, 0},
	/* RFS_SYMLINK = 13 */
	{rfs_symlink,
		 xdr_slargs, 0, sizeof(struct nfsslargs),
		 xdr_enum, 0, sizeof(enum nfsstat),
		 nullfree, 0,
		 0, 0},
	/* RFS_MKDIR = 14 */
	{rfs_mkdir,
		 xdr_creatargs, 0, sizeof(struct nfscreatargs),
		 xdr_diropres, 0, sizeof(struct nfsdiropres),
		 nullfree, 0,
		 0, 0},
	/* RFS_RMDIR = 15 */
	{rfs_rmdir,
		 xdr_diropargs, 0, sizeof(struct nfsdiropargs),
		 xdr_enum, 0, sizeof(enum nfsstat),
		 nullfree, 0,
		 0, 0},
	/* RFS_READDIR = 16 */
	{rfs_readdir,
		 xdr_rddirargs, 0, sizeof(struct nfsrddirargs),
		 xdr_putrddirres, 0, sizeof(struct nfsrddirres),
		 rfs_rddirfree, RFS_IDEMPOTENT,
		 0, 0},
	/* RFS_STATFS = 17 */
	{rfs_statfs,
		 xdr_fhandle, 0, sizeof(fhandle_t),
		 xdr_statfs, 0, sizeof(struct nfsstatfs),
		 nullfree, RFS_IDEMPOTENT,
		 0, 0},
};

union rfs_args {
	/*
	 * VERSION 2
	 */
	/* RFS_NULL = 0 */
	/* RFS_GETATTR = 1 */
	fhandle_t nfs2_getattr_args;
	/* RFS_SETATTR = 2 */
	struct nfssaargs nfs2_setattr_args;
	/* RFS_ROOT = 3 *** NO LONGER SUPPORTED *** */
	/* RFS_LOOKUP = 4 */
	struct nfsdiropargs nfs2_lookup_args;
	/* RFS_READLINK = 5 */
	fhandle_t nfs2_readlink_args;
	/* RFS_READ = 6 */
	struct nfsreadargs nfs2_read_args;
	/* RFS_WRITECACHE = 7 *** NO LONGER SUPPORTED *** */
	/* RFS_WRITE = 8 */
	struct nfswriteargs nfs2_write_args;
	/* RFS_CREATE = 9 */
	struct nfscreatargs nfs2_create_args;
	/* RFS_REMOVE = 10 */
	struct nfsdiropargs nfs2_remove_args;
	/* RFS_RENAME = 11 */
	struct nfsrnmargs nfs2_rename_args;
	/* RFS_LINK = 12 */
	struct nfslinkargs nfs2_link_args;
	/* RFS_SYMLINK = 13 */
	struct nfsslargs nfs2_symlink_args;
	/* RFS_MKDIR = 14 */
	struct nfscreatargs nfs2_mkdir_args;
	/* RFS_RMDIR = 15 */
	struct nfsdiropargs nfs2_rmdir_args;
	/* RFS_READDIR = 16 */
	struct nfsrddirargs nfs2_readdir_args;
	/* RFS_STATFS = 17 */
	fhandle_t nfs2_statfs_args;
};

union rfs_res {
	/*
	 * VERSION 2
	 */
	/* RFS_NULL = 0 */
	/* RFS_GETATTR = 1 */
	struct nfsattrstat nfs2_getattr_res;
	/* RFS_SETATTR = 2 */
	struct nfsattrstat nfs2_setattr_res;
	/* RFS_ROOT = 3 *** NO LONGER SUPPORTED *** */
	/* RFS_LOOKUP = 4 */
	struct nfsdiropres nfs2_lookup_res;
	/* RFS_READLINK = 5 */
	struct nfsrdlnres nfs2_readlink_res;
	/* RFS_READ = 6 */
	struct nfsrdresult nfs2_read_res;
	/* RFS_WRITECACHE = 7 *** NO LONGER SUPPORTED *** */
	/* RFS_WRITE = 8 */
	struct nfsattrstat nfs2_write_res;
	/* RFS_CREATE = 9 */
	struct nfsdiropres nfs2_create_res;
	/* RFS_REMOVE = 10 */
	enum nfsstat nfs2_remove_res;
	/* RFS_RENAME = 11 */
	enum nfsstat nfs2_rename_res;
	/* RFS_LINK = 12 */
	enum nfsstat nfs2_link_res;
	/* RFS_SYMLINK = 13 */
	enum nfsstat nfs2_symlink_res;
	/* RFS_MKDIR = 14 */
	struct nfsdiropres nfs2_mkdir_res;
	/* RFS_RMDIR = 15 */
	enum nfsstat nfs2_rmdir_res;
	/* RFS_READDIR = 16 */
	struct nfsrddirres nfs2_readdir_res;
	/* RFS_STATFS = 17 */
	struct nfsstatfs nfs2_statfs_res;
};

struct rfs_disptable {
	int dis_nprocs;
	char **dis_procnames;
	kstat_named_t *dis_proccnt;
	struct rfsdisp *dis_table;
} rfs_disptable[] = {
	{sizeof (rfsdisptab_v2) / sizeof (rfsdisptab_v2[0]),
	    rfsnames,
	    rfsproccnt_v2, rfsdisptab_v2},
};


/*
 * Static array that defines which nfs rpc's are nonidempotent
 * Used by the client.
 */
int nonidempotent[RFS_NPROC] = {
	FALSE,
	FALSE,
	TRUE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	TRUE,
	TRUE,
	TRUE,
	TRUE,
	TRUE,
	TRUE,
	TRUE,
	TRUE,
	FALSE,
	FALSE,
};

int nfs_idempotent_drc = 0; /* use DRC on all op types */
/* idempotent ops = 0, 1, 3, 4, 5, 6, 7, 16, 17 */
int nfs_drc[RFS_NPROC] = {0,0,1,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0};
unsigned int no_nfs_drc = 0;

struct rfsspace {
	struct rfsspace *rs_next;
	caddr_t		rs_dummy;
};

struct rfsspace *rfsfreesp = NULL;

int rfssize = 0;
int ressize = 0;
int rfs_nresults = 0;

#define RFSGET(rs) { \
	int i; \
	struct rfsdisp *dis; \
	/*smp_lock(&lk_nfsargs, LK_RETRY);*/ \
	if (!rfssize) { \
		rfssize = sizeof(union rfs_args); \
		ressize = sizeof(union rfs_res); \
		if ( ressize > rfssize) \
			rfssize = ressize; \
	} \
	if (rfsfreesp) { \
		(rs) = (caddr_t)rfsfreesp; \
		rfsfreesp = rfsfreesp->rs_next; \
		/*smp_unlock(&lk_nfsargs);*/ \
	} else { \
		/*smp_unlock(&lk_nfsargs);*/ \
		(rs) = (caddr_t)kmem_alloc(rfssize); \
		++rfs_nresults; \
	} \
	}

#define RFSPUT(rs) { \
	/*smp_lock(&lk_nfsargs, LK_RETRY);*/ \
	((struct rfsspace *)(rs))->rs_next = rfsfreesp; \
	rfsfreesp = (struct rfsspace *)(rs); \
	/*smp_unlock(&lk_nfsargs);*/ \
	}

/*
 * If nfsportmon is set, then clients are required to use
 * privileged ports (ports < IPPORT_RESERVED) in order to get NFS services.
 *
 *	N.B.:  this attempt to carry forward the already ill-conceived
 *	notion of privileged ports for TCP/UDP is really quite ineffectual.
 *	Not only is it transport-dependent, it's laughably easy to spoof.
 *	If you're really interested in security, you must start with secure
 *	RPC instead.
 */
int nfsportmon = 0;
int nfs_cpu_count[32];
int nfs_tracereplies = 1;
int rfs_dupbusy = 0;
int rfs_cjtrace = 0;
int rpcholdxprt = 0;

/* CJXXX */
/*
 * kernel-based RPC server duplicate transaction cache flag values
 */
#define	NDUP_NEW			0x0	/* new request */
#define	NDUP_INPROGRESS		0x01	/* request already going */
#define	NDUP_DONE		0x02	/* request done */
#define	NDUP_DROP		0x03	/* request done */

int
rfs_dispatch(req, xprt)
	struct svc_req *req;
	register SVCXPRT *xprt;
{
	register struct rfsdisp *disp = NULL;
	int error = 0;
	caddr_t args = NULL;
	caddr_t res = NULL;
	struct nfs_req *nreq = NULL;
	struct kexport *ep;
	nfsv2fh_t *fh;

	int which;
	int vers;
	struct authunix_parms *aup;
	struct ucred *newcr = NULL;
	int ret = 0;
	int busywrite = 0;
	int dupstat = 0;

	struct nfs_svinfo *psv = (struct nfs_svinfo *)PNFS_SV;

	/*smp_lock(&lk_nfsstat, LK_RETRY);*/
	svstat.ncalls++;
	/*smp_unlock(&lk_nfsstat);*/
	/*nfs_cpu_count[CURRENT_CPUDATA->cpu_num]++;*/

#ifdef CJKTRACE
	if (nfs_cjtrace && !rfs_cjtrace) {
		LOG_SYS_NFS(psv, NFS_START);
		LOG_SYS_NFS(psv, NFS_START);
		LOG_SYS_NFS(psv, NFS_START);
		++rfs_cjtrace;
	}
#endif /* CJKTRACE */

	/*
	 * Allocate request struct.
	 */
	RFSGETREQ(nreq);
	nreq->nrq_xprt = xprt;

	vers = req->rq_vers;
	if (vers < VERSIONMIN || vers > VERSIONMAX) {
		svcerr_progvers(req->rq_xprt, (u_int)VERSIONMIN,
		    (u_int)VERSIONMAX);
		DPRINTF(nfssvdebug,
			("dispatch: bad version 0x%x\n", vers));
		error++;
		goto done;
	}
	vers -= VERSIONMIN; /* set up for use as table index */

	which = req->rq_proc;
	nreq->nrq_vers = vers;
	nreq->nrq_xid = REQTOXID(req);
	nreq->nrq_proc = which;
	nreq->nrq_addr = REQTOADDR(req);

#ifdef CJKTRACE
	if (nfs_cjtrace && which == RFS_WRITE)
		LOG_SYS_NFS(psv, NFS_DISP_WRITE);
#endif /* CJKTRACE */
	if (which < 0 || which >= rfs_disptable[vers].dis_nprocs) {
		DPRINTF(nfssvdebug,
			("dispatch: vers %d bad proc 0x%x\n", vers+VERSIONMIN,
			 which));
		svcerr_noproc(req->rq_xprt);
		error++;
		goto done;
	}

	rfs_disptable[vers].dis_proccnt[which].count++;
	disp = &rfs_disptable[vers].dis_table[which];
	nreq->nrq_disp = disp;

	DPRINTF(nfssvdebug, ("dispatch: start %s\n", rfsnames[which]));


	/*
	 * Allocate args struct.
	 */
	RFSGET(args);
	nreq->nrq_args = args;

	/*
	 * Deserialize into args struct.
	 */
	if (disp->dis_fastxdrargs == NULL ||
	    !SVC_GETARGS(xprt, disp->dis_fastxdrargs, (char *)&args)) {
		bzero((caddr_t)args, (u_int)disp->dis_argsz);
		if (!SVC_GETARGS(xprt, disp->dis_xdrargs, args)) {
			svcerr_decode(xprt);
			error++;
			DPRINTF(nfssvdebug, ("dispatch: bad args %s",
					     rfs_disptable[vers].dis_procnames[which]));
			goto done;
		}
	}

	/*
	 * Allocate results struct.
	 */
	RFSGET(res);
	nreq->nrq_res = res;

	/* CJXXX: look into skipping this bzero */
	bzero((caddr_t)res, disp->dis_ressz);

	/*
	 * Find export information and check authentication,
	 * setting the credential if everything is ok.
	 */
	if (which != RFS_NULL) {
		/*
		 * XXX: this isn't really quite correct. Instead of doing
		 * this blind cast, we should extract out the fhandle for
		 * each NFS call. What's more, some procedures (like rename)
		 * have more than one fhandle passed in, and we should check
		 * that the two fhandles point to the same exported path.
		 */
		if (disp->dis_getfh)
			fh = (*disp->dis_getfh)(args);
		else
			fh = (nfsv2fh_t *)args;

		ep = exported;
		while (ep) {
			if ( /* export entry id matches fhandle export id */
			    fsid_equal(&ep->e_fsid, &fh->fh_generic.fh_fsid)  &&
			    fid_equal(&ep->e_fid, &fh->fh_generic.fh_efid) )  {
				break;
			} else
				ep = ep->e_next;
		}

		newcr = crget();
		nreq->nrq_holdcred = u.u_cred;
		u.u_cred = newcr;

		if (ep != NULL && !checkauth(ep, req, newcr)) {
			svcerr_weakauth(xprt);
			error++;
			goto done;
		}
	}


	if (nfsportmon) {
		/*
		 * Check for privileged port number
		 */
		static count = 0;
		if (ntohs(xprt->xp_raddr.sin_port) >= IPPORT_RESERVED) {
			svcerr_weakauth(xprt);
			if (count == 0) {
				printf("NFS request from unprivileged port, ");
				printf("source IP address = %s\n",
				       inet_ntoa(xprt->xp_raddr.sin_addr));
			}
			count++;
			count %= 256;
			error++;
			goto done;
		}
	}


	/* CJXXX: */
	/* NB: error cannot be set past here or the dup cache is screwed up */
	if (!(disp->dis_flags & RFS_IDEMPOTENT)) {
		dupstat = svckudp_dup(nreq->nrq_xid, which, vers,
				      nreq->nrq_addr, res, disp->dis_ressz);
		switch (dupstat) {
		      case NDUP_INPROGRESS:
			++nreq->nrq_busyxid;
			++nfs_throwaways[which];
			DPRINTF(nfssvdebug,
				("dispatch: DUPXID1 0x%x\n", nreq->nrq_xid));
			/*
			 * If write gathering, must let rfs_writeg()
		         * process this duplicate of a busy request.
			 * This starts a race between this request
			 * (and any other dups) and the first one to pass
			 * here.
			 */
			if (nreq->nrq_vers = VERSIONMIN &&
			    which == RFS_WRITE && nfs_write_gather) {
				struct nfswriteargs *p =
					(struct nfswriteargs *)args;
				p->wa_dupbusy++;
				++rfs_dupbusy;
			}
			else {
				goto done;
			}
		      case NDUP_NEW:
			/*smp_lock(&lk_nfsstat, LK_RETRY);*/
			svstat.reqs[which]++;
			/*smp_unlock(&lk_nfsstat);*/
			
			/*
			 * Call service routine
			 */
			DPRINTF(nfssvdebug,
				("dispatch: call %s %d xid 0x%x\n",
				 rfs_disptable[vers].dis_procnames[which],
				 which, nreq->nrq_xid));
			
			psv->nfs_sv_which = which;
			psv->nfs_sv_state = NFSSV_STARTED;
			nfs_sv_active++;
			
			ret = (*disp->dis_proc)(args, res, nreq, xprt, ep);
			
#ifdef CJKTRACE
			/* if (nfs_cjtrace && nreq->nrq_vers == VERSIONMIN &&
			   which == RFS_WRITE)
			   LOG_SYS_NFS(psv, NFS_WRITE_RETURN); */
#endif /* CJKTRACE */
			
			nfs_sv_active--;
			nfs_sv_active_hist[nfs_sv_active]++;
			
			DPRINTF(nfssvdebug,
				("dispatch: %s %d returns %d xid 0x%x\n",
				 rfs_disptable[vers].dis_procnames[which],
				 which, *(int *)res, nreq->nrq_xid));
			break;
		      case NDUP_DONE:
			break;
		      default:
			printf("rfs_dispatch botch\n");
			break;
		}
	} else {
		/*smp_lock(&lk_nfsstat, LK_RETRY);*/
		svstat.reqs[which]++;
		/*smp_unlock(&lk_nfsstat);*/
		
		/*
		 * Call service routine
		 */
		DPRINTF(nfssvdebug,
			("dispatch: call %s %d xid 0x%x\n",
			 rfs_disptable[vers].dis_procnames[which],
			 which, nreq->nrq_xid));
			
		psv->nfs_sv_which = which;
		psv->nfs_sv_state = NFSSV_STARTED;
		nfs_sv_active++;
			
		ret = (*disp->dis_proc)(args, res, nreq, xprt, ep);
		
		nfs_sv_active--;
		nfs_sv_active_hist[nfs_sv_active]++;
		
		DPRINTF(nfssvdebug,
			("dispatch: %s %d returns %d xid 0x%x\n",
			 rfs_disptable[vers].dis_procnames[which],
			 which, *(int *)res, nreq->nrq_xid));
	}


      done:

	/*if (CURRENT_CPUDATA->cpu_hlock != NULL)*/
		/*panic("nfsd holding lock");*/

	/* Check whether current xprt has to be exchanged. */
	if (!ret) {
		psv->nfs_sv_state = NFSSV_REPLY;
		rfs_sendreply(nreq, error);
	} else if (ret == RPCNOREPLY) {
		ret = 0;
	} else if (ret == RPCHOLDXPRT) {
		++rpcholdxprt;
	} else {
		printf("rfs_dispatch: bad rfs reply %d\n", ret);
		panic("rfs_dispatch");
	}
	psv->nfs_sv_state = NFSSV_IDLE;
	psv->nfs_sv_vp = NULL;
	psv->nfs_sv_fh = NULL;

#ifdef CJKTRACE
	if (nfs_cjtrace && which == RFS_WRITE)
		LOG_SYS_NFS(psv, NFS_WRITE_DONE);
#endif /* CJKTRACE */

	return (ret);

}

int rfs_dupbusy2 = 0;

rfs_sendreply(nreq, error)
	struct nfs_req * nreq;
	int error;
{
	struct ucred * tmpcr;
	int which = nreq->nrq_proc;
	struct nfs_svinfo *psv = (struct nfs_svinfo *)PNFS_SV;

	/* NB: if error is set, nrq_disp may be NULL */

	if (!error &&
	    !(nreq->nrq_disp->dis_flags & RFS_IDEMPOTENT) &&
	    !nreq->nrq_busyxid) {
		svckudp_dupdone(nreq->nrq_xid, which, nreq->nrq_vers,
				nreq->nrq_addr, nreq->nrq_res,
				nreq->nrq_disp->dis_ressz, NDUP_DONE);
	}

	/*
	 * Serialize and send results struct
	 */
	if (error && nfs_tracereplies)
		printf("rfs_dispatch: dispatch error, no reply\n");

	if (!error && !nreq->nrq_busyxid) {
		DPRINTF(nfssvdebug,
			("dispatch: call svc_sendreply %d %d\n",
			time.tv_sec, time.tv_usec));
		DPRINTF(rrokdebug,
			("dispatch: do reply xprt 0x%x xres 0x%x res 0x%x\n",
			nreq->nrq_xprt,	nreq->nrq_disp->dis_xdrres,
			(caddr_t)nreq->nrq_res));
#ifdef CJKTRACE
/*		if (nfs_cjtrace && which == RFS_WRITE)
			LOG_SYS_NFS(psv, NFS_WRITE_REPLY_START);*/
#endif /* CJKTRACE */
		/* CJXXX: skipping fast xdr here */
		if (!svc_sendreply(nreq->nrq_xprt,
				   nreq->nrq_disp->dis_xdrres,
				   (caddr_t)nreq->nrq_res)) {
			printf("rfs_dispatch: sendreply failed\n");
			error++;
		}
		DPRINTF(rrokdebug, ("dispatch: reply done\n"));
		DPRINTF(nfssvdebug && error,
			("dispatch: svc_sendreply returns error"));
#ifdef CJKTRACE
		if (nfs_cjtrace && which == RFS_WRITE)
			LOG_SYS_NFS(psv, NFS_WRITE_REPLY_DONE);
#endif /* CJKTRACE */
		DPRINTF(nfssvdebug, ("dispatch: free results\n"));
	}

	/*
	 * Free results struct
	 */
	if (nreq->nrq_res != NULL) {
		if (nreq->nrq_disp->dis_resfree != nullfree ) {
			(*nreq->nrq_disp->dis_resfree)(nreq->nrq_res);
		}
		/* rfsput((struct rfsspace *)nreq->nrq_res); */
		RFSPUT(nreq->nrq_res);
	}

	/*
	 * Free arguments struct
	 */
	DPRINTF(nfssvdebug, ("dispatch: call svc_freeargs1\n"));
	if (!SVC_FREEARGS(nreq->nrq_xprt,
			  nreq->nrq_disp != NULL ?
			  nreq->nrq_disp->dis_xdrargs : NULL,
			  nreq->nrq_args)) {
		error++;
		DPRINTF(nfssvdebug,
			("dispatch: svc_freeargs returns %d\n", error));
		
	}
	
	if (nreq->nrq_args != NULL) {
		/* rfsput((struct rfsspace *)nreq->nrq_args); */
		RFSPUT(nreq->nrq_args);
	}

	/*
	 * restore original credentials
	 */
	if (nreq->nrq_holdcred) {
		tmpcr = u.u_cred;
		u.u_cred = nreq->nrq_holdcred;
		crfree(tmpcr);
	}
	/*smp_lock(&lk_nfsstat, LK_RETRY);*/
	svstat.nbadcalls += error;
	/*smp_unlock(&lk_nfsstat);*/

	/*
	 * Free request struct.
	 */
	/* rfsputreq(nreq); */
	RFSPUTREQ(nreq);

	DPRINTF(nfssvdebug, ("rfs_sendreply: done\n"));

}

/*
 * Determine if two addresses are equal
 */
eqaddr(addr1, addr2)
        struct sockaddr *addr1;
        struct sockaddr *addr2;
{
        return (((struct sockaddr_in *) addr1)->sin_addr.s_addr ==
                ((struct sockaddr_in *) addr2)->sin_addr.s_addr);

        return (0);
}

hostinlist(sa, addrs)
        struct sockaddr *sa;
        struct exaddrlist *addrs;
{
        int i;

        for (i = 0; i < addrs->naddrs; i++) {
                if (eqaddr(sa, &addrs->addrvec[i])) {
                        return (1);
                }
        }
        return (0);
}

checkauth(xpd, req, cred)
	struct kexport *xpd;
	struct svc_req *req;
	struct ucred *cred;
{
        struct authunix_parms *aup;
        int flavor, len;
	uid_t anon, rootmap;

        /*
         * Set uid, gid, and gids to auth params
         */
        flavor = req->rq_cred.oa_flavor;

	anon = xpd->e_anon;
	rootmap = xpd->e_rootmap;

	if (anon == (uid_t) -2) 
		anon = 65534; /* -2 when uid_t is a short */
	if (rootmap == (uid_t) -2) 
		rootmap = 65534;

	switch (flavor) {
        case AUTH_NULL:
		cred->cr_uid = anon;
		cred->cr_gid = anon;
		cred->cr_ngroups = 0;
		break;

	case AUTH_UNIX:
                aup = (struct authunix_parms *)req->rq_clntcred;
		/*
		 * If root user and rootmap!=0 and client is not in
		 * list of addrs allowed root access, then set auth 
		 * to anon uid.
		 */
                if (aup->aup_uid == 0 && 
		    (rootmap != 0 && !hostinlist((struct sockaddr *)
			    svc_getcaller(req->rq_xprt), &xpd->e_rootaddrs))) {

			cred->cr_uid = anon;
			cred->cr_gid = anon;
			cred->cr_ngroups = 0;
		} else {
			/*
			 * Hack around the fact that ufs (and various other
			 * portions of the kernel) do not support 32 bit
			 * uid/gid's.  We need a way of representing user
			 * "nobody" whose uid is -2 which is 0xfffffffe
			 * in 32 bits and 65534 in 16 bits.
			 */
			if ((uid_t)aup->aup_uid == (uid_t)0xfffffffe)
                        	cred->cr_uid = 65534;
			else
                        	cred->cr_uid = (uid_t)aup->aup_uid;
			if ((uid_t)aup->aup_gid == (uid_t)0xfffffffe)
                        	cred->cr_gid = 65534;
			else
                        	cred->cr_gid = (uid_t)aup->aup_gid;
                        len = MIN(aup->aup_len, NGROUPS);
                        cred->cr_ngroups = len;
                        bcopy((caddr_t)aup->aup_gids, (caddr_t)cred->cr_groups,
                               len * sizeof (gid_t));
		}
		break;

	default:
		return(0);
	}
	return(cred->cr_uid != (uid_t) -1);
}
