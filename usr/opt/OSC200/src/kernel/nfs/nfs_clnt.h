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
 * @(#)$RCSfile: nfs_clnt.h,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1992/08/25 14:08:12 $
 */

/*	@(#)nfs_clnt.h	1.4 90/07/02 NFSSRC4.1 from 2.30 89/08/31 SMI 	*/

#ifndef __NFS_CLNT_HEADER__
#define __NFS_CLNT_HEADER__

/*
 * vfs pointer to mount info
 */
#define	vftomi(mp)	((struct mntinfo *)((mp)->m_data))

/*
 * vnode pointer to mount info
 */
#define	vtomi(vp)	((struct mntinfo *)(((vp)->v_mount)->m_data))

/*
 * NFS vnode to server's block size
 */
#define	vtoblksz(vp)	(vtomi(vp)->mi_bsize)


#define	HOSTNAMESZ	32
#define	ACREGMIN	3	/* min secs to hold cached file attr */
#define	ACREGMAX	60	/* max secs to hold cached file attr */
#define	ACDIRMIN	30	/* min secs to hold cached dir attr */
#define	ACDIRMAX	60	/* max secs to hold cached dir attr */
#define ACMINMAX        3600    /* 1 hr is longest min timeout */
#define ACMAXMAX        36000   /* 10 hr is longest max timeout */

#define	NFS_CALLTYPES	3	/* Lookups, Reads, Writes */

/*
 * Fake errno passed back from rfscall to indicate transfer size adjustment
 */
#define	ENFS_TRYAGAIN	999

/*
 * NFS private data per mounted file system
 */
struct mntinfo {
	struct sockaddr_in mi_addr;	/* server's address */
	struct vnode	*mi_rootvp;	/* root vnode */
	u_int		 mi_hard : 1;	/* hard or soft mount */
	u_int		 mi_printed : 1; /* not responding message printed */
	u_int		 mi_int : 1;	/* interrupts allowed on hard mount */
	u_int		 mi_down : 1;	/* server is down */
	u_int		 mi_noac : 1;	/* don't cache attributes */
	u_int		 mi_nocto : 1;	/* no close-to-open consistency */
	u_int		 mi_dynamic : 1; /* dynamic transfer size adjustment */
	int		 mi_refct;	/* active vnodes for this vfs */
	int		 mi_tsize;	/* transfer size (bytes) */
					/* really read size */
	int		 mi_stsize;	/* server's max transfer size (bytes) */
					/* really write size */
	int		 mi_bsize;	/* server's disk block size */
	int		 mi_mntno;	/* kludge to set client rdev for stat*/
	int		 mi_timeo;	/* inital timeout in 10th sec */
	int		 mi_retrans;	/* times to retry request */
	char		 mi_hostname[HOSTNAMESZ];	/* server's hostname */
	char		*mi_netname;	/* server's netname */
	int		 mi_netnamelen;	/* length of netname */
	int		 mi_authflavor;	/* authentication type */
	u_int		 mi_acregmin;	/* min secs to hold cached file attr */
	u_int		 mi_acregmax;	/* max secs to hold cached file attr */
	u_int		 mi_acdirmin;	/* min secs to hold cached dir attr */
	u_int		 mi_acdirmax;	/* max secs to hold cached dir attr */
	/*
	 * Extra fields for congestion control, one per NFS call type,
	 * plus one global one.
	 */
	struct rpc_timers mi_timers[NFS_CALLTYPES+1];
	int		mi_curread;	/* current read size */
	int		mi_curwrite;	/* current write size */
	struct pathcnf *mi_pathconf;	/* static pathconf kludge */
	int		mi_flags;
};

/*
 * Mark cached attributes as timed out
 */
#define	PURGE_ATTRCACHE(vp)	(vtor(vp)->r_attrtime = time)

/*
 * Mark cached attributes as uninitialized (must purge all caches first)
 */
#define	INVAL_ATTRCACHE(vp)	(vtor(vp)->r_attrtime.tv_sec = 0)


/*
 * If returned error is ESTALE flush all caches.
 */
#define PURGE_STALE_FH(errno, vp) if ((errno) == ESTALE) {ubc_invalidate(vp, 0, 0, B_INVAL|B_BUSY); vtor(vp)->r_attrtime.tv_sec = 0; cache_purge(vp); vtor(vp)->r_flags &= ~REOF;}

/*
/*
 * Is cache valid?
 * Swap is always valid, if no attributes (attrtime == 0) or
 * if mtime matches cached mtime it is valid
 * If server thinks that filesize has changed then caches aren't valid.
 */
#define	CACHE_VALID(rp, mtime, fsize) \
	( (rp)->r_attrtime.tv_sec == 0 || \
	    (((mtime).tv_sec == (rp)->r_attr.va_mtime.tv_sec && \
	    (mtime).tv_usec == (rp)->r_attr.va_mtime.tv_usec) && \
	    ((fsize) == (rp)->r_attr.va_size)))


#endif /* !__NFS_CLNT_HEADER__ */
