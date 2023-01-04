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
static char	*sccsid = "@(#)$RCSfile: vfs_lookup.c,v $ $Revision: 4.2.17.8 $ (DEC) $Date: 1993/08/26 12:57:35 $";
#endif 
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
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
 *	@(#)vfs_lookup.c	7.18 (Berkeley) 8/10/89
 */

#include <sys/secdefines.h>
#if	SEC_BASE
#include <sys/security.h>
#endif

#include <sys/param.h>
#include <sys/time.h>
#include <sys/user.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/errno.h>
#include <sys/mode.h>
#include <ufs/inode.h>
#include <sys/stat.h>
#ifdef KTRACE
#include <sys/proc.h>
#include <sys/ktrace.h>
#endif

#if	MACH
#include <sys/user.h>
#include <kern/zalloc.h>
#else
#include <sys/malloc.h>
#endif	

#include <sys/dk.h>	/* for SAR counters */
#include <dcedfs.h>

#if	MACH
#define PN_ALLOCATE(buf)		ZALLOC(pathname_zone, (buf), caddr_t)
#define	PN_DEALLOCATE(buf)		ZFREE(pathname_zone, (buf))
#else
#define PN_ALLOCATE(buf)		MALLOC((buf), caddr_t, MAXPATHLEN, M_NAMEI, M_WAITOK)
#define	PN_DEALLOCATE(buf)		FREE((buf), M_NAMEI);
#endif
/*
 * Convert a pathname into a pointer to a locked inode.
 * This is a very central and rather complicated routine.
 *
 * The flag argument is LOOKUP, CREATE, RENAME, or DELETE depending on
 * whether the name is to be looked up, created, renamed, or deleted.
 * When CREATE, RENAME, or DELETE is specified, information usable in
 * creating, renaming, or deleting a directory entry may be calculated.
 *
 * The FOLLOW flag is set when symbolic links are to be followed
 * when they occur at the end of the name translation process.
 * Symbolic links are always followed for all other pathname
 * components other than the last.
 *
 * The segflg defines whether the name is to be copied from user
 * space or kernel space.
 *
 * Overall outline of namei:
 *
 *	copy in name
 *	get starting directory
 * dirloop:
 *	copy next component of name to ndp->ni_dent
 *	handle degenerate case where name is null string
 *	if .. and on mounted filesys, find parent
 *	call lookup routine for next component name
 *	  component vnode returned in ni_vp (if it exists), locked.
 *	if symbolic link, massage name in buffer and continue at dirloop
 *	if result inode is mounted on, find mounted on vnode
 *	if more components of name, do next level at dirloop
 *	return the answer in ni_vp as locked vnode;
 *
 */

/*
 * XXX -- temporary hacking to get binary compatibility for lookup of
 *	  null string ("");  Remove this when no longer required!
 */
int nullcompat = 0;
/* end hack */

namei(ndp)
	register struct nameidata *ndp;
{
	register char *cp;		/* pointer into pathname argument */
	register struct vnode *dp = 0;	/* the directory we are searching */
	register int i;		   	/* Temp counter */
	struct vnode *tdp;		/* saved dp */
	struct vnode *cvp;		/* covered vnode */
	struct mount *mp;		/* mount table entry */
	struct mount *temp_mp;		/* for temporary use */
	struct vnode *bypassvp;		/* ignore mount rules for this vnode */
	int docache;			/* == 0 do not cache last component */
	int flag;			/* LOOKUP, CREATE, RENAME or DELETE */
	int wantparent;			/* 1 => wantparent flag */
	int getbuf;			/* 1 => allocate a pathname buffer */
	int rdonly;			/* mounted read-only flag bit(s) */
	u_long vflag;			/* temporary copy of vnode flags */
	int mflag;			/* temporary copy of mount flags */
	int error = 0;
	int hadslash = 0;
	int stripslash;			/* strip trailing slashes */
	struct vnode *vp;
	struct stat sb;

        tf_namei++; /* global table() system call counter (see table.h) */

	ndp->ni_dvp = NULL;
	flag = ndp->ni_nameiop & OPFLAG;
	wantparent = ndp->ni_nameiop & WANTPARENT;
	stripslash = ndp->ni_nameiop & STRIPSLASH;
	docache = (ndp->ni_nameiop & NOCACHE) ^ NOCACHE;
	getbuf = (ndp->ni_nameiop & HASBUF) ^ HASBUF;
	/*
	 * OK to cache hits for CREATE as long as they are positive.
	 * This must be handled by the fs-dependent lookups.
	 */
	if (flag == DELETE)
		docache = 0;
	rdonly = M_RDONLY;
	if (ndp->ni_nameiop & REMOTE)
		rdonly |= M_EXRDONLY;
	if (ndp->ni_nameiop & BYPASSVP)
		bypassvp = ndp->ni_bypassvp;
	else
		bypassvp = NULLVP;

	/*
	 * Get a buffer for the name to be translated, and copy the
	 * name into the buffer.
	 */
	if (getbuf) {
		PN_ALLOCATE(ndp->ni_pnbuf);
		if (ndp->ni_segflg == UIO_SYSSPACE)
			error = copystr(ndp->ni_dirp, ndp->ni_pnbuf, 
					MAXPATHLEN, &ndp->ni_pathlen);
		else
			error = copyinstr(ndp->ni_dirp, ndp->ni_pnbuf, 
					  MAXPATHLEN, &ndp->ni_pathlen);
		if (error) {
			PN_DEALLOCATE(ndp->ni_pnbuf);
			ndp->ni_vp = NULL;
			if (error == ENOENT)
				error = ENAMETOOLONG;
			return (error);
		}
		/*
		 * Strip any trailing slashes if requested.  POSIX
		 * only requires this for directories.  The only 
		 * operations that should be specifying this are those
		 * that manipulate directories (mkdir, rmdir, rename).
		 * Trailing slashes for regular files generate an error.
		 * Note that stripping the slashes makes "foo/" look
		 * like "foo", rather than "foo/.".  This is not
		 * the same as the old behavior.
		 * Note that the pathlen includes the null character.
		 */
		if (stripslash) {
			cp = ndp->ni_pnbuf + ndp->ni_pathlen - 2;
			/*
		 	 * catch the degenerate name "/"
		 	 */
			while ((ndp->ni_pathlen > 2) && (*cp == '/')) {
				*cp-- = 0;
				ndp->ni_pathlen--;
			}
		}
		ndp->ni_ptr = ndp->ni_pnbuf;
	}
	ndp->ni_loopcnt = 0;
	dp = ndp->ni_cdir;
	VREF(dp);
	/*
	 * vmountread returns holding the vnode's mount structure's
	 * lookup lock for reading; this lock should be released
	 * via MOUNT_LOOKUP_DONE.
	 */
	vmountread(dp);
#ifdef KTRACE
	if (KTRPOINT(u.u_procp, KTR_NAMEI))
		ktrnamei(u.u_procp->p_tracep, ndp->ni_pnbuf);
#endif

start:
	/*
	 * Get starting directory.
	 * Done at start of translation and after symbolic link.
	 */
	if (*ndp->ni_ptr == '/') {
		MOUNT_LOOKUP_DONE(dp->v_mount);
		vrele(dp);
		while (*ndp->ni_ptr == '/') {
			ndp->ni_ptr++;
			ndp->ni_pathlen--;
		}
		if ((dp = ndp->ni_rdir) == NULL)
			dp = rootdir;
		VREF(dp);
		vmountread(dp);
		hadslash = 1;
	}
	ndp->ni_endoff = 0;

	/*
	 * We come to dirloop to search a new directory.
	 */
dirloop:
	/*
	 * Copy next component of name to ndp->ni_dent.
	 * XXX kern_exec looks at d_name
	 * ??? The ni_hash value may be useful for vfs_cache
	 * XXX There must be the last component of the filename left
	 * somewhere accessible via. ndp for NFS (and any other stateless file
	 * systems) in case they are doing a CREATE. The "Towards a..." noted
	 * that ni_ptr would be left pointing to the last component, but since
	 * the ni_pnbuf gets free'd, that is not a good idea.
	 */
	if (getbuf) {
		ndp->ni_hash = 0;
		for (cp = ndp->ni_ptr, i = 0; *cp != 0 && *cp != '/'; cp++) {
			if (i >= NAME_MAX) {
				error = ENAMETOOLONG;
				goto bad;
			}
			ndp->ni_dent.d_name[i++] = *cp;
			ndp->ni_hash += (unsigned char)*cp * i;
		}
		ndp->ni_namelen = i;
		ndp->ni_dent.d_namlen = i;
		ndp->ni_dent.d_name[i] = '\0';
		ndp->ni_pathlen -= i;
		ndp->ni_next = cp;
#ifdef NAMEI_DIAGNOSTIC
		printf("{%s}: ", ndp->ni_dent.d_name);
#endif
	}
	cp = ndp->ni_next;
	ndp->ni_makeentry = 1;
	if (*cp == '\0' && docache == 0)
		ndp->ni_makeentry = 0;
	ndp->ni_isdotdot = (ndp->ni_namelen == 2 &&
		ndp->ni_dent.d_name[1] == '.' && ndp->ni_dent.d_name[0] == '.');

	/*
	 * Check for degenerate name (e.g. / or "").  POSIX demands that
	 * a path of "" generate an error of ENOENT.  Any leading "/"
	 * characters have been removed, and remembered by the
	 * hadslash variable.
	 */
	if (ndp->ni_ptr[0] == '\0') {
		if (hadslash == 0 && !nullcompat) {
			error = ENOENT;
			goto bad;
		}
		/*
		 * Must have been "/" or trailing "/", which 
		 * is only valid for directories.
		 * Note: ni_dvp may be referenced (if wantparent).
		 */
		BM(VN_LOCK(dp));
		if (dp->v_type != VDIR) {
			error = ENOTDIR;
			BM(VN_UNLOCK(dp));
			goto bad;
		}
		BM(VN_UNLOCK(dp));
		if ((ndp->ni_dvp == NULLVP) && wantparent) {
			/* Had to be "/" -- let caller clean up */
			ASSERT((dp == rootdir) || (dp == ndp->ni_rdir));
			ndp->ni_dvp = dp;
			VREF(dp);
		}
		ndp->ni_vp = dp;
		goto done;
	}

#if	SEC_MAC
checkdotdot:
#endif
	/*
	 * Handle "..": two special cases.
	 * 1. If at root directory (e.g. after chroot)
	 *    then ignore it so can't get out.
	 * 2. If this vnode is the root of a mounted
	 *    file system, then replace it with the
	 *    vnode which was mounted on so we take the
	 *    .. in the other file system.
	 */
	if (ndp->ni_isdotdot) {
		for (;;) {
			if (dp == ndp->ni_rdir || dp == rootdir) {
				ndp->ni_dvp = dp;
				ndp->ni_vp = dp;
				VREF(dp);
				goto nextname;
			}
			BM(VN_LOCK(dp));
			vflag = dp->v_flag;
			BM(VN_UNLOCK(dp));
			if ((vflag & VROOT) == 0 ||
			    (ndp->ni_nameiop & NOCROSSMOUNT))
				break;
			tdp = dp;
			dp = dp->v_mount->m_vnodecovered;
			VREF(dp);
			/*
			 * We are crossing a mount point, going up.
			 * Release mount lookup lock on current mountpoint
			 * and acquire the new mountpoint's lookup lock.
			 * After acquiring the new lookup lock, we can release
			 * our reference on the root vnode of the previous
			 * filesystem.  By releasing the previous lookup lock
			 * first we avoid deadlock with an unmount coming down.
			 */
			MOUNT_LOOKUP_DONE(tdp->v_mount);
			/*
			 * This may need to change if we support forcible
			 * unmount - XXX
			 */
			vmountread(dp);
			vrele(tdp);
		}
	}

	/*
	 * We now have a segment name to search for, and a directory to search.
	 */
#if defined(DCEDFS) && DCEDFS
	if (ndp->ni_nameiop & SPECLOOKUP)
	  error = (*ndp->ni_lookup)(dp, ndp);
	else
#endif  /* DCEDFS */
	VOP_LOOKUP(dp, ndp, error);
	if (error) {
		if (ndp->ni_vp != NULL)
			panic("leaf should be empty");
#ifdef NAMEI_DIAGNOSTIC
		printf("not found\n");
#endif
		if (flag == LOOKUP || flag == DELETE ||
		    error != ENOENT || *cp != 0)
			goto bad;
		/*
		 * If creating and at end of pathname, then can consider
		 * allowing file to be created.
		 */
		BM(VN_LOCK(ndp->ni_dvp));
		temp_mp = ndp->ni_dvp->v_mount;
		BM(VN_UNLOCK(ndp->ni_dvp));
		BM(MOUNT_LOCK(mp));
		mflag = temp_mp->m_flag;
		BM(MOUNT_UNLOCK(mp));
		if (mflag & rdonly) {
			error = EROFS;
			goto bad;
		}
		/*
		 * We return with ni_vp NULL to indicate that the entry
		 * doesn't currently exist, leaving a pointer to the
		 * (possibly locked) directory inode in ndp->ni_dvp.
		 */
		if (getbuf)
			PN_DEALLOCATE(ndp->ni_pnbuf);
		MOUNT_LOOKUP_DONE(dp->v_mount);
		return (0);	/* should this be ENOENT? */
	}
#ifdef NAMEI_DIAGNOSTIC
	printf("found\n");
#endif

	/*
	 * Check for symbolic link
	 */
	dp = ndp->ni_vp;
	ASSERT(dp != NULLVP && dp->v_mount != NULLMOUNT);
	VN_LOCK(dp);	

	/*
	 * If the current vnode is a directory and it is legal to
	 * cross mountpoints, repeatedly find the root of each
	 * filesystem mounted here.
	 *
	 * 1.	If current vnode is the bypass vnode, don't cross
	 *	into new filesystem.  Filesystem-specific mount routines
	 *	use the bypass vnode to avoid deadlock when calling namei
	 *	in case the pathname runs through the vnode being mounted on.
	 * 2.	Wait for mount/unmount attempts to finish on the
	 *	new filesystem
	 * 3.	If there is no filesystem mounted here, we are done.
	 * 4.	Acquire the new filesystem's mount lookup lock for
	 *	reading, to synchronize with unmount.
	 * 5.	Retrieve the new filesystem's root vnode.
	 * 6.	It's now safe to release the old filesystem's lookup lock.
	 * 7.	Repeat above until we've run out of mounted filesystems.
	 */
	while (((dp->v_type == VDIR || dp->v_type == VREG)
		&& (ndp->ni_nameiop & NOCROSSMOUNT) == 0)) {
			if (dp == bypassvp)
				break;
			vmountwait(dp);
			if ((mp = dp->v_mountedhere) == NULLMOUNT)
				break;
			MOUNT_LOOKUP(mp);
			VN_UNLOCK(dp);
			VFS_ROOT(mp, &tdp, error);
			if (error) {
				MOUNT_LOOKUP_DONE(mp);
				goto bad2;
			}
			ASSERT(tdp != NULLVP && tdp->v_mount != NULLMOUNT);
			MOUNT_LOOKUP_DONE(dp->v_mount);
			vrele(dp);
			ndp->ni_vp = dp = tdp;
			VN_LOCK(dp);
		}

	if ((dp->v_type == VLNK) &&
	    ((ndp->ni_nameiop & FOLLOW) || *ndp->ni_next == '/')) {
		struct iovec aiov;
		struct uio auio;
		int linklen;

		VN_UNLOCK(dp);
		if (!getbuf)
			panic("namei: unexpected symlink");
		if (++ndp->ni_loopcnt > MAXSYMLINKS) {
			error = ELOOP;
			goto bad2;
		}
		if (ndp->ni_pathlen > 1)
			PN_ALLOCATE(cp);
		else
			cp = ndp->ni_pnbuf;
		aiov.iov_base = cp;
		aiov.iov_len = MAXPATHLEN;
		auio.uio_iov = &aiov;
		auio.uio_iovcnt = 1;
		auio.uio_offset = 0;
		auio.uio_rw = UIO_READ;
		auio.uio_segflg = UIO_SYSSPACE;
		auio.uio_resid = MAXPATHLEN;
		VOP_READLINK(dp, &auio, ndp->ni_cred, error);
		if (error) {
			if (ndp->ni_pathlen > 1)
				PN_DEALLOCATE(cp);
			goto bad2;
		}
		linklen = MAXPATHLEN - auio.uio_resid;
		if (linklen + ndp->ni_pathlen >= MAXPATHLEN) {
			if (ndp->ni_pathlen > 1)
				PN_DEALLOCATE(cp);
			error = ENAMETOOLONG;
			goto bad2;
		}
		if (ndp->ni_pathlen > 1) {
			bcopy(ndp->ni_next, cp + linklen, ndp->ni_pathlen);
			PN_DEALLOCATE(ndp->ni_pnbuf);
			ndp->ni_pnbuf = cp;
		} else
			ndp->ni_pnbuf[linklen] = '\0';
		ndp->ni_ptr = cp;
		/*
		 * If we've crossed just crossed a mount pointed and
		 * we're looking at a symlink (automount) then unlock
		 * the symlink's mount structure and relock the starting
		 * dp's mount structure before continuing with the string
		 * from the symlink.
		 */
		if (dp->v_mount != ndp->ni_dvp->v_mount) {
			MOUNT_LOOKUP_DONE(dp->v_mount);
			vmountread(ndp->ni_dvp);
		}
		vrele(dp);
		dp = ndp->ni_dvp;
		ndp->ni_pathlen += linklen;
		goto start;
	}
	/*
	 * It is okay to do this unlock even though dp may have changed during
	 * the while loop above.  If we skipped over the while loop, dp has
	 * not changed.  If we went through the while loop, we unlocked the old
	 * dp, and now have the new dp locked, so we have to unlock that.
	 */
	VN_UNLOCK(dp);

#if	SEC_MAC
	switch (mld_traverse(ndp, &error)) {
	    case SEC_MLD_DIVERT:
		/*
		 * The vnode found by the previous VOP_LOOKUP is a multilevel
		 * directory. ndp->ni_dvp now points at the mld, possibly
		 * locked, and ndp->ni_vp points at the locked subdirectory
		 * corresponding to our sensitivity level.
		 */
		dp = ndp->ni_vp;
		break;
	    case SEC_MLD_PASS:
		/*
		 * Either the vnode just found is not a multilevel directory
		 * or the MULTILEVELDIR privilege is in effect.
		 */
		break;
	    case SEC_MLD_DOTDOT:
		/*
		 * The vnode just found is a multilevel directory and the
		 * component name is "..", meaning that we are backing up
		 * the tree from a mld subdirectory.  Repeat the lookup,
		 * this time in the mld itself.
		 */
		vrele(ndp->ni_dvp);
		goto checkdotdot;
	    case SEC_MLD_ERROR:
		/*
		 * We should have diverted to a subdirectory but were
		 * unable to create it. ni_dvp has been vrele'd.
		 */
		goto bad;
	    default:
		panic("namei: unexpected return from mld_traverse");
		goto bad2;
	}
#endif	/* SEC_MAC */

nextname:
	/*
	 * Not a symbolic link.  If more pathname,
	 * continue at next component, else return.
	 */
	ndp->ni_ptr = ndp->ni_next;
	if (*ndp->ni_ptr == '/') {
		while (*ndp->ni_ptr == '/') {
			ndp->ni_ptr++;
			ndp->ni_pathlen--;
		}
		/*
		 * Trailing / is allowed for directories, so, if we've got
		 * a directory, wantparent is true, and there's no pathname
		 * left, we don't want to release the
		 * parent directory.  We know which path will be taken
		 * from dirloop based on this information.
		 */
		BM(VN_LOCK(dp));
		if ((*ndp->ni_ptr != '\0') || (!wantparent) ||
		    (dp->v_type != VDIR)) {
			BM(VN_UNLOCK(dp));
			vrele(ndp->ni_dvp);
		} else
			BM(VN_UNLOCK(dp));
		hadslash = 1;
		goto dirloop;
	}
	ASSERT(dp != NULLVP && dp->v_mount != NULLMOUNT);
	/*
	 * Check for read-only file systems.
	 */
	if (flag == DELETE || flag == RENAME) {
		int dpflag, dvpflag; 
		/*
		 * Disallow directory write attempts on read-only
		 * file systems.
		 */
		BM(MOUNT_LOCK(dp->v_mount));
		dpflag = dp->v_mount->m_flag;
		BM(MOUNT_UNLOCK(dp->v_mount));
		BM(MOUNT_LOCK(ndp->ni_dvp->v_mount));
		dvpflag = ndp->ni_dvp->v_mount->m_flag;
		BM(MOUNT_UNLOCK(ndp->ni_dvp->v_mount));
		if ((dpflag & rdonly) ||
		    (wantparent && (dvpflag & rdonly))){
			error = EROFS;
			goto bad2;
		}
	}
	ASSERT(dp != NULLVP && dp->v_mount != NULLMOUNT);
	if (!wantparent)
		vrele(ndp->ni_dvp);

done:
	/* store security relevant info on auditable operations */
	if ( DO_AUDIT(u.u_event,error) && ndp && u.u_vno_indx < AUD_VNOMAX ) {
		if ( (vp = ndp->ni_vp) && vn_stat ( vp, &sb ) == 0 ) {
			u.u_vno_dev[u.u_vno_indx]    = S_ISCHR(sb.st_mode) || S_ISBLK(sb.st_mode) ?
                            sb.st_rdev : sb.st_dev;
			u.u_vno_num[u.u_vno_indx]    = sb.st_ino;
#ifdef notdef
			u.u_vno_aud[u.u_vno_indx]    = sb.st_flags&INOAUDIT ? '1' : '\0';
#endif notdef
			u.u_vno_mode[u.u_vno_indx++] = sb.st_mode;
		}
	}

	if (getbuf)
		PN_DEALLOCATE(ndp->ni_pnbuf);
	MOUNT_LOOKUP_DONE(dp->v_mount);
	return (0);

bad2:
	vrele(ndp->ni_dvp);
bad:
	MOUNT_LOOKUP_DONE(dp->v_mount);
	vrele(dp);
	ndp->ni_vp = NULL;
	if (getbuf)
		PN_DEALLOCATE(ndp->ni_pnbuf);
	return (error);
}
