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
static char	*sccsid = "@(#)$RCSfile: s5fs_lookup.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/05/27 19:26:52 $";
#endif
/*
 */
/*
 * (c) Copyright 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * History:
 *
 *	August 20, 1991: vsp
 *		OSF/1 Release 1.0.1 bug fixes.
 *		inode locking fixes, updated s5checkdir()
 *
 * 	ADK:
 *		original OSF/1 Release 1.0
 *
 *
 */

#include <sysv_fs.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <s5fs/s5param.h>
#include <s5fs/filsys.h>
#include <s5fs/s5inode.h>
#include <s5fs/s5dir.h>
#include <sys/dk.h>	/* for SAR counters */

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
 * If flag has WANTPARENT or'ed into it and the target of the pathname
 * exists, lookup returns both the target and its parent directory locked.
 * When creating or renaming and WANTPARENT is specified, the target may
 * not be ".".  When deleting and WANTPARENT is specified, the target may
 * be "."., but the caller must check to ensure it does an vrele and iput
 * instead of two iputs.
 *
 * Overall outline of s5fs_lookup:
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
 * NOTE: (LOOKUP) currently returns all of inodes unlocked.
 */
s5fs_lookup(vp, ndp)
	struct vnode *vp;
	register struct nameidata *ndp;
{
	register struct vnode *vdp;	/* vnode copy of dp */
	register struct s5inode *dp;	/* the directory we are searching */
	struct buf *bp = 0;		/* a buffer of directory entries */
	register struct s5direct *ep;	/* the current directory entry */
	int entryoff; 			/* offset of ep in bp's buffer */
	enum {NONE, FOUND} slotstatus;
	int slotoffset = -1;		/* offset of area with free space */
	struct s5inode *pdp;		/* saved dp during symlink work */
	struct s5inode *tdp;		/* returned by s5iget */
	int flag;			/* LOOKUP/CREATE/RENAME or DELETE */
	int wantparent;			/* 1 => wantparent/lockparent flag */
	int bsize; 			/* block size */
	int s5dirlen = sizeof(struct s5direct);
	int endsearch, lbn, nbn, error = 0;
	unsigned int namlen;

	ndp->ni_dvp = vp;
	ndp->ni_vp = NULL;
	dp = S5VTOI(vp);

	flag = ndp->ni_nameiop & OPFLAG;
	wantparent = ndp->ni_nameiop & (WANTPARENT);

	/*
	 * Check accessiblity of directory.
	 */
	if ((dp->i_mode&S5IFMT) != S5IFDIR)
		return (ENOTDIR);
	/*
	 * Skip access check if flag is set.
	 */
	if (error = s5iaccess(dp, S5IEXEC, ndp->ni_cred))
		return (error);
	/*
	 * We now have a segment name to search for, and a directory to 
	 * search.
	 */
	if ((flag == CREATE || flag == RENAME) && *ndp->ni_next == 0) 
		slotstatus = NONE;
	else
		slotstatus = FOUND;

	/* 
	 * Set up to search a directory 
	 */  
	s5ILOCK(dp);
	ndp->ni_offset = 0; 
	endsearch = dp->i_size;
	slotoffset = -1;
	lbn = -1;
	bp = NULL;
	entryoff = 0;
	bsize = FsBSIZE(dp->i_s5fs);
	/*
	 * Record timestamp while holding the read lock on the directory.
	 */
	ndp->ni_dirstamp = dp->i_dirstamp;

searchloop:
	while (ndp->ni_offset < endsearch) {
 
		/* 
		 * If the offset is on a block boundary,
		 * read the next directory block. 
		 */
		if (lbn != (nbn = FsBNO(bsize, ndp->ni_offset))) {
			lbn = nbn;
			entryoff = 0;
			if (bp != NULL)
				brelse(bp);
			/* global table() system call counter (see table.h) */
        		tf_dirblk++;
			error = bread(vp, lbn, bsize, NOCRED, &bp);
			if (error) {
			        s5IUNLOCK(dp);
				brelse(bp);
				return (error);
			}	
		}	

		ep = (struct s5direct *)(bp->b_un.b_addr + entryoff);
		/*
		 * Check for a name match.
		 */
		if (ep->d_ino) {  
		       /* If the name from namei is > s5DIRSIZ, we
			  compare only s5DIRSIZ characters and the
			  file name will be truncated.
			  XXX - Need to ensure POSIX_NO_TRUNC is
			        returned correctly for System V fs
		       */
			namlen=strnlen(ndp->ni_dent.d_name,s5DIRSIZ);
			if (strnlen(ep->d_name, s5DIRSIZ) == namlen &&
			    !bcmp(ndp->ni_ptr, ep->d_name,(unsigned)namlen)){
				/*
				 * Save directory entry's inode number and
				 * dir size in ndp->ni_gpdent, and release
				 * directory buffer.
				 */
				ndp->ni_gpdent.d_ino = ep->d_ino;
				brelse(bp);
				goto found;
			}
		}
		else { 	
			/* 
			 * We found an empty slot for new entry 
			 */
			if (slotoffset == -1) 
				slotoffset = ndp->ni_offset; 
			slotstatus = FOUND;
		}	

		ndp->ni_offset += s5dirlen;
		entryoff += s5dirlen;		

	} /* while loop */

/* notfound: */

	if (bp != NULL)
		brelse(bp);
	/*
	 * If creating, and at end of pathname and current
	 * directory has not been removed, then can consider
	 * allowing file to be created.
	 */
	if ((flag == CREATE || flag == RENAME) &&
	    *ndp->ni_next == 0 && dp->i_nlink != 0) {
		/*
		 * Access for write is interpreted as allowing
		 * creation of files in the directory.
		 */
		error = s5iaccess(dp, S5IWRITE, ndp->ni_cred);
		if (error) {
		        s5IUNLOCK(dp);
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
			ndp->ni_offset = dp->i_size; 
			ndp->ni_count = 0;
		} else {
			ndp->ni_offset = slotoffset;
			ndp->ni_count = s5dirlen;
		}

		dp->i_flag |= S5IUPD|S5ICHG;
		/*
		 * We return with the directory locked, so that
		 * the parameters we set up above will still be
		 * valid if we actually decide to do a s5direnter().
		 * We return ni_vp == NULL to indicate that the entry
		 * does not currently exist; we leave a pointer to
		 * the (locked) directory inode in ndp->ni_dvp.
		 *
		 * NB - if the directory is unlocked, then this
		 * information cannot be used.
		 */
	}
	s5IUNLOCK(dp);

	return (ENOENT);

found:
	/*
	 * Found component in pathname.
	 */

	/*
	 * If deleting, and at end of pathname, return
	 * parameters which can be used to remove file.
	 * If the wantparent flag isn't set, we return only
	 * the directory (in ndp->ni_dvp), otherwise we go
	 * on and lock the inode, being careful with ".".
	 */
	s5IUNLOCK(dp);
	if (flag == DELETE && *ndp->ni_next == 0) {
		/*
		 * Write access to directory required to delete files.
		 */
		if (error = s5iaccess(dp, S5IWRITE, ndp->ni_cred))
			return (error);
		/*
		 * Return pointer to current entry in ndp->ni_offset.
		 * NOTE: ndp->ni_count is not used in S5FS. 
		 */

		vdp = S5ITOV(dp);

		if (dp->i_number == ndp->ni_gpdent.d_ino) {
			VREF(vdp);  /* This is "." case */
		} else {
			if (error = s5iget(dp, ndp->ni_gpdent.d_ino, &tdp))
				return (error);
			vdp = S5ITOV(tdp);
			/*
			 * If directory is "sticky", then user must own
			 * the directory, or the file in it, else he
			 * may not delete it (unless he's root). This
			 * implements append-only directories.
			 */
			if ((dp->i_mode & S5ISVTX) &&
			    ndp->ni_cred->cr_uid != 0 &&
			    ndp->ni_cred->cr_uid != dp->i_uid &&
			    tdp->i_uid != ndp->ni_cred->cr_uid) {
				s5iput(tdp);
				return (EPERM);
			}
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
		if (error = s5iaccess(dp, S5IWRITE, ndp->ni_cred))
			return (error);
		/*
		 * Careful about locking second inode.
		 * This can only occur if the target is ".".
		 */
		if (dp->i_number == ndp->ni_gpdent.d_ino)
			return (EISDIR);
		if (error = s5iget(dp, ndp->ni_gpdent.d_ino, &tdp))
			return (error);
		ndp->ni_vp = S5ITOV(tdp);
		return (0);
	}

	/*
	 * Step through the translation in the name.  We do not `iput' the
	 * directory because we may need it again if a symbolic link
	 * is relative to the current directory.  Instead we save it
	 * unlocked as "pdp".  We must get the target inode before unlocking
	 * the directory to insure that the inode will not be removed
	 * before we get it.  We prevent deadlock by always fetching
	 * inodes from the root, moving down the directory tree. Thus
	 * when following backward pointers ".." we must unlock the
	 * parent directory before getting the requested directory.
	 * There is a potential race condition here if both the current
	 * and parent directories are removed before the `iget' for the
	 * inode associated with ".." returns.  We hope that this occurs
	 * infrequently since we cannot avoid this race condition without
	 * implementing a sophisticated deadlock detection algorithm.
	 * Note also that this simple deadlock detection scheme will not
	 * work if the file system has any hard links other than ".."
	 * that point backwards in the directory structure.
	 */
	pdp = dp;
	if (ndp->ni_isdotdot) {
		if (error = s5iget(dp, ndp->ni_gpdent.d_ino, &tdp)) {
			return (error);
		}
		ndp->ni_vp = S5ITOV(tdp);
	} else if (dp->i_number == ndp->ni_gpdent.d_ino) {
		vdp = S5ITOV(dp);
		VREF(vdp);	/* we want ourself, ie "." */
		ndp->ni_vp = vdp;
	} else {
		if (error = s5iget(dp, ndp->ni_gpdent.d_ino, &tdp))
			return (error);
		ndp->ni_vp = S5ITOV(tdp);
	}

	return (0);
}




int strnlen(s, n)
	register char *s;
	register int n;
{
	register int m;

	for (m=0; *s != '\0'; s++)
		if (++m >= n)
			return (n);
	return (m);
}


/*
 * Check if source directory is in the path of the target directory.
 * Target is supplied locked, source is unlocked.
 * The target is always s5iput() before returning.
 */
s5checkpath(source, target, cred)
     struct s5inode *source, *target;
     struct ucred *cred;
{
     struct s5dirtemp dirbuf;
     struct s5inode *ip;
     int error = 0;

     ip = target;
     if (ip->i_number == source->i_number) {
           error = EEXIST;
	   goto out;
     }
     if (ip->i_number == s5ROOTINO)
           goto out;
     for (;;) {
           if ((ip->i_mode & S5IFMT) != S5IFDIR) {
	         error = ENOTDIR;
		 break;
	   }
	   error = vn_rdwr(UIO_READ, S5ITOV(ip), (caddr_t)&dirbuf,
			   sizeof (struct s5dirtemp), (off_t)0, UIO_SYSSPACE,
			   IO_NODELOCKED, cred, (int*)0);
	   if (error != 0)
	         break;
	   if (dirbuf.dotdot_name[0] != '.' ||
	       dirbuf.dotdot_name[1] != '.' ||
	       dirbuf.dotdot_name[2] != '\0' ) {
	           error = ENOTDIR;
		   break;
	   }
	   if (dirbuf.dotdot_ino == source->i_number) {
	         error = EINVAL;
		 break;
	   }
	   if (dirbuf.dotdot_ino == s5ROOTINO)
	         break;
	   s5iput(ip);
	   if (error = s5iget(ip, dirbuf.dotdot_ino, &ip))
	         break;
   }

out:
     if (error == ENOTDIR)
           printf("s5checkpath: .. not a directory\n");
     if (ip != NULL)
           s5iput(ip);
     return (error);
}


/*
 * Check whether the state of the parent directory has changed since
 * the lookup was done in namei.  The flag passed in indicates whether
 * we want to check for additions or deletions from the parent directory.
 */
s5checkdir(ndp, flag)
register struct nameidata *ndp;
int flag;
{

	int error = 0;
	struct s5inode *dp = S5VTOI(ndp->ni_dvp);
	char tmp = 0;

	ASSERT(s5ILOCK_HOLDER(dp));
	/*
	 * We have to set up the ndp->ni_ptr to point to the last component
	 * because lookup expects the component to be there.
	 */
	ndp->ni_ptr = &ndp->ni_dent.d_name[0];
	ndp->ni_next = &tmp;

	while (ndp->ni_dirstamp != dp->i_dirstamp) {
		register struct vnode *vp;

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
		 *    error of ENOENT.
		 *    One other case in deleting.  It could happen that
		 *    between the first namei and now, the target was
		 *    not only deleted, but recreated under the same name.
		 *    To avoid removing the new file, we check for this
		 *    condition and vrele the new vnode and return ENOENT
		 *    so the old vnode will also be released.
		 *
		 *    References on the parent directory are handled in
		 *    the layers above.
		 */
		vp = ndp->ni_vp;
		s5_SET_RECURSIVE(dp);
		error = s5fs_lookup(S5ITOV(dp), ndp);
		s5_CLEAR_RECURSIVE(dp);
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
					vrele(ndp->ni_vp);
					error = ENOENT;
					break;
				} else
					vrele(vp);
			}
		}
	}
	ASSERT(s5ILOCK_HOLDER(dp));
	return(error);
}


/*
 * Rewrite an existing directory entry to point at the inode
 * supplied.  The parameters describing the directory entry are
 * set up by a call to namei.
 */
s5dirrewrite(dp, ip, ndp)
     struct s5inode *dp, *ip;
     struct nameidata *ndp;
{
	struct s5direct tdirect;
	int i = 0, error = 0;

	ASSERT(s5ILOCK_HOLDER(dp));
	tdirect.d_ino = ip->i_number;
	while ((tdirect.d_name[i] = ndp->ni_gpdent.d_name[i]) != '\0'
		&& ++i < s5DIRSIZ)
	       ;
	ndp->ni_count = ndp->ni_resid = s5DIRECTSIZE;
        ndp->ni_base = (caddr_t)&tdirect;
	ndp->ni_iovcnt = 1;
        ndp->ni_uioseg = UIO_SYSSPACE;
        ndp->ni_rw = UIO_WRITE;

	s5_SET_RECURSIVE(dp);
        error = s5fs_write(S5ITOV(dp), &ndp->ni_uio, IO_SYNC, ndp->ni_cred);
	s5_CLEAR_RECURSIVE(dp);
	ASSERT(s5ILOCK_HOLDER(dp));
	return(error);
}
	

/*
 * Check if a directory is empty or not.
 * Inode supplied must not be locked, since it will be calling
 * via vn_rdwr.
 *
 * NB: does not handle corrupted directories.
 */
s5dirempty(ip, parentino, cred)
     register struct s5inode *ip;
     s5ino_t parentino;
     struct ucred *cred;
{
	register off_t off;
	int error = 0;
	struct s5direct dp;
	int count;

	ASSERT(s5ILOCK_HOLDER(ip));
	for (off = 0; off < ip->i_size; off += s5DIRECTSIZE) {
		s5_SET_RECURSIVE(ip);
		error = vn_rdwr(UIO_READ, S5ITOV(ip), (caddr_t) &dp,
				s5DIRECTSIZE, off, UIO_SYSSPACE, 
				IO_NODELOCKED, cred, &count);
		s5_CLEAR_RECURSIVE(ip);
		/*
		 * Since we read s5DIRECTSIZE, residual must be 0
		 * unless we're at end of file.
		 */
		if (error || count != 0)
		    return (0);
		/* skip empty entries */
		if (dp.d_ino == 0)
		    continue;
		/* accept only "." and ".." */
		if (dp.d_name[0] != '.')
		    return (0);
		if (dp.d_name[1] == '\0')    /* "." */
		    continue;
		if  (dp.d_name[1] == '.' && dp.d_name[2] == '\0'
		     && dp.d_ino == parentino)
		        continue;
		ASSERT(s5ILOCK_HOLDER(ip));
		return(0);
	}
	ASSERT(s5ILOCK_HOLDER(ip));
	return (1);
}


/*
 * Remove a directory entry after a call to namei, using
 * the parameters which it left in nameidata.  The entry
 * ni_offset contains the offset into the directory of the
 * entry to be eliminated.  We only need to zero the inode
 * number to mark the entry as free.
 *
 * This routine assumes that the directory is locked when
 * called.  It temporarily gives up the lock when calling
 * s5fs_write.
 */
s5dirremove(ndp)
     register struct nameidata *ndp;
{
	register struct s5inode *dp = S5VTOI(ndp->ni_dvp);
	struct s5direct tdirect;
	int error;

	ASSERT(s5ILOCK_HOLDER(dp));
	if (error = s5checkdir(ndp, DEL))
		return(error);
	ASSERT(s5ILOCK_HOLDER(dp));
	tdirect.d_ino = 0;
	bzero(tdirect.d_name, s5DIRSIZ);
	
	ndp->ni_count = ndp->ni_resid = s5DIRECTSIZE;
        ndp->ni_base = (caddr_t)&tdirect;
	ndp->ni_iovcnt = 1;
        ndp->ni_uioseg = UIO_SYSSPACE;
        ndp->ni_rw = UIO_WRITE;

	s5_SET_RECURSIVE(dp);
        error = s5fs_write(ndp->ni_dvp, &ndp->ni_uio, IO_SYNC, ndp->ni_cred);
	s5_CLEAR_RECURSIVE(dp);
	dp->i_dirstamp++;

	return (error);
}
