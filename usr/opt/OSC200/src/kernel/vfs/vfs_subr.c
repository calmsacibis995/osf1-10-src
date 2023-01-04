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
static char	*sccsid = "@(#)$RCSfile: vfs_subr.c,v $ $Revision: 4.3.23.7 $ (DEC) $Date: 1993/11/23 21:36:44 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
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
 *	@(#)vfs_subr.c	7.22 (Berkeley) 12/31/89
 */

/*
 * External virtual filesystem routines
 */

#include <sys/secdefines.h>

#include <rt_preempt.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/time.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <sys/namei.h>
#include <sys/ucred.h>
#include <sys/errno.h>
#include <sys/lock_types.h>
#if	MACH
#include <mach/vm_param.h>
#include <kern/zalloc.h>
#include <vm/vm_object.h>
#include <sys/vfs_proto.h>

/*
 * Zones used by the VFS code for dynamic data structures
 */
zone_t	pathname_zone;
zone_t	mount_zone;

#else	/* MACH */
#include <sys/malloc.h>
#endif	/* MACH */

#include <dcedfs.h>

/*
 * XXX Get rid of this when assertion in vgone goes away
 */
extern struct vnode *rootvp;
extern struct vnode *rootdir;

extern int mount_maxtype;

/*
 * Important global vnode table variables.  These variables are
 * initialized in machine-dependent code.
 */
struct vnode	*vnode;			/* base of table */
struct vnode	*vnodeNVNODE;		/* end of table */
int		nvnode;			/* number of vnodes in table */
int		vn_maxprivate;		/* max size of fs-specific info. */

/*
 * The vnode free list lock, vn_free_lock, is only used in this file.
 */
udecl_simple_lock_data(,vn_free_lock)

#define	VN_FREE_LOCK()		usimple_lock(&vn_free_lock)
#define	VN_FREE_UNLOCK()	usimple_unlock(&vn_free_lock)
#define	VN_FREE_LOCK_INIT()	usimple_lock_init(&vn_free_lock)

/*
 * Lock for the vfssw to synchronize addition and deletion of filesystems.
 * This lock must be taken for reading during mount and unmount operations,
 * and taken for writing when adding or deleting filesystems.
 */
lock_data_t	vfssw_lock;

/*
 * Lock for linked list of mounted file systems.
 */
udecl_simple_lock_data(,mountlist_lock)

void vclean();

/*
 * Arg passed to wait_for_vxlock to indicate whether or not to set VXLOCK
 */
#define VX_NOLOCK	0
#define VX_LOCK		1

/*
 * clear the VXLOCK flag and wakeup sleepers.
 */
void
clear_vxlock(vp)
	register struct vnode *vp;
{
	VN_LOCK(vp);
	ASSERT(vp->v_flag & VXLOCK);
	vp->v_flag &= ~VXLOCK;
	if (vp->v_flag & VXWANT) {
		vp->v_flag &= ~VXWANT;
		VN_UNLOCK(vp);
		thread_wakeup((vm_offset_t)&vp->v_flag);
	} else
		VN_UNLOCK(vp);
}

/*
 * look at VXLOCK and block on VXWAIT
 * Takes locked vp, and it return locked.
 */
wait_for_vxlock(vp, lock)
	register struct vnode *vp;
	int lock;
{
	int slept = 0;

	LASSERT(SLOCK_HOLDER(&vp->v_lock));
	while (vp->v_flag & VXLOCK) {
		vp->v_flag |= VXWANT;
		assert_wait((vm_offset_t)&vp->v_flag, FALSE);
		VN_UNLOCK(vp);
		thread_block();
		VN_LOCK(vp);
		slept = 1;
	}
	if ((slept == 0) && (lock == VX_LOCK))
		vp->v_flag |= VXLOCK;
	return (slept);
}


/*
 * Lookup a mount point by filesystem identifier.
 *
 * getvfs returns a mount point locked for reading in the
 * normal case.  This is done so that the caller has some
 * guarantee that the filesystem will not disappear during
 * the course of subsequent operations.  The caller must
 * dispose of the mount structure by calling MOUNT_LOOKUP_DONE.
 * Without this guarantee, it would be possible for the filesystem
 * to become unmounted at almost any time while the caller does
 * its business!
 *
 * Unfortunately, while finding the required filesystem is easy,
 * locking it is not.  We can not contend for the mount lookup
 * lock while holding the mount list lock; but releasing the mount
 * list lock and then acquiring the mount structure lookup lock
 * introduces a race whereby the mount structure could be unmounted
 * and re-allocated before we succeed in acquiring the lookup lock.
 *
 * It is also highly desireable that getvfs operations pend during
 * unmount attempts on a filesystem; otherwise, races can be introduced
 * whereby unmount will succeed and leave vnodes in core (very bad)
 * or unmount will fail because of a file handle translation (not too bad).
 *
 * The upshot is that getvfs must do the equivalent of a pathname
 * translation; it must find the filesystem's covered vnode, wait for
 * VMOUNTING to clear on that vnode, and only then may it acquire the
 * mount structure's lookup lock.
 */
struct mount *
getvfs(fsid)
	fsid_t *fsid;
{
	register struct mount *mp;
	register struct vnode *cvp;

	MOUNTLIST_LOCK();
	mp = rootfs;
	do {
		if (mp->m_stat.f_fsid.val[0] == fsid->val[0] &&
		    mp->m_stat.f_fsid.val[1] == fsid->val[1]) {
			if ((cvp = mp->m_vnodecovered) == NULLVP) {
				ASSERT(mp == rootfs);
				/*
				 * We can cheat here because we know
				 * that the root filesystem can not
				 * become unmounted.
				 */
				MOUNTLIST_UNLOCK();
				MOUNT_LOOKUP_START(mp);
				return(mp);
			}
			VREF(cvp);
			MOUNTLIST_UNLOCK();
			VN_LOCK(cvp);
			vmountwait(cvp);
			mp = cvp->v_mountedhere;
			if (mp != NULLMOUNT) {
				MOUNT_LOOKUP(mp); /* forcible unmount?  XXX */
				VN_UNLOCK(cvp);
				vrele(cvp);
				if (mp->m_stat.f_fsid.val[0] == fsid->val[0] &&
				    mp->m_stat.f_fsid.val[1] == fsid->val[1])
					return(mp);
				else
					MOUNT_LOOKUP_DONE(mp);
			}
			VN_UNLOCK(cvp);
			vrele(cvp);
			return (NULLMOUNT);
		}
		mp = mp->m_next;
	} while (mp != rootfs);
	MOUNTLIST_UNLOCK();
	return (NULLMOUNT);
}

/*
 * Set vnode attributes to VNOVAL
 *
 * MP: assume that vap is locked, if necessary.
 */
#undef vattr_null
void
vattr_null(vap)
	register struct vattr *vap;
{

	vap->va_type = VNON;
	vap->va_symlink = (char *) 0;
#if __alpha
        vap->va_mode = (u_short)VNOVAL;
        vap->va_nlink = (short)VNOVAL;
        vap->va_uid = (uid_t)VNOVAL;
        vap->va_gid = (gid_t)VNOVAL;
        vap->va_fsid = (int)VNOVAL;
        vap->va_fileid = (int)VNOVAL;
        vap->va_size = VNOVAL;
        vap->va_blocksize = (int)VNOVAL;
        vap->va_rdev = (dev_t)VNOVAL;
        vap->va_bytes = VNOVAL;
        vap->va_atime.tv_sec = (int)VNOVAL;
        vap->va_atime.tv_usec = (int)VNOVAL;
        vap->va_mtime.tv_sec = (int)VNOVAL;
        vap->va_mtime.tv_usec = (int)VNOVAL;
        vap->va_ctime.tv_sec = (int)VNOVAL;
        vap->va_ctime.tv_usec = (int)VNOVAL;
        vap->va_flags = (u_int)VNOVAL;
        vap->va_gen = (u_int)VNOVAL;
#else
	vap->va_mode = vap->va_nlink = vap->va_uid = vap->va_gid =
		vap->va_fsid = vap->va_fileid = vap->va_size =
		vap->va_size_rsv = vap->va_blocksize = vap->va_rdev =
		vap->va_bytes = vap->va_bytes_rsv =
		vap->va_atime.tv_sec = vap->va_atime.tv_usec =
		vap->va_mtime.tv_sec = vap->va_mtime.tv_usec =
		vap->va_ctime.tv_sec = vap->va_ctime.tv_usec =
		vap->va_flags = vap->va_gen = VNOVAL;
#endif
}

/*
 * Initialize a nameidata structure
 */
#if	MACH
ndinit(ndp, utnd)
#else
ndinit(ndp)
#endif
	register struct nameidata *ndp;
#if	MACH
	register struct utask_nd *utnd;
#endif
{

	bzero((caddr_t)ndp, sizeof(struct nameidata));
	ndp->ni_iov = &ndp->ni_nd.nd_iovec;
	ndp->ni_iovcnt = 1;
	ndp->ni_base = (caddr_t)&ndp->ni_dent;
	ndp->ni_rw = UIO_WRITE;
	ndp->ni_segflg = UIO_SYSSPACE;
	/*
	 * Default the segflg of the embedded uio structure to UIO_SYSSPACE.
	 * The ni_segflg is just for the pathname itself.
	 */
	ndp->ni_uioseg = UIO_SYSSPACE;
#if	MACH
	/*
	 * NOTE:
	 * In Mach, the ni_cdir, ni_rdir, and ni_cred are all in the
	 * utask structure encapsulated in a utask_nd structure.  The
	 * nameidata structure points at it.
	 * If a utask_nd ptr was passed in, use it; otherwise, use the
	 * one in the uarea.
	 * WARNING:  this routine should not be used by anyone expecting
	 * the new nameidata structure to outlast the current process.
	 * It's for temporary use of nameidata (e.g. rename) only.
	 */
	if (!utnd) 
		utnd = &(u.utask->uu_utnd); 
	ndp->ni_utnd = utnd;

#endif
}

/*
 * Duplicate a nameidata structure
 */
nddup(ndp, newndp)
	register struct nameidata *ndp, *newndp;
{

#if	MACH
	/*
	 * Callers of this routine MUST have allocated ni_utnd, and
	 * pointed the nameidata structure at it.
	 */
	ASSERT(newndp->ni_utnd != 0);
	ndinit(newndp, newndp->ni_utnd);
#else
	ndinit(newndp);
#endif
	UTND_LOCK(ndp->ni_utnd);
	newndp->ni_cdir = ndp->ni_cdir;
	VREF(newndp->ni_cdir);
	newndp->ni_rdir = ndp->ni_rdir;
	if (newndp->ni_rdir)
		VREF(newndp->ni_rdir);
	UTND_UNLOCK(ndp->ni_utnd);
	newndp->ni_cred = ndp->ni_cred;
	crhold(newndp->ni_cred);
}

/*
 * Release a nameidata structure
 */
ndrele(ndp)
	register struct nameidata *ndp;
{

	if (ndp->ni_utnd == NULL)
		panic("NULL utask_nd pointer in ndrele");
	vrele(ndp->ni_cdir);
	if (ndp->ni_rdir)
		vrele(ndp->ni_rdir);
	crfree(ndp->ni_cred);
}

/*
 * Routines having to do with the management of the vnode table.
 */
struct vnode *vfreeh, **vfreet;
extern struct vnodeops dead_vnodeops, spec_vnodeops;

#if	MACH
/*
 * These variables are all initialized in machine-dependent code.
 */
extern int	path_num_max;		/* pathname translation buffers */
/* int	ucred_max;			/* credentials */
extern int	nmount_max;		/* mount structures */
#endif

long numvnodes;
struct vattr va_null;

#define NEXT_VNODE(vp) (struct vnode *)((vm_offset_t)vp->v_data + vn_maxprivate)
/*
 * Initialize the vnode structures and initialize each file system type.
 * MP:  this routine is called at system startup with no other threads
 * active in the filesystem.
 */
vfsinit()
{
	register struct vnode *vp = vnode;
	struct vnode *lastvp;
	struct vfsops **vfsp;
	extern void spec_init();
	extern struct vfsops dead_vfsops;
	struct mount *mp;

	/*
	 * Build vnode free list.
	 */
	vfreeh = vp;
	vfreet = &vp->v_freef;
	vp->v_freeb = &vfreeh;
	vp->v_op = &dead_vnodeops;
	vp->v_mount = DEADMOUNT;
	vp->v_type = VBAD;
	vp->v_id = 0;
	vp->v_numoutput = 0;
#if	MACH
	vp->v_object= VM_OBJECT_NULL;
#endif
	numvnodes++;
	VN_LOCK_INIT(vp);
	VN_BUFLISTS_LOCK_INIT(vp);
	VN_AUX_LOCK_INIT(vp);
	for (vp = NEXT_VNODE(vp); NEXT_VNODE(vp) <= vnodeNVNODE; vp = NEXT_VNODE(vp)) {
		*vfreet = vp;
		vp->v_freeb = vfreet;
		vfreet = &vp->v_freef;
		vp->v_op = &dead_vnodeops;
		vp->v_mount = DEADMOUNT;
		vp->v_type = VBAD;
		vp->v_numoutput = 0;
#if	MACH
		vp->v_object= VM_OBJECT_NULL;
#endif
		VN_LOCK_INIT(vp);
		VN_BUFLISTS_LOCK_INIT(vp);
		VN_AUX_LOCK_INIT(vp);
		lastvp = vp;
		numvnodes++;
	}
	lastvp->v_freef = NULLVP;
	/*
	 * Initialize global file system locks.
	 */
	VN_FREE_LOCK_INIT();
	MOUNTLIST_LOCK_INIT();
	VFSSW_LOCK_INIT();
	/*
	 * Initialize data structures for handling special files
	 */
	spec_init();
	/*
	 * Initialize the vnode name cache
	 */
	nchinit();
	/*
	 * Initialize the vattr structure template
	 */
	vattr_null(&va_null);

#if	MACH
	/*
 	* Initialize virtual file system data structures, primarily zones.
 	* Initially, these will be neither exhaustible nor pageable.
 	* These attributes should be examined further.  For now, we
 	* panic if they run out, which is fine for debug, but not
 	* production.
 	* The sleepable attribute is not used at this time, so is a noop.
 	*/

	/*
	 * Pathname lookup buffers.  These are pretty transient, and
	 * VERY heavily used.
	 */
	pathname_zone = zinit(MAXPATHLEN,
			      path_num_max*MAXPATHLEN,
			      4 * PAGE_SIZE,
			      "pathbufs");
	if (pathname_zone == (zone_t) NULL)
		panic("vfsinit: no zones1");

	/*
	 * Mount structures.  These tend to stick around for a while
	 * once allocated, but that doesn't happen often.
	 */
	mount_zone = zinit(sizeof(struct mount),
			      nmount_max*sizeof(struct mount),
			      PAGE_SIZE,
			      "mounts");
	if (mount_zone == (zone_t) NULL)
		panic("vfsinit: no zones2");
	/*
	 * Initialize dead_mount structure
	 */
	mp = DEADMOUNT;
#if	SER_COMPAT || RT_PREEMPT
	mp->m_funnel = FUNNEL_NULL;
#endif
	mp->m_op = &dead_vfsops;
	/*
	 * N.B.  m_flag field is 0.  Since "anonymous" vnodes (created
	 *	 by bdevvp) will point to the dead_mount structure, these
	 *	 flags may occasionally be examined.  0 should be safe.
	 */
	MOUNT_LOOKUP_LOCK_INIT(mp);
	MOUNT_VLIST_LOCK_INIT(mp);
	MOUNT_LOCK_INIT(mp);
	/*
	 * XXX -- security defaults???
	 */
#endif	/* MACH */

	/*
	 * Initialize each file system type.
	 */
	for (vfsp = &vfssw[0]; vfsp <= &vfssw[mount_maxtype]; vfsp++) {
		if (*vfsp == NULL)
			continue;
		(*(*vfsp)->vfs_init)();
	}
#if	1
	/*
	 * Fake dynamic loading of MFS until we have the time to do
	 * MFS for real.  (We currently load NFS.) 
	 */
	{
		extern struct vfsops mfs_vfsops;

		if (vfssw_add(MOUNT_MFS, &mfs_vfsops))
			printf("Error configuring MFS\n");
	}
#endif
	
}

/*
 * Return the next vnode from the free list.
 * MP notes:
 *	-- vfreeh, vfreet, v_freef, v_freeb can only be accessed under 
 *	   vnode free list lock.
 *	Algorithm:
 *		-- lock vnode free list.
 *		-- conditionally lock free vnode, skipping locked 
 *		   and VXLOCK'd vnodes.  This avoids races with vgone.
 *		-- remove from free list and unlock freelist.
 *		-- if not already vgone'd, unlock, vgone it, lock again.
 *		-- initialize and reference the vnode.
 *		   (NB: name cache was purged during vgone (vclean)).
 *		-- unlock vnode, put it on a mount list and return.
 *		note:  it is important to atomically check VXLOCK, VBAD,
 *		       and call vgone, without releasing vnode lock, to
 *		       avoid races.
 * Note to callers:
 * This function used to do insmntque to add the new vnode to its new
 * mount list.  To avoid races, and keep caller code cleaner, we don't
 * do this, but require the callers (e.g. iget) to call insmntque when
 * vnode initialization is complete.  Upon return, the vnode is not on
 * any old lists, since vgone removes them.
 */
getnewvnode(tag, vops, vpp)
	enum vtagtype tag;
	struct vnodeops *vops;
	struct vnode **vpp;
{
	register struct vnode *vp, *vq;
	register int locked;

	VN_FREE_LOCK();	
	vp = vfreeh;
	/*
	 * Try to find an unlocked vnode with VXLOCK clear.
	 */
	while (vp) {
		if (((locked = VN_LOCK_TRY(vp)) == 0) || 
		    (vp->v_flag & VXLOCK)) {
			if (locked)
				VN_UNLOCK(vp);
			vp = vp->v_freef;
		} else {
			/*
	 		 * We have locked vnode now, without VXLOCK set.
	 		 * Remove it from freelist. 
			 */
			if (vp->v_usecount)
				panic("free vnode isn't");
			/*
			 * Note: New vnode may not be first on the list
			 */
			if (vq = vp->v_freef)
				vq->v_freeb = vp->v_freeb;
			else
				vfreet = vp->v_freeb;
			*vp->v_freeb = vq;
			vp->v_freef = NULLVP;
			vp->v_freeb = (struct vnode **) 0;
			break;
		}
	}
	if (vp == NULLVP) {
		VN_FREE_UNLOCK();
		tablefull("vnode");
		*vpp = 0;
		return (EMFILE);
	}
	numvnodes--;
	VN_FREE_UNLOCK();
	if (vp->v_type != VBAD) {
		/*
		 * VXLOCK is clear, don't sleep (doesn't really matter)
		 */
		(void) vgone(vp, VX_NOSLEEP, (struct vnodeops *) 0);
	}
	/*
	 * vnode is locked.
	 * Note:  This vnode CANNOT be found on ANY lists at this
	 *	time.  vgone makes sure of that.  There is no need
	 *	for any further locking, although we hold the vnode
	 *	lock anyway.  The "exception" is the name cache, but
	 *	only the v_id field is accessed, and we don't
	 *	change that.
	 */
	vp->v_type = VNON;
	vp->v_flag = 0;
	vp->v_shlockc = 0;
	vp->v_exlockc = 0;
	vp->v_lastr = 0;
	vp->v_socket = 0;
	vp->v_tag = tag;
	vp->v_op = vops;
	vp->v_wrcnt = 0;
	vp->v_rdcnt = 0;
#if	SEC_FSCHANGE
	vp->v_secop = 0;
#endif
	vp->v_usecount++;
	*vpp = vp;
	VN_UNLOCK(vp);
	return (0);
}

/*
 * Free an unused, but referenced vnode and put it at the front 
 * of the vnode free list (for makealias).
 * Takes a vnode with v_usecount of 1.
 */
void
vfree(vp)
	register struct vnode *vp;
{
	VN_LOCK(vp);
	ASSERT(vp->v_usecount == 1);
	--vp->v_usecount;
	VN_FREE_LOCK();
	vp->v_freef = vfreeh;
	vp->v_freeb = &vfreeh;
	if (vfreeh)
		vfreeh->v_freeb = &vp->v_freef;
	else
		vfreet = &vp->v_freef;
	vfreeh = vp;
	numvnodes++;
	VN_FREE_UNLOCK();
	VN_UNLOCK(vp);
}



/*
 * Move a vnode from one mount queue to another.
 * MP:
 *	-- VXLOCK is set on vp (except for call from file system specific
 *	   functions (e.g. iget)), at which time the vnode is unfindable.
 *	-- lock order:  mount vlist lock, then vnode lock. 
 *	-- Vnode is NOT locked.
 *	-- mp cannot be unmounted.
 *	-- v_mount cannot change -- accessed under VXLOCK only.
 *	-- v_mountb, v_mountf under control of mount list lock. 
 */
void
insmntque(vp, mp)
	register struct vnode *vp;
	register struct mount *mp;
{
	struct vnode *vq;
	register struct mount *svmp;

	ASSERT(vp->v_mount != NULLMOUNT);
	if ((svmp = vp->v_mount) != DEADMOUNT) {
		MOUNT_VLIST_LOCK(svmp);
		/*
	 	 * Delete from old mount point vnode list, if on one.
		 */
		if (vp->v_mountb) {
			if (vq = vp->v_mountf)
				vq->v_mountb = vp->v_mountb;
			*vp->v_mountb = vq;
		}
		MOUNT_VLIST_UNLOCK(svmp);
	}
	/*
	 * Insert into list of vnodes for the new mount point, if available.
	 * MP:  don't need to lock; this is only done here.
	 */


	VN_LOCK(vp);
	vp->v_mount = mp;
	VN_UNLOCK(vp);
	if (mp == DEADMOUNT) {
		vp->v_mountf = NULLVP;
		vp->v_mountb = (struct vnode **) 0;
		return;
	} 
	if (vp->v_type == VREG) ubc_object_allocate(vp);
	MOUNT_VLIST_LOCK(mp);
	if (mp->m_mounth) {
		vp->v_mountf = mp->m_mounth;
		vp->v_mountb = &mp->m_mounth;
		mp->m_mounth->v_mountb = &vp->v_mountf;
		mp->m_mounth = vp;
	} else {
		mp->m_mounth = vp;
		vp->v_mountb = &mp->m_mounth;
		vp->v_mountf = NULLVP;
	}
	MOUNT_VLIST_UNLOCK(mp);
}

/*
 * Grab a particular vnode from the free list, increment its
 * reference count and lock it. 
 *
 * Vgetm returns 0 if it was successful.
 * If the vnode has the VXLOCK flag set, vgetm will return 1 after
 * possibly waiting for the flag to clear, depending upon the wait
 * parameter. 
 *
 * The VXLOCK flag indicates that the vnode is in a state of transition
 * and will be in a different state upon its clearing.
 *
 * vgetm comes in two flavors:
 * In flavor #1 (wait == 1), the process is awakened when VXLOCK is
 * cleared, and an error returned to indicate that the vnode is no 
 * longer usable (possibly having been changed to a new file system type).
 *
 * Flavor #2 (wait == 0) will return an error immediately without sleeping,
 * if VXLOCK is set.  
 * This version was created for those conditions when the sleep/wakeup 
 * synchronization is not necessary.
 *
 * This function is called via two different macros, one which sets
 * the wait parameter to 0, and one which sets it to 1.
 * These are defined in vnode.h as vget() and vget_nowait().
 *	vget(vp) => vgetm(vp, 1)
 *	vget_nowait(vp) => vgetm(vp, 0)
 *
 * MP:  -- vget takes an unlocked vnode; vget_nowait takes a locked vnode.
 *	-- lock order for vnode and freelist locks is (vnode, then free list).
 *
 */
vgetm(vp, wait)
	register struct vnode *vp;
	int wait;
{
	register struct vnode *vq;

	if (wait) {
		VN_LOCK(vp);
		if (wait_for_vxlock(vp, VX_NOLOCK)) {
			VN_UNLOCK(vp);
			return (1);
		}
	} else {
		if (vp->v_flag & VXLOCK)
			return (1);
	}
	LASSERT(SLOCK_HOLDER(&vp->v_lock));
	if (vp->v_usecount == 0) {
		VN_FREE_LOCK();	
		if (vq = vp->v_freef)
			vq->v_freeb = vp->v_freeb;
		else
			vfreet = vp->v_freeb;
		*vp->v_freeb = vq;
		vp->v_freef = NULLVP;
		vp->v_freeb = (struct vnode **) 0;
		numvnodes--;
		VN_FREE_UNLOCK();	
	}
	ASSERT(vp->v_freef == NULLVP);
	ASSERT(vp->v_freeb == (struct vnode **) 0);
	vp->v_usecount++;	/* vnode is locked */
	if (wait)
		VN_UNLOCK(vp);
	return (0);
}

#if	MACH_ASSERT
/*
 * Vnode reference, just increment the count
 */
void
vref(vp)
	struct vnode *vp;
{

	VN_LOCK(vp);
#if defined(DCEDFS) && DCEDFS
	if (!(vp->v_flag & V_PRIVATE)) {
#endif  /* DCEDFS */
	ASSERT(vp->v_freef == NULLVP);
	ASSERT(vp->v_freeb == (struct vnode **) 0);
#if defined(DCEDFS) && DCEDFS
      }
#endif  /* DCEDFS */
	vp->v_usecount++;
	if(vp->v_usecount == 0)
	{
		vprint("vref: v_usecount wraparound", vp);
		panic("v_usecount wraparound\n");
	}
	VN_UNLOCK(vp);
}
#endif

/*
 * Vnode release.
 * If count will drop to zero, call inactive routine, then re-check
 * the usecount; if still 1, decrement count and return to freelist.  
 *
 * Synchronization with vgone and vclean:
 *	Two cases in vrele:
 *		1.  v_usecount > 1 => simply decrement and return.
 *		2.  v_usecount == 1 => we're going to call VOP_INACTIVE
 *		  a) If VXLOCK not set, just call through.
 *		     We are making the filesystem-specific vn_inactive
 *		     functions responsible for dealing with this race, which
 *		     means that they can be called simultaneously on the
 *		     same vnode, but ONLY from vclean.  The inactive 
 *		     function can detect this race by checking for VXLOCK.
 *		  b) If VXLOCK is set, then we sleep on it until it's done.
 *		     Even though ref. count is > 0, and we KNOW that
 *		     the inactive function will be called from vclean.  
 *		     There is a hole between the setting of VXLOCK and the
 *		     checking of the usecount in vclean.  If
 *		     we simply decrement the usecount and return, it could 
 *		     be that we are in that hole, which would mean that
 *		     the inactive function would not be called.
 *
 *	The bottom line is that the inactive functions need to synchronize
 *	themselves.
 *
 *	NOTE:  we could put VINACTIVATING back in, but that would cause
 *		forcible unmount to fail if you're hung in the inactive
 *		routine (e.g. nfs server not available).  VINACTIVATING
 *		synchronized inactive and vclean (set here, vclean slept
 *		on it).
 */
void
vrele(vp)
	register struct vnode *vp;
{
	int error;	/* For VOP_INACTIVE call */
	extern struct vnode *consvp;
	decl_simple_lock_data(extern, consvp_lock)

	if (vp == NULLVP)
		panic("vrele: null vp");
	VN_LOCK(vp);
	if (vp == consvp && vp->v_usecount <= 2) {
		simple_lock(&consvp_lock);
		if (vp == consvp) {
			consvp = NULLVP;
			vp->v_usecount--;
		}
		simple_unlock(&consvp_lock);
	}
	if (vp->v_usecount > 1) {
		vp->v_usecount--;
		VN_UNLOCK(vp);
		return;
	}
	ASSERT(vp->v_usecount == 1);
	if (vp->v_usecount < 1) {
		vprint("vrele: bad ref count", vp);
		panic("vrele: bad ref count");
	}
	if (vp->v_flag & VXLOCK)
		wait_for_vxlock(vp, VX_NOLOCK);
	else {
		VN_UNLOCK(vp);
		VOP_INACTIVE(vp, error);
		VN_LOCK(vp);
	}
	/*
	 * Decrement and re-check use count.  
	 * Someone could have done a legitimate
	 * vget() during the VOP_INACTIVE.  If 0, add to free list.
	 */
	if (--vp->v_usecount == 0) {
#if defined(DCEDFS) && DCEDFS
	        if (!(vp->v_flag & V_PRIVATE)) {
#endif /* DCEDFS */
		ASSERT(vp->v_freef == NULLVP);
		ASSERT(vp->v_freeb == (struct vnode **) 0);
		VN_FREE_LOCK();	
		if (vfreeh == (struct vnode *)0) {
			/*
			 * insert into empty list
			 */
			vfreeh = vp;
			vp->v_freeb = &vfreeh;
		} else {
			/*
			 * insert at tail of list
			 */
			*vfreet = vp;
			vp->v_freeb = vfreet;
		}
		vp->v_freef = NULLVP;
		vfreet = &vp->v_freef;
		numvnodes++;
		VN_FREE_UNLOCK();	
#if defined(DCEDFS) && DCEDFS
	}
#endif /* DCEDFS */
	}
	VN_UNLOCK(vp);
}

#if	MACH_ASSERT
/*
 * Page or buffer structure gets a reference.
 */
vhold(vp)
	register struct vnode *vp;
{

	VN_LOCK(vp);
	vp->v_holdcnt++;
	VN_UNLOCK(vp);
}

/*
 * Page or buffer structure frees a reference.
 */
holdrele(vp)
	register struct vnode *vp;
{

	VN_LOCK(vp);
	if (vp->v_holdcnt <= 0) {
		vprint("holdrele: bad count", vp);
		panic("holdrele: holdcnt");
	}
	vp->v_holdcnt--;
	VN_UNLOCK(vp);
}
#endif	/* MACH_ASSERT */

/*
 * Remove any vnodes in the vnode table belonging to mount point mp.
 *
 * If FORCECLOSE is specified, there should not be any active ones,
 * return error if any are found (nb: this is a user error, not a
 * system error). If FORCECLOSE is specified, detach any active vnodes
 * that are found.
 * Assumptions:
 *	-- No path translation
 *	-- No syncs
 *	-- List can change due to close/exit, vnode_pager activity
 *	   Deal with this by "goto loop" -- not great, but it works.
 * MP:
 *	-- Lock order:  mount vlist lock, then vnode lock.
 */
int busyprt = 0;	/* patch to print out busy vnodes */

vflush(mp, skipvp, flags)
	struct mount *mp;
	struct vnode *skipvp;
	int flags;
{
	register struct vnode *vp, *nvp;
	int busy;

loop:
	busy = 0;
	MOUNT_VLIST_LOCK(mp);
	for (vp = mp->m_mounth; vp; vp = nvp) {
		nvp = vp->v_mountf;
		/*
		 * Skip over a selected vnode.
		 * Used by ufs to skip over the quota structure inode.
		 */
		if (vp == skipvp)
			continue;
		VN_LOCK(vp);
		if ((flags & SKIPSYSTEM) && (vp->v_flag & VSYSTEM)) {
			VN_UNLOCK(vp);
			continue;
		}
			
		/*
		 * With an unused vnode, all we need to do is vgone it.
		 * We need to start over, because the mount vlist may
		 * have changed.
		 */
		if (vp->v_usecount == 0) {
			MOUNT_VLIST_UNLOCK(mp);
			(void) vgone(vp, VX_SLEEP, (struct vnodeops *) 0);
			VN_UNLOCK(vp);
			goto loop;
		}
		/*
		 * For block or character devices, revert to an
		 * anonymous device. For all other files, just kill them.
		 */
		if (flags & FORCECLOSE) {
			MOUNT_VLIST_UNLOCK(mp);
			if (vp->v_type != VBLK && vp->v_type != VCHR) {
				(void) vgone(vp, VX_SLEEP, 0);
				VN_UNLOCK(vp);
			} else {
				/* wait for flag -- race with clearalias */
				int ret = wait_for_vxlock(vp, VX_LOCK);
				VN_UNLOCK(vp);
				if (ret == 0) {
					vclean(vp, 0, &spec_vnodeops);
					/*
					 * Problem: someone may be
					 * looking at v_mount on a 
					 * referenced vnode, assuming it's
					 * valid.  To make forcible unmount
					 * really work, we need to be able
					 * to deal with a changing v_mount;
					 * currently, we assume it's
					 * read-only.
					 */
					insmntque(vp, DEADMOUNT);
					clear_vxlock(vp);
				}
			}
			goto loop;
		}
		VN_UNLOCK(vp);
		if (busyprt)
			vprint("vflush: busy vnode", vp);
		busy++;
	}
	MOUNT_VLIST_UNLOCK(mp);
	if (busy)
		return (EBUSY);
	return (0);
}

/*
 * Disassociate the underlying file system from a vnode.
 * Synchronization:  see comments above vrele().
 *	-- Race in inactive functions.  They must synchronize.
 * Assumptions:
 *	-- vnode is unlocked.
 *	-- VXLOCK set.  This prevents other activity while we do
 *	   the dirty work.
 */
void
vclean(vp, flags, newops)
	register struct vnode *vp;
	long flags;
	struct vnodeops *newops;
{
	struct vnodeops *origops;
	int active;

	VN_LOCK(vp);
	ASSERT(vp->v_flag & VXLOCK);

	/*
	 * Check to see if the vnode is in use.
	 * If so we have to reference it before we clean it out
	 * so that its count cannot fall to zero and generate a
	 * race against ourselves to recycle it.
	 */
	if (active = vp->v_usecount)
		vp->v_usecount++;
	if (flags & DOCLOSE) {
		VN_UNLOCK(vp);
		vinvalbuf(vp, 1);
		if (vp->v_type == VREG)
			ubc_invalidate(vp, 0, 0, 0);
		VN_LOCK(vp);
		ASSERT(vp->v_numoutput == 0);
	}
	/*
	 * Prevent any further operations on the vnode from
	 * being passed through to the old file system.
	 */

	origops = vp->v_op;
	if (vp->v_type == VREG) ubc_object_free(vp, newops);
	else {
#if defined(DCEDFS) && DCEDFS
	        if (!(vp->v_flag & V_CONVERTED))
		      vp->v_op = newops;
#else
		vp->v_op = newops;
#endif /*  DCEDFS */
	}
	vp->v_tag = VT_NON;
	VN_UNLOCK(vp);
	/*
	 * If purging an active vnode, it must be closed
	 * and deactivated before being reclaimed.
	 *
	 * The vn_inactive call here won't race with any out of vrele
	 * because this would only be called with an already-referenced
	 * vnode, and vrele only calls VOP_INACTIVE for unreferenced ones.
	 *
	 * NOTE:  we don't know the mode (FREAD or FWRITE of a file 
	 * 	  being closed here, so we can't intelligently change
	 *	  the v_rdcnt and/or v_wrcnt in the vnode.  It doesn't
	 *	  really matter, since if this function is being called
	 *	  then access to the vnode is going to be an error,
	 *	  except for device files, for which the rdcnt and wrcnt
	 *	  are not used (yet...).
	 */

	/*
	 * Since we're calling back into a filesystem that may be
	 * funneled, we need to honor that here. Normally, the _VOP_
	 * macros do the MOUNT_FUNNEL(), but here we need to do
	 * them explicitly.
	 */
	MOUNT_FUNNEL(vp->v_mount);
	if (active) {
		if (flags & DOCLOSE)
			(*(origops->vn_close))(vp, 0, NOCRED);
		(*(origops->vn_inactive))(vp);
	}

	/*
	 * Reclaim the vnode.
	 */
	if ((*(origops->vn_reclaim))(vp)) {
		MOUNT_UNFUNNEL(vp->v_mount);
		panic("vclean: cannot reclaim");
	}
	MOUNT_UNFUNNEL(vp->v_mount);

#if defined(DCEDFS) && DCEDFS
	if ((vp->v_flag & V_CONVERTED)) {
	        VN_LOCK(vp);
	        vp->v_op = newops;
		VN_UNLOCK(vp);
	}
#endif  /* DCEDFS */
 
	/*
	 * vn_reclaim may have done this already, but we must be sure.
	 */
	cache_purge(vp);

	if (active)
		vrele(vp);
}


/*
 * Eliminate all activity associated with a vnode
 * in preparation for reuse.
 * Assumptions:
 *	-- Vnode is locked.
 *	-- VXLOCK was not set by caller, but it could be set (race).
 * Algorithm:
 *	-- set VXLOCK upon entering, clear it upon return.
 *	-- Return value == 0 if VXLOCK was not set upon entering.
 *	-- Return value == 1 if VXLOCK was set upon entering.
 *	-- If VXLOCK was set, and sleep == VX_SLEEP, sleep on it, then
 *	   return.
 *	-- If VXLOCK was set, and sleep == VX_NOSLEEP, return immediately.
 *	-- It is easier for some callers to avoid checking for VBAD (an
 *	   already-vgone'd vnode), so we do it here, and simply return 0.
 * MP:
 *	-- lock order:  SPECHASH_LOCK, then vnode lock.
 *			vnode lock, then vnode freelist lock.
 *
 */
vgone(vp, sleep, ops)
	register struct vnode *vp;
	int sleep;
	struct vnodeops	*ops;
{
	register struct vnode *vq;
	struct vnode *vx;
	long count;

	/*
	 * Paranoia -- we better not be vgone'ing the root device vnode
	 *		or root directory.
	 */
	ASSERT((vp != rootvp) && (vp != rootdir));
	/*
	 * vnode is locked
	 */
	LASSERT(SLOCK_HOLDER(&vp->v_lock));
	
	/*
	 * If a vgone (or vclean) is already in progress,
	 * sleep until it's clear if VX_SLEEP was passed in, otherwise
	 * return now.  wait_for_vxlock returns 1 if it slept.
	 */
	if ((vp->v_flag & VXLOCK) && (sleep == VX_NOSLEEP))
		return (1);
	if (wait_for_vxlock(vp, VX_LOCK))
		return (1);

	/*
	 * There is a slim possibility that this vnode was already
	 * vgone'd, in which case, it will have v_type == VBAD.
	 */
	if (vp->v_type == VBAD) {
		VN_UNLOCK(vp);
		clear_vxlock(vp);
		VN_LOCK(vp);
		return (0);
	}

	VN_UNLOCK(vp);
	/*
	 * Clean out the filesystem specific data.
	 */
	if (ops == (struct vnodeops *) 0)
		ops = &dead_vnodeops;

	vclean(vp, DOCLOSE, ops);
	/*
	 * Delete from old mount point vnode list, if on one.
	 * v_mount only changes under VXLOCK, or when the vnode is
	 * unavailable, so we can use at it here.
	 */
	if (vp->v_mount != DEADMOUNT)
		insmntque(vp, DEADMOUNT);

	VN_LOCK(vp);
	/*
	 * If it is on the freelist, move it to the head of the list.
	 * A locked vnode with VXLOCK set and v_usecount of 0 MUST
	 * be on the vnode free list.
	 */
	if (vp->v_usecount == 0) {
		VN_FREE_LOCK();
		if (vp->v_freeb) {
			if (vq = vp->v_freef)
				vq->v_freeb = vp->v_freeb;
			else
				vfreet = vp->v_freeb;
			*vp->v_freeb = vq;
			vp->v_freef = vfreeh;
			vp->v_freeb = &vfreeh;
			vfreeh->v_freeb = &vp->v_freef;
			vfreeh = vp;
		}
		VN_FREE_UNLOCK();
	}
	vp->v_type = VBAD;
	/*
	 * wake up sleepers.  clear_vxlock() expects unlocked vnode.
	 */
	VN_UNLOCK(vp);
	clear_vxlock(vp);
	/*
	 * we return from this function with vp locked, since we were
	 * called with it locked.
	 */
	VN_LOCK(vp);
	return (0);
}

/*
 * vfssw_add and vfssw_del.
 * Routines to manipulate the VFS switch structure.
 *
 * These functions are intended to be called by filesystem configuration
 * entry points to add and delete file systems from the VFS switch.
 *
 * vfssw_add(fstype, fsops)
 *	short fstype;		file system identifier (index into vfssw)
 *	struct vfsops *fsops;	pointer to VFS operations
 *
 * Algorithm:
 *	1.  Insert in VFS switch, if not there already.
 *	2.  Call file system's initialization function; if an error is
 *	    returned, then the file system is removed from the switch.
 *
 * Notes:
 * 	At this time, the implementation requires that the fstype argument
 * 	be an index into the vfssw.  As a result, a given file system will
 *	always be in the same place in the VFS switch.
 *
 * vfssw_del(fstype)
 *	short fstype;	file system identifier (index into vfssw)
 *
 * Algorithm:
 *	1.  Be sure that there are no active mounts of this file system type.
 *	2.  Remove the file system operations from the VFS switch.
 *
 * Notes:
 * 	It is assumed that if a file system has cleanup to do, it will
 *	do that after calling this function.
 *
 * Lock Synchronization:
 *	The VFSSW_READ_LOCK must be held during the mount operation.
 *	The VFSSW_WRITE_LOCK must be held during vfssw_add and vfssw_del.
 *	The VFSSW_*_LOCK has precedence over any other file system lock.
 */
vfssw_add(fstype, fsops)
	short fstype;
	struct vfsops *fsops;
{
	register int i;
	int error = 0;

	if (fstype < 0 || fstype > mount_maxtype)
		return (EINVAL);
	VFSSW_WRITE_LOCK();
	if (vfssw[fstype] == (struct vfsops *) 0) {
		vfssw[fstype] = fsops;
		if (error = (*(fsops)->vfs_init)())
			vfssw[fstype] = (struct vfsops *) 0;
	} else
		error = EBUSY;
	VFSSW_WRITE_UNLOCK();
	return (error);
}

vfssw_del(fstype)
	short fstype;
{
	register struct mount *mp;
	int error = 0;

	if (fstype < 0 || fstype > mount_maxtype)
		return (EINVAL);
	VFSSW_WRITE_LOCK();
	if (vfssw[fstype] != (struct vfsops *) 0) {
		MOUNTLIST_LOCK();
		mp = rootfs;
		do {
			if (mp->m_stat.f_type == fstype)
				break;
			mp = mp->m_next;
		} while (mp != rootfs);
		MOUNTLIST_UNLOCK();
		if (mp == rootfs)
			/* no active mounts */
			vfssw[fstype] = (struct vfsops *) 0;
		else
			error = EBUSY;
	} else
		error = ENODEV;
	VFSSW_WRITE_UNLOCK();
	return (error);
}

/*
 * Print out a description of a vnode.
 * MP:  no locking here.  It's only done under debug or panic situations.
 */
static char *typename[] =
   { "VNON", "VREG", "VDIR", "VBLK", "VCHR", "VLNK", "VSOCK", "VFIFO", "VBAD" };

vprint(label, vp)
	char *label;
	register struct vnode *vp;
{
	int indx = (int) vp->v_type;
	int error;

	if (label != NULL)
		printf("%s: ", label);
	printf("type %s, usecount %d, refcount %d", typename[indx],
		vp->v_usecount, vp->v_holdcnt);
	if (vp->v_flag) {
		printf(", flags (");
		if (vp->v_flag & VROOT)
			printf("%s", "|VROOT");
		if (vp->v_flag & VXLOCK)
			printf("%s", "|VXLOCK");
		if (vp->v_flag & VXWANT)
			printf("%s", "|VXWANT");
		if (vp->v_flag & VEXLOCK)
			printf("%s", "|VEXLOCK");
		if (vp->v_flag & VSHLOCK)
			printf("%s", "|VSHLOCK");
		if (vp->v_flag & VLWAIT)
			printf("%s", "|VLWAIT");
		if (vp->v_flag & VMOUNTING)
			printf("%s", "|VMOUNTING");
		if (vp->v_flag & VMOUNTWAIT)
			printf("%s", "|VMOUNTWAIT");
		printf(")");
	}
	if (vp->v_op) {
		printf("\n\t");
		VOP_PRINT(vp, error);
	} else {
		printf(", no ops\n");
	}
}
