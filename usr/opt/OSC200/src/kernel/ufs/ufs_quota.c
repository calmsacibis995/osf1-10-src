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
static char	*sccsid = "@(#)$RCSfile: ufs_quota.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/05/12 12:59:27 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * OSF/1 Release 1.0.3
 */
/*
 * Copyright (c) 1982, 1986, 1990 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Robert Elz at The University of Melbourne.
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
 *	@(#)ufs_quota.c	7.4 (Berkeley) 6/28/90
 */
#include <sys/secdefines.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/ucred.h>
#include <sys/namei.h>
#include <sys/errno.h>
#if	MACH
#include <kern/thread.h>
#include <kern/kalloc.h>
#include <kern/macro_help.h>
#else
#include <sys/malloc.h>
#endif
#include <sys/file.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/lock_types.h>
#include <ufs/fs.h>
#include <ufs/fs_proto.h>
#include <ufs/quota.h>
#include <ufs/inode.h>
#include <ufs/ufsmount.h>


/*
 * Quota statistics.
 */
u_int dq_collisions;			/* redundant insertion count */
vdecl_simple_lock_data(,quota_stats_lock)
#define	QSTATS(action)		STATS_ACTION(&quota_stats_lock,(action))

#if	SEC_PRIV
#define	ESCAPE_QCHECK(cr)	privileged(SEC_LIMIT,0)
#else
#define	ESCAPE_QCHECK(cr)	((cr)->cr_uid == 0)
#endif


/*
 * Quota name to error message mapping.
 */
static char *quotatypes[] = INITQFNAMES;

/*
 * Set up the quotas for an inode.
 *
 * This routine completely defines the semantics of quotas.
 * If other criterion want to be used to establish quotas, the
 * MAXQUOTAS value in quotas.h should be increased, and the
 * additional dquots set up here.
 */
getinoquota(ip)
	register struct inode *ip;
{
	struct ufsmount *ump;
	struct vnode *vp = ITOV(ip);
	struct dquot *dq;
	int error;
	u_int flag;

	ump = VFSTOUFS(vp->v_mount);
	UMPQ_READ_LOCK(ump);
	error = 0;

	/*
	 * Is someone else already filling in the dquots?
	 * If so, wait for them to finish; if not, indicate
	 * that we are about to fill in the dquots.
	 */
	IN_LOCK(ip);
	while (ip->i_flag & IQUOTING) {
		ip->i_flag |= IQUOTWAIT;
		assert_wait((vm_offset_t)ip->i_dquot, FALSE);
		IN_UNLOCK(ip);
		thread_block();
		IN_LOCK(ip);
	}
	ip->i_flag |= IQUOTING;
	IN_UNLOCK(ip);

	/*
	 * Set up the user quota based on file uid.
	 * EINVAL means that quotas are not enabled.
	 */
	if ((dq = ip->i_dquot[USRQUOTA]) == NODQUOT)
		if (!(error = dqget(vp, ip->i_uid, ump, USRQUOTA, &dq))) {
			ip->i_dquot[USRQUOTA] = dq;
			DQ_UNLOCK(dq);
		} else if (error != EINVAL)
			goto out;

	/*
	 * Set up the group quota based on file gid.
	 * EINVAL means that quotas are not enabled.
	 */
	if ((dq = ip->i_dquot[GRPQUOTA]) == NODQUOT)
		if (!(error = dqget(vp, ip->i_gid, ump, GRPQUOTA, &dq))) {
			ip->i_dquot[GRPQUOTA] = dq;
			DQ_UNLOCK(dq);
		}

out:
	IN_LOCK(ip);
	flag = ip->i_flag;
	ip->i_flag &= ~(IQUOTING|IQUOTWAIT);
	IN_UNLOCK(ip);
	if (flag & IQUOTWAIT)
		thread_wakeup((vm_offset_t)ip->i_dquot);
	UMPQ_READ_UNLOCK(ump);
	return (error == EINVAL ? 0 : error);
}

/*
 * Update disk usage, and take corrective action.
 *
 * N.B.:  We must update each quota as it is checked,
 * which forces us to back out whatever changes we've
 * made if the proposed change fails.
 */
chkdq(ip, change, cred, flags)
	register struct inode *ip;
	long change;
	struct ucred *cred;
	int flags;
{
	register struct dquot *dq;
	register int i;
	int ncurblocks, error;
	struct mount *mp;
	struct ufsmount *ump;
	u_int flagword;

	LASSERT(IN_WRITE_HOLDER(ip));

	mp = ITOV(ip)->v_mount;
	/*
	 * If the filesystem doesn't have
	 * quotas turned on, terminate early.
	 */
	BM(MOUNT_LOCK(mp));
	flagword = mp->m_flag;
	BM(MOUNT_UNLOCK(mp));
	if ((flagword & M_QUOTA) == 0)
		return (0);

#ifdef DIAGNOSTIC
	if ((flags & CHOWN) == 0)
		chkdquot(ip);
#endif

	if (change == 0)
		return (0);

	/*
	 * Loop through the inode's dquots checking all
	 * existing quotas.
	 */
	ump = VFSTOUFS(mp);
	UMPQ_READ_LOCK(ump);
	for (i = 0; i < MAXQUOTAS; ++i) {
		if ((dq = ip->i_dquot[i]) == NODQUOT)
			continue;
		DQ_LOCK(dq);
		if (change < 0) {
			ncurblocks = dq->dq_curblocks + change;
			if (ncurblocks >= 0)
				dq->dq_curblocks = ncurblocks;
			else
				dq->dq_curblocks = 0;
			dq->dq_flags &= ~DQ_BLKS;
			dq->dq_flags |= DQ_MOD;
		} else {
			if ((flags & FORCE) == 0 && !ESCAPE_QCHECK(cred))
				if (error = chkdqchg(ip, change, cred, i)) {
					DQ_UNLOCK(dq);
					/*
					 * Back out previous changes.  Must
					 * leave dquot marked as modified.
					 */
					for (--i; i >= 0; --i) {
						dq = ip->i_dquot[i];
						if (dq == NODQUOT)
							continue;
						DQ_LOCK(dq);
						dq->dq_curblocks -= change;
						DQ_UNLOCK(dq);
					}
					UMPQ_READ_UNLOCK(ump);
					return (error);
				}
			dq->dq_curblocks += change;
			dq->dq_flags |= DQ_MOD;
		}
		DQ_UNLOCK(dq);
	}
	UMPQ_READ_UNLOCK(ump);
	return (0);
}

/*
 * Check for a valid change to a users allocation.
 * Issue an error message if appropriate.
 */
chkdqchg(ip, change, cred, type)
	struct inode *ip;
	long change;
	struct ucred *cred;
	int type;
{
	register struct dquot *dq = ip->i_dquot[type];
	int ncurblocks, sec;
	int s;

	LASSERT(IN_WRITE_HOLDER(ip));
	LASSERT(DQ_HOLDER(dq));
	LASSERT(UMPQ_READ_HOLDER(VFSTOUFS(ITOV(ip)->v_mount)));

	ncurblocks = dq->dq_curblocks + change;
	/*
	 * If user would exceed their hard limit, disallow space allocation.
	 * If hardlimit is 1, disallow any space allocation.
	 */
	if ((dq->dq_bhardlimit == 1 && ncurblocks) ||
	    (ncurblocks > dq->dq_bhardlimit && dq->dq_bhardlimit)) {
		/*
		 * i_uid is invariant under inode I/O writelock.
		 */
		if ((dq->dq_flags & DQ_BLKS) == 0 &&
		    ip->i_uid == cred->cr_uid) {
			/*
			 * It's not a great idea to hold the inode, quota
			 * and ufsmount quota locks across a user-land
			 * printf.  4.3-Reno does the printf with the
			 * inode locked, but that's not a good excuse.
			 * What if the user flow controls the message?  XXX
			 */
			uprintf("\n%s: write failed, %s disk limit reached\n",
				ip->i_fs->fs_fsmnt, quotatypes[type]);
			dq->dq_flags |= DQ_BLKS;
		}
		return (EDQUOT);
	}
	/*
	 * If user is over their soft limit for too long, disallow space
	 * allocation. Reset time limit as they cross their soft limit.
	 * If soft limit is 1, we allow temporary allocation only (i.e.
	 * the user cannot allocate 1 block "forever").
	 */
	if ((dq->dq_bsoftlimit == 1 && ncurblocks) ||
	    (ncurblocks > dq->dq_bsoftlimit && dq->dq_bsoftlimit)) {
		s = splhigh();
		TIME_READ_LOCK();
		sec = time.tv_sec;
		TIME_READ_UNLOCK();
		splx(s);
		if (dq->dq_curblocks < dq->dq_bsoftlimit) {
			dq->dq_btime = sec +
			    VFSTOUFS(ITOV(ip)->v_mount)->um_btime[type];
			if (ip->i_uid == cred->cr_uid)
				uprintf("\n%s: warning, %s %s\n",
					ip->i_fs->fs_fsmnt, quotatypes[type],
					"disk quota exceeded");
			return (0);
		}
		if (sec > dq->dq_btime) {
			if ((dq->dq_flags & DQ_BLKS) == 0 &&
			    ip->i_uid == cred->cr_uid) {
				uprintf("\n%s: write failed, %s %s\n",
					ip->i_fs->fs_fsmnt, quotatypes[type],
					"disk quota exceeded too long");
				dq->dq_flags |= DQ_BLKS;
			}
			return (EDQUOT);
		}
	}
	return (0);
}

/*
 * Check the inode limit, applying corrective action.
 *
 * Caller makes two guarantees.  First, the caller
 * holds the inode referenced so it won't be deallocated.
 * Second, the caller guarantees that chkiq does not
 * race ufs/ufs_vnops.c:chown1().  In most cases, chkiq
 * isi called when the inode is newly allocated or about
 * to be freed, so no one else knows about the inode and
 * therefore there's no concern about racing chown1.  In any
 * other case, the caller must acquire the inode I/O write
 * lock to guarantee that chkiq does not race chown1.
 */
chkiq(ip, change, cred, flags)
	register struct inode *ip;
	long change;
	struct ucred *cred;
	int flags;
{
	register struct dquot *dq;
	register struct mount *mp;
	register struct ufsmount *ump;
	register int i;
	int ncurinodes, flagword, error;

	mp = ITOV(ip)->v_mount;

	/*
	 * If the filesystem doesn't have
	 * quotas turned on, terminate early.
	 */
	BM(MOUNT_LOCK(mp));
	flagword = mp->m_flag;
	BM(MOUNT_UNLOCK(mp));
	if ((flagword & M_QUOTA) == 0)
		return (0);

#ifdef DIAGNOSTIC
	if ((flags & CHOWN) == 0)
		chkdquot(ip);
#endif

	if (change == 0)
		return (0);

	/*
	 * Loop through the inode's dquots checking all
	 * existing quotas.
	 */
	ump = VFSTOUFS(mp);
	UMPQ_READ_LOCK(ump);
	for (i = 0; i < MAXQUOTAS; ++i) {
		if ((dq = ip->i_dquot[i]) == NODQUOT)
			continue;
		DQ_LOCK(dq);
		if (change < 0) {
			ncurinodes = dq->dq_curinodes + change;
			if (ncurinodes >= 0)
				dq->dq_curinodes = ncurinodes;
			else
				dq->dq_curinodes = 0;
			dq->dq_flags &= ~DQ_INODS;
			dq->dq_flags |= DQ_MOD;
		} else {
			if ((flags & FORCE) == 0 && !ESCAPE_QCHECK(cred))
				if (error = chkiqchg(ip, change, cred, i)) {
					DQ_UNLOCK(dq);
					for (--i; i >= 0; --i) {
						dq = ip->i_dquot[i];
						if (dq == NODQUOT)
							continue;
						DQ_LOCK(dq);
						dq->dq_curinodes -= change;
						DQ_UNLOCK(dq);
					}
					UMPQ_READ_UNLOCK(ump);
					return (error);
				}
			dq->dq_curinodes += change;
			dq->dq_flags |= DQ_MOD;
		}
		DQ_UNLOCK(dq);
	}
	UMPQ_READ_UNLOCK(ump);
	return (0);
}

/*
 * Check for a valid change to a users allocation.
 * Issue an error message if appropriate.
 *
 * The guarantees required of chkiq's caller also apply here.
 */
chkiqchg(ip, change, cred, type)
	struct inode *ip;
	long change;
	struct ucred *cred;
	int type;
{
	register struct dquot *dq = ip->i_dquot[type];
	int ncurinodes, sec;
	uid_t iuid;
	int s;

	LASSERT(DQ_HOLDER(dq));
	LASSERT(UMPQ_READ_HOLDER(VFSTOUFS(ITOV(ip)->v_mount)));

	/*
	 * chkiqchg may be called under some circumstances
	 * without the inode I/O lock held for writing.  In that
	 * case, technically the use of i_uid falls under BM.
	 */
	BM(IN_LOCK(ip));
	iuid = ip->i_uid;
	BM(IN_UNLOCK(ip));

	ncurinodes = dq->dq_curinodes + change;
	/*
	 * If user would exceed their hard limit, disallow inode allocation.
	 * If hard limit is 1, disallow all allocation.
	 */
	if ((dq->dq_ihardlimit == 1 && ncurinodes) ||
	   (ncurinodes > dq->dq_ihardlimit && dq->dq_ihardlimit)) {
		if ((dq->dq_flags & DQ_INODS) == 0 &&
		    iuid == cred->cr_uid) {
			/*
			 * See note in chkdqchg about holding inode
			 * and dquot locks across uprintf.
			 */
			uprintf("\n%s: write failed, %s inode limit reached\n",
			    ip->i_fs->fs_fsmnt, quotatypes[type]);
			dq->dq_flags |= DQ_INODS;
		}
		return (EDQUOT);
	}
	/*
	 * If user is over their soft limit for too long, disallow inode
	 * allocation. Reset time limit as they cross their soft limit.
	 * If soft limit is 1, allow temporary allocation only.
	 */
	if ((dq->dq_isoftlimit == 1 && ncurinodes) ||
	    (ncurinodes > dq->dq_isoftlimit && dq->dq_isoftlimit)) {
		s = splhigh();
		TIME_READ_LOCK();
		sec = time.tv_sec;
		TIME_READ_UNLOCK();
		splx(s);
		if (dq->dq_curinodes < dq->dq_isoftlimit) {
			dq->dq_itime = sec +
			    VFSTOUFS(ITOV(ip)->v_mount)->um_itime[type];
			if (iuid == cred->cr_uid)
				uprintf("\n%s: warning, %s %s\n",
				    ip->i_fs->fs_fsmnt, quotatypes[type],
				    "inode quota exceeded");
			return (0);
		}
		if (sec > dq->dq_itime) {
			if ((dq->dq_flags & DQ_INODS) == 0 &&
			    iuid == cred->cr_uid) {
				uprintf("\n%s: write failed, %s %s\n",
				    ip->i_fs->fs_fsmnt, quotatypes[type],
				    "inode quota exceeded too long");
				dq->dq_flags |= DQ_INODS;
			}
			return (EDQUOT);
		}
	}
	return (0);
}

#ifdef	DIAGNOSTIC
/*
 * On filesystems with quotas enabled,
 * it is an error for a file to change size and not
 * to have a dquot structure associated with it.
 *
 * Not strictly true on a multiprocessor.  Quotas could
 * be turned off any old time between a getinoquota/chkdq
 * sequence.
 */
chkdquot(ip)
	register struct inode *ip;
{
	struct ufsmount *ump;
	struct mount *mp;
	register int i;
	register struct dquot *dq;

	mp = ITOV(ip)->v_mount;
	ump = VFSTOUFS(mp);
	UMPQ_READ_LOCK(ump);

	for (i = 0; i < MAXQUOTAS; i++) {
		if (ump->um_quotas[i] == NULLVP)
			continue;
		dq = ip->i_dquot[i];
		if (dq == NODQUOT) {
			vprint("chkdquot: missing dquot", ITOV(ip));
			panic("missing dquot");
		}
		BM(MOUNT_LOCK(mp));
		ASSERT(mp->m_flag & M_QUOTA);
		BM(MOUNT_UNLOCK(mp));
	}
	UMPQ_READ_UNLOCK(ump);
}
#endif /* DIAGNOSTIC */


/*
 * Changing ownership of a file is a very special case indeed.
 * This routine depends on having chown serialized via the inode
 * I/O lock.  Thus, there are no concerns about racing chowns or
 * about quota_chown racing getinoquota.
 *
 * Steps in the process:
 *
 *	1.  Build vector of dquots differing between old owner and new.
 *	2.  Build a fake inode containing the differing dquots.
 *	3.  Charge block count to differing dquots.
 *	4.  Attach new dquots to inode in place of old ones.
 *
 * There is no need to check the proposed change against dquots that
 * don't change between the old and new ownership.
 *
 * Results:
 *	Success:  old dquots lose blocks, new dquots gain them; caller
 *		must immediately change inode uid and/or gid to match!
 *	Failure:  no changes to inode's old dquots.
 */
quota_chown(ip, new_uid, new_gid, flags, cred)
register struct inode *ip;
uid_t new_uid;
gid_t new_gid;
struct ucred *cred;
{
	register struct vnode *vp, *tvp;
	register struct inode *tip;
	struct ufsmount *ump;
	int different[MAXQUOTAS];
	u_short owner[MAXQUOTAS];
	uid_t ouid;
	gid_t ogid;
	long change;
	int error, i;
	char fake_vnode[FAKE_INODE_SIZE];

	LASSERT(IN_WRITE_HOLDER(ip));

	ouid = (uid_t) ip->i_uid;
	ogid = (gid_t) ip->i_gid;
	change = ip->i_blocks;
	vp = ITOV(ip);
	tvp = (struct vnode *) fake_vnode;
	fake_inode_init(tvp, vp->v_mount);
	tip = VTOI(tvp);
	tip->i_uid = new_uid;
	tip->i_gid = new_gid;
#if	MACH_ASSERT || MACH_LDEBUG
	IN_WRITE_LOCK(tip);
#endif
	error = 0;
	ump = VFSTOUFS(vp->v_mount);

	/*
	 * 1.  Compute differing dquots.
	 */
	different[USRQUOTA] = (ouid != new_uid);
	owner[USRQUOTA] = (u_short) new_uid;
	different[GRPQUOTA] = (ogid != new_gid);
	owner[GRPQUOTA] = (u_short) new_gid;

	UMPQ_READ_LOCK(ump);

	/*
	 * 2.  Build fake inode containing differing dquots.
	 */
	for (i = 0; i < MAXQUOTAS; ++i)
		if (different[i]) {
			error = dqget(vp, owner[i], ump, i, &tip->i_dquot[i]);
			if (error == 0)
				DQ_UNLOCK(tip->i_dquot[i]);
			else if (error != EINVAL)
				goto out;
		} else
			tip->i_dquot[i] = NODQUOT;

	/*
	 * 3.  Check whether new owner can take on additional blocks
	 * and inode.  Unfortunately, we don't have a way to check both
	 * changes atomically.  Note that this chkiq call requires
	 * holding the inode I/O write lock to prevent chkiq from racing
	 * chown.  Furthermore, note that we take the ufsmount quota
	 * lock recursively for reading when we call chkdq and chkiq.
	 */
	if (error = chkdq(tip, change, cred, CHOWN|flags))
		goto out;
	if (error = chkiq(tip, 1, cred, CHOWN|flags)) {
		(void) chkdq(tip, -change, cred, CHOWN|FORCE|flags);
		goto out;
	}
	/* Now we have successfully given the new owner the additional
	 * blocks.  We want to remove the blocks from the old owner.
	 *
	 * We can't just call chkdq() using the real inode ip (for the
	 * old owner/group), because the only quotas which need to be
	 * adjusted those which are changing, i.e. see above where we
	 * attached tip->i_dquot[i] only if different[i].  Note that
	 * simply calling chkdq using ip will over decrement the quotas
	 * if !different[i].
	 * 
	 * We'll swap the dquots below, so that the real inode ip will
	 * have the correct quotas (i.e. it will appear that the chown/chgrp
	 * has already succeeded).  And we'll setup the fake inode to now
	 * look like the old owner/group with the old quotas attached.
	 */
	ASSERT(error == 0);

out:
	for (--i; i >= 0; --i) {
		if (different[i]) {
			if (error) {
				/*
				 * Release accumulated new dquots.
				 */
				dqrele(vp, tip->i_dquot[i]);
			} else {
				/* 4.  Replace old dquots with new ones.
				 *     Note that only the i_dquot[i] which
				 *     are different[i] will be swapped
				 *     between ip and tip;  i.e. tip will
				 *     have the dquots which need to be
				 *     adjusted for the old owner/group.
				 */
				register struct dquot *tmp_dqp;	/* temp swap */

				tmp_dqp = ip->i_dquot[i];
				ip->i_dquot[i]  = tip->i_dquot[i];
				tip->i_dquot[i] = tmp_dqp;
			}
		}
	}

	if (!error) {
		/* 5.  Remove the quotas from the original owner as well;
		 *     we've left tip (the fake inode) from step#4 above,
		 *     with the original quotas (which were different[i])
		 *     for the old owner/group.  So we use tip to now
		 *     adjust the original owner/group quotas.
		 */
		tip->i_uid = ouid;	/* first make tip look like */
		tip->i_gid = ogid;	/*    old owner/group.      */
		if ((error = chkdq(tip, -change, cred, CHOWN|flags)) == 0)
			if (error = chkiq(tip, -1, cred, CHOWN|flags))
				chkdq(tip, change, cred, CHOWN|FORCE|flags);
		/* finally, release our old/original dquots */
		for (i=0; i<MAXQUOTAS; i++)
			if (different[i])
				dqrele(vp, tip->i_dquot[i]);
	}
	UMPQ_READ_UNLOCK(ump);
#if	MACH_ASSERT || MACH_LDEBUG
	IN_WRITE_UNLOCK(tip);
#endif
	return (error);
}

/*
 * Code to process quotactl commands.
 */

/*
 * Q_QUOTAON - set up a quota file for a particular file system.
 */
quotaon(ndp, mp, type, fname)
	register struct nameidata *ndp;
	struct mount *mp;
	register int type;
	caddr_t fname;
{
	register struct ufsmount *ump = VFSTOUFS(mp);
	register struct vnode *vp;
	register struct inode *ip;
	struct vnode *nextvp;
	struct dquot *dq;
	int error, skip;
	
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = fname;
	if (error = vn_open(ndp, FREAD|FWRITE, 0))
		return (error);
	vp = ndp->ni_vp;
	if (vp->v_type != VREG) {
		vrele(vp);
		return (EACCES);
	}

	/*
	 * Block out all other quota operations on
	 * this filesystem.  When we finally acquire
	 * the lock, all pending quota operations
	 * will have completed and no new ones will
	 * be able to start.
	 *
	 * Set the ufsmount quota lock recursive so we
	 * can re-use quotaoff and getinoquota.
	 */
	UMPQ_WRITE_LOCK(ump);
	UMPQ_LOCK_RECURSIVE(ump);

	/*
	 * Turn off any existing quotas.
	 * After doing so, no one else is
	 * using the um_q*[type] fields we
	 * intend to use.
	 */
	if (ump->um_quotas[type] != NULLVP)
		quotaoff(mp, type);
	ASSERT(ump->um_quotas[type] == NULLVP);
	ASSERT(ump->um_cred[type] == NOCRED);

	MOUNT_LOCK(mp);
	mp->m_flag |= M_QUOTA;
	MOUNT_UNLOCK(mp);
	VN_LOCK(vp);
	vp->v_flag |= VSYSTEM;
	VN_UNLOCK(vp);
	ump->um_quotas[type] = vp;


	/*
	 * Save the credential of the process that turned on quotas.
	 * Set up the time limits for this quota.
	 */
	crhold(ndp->ni_cred);
	ump->um_cred[type] = ndp->ni_cred;
	ump->um_btime[type] = MAX_DQ_TIME;
	ump->um_itime[type] = MAX_IQ_TIME;
	if (dqget(NULLVP, 0, ump, type, &dq) == 0) {
		if (dq->dq_btime > 0)
			ump->um_btime[type] = dq->dq_btime;
		if (dq->dq_itime > 0)
			ump->um_itime[type] = dq->dq_itime;
		dqput(NULLVP, dq);
	}

	/*
	 * Search vnodes associated with this mount point,
	 * adding references to quota file being opened.
	 */
	error = 0;
	MOUNT_VLIST_LOCK(mp);
again:
	for (vp = mp->m_mounth; vp; vp = nextvp) {
		nextvp = vp->v_mountf;
		VN_LOCK(vp);
		skip = (vp->v_usecount == 0 || vp->v_wrcnt == 0 ||
			vget_nowait(vp));
		VN_UNLOCK(vp);
		if (skip)
			continue;
		MOUNT_VLIST_UNLOCK(mp);
		error = getinoquota(VTOI(vp));
		vrele(vp);
		if (error) {
			printf("quotaon:  error initializing quotas\n");
			/*
			 * We don't expect this case to happen often so
			 * we don't mind taking the lock just so we can
			 * release it again, below.
			 */
			MOUNT_VLIST_LOCK(mp);
			break;
		}
		MOUNT_VLIST_LOCK(mp);
		/*
		 * Dangerous check:  assumes that if vp is deallocated
		 * the memory is not re-used for something other than
		 * a vnode.  (Direct from 4.3BSD-Reno.)
		 */
		if (vp->v_mountf != nextvp || vp->v_mount != mp)
			goto again;
	}
	MOUNT_VLIST_UNLOCK(mp);

	if (error)
		quotaoff(mp, type);
#if	MACH_ASSERT
	else {
		vp = ump->um_quotas[type];
		BM(VN_LOCK(vp));
		ASSERT(vp->v_usecount > 0);
		BM(VN_UNLOCK(vp));
	}
#endif

	UMPQ_LOCK_UNRECURSIVE(ump);
	UMPQ_WRITE_UNLOCK(ump);

	return (error);
}

/*
 * Q_QUOTAOFF - turn off disk quotas for a filesystem.
 */
quotaoff(mp, type)
	struct mount *mp;
	register int type;
{
	register struct vnode *vp;
	struct vnode *qvp, *nextvp;
	struct ufsmount *ump = VFSTOUFS(mp);
	register struct dquot *dq;
	register struct inode *ip;
	int skip;

	UMPQ_WRITE_LOCK(ump);

	if ((qvp = ump->um_quotas[type]) == NULLVP)
		goto out;
	ump->um_qflags[type] |= QTF_CLOSING;

#if	MACH_ASSERT
	BM(VN_LOCK(qvp));
	ASSERT(qvp->v_usecount > 0);
	BM(VN_UNLOCK(vp));
#endif
	/*
	 * Very ugly indeed:  vrele can call ufs_inactive, which
	 * in turn will call getinoquota and dqget.  We must permit
	 * dqget to take the ufsmount lock recursively.  Ugh!
	 */
	UMPQ_LOCK_RECURSIVE(ump);

	/*
	 * Search vnodes associated with this mount point,
	 * deleting any references to quota file being closed.
	 */
	MOUNT_VLIST_LOCK(mp);
again:
	for (vp = mp->m_mounth; vp; vp = nextvp) {
		nextvp = vp->v_mountf;
		VN_LOCK(vp);
		skip = vget_nowait(vp);
		VN_UNLOCK(vp);
		if (skip)
			continue;
		MOUNT_VLIST_UNLOCK(mp);
		ip = VTOI(vp);
		dq = ip->i_dquot[type];
		ip->i_dquot[type] = NODQUOT;
		dqrele(vp, dq);
		vrele(vp);
		MOUNT_VLIST_LOCK(mp);
		if (vp->v_mountf != nextvp || vp->v_mount != mp)
			goto again;
	}
	MOUNT_VLIST_UNLOCK(mp);
	UMPQ_LOCK_UNRECURSIVE(ump);
#if	MACH_ASSERT
	BM(VN_LOCK(qvp));
	ASSERT(qvp->v_usecount > 0);
	BM(VN_UNLOCK(vp));
#endif

	/*
	 * Eliminate all trace of this filesystem's
	 * quota type's quota vnode.
	 */
	dqflush(ump, qvp);
	VN_LOCK(qvp);
	qvp->v_flag &= ~VSYSTEM;
	VN_UNLOCK(qvp);
	vrele(qvp);

	/*
	 * No users remain of the ufs mount's
	 * quota fields.
	 */
	ump->um_quotas[type] = NULLVP;
	crfree(ump->um_cred[type]);
	ump->um_cred[type] = NOCRED;
	ump->um_qflags[type] &= ~QTF_CLOSING;

	for (type = 0; type < MAXQUOTAS; type++)
		if (ump->um_quotas[type] != NULLVP)
			break;
	if (type == MAXQUOTAS) {
		MOUNT_LOCK(mp);
		mp->m_flag &= ~M_QUOTA;
		MOUNT_UNLOCK(mp);
	}

out:
	UMPQ_WRITE_UNLOCK(ump);
	return (0);
}

/*
 * Q_GETQUOTA - return current values in a dqblk structure.
 */
getquota(mp, id, type, addr)
	struct mount *mp;
	u_int id;
	int type;
	caddr_t addr;
{
	struct dquot *dq;
	struct dqblk temp_dqb;
	struct ufsmount *ump;
	int error;

	ump = VFSTOUFS(mp);
	UMPQ_READ_LOCK(ump);
	if (error = dqget(NULLVP, id, VFSTOUFS(mp), type, &dq)) {
		UMPQ_READ_UNLOCK(ump);
		return (error);
	}
	temp_dqb = dq->dq_dqb;
	dqput(NULLVP, dq);
	UMPQ_READ_UNLOCK(ump);
	error = copyout((caddr_t)&temp_dqb, addr, sizeof (struct dqblk));
	return (error);
}

/*
 * Q_SETQUOTA - assign an entire dqblk structure.
 */
setquota(mp, id, type, addr)
	struct mount *mp;
	u_int id;
	int type;
	caddr_t addr;
{
	register struct dquot *dq;
	struct dquot *ndq;
	struct ufsmount *ump = VFSTOUFS(mp);
	struct dqblk newlim;
	int error, s;
	u_int sec;

	if (error = copyin(addr, (caddr_t)&newlim, sizeof (struct dqblk)))
		return (error);
	UMPQ_READ_LOCK(ump);
	if (error = dqget(NULLVP, id, ump, type, &ndq)) {
		UMPQ_READ_UNLOCK(ump);
		return (error);
	}
	dq = ndq;
	/*
	 * Copy all but the current values.
	 * Reset time limit if previously had no soft limit or were
	 * under it, but now have a soft limit and are over it.
	 */
	newlim.dqb_curblocks = dq->dq_curblocks;
	newlim.dqb_curinodes = dq->dq_curinodes;
	if (dq->dq_id != 0) {
		newlim.dqb_btime = dq->dq_btime;
		newlim.dqb_itime = dq->dq_itime;
	}
	s = splhigh();
	TIME_READ_LOCK();
	sec = time.tv_sec;
	TIME_READ_UNLOCK();
	splx(s);
	if (newlim.dqb_bsoftlimit &&
	    dq->dq_curblocks >= newlim.dqb_bsoftlimit &&
	    (dq->dq_bsoftlimit == 0 || dq->dq_curblocks < dq->dq_bsoftlimit))
		newlim.dqb_btime = sec + ump->um_btime[type];
	if (newlim.dqb_isoftlimit &&
	    dq->dq_curinodes >= newlim.dqb_isoftlimit &&
	    (dq->dq_isoftlimit == 0 || dq->dq_curinodes < dq->dq_isoftlimit))
		newlim.dqb_itime = sec + ump->um_itime[type];
	dq->dq_dqb = newlim;
	if (dq->dq_curblocks < dq->dq_bsoftlimit)
		dq->dq_flags &= ~DQ_BLKS;
	if (dq->dq_curinodes < dq->dq_isoftlimit)
		dq->dq_flags &= ~DQ_INODS;
	if (dq->dq_isoftlimit == 0 && dq->dq_bsoftlimit == 0 &&
	    dq->dq_ihardlimit == 0 && dq->dq_bhardlimit == 0)
		dq->dq_flags |= DQ_FAKE;
	else
		dq->dq_flags &= ~DQ_FAKE;
	dq->dq_flags |= DQ_MOD;
	dqput(NULLVP, dq);
	UMPQ_READ_UNLOCK(ump);
	return (0);
}

/*
 * Q_SETUSE - set current inode and block usage.
 */
setuse(mp, id, type, addr)
	struct mount *mp;
	u_int id;
	int type;
	caddr_t addr;
{
	register struct dquot *dq;
	struct ufsmount *ump = VFSTOUFS(mp);
	struct dquot *ndq;
	struct dqblk usage;
	int error, s;
	u_long sec;

	if (error = copyin(addr, (caddr_t)&usage, sizeof (struct dqblk)))
		return (error);
	UMPQ_READ_LOCK(ump);
	if (error = dqget(NULLVP, id, ump, type, &ndq)) {
		UMPQ_READ_UNLOCK(ump);
		return (error);
	}
	dq = ndq;
	/*
	 * Reset time limit if have a soft limit and were
	 * previously under it, but are now over it.
	 */
	s = splhigh();
	TIME_READ_LOCK();
	sec = time.tv_sec;
	TIME_READ_UNLOCK();
	splx(s);
	if (dq->dq_bsoftlimit && dq->dq_curblocks < dq->dq_bsoftlimit &&
	    usage.dqb_curblocks >= dq->dq_bsoftlimit)
		dq->dq_btime = sec + ump->um_btime[type];
	if (dq->dq_isoftlimit && dq->dq_curinodes < dq->dq_isoftlimit &&
	    usage.dqb_curinodes >= dq->dq_isoftlimit)
		dq->dq_itime = sec + ump->um_itime[type];
	dq->dq_curblocks = usage.dqb_curblocks;
	dq->dq_curinodes = usage.dqb_curinodes;
	if (dq->dq_curblocks < dq->dq_bsoftlimit)
		dq->dq_flags &= ~DQ_BLKS;
	if (dq->dq_curinodes < dq->dq_isoftlimit)
		dq->dq_flags &= ~DQ_INODS;
	dq->dq_flags |= DQ_MOD;
	dqput(NULLVP, dq);
	UMPQ_READ_UNLOCK(ump);
	return (0);
}

/*
 * Q_SYNC - sync quota files to disk.
 */
qsync(mp)
	struct mount *mp;
{
	struct ufsmount *ump = VFSTOUFS(mp);
	register struct vnode *vp, *nextvp;
	register struct inode *ip;
	register struct dquot *dq;
	register int i, error = 0;
	u_int mflag;

	UMPQ_READ_LOCK(ump);

	/*
	 * Only qsync a filesystem that has quotas on it.
	 * Furthermore, only allow one qsync at a time on
	 * that filesystem.
	 */
	BM(MOUNT_LOCK(mp));
	mflag = mp->m_flag;
	BM(MOUNT_UNLOCK(mp));
	if ((mflag & M_QUOTA) == 0)
		goto noquotas;
	UMPQ_SYNC_LOCK(ump);
	if (ump->um_qsync)
		goto qsyncing;
	ump->um_qsync++;
	UMPQ_SYNC_UNLOCK(ump);

	/*
	 * Search vnodes associated with this mount point,
	 * synchronizing any modified dquot structures.
	 */
	MOUNT_VLIST_LOCK(mp);
again:
	for (vp = mp->m_mounth; vp; vp = nextvp) {
		nextvp = vp->v_mountf;
		VN_LOCK(vp);
		error = vget_nowait(vp);
		VN_UNLOCK(vp);
		if (error)
			continue;
		MOUNT_VLIST_UNLOCK(mp);
		ip = VTOI(vp);
		for (i = 0; i < MAXQUOTAS; i++) {
			if ((dq = ip->i_dquot[i]) != NODQUOT) {
				DQ_LOCK(dq);
				if (dq->dq_flags & DQ_MOD)
					dqsync(vp, dq);
				DQ_UNLOCK(dq);
			}
		}
		vrele(vp);
		MOUNT_VLIST_LOCK(mp);
		if (vp->v_mountf != nextvp || vp->v_mount != mp)
			goto again;
	}
	MOUNT_VLIST_UNLOCK(mp);
	UMPQ_SYNC_LOCK(ump);
	ump->um_qsync = 0;
qsyncing:
	UMPQ_SYNC_UNLOCK(ump);
noquotas:
	UMPQ_READ_UNLOCK(ump);
	return (0);
}

/*
 * Code pertaining to management of the in-core dquot data structures.
 */

/*
 * Dquot cache - hash chain headers.
 */
struct	dqhead {
	union {
		struct	dqhead	*dqu_head[2];
		struct	dquot	*dqu_chain[2];
	} dq_u;
	u_int	dqh_timestamp;
	udecl_simple_lock_data(,dqh_lock)
};

#define	dqh_head		dq_u.dqu_head
#define	dqh_chain		dq_u.dqu_chain
#define	dqh_forw		dqh_chain[0]
#define	dqh_back		dqh_chain[1]
#define	DQHEAD_NULL		((struct dqhead *) NULL)
#define	DQH_LOCK(dhp)		usimple_lock(&(dhp)->dqh_lock)
#define	DQH_UNLOCK(dhp)		usimple_unlock(&(dhp)->dqh_lock)
#define	DQH_LOCK_TRY(dhp)	usimple_lock_try(&(dhp)->dqh_lock)
#define	DQH_LOCK_INIT(dhp)	usimple_lock_init(&(dhp)->dqh_lock)

struct dqhead			*dqhashtbl;
int				dqhash;

/*
 * Dquot free list.
 */
#define	DQUOTINC	5	/* minimum free dquots desired */
struct dquot *dqfreel, **dqback = &dqfreel;
int numdquot, desireddquot = DQUOTINC;

#if	MACH
#define	DQ_ALLOCATE(dq)		(dq) = (struct dquot *) kalloc(sizeof(struct dquot))
#define	DQ_DEALLOCATE(dq)	kfree((dq), sizeof(struct dquot))
#define	MAXDQUOT		(MAXQUOTAS * nvnode)
#else
#define	DQ_ALLOCATE(dq)		(dq) = (struct dquot *) malloc(sizeof *(dq), \
							       M_DQUOT, \
							       M_WAITOK)
#define	DQ_FREE(dq)		free((caddr_t)(dq), M_DQUOT)
#define	MAXDQUOT		(MAXQUOTAS * desiredvnodes)
#endif

udecl_simple_lock_data(,dqfree_lock)
#define	DQFREE_LOCK()		usimple_lock(&dqfree_lock)
#define	DQFREE_UNLOCK()		usimple_unlock(&dqfree_lock)
#define	DQFREE_LOCK_TRY()	usimple_lock_try(&dqfree_lock)
#define	DQFREE_LOCK_INIT()	usimple_lock_init(&dqfree_lock)
#if	UNIX_LOCKS
#define	DQFREE_HOLDER()		SLOCK_HOLDER(&dqfree_lock)
#else
#define	DQFREE_HOLDER()
#endif

/*
 * Initialize the quota system.
 */
dqinit()
{
	register struct dqhead	*dhp;
#if	MACH
	register vm_size_t	dquotsize;
	register vm_size_t	dqheadsize;
	register vm_size_t	dqhashsize;
#else
	register int		dqhashsize;
#endif

#if	MACH
	dquotsize = (vm_size_t) sizeof(struct dquot);
#define	MINDQHSZ	(MINNCHSZ*4)
	dqheadsize = (vm_size_t) sizeof(struct dqhead);
	dqhashsize = MINDQHSZ * dqheadsize;
	dqhashtbl = (struct dqhead *)kalloc(dqhashsize);
#else
	dqhashsize = roundup((desiredvnodes + 1) * sizeof *dhp / 2,
		NBPG * CLSIZE);
	dqhashtbl = (union dqhead *)malloc(dqhashsize, M_DQUOT, M_WAITOK);
#endif
	for (dqhash = 1; dqhash <= dqhashsize / sizeof *dhp; dqhash <<= 1)
		/* void */;
	dqhash = (dqhash >> 1) - 1;
	for (dhp = &dqhashtbl[dqhash]; dhp >= dqhashtbl; dhp--) {
		dhp->dqh_head[0] = dhp;
		dhp->dqh_head[1] = dhp;
		dhp->dqh_timestamp = 0;
		DQH_LOCK_INIT(dhp);
	}
	DQFREE_LOCK_INIT();
	VSTATS_LOCK_INIT(&quota_stats_lock);
}

/*
 * Obtain a dquot structure for the specified identifier and quota file
 * reading the information from the file if necessary.
 *
 * Assumes caller holds ufsmount quota lock for reading or writing,
 * as appropriate.
 */
dqget(vp, id, ump, type, dqp)
	struct vnode *vp;
	u_int id;
	register struct ufsmount *ump;
	register int type;
	struct dquot **dqp;
{
	register struct dquot *dq, *dq2, *dp;
	register struct dqhead *dh;
	register struct vnode *dqvp;
	struct iovec aiov;
	struct uio auio;
	int error, s;
	u_int otimestamp, sec;

	LASSERT(UMPQ_READ_HOLDER(ump) || UMPQ_WRITE_HOLDER(ump));

	dqvp = ump->um_quotas[type];
	if (dqvp == NULLVP || (ump->um_qflags[type] & QTF_CLOSING) ||
	    dqvp == vp) {
		*dqp = NODQUOT;
		return (EINVAL);
	}
	/*
	 * Check the cache first.
	 */
	dh = &dqhashtbl[((((int)(dqvp)) >> 8) + id) & dqhash];
again:
	DQH_LOCK(dh);
	for (dq = dh->dqh_forw; dq != (struct dquot *)dh; dq = dq->dq_forw) {
		/*
		 * dq_id is invariant under hash chain lock
		 * and having limited our search to dquots
		 * on ump, dq->dq_ump->um_quotas[] is invariant
		 * under ufsmount quota lock.
		 */
		if (dq->dq_id != id ||
		    dq->dq_ump != ump ||
		    dq->dq_ump->um_quotas[dq->dq_type] != dqvp)
			continue;
		/*
		 * Cache hit with no references.  Take
		 * the structure off the free list.
		 */
		if (dq->dq_cnt == 0) {
			DQFREE_LOCK();
			dp = dq->dq_freef;
			if (dp != NODQUOT)
				dp->dq_freeb = dq->dq_freeb;
			else
				dqback = dq->dq_freeb;
			*dq->dq_freeb = dp;
			DQFREE_UNLOCK();
		}
		dq->dq_cnt++;
		DQH_UNLOCK(dh);
		DQ_LOCK(dq);
		if (dq->dq_flags & DQ_READERROR) {
			dqput(vp, dq);
			*dqp = NODQUOT;
			return (EIO);
		}
		*dqp = dq;
		return (0);
	}
	otimestamp = dh->dqh_timestamp;
	DQH_UNLOCK(dh);
	/*
	 * Not in cache, allocate a new one.
	 */
	DQFREE_LOCK();
loop:
	if (numdquot < desireddquot) {
		numdquot++;
		DQFREE_UNLOCK();
		DQ_ALLOCATE(dq);
		bzero((char *)dq, sizeof *dq);
		DQ_LOCK_INIT(dq);
	} else 
		if (dqfreel == NODQUOT && numdquot < MAXDQUOT) {
			desireddquot += DQUOTINC;
			desireddquot = MIN(desireddquot, MAXDQUOT);
			goto loop;
		} else {
		for (dq = dqfreel; dq != NODQUOT; dq = dq->dq_freef) {
			if (!DQH_LOCK_TRY(dq->dq_hash))
				continue;
			ASSERT(!dq->dq_cnt && !(dq->dq_flags & DQ_MOD));
			dp = dq->dq_freef;
			if (dp != NODQUOT)
				dp->dq_freeb = dq->dq_freeb;
			else
				dqback = dq->dq_freeb;
			*dq->dq_freeb = dp;
			DQFREE_UNLOCK();
			remque(dq);
			DQH_UNLOCK(dq->dq_hash);
			dq->dq_freef = NULL;
			dq->dq_freeb = NULL;
			break;
		}
		if (dq == NODQUOT) {
			if (numdquot < MAXDQUOT) {
				desireddquot += DQUOTINC;
				desireddquot = MIN(desireddquot, MAXDQUOT);
				goto loop;
			}
			DQFREE_UNLOCK();
			tablefull("dquot");
			*dqp = NODQUOT;
			return (EUSERS);
		}
	}
	LASSERT(!DQFREE_HOLDER());

	/*
	 * Initialize the contents of the dquot structure.
	 * Nobody else knows about this dquot so it's legal
	 * to clobber all the interesting values and toy with
	 * the cnt field.
	 */
	if (!DQ_LOCK_TRY(dq))
		panic("locked dquot on freelist");
	dq->dq_flags = 0;
	dq->dq_id = id;
	dq->dq_ump = ump;
	dq->dq_type = type;
	dq->dq_cnt++;
	dq->dq_hash = dh;

	/*
	 * Insert new dquot into hash chain.  Two observations:
	 *	1.  Must re-check the hash chain to guarantee
	 *	that no similar dquot has already been inserted.
	 *	2.  The contents of the dqb are protected by the
	 *	dquot lock, which we hold.
	 */
	DQH_LOCK(dh);
	if (otimestamp != dh->dqh_timestamp) {
		for (dq2 = dh->dqh_forw; dq2 != (struct dquot *)dh;
		     dq2 = dq2->dq_forw) {
			if (dq2->dq_id != id ||
			    dq2->dq_ump != ump ||
			    dq2->dq_ump->um_quotas[dq2->dq_type] != dqvp)
				continue;
			/*
			 * Collision!  Drop the dquot we just built
			 * and start all over again.
			 */
			DQH_UNLOCK(dh);
			dqput(vp, dq);
			QSTATS(dq_collisions++);
			goto again;
		}
	}
	insque(dq, dh);
	dh->dqh_timestamp++;
	DQH_UNLOCK(dh);

	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	aiov.iov_base = (caddr_t)&dq->dq_dqb;
	aiov.iov_len = sizeof (struct dqblk);
	auio.uio_resid = sizeof (struct dqblk);
	auio.uio_offset = (off_t)(id * sizeof (struct dqblk));
	auio.uio_segflg = UIO_SYSSPACE;
	auio.uio_rw = UIO_READ;

	ASSERT(vp != dqvp);
	VOP_READ(dqvp, &auio, 0, ump->um_cred[type], error);
	if (auio.uio_resid == sizeof(struct dqblk) && error == 0)
		bzero((caddr_t)&dq->dq_dqb, sizeof(struct dqblk));

	/*
	 * I/O error in reading quota file, release
	 * quota structure and reflect problem to caller.
	 */
	if (error) {
		DQH_LOCK(dh);
		remque(dq);
		DQH_UNLOCK(dh);
		dq->dq_flags |= DQ_READERROR;
		dq->dq_forw = dq;	/* on a private, unfindable hash */
		dq->dq_back = dq;
		dq->dq_hash = &dqhashtbl[0];
		dqput(vp, dq);
		*dqp = NODQUOT;
		return (error);
	}

	/*
	 * Check for no limit to enforce.
	 * Initialize time values if necessary.
	 */
	if (dq->dq_isoftlimit == 0 && dq->dq_bsoftlimit == 0 &&
	    dq->dq_ihardlimit == 0 && dq->dq_bhardlimit == 0)
		dq->dq_flags |= DQ_FAKE;
	if (dq->dq_id != 0) {
		s = splhigh();
		TIME_READ_LOCK();
		sec = time.tv_sec;
		TIME_READ_UNLOCK();
		splx(s);
		if (dq->dq_btime == 0)
			dq->dq_btime = sec + ump->um_btime[type];
		if (dq->dq_itime == 0)
			dq->dq_itime = sec + ump->um_itime[type];
	}

	LASSERT(DQ_HOLDER(dq));
	*dqp = dq;
	return (0);
}

/*
 * Release a reference to an unlocked dquot.
 */
void
dqrele(vp, dq)
	struct vnode *vp;
	register struct dquot *dq;
{
	if (dq == NODQUOT)
		return;

	DQ_LOCK(dq);
	dqput(vp, dq);
}

/*
 * Release a reference to a locked dquot.
 */
void
dqput(vp, dq)
	struct vnode *vp;
	register struct dquot *dq;
{
	register struct dqhead *dh;
	register short cnt;

	if (dq == NODQUOT)
		return;
	LASSERT(DQ_HOLDER(dq));
	ASSERT(dq->dq_hash != DQHEAD_NULL);

	dh = dq->dq_hash;
	DQH_LOCK(dh);
	if (dq->dq_cnt > 1) {
		dq->dq_cnt--;
		DQH_UNLOCK(dh);
		DQ_UNLOCK(dq);
		return;
	}
	DQH_UNLOCK(dh);

	if (dq->dq_flags & DQ_MOD)
		(void) dqsync(vp, dq);
	DQ_UNLOCK(dq);

	DQH_LOCK(dh);
	if (--dq->dq_cnt > 0) {
		DQH_UNLOCK(dh);
		return;
	}

	DQFREE_LOCK();
	if (dqfreel != NODQUOT) {
		*dqback = dq;
		dq->dq_freeb = dqback;
	} else {
		dqfreel = dq;
		dq->dq_freeb = &dqfreel;
	}
	dq->dq_freef = NODQUOT;
	dqback = &dq->dq_freef;
	DQFREE_UNLOCK();
	DQH_UNLOCK(dh);
}

/*
 * Update the disk quota in the quota file.
 */
dqsync(vp, dq)
	struct vnode *vp;
	register struct dquot *dq;
{
	struct vnode *dqvp;
	struct iovec aiov;
	struct uio auio;
	int error;

	if (dq == NODQUOT)
		panic("dqsync: dquot");
	LASSERT(DQ_HOLDER(dq));
	if ((dq->dq_flags & DQ_MOD) == 0)
		return (0);
	if ((dqvp = dq->dq_ump->um_quotas[dq->dq_type]) == NULLVP)
		panic("dqsync: file");
	ASSERT(vp != dqvp);

	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	aiov.iov_base = (caddr_t)&dq->dq_dqb;
	aiov.iov_len = sizeof (struct dqblk);
	auio.uio_resid = sizeof (struct dqblk);
	auio.uio_offset = (off_t)(dq->dq_id * sizeof (struct dqblk));
	auio.uio_segflg = UIO_SYSSPACE;
	auio.uio_rw = UIO_WRITE;

	VOP_WRITE(dqvp, &auio, 0, dq->dq_ump->um_cred[dq->dq_type], error);

	if (auio.uio_resid && error == 0)
		error = EIO;
	dq->dq_flags &= ~DQ_MOD;
	return (error);
}

/*
 * Flush all entries from the cache for a particular vnode.
 */
dqflush(ump, vp)
	register struct ufsmount *ump;
	register struct vnode *vp;
{
	register struct dqhead *dh;
	register struct dquot *dq, *nextdq;

	LASSERT(UMPQ_WRITE_HOLDER(ump));
	/*
	 * Move all dquot's that used to refer to this quota
	 * file off their hash chains (they will eventually
	 * fall off the head of the free list and be re-used).
	 *
	 * We deliberately refrain from altering dq_hash because
	 * there's no convenient way to do so without locking
	 * the free list at the same time.  dqget therefore may
	 * take a meaningless (but harmless) hash chain lock
	 * when reclaiming the dquot.
	 */
	for (dh = &dqhashtbl[dqhash]; dh >= dqhashtbl; dh--) {
		DQH_LOCK(dh);
		for (dq = dh->dqh_forw; dq != (struct dquot *)dh;
		     dq = nextdq) {
			nextdq = dq->dq_forw;
			if (dq->dq_ump != ump ||
			    dq->dq_ump->um_quotas[dq->dq_type] != vp)
				continue;
			if (dq->dq_cnt)
				panic("dqflush: stray dquot");
			remque(dq);
			dq->dq_forw = dq;
			dq->dq_back = dq;
			dq->dq_ump = (struct ufsmount *)0;
		}
		DQH_UNLOCK(dh);
	}
}
