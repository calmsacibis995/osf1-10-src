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
 * @(#)$RCSfile: rnode.h,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/03/30 22:28:18 $
 */
/*	@(#)rnode.h	1.5 90/07/02 NFSSRC4.1 from 1.27 89/12/13 SMI 	*/
/*	Copyright (C) 1988, Sun Microsystems Inc. */

#ifndef _nfs_rnode_h
#define _nfs_rnode_h

/*
 * Remote file information structure.
 * The rnode is the "inode" for remote files.  It contains all the
 * information necessary to handle remote file on the client side.
 *
 * Note on file sizes:  we keep two file sizes in the rnode: the size
 * according to the client (r_size) and the size according to the server
 * (r_attr.va_size).   They can differ because we modify r_size during a
 * write system call (nfs_rdwr), before the write request goes over the
 * wire (before the file is actually modified on the server).  If an OTW
 * request occurs before the cached data is written to the server the file
 * size returned from the server (r_attr.va_size) may not match r_size. 
 * r_size is the one we use, in general.  r_attr->va_size is only used to
 * determine whether or not our cached data is valid.
 */

struct rnode {
	struct rnode	*r_freef;	/* free list forward pointer */
	struct rnode	*r_freeb;	/* free list back pointer */
	struct rnode	*r_hash;	/* rnode hash chain */
	struct vnode	*r_vnode;	/* vnode for remote file */
	nfsv2fh_t	r_fh;		/* file handle */
	u_short		r_flags;	/* flags, see below */
	short		r_error;	/* async write error */
	daddr_t		r_lastr;	/* last block read (read-ahead) */
	int		r_owner;	/* proc index for locker of rnode */
	int		r_count;	/* number of rnode locks for r_owner */
	struct ucred	*r_cred;	/* current credentials */
	struct nameidata *r_ndp;	/* unlink information */
	u_int		r_size;		/* client's view of file size	*/
	struct vattr	r_attr;		/* cached vnode attributes */
	struct timeval	r_attrtime;	/* time attributes become invalid */
};

/*
 * Flags
 */
#define	RLOCKED		0x01		/* rnode is in use */
#define	RWANT		0x02		/* someone wants a wakeup */
#define	RATTRVALID	0x04		/* Attributes in the rnode are valid */
#define	REOF		0x08		/* EOF encountered on read */
#define	RDIRTY		0x10		/* dirty pages from write operation */
#define RNOCACHE	0x20		/* don't cache read and write blocks */
#define RFREEING	0x40		/* Rnode is being freed */

/*
 * Convert between vnode and rnode
 * rtol() returns a symlink cache string pointer. Details in nfs_readlink().
 */
#define	rtov(rp)	((rp)->r_vnode)
#define	vtor(vp)	((struct rnode *)((vp)->v_data))
#define	vtofh(vp)	(&(vtor(vp)->r_fh))
#define	rtofh(rp)	(&(rp)->r_fh)
#define rtol(rp)	(((char *) (rp)) + sizeof(struct rnode))

/*
 * Lock and unlock rnodes.
 */
#define	RLOCK(rp) { \
	while (((rp)->r_flags & RLOCKED) && \
	    (rp)->r_owner != u.u_procp - proc) { \
		(rp)->r_flags |= RWANT; \
		(void) sleep((caddr_t)(rp), PINOD); \
	} \
	(rp)->r_owner = u.u_procp - proc; \
	(rp)->r_count++; \
	(rp)->r_flags |= RLOCKED; \
}

#define	RUNLOCK(rp) { \
	if (--(rp)->r_count < 0) \
		panic("RUNLOCK"); \
	if ((rp)->r_count == 0) { \
		(rp)->r_flags &= ~RLOCKED; \
		if ((rp)->r_flags & RWANT) { \
			(rp)->r_flags &= ~RWANT; \
			wakeup((caddr_t)(rp)); \
		} \
	} \
}
#endif /* !_nfs_rnode_h */
