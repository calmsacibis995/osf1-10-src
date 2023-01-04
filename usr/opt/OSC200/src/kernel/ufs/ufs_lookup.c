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
static char	*sccsid = "@(#)$RCSfile: ufs_lookup.c,v $ $Revision: 4.2.22.3 $ (DEC) $Date: 1993/08/26 12:57:31 $";
#endif 
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.2
 */
/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)ufs_lookup.c	7.19 (Berkeley) 11/30/89
 *
 * Revision History
 *
 * 1 Jul 91 -- prs
 *	Added cache_purge() calls to direnter() to fix race problems
 *	between creates and lookups.
 *
 * 10 Jun 91 -- prs
 *	Merged in OSF 1.0.1 patch tape.
 *
 */

#include <sys/secdefines.h>
#if	SEC_BASE
#include <sys/security.h>
#endif

#include <sys/param.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <ufs/fs.h>

#include <dcedfs.h>

#if defined(DCEDFS) && DCEDFS
enum vnuio_rw { VNUIO_READ, VNUIO_WRITE, VNUIO_READNESTED, VNUIO_WRITENESTED };
#endif /* DCEDFS */

extern struct	nchstats nchstats;
int	dirchk = 0;

/*
 * Convert a component of a pathname into a pointer to a locked inode.
 * This is a very central and rather complicated routine.
 * If the file system is not maintained in a strict tree hierarchy,
 * this can result in a deadlock situation (see comments in code below).
 *
 * The flag argument is LOOKUP, CREATE, RENAME, or DELETE depending on
 * whether the name is to be looked up, created, renamed, or deleted.
 * When CREATE, RENAME, or DELETE is specified, information usable in
 * creating, renaming, or deleting a directory entry may be calculated.
 * NOTE:  RENAME must be forced to do a scandir, and CREATE must 
 * do the scandir if there is a negative cache hit (see cache_lookup).
 *
 * Overall outline of ufs_lookup:
 *
 *	check accessibility of directory
 *	look for name in cache, if found, then if at end of path
 *	  and deleting or creating, drop it, else return name
 *	search for name in directory, to found or notfound
 * notfound:
 *	if creating, return locked directory, leaving info on available slots
 *	else return error
 * found:
 *	if at end of path and deleting, return information to allow delete
 *	if not at end, add name to cache; if at end and neither creating
 *	  nor deleting, add name to cache
 *
 * NOTE: (LOOKUP) currently returns the parent inode unlocked.
 */
ufs_lookup(vp, ndp)
	struct vnode *vp;
	register struct nameidata *ndp;
{
	register struct vnode *vdp;	/* vnode copy of dp */
	register struct inode *dp;	/* the directory we are searching */
	struct inode *pdp;		/* saved dp during symlink work */
	int error;

	ndp->ni_dvp = vp;
	ndp->ni_vp = NULL;
	dp = VTOI(vp);
	ASSERT(vp->v_usecount);
	/*
	 * Check accessiblity of directory.
	 *
	 * If we are processing a '..' from namei, and this '..' spans
	 * a mount point, we shouldn't check mode bits for exec access
	 * if NOCROSSMOUNT is false.
	 */
	if ((dp->i_mode&IFMT) != IFDIR)
		return (ENOTDIR);
	if (ndp->ni_isdotdot && (vp->v_mountedhere != NULLMOUNT) &&
	    !(ndp->ni_nameiop & NOCROSSMOUNT)) {
		/* no access checking */
	} else {	
		if (error = iaccess(dp, IEXEC, ndp->ni_cred))
			return (error);
	}
loop:
	/*
	 * We now have a segment name to search for, and a directory to search.
	 *
	 * Before tediously performing a linear scan of the directory,
	 * check the name cache to see if the directory/name pair
	 * we are looking for is known already.
	 */
	if (error = cache_lookup(ndp)) {
		int vpid;	/* capability number of vnode */

		if (error == ENOENT)
			return (error);
#if	0
		if (vp == ndp->ni_rdir && ndp->ni_isdotdot)
			panic("ufs_lookup: .. through root");
#endif
		/*
		 * Get the next vnode in the path.
		 * See comment below starting `Step through' for
		 * an explaination of the locking protocol.
		 */
		pdp = dp;
		dp = VTOI(ndp->ni_vp);
		vdp = ndp->ni_vp;
#if	UNIX_LOCKS
		vpid = ndp->ni_vpid;
#else
		vpid = vdp->v_id;
#endif
		if (pdp == dp) {
			ASSERT(vdp->v_usecount);
			VREF(vdp);
			error = 0;
		} else {
			VN_LOCK(vdp);
			error = vget_nowait(vdp);
			VN_UNLOCK(vdp);
		}
		if (!error) {
			/*
			 * Check that the capability number did not change
			 * while we were waiting for the lock.
			 * For UNIX_LOCKS, we saved the capability number
			 * within cache_lookup (ndp->ni_vpid) to compare it
			 * post vget();  it's possible that vdp->v_tag!=VT_UFS
			 * which would make use of dp invalid.
			 */
			if (vpid != vdp->v_id) {
				vrele(vdp);	/* we lost race to the vnode */
			} else {
				IN_LOCK(dp);
				/*
				 * If the inode is being inactivated, we must
				 * wait until the inactivation completes.
				 * Then we must try to find it in the cache.
				 * again. The inode may have been reclaimed.
				 */
				if (dp->i_flag & INACTIVATING) {
					dp->i_flag |= INACTWAIT;
					assert_wait((vm_offset_t)&dp->i_flag, FALSE);
					IN_UNLOCK(dp);
					thread_block();
					iput(dp);
					dp = pdp;	/* reset to parent */
					ndp->ni_vp = NULLVP;
					goto loop;
				}
				IN_UNLOCK(dp);
				return 0;
			}
		}
		ndp->ni_vp = NULL;
	}
	return(scandir(vp, ndp));
}

scandir(vp, ndp)
struct vnode *vp;
struct nameidata *ndp;
{
	register struct dirent *ep;	/* the current directory entry */
	register struct vnode *vdp;	/* vnode copy of dp */
	register struct inode *dp;	/* the directory we are searching */
	register struct fs *fs;		/* file system that directory is in */
	int entryoffsetinblock;		/* offset of ep in bp's buffer */
	enum {NONE, COMPACT, FOUND} slotstatus;
	int slotoffset;			/* offset of area with free space */
	int slotsize;			/* size of area at slotoffset */
	int slotfreespace;		/* amount of space free in slot */
	int slotneeded;			/* size of the entry we're seeking */
	int numdirpasses;		/* strategy for directory search */
	int endsearch;			/* offset to end directory search */
	int prevoff;			/* ndp->ni_offset of previous entry */
	off_t enduseful;		/* pointer past last used dir slot */
	int flag;			/* LOOKUP, CREATE, RENAME, or DELETE */
	int wantparent;			/* 1 => wantparent flag */
	struct buf *bp;			/* a buffer of directory entries */
	struct inode *tdp;		/* returned by iget */
	int error;
	int initial_offset;
	u_int	dpsize;
	enum vtype type;
	int trys = 0;

rescan:
	/*
	 * Initialize locals
	 */
	slotoffset = -1;
	bp = 0;
	dp = VTOI(vp);
	fs = dp->i_fs;
	flag = ndp->ni_nameiop & OPFLAG;
	wantparent = ndp->ni_nameiop & (WANTPARENT);

#if SEC_MAC
	/* XXX need locking here */
	/*
	 * Deny an attempt to make a new entry in a multilevel directory
	 * unless it is an attempt to create a mld subdirectory.
	 */
	if ((flag == CREATE || flag == RENAME) && *ndp->ni_next == 0)
		if ((dp->i_type_flags & SEC_I_MLD) &&
		    (ndp->ni_nameiop & MLDCREATE) == 0)
			return EACCES;
#endif /* SEC_MAC */

	/*
	 * Suppress search for slots unless creating
	 * file and at end of pathname, in which case
	 * we watch for a place to put the new file in
	 * case it doesn't already exist.
	 */
	slotstatus = FOUND;
	if ((flag == CREATE || flag == RENAME) && *ndp->ni_next == 0) {
		slotstatus = NONE;
		slotfreespace = 0;
		slotneeded = DIRSIZ(&ndp->ni_dent);
	}

	/*
	 * If there is cached information on a previous search of
	 * this directory, pick up where we last left off.
	 * We cache only lookups as these are the most common
	 * and have the greatest payoff. Caching CREATE has little
	 * benefit as it usually must search the entire directory
	 * to determine that the entry does not exist. Caching the
	 * location of the last DELETE or RENAME has not reduced
	 * profiling time and hence has been removed in the interest
	 * of simplicity.
	 */
	IN_READ_LOCK(dp);
	IN_LOCK(dp);
	dpsize = dp->i_size;
	initial_offset = dp->i_diroff;
	if (flag != LOOKUP || initial_offset == 0 || initial_offset > dp->i_size) {
		IN_UNLOCK(dp);
		ndp->ni_offset = 0;
		numdirpasses = 1;
	} else {
		IN_UNLOCK(dp);
		ndp->ni_offset = initial_offset;
		entryoffsetinblock = blkoff(fs, ndp->ni_offset);
		if (entryoffsetinblock != 0) {
			error = blkatoff(dp, ndp->ni_offset, (char **)0, &bp);
			if (error) {
				IN_READ_UNLOCK(dp);
				return (error);
			}
			if (dirchk && dirbadblock(bp))
				dirbad(dp, ndp->ni_offset,
				       "mangled entry in block");
		}
		numdirpasses = 2;
		NC_STATS(nchstats.ncs_2passes++);
	}
	endsearch = roundup(dpsize, DIRBLKSIZ);
	enduseful = 0;

	/*
	 * Record timestamp while holding the read lock on the directory.
	 */
	ndp->ni_dirstamp = dp->i_dirstamp;
searchloop:
	while (ndp->ni_offset < endsearch) {
		/*
		 * If offset is on a block boundary,
		 * read the next directory block.
		 * Release previous if it exists.
		 */
		if (blkoff(fs, ndp->ni_offset) == 0) {
			if (bp != NULL)
				brelse(bp);
			error = blkatoff(dp, ndp->ni_offset, (char **)0, &bp);
			if (error) {
				IN_READ_UNLOCK(dp);
				return (error);
			}
			entryoffsetinblock = 0;
			if (dirchk && dirbadblock(bp))
				dirbad(dp, ndp->ni_offset,
				       "mangled entry in block");
		}
		/*
		 * If still looking for a slot, and at a DIRBLKSIZE
		 * boundary, have to start looking for free space again.
		 */
		if (slotstatus == NONE &&
		    (entryoffsetinblock & (DIRBLKSIZ - 1)) == 0) {
			slotoffset = -1;
			slotfreespace = 0;
		}
		/*
		 * Get pointer to next entry.
		 * Full validation checks are slow, so we only check
		 * enough to insure forward progress through the
		 * directory. Complete checks can be run by patching
		 * "dirchk" to be true.
		 */
		ASSERT(bp != NULL);
		ep = (struct dirent *)(bp->b_un.b_addr + entryoffsetinblock);
		if (ep->d_reclen == 0 ||
		    dirchk && dirbadentry(ep, entryoffsetinblock))
			dirbad(dp, ndp->ni_offset, "mangled entry");

		/*
		 * If an appropriate sized slot has not yet been found,
		 * check to see if one is available. Also accumulate space
		 * in the current block so that we can determine if
		 * compaction is viable.
		 */
		if (slotstatus != FOUND) {
			int size = ep->d_reclen;

			if (ep->d_fileno != 0)
				size -= DIRSIZ(ep);
			if (size > 0) {
				if (size >= slotneeded) {
					slotstatus = FOUND;
					slotoffset = ndp->ni_offset;
					slotsize = ep->d_reclen;
				} else if (slotstatus == NONE) {
					slotfreespace += size;
					if (slotoffset == -1)
						slotoffset = ndp->ni_offset;
					if (slotfreespace >= slotneeded) {
						slotstatus = COMPACT;
						slotsize = ndp->ni_offset +
						      ep->d_reclen - slotoffset;
					}
				}
			}
		}

		/*
		 * Check for a name match.
		 */
		if (ep->d_fileno) {
			if (ep->d_namlen == ndp->ni_dent.d_namlen &&
			    !bcmp(ndp->ni_ptr, ep->d_name,
				(unsigned)ep->d_namlen)) {
				/*
				 * Save directory entry's inode number and
				 * reclen in ndp->ni_dent, and release
				 * directory buffer.
				 */
				ndp->ni_dent.d_fileno = ep->d_fileno;
				ndp->ni_dent.d_reclen = ep->d_reclen;
				brelse(bp);
				goto found;
			}
		}
		prevoff = ndp->ni_offset;
		ndp->ni_offset += ep->d_reclen;
		entryoffsetinblock += ep->d_reclen;
		if (ep->d_fileno)
			enduseful = ndp->ni_offset;
	}
/* notfound: */
	/*
	 * If we started in the middle of the directory and failed
	 * to find our target, we must check the beginning as well.
	 */
	if (numdirpasses == 2) {
		numdirpasses--;
		ndp->ni_offset = 0;
		endsearch = initial_offset;
		goto searchloop;
	}
	if (bp != NULL)
		brelse(bp);
	/*
	 * If creating, and at end of pathname and current
	 * directory has not been removed, then can consider
	 * allowing file to be created.
	 */
	IN_LOCK(dp);
	if ((flag == CREATE || flag == RENAME) &&
	    *ndp->ni_next == 0 && dp->i_nlink != 0) {

		IN_UNLOCK(dp);
#if SEC_MAC
		/*
		 * When creating a new file (but not when renaming)
		 * check SP_CREATEACC access to parent directory.
	 	 * This is equivalent to checking write access with
		 * an implicit increasing tree check by the MAC policy.
		 * Suppress the check when creating a mld subdirectory
		 * since this will usually involve a write down and we
		 * don't want the operation to fail just because the
		 * process doesn't have the necessary privilege to write
		 * down. The increasing tree check will be performed
		 * when the subdirectory is actually created.
		 * When renaming we do a normal write check on the
		 * parent and rely on the rename code to perform the
		 * increasing tree check.
		 */
		if (security_is_on) {
		if (flag == CREATE) {
			if ((ndp->ni_nameiop & MLDCREATE) == 0 &&
			    (error = iaccess(dp, SP_CREATEACC, ndp->ni_cred))) {
				IN_READ_UNLOCK(dp);
				return (error);
			}
		}
		} else
#endif /* SEC_MAC */
		/*
		 * Access for write is interpreted as allowing
		 * creation of files in the directory.
		 */
		if (error = iaccess(dp, IWRITE, ndp->ni_cred)) {
			IN_READ_UNLOCK(dp);
			return (error);
		}
		/*
		 * Return an indication of where the new directory
		 * entry should be put.  If we didn't find a slot,
		 * then set ndp->ni_count to 0 indicating that the new
		 * slot belongs at the end of the directory. If we found
		 * a slot, then the new entry can be put in the range
		 * [ndp->ni_offset .. ndp->ni_offset + ndp->ni_count)
		 */
		if (slotstatus == NONE) {
			ndp->ni_offset = roundup(dpsize, DIRBLKSIZ);
			ndp->ni_count = 0;
			enduseful = ndp->ni_offset;
		} else {
			ndp->ni_offset = slotoffset;
			ndp->ni_count = slotsize;
			if (enduseful < slotoffset + slotsize)
				enduseful = slotoffset + slotsize;
		}
		ndp->ni_endoff = roundup(enduseful, DIRBLKSIZ);
		IN_LOCK(dp);
		dp->i_flag |= IUPD|ICHG;
		/*
		 * We return ni_vp == NULL to indicate that the entry
		 * does not currently exist; we leave a pointer to
		 * the directory inode in ndp->ni_dvp.
		 *
		 */
	}
	IN_UNLOCK(dp);
	/*
	 * Insert name into cache (as non-existent) if appropriate.
	 */
#if SEC_MAC
	/*
	 * But if we are traversing a multilevel directory, we don't
	 * want a cache entry saying the mld subdirectory doesn't
	 * exist, since it is about to be created.
	 */
	if ((ndp->ni_nameiop & MLDCREATE) == 0)
#endif /* SEC_MAC */
	/*
	 * Don't cache negative hits for CREATE.  The cache will get
	 * confused.
	 */
	if (ndp->ni_makeentry && (flag != CREATE) && (flag != RENAME))
		cache_enter(ndp);
	IN_READ_UNLOCK(dp);
	return (ENOENT);

found:
	if (numdirpasses == 2)
		NC_STATS(nchstats.ncs_pass2++);
	/*
	 * Check that directory length properly reflects presence
	 * of this entry.
	 */
	if (entryoffsetinblock + DIRSIZ(ep) > dpsize)
		dirbad(dp, ndp->ni_offset, "i_size too small");

	/*
	 * Found component in pathname.
	 * If the final component of path name, save information
	 * in the cache as to where the entry was found.
	 */
	if (*ndp->ni_next == '\0' && flag == LOOKUP) {
		IN_LOCK(dp);
		dp->i_diroff = ndp->ni_offset &~ (DIRBLKSIZ - 1);
		IN_UNLOCK(dp);
	}

	/*
	 * If deleting, and at end of pathname, return
	 * parameters which can be used to remove file.
	 * If the wantparent flag isn't set, we return only
	 * the directory (in ndp->ni_dvp), otherwise we go
	 * on and lock the inode, being careful with ".".
	 */
	IN_READ_UNLOCK(dp);
	if (flag == DELETE && *ndp->ni_next == 0) {
		/*
		 * Write access to directory required to delete files.
		 */
		if (error = iaccess(dp, IWRITE, ndp->ni_cred))
			return (error);
		/*
		 * Return pointer to current entry in ndp->ni_offset,
		 * and distance past previous entry (if there
		 * is a previous entry in this block) in ndp->ni_count.
		 * Save directory inode pointer in ndp->ni_dvp for dirremove().
		 */
		if ((ndp->ni_offset&(DIRBLKSIZ-1)) == 0)
			ndp->ni_count = 0;
		else
			ndp->ni_count = ndp->ni_offset - prevoff;
		vdp = ITOV(dp);
		if (dp->i_number == ndp->ni_dent.d_fileno) {
			VREF(vdp);
		} else {
			if (error = iget(dp, ndp->ni_dent.d_fileno, &tdp, 0))
				return (error);
			vdp = ITOV(tdp);
			/*
			 * There's a race here.  It's possible that the
			 * inode has been changed between the name match and
			 * the iget(), after the directory lock was released.
			 * This is indicated by a vnode of type VNON
			 * returned from iget().
			 * We put the inode back and attempt to rescan.  The
			 * rescan is simpler than returning ENOENT because of
			 * the state that is created above, and assumed by
			 * the callers of scandir().
			 * An alternative fix would be to close the hole
			 * by keeping the inode locked through the
			 * iget().  This is not done in the interest of
			 * parallelism.
			 */
			 BM(VN_LOCK(vdp));
			 type = vdp->v_type;
			 BM(VN_UNLOCK(vdp));
			 if (type == VNON) {
				vrele(vdp);
				if (++trys < 5)
					goto rescan;
				ndp->ni_vp = NULLVP;
				return(ENOENT);
			}
			/*
			 * If directory is "sticky", then user must own
			 * the directory, or the file in it, else he
			 * may not delete it (unless he's root). This
			 * implements append-only directories.
			 */
			BM(IN_LOCK(dp));
#if     SEC_BASE
		    if(check_privileges)
                        if ((dp->i_mode & ISVTX) &&
                            ndp->ni_cred->cr_uid != dp->i_uid) {
				BM(IN_UNLOCK(dp));
				BM(IN_LOCK(tdp));
                                if (tdp->i_uid != ndp->ni_cred->cr_uid) {
                                        BM(IN_UNLOCK(tdp));
                                        if (!privileged(SEC_OWNER, 0)) {
                                                iput(tdp);
                                                return (EPERM);
                                        }
                                } else
                                        BM(IN_UNLOCK(tdp));
			} else
				BM(IN_UNLOCK(dp));
		   else
#endif
			if ((dp->i_mode & ISVTX) &&
			    ndp->ni_cred->cr_uid != 0 &&
			    ndp->ni_cred->cr_uid != dp->i_uid) {
				BM(IN_UNLOCK(dp));
				BM(IN_LOCK(tdp));
			    	if (tdp->i_uid != ndp->ni_cred->cr_uid) {
					BM(IN_UNLOCK(tdp));
					iput(tdp);
					return (EPERM);
				} else
					BM(IN_UNLOCK(tdp));
			} else
				BM(IN_UNLOCK(dp));
#if	SEC_ARCH
			/*
			 * Check delete access to the file.
			 */
			if (security_is_on) {
			if (error = iaccess(tdp, SP_DELETEACC, ndp->ni_cred)) {
				iput(tdp);
				return error;
			}
			}
#endif	/* SEC_ARCH */
		}
		ndp->ni_vp = vdp;
		return (0);
	}

	/*
	 * If rewriting (RENAME), return the inode and the
	 * information required to rewrite the present directory
	 * Must get inode of directory entry to verify it's a
	 * regular file, or empty directory.
	 */
	if (flag == RENAME && wantparent && *ndp->ni_next == 0) {
		if (error = iaccess(dp, IWRITE, ndp->ni_cred))
			return (error);
		if (dp->i_number == ndp->ni_dent.d_fileno)
			return (EISDIR);
		if (error = iget(dp, ndp->ni_dent.d_fileno, &tdp, 0))
			return (error);
		/*
		 * Check for race between the name match and
		 * iget() of the file -- could be deleted -- see
		 * comment above.
		 */
		vdp = ITOV(tdp);
		BM(VN_LOCK(vdp));
		type = vdp->v_type;
		BM(VN_UNLOCK(vdp));
		if (type == VNON) {
			vrele(vdp);
			if (++trys < 5)
				goto rescan;
			ndp->ni_vp = NULLVP;
			return(ENOENT);
		}
		ndp->ni_vp = vdp;
		return (0);
	}

	/*
	 * Step through the translation in the name.  We do not `iput' the
	 * directory because we may need it again if a symbolic link is
	 * relative to the current directory.  We prevent deadlock by always
	 * fetching inodes from the root, moving down the directory tree.
	 * There is a potential race condition here if both the current
	 * and parent directories are removed before the `iget' for the
	 * inode associated with ".." returns.  We hope that this occurs
	 * infrequently since we cannot avoid this race condition without
	 * implementing a sophisticated deadlock detection algorithm.
	 * Note also that this simple deadlock detection scheme will not
	 * work if the file system has any hard links other than ".."
	 * that point backwards in the directory structure.
	 */
	if (ndp->ni_isdotdot) {
		if (error = iget(dp, ndp->ni_dent.d_fileno, &tdp, 0)) {
			return (error);
		}
		ndp->ni_vp = ITOV(tdp);
	} else if (dp->i_number == ndp->ni_dent.d_fileno) {
		vdp = ITOV(dp);
		VREF(vdp);	/* we want ourself, ie "." */
		ndp->ni_vp = vdp;
	} else {
		if (error = iget(dp, ndp->ni_dent.d_fileno, &tdp, 0))
			return (error);
		ndp->ni_vp = ITOV(tdp);
	}
	ASSERT(ndp->ni_vp);
	BM(VN_LOCK(ndp->ni_vp));
	type = ndp->ni_vp->v_type;
	BM(VN_UNLOCK(ndp->ni_vp));
	if (type == VNON) {
		/*
		 * Hit the race mentioned above
		 */
		vrele(ndp->ni_vp);
		ndp->ni_vp = NULLVP;
		if (++trys < 5)
			goto rescan;
		return(ENOENT);
	}
	/*
	 * Insert name into cache if appropriate.
	 */
	if (ndp->ni_makeentry) {
		/* we need to interlock against a race in ufs_remove
		 * where we do a cache_purge-dirremove;  we use SCANDIR_LOCK
		 * to be equivalent to IN_READ_LOCK for code consistency.
		 */
		IN_READ_LOCK(dp);
		if (VTOI(ndp->ni_vp)->i_nlink > 0)
			/* if indeed link count is zero, we don't want it
			 * in our namecache (i.e. it's no longer in our
			 * namespace.
			 */
			cache_enter(ndp);
		IN_READ_UNLOCK(dp);
	}
	return (0);
}


dirbad(ip, offset, how)
	struct inode *ip;
	off_t offset;
	char *how;
{

	printf("%s: bad dir ino %d at offset %d: %s\n",
	    ip->i_fs->fs_fsmnt, ip->i_number, offset, how);
	panic("bad dir");
}

/*
 * Do consistency checking on a directory entry:
 *	record length must be multiple of 4
 *	entry must fit in rest of its DIRBLKSIZ block
 *	record must be large enough to contain entry
 *	name is not longer than NAME_MAX
 *	name must be as long as advertised, and null terminated
 */
dirbadentry(ep, entryoffsetinblock)
	register struct dirent *ep;
	int entryoffsetinblock;
{
	register int i;

	if ((ep->d_reclen & 0x3) != 0 ||
	    ep->d_reclen > DIRBLKSIZ - (entryoffsetinblock & (DIRBLKSIZ - 1)) ||
	    ep->d_reclen < DIRSIZ(ep) || ep->d_namlen > NAME_MAX)
		return (1);
	for (i = 0; i < ep->d_namlen; i++)
		if (ep->d_name[i] == '\0')
			return (1);
	return (ep->d_name[i]);
}

/*
 * Do consistency checking on a block of directory entries:
 *	record length must be multiple of 4
 *	entry must fit in rest of its DIRBLKSIZ block
 *	record must be large enough to contain entry
 *	name is not longer than NAME_MAX
 *	name must be as long as advertised, and null terminated
 */
dirbadblock(bp)
	register struct buf *bp;
{
	register struct dirent *ep;
	int off;
	int start;
	register struct inode *ip;
	register struct fs *fs;

	ip = VTOI(bp->b_vp);
	fs = ip->i_fs;
	start = bp->b_lblkno * fs->fs_bsize;

	for (off = 0;
	     off < bp->b_bcount && start + off < ip->i_size;
	     off += ep->d_reclen) {
		ep = (struct dirent *)(bp->b_un.b_addr + off);
		if (dirbadentry(ep, off))
			return(1);
	}
	return(0);
}

/*
 * Write a directory entry after a call to namei, using the parameters
 * which it left in nameidata.  The argument ip is the inode which the
 * new directory entry will refer to.  The nameidata field ndp->ni_dvp
 * is a pointer to the directory to be written, which was left locked by
 * namei.  Remaining parameters (ndp->ni_offset, ndp->ni_count) indicate
 * how the space for the new entry is to be gotten.
 */
direnter(ip, ndp)
	struct inode *ip;
	register struct nameidata *ndp;
{
	register struct dirent *ep, *nep;
	register struct inode *dp = VTOI(ndp->ni_dvp);
	struct buf *bp;
	int loc, spacefree, error = 0;
	u_int dsize;
	int newentrysize;
	char *dirbuf;
	struct vnode *vp = ITOV(dp);
	
	LASSERT(IN_WRITE_HOLDER(dp));
	ndp->ni_dent.d_fileno = ip->i_number;
	newentrysize = DIRSIZ(&ndp->ni_dent);
	if (error = checkdir(ndp, ADD|DEL))
		return(error);
	LASSERT(IN_WRITE_HOLDER(dp));
	/*
	 * Check to make sure the directory hasn't been removed.
	 */
	if (dp->i_nlink < 2) 
		return(ENOENT);
	if (ndp->ni_count == 0) {
		/*
		 * If ndp->ni_count is 0, then scandir couldn't find
		 * space in the directory. In this case ndp->ni_offset
		 * will be on a directory block boundary and we will write
		 * the new entry into a fresh block.
		 */
		if (ndp->ni_offset&(DIRBLKSIZ-1))
			panic("wdir: newblk");
		ndp->ni_dent.d_reclen = DIRBLKSIZ;
		ndp->ni_count = newentrysize;
		ndp->ni_resid = newentrysize;
		ndp->ni_base = (caddr_t)&ndp->ni_dent;
		ndp->ni_uioseg = UIO_SYSSPACE;
		ndp->ni_rw = UIO_WRITE;
		IN_SET_RECURSIVE(dp);
		error =
		    ufs_write(ndp->ni_dvp, &ndp->ni_uio, IO_SYNC, ndp->ni_cred);
		IN_CLEAR_RECURSIVE(dp);
		LASSERT(IN_WRITE_HOLDER(dp));
		if (DIRBLKSIZ > dp->i_fs->fs_fsize)
			panic("wdir: blksize"); /* XXX - should grow w/balloc */
		else {
			/* should we check for error first ? */
			IN_LOCK(dp);
			dp->i_size = roundup(dp->i_size, DIRBLKSIZ);
			dp->i_flag |= ICHG;
			IN_UNLOCK(dp);
			dp->i_dirstamp++;
			if (!error && ndp->ni_makeentry) {
				ndp->ni_vp = ITOV(ip);
				ndp->ni_ptr = ndp->ni_dent.d_name;
				cache_enter(ndp);
				ndp->ni_vp = NULLVP;
			}
		}
		return (error);
	}

	/*
	 * If ndp->ni_count is non-zero, then namei found space for the new
	 * entry in the range ndp->ni_offset to ndp->ni_offset + ndp->ni_count.
	 * in the directory.  To use this space, we may have to compact
	 * the entries located there, by copying them together towards
	 * the beginning of the block, leaving the free space in
	 * one usable chunk at the end.
	 */

	/*
	 * Increase size of directory if entry eats into new space.
	 * This should never push the size past a new multiple of
	 * DIRBLKSIZE.
	 *
	 * N.B. - THIS IS AN ARTIFACT OF 4.2 AND SHOULD NEVER HAPPEN.
	 */
	BM(IN_LOCK(dp));
	if (ndp->ni_offset + ndp->ni_count > dp->i_size)
		dp->i_size = ndp->ni_offset + ndp->ni_count;
	BM(IN_UNLOCK(dp));
	/*
	 * Get the block containing the space for the new directory entry.
	 */
	if (error = blkatoff(dp, ndp->ni_offset, (char **)&dirbuf, &bp)) 
		return (error);
	/*
	 * Find space for the new entry.  In the simple case, the
	 * entry at offset base will have the space.  If it does
	 * not, then scandir arranged that compacting the region
	 * ndp->ni_offset to ndp->ni_offset+ndp->ni_count would yield the space.
	 */
	LASSERT(IN_WRITE_HOLDER(dp));
	ep = (struct dirent *)dirbuf;
	dsize = DIRSIZ(ep);
	spacefree = ep->d_reclen - dsize;
	for (loc = ep->d_reclen; loc < ndp->ni_count; ) {
		nep = (struct dirent *)(dirbuf + loc);
		if (ep->d_fileno) {
			/* trim the existing slot */
			ep->d_reclen = dsize;
			ep = (struct dirent *)((char *)ep + dsize);
		} else {
			/* overwrite; nothing there; header is ours */
			spacefree += dsize;
		}
		dsize = DIRSIZ(nep);
		spacefree += nep->d_reclen - dsize;
		loc += nep->d_reclen;
		bcopy((caddr_t)nep, (caddr_t)ep, dsize);
	}
	/*
	 * Update the pointer fields in the previous entry (if any),
	 * copy in the new entry, and write out the block.
	 */
	if (ep->d_fileno == 0) {
		if (spacefree + dsize < newentrysize)
			panic("wdir: compact1");
		ndp->ni_dent.d_reclen = spacefree + dsize;
	} else {
		if (spacefree < newentrysize)
			panic("wdir: compact2");
		ndp->ni_dent.d_reclen = spacefree;
		ep->d_reclen = dsize;
		ep = (struct dirent *)((char *)ep + dsize);
	}
	bcopy((caddr_t)&ndp->ni_dent, (caddr_t)ep, (u_int)newentrysize);
	error = bwrite(bp);
	LASSERT(IN_WRITE_HOLDER(dp));
	IN_LOCK(dp);
	dp->i_flag |= IUPD|ICHG;
	IN_UNLOCK(dp);
	dp->i_dirstamp++;
	BM(IN_LOCK(dp));
	if (ndp->ni_endoff && ndp->ni_endoff < dp->i_size) {
		BM(IN_UNLOCK(dp));
		IN_SET_RECURSIVE(dp);
		(void) itrunc(dp, ndp->ni_endoff, IO_SYNC);
		IN_CLEAR_RECURSIVE(dp);
		LASSERT(IN_WRITE_HOLDER(dp));
	} else
		BM(IN_UNLOCK(dp));
	if (!error && ndp->ni_makeentry) {
		ndp->ni_vp = ITOV(ip);
		ndp->ni_ptr = ndp->ni_dent.d_name;
		cache_enter(ndp);
		ndp->ni_vp = NULLVP;
	}
	return (error);
}

/*
 * Remove a directory entry after a call to namei, using
 * the parameters which it left in nameidata. The entry
 * ni_offset contains the offset into the directory of the
 * entry to be eliminated.  The ni_count field contains the
 * size of the previous record in the directory.  If this
 * is 0, the first entry is being deleted, so we need only
 * zero the inode number to mark the entry as free.  If the
 * entry isn't the first in the directory, we must reclaim
 * the space of the now empty record by adding the record size
 * to the size of the previous entry.
 */
dirremove(ndp)
	register struct nameidata *ndp;
{
	register struct inode *dp = VTOI(ndp->ni_dvp);
	struct dirent *ep;
	struct buf *bp;
	int error;

	LASSERT(IN_WRITE_HOLDER(dp));
	if (error = checkdir(ndp, DEL))
		return(error);
	LASSERT(IN_WRITE_HOLDER(dp));
	if (ndp->ni_count == 0) {
		/*
		 * First entry in block: set d_fileno to zero.
		 */
		ndp->ni_dent.d_fileno = 0;
		ndp->ni_count = ndp->ni_resid = DIRSIZ(&ndp->ni_dent);
		ndp->ni_base = (caddr_t)&ndp->ni_dent;
		ndp->ni_uioseg = UIO_SYSSPACE;
		ndp->ni_rw = UIO_WRITE;
		IN_SET_RECURSIVE(dp);
		error =
		    ufs_write(ndp->ni_dvp, &ndp->ni_uio, IO_SYNC, ndp->ni_cred);
		IN_CLEAR_RECURSIVE(dp);
		LASSERT(IN_WRITE_HOLDER(dp));
	} else {
		/*
		 * Collapse new free space into previous entry.
		 */
		if (error = blkatoff(dp, ndp->ni_offset - ndp->ni_count,
		    (char **)&ep, &bp))
			return (error);
		ep->d_reclen += ndp->ni_dent.d_reclen;
		LASSERT(IN_WRITE_HOLDER(dp));
		error = bwrite(bp);
		IN_LOCK(dp);
		dp->i_flag |= IUPD|ICHG;
		IN_UNLOCK(dp);
	}
	dp->i_dirstamp++;
	return (error);
}

/*
 * Rewrite an existing directory entry to point at the inode
 * supplied.  The parameters describing the directory entry are
 * set up by a call to namei.
 */
dirrewrite(dp, ip, ndp)
	struct inode *dp, *ip;
	struct nameidata *ndp;
{

	int error = 0;

	LASSERT(IN_WRITE_HOLDER(dp));
	ndp->ni_dent.d_fileno = ip->i_number;
	ndp->ni_count = ndp->ni_resid = DIRSIZ(&ndp->ni_dent);
	ndp->ni_base = (caddr_t)&ndp->ni_dent;
	ndp->ni_uioseg = UIO_SYSSPACE;
	ndp->ni_rw = UIO_WRITE;
	IN_SET_RECURSIVE(dp);
	error = ufs_write(ITOV(dp), &ndp->ni_uio, IO_SYNC, ndp->ni_cred);
	IN_CLEAR_RECURSIVE(dp);
	LASSERT(IN_WRITE_HOLDER(dp));
	dp->i_dirstamp++;
	return(error);
}

/*
 * Return buffer with contents of block "offset"
 * from the beginning of directory "ip".  If "res"
 * is non-zero, fill it in with a pointer to the
 * remaining space in the directory.
 */
blkatoff(ip, offset, res, bpp)
	struct inode *ip;
	off_t offset;
	char **res;
	struct buf **bpp;
{
	register struct fs *fs = ip->i_fs;
	daddr_t lbn = lblkno(fs, offset);
	int bsize = blksize(fs, ip, lbn);
	struct buf *bp;
	daddr_t bn;
	int error;

	*bpp = 0;
	if (error = bread(ITOV(ip), lbn, bsize, NOCRED, &bp)) {
		brelse(bp);
		return (error);
	}
	if (res)
		*res = bp->b_un.b_addr + blkoff(fs, offset);
	*bpp = bp;
	return (0);
}

/*
 * Check if a directory is empty or not.
 * Inode supplied must be locked.
 *
 * Using a struct dirtemplate here is not precisely
 * what we want, but better than using a struct dirent.
 *
 * NB: does not handle corrupted directories.
 */
dirempty(ip, parentino, cred)
	register struct inode *ip;
	ino_t parentino;
	struct ucred *cred;
{
	register off_t off;
	struct dirtemplate dbuf;
	register struct dirent *dp = (struct dirent *)&dbuf;
	int error, count;
	int size;
#define	MINDIRSIZ (sizeof (struct dirtemplate) / 2)

	LASSERT(IN_READ_HOLDER(ip));
	BM(IN_LOCK(ip));
	size = ip->i_size;
	BM(IN_UNLOCK(ip));
	for (off = 0; off < size; off += dp->d_reclen) {
#if defined(DCEDFS) && DCEDFS
                error = vn_rdwr(VNUIO_READNESTED, ITOV(ip), (caddr_t)dp, 
		   MINDIRSIZ,
#else
		error = vn_rdwr(UIO_READ, ITOV(ip), (caddr_t)dp, MINDIRSIZ,
#endif /* DCEDFS */
		    off, UIO_SYSSPACE, 0, cred, &count);
		/*
		 * Since we read MINDIRSIZ, residual must
		 * be 0 unless we're at end of file.
		 */
		if (error || count != 0)
			return (0);
		/* avoid infinite loops */
		if (dp->d_reclen == 0)
			return (0);
		/* skip empty entries */
		if (dp->d_fileno == 0)
			continue;
		/* accept only "." and ".." */
		if (dp->d_namlen > 2)
			return (0);
		if (dp->d_name[0] != '.')
			return (0);
		/*
		 * At this point d_namlen must be 1 or 2.
		 * 1 implies ".", 2 implies ".." if second
		 * char is also "."
		 */
		if (dp->d_namlen == 1)
			continue;
		if (dp->d_name[1] == '.' && dp->d_fileno == parentino)
			continue;
		return (0);
	}
	return (1);
}

/*
 * Check if source directory is in the path of the target directory.
 * Target is supplied locked, source is unlocked.
 * The target is always iput() before returning.
 *
 * Can't iput target and expect to use it in ufs_rename, who called us.
 * Therefore, target is NEVER iput before returning.
 */
checkpath(source, target, cred)
	struct inode *source, *target;
	struct ucred *cred;
{
	struct dirtemplate dirbuf;
	struct inode *ip, *newip;
	int error = 0;

	ip = target;
	if (ip->i_number == source->i_number) {
		error = EEXIST;
		goto out;
	}
	if (ip->i_number == ROOTINO)
		goto out;

	for (;;) {
		BM(IN_LOCK(ip));
		if ((ip->i_mode&IFMT) != IFDIR) {
			BM(IN_UNLOCK(ip));
			error = ENOTDIR;
			break;
		}
		BM(IN_UNLOCK(ip));
#if defined(DCEDFS) && DCEDFS
                error = vn_rdwr(VNUIO_READNESTED, ITOV(ip), (caddr_t)&dirbuf,
#else
		error = vn_rdwr(UIO_READ, ITOV(ip), (caddr_t)&dirbuf,
#endif /* DCEDFS */
			sizeof (struct dirtemplate), (off_t)0, UIO_SYSSPACE,
			0, cred, (int *)0);
		if (error != 0)
			break;
		if (dirbuf.dotdot_namlen != 2 ||
		    dirbuf.dotdot_name[0] != '.' ||
		    dirbuf.dotdot_name[1] != '.') {
			error = ENOTDIR;
			break;
		}
		if (dirbuf.dotdot_ino == source->i_number) {
			error = EINVAL;
			break;
		}
		if (dirbuf.dotdot_ino == ROOTINO)
			break;
		if (error = iget(ip, dirbuf.dotdot_ino, &newip, 0))
			break;
		iput(ip);
		ip = newip;
	}

out:
	if (error == ENOTDIR)
		printf("checkpath: .. not a directory\n");
	if (ip != NULL)
		iput(ip);
	return (error);
}

/*
 * Check whether the state of the parent directory has changed since
 * the lookup was done in namei.  The flag passed in indicates whether
 * we want to check for additions or deletions from the parent directory.
 */
checkdir(ndp, flag)
register struct nameidata *ndp;
int flag;
{

	int error = 0;
	struct inode *dp = VTOI(ndp->ni_dvp);
	char tmp = 0;

	/*
	 * We have to set up the ndp->ni_ptr to point to the last component
	 * because scandir expects the component to be there.
	 */
	ndp->ni_ptr = &ndp->ni_dent.d_name[0];
	ndp->ni_next = &tmp;

        /*
         * There is a small hole here.  It is possible (but not very likely)
         * that this thread could have taken billions & billions of interrupts
	 * and the stamp wrapped and is back where it was when it was first
         * recorded.  The pains of plugging this are too great, this is a hole
         * we can live with.
         */
	while (ndp->ni_dirstamp != dp->i_dirstamp) {
		register struct vnode *vp;

		LASSERT(IN_WRITE_HOLDER(dp));
		NC_STATS(nchstats.ncs_dirscan++);
		/*
		 * Two cases: adding and deleting.
		 * 1) adding: ni_vp is not meaningful (i.e. target does,
		 *    or did not, exist).  If the scandir below is
		 *    successful, it means that someone beat us to the
		 *    creation of the name, so we must vrele the vnode and
		 *    return EEXIST.
		 * 2) deleting: ni_vp must point to the vnode to be
		 *    deleted.  If the scandir is successful, we've got
		 *    an extra reference to the vnode, and it must be
		 *    vrele'd before we return.  If the scandir is
		 *    unsuccessful, then someone beat us to the removal,
		 *    and the vnode will be vrele'd when we return the
		 *    error of ENOENT.  This counts on the fact that
		 *    scandir will not null ni_vp on failure.
		 *
		 *    One other case in deleting.  It could happen that
		 *    between the first namei and now, the target was
		 *    not only deleted, but recreated under the same name.
		 *    To avoid removing the new file, we check for this
		 *    condition and vrele the new vnode, null the ni_vp,
		 *    and return ENOENT.  The caller must maintain a
		 *    handle on the old vnode to do the vrele in the
		 *    error case.  It cannot rely on the ni_vp remaining
		 *    constant.
		 *
		 *    References on the parent directory are handled in
		 *    the layers above.
		 */
		vp = ndp->ni_vp;
		IN_SET_RECURSIVE(dp);
		error = scandir(ITOV(dp), ndp);
		IN_CLEAR_RECURSIVE(dp);
		LASSERT(IN_WRITE_HOLDER(dp));
		/*
		 * If the ADD flag is set, return EEXIST if we have
		 * found the entry - someone else beat us to adding
		 * that entry.  If the DEL flag is *also* set, we don't
		 * want to return an error if we didn't find the entry
		 * (that is what we are hoping for!).  We need to
		 * recheck the timestamps and possibly rescan in that
		 * case.
		 *
		 * Otherwise, if only the DEL flag is set and we got
		 * back an error, return it.
		 * 
		 * If the DEL flag is set and the RNM flag is set, then
		 * we found a different vnode, return the new one.
		 *
		 * NOTE: Even though there is a RNM flag, the assertion
		 * below is correct.  One of ADD or DEL must be set.
		 */
		ASSERT(flag&(ADD|DEL));
		if (flag & ADD) {
			if (!error) {
				ASSERT(ndp->ni_vp != NULLVP);
				vrele(ndp->ni_vp);
				error = EEXIST;
				break;
			} 
			if (error != ENOENT)
				break;
			error = 0;
		} else if (flag & DEL) {
			if (error)
				break;
			else {
				ASSERT(ndp->ni_vp != NULLVP);
				if (vp != ndp->ni_vp) {
					/* file deleted, then re-created */
					if (flag & RNM) {
						vrele(vp);
						error = EEXIST;
					} else {
						vrele(ndp->ni_vp);
						ndp->ni_vp = NULL;
						error = ENOENT;
					}
					break;
				} else
					vrele(vp);
			}
		}
	}
	return(error);
}
