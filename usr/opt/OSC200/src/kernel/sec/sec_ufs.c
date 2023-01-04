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
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/* @(#)sec_ufs.c	6.2 18:08:32 1/31/91 SecureWare */
/*
 * Copyright (c) 1990 SecureWare, Inc.  All Rights Reserved.
 *
 * Security function for the UFS filesystem type for the OSF/1 kernel.
 */

#include <sec/include_sec>

#if SEC_FSCHANGE
#include <sys/sec_objects.h>

/*
 * Define the structure pointed to by vp->v_secop for ufs file systems.
 * Currently, this is the only file system type that supports secure ops.
 * Callers of these VOP_* functions must be sure that the vnode in question
 * is part of a secure file system, else vp->v_secop will be NULL.
 * This is typically done by calling VHASSECOPS(vp) or
 * is_secure_filesystem(vp).
 */

int	ufs_getsecattr(), ufs_setsecattr(), ufs_dirempty();


struct vnsecops	ufs_vnsecops = {
	ufs_getsecattr,		/* VOP_GETSECATTR function */
	ufs_setsecattr,		/* VOP_SETSECATTR function */
	ufs_dirempty		/* VOP_DIREMPTY   function */
};

/*
 * ufs_getsecattr (typically called by VOP_GETSECATTR)
 *
 * Retrieve selected security attributes from the specified file.
 * No access checking is performed; that is assumed to have been
 * done by the caller.
 * XXX Need to fix races between ufs_getsecattr() and ufs_setsecattr()
 * when caller can't hold vnode lock across both calls
 */

ufs_getsecattr(vp, vsap, cred)
	register struct vnode		*vp;
	register struct vsecattr	*vsap;
	struct ucred			*cred;
{
	register struct inode	*ip = VTOI(vp);
	register int		valid = vsap->vsa_valid;
	register int		i;

	if (valid & VSA_GPRIV)
		for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i)
			vsap->vsa_gpriv[i] = ip->i_gpriv[i];

	if (valid & VSA_PPRIV)
		for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i)
			vsap->vsa_ppriv[i] = ip->i_ppriv[i];
	
	if (valid & VSA_TYPE_FLAGS)
		vsap->vsa_type_flags = ip->i_type_flags;
	
	if (valid & VSA_TAG) {
		i = OBJECT_TAG(vsap->vsa_policy, vsap->vsa_tagnum);
		vsap->vsa_tag = ip->i_tag[i];
	} else if (valid & VSA_ALLTAGS)
		bcopy(ip->i_tag, vsap->vsa_tags, sizeof ip->i_tag);

	return 0;
}

/*
 * ufs_setsecattr (typically called by VOP_SETSECATTR)
 *
 * Change selected security attributes of the specified file.
 * The vnode must be locked by the caller.
 * XXX Possible races between get and set without vnode lock
 * XXX fix inode locking here
 */
ufs_setsecattr(vp, vsap, cred)
	register struct vnode		*vp;
	register struct vsecattr	*vsap;
	struct ucred			*cred;
{
	register struct inode	*ip = VTOI(vp);
	register int		i;
	register int		valid = vsap->vsa_valid;
	int			error = 0;

	/*   
	 * If setting the granted privilege set, make sure that the
	 * new value does not exceed the potential privilege set.
	 * If the potential set is also being changed, make the check
	 * against the new value for P, otherwise against the current
	 * value.
	 */

	if (valid & VSA_GPRIV) {
		register priv_t *ppriv;
		
		ppriv = (valid & VSA_PPRIV) ? vsap->vsa_ppriv : ip->i_ppriv;
		for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i)
			if (vsap->vsa_gpriv[i] & ~ppriv[i]) {
				return EPERM;
			}	
	}

	if (valid & (VSA_GPRIV|VSA_PPRIV|VSA_TYPE_FLAGS))
		ip->i_flag |= ICHG;

	if (valid & VSA_GPRIV)
		for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i)
			ip->i_gpriv[i] = vsap->vsa_gpriv[i];

	if (valid & VSA_PPRIV)
		/* Reduce G so that it doesn't exceed P */
		for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
			ip->i_gpriv[i] &= vsap->vsa_ppriv[i];
			ip->i_ppriv[i] = vsap->vsa_ppriv[i];
		}

	if (valid & VSA_TYPE_FLAGS)
		ip->i_type_flags = vsap->vsa_type_flags;

	if (valid & VSA_TAG) {

		if (!(error = SP_SETATTR_CHECK(vsap->vsa_policy, OT_REGULAR, vp,
					vsap->vsa_parent, ip->i_tag,
					vsap->vsa_tagnum, vsap->vsa_tag)) &&
		    !(error = ufs_fsync(vp, FWRITE, cred, MNT_WAIT)) &&
		    !(error = SP_SETATTR(vsap->vsa_policy, SEC_OBJECT,
					 ip->i_tag, vsap->vsa_tagnum, vsap->vsa_tag)))
		{
			/*
			 * POSIX ACLS --
                         * user, group, mode change may be a byproduct
                         * of a mode change
                         */

                        if (valid & VSA_UID)
                                ip->i_uid = vsap->vsa_uid;
                        if (valid & VSA_GID)
                                ip->i_gid = vsap->vsa_gid;
                        if (valid & VSA_MODE)
                                ip->i_mode =
                                  (ip->i_mode & ~0777) | vsap->vsa_mode;
			ip->i_flag |= ICHG;
		}
	}

	return error;
}

/*
 * Change the setting of the multilevel directory bit of a file.
 * It is critical that this code be run with the inode locked!
 */

ufs_chmultdir(vp, dvp, on_or_off, cred)
	register struct vnode		*vp;
	register struct vnode		*dvp;
	struct ucred			*cred;
{
	register struct inode	*ip = VTOI(vp);
	int error = 0;

	on_or_off = on_or_off ? SEC_I_MLD : 0;
	IN_READ_LOCK(ip);

	/* If this is an MLDCHILD, changing SEC_I_MLD isn't rational. */

	if (ip->i_type_flags & SEC_I_MLDCHILD) {
		error = EINVAL;
		goto out;
	}

	/* This call makes no sense on anything but a directory */

	if (!IS_DIRECTORY(vp)) {
		error = ENOTDIR;
		goto out;
	}

	/* The directory must be empty. */

	if (!dirempty(ip, VTOI(dvp)->i_number, cred)) {
		error = ENOTEMPTY;
		goto out;
	}

	/*
	 * Old code considered it an error to set the MLD bit to its current
	 * setting.  This seems overly harsh, but for now it stays.
	 */

	if ((ip->i_type_flags & SEC_I_MLD) == on_or_off) {
		error = EACCES;
		goto out;
	}

	/* Toggle the bit in ip and vp, and mark the inode as changed. */
	
	IN_LOCK(ip);
	ip->i_type_flags ^= SEC_I_MLD;
	ip->i_flag |= ICHG;
	IN_UNLOCK(ip);

	VN_LOCK(vp);
		if (on_or_off)
			vp->v_flag |= VMLD;
		else	vp->v_flag &=~VMLD;
	VN_UNLOCK(vp);

out:	IN_READ_UNLOCK(ip);
	return(error);
}


/*
 * ufs_dirempty (typically called by VOP_DIREMPTY)
 *
 * Determine whether or not a directory is empty. Return non-zero if
 * empty, zero if not. Vnode of directory to be tested must be locked.
 * XXX Possible races here when directory lock not held across call.
 */

ufs_dirempty(vp, dvp, cred)
	struct vnode	*vp;	/* directory to be tested */
	struct vnode	*dvp;	/* parent of directory to be tested */
	struct ucred	*cred;	/* caller's credentials */
{
	int is_empty;

	/*
	 * dirempty assumes that the inode is read locked.
	 */
	IN_READ_LOCK(VTOI(vp));
	is_empty = dirempty(VTOI(vp), VTOI(dvp)->i_number, cred);
	IN_READ_UNLOCK(VTOI(vp));
	return is_empty;
}

/*
 * Make sure that if the filesystem being mounted is labeled there are
 * no global tags associated with it, and that if it is unlabeled there
 * are global tags associated with it.  Also verify the relationship
 * between the mount point and the mounted root.
 */

sec_ufsmountcheck(mp)
	struct mount	*mp;
{
	int		error;
	struct vnode	*rvp;
	struct vsecattr	vsattr;

	if (FsSEC(VFSTOUFS(mp)->um_fs)) {
		/*
		 * The filesystem has labels; make sure it is not
		 * mounted with global labels.
		 */
		if ((mp->m_flag & M_SECURE) == 0) {
			return EINVAL;
		}

		/*
		 * If we are mounting the root filesystem, there is
		 * no mount point against which to check access.
		 */
		if (mp->m_vnodecovered == NULL)
			return 0;

	} else {

#define INSECURE_ROOT
#ifndef INSECURE_ROOT
		/*
		 * The root filesystem must be labeled.
		 */
		if (mp->m_vnodecovered == NULL) {
			printf("Root filesystem is unlabeled.");
			return EINVAL;
		}
#else /* INSECURE_ROOT */
		/*
		 * To facilitate initial bootstrapping of a new secure system,
		 * we force an unlabeled root filesystem to have a usable set
		 * of global labels.
		 */
		if (mp->m_vnodecovered == 0) {
	
			bzero(mp->m_tag, sizeof mp->m_tag);
	
			mp->m_flag &= ~M_SECURE;
			sec_switch_security(0); /* turn security off */
			return 0;
		}
#endif /* INSECURE_ROOT */

		/*
		 * The mounted filesystem does not have labels.  If no global
		 * labels have been supplied (by the lmount syscall), disallow
		 * the mount.
		 */
		if (mp->m_flag & M_SECURE) {
			return EINVAL;
		}
	}

	/*
	 * Retrieve the tags associated with the mount point.
	 */
	if (VHASSECOPS(mp->m_vnodecovered)) {
		vsattr.vsa_valid = VSA_ALLTAGS | VSA_TYPE_FLAGS;
		if (VOP_GETSECATTR(mp->m_vnodecovered, &vsattr, NULL, error))
			return error;
		
		/*
		 * Examine the type flags field of the mount point to
		 * determine if it is a multilevel diversion directory
		 * (MLD child).
		 * Mounts are not permitted on these directory types.
		 */
		if (vsattr.vsa_type_flags & SEC_I_MLDCHILD) {
			return EINVAL;
		}
		
	} else {
		/*
		 * mount point isn't on extended FS; get tags
		 * specified when it was mounted.
		 */
		bcopy(mp->m_vnodecovered->v_mount->m_tag, vsattr.vsa_tags,
			sizeof(vsattr.vsa_tags));
	}

	/*
	 * Get the root of the mounted filesystem.
	 */

	VFS_ROOT(mp, &rvp, error);

	if (!error) {
		/*
		 * Perform the access check.
		 */
		if (SP_ACCESS(vsattr.vsa_tags, VTOI(rvp)->i_tag,
			      SP_MOUNTACC, NULL)) {
			error = EACCES;
		}
		vrele(rvp);
	}

	return (error);

}

/*
 * Upon a fresh allocation of a new i-node, make sure any fields related to
 * a secure i-node are cleared, or residue will take the place of valid data.
 * (Solves an object reuse condition.)  This is called only from ialloc().
 * Note that we don't need to do inode locking here, since nobody knows
 * about this inode yet.  (See comments in ialloc().)
 */
void
sec_ialloc(ip)
	register struct inode *ip;
{
	register int scan;

	for (scan = 0; scan < SEC_SPRIVVEC_SIZE; scan++)  {
		ip->i_gpriv[scan] = (priv_t) 0;
		ip->i_ppriv[scan] = (priv_t) 0;
	}

	/*
	 * POSIX ACLS -- removed SP_OBJECT_CREATE
	 */

	ip->i_parent = 0;
	ip->i_type_flags = (ushort) 0;
}

#endif /* SEC_FSCHANGE */
/*
 * sec_ialloc1()
 *
 * Copy tags or create security policy objects.  If flag is TRUE,
 * copy dp's tags to ip.  Otherwise, if this inode's filesystem is
 * secure, use SP_OBJECT_CREATE() to create/copy the tags.  Finally,
 * if this inode's filesystem isn't secure, copy the mount point's
 * tags to ip.
 *
 * This routine is outside the SEC_FSCHANGE conditional since it is
 * called whether SEC_FSCHANGE is on or not.
 *
 * NOTE: If SEC_FSCHANGE is not on, the i_tag array will not exist, so
 * this function would not compile (this isn't the only place this
 * happens).
 *
 * Locking: The inode's incore lock is locked upon entry and exit.
 */
void
sec_ialloc1(
	struct inode *ip, 
	struct nameidata *ndp, 
	struct inode *dp,
	boolean_t flag)
{

	LASSERT(SLOCK_HOLDER(&ip->i_incore_lock));

	if (flag) {
		IN_LOCK(dp);
		bcopy(dp->i_tag, ip->i_tag, SEC_NUM_TAGS * sizeof(tag_t));
		IN_UNLOCK(dp);
	} else if (FsSEC(ip->i_fs)) {
		tag_t tags[SEC_NUM_TAGS];
                dac_t	dac;
                int	mapflags;

		IN_LOCK(dp);
		bcopy(dp->i_tag, tags, sizeof(tags));
		IN_UNLOCK(dp);
                /* pass dac structure and umask to object creation macro */
                dac.uid = ip->i_uid;
                dac.gid = ip->i_gid;
                dac.mode = ip->i_mode;
		/*
		 * SP_OBJECT_CREATE may sleep, so we unlock the inode
		 * lock.  It's state is consistent, so this is safe.
		 * The only way it can be found is via ufs_sync(), which
		 * will be a nop for an inode in this state.
		 * The i_tag array may change, but nobody can be looking
		 * at it at this time.
		 */
		IN_UNLOCK(ip);
                mapflags = SP_OBJECT_CREATE(SIP->si_tag, ip->i_tag,
					    tags, SEC_OBJECT,
					    &dac, 0);

		IN_LOCK(ip);
                /* if policy requires change to inode, reflect that as well */
                if (mapflags & SEC_NEW_UID)
                        ip->i_uid = dac.uid;
                if (mapflags & SEC_NEW_GID)
                        ip->i_gid = dac.gid;
                if (mapflags & SEC_NEW_MODE)
                        ip->i_mode = (ip->i_mode & ~0777) | (dac.mode & 0777);
	} else {
		/* m_tag array in mount structure is immutable */
		bcopy(ITOV(ip)->v_mount->m_tag, ip->i_tag,
		      SEC_NUM_TAGS * sizeof(tag_t));
	}
}
