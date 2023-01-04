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
static char	*sccsid = "@(#)$RCSfile: spec_vnops.c,v $ $Revision: 4.4.16.8 $ (DEC) $Date: 1993/12/15 21:07:26 $";
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
 * spec_vnops.c
 *
 * Modification History:
 *
 * 7-sep-91 Brian Harrigan
 *      Removed 30-aug changes since macros were changes in .h files
 *
 * 30-AUG-91    Brian Harrigan
 *      Changed FUNNEL defs for RT_PREEMPT for the RT MPK
 *	(EFT) This Hack should be fixed in the .h files ...
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
 *	@(#)spec_vnops.c	7.20 (Berkeley) 11/30/89
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/disklabel.h>
#include <sys/sysaio.h>
#include <kern/parallel.h>
#include <sys/vp_swap.h>
#if	MACH
#include <mach/vm_param.h>
#include <kern/zalloc.h>
#else
#include <sys/malloc.h>
#endif
#include <vm/vm_mmap.h>
#include <vm/vm_kern.h>
#include <sys/open.h>
#include <sys/poll.h>
#include <sys/flock.h>


int	spec_lookup(),
	spec_getattr(),
	spec_open(),
	spec_read(),
	spec_clread(),
	spec_write(),
	spec_clwrite(),
	spec_strategy(),
	spec_bmap(),
	spec_ioctl(),
	spec_select(),
	spec_seek(),
	spec_close(),
	spec_reclaim(),
	spec_print(),
	spec_ebadf(),
	spec_badop(),
	spec_nullop(),
	spec_mmap(),
	spec_swap(),
	spec_bread(),
	spec_brelse(),
	spec_clopen(),
	spec_clsetattr(),
	spec_claccess(),
	spec_lockctl();

struct vnodeops spec_vnodeops = {
	spec_lookup,		/* lookup */
	spec_badop,		/* create */
	spec_badop,		/* mknod */
	spec_open,		/* open */
	spec_close,		/* close */
	spec_ebadf,		/* access */
	spec_ebadf,		/* getattr */
	spec_ebadf,		/* setattr */
	spec_read,		/* read */
	spec_write,		/* write */
	spec_ioctl,		/* ioctl */
	spec_select,		/* select */
	spec_mmap,		/* mmap */
	spec_nullop,		/* fsync */
	spec_seek,		/* seek */
	spec_badop,		/* remove */
	spec_badop,		/* link */
	spec_badop,		/* rename */
	spec_badop,		/* mkdir */
	spec_badop,		/* rmdir */
	spec_badop,		/* symlink */
	spec_badop,		/* readdir */
	spec_badop,		/* readlink */
	spec_badop,		/* abortop */
	spec_nullop,		/* inactive */
	spec_reclaim,		/* reclaim */
	spec_bmap,		/* bmap */
	spec_strategy,		/* strategy */
	spec_print,		/* print */
	spec_badop,		/* page_read */
	spec_badop,		/* page_write */
	spec_badop,		/* getpage */
	spec_badop,		/* putpage */
	spec_swap,		/* swap */
	spec_bread,		/* buffer read */
	spec_brelse,		/* buffer release */
	spec_lockctl,		/* file locking */
	spec_nullop,		/* fsync byte range */
};

/*
 * Data structures required for clone devices.
 * 1.  A new operations array.
 * 2.  A new node type and associate macros.
 *
 * Note:  All new clone devices which MUST NOT allow
 *        mmap functionality should specify NODEV in the
 *	  cdevsw table for mmap.  Otherwise, spec_mmap
 *	  will be called for that device.
 */
struct vnodeops spec_cloneops = {
	spec_badop,		/* lookup */
	spec_badop,		/* create */
	spec_badop,		/* mknod */
	spec_clopen,		/* open */
	spec_close,		/* close */
	spec_claccess,		/* access */
	spec_getattr,		/* getattr */
	spec_clsetattr,		/* setattr */
	spec_clread,		/* read */
	spec_clwrite,		/* write */
	spec_ioctl,		/* ioctl */
	spec_select,		/* select */
	spec_mmap,		/* mmap */
	spec_nullop,		/* fsync */
	spec_seek,		/* seek */
	spec_badop,		/* remove */
	spec_badop,		/* link */
	spec_badop,		/* rename */
	spec_badop,		/* mkdir */
	spec_badop,		/* rmdir */
	spec_badop,		/* symlink */
	spec_badop,		/* readdir */
	spec_badop,		/* readlink */
	spec_badop,		/* abortop */
	spec_nullop,		/* inactive */
	spec_reclaim,		/* reclaim */
	spec_badop,		/* bmap */
	spec_badop,		/* strategy */
	spec_print,		/* print */
	spec_badop,		/* page_read */
	spec_badop,		/* page_write */
	spec_badop,		/* getpage */
	spec_badop,		/* putpage */
	spec_badop,		/* swap */
	spec_bread,		/* buffer read */
	spec_brelse,		/* buffer release */
	spec_lockctl,		/* file locking */
	spec_nullop,		/* fsync byte range */
};

struct spec_node {
	struct vnode *sn_vnode;
	struct vattr sn_vattr;
};

#define VTOS(vp)	((struct spec_node *)(vp)->v_data)

/*
 * Local shorthand
 */
#define v_alias v_specinfo->si_alias
#define v_shadowvp v_specinfo->si_shadowvp
#define v_nextalias v_specinfo->si_nextalias

extern void vfree();

/*
 * The hash chain.
 * This MUST be a power of two for the SPECHASH macro to work.
 * The size is enforced where the list is allocated (machine-dependent).
 */
struct spechash *speclisth;		/* the spec hash table */
#define SPECHASH(rdev)  (((rdev>>5)+(rdev))&(spechsz-1))


/* debug XXX */
int checkspeclist = 1;

#define SPECHASH_LOCK(sh)	usimple_lock(&sh->sh_lock)
#define SPECHASH_UNLOCK(sh)	usimple_unlock(&sh->sh_lock)
#define SPECHASH_LOCK_INIT(sh)	usimple_lock_init(&sh->sh_lock)

/*
 * Memory allocation macros
 */
#if	MACH
zone_t specinfo_zone;
zone_t specalias_zone;

#define	SPEC_ALLOCATE(si)	ZALLOC(specinfo_zone, si, struct specinfo *)
#define	SPEC_DEALLOCATE(si)	ZFREE(specinfo_zone, (si))
#define	SA_ALLOCATE(sa)		ZALLOC(specalias_zone, sa, struct specalias *)
#define	SA_DEALLOCATE(sa)	ZFREE(specalias_zone, (sa))
#else	/* MACH */
#define	SPEC_ALLOCATE(si)	MALLOC(si, (struct specinfo *),	\
				      (u_long)sizeof(struct specinfo),\
				       M_VNODE, M_WAITOK)
#define	SPEC_DEALLOCATE(si)	FREE((caddr_t)si, M_VNODE)
#define SA_ALLOCATE(sa)		MALLOC(sa, (struct specalias *), \
					(u_long)sizeof(struct specalias),\
					M_VNODE, M_WAITOK)
#define	SA_DEALLOCATE(sa)	FREE((caddr_t)sa, M_VNODE)
#endif	/* MACH */

#define SPECALIAS_MAX	nvnode
#define SPECINFO_MAX	nvnode


/*
 * General rules for aliases.
 *
 *	-- Specinfo structures are allocated for all device vnodes during
 *	   pathname translation.  They hold the device number and fields
 *	   to link the vnode on an alias list, if necessary; they are not
 *	   linked onto an alias list until the device is opened.
 *	-- Specalias structures are allocated during the open of a device.
 *	   They are linked on a hashchain to associate multiple vnodes that
 *	   reference the same physical device.
 *	-- During makealias, called from spec_open, a vnode is attached
 *	   to an alias list, possibly allocating a specalias structure
 *	   to put on the hashchain.  If the device is a block device, 
 *	   there is a vnode allocated also to be the "shadow" vnode used
 *	   for operations involving the buffer cache (read, write, buffer
 *	   flushing).  This vnode provides cache consistency for all of
 *	   the open vnodes representing the same block device.
 *	-- Specinfo structures and aliases are unlinked
 *	   and deallocated in spec_reclaim, when vnodes are being recycled.
 *	-- It is legitimate to have vnodes with v_usecount of 0 on an alias
 *	   list (opened, then closed, but not yet reclaimed).
 *	-- It is legitimate to have an alias structure with a sa_usecount of
 *	   0 on a hashchain, as long as its alias list is non-null.
 *	-- It is possible to have an alias structure with a sa_usecount of
 *	   > 0, with a null alias list (clearalias in progress; in this case
 *	   multiple opens of a vnode will only have a single close, so the
 *	   count won't match).
 *	-- Specalias structures for mounted file systems have the
 *	   SA_MOUNTED flag set.
 *	-- all fields inside specinfo, specalias, and spechash structures
 *	   are under protection of the spechash lock for the 
 *	   hashchain (MP only). 
 *
 * 	Synchronization with flags:
 *	-- spec_close waits for SA_GOING (clearalias).
 *	-- makealias and clearalias wait for SA_GOING and SA_CLOSING.
 *	-- those who sleep with SA_WAIT set MUST NOT re-check the flags
 *	   after awakening, but must start over, since the alias
 *	   structure could have been deallocated.
 *	MP synchronization:
 *	-- Because of the possibility of clearalias being called virtually
 *	   any time, we must validate the vnode when called through the
 *	   VOP_* interface, and other times vnodes are used as well.  It is
 *	   sufficient to check for VXLOCK and type != VBAD, since our 
 *	   vnode reference will prevent recycling of the vnode to another
 *	   file system.
 *	-- It is necessary to take both the vnode and the spechash lock 
 *	   at the same time to ensure synchronization.  In this case, we
 *	   need to take the vnode lock FIRST.
 */

/*
 * specalloc
 *	Called from path translation routines to allocate a specinfo
 *	structure to be associated with a device vnode.
 * Assumptions:
 *	-- v_type is set correctly.
 *	-- caller will set v_rdev.
 *	-- vnode is not on ANY lists, and cannot be found, so no locking
 *	   is required.
 */
specalloc(vp, dev)
	struct vnode *vp;
	dev_t dev;
{
	register struct specinfo *si;

	SPEC_ALLOCATE(si);
	if (si == (struct specinfo *)0) {
		return (ENOMEM);
	}
	bzero((caddr_t)si, sizeof(struct specinfo));
	si->si_rdev = dev;
	vp->v_specinfo = si;
	return (0);
}

/*
 * Create a special file vnode.
 * Defaults to v_type of VBLK.
 * Used for:
 *	root filesystem, argdev, and swap areas.
 *	memory file system special devices.
 *	clone devices (VCHR).
 * MP:
 *	-- No locking needed.  We get a new vnode; no one can
 *	   find it, so there's no contention.
 */
bdevvp(dev, vpp)
	dev_t dev;
	struct vnode **vpp;
{
	register struct vnode *vp;
        struct specinfo *nsi;
	struct vnode *nvp;
	int error;

	SPEC_ALLOCATE(nsi);
	if (nsi == (struct specinfo *)0) {
		*vpp = 0;
		return (ENOMEM);
	}
	error = getnewvnode(VT_NON, &spec_vnodeops, &nvp);
	if (error) {
		SPEC_DEALLOCATE(nsi);
		*vpp = 0;
		return (error);
	}
	bzero((caddr_t)nsi, sizeof(struct specinfo));
	vp = nvp;
	vp->v_specinfo = nsi;
	vp->v_type = VBLK;
	vp->v_rdev = dev;
	insmntque(vp, DEADMOUNT);
	*vpp = vp;
	return (0);
}


/*
 * makealias
 *	Attach a vnode to an alias structure, allocating one if necessary.
 *	Synchronization:
 *		-- wait for in-progress spec_close and clearalias calls.
 *		-- lock order: vnode, THEN spechash lock.  Need this to
 *		   guarantee vnode consistency.
 */
makealias(vp)
	register struct vnode *vp;
{
        register struct specalias *sa;
        register struct specalias *nsa;
        struct spechash *spechashp;
        register struct vnode *nvp;
        register dev_t rdev;
        register enum vtype type;
	int allocated = 0;

loop:
	VN_LOCK(vp);
	/*
	 * Check for a change of state.   Type could only be VBAD if
	 * we've been vgone'd because of reference count.
	 */
	if ((vp->v_flag & VXLOCK) || (vp->v_type == VBAD)) {
		VN_UNLOCK(vp);
		if (allocated) {
			if (type == VBLK) {
				ASSERT(nvp);
				nvp->v_alias = (struct specalias *) 0;
				vfree(nvp);
			}
			SA_DEALLOCATE(nsa);
		}
		return (ENXIO);
	}
	rdev = vp->v_rdev;
	type = vp->v_type;
	spechashp = &speclisth[SPECHASH(rdev)];
	SPECHASH_LOCK(spechashp);
	sa = vp->v_alias;
	if(sa && sa->sa_rdev == NODEV)
		rdev = NODEV;
	VN_UNLOCK(vp);

	if (rdev != NODEV && sa == (struct specalias *) 0)
		for (sa = spechashp->sh_alias; 
		     sa && ((sa->sa_rdev != rdev) || (sa->sa_type != type)); 
		     sa = sa->sa_next);
	if (sa != (struct specalias *) 0) {
		ASSERT(rdev != NODEV);
		if ((sa->sa_flag & (SA_CLOSING|SA_GOING)) != 0) {
			sa->sa_flag |= SA_WAIT;
			assert_wait((vm_offset_t)&sa->sa_flag, FALSE);
			SPECHASH_UNLOCK(spechashp);
			thread_block();
			goto loop;
		}
		if (allocated) {
			/*
			 * We get here in the case of racing makealias calls
			 */
			if (type == VBLK) {
				ASSERT(nvp);
				nvp->v_alias = (struct specalias *) 0;
				vfree(nvp);
			}
			SA_DEALLOCATE(nsa);
		}
	} else {
		/*
		 * Not on list, so we allocate new structures and
		 * goto loop in case we slept and someone got in and
		 * created this alias before us.  In that case, we
		 * deallocate.  If all works out, we'll get back here
		 * again, with the allocated flag set.
		 */
		if (!allocated) {
			SPECHASH_UNLOCK(spechashp);
			SA_ALLOCATE(nsa);
			if (nsa == (struct specalias *)0)
				return (ENOMEM);
			bzero((caddr_t)nsa, sizeof(struct specalias));
			nsa->sa_type = type;
			nsa->sa_rdev = rdev;
			if (type == VBLK) {
				struct vnode *tvp;
				int error;
				if (error = bdevvp(rdev, &tvp)) {
					SA_DEALLOCATE(nsa);
					return (error);
				}
				nvp = tvp;
				nsa->sa_vnode = nvp;
				nvp->v_alias = nsa;
			}
			allocated = 1;
			goto loop;
		}
		sa = nsa;
		if(rdev != NODEV) {
		    sa->sa_next = spechashp->sh_alias;
		    spechashp->sh_alias = sa;
		}
	}
	/*
	 * put our reference on the alias structure
	 */
	sa->sa_usecount++;
	/*
	 * The hashchain is still locked
	 */
	LASSERT(SLOCK_HOLDER(&spechashp->sh_lock));
	/*
	 * Determine whether the vnode is already on the list.
	 * If not, add it.
	 */
	for (nvp = sa->sa_vlist; (nvp && nvp != vp); nvp = nvp->v_nextalias);
	if (!nvp) {
		vp->v_nextalias = sa->sa_vlist;
		vp->v_alias = sa;
		sa->sa_vlist = vp;
		/*
		 * set up shadow vnode for block devices
		 */
		if (type == VBLK)
			vp->v_shadowvp = sa->sa_vnode;
	} else {
		/* 
		 * this is DEBUG code that should disappear.  Want
		 * to verify list for now.
		 */
		if (checkspeclist) {
			register struct vnode *tvp;

			for (tvp=sa->sa_vlist; tvp; tvp=tvp->v_nextalias) {
				if (tvp == vp)
					break;
			}
			if (tvp != vp)
				panic("makealias: not on vlist");
		}
	}
	SPECHASH_UNLOCK(spechashp);
	return (0);
}

/*
 * Eliminate all activity associated with  the requested vnode
 * and with all vnodes aliased to the requested vnode.
 * 
 * This routine must traverses the alias list and vgone all of the
 * vnodes it finds.
 *
 * Synchronization issues:
 *	-- Wait for other clearalias calls in progress.
 *	-- Wait for in-progress spec_close calls to complete.
 *	   If, while we're sleeping on SA_CLOSING, someone else
 *	   sneaks in and vgone's us, we should exit.  So we re-check
 *	   the vnode after sleeping.
 * 	-- If we encounter a vnode with VXLOCK set, we simply wait for
 *	   it to clear (in vgone), and continue.  Since we have SA_GOING set
 *	   by then, we can guarantee that the alias specinfo structure will
 *	   remain valid.
 */
void
clearalias(vp, ops)
	register struct vnode *vp;
	struct vnodeops	*ops;
{
	struct spechash *spechashp;
	register struct specalias *sa;
	register struct specalias *tsa;
	register struct vnode *tvp;
	register enum vtype type;
	register dev_t	rdev;
	int closeit;
	void *private;

loop:
	VN_LOCK(vp);
	/*
	 * Check for a change of state.   Type could only be VBAD if
	 * we've been vgone'd because of reference count.
	 */
	if ((vp->v_flag & VXLOCK) || (vp->v_type == VBAD)) {
		VN_UNLOCK(vp);
		return; 
	}
	rdev = vp->v_rdev;
	type = vp->v_type;
	/*
	 * Don't do anything to mounted devices.
	 */
	if (type == VBLK) {
		VN_UNLOCK(vp);
		if (setmount(vp, SM_MOUNTED))
			return;
		VN_LOCK(vp);
	}
	spechashp = &speclisth[SPECHASH(rdev)];
	SPECHASH_LOCK(spechashp);
	if (sa = vp->v_alias)
		private = sa->sa_private;
	else
		private = 0;

	VN_UNLOCK(vp);
	/*
	 * If the device was never opened we don't have an alias 
	 * structure.  Therefore we must search for one, and if found, 
	 * vgone all of its aliases.
	 */
	if (sa == (struct specalias *) 0) {
		for (sa = spechashp->sh_alias;
		     sa && ((sa->sa_rdev != rdev) || (sa->sa_type != type));
		     sa = sa->sa_next);
		if (sa == (struct specalias *) 0) {
			SPECHASH_UNLOCK(spechashp);
			return;
		}
	}
	/*
	 * Remember if there had been an active open on the device
	 */
	closeit = sa->sa_usecount;
	/*
	 * The v_alias field MUST be valid.  We know that we have a referenced
	 * device vnode, and its VXLOCK flag was NOT set when we locked the
	 * hashchain.
	 */
	if ((sa->sa_flag & SA_GOING) != 0) {
		/* clearalias in progress */
		sa->sa_flag |= SA_WAIT;
		assert_wait((vm_offset_t)&sa->sa_flag, FALSE);
		SPECHASH_UNLOCK(spechashp);
		thread_block();
		/*
		 * Can return; close couldn't have awoken us, and
		 * interrupts shouldn't have.  If they could, then
		 * we might want to re-check the SA_GOING flag.
		 */
		return;
	}
	if ((sa->sa_flag & SA_CLOSING) != 0) {
		/* spec_close in progress */
		sa->sa_flag |= SA_WAIT;
		assert_wait((vm_offset_t)&sa->sa_flag, FALSE);
		SPECHASH_UNLOCK(spechashp);
		thread_block();
		goto loop;
	}
	/*
	 * We have the hashchain locked; we now lock out 
	 * open/close/clearalias calls by setting SA_GOING.  At that
	 * point it's safe to traverse the list, calling vgone.
	 *
	 * Synchronization NOTES:
	 * -- Vgone will call spec_reclaim, which will
	 *    deallocate the specinfo structure on the hash chain 
	 * -- spec_reclaim will not deallocate the specalias structure
	 *    if SA_GOING is set, so it's always valid here.
	 * -- While SA_GOING is set, NOBODY else can mess with the alias
	 *    list we're working on (outside of vgone/vclean, which
	 *    is synchronized with VXLOCK).
	 */
	sa->sa_flag |= SA_GOING;
	while (tvp = sa->sa_vlist) {
		SPECHASH_UNLOCK(spechashp);
		VN_LOCK(tvp);
		(void) vgone(tvp, VX_SLEEP, ops);
		VN_UNLOCK(tvp);
		SPECHASH_LOCK(spechashp);
	}

	/*
	 * Need to get rid of shadow vnode.  Any associated
	 * buffers were flushed in spec_close.  This vnode is on no
	 * other lists.
	 */
	if (type == VBLK) {
		ASSERT(sa->sa_vnode->v_holdcnt == 0);
		vfree(sa->sa_vnode);
	}
	/*
	 * call the device close if necessary
	 */
	if (closeit) {
		/*
	 	 * Need to unlock the hash chain for the close function.
	 	 */
		SPECHASH_UNLOCK(spechashp);
		speclose(rdev, 0, (type == VBLK ? S_IFBLK : S_IFCHR), NOCRED, private);
		SPECHASH_LOCK(spechashp);
	}

	/*
	 * The vnode list is clean.  Now we remove the alias structure
	 * from the hashchain and deallocate it.
	 */
	tsa = spechashp->sh_alias;
	if (tsa == sa) {
		spechashp->sh_alias = sa->sa_next;
	} else {
		while (tsa && (tsa->sa_next != sa))
			tsa = tsa->sa_next;
		ASSERT(tsa);
		tsa->sa_next = sa->sa_next;
	}

	/*
	 * There is now no way to access the alias structure.  Anyone
	 * who once did is sleeping on it, so we unlock it.
	 */

	SPECHASH_UNLOCK(spechashp);
	/*
	 * wake up sleepers
	 */
	if ((sa->sa_flag & SA_WAIT) != 0) {
		sa->sa_flag &= ~SA_WAIT;
		thread_wakeup((vm_offset_t)&sa->sa_flag);
	}
	SA_DEALLOCATE(sa);
}

/*
 * Reclaim a special file vnode.  
 *
 * This involves removing it from any alias lists it's on, deallocate
 * its specinfo structure, and possibly deallocating the specalias 
 * structure for the alias.
 *
 * Synchronization NOTES:
 *	-- this funtion is only called from vclean, to remove vnodes from
 *	   alias lists even if they have active reference counts.
 *	-- If SA_GOING is set on the specalias structure, then we are 
 *	   being called as a result of clearalias.  In this case, we leave
 *	   the specalias structure alone, so clearalias can deallocate it.
 *	-- VXLOCK is set, so nothing else can be happening to the vnode.
 *
 * We must be prepared to find no alias list because this routine is
 * called even if there were no opens done on the vnode.
 */
spec_reclaim(vp)
	struct vnode *vp;
{
	struct spechash *spechashp;
	register struct specalias *sa;

	ASSERT(vp->v_flag & VXLOCK);
	spechashp = &speclisth[SPECHASH(vp->v_rdev)];

	SPECHASH_LOCK(spechashp);
	sa = vp->v_alias;
	/*
	 * Take the vnode off of the alias list
	 */
	if (sa != (struct specalias *) 0) {
		register struct vnode *tvp;
		tvp = sa->sa_vlist;
		if (tvp == vp)
			sa->sa_vlist = vp->v_nextalias;
		else {
			while (tvp && tvp->v_nextalias != vp)
				tvp = tvp->v_nextalias;
			if (tvp) 
				tvp->v_nextalias = vp->v_nextalias;
		}
	}
	/*
	 * Possibly deallocate the specalias structure on the alias list.
	 * It's usecount is changed in makealias and spec_close.
	 * It is valid to have a sa_usecount of 0, but still have a
	 * list attached.  If there are vnodes on the list, don't deallocate.
	 * If clearalias is being used, it is also valid to have an empty
	 * list with sa_usecount > 0.
	 */
	if (sa && (sa->sa_usecount == 0) && ((sa->sa_flag & SA_GOING) == 0) &&
	    (sa->sa_vlist == (struct vnode *) 0)) {
		if (sa->sa_rdev != NODEV) {
			register struct specalias *tsa;

			tsa = spechashp->sh_alias;
			if (tsa == sa) {
				spechashp->sh_alias = sa->sa_next;
			} else {
				while (tsa && (tsa->sa_next != sa))
					tsa = tsa->sa_next;
				ASSERT(tsa);
				tsa->sa_next = sa->sa_next;
			}
		}
		SPECHASH_UNLOCK(spechashp);
		if (sa->sa_type == VBLK) {
			ASSERT(sa->sa_vnode);
			sa->sa_vnode->v_alias = (struct specalias *) 0;
			vfree(sa->sa_vnode);
		}
		SA_DEALLOCATE(sa);
	} else
		SPECHASH_UNLOCK(spechashp);
	/*
	 * Deallocate the specinfo structure from the vnode
	 */
	SPEC_DEALLOCATE(vp->v_specinfo);
	vp->v_specinfo = (struct specinfo *) 0;
	return (0);
}


/*
 * Trivial lookup routine that always fails.
 */
spec_lookup(vp, ndp)
	struct vnode *vp;
	struct nameidata *ndp;
{

	ndp->ni_dvp = vp;
	ndp->ni_vp = NULL;
	return (ENOTDIR);
}

/*
 * Open called to allow handler
 * of special files to initialize and
 * validate before actual IO.
 */
/* ARGSUSED */
spec_open(vpp, mode, cred)
	struct vnode **vpp;
	int mode;
	struct ucred *cred;
{
	register struct vnode *vp = *vpp;
	dev_t dev, newdev;
	register int maj;
	register struct mount *mp;
	enum vtype type;
	int flag; 
	int ret_val = 0;
	void *private = 0;
	int	mntflg;		/*eq OTYP_MNT if open originated in mount*/
	int	ddi_flag;	/*value of d_flags in bdevsw or cdevsw*/

	VN_LOCK(vp);
	/*
	 * Check for a change of state.   Type could only be VBAD if
	 * we've been vgone'd because of reference count.
	 */
	if ((vp->v_flag & VXLOCK) || (vp->v_type == VBAD)) {
		VN_UNLOCK(vp);
		return (ENXIO); 
	}
	dev = (dev_t) vp->v_rdev;
	maj = major(dev);
	mp = vp->v_mount;
	type = vp->v_type;
	VN_UNLOCK(vp);

	if (mp != DEADMOUNT) {
		BM(MOUNT_LOCK(mp));
		flag = mp->m_flag;
		BM(MOUNT_UNLOCK(mp));
		if (flag & M_NODEV)
			return (ENXIO);
	}
	/*
	 * if makealias returns a non-zero value, it means
	 * there was a problem creating an alias (we could have
	 * been vgone'd), so return an error.
	 */
	if ((ret_val = makealias(vp)) != 0)
		return (ret_val);
	/*
	 * Can't open a mounted block device
	 */
	if ((type == VBLK) && setmount(vp, SM_MOUNTED))	
		ret_val = EBUSY;
	else {
		newdev = dev;
		switch (type) {
		case VCHR:
			if ((u_int)maj >= nchrdev)
				ret_val = ENXIO;
			else {
				/*
				 * If the driver in question supports the 
				 * DDI/DKI interfaces, pass the pointer to 
				 * the ucred structure to the device 
				 * dependent entry point.
				 */
				/*See if driver follows DDI/DKI interfaces*/
				/*CDEVSW_FLAGS sets ddi_flag*/
	  			CDEVSW_FLAGS(maj, ddi_flag);
				if (!(ddi_flag & C_DDIDKI))
					CDEVSW_OPEN(maj, dev, mode, S_IFCHR, 
					    &newdev, ret_val, cred, &private); 
				else 
					CDEVSW_DDI_OPEN(maj, dev, mode, 
					    S_IFCHR, cred, ret_val);
			}
			break;
		case VBLK:
			if ((u_int)maj >= nblkdev)
				ret_val = ENXIO;
			else {
				/*
				 * If the driver in question supports the 
				 * DDI/DKI interfaces, pass the pointer to 
				 * the ucred structure to the device 
				 * dependent entry point. If the open is the 
				 * result of a mount call, and the device 
				 * follows the DDI/DKI interface, the driver 
				 * expects the OTYP_MNT flag. OSF/1 was 
				 * modified to pass this flag in the mode 
				 * parameter.
				 */
				/*See if open called as a result of mount*/
				mntflg = mode & OTYP_MNT;
				mode = mode & ~OTYP_MNT;

				/*BDEVSW_FLAGS sets ddi_flag*/
				BDEVSW_FLAGS(maj, ddi_flag);
				if (!(ddi_flag & B_DDIDKI))
				   BDEVSW_OPEN(maj, dev, mode, S_IFBLK, 
					ret_val, cred, &private);
				else
				   BDEVSW_DDI_OPEN(maj, dev, mode, 
					mntflg|S_IFBLK, cred, ret_val);
			}
			break;
		default:
			panic("spec_open type");
		}
	}
	/*
	 * If there was an error, we have a bogus reference in the 
	 * alias structure, since makealias was called, but
	 * there was a later failure.  We need to clean this up.
	 * Lock order is vnode, then spechash list.
	 */
	if (ret_val) {
		struct specalias *sa; 
		struct spechash *spechashp;
		VN_LOCK(vp);
		type = vp->v_type;
		if ((vp->v_flag & VXLOCK) == 0 && (type != VBAD)) {
			sa = vp->v_alias;
			ASSERT(sa);
			spechashp = &speclisth[SPECHASH(dev)];
			SPECHASH_LOCK(spechashp);
			ASSERT(sa->sa_usecount > 0);
			sa->sa_usecount--;
			SPECHASH_UNLOCK(spechashp);
		}
		VN_UNLOCK(vp);
		/*
		 * If the error is ECLONEME, then the driver wants to be
		 * cloned.  We call spec_clone() to do the work.
		 */
		if (ret_val == ECLONEME) {
			if (type != VCHR)
				ret_val = EINVAL;
			else {
				ret_val = spec_clone(vpp, mode, cred, &private, 0);
			}
		}
	}
	/* Set device private pointer. */
        if (ret_val == 0 && (long)private) {
                vp = *vpp;
                VN_LOCK(vp);
                if ((vp->v_flag & VXLOCK) == 0 && (vp->v_type != VBAD)) {
                        struct spechash *spechashp;
                        ASSERT(vp->v_alias);
                        spechashp = &speclisth[SPECHASH(dev)];
                        SPECHASH_LOCK(spechashp);
                        vp->v_alias->sa_private = private;
                        SPECHASH_UNLOCK(spechashp);
                }
                VN_UNLOCK(vp);
        }
	return(ret_val);
}

/*
 * The next few functions deal with clone devices -- their
 * creation and use.
 */
/*
 * Initialize a newly cloned vnode.
 *
 * -- fill in spec_node data structure.
 * -- cannot fail.
 * -- nobody knows about the vnode yet.
 *
 * Security considerations:
 *      When creating a normal clone, we are actually creating a new
 *      instance of an object which is simply being named by the
 *      filesystem namespace (e.g. /dev/pipe).  This implies that we
 *      really want to associate the current process security information
 *      with the clone, rather than the attributes of the underlying
 *      node, which is just a handle, anyway.  The exception is when
 *      doing FFS CLONE mounts, which create copies of existing device
 *      nodes.  In this case, we actually want to copy the security
 *      attributes of the node being cloned.  This is detected by the
 *      flag, SPEC_DONTOPEN (see ffs_mount for details).
 */

int
spec_clone_init(vp, oldvp, cred, flags)
        struct vnode *vp;
        struct vnode *oldvp;
        struct ucred *cred;
        int flags;
{
        struct spec_node *sp;
        register struct vattr *vap;
        struct timeval tval;
        int i, error;
        struct vattr vattrs;

        VOP_GETATTR(oldvp, &vattrs, cred, error);
        if (error)
                return(error);
        vp->v_op = &spec_cloneops;
        sp = VTOS(vp);
        sp->sn_vnode = vp;
        vap = &sp->sn_vattr;
        vattr_null(vap);
#if     SEC_FSCHANGE
        if (flags & SPEC_DONTOPEN) {
                tag_t *tags;
                struct vsecattr sec_vattrs;

                /*
                 * Copy security tags of underlying object
                 */
                if (VHASSECOPS(oldvp)) {

                        sec_vattrs.vsa_valid = VSA_ALLTAGS;
                        VOP_GETSECATTR(oldvp, &sec_vattrs, cred, error);
                        if (error)
                                return(error);
                        tags = sec_vattrs.vsa_tags;
                } else {
                        /*
                         * Use old vp's mount point tags.
                         * No lock required because they can't change.
                        */
                        tags = oldvp->v_mount->m_tag;
                }
                bcopy(tags, sp->sn_tag, sizeof(sp->sn_tag));
        } else {
                /*
                 * Need to create tags based on current process.
                 */
                SP_OBJECT_CREATE(SIP->si_tag, sp->sn_tag, (tag_t *) 0,
                                 SEC_OBJECT, (dac_t *) 0, (mode_t) 0);

        }
        audstub_levels(sp->sn_tag);
        vp->v_secop = &spec_secops;
#endif  /* SEC_FSCHANGE */
        vap->va_type = VCHR;
        /*
         * This is a bit ugly, but we want a unique dev, ino pair, so
         * we use the rdev for the dev, and the address of the vnode for
         * the inode.  Since it is a clone, the vp will guarantee it is
         * unique, regardless of the file system on which the name is
         * located.
         */
        vap->va_fsid = (long) vp->v_rdev;
        vap->va_fileid = (long) vp;
        vap->va_blocksize = MAXBSIZE;
        vap->va_rdev = vp->v_rdev;
        /*
         * Now do the time -- use current time
         */
        microtime(&tval);
        vap->va_atime = vap->va_mtime = vap->va_ctime = tval;

        /* Now do uid, gid, mode */
        vap->va_uid = vattrs.va_uid;
        vap->va_gid = vattrs.va_gid;
        vap->va_mode = vattrs.va_mode;

        vap->va_nlink = 1;
        return(0);
}

static int
dupalias(oldvp, vp)
     struct vnode *oldvp;
     struct vnode *vp;
{
        struct specalias *sa;
        struct spechash *spechashp;

        ASSERT(vp->v_type == VCHR);
        VN_LOCK(oldvp);
        spechashp = &speclisth[SPECHASH(oldvp->v_rdev)];
        SPECHASH_LOCK(spechashp);
        sa = vp->v_alias = oldvp->v_alias;
        VN_UNLOCK(oldvp);

        if (!sa) {
                /*
                 * If the vnode we're dup'ing is bad, this would
                 * happen (e.g. vgone'd).  Once we've got the hash
                 * chain locked, the alias is safe.
                 */
                SPECHASH_UNLOCK(spechashp);
                return (ENXIO);
        }
        sa->sa_usecount++;

#if     MACH_ASSERT
        {
                struct vnode *nvp;

                /*
                 * The new vnode cannot be on the list, since we just
                * created it.  The old one must be, however.
                 */
                for (nvp = sa->sa_vlist; (nvp && nvp != vp);
                                         nvp = nvp->v_nextalias);
                ASSERT(!nvp);
                /* now check for the old one */
                for (nvp = sa->sa_vlist; (nvp && nvp != oldvp);
                                         nvp = nvp->v_nextalias);
                ASSERT(nvp);
        }
#endif
        vp->v_nextalias = sa->sa_vlist;
        vp->v_alias = sa;
        sa->sa_vlist = vp;
        SPECHASH_UNLOCK(spechashp);
        return (0);
}
/*
 * Clone a vnode.
 * Algorithm:
 *	1.  Allocating and initializing a new (anonymous) vnode.
 *	2.  Call the driver with the O_DOCLONE flag set in the mode field.
 *	3.  Clean up if there is an error.
 * NB:  Clone vnodes never get on any lists or hash chains, other than
 *	the alias list.  They are guaranteed unique, and filesystem
 *	independent.
 */
spec_clone(vpp, mode, cred, private, flags)
	struct vnode **vpp;
	int mode;
	struct ucred *cred;
	void **private;
	int flags;
{
	register struct vnode *vp = *vpp;
	struct vnode *newvp;
	int error = 0;
	dev_t dev, newdev;
	int maj;
	extern void vfree();

	BM(VN_LOCK(vp));
	dev = vp->v_rdev;
	BM(VN_UNLOCK(vp));
	maj = major(dev);
	/*
	 * Allocate and initialize the new vnode
	 */
	if ((error = bdevvp(dev, &newvp)) == 0) {
		/*
	 	 * Nobody knows about new vnode yet; no locking required.
	 	 */
		newvp->v_type = VCHR;	/* bdevvp returns VBLK by default */
		newdev = dev;
		if (!(flags & SPEC_DONTOPEN))
			CDEVSW_OPEN(maj, dev, mode|O_DOCLONE, S_IFCHR, &newdev, error, cred, private);
		else
			error = dupalias(vp, newvp);
		/*
                 * If the error is still ECLONEME, then the device wants an
                 * "anonymous" alias, which is flagged with NODEV in
                 * alias->sa_rdev. The real major number is expected in
                 * vnode->v_rdev.
                 */
                if (error == ECLONEME) {
                        newvp->v_rdev = NODEV;
                        if ((error =
                            spec_clone_init(newvp, vp, cred, flags)) == 0) {
                                error = makealias(newvp);
                                newvp->v_rdev = newdev;
                        }
                } else if (error == 0) {
                        newvp->v_rdev = newdev;
                        if (((error =
                                spec_clone_init(newvp, vp, cred, flags))== 0) &&
                                !(flags & SPEC_DONTOPEN))
                                        error = makealias(newvp);
                }
                if (error == 0) {
                        *vpp = newvp;
                        vrele(vp);
                } else
                        vfree(newvp);
	}
	return (error);
}

/*
 * spec_settime.
 *
 * Called by clone operations to update access and modification
 * times on the vnode.
 */

void
spec_settime(vp, tvalp)
	struct vnode *vp;
	struct timeval *tvalp;
{
	struct timeval tval;

	microtime(&tval);
	VN_LOCK(vp);
	*tvalp = tval;
	VN_UNLOCK(vp);
}

/*
 * spec_clread
 *
 * Intercept operation for clone reads -- updates access time.
 * Safe from changes to dead ops, since no dangerous data structures
 * are touched, and the spec_node is always present, if referenced.
 */
spec_clread(vp, uio, ioflag, cred)
	register struct vnode *vp;
	register struct uio *uio;
	int ioflag;
	struct ucred *cred;
{

	spec_settime(vp, &(VTOS(vp))->sn_vattr.va_atime);
	return (spec_read(vp, uio, ioflag, cred));
}

/*
 * spec_clwrite
 *
 * Intercept operation for clone writes -- updates modify and creation times.
 * Safe from changes to dead ops, since no dangerous data structures
 * are touched, and the spec_node is always present, if referenced.
 */
spec_clwrite(vp, uio, ioflag, cred)
	register struct vnode *vp;
	register struct uio *uio;
	int ioflag;
	struct ucred *cred;
{

	register struct spec_node *sp = VTOS(vp);

	spec_settime(vp, &sp->sn_vattr.va_mtime);
	/* use the same time */
	VN_LOCK(vp);
	sp->sn_vattr.va_ctime = sp->sn_vattr.va_mtime;
	VN_UNLOCK(vp);
	return (spec_write(vp, uio, ioflag, cred));
}

/*
 * spec_getattr
 *
 * Used by clone devices for fstat.  Returns as much of
 * a struct vattr as makes sense for clones.
 * NOTES:
 *	Synchronization controlled by vnode spin lock.
 */
spec_getattr(vp, vap, cred)
	struct vnode *vp;
	struct vattr *vap;
	struct ucred *cred;
{
	VN_LOCK(vp);
	if ((vp->v_flag & VXLOCK) || (vp->v_type == VBAD)) {
		VN_UNLOCK(vp);
		return (EBADF); 
	}
	ASSERT(vp->v_type == VCHR);
	bcopy(&(VTOS(vp)->sn_vattr), vap, sizeof(struct vattr));
	VN_UNLOCK(vp);
	return (0);
}

/*
 * Vnode op for read
 */
spec_read(vp, uio, ioflag, cred)
	register struct vnode *vp;
	register struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	struct buf *bp;
	daddr_t bn;
	long bsize, bscale;
	struct partinfo dpart;
	register int n, on;
	int error = 0;
	extern int mem_no;
	enum vtype type;
	dev_t rdev;
	struct vnode *tvp;
	void *private;
	int retval;
	int ddi_flag;		/*value of d_flags from bdevsw or cdevsw*/


	if (uio->uio_rw != UIO_READ)
		panic("spec_read mode");
	if (uio->uio_resid == 0)
		return (0);
	VN_LOCK(vp);
	if ((vp->v_flag & VXLOCK) || (vp->v_type == VBAD)) {
		VN_UNLOCK(vp);
		return (EIO); 
	}
	type = vp->v_type;
	tvp = vp->v_shadowvp;	/* in case we need it */
	rdev = vp->v_rdev;
	private = vp->v_alias->sa_private;
	VN_UNLOCK(vp);
	switch (type) {
	case VCHR:
		/*
		 * Negative offsets allowed only for /dev/kmem.
		 */
		if (uio->uio_offset < 0 && major(rdev) != mem_no)
			return (EINVAL);
		/*
		 * If the driver in question supports the DDI/DKI interfaces, 
		 * pass the pointer to the ucred structure to the device 
		 * dependent entry point.
		 */
		/*CDEVSW_FLAGS sets ddi_flag*/
		CDEVSW_FLAGS(major(rdev), ddi_flag);
		if (!(ddi_flag & C_DDIDKI))
			CDEVSW_READ(major(rdev), rdev, uio, ioflag, 
				    error, private);
		else
			CDEVSW_DDI_READ(major(rdev), rdev, uio, cred, error);
		  
		return(error);
	case VBLK:
		/*
		 * Switch to shadow vnode.
		 */
		ASSERT(tvp);
		vp = tvp;

		if (uio->uio_offset < 0)
			return (EINVAL);
		bsize = BLKDEV_IOSIZE;
		/*
		 * If the driver for the block device is using
		 * the SVR4 interface, pass it a pointer to the credential 
		 * structure.
		 */
		/*BDEVSW_FLAGS sets ddi_flag*/
		BDEVSW_FLAGS(major(rdev),ddi_flag);
		if (!(ddi_flag & B_DDIDKI))
		  BDEVSW_IOCTL(major(rdev), rdev, DIOCGPART,
			 (caddr_t)&dpart, FREAD, error, cred, private,&retval);
		else
		  BDEVSW_DDI_IOCTL(major(rdev), rdev, DIOCGPART,
		    (caddr_t)&dpart, FREAD, cred, &retval, error);
		if (error == 0) {
			if (dpart.part->p_fstype == FS_BSDFFS &&
			    dpart.part->p_frag != 0 && dpart.part->p_fsize!=0)
				bsize = dpart.part->p_frag *
				    dpart.part->p_fsize;
		}
		bscale = bsize / DEV_BSIZE;
		do {
			bn = (uio->uio_offset / DEV_BSIZE) &~ (bscale - 1);
			on = uio->uio_offset % bsize;
			n = MIN((unsigned)(bsize - on), uio->uio_resid);
			VN_LOCK(vp);
			if (vp->v_lastr + bscale == bn) {
				VN_UNLOCK(vp);
				error = breada(vp, bn, (int)bsize, bn + bscale,
					(int)bsize, NOCRED, &bp);
			} else {
				VN_UNLOCK(vp);
				error = bread(vp, bn, (int)bsize, NOCRED, &bp);
			}
			VN_LOCK(vp);
			vp->v_lastr = bn;
			VN_UNLOCK(vp);
			n = MIN(n, bsize - bp->b_resid);
			if (error) {
				brelse(bp);
				return (error);
			}
			error = uiomove(bp->b_un.b_addr + on, n, uio);
			if (n + on == bsize)
				bp->b_flags |= B_AGE;
			brelse(bp);
		} while (error == 0 && uio->uio_resid > 0 && n != 0);
		return (error);
	default:
		panic("spec_read type");
	}
	/* NOTREACHED */
}

/*
 * Vnode op for write
 */
spec_write(vp, uio, ioflag, cred)
	register struct vnode *vp;
	register struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	struct buf *bp;
	daddr_t bn;
	int bsize, blkmask;
	struct partinfo dpart;
	register int n, on, i;
	int error = 0;
	struct vnode *tvp;
	dev_t rdev;
	enum vtype type;
	void *private;
	int retval;
#if	!MACH
	int count;
#endif
	int ddi_flag;		/*value of d_flags from bdevsw or cdevsw*/
	extern int mem_no;

	if (uio->uio_rw != UIO_WRITE)
		panic("spec_write mode");
	VN_LOCK(vp);
	if ((vp->v_flag & VXLOCK) || (vp->v_type == VBAD)) {
		VN_UNLOCK(vp);
		return (EIO); 
	}
	type = vp->v_type;
	tvp = vp->v_shadowvp;	/* in case we need it */
	rdev = vp->v_rdev;
	private = vp->v_alias->sa_private;
	VN_UNLOCK(vp);
	switch (type) {
	case VCHR:
		/*
		 * Negative offsets allowed only for /dev/kmem
		 */
		if (uio->uio_offset < 0 && major(rdev) != mem_no)
			return (EINVAL);
		/*
		 * If the driver in question supports the DDI/DKI interfaces, 
		 * pass the DDI/DKI parameters.
		 */
		/*CDEVSW_FLAGS sets ddi_flag*/
		CDEVSW_FLAGS(major(rdev), ddi_flag);
		if (!(ddi_flag & C_DDIDKI))
		  CDEVSW_WRITE(major(rdev), rdev, uio, ioflag, error, private);
		else
		  CDEVSW_DDI_WRITE(major(rdev), rdev, uio, cred, error);
		return(error);
	case VBLK:
		/*
		 * Switch to shadow vnode.
		 */
		ASSERT(tvp);
		vp = tvp;

		if (uio->uio_resid == 0)
			return (0);
		if (uio->uio_offset < 0)
			return (EINVAL);
		bsize = BLKDEV_IOSIZE;
		/*
		 * If the driver in question supports the DDI/DKI interfaces, 
		 * pass the DDI/DKI specific parameters.
		 */
		/*BDEVSW_FLAGS sets ddi_flag*/
		BDEVSW_FLAGS(major(rdev),ddi_flag);
		if (!(ddi_flag & B_DDIDKI))
		  BDEVSW_IOCTL(major(rdev), rdev, DIOCGPART,
		    (caddr_t)&dpart, FREAD, error, cred, private,&retval);
		else
		  BDEVSW_DDI_IOCTL(major(rdev), rdev, DIOCGPART,
		    (caddr_t)&dpart, FREAD, cred, &retval, error);
		if (error == 0) {
			if (dpart.part->p_fstype == FS_BSDFFS &&
			    dpart.part->p_frag != 0 && dpart.part->p_fsize!=0)
				bsize = dpart.part->p_frag *
				    dpart.part->p_fsize;
		}
		blkmask = (bsize / DEV_BSIZE) - 1;
		error = 0;
		do {
			bn = (uio->uio_offset / DEV_BSIZE) &~ blkmask;
			on = uio->uio_offset % bsize;
			n = MIN((unsigned)(bsize - on), uio->uio_resid);
			if (n == bsize)
				bp = getblk(vp, bn, bsize);
			else
				error = bread(vp, bn, bsize, NOCRED, &bp);
			n = MIN(n, bsize - bp->b_resid);
			if (error) {
				brelse(bp);
				return (error);
			}
			error = uiomove(bp->b_un.b_addr + on, n, uio);
			if (n + on == bsize) {
				bp->b_flags |= B_AGE;
				bawrite(bp);
			} else
				bdwrite(bp, bp->b_vp);
		} while (error == 0 && uio->uio_resid > 0 && n != 0);
		return (error);
	default:
		panic("spec_write type");
	}
	/* NOTREACHED */
}

/*
 * Device ioctl operation.
 */
/* ARGSUSED */
spec_ioctl(vp, com, data, fflag, cred, retval)
	struct vnode *vp;
	unsigned int com;
	caddr_t data;
	int fflag;
	struct ucred *cred;
	int *retval;
{
	dev_t dev;
	int error, flags;
	enum vtype type;
	void *private;

	VN_LOCK(vp);
	/*
	 * Check for a change of state.   Type could only be VBAD if
	 * we've been vgone'd because of reference count.
	 */
	if ((vp->v_flag & VXLOCK) || (vp->v_type == VBAD)) {
		VN_UNLOCK(vp);
		return (EIO); 
	}
	type = vp->v_type;
	dev = vp->v_rdev;
	if (vp->v_alias)
		private = vp->v_alias->sa_private;
	else
		private = NULL;
	VN_UNLOCK(vp);
	*retval = 0;
	/* Since STREAMS uses the private pointer, if it is null, then
	 * this is definately not a stream.  This also protects some
	 * drivers from getting this ioctl which they might not be
	 * able to handle.
	 */
	if(com == I_ISASTREAM && private == NULL)
		return EIO;
	switch (type) {
	case VCHR:
		if (com != AIOCTLRW) {
			/*
			 * If the driver in question supports the
			 * DDI/DKI interfaces, pass the pointer to the
			 * ucred structure to the device dependent
			 * entry point.
			 */
			/*CDEVSW_FLAGS sets flags*/
			CDEVSW_FLAGS(major(dev), flags);
			if (!(flags & C_DDIDKI))
			    CDEVSW_IOCTL(major(dev), dev, com, data, fflag, 
				       error, cred, private, retval);
			else
			    CDEVSW_DDI_IOCTL(major(dev), dev, com, data, 
					     fflag, cred, retval, error);
		} else {
			error = aio_ioctl(dev, com, data, fflag, cred, private);
		}
		return (error);
	case VBLK:
		if (com == 0 && (int)data == B_TAPE) {
			BDEVSW_FLAGS(major(dev),flags);
			if (flags & B_TAPE)
				return (0);
			else
				return (1);
		}
		/*
		 * If the driver in question supports the DDI/DKI interfaces, 
		 * pass the DDI/DKI specific parameters.
		 */
		/*BDEVSW_FLAGS sets flags*/
		BDEVSW_FLAGS(major(dev),flags);
		if (!(flags & B_DDIDKI))
		    BDEVSW_IOCTL(major(dev), dev, com, data, fflag, error, 
				 cred, private, retval);
    	    	else
       	    	    BDEVSW_DDI_IOCTL(major(dev), dev, com, data, fflag,
			cred, retval, error);
		return(error);
	default:
		panic("spec_ioctl");
		/* NOTREACHED */
	}
}


/* ARGSUSED */
spec_select(vp, events, revents, scanning, cred)
	struct vnode *vp;
	short *events, *revents;
	int scanning;
	struct ucred *cred;
{
	register dev_t dev;
	enum vtype type;
	int error;
	void *private;
	int ddi_flags;			/*d_flags from cdevsw or bdevsw*/
	struct pollhead	*my_pollhead;	/*returned by character device*/

	VN_LOCK(vp);
	/*
	 * Check for a change of state.  Type could only be VBAD if
	 * we've been vgone'd because of reference count.
	 */
	if ((vp->v_flag & VXLOCK) || (vp->v_type == VBAD)) {
		VN_UNLOCK(vp);
		printf("spec_select: bad vnode\n");
		return (0);
	}
	type = vp->v_type;
	dev = vp->v_rdev;
	private = vp->v_alias->sa_private;
	VN_UNLOCK(vp);
	switch (type) {

	default:
		return (0);		/* no error XXX */

	case VCHR:
/*
 * The interface to the SVR4 poll driver entry point is different
 * from the OSF/1 interface to driver poll/select entry point.  One
 * function of this code is to map the interfaces.
 * 
 * Existing OSF/1 character drivers are responsible for registering event
 * notification requests via the select_enqueue function. In SVR4, the 
 * character drivers notify the kernel that they want the kernel to queue
 * an event notification request for them by returning 0 in revents and a
 * pointer to the device's event queue.  If an SVR4 driver that follows
 * the DDI/DKI interfaces returns these values to us, do the select_enqueue
 * for the driver.
 *  
 * The parameter with the value of FALSE in the CDEVSW_DDI_SELECT macro
 * corresponds to the SVR4 anyyet flag (Please refer to the DDI/DKI
 * Reference Manual). The best guess (without looking at SVR4 code) is
 * that poll uses this flag to tell drivers if any events have occurred
 * on other devices.  If the answer is yes, and the "current" driver doesn't
 * have any events pending, the current driver doesn't need to return a
 * pointer to its pollhead structure, since there is no need to wait. This
 * was probably considered a performance enhancement. 
 *
 * OSF/1 handles this situation in a completely different manner.  If 
 * multiple device file descriptors are specified in a poll or select call,
 * OSF/1 examines each device and will queue event notification requests for
 * each device that doesn't have any events pending, whether other devices
 * have events pending or not.  When the poll system call discovers that some
 * devices have returned events, it will loop through all the file descrip-
 * again, flushing any queued event notification requests. 
 * 
 */
		CDEVSW_FLAGS(major(dev), ddi_flags);
		if (!(ddi_flags & C_DDIDKI))
		   CDEVSW_SELECT(major(dev),dev,events,revents,scanning,
				 error,private);
		else {
		    /* Always pass FALSE for the anyyet flag */
		    CDEVSW_DDI_SELECT(major(dev), dev, *events, FALSE,
				      revents, &my_pollhead, error); 
		    if (error)
		        return(error);
		    else {
		        /* register event notification request for driver */
			if (revents != NULL && 
			    *revents == 0 && 
			    my_pollhead != NULL)
				select_enqueue(my_pollhead);
		    }
		}
		return (error);
	}
}

/*
 * Seek on a special file.  This always succeeds.
 */
/* ARGSUSED */
spec_seek(vp, oldoff, newoff, cred)
	struct vnode *vp;
	off_t oldoff, newoff;
	struct ucred *cred;
{

        caddr_t data = 0;
        dev_t rdev;
        int type, retval, error;

        VN_LOCK(vp);
        if ((vp->v_flag & VXLOCK) || (vp->v_type == VBAD)) {
                VN_UNLOCK(vp);
                return EBADF;
        }
        type = vp->v_type;
        rdev = vp->v_rdev;
        VN_UNLOCK(vp);
        /*
         * If not a stream or not a pipe, success.
         */
        if (type != VCHR || major(rdev) == mem_no ||
            (error = spec_ioctl(vp, I_ISASTREAM, &data, 0, cred, &retval)) < 0)
                return 0;
        switch (retval) {
        case I_PIPE:
        case I_FIFO:
                return ESPIPE;
        default:
                return 0;
        }
}

/*
 * Just call the device strategy routine
 */
spec_strategy(bp)
	register struct buf *bp;
{
	int error;

	BDEVSW_STRATEGY(major(bp->b_dev), bp, error);
	return (0);
}

/*
 * This is a noop, simply returning what one has been given.
 */
spec_bmap(vp, bn, vpp, bnp)
	struct vnode *vp;
	daddr_t bn;
	struct vnode **vpp;
	daddr_t *bnp;
{

	if (vpp != NULL)
		*vpp = vp;
	if (bnp != NULL)
		*bnp = bn;
	return (0);
}

/*
 * The function that calls the actual devsw close.  It is separate
 * so the code does not need to be duplicated in spec_close.
 */

speclose(dev, flag, mode, cred, private)
	dev_t	dev;
	int flag;
	int mode;
	struct ucred *cred;
	void *private;
{
	register int error;
	int	mntflg;		/*eq OTYP_MNT if close originated in mount*/
	int	ddi_flag;	/*value of d_flags from bdevsw or cdevsw*/

	if (mode == S_IFCHR) {
		/*
		 * If the character driver in question supports the
		 * DDI/DKI interfaces, pass the pointer to the ucred
		 * structure to the device dependent entry point.
		 */
		/*See if driver follows DDI/DKI interfaces*/
		CDEVSW_FLAGS(major(dev), ddi_flag);
		if (!(ddi_flag & C_DDIDKI))
			CDEVSW_CLOSE(major(dev), dev, flag, mode, error, cred, 
				private);
		else
			CDEVSW_DDI_CLOSE(major(dev), dev, flag, mode, cred, 
			  error);
	} else {
		/*
		 * If the block driver in question supports the
		 * DDI/DKI interfaces, pass the pointer to the ucred
		 * structure to the device dependent entry point.  If
		 * the open is the result of a mount call, and the
		 * device follows the DDI/DKI interface, the driver
		 * expects the OTYP_MNT flag.  OSF/1 has been modified
		 * to sureptiously pass this flag in the mode
		 * parameter.
		 */
		/*See if open called as a result of mount*/
		mntflg = flag & OTYP_MNT;
		flag = flag & ~OTYP_MNT;
		BDEVSW_FLAGS(major(dev), ddi_flag);
		if (!(ddi_flag & B_DDIDKI))
			BDEVSW_CLOSE(major(dev), dev, flag, mode, 
				     error, cred, private);
		else
			BDEVSW_DDI_CLOSE(major(dev), dev, flag, mntflg|mode,
				cred, error);
        }
	return (error);
}

/*
 * Device close routine
 * Synchronization:
 *	-- lock order: vnode, then spec hashchain lock.
 */
/* ARGSUSED */
spec_close(vp, flag, cred)
	register struct vnode *vp;
	int flag;
	struct ucred *cred;
{
	int mode, release, opaq;
	int error = 0;
	register struct specalias *sa;
	struct spechash *spechashp;
	enum vtype type;
	dev_t rdev;
	struct vnode *tvp;
	void *private;

	/*
	 * Synchronization issue:
	 * If this vnode has been vgone'd, or is in the process of
	 * being vgone'd, then we have nothing to do.  The major issue
	 * is making sure that the driver close routine is called.
	 * If wer're in the process of vgone'ing the vnode,
	 * then it's due to one of the following: 
	 * clearalias, getnewvnode, or unlink.
	 * Clearalias will call the driver close routine, 
	 * if required.  If it's one of the latter two conditions,
	 * then it has already been called, via a close() call.
	 */
	VN_LOCK(vp);
	type = vp->v_type;
	if ((type == VBAD) || (vp->v_flag & VXLOCK)) {
		VN_UNLOCK(vp);
		return (0); 
	}
	tvp = vp->v_shadowvp;	/* in case we need it */
	rdev = vp->v_rdev;
	if(vp->v_alias)
		private = vp->v_alias->sa_private;
	else
		private = 0;
	VN_UNLOCK(vp);

	if (type == VBLK) {
		/*
		 * On last close of a block device (that isn't mounted)
		 * we must invalidate any in core blocks, so that
		 * we can, for instance, change floppy disks.
		 *
		 * Must use the shadowvp, since it's the one used for
		 * reads and writes.  Only do this, if the block device
		 * has been opened (i.e. we have allocated a shadow vnode).
		 *
		 * This code used to simply return 0 if vinvalbuf returned
		 * a non-zero value.  This indicates a busy vnode/alias,
		 * but we can't do this; the usecount on the alias struct.
		 * needs to be decremented, so we just drop through and
		 * let things take care of themselves.  If the alias is
		 * busy, the usecount will catch it.
		 */
		if (tvp) {
			(void) vflushbuf(tvp, 0);
			(void) vinvalbuf(tvp, 1);
		}
		mode = S_IFBLK;
	} else if (type == VCHR)
		mode = S_IFCHR;
	else
		panic("spec_close: bad type");


	VN_LOCK(vp);
	if (vp->v_type == VBAD) {
		VN_UNLOCK(vp);
		return (0); 
	}
	spechashp = &speclisth[SPECHASH(rdev)];
	SPECHASH_LOCK(spechashp);
	sa = vp->v_alias;
	VN_UNLOCK(vp);
	if (sa == (struct specalias *) 0) {
		SPECHASH_UNLOCK(spechashp);
		return (0);
	}
	/*
 	 * If we're in the middle of clearalias (SA_GOING set), don't do
	 * the close, clearalias will do it (once and only once).  No need
	 * to change the sa_usecount -- it'll be bogus anyway.
	 */
	if ((sa->sa_flag & SA_GOING) != 0) {
		SPECHASH_UNLOCK(spechashp);
		return (0);
	}
	/*
	 * Synchronization:
	 *
	 * We don't really want to close the device if it is still in
	 * use, unless we are trying to close it forcibly (done above).
	 * The sa_usecount field of the specalias structure holds the
	 * count of active users of the device.  When it goes to zero,
	 * we call the device close.
	 *
	 * The vnode is unlinked from the alias list and the specinfo
	 * and specalias structures associated are deallocated in spec_reclaim.
	 */
	ASSERT(sa->sa_usecount > 0);
	if (--sa->sa_usecount == 0) {
		/*
		 * This is the last close on this device.  We want
		 * to call the devsw.d_close, and then deallocate
		 * this guy.
		 */
		ASSERT((sa->sa_flag & SA_CLOSING) == 0);
		sa->sa_flag |= SA_CLOSING;
		SPECHASH_UNLOCK(spechashp);
		error = speclose(rdev, flag, mode, cred, private);
		SPECHASH_LOCK(spechashp);
		sa->sa_flag &= ~SA_CLOSING;

		/* private pointer is no longer valid since the device has
		 * been closed (and has presumably released what the
		 * private pointer references).
		 */
		VN_LOCK(vp);
		sa->sa_private = 0;
		VN_UNLOCK(vp);

		ASSERT(sa->sa_usecount == 0);	/* better be unused */
		/*
		 * Wake up sleepers
		 */
		if ((sa->sa_flag & SA_WAIT) != 0) {
			sa->sa_flag &= ~SA_WAIT;
			SPECHASH_UNLOCK(spechashp);
			thread_wakeup((vm_offset_t)&sa->sa_flag);
		} else
			SPECHASH_UNLOCK(spechashp);
	} else
		SPECHASH_UNLOCK(spechashp);

	return (error);
}


/*
 * Print out the contents of a special device vnode.
 */
spec_print(vp)
	struct vnode *vp;
{
	dev_t rdev;
	VN_LOCK(vp);
	/*
	 * Check for a change of state.   Type could only be VBAD if
	 * we've been vgone'd because of reference count.
	 */
	if ((vp->v_flag & VXLOCK) || (vp->v_type == VBAD)) {
		VN_UNLOCK(vp);
		printf("spec_print: type == VBAD\n");
		return; 
	}
	rdev = vp->v_rdev;
	VN_UNLOCK(vp);
	printf("tag VT_NON, dev %d, %d\n", major(rdev),
		minor(rdev));
}

/*
 * Special device failed operation
 */
spec_ebadf()
{

	return (EBADF);
}

/*
 * Special device bad operation
 */
spec_badop()
{

	panic("spec_badop called");
	/* NOTREACHED */
}

/*
 * Special device null operation
 */
spec_nullop()
{

	return (0);
}


/*
 * Lookup a vnode by device number.
 */
vfinddev(dev, type, vpp)
	dev_t dev;
	enum vtype type;
	struct vnode **vpp;
{
	int ret = 1;
	struct spechash *spechashp = &speclisth[SPECHASH(dev)];
	register struct specalias *sa;

	SPECHASH_LOCK(spechashp);
	for (sa = spechashp->sh_alias; 
	     sa && ((sa->sa_rdev != dev) || (sa->sa_type != type)); 
	     sa = sa->sa_next);
	/*
	 * If we got a hit, just return the first one on the list.
	 * There better be one.  The assertions are debug; this
	 * could be condensed into one if statement upon their removal.
	 * NOTE:  this is only used by mfs_strategy now.  It expects
	 *	  a referenced vnode.
	 */
	if (sa != (struct specalias *) 0) {
		ASSERT(sa->sa_vlist != (struct vnode *) 0);
		if (sa->sa_vlist != (struct vnode *) 0) {
			*vpp = sa->sa_vlist;
			ASSERT((*vpp)->v_usecount > 0);
			ret = 0;
		}
	}
	SPECHASH_UNLOCK(spechashp);
	return (ret);
}

/*
 * Change from block vnode to its shadow vnode for the purposes
 * of the buffer cache.
 *
 * Called from mntflushbuf and mntinvalbuf.
 * Assumptions:
 *	-- caller verified that v_type is VBLK.
 * Synchronization:
 *	-- takes a locked vnode, and returns it locked.
 */
struct vnode *
shadowvnode(vp)
	struct vnode *vp;
{
	ASSERT(vp->v_type == VBLK);
	if (vp->v_flag & VXLOCK) 
		return ((struct vnode *) 0);
	/*
	 * Although the v_shadowvp field is under protection of the
	 * spec hashchain lock, we know that it's safe here, since it
	 * will only disappear under VXLOCK, and that can only be set
	 * under vnode lock.
	 */
	return (vp->v_shadowvp);
}

/*
 * spec_setopen
 *
 * This is a special purpose function for use by routines that bypass
 * spec_open(), but do the same thing.  That is, reference a vnode and
 * use it as an open file.
 *
 * Currently, the only client of this function is ioctl (TIOCSCTTY) for
 * setting the controlling tty.
 *
 * NOTES:  the type of this vnode shouldn't change.  It won't change on
 *	   forcible unmount, and we're the session leader (for the ioctl)
 *	   so no one else can be doing clearalias on us.  If these 
 * 	   assumptions aren't valid, we should do some sanity checking on
 *	   v_type and v_flag (see assertions).
 */
void
spec_setopen(vp)
	struct vnode *vp;
{
	struct spechash *spechashp;
	register struct specalias *sa;

	VN_LOCK(vp);
	ASSERT(vp->v_type != VBAD);
	ASSERT((vp->v_flag & VXLOCK) == 0);
	spechashp = &speclisth[SPECHASH(vp->v_rdev)];
	sa = vp->v_alias;
	++vp->v_usecount;
	VN_UNLOCK(vp);

	ASSERT(sa);
	SPECHASH_LOCK(spechashp);
	++sa->sa_usecount;
	SPECHASH_UNLOCK(spechashp);
}


/*
 * Several functions in one:
 *
 * 1.  Check to see if a device vnode is open (mount)
 * 2.  Check to see if a device vnode is mounted (spec_open)
 * 3.  Mark a device vnode as mounted (mount)
 * 4.  Mark a device vnode as NOT mounted (unmount)
 *
 * It depends on the flag argument passed, mode.
 * 	SM_OPEN		-- ret. EBUSY if device is open.
 * 	SM_MOUNTED	-- ret. EBUSY if device is mounted.
 * 	SM_SETMOUNT	-- set mounted flag.
 * 	SM_CLEARMOUNT	-- clear mounted flag.
 *
 * NOTES:
 * The check for open devices assumes that open has been called, which
 * means that it will return EBUSY if the usecount is != 1, not 0.
 *
 * For this function, it is possible that, if forcible
 * unmounts are taking place, for the vp to be in a state of transition.
 * The file system upon which the device file was located could be on its
 * way out.  If this happens, the v_type and v_rdev should remain 
 * consistent, in any case, since devices are only dissociated from their
 * file systems, not totally destroyed.
 */
int
setmount(vp, mode)
	register struct vnode *vp;
	register int mode;
{
	register struct specalias *sa;
	struct spechash *spechashp;
	int ret = 0;
	
	VN_LOCK(vp);
	if ((vp->v_flag & VXLOCK) || (vp->v_type == VBAD)) {
		VN_UNLOCK(vp);
		return (EBUSY); 
	}
	sa = vp->v_alias;
	spechashp = &speclisth[SPECHASH(vp->v_rdev)];
	SPECHASH_LOCK(spechashp);
	VN_UNLOCK(vp);
	if (sa) {
		if (sa->sa_flag & SA_MOUNTED) {
			if (mode & SM_MOUNTED)
				ret = EBUSY;
			else if (mode & SM_CLEARMOUNT)
				sa->sa_flag &= ~SA_MOUNTED;
			else
				panic("setmount: already mounted");
		} else {
			/* usecount of one is not busy */
			if (mode & SM_OPEN && sa->sa_usecount != 1)
				ret = EBUSY;
			else if (mode & SM_SETMOUNT)
				sa->sa_flag |= SA_MOUNTED;
			else if (mode & SM_CLEARMOUNT)
				panic("setmount: not mounted");
		}
	} else if ((mode & (SM_OPEN|SM_MOUNTED)) == 0)
		panic("setmount: no alias");
	SPECHASH_UNLOCK(spechashp);
	return (ret);
}

spec_init()
{
	register int i;

	for (i = 0; i < spechsz; i++) {
		SPECHASH_LOCK_INIT((&speclisth[i]));
		speclisth[i].sh_alias = 0;
	}
	ASSERT((spechsz & spechsz-1) == 0);
#if	MACH
	specalias_zone = zinit(sizeof(struct specalias),
				SPECALIAS_MAX*(sizeof(struct specalias)),
				PAGE_SIZE, "specalias");
	if (specalias_zone == (zone_t) NULL)
		panic("spec_init: no zones1");
	specinfo_zone = zinit(sizeof(struct specinfo),
				SPECINFO_MAX*(sizeof(struct specinfo)),
				PAGE_SIZE, "specinfo");
	if (specinfo_zone == (zone_t) NULL)
		panic("spec_init: no zones2");
#endif
}

/*
 *
 */

spec_swap(struct vnode *vp,
	vp_swap_op_t op,	
	caddr_t data,
	struct ucred *cred)
{
	register struct vps_info *vps = (struct vps_info *) data;
	long size;
	dev_t dev;
	int error;
	
	switch (op) {
	case VPS_OPEN:
		if (vp->v_type != VBLK) return EINVAL;
		if (vp->v_shadowvp && vp->v_shadowvp->v_flag & VSWAP)
			return EBUSY;
		if (error = spec_open(&vp, FREAD|FWRITE, cred)) return error;
		dev = vp->v_rdev;
		BDEVSW_PSIZE(major(dev), dev, size);
		if (size < 0) {
			spec_close(vp, 0, cred);
			return EINVAL;
		}
		vp->v_shadowvp->v_flag |= VSWAP;
		vps->vps_size = dbtob(size);
		vps->vps_shift = DEV_BSHIFT;
		vps->vps_flags = VPS_NOMAP;
		VREF(vp);
		vps->vps_vp = vp;
		vps->vps_rvp = vp;
		vps->vps_dev = dev;
		return 0;	
	case VPS_CLOSE:
		if (vp->v_type != VBLK || !vp->v_shadowvp ||
			(vp->v_shadowvp->v_flag & VSWAP) == 0)
			panic("spec_swap: close inconsistency");
		vp->v_shadowvp->v_flag ^= VSWAP;
		spec_close(vp, 0, cred);
		vrele(vp);
		return 0;
	default:
		return EINVAL;
	}
}

spec_mmap(register struct vnode *vp, 
	vm_offset_t offset,
	vm_map_t map,
	vm_offset_t *addrp,
	vm_size_t len,
	vm_prot_t prot,
	vm_prot_t maxprot,
	int flags,
	struct ucred *cred)
{
	extern int nodev();
	struct vp_mmap_args args;
	register struct vp_mmap_args *ap = &args;

	if (vp->v_type != VCHR) return ENODEV;
	ap->a_mapfunc = cdevsw[major(vp->v_rdev)].d_mmap;
	if (ap->a_mapfunc == (int (*)()) nodev) return ENODEV;
	ap->a_dev = vp->v_rdev;
	ap->a_offset = offset;
	ap->a_vaddr = addrp;
	ap->a_size = len;
	ap->a_prot = prot,
	ap->a_maxprot = maxprot;
	ap->a_flags = flags;
	return u_dev_create(map, vp, (vm_offset_t) ap);
}

spec_bread(vp, lbn, bpp, cred)
	register struct vnode *vp;
	off_t lbn;
	struct buf **bpp;
	struct ucred *cred;
{
	return (EOPNOTSUPP);
}

spec_brelse(vp, bp)
	register struct vnode *vp;
	register struct buf *bp;
{
	return (EOPNOTSUPP);
}

/*
 * Special device file locking.
 */
int
spec_lockctl(vp, eld, flag, cred, clid, offset)
        struct vnode *vp;
        struct eflock *eld;
        int flag;
	struct ucred *cred;
	pid_t clid;
        off_t offset;
{
        int error;

	if (vp->v_type != VBLK && vp->v_type != VCHR)
		return (EINVAL);

        if (flag & CLNFLCK)
                return(cleanlocks(vp));
        if (flag & VNOFLCK)
                return(locked(vp, eld, flag));

        if (flag & GETFLCK) {
                return(getflck(vp, eld, offset, clid, FILE_LOCK));
        }
	if (flag & RGETFLCK) {
		return(getflck(vp, eld, offset, clid, LOCKMGR));
	}

        if (flag & SETFLCK) {
                return(setflck(vp, eld, flag&SLPFLCK, offset, clid, FILE_LOCK));
        }
        if (flag & RSETFLCK) {
		if (eld->l_type == F_UNLKSYS) {
			kill_proc_locks(eld);
			return(0);
		}
                return(setflck(vp, eld, flag&SLPFLCK, offset, clid, LOCKMGR));
        }

        if (flag & ENFFLCK)
                return(spec_setvlocks(vp));
        return(EINVAL);
}

static int
spec_setvlocks(vp)
	struct vnode *vp;
{
	struct vattr vattr;
	int error;
	
	VOP_GETATTR(vp, &vattr, u.u_cred, error);
	if (error)
		goto out;

        VN_LOCK(vp);
        if ((vattr.va_mode & VSGID) && (!(vattr.va_mode & S_IXGRP)))
                vp->v_flag |= VENF_LOCK;
        vp->v_flag |= VLOCKS;
        VN_UNLOCK(vp);
out:
        return(error);
}

/* Stuff added for FFS */

/*
 * spec_clopen
 *  open for special clone device (only accessible via file-on-file
 *  mounts)
 */
int
spec_clopen(vpp, mode, cred)
        struct vnode **vpp;
        int mode;
        struct ucred *cred;
{
        dev_t dev, newdev;
        int maj;
        int error;
        struct spechash *spechashp;
        void * privatep;
        struct vnode *vp = *vpp;
        int type;

        VN_LOCK(vp);
        dev = vp->v_rdev;
        newdev = dev;

        type = vp->v_type;
        if (type == VBAD) {
                VN_UNLOCK(vp);
                return (0);     /* will fail later */
        }
        spechashp = &speclisth[SPECHASH(dev)];
        SPECHASH_LOCK(spechashp);
        if (vp->v_alias == NULL)
                panic("spec_clopen: not called on reopen");
	privatep = vp->v_alias->sa_private;
        VN_UNLOCK(vp);
        SPECHASH_UNLOCK(spechashp);

        maj = major(dev);
        mode = mode & (~O_DOCLONE); /* make sure O_DOCLONE is off */
        if (type == VCHR)
                CDEVSW_OPEN(maj, dev, mode, S_IFCHR,
                            &newdev,error,cred, &privatep);
        else
                BDEVSW_OPEN(maj, dev, mode, S_IFBLK,
                            error,cred,&privatep);

        spec_setopen(*vpp);
        /*
         * spec_setopen grabs a vnode reference which we do not
         * want, since this function is only called after a pathname
         * lookup, which has already obtained an open reference.
         */
        vrele(*vpp);
        return(error);
}

/*
 * Check mode permission on vnode pointer. Mode is READ, WRITE or EXEC.
 * The mode is shifted to select the owner/group/other fields. The
 * super user is granted all permissions.
 * Again, this is necessary as a result of file-on-file mounts, since
 * they make cloned vnodes visible.
 *
 * NB: Called from vnode op table. It seems this could all be done
 * using vattr's but... not atomically.
 */
spec_claccess(vp, mode, cred)
        struct vnode *vp;
        int mode;
        struct ucred *cred;
{
#if     SEC_ARCH
        udac_t          udac;
#else
        register gid_t *gp;
        uid_t iuid;
        gid_t igid;
        u_short imode;
        int i;
#endif  /* SEC_ARCH */
        struct spec_node *sp;
        struct vattr *vsp;

        sp = VTOS(vp);
        vsp = &sp->sn_vattr;

#if     SEC_ARCH
        udac.uid = udac.cuid = vsp->va_uid;
        udac.gid = udac.cgid = vsp->va_gid;
        VN_LOCK(vp);
        udac.mode = vsp->va_mode & 07777;
        VN_UNLOCK(vp);

        if (SP_ACCESS(SIP->si_tag, sp->sn_tag, mode, &udac)) {
                return EACCES;
        }
        return 0;
#else   /* !SEC_ARCH */

        /*
         * If you're the super-user, you always get access.
         */
#if     SEC_BASE
        if (privileged(SEC_ALLOWDACACCESS, 0))
#else
        if (cred->cr_uid == 0)
#endif
                return (0);
        /*
         * Access check is based on only one of owner, group, public.
         * If not owner, then check group. If not a member of the
         * group, then check public access.
         */
        VN_LOCK(vp);
        iuid = vsp->va_uid;
        igid = vsp->va_gid;
        imode = vsp->va_mode;
        VN_UNLOCK(vp);
        if (cred->cr_uid != iuid) {
                mode >>= 3;
                if (igid == cred->cr_gid)
                        goto found;
                gp = cred->cr_groups;
                for (i = 0; i < cred->cr_ngroups; i++, gp++)
                        if (igid == *gp)
                                goto found;
                mode >>= 3;
found:
                ;
        }
        if ((imode & mode) == mode)
                return (0);
        return (EACCES);
#endif  /* !SEC_ARCH */
}
/*
 * spec_clsetattr
 *
 * Used by clone devices for chmod & chown.  Sets fields from
 * a struct vattr as specified.
 * This was added to support file-on-file mounts since this
 * provides a way to access cloned vnodes.
 *
 * NOTES:
 *      Only VCHR files will be accessed this way.
 *      Synchronization controlled by vnode spin lock.
 */
int
spec_clsetattr(vp, vap, cred)
        struct vnode *vp;
        struct vattr *vap;
        struct ucred *cred;
{
        struct spec_node *sp;
        struct vattr *svp;
        uid_t iuid;
        gid_t igid;
        int error = 0;

        VN_LOCK(vp);
        if ((vp->v_flag & VXLOCK) || (vp->v_type == VBAD)) {
                VN_UNLOCK(vp);
                return (EBADF);
        }
        ASSERT(vp->v_type == VCHR);
        sp = VTOS(vp);
        svp = &sp->sn_vattr;
        VN_UNLOCK(vp);
        /*
         * Check for unsettable attributes.
         */
        if ((vap->va_type != VNON) || (vap->va_nlink != VNOVAL) ||
            (vap->va_fsid != VNOVAL) || (vap->va_fileid != VNOVAL) ||
            (vap->va_blocksize != VNOVAL) || (vap->va_rdev != VNOVAL) ||
            ((int)vap->va_bytes != VNOVAL) || (vap->va_gen != VNOVAL)) {
                return (EINVAL);
        }
        /*
         * Go through the fields and update iff not VNOVAL.
         */
        if (vap->va_uid != (uid_t) VNOVAL || vap->va_gid != (gid_t) VNOVAL) {

#if     SEC_BASE
                VN_LOCK(vp);
                audstub_dac(0, svp->va_uid, svp->va_gid, svp->va_mode);

                VN_UNLOCK(vp);
                error = spec_chown1(vp, vap->va_uid, vap->va_gid, cred);
                VN_LOCK(vp);
                audstub_dac(1, svp->va_uid, svp->va_gid, svp->va_mode);
                VN_UNLOCK(vp);
                if (error)
                        return (error);
#else
                if (error = spec_chown1(vp, vap->va_uid, vap->va_gid, cred))
                        return (error);
#endif
        }
        if (vap->va_size != VNOVAL)
                return (EINVAL);        /* can't change size of VCHR */

        iuid = svp->va_uid;
        igid = svp->va_gid;
        if (vap->va_atime.tv_sec != VNOVAL || vap->va_mtime.tv_sec != VNOVAL) {
                struct timeval tv;
#if     SEC_ARCH
                VOP_ACCESS(vp, SP_SETATTRACC, cred, error);
                if (error)
                        return error;
#endif
#if     SEC_BASE
                if (cred->cr_uid != iuid && !privileged(SEC_OWNER, EPERM))
                        return EPERM;
#else
                if (cred->cr_uid != iuid) {
                        VOP_ACCESS(vp, VWRITE, cred, error);
                        if (error) {

                               if (error == EACCES)
                                        error = EPERM;
                                return(error);
                        }
                }
#endif
                microtime(&tv);
                VN_LOCK(vp);
                svp->va_atime = vap->va_atime;
                svp->va_mtime = vap->va_mtime;
                svp->va_ctime.tv_sec  = tv.tv_sec;
                VN_UNLOCK(vp);
        }
        if (vap->va_mode != (u_short) VNOVAL) {
#if     SEC_BASE
                VN_LOCK(vp);
                audstub_dac(0, svp->va_uid, svp->va_gid, svp->va_mode);
                VN_UNLOCK(vp);
                error = spec_chmod1(vp, (int)vap->va_mode, cred);
                VN_LOCK(vp);
                audstub_dac(1, svp->va_uid, svp->va_gid, svp->va_mode);
                VN_UNLOCK(vp);
#else
                error = spec_chmod1(vp, (int)vap->va_mode, cred);
#endif
        }
        if (vap->va_flags != VNOVAL) {
                /*
                 * We don't currently allow setting of flags, and it
                 * makes no sense for a clone, anyway.
                 */
                return (EINVAL);
        }
        return (error);
}

/*
 * Change the mode on a file.
 */
spec_chmod1(vp, mode, cred)
        register struct vnode *vp;
        register int mode;
        struct ucred *cred;
{
        int error;
        struct spec_node  *sp;
        struct vattr *svp;

        sp = VTOS(vp);
        svp = &sp->sn_vattr;
        VN_LOCK(vp);
#if     SEC_BASE
        if (cred->cr_uid != svp->va_uid && !privileged(SEC_OWNER, EPERM) ||
            !sec_mode_change_permitted(mode)) {
                VN_UNLOCK(vp);
                return EPERM;
        }
#else
        if (cred->cr_uid != svp->va_uid && (error = suser(cred, &u.u_acflag))) {
                VN_UNLOCK(vp);
                return (error);
        }
#endif
#if     SEC_ARCH
        /*
         * Must unlock vnode across this call; otherwise deadlock!
         */
        VN_UNLOCK(vp);
        VOP_ACCESS(vp, SP_SETATTRACC, cred, error);
        if (error)
                return error;
        VN_LOCK(vp);
#endif
        svp->va_mode &= ~07777;
#if     SEC_BASE
        if ((mode & S_ISVTX) && !privileged(SEC_LOCK, 0))
                mode &= ~S_ISVTX;
        if (!groupmember(svp->va_gid, cred) && (mode & S_ISGID) &&
            !privileged(SEC_SETPROCIDENT, 0))
                mode &= ~S_ISGID;
        /* no privileges, since we're doing devices only here */
#else   /* SEC_BASE */
        if (cred->cr_uid) {
                mode &= ~S_ISVTX;
                if (!groupmember(svp->va_gid, cred))
                        mode &= ~S_ISGID;
        }
#endif  /* SEC_BASE */
        svp->va_mode |= mode & 07777;
#if     SEC_ARCH
        {
                dac_t dac;
                dac.uid = svp->va_uid;
                dac.gid = svp->va_gid;
                dac.mode = svp->va_mode & 0777;
                VN_UNLOCK(vp);
                spec_change_object(vp, &dac, SEC_NEW_MODE);
        }
#else
        VN_UNLOCK(vp);
#endif  /* SEC_ARCH */
        return (0);
}

/*
 * Perform chown operation on a cloned vnode (must be VCHR).
 */
spec_chown1(vp, uid, gid, cred)
        register struct vnode *vp;
        uid_t uid;
        gid_t gid;
        struct ucred *cred;
{
        uid_t ouid;
        gid_t ogid;
        int error;
        struct spec_node *sp;
        struct vattr *vsp;

        sp = VTOS(vp);
        vsp = &sp->sn_vattr;

        VN_LOCK(vp);
        if (uid == (uid_t) VNOVAL)
                uid = vsp->va_uid;
        if (gid == (gid_t) VNOVAL)
                gid = vsp->va_gid;
#if     SEC_BASE
        /*
         * Must own the file or have OWNER privilege to change

         * its owner or group.  Also, if we are changing the
         * file's owner or are changing its gid to a group we
         * don't belong to, must have CHOWN privilege.
         */
        if (!sec_owner(vsp->va_uid, vsp->va_uid) ||
            !sec_owner_change_permitted(vsp->va_uid, vsp->va_gid, uid, gid)) {
                 VN_UNLOCK(vp);
                 return EPERM;
        }
#else
        /*
         * If we don't own the file, are trying to change the owner
         * of the file, or are not a member of the target group,
         * the caller must be superuser or the call fails.
         */
        if ((cred->cr_uid != vsp->va_uid || uid != vsp->va_uid ||
            !groupmember((gid_t)gid, cred)) &&
            (error = suser(cred, &u.u_acflag))) {
                VN_UNLOCK(vp);
                return (error);
        }
#endif
#if     SEC_ARCH
        VN_UNLOCK(vp); /* unlock for access call */
        VOP_ACCESS(vp, SP_SETATTRACC, cred, error);
        if (error)
                return error;
        VN_LOCK(vp);
#endif
        ouid = vsp->va_uid;
        ogid = vsp->va_gid;

        vsp->va_uid = uid;
        vsp->va_gid = gid;
#if     !SEC_BASE
        if (cred->cr_uid != 0)
#endif
                vsp->va_mode &= ~(S_ISUID | S_ISGID);
#if     SEC_ARCH
        {
                dac_t dac;

                dac.uid = vsp->va_uid;
                dac.gid = vsp->va_gid;
                dac.mode = vsp->va_mode & 0777;
                VN_UNLOCK(vp);
                spec_change_object(vp, &dac, SEC_NEW_UID|SEC_NEW_GID);
        }
#else
        VN_UNLOCK(vp);
#endif  /* SEC_ARCH */
        return (0);
}

/*
 * Common open routine for normal and clonable drivers.
 * Allocates and manages minor numbers and per-device structures.
 *
 * Parameters:
 *      mode            open flags (in, for O_DOCLONE only)
 *      devp            (in for non-clone, in/out for clone)
 *                              major(*devp) is major being opened
 *                              minor(*devp) is minor for non-clone
 *                                   "       is minor limit for clone
 *                              *devp is set to new maj,min for clone
 *      size            size of device structure to allocate (in)
 *      dpp             pointer to device structure pointer (out)
 *                              *dpp is set to allocated device structure
 *
 * The device structure is returned zeroed on allocation (first open).
 * Even when size == 0, the device structure pointer is required, and
 * it must be passed to cdevsw_close_comm() in order to free the minor
 * number and associated storage.
 */

typedef struct dev_ent {
        struct dev_ent  *next, *prev;           /* Hash chain of opens */
        struct dev_list *head;                  /* Hash chain list head */
        long            devminor;               /* Minor of this open */
	long		size;			/* Size of entry */
} dev_entry_t;

#define DEV_HASH        31                      /* ^2 - 1 saves space */

typedef struct dev_list {
        struct { struct dev_ent *next, *prev; } heads[DEV_HASH];
        decl_simple_lock_data(,lock)            /* Lock for insert/remove */
} dev_list_t;

int
cdevsw_open_comm(
        int             mode,
        dev_t           *devp,
        unsigned        size,
        void **         dpp)
{
        long devmajor, devminor;
        dev_entry_t *devq, *dev, *newdev;
        dev_list_t *devlist;
        int i, error = 0;

        if (devp == NULL || dpp == NULL)
                return ENXIO;
        if (*devp == NODEV) {
                if (*dpp)               /* reopen */
                        ;
                else if (!size)
                        *dpp = NULL;
                else {
			dev = (dev_entry_t *)kmem_alloc(kernel_map,size + sizeof (dev_entry_t));
                        if (dev == NULL)
                                return ENOMEM;
                        dev->next = dev->prev = dev;
                        dev->head = NULL;
                        dev->devminor = -1L;
			dev->size = size + sizeof(dev_entry_t);
                        bzero((caddr_t)(dev + 1), size);
                        *dpp = (void *)(dev + 1);
                }
                return 0;
        }
        if ((unsigned)(devmajor = major(*devp)) >= nchrdev)
                return ENODEV;
        devlist = (dev_list_t *)(cdevsw[devmajor].d_ttys);
        if (devlist == NULL) {
                /* Cdevsw lock protects initialization of d_ttys */
                if (lock_read_to_write(&(cdevlock[devmajor].dsw_lock)))
                        lock_write(&(cdevlock[devmajor].dsw_lock));
                devlist = (dev_list_t *)(cdevsw[devmajor].d_ttys);
                if (devlist == NULL) {
			devlist = (dev_list_t *)kmem_alloc(kernel_map,
					sizeof (dev_list_t));
                        if (devlist == NULL) {
                                lock_write_to_read(&(cdevlock[devmajor].dsw_lock
));
                                return ENOMEM;
                        }
                        simple_lock_init(&devlist->lock);
                        for (i = 0; i < DEV_HASH; i++) {
                                devlist->heads[i].next = devlist->heads[i].prev
                                    = (struct dev_ent *)&(devlist->heads[i]);
                        }
                        cdevsw[devmajor].d_ttys = (struct tty *)devlist;
                }
                lock_write_to_read(&(cdevlock[devmajor].dsw_lock));
        }
        simple_lock(&devlist->lock);
        if ((mode & O_DOCLONE) == 0) {
                /*
                 * Regular (non-clone) open. Check reopen and if not,
                 * insert new entry in increasing sorted order.
                 */
                devminor = minor(*devp);
                dev = devq =
                    (struct dev_ent *)&(devlist->heads[devminor % DEV_HASH]);
                while (dev->next != devq && dev->next->devminor <= devminor)
                        dev = dev->next;
        } else {
                /*
                 * Clone open. Choose the smallest new minor, by inspecting
                 * each hashlist entry. Insert it maintaining sorted order
                 * as above. Minor(*devp) indicates maxminor, which defaults
                 * to minorlimit in clone_open().
                 */
                int maxminor = minor(*devp);
                for (newdev = NULL, i = 0; ; ) {
                        devq = (struct dev_ent *)&(devlist->heads[i]);
                        if (i <= maxminor) {
                                if ((dev = devq->next) == devq) {
                                        /* Ascending search found empty list */
                                        devminor = i;
                                        break;
                                }
                                /* Find the first hole in the chain and test */
                                while (dev->next != devq &&
                                       dev->next->devminor - dev->devminor
                                       == DEV_HASH)
                                        dev = dev->next;
                                devminor = dev->devminor + DEV_HASH;
                                if (devminor <= maxminor && (newdev == NULL ||
                                    devminor < newdev->devminor + DEV_HASH))
                                        newdev = dev;
                        }
                        if (++i == DEV_HASH) {
                                if (dev = newdev) {
                                        devminor = dev->devminor + DEV_HASH;
                                        break;
                                }
                                error = ETOOMANYREFS;
                                goto out;
                        }
                }
        }
        if (dev == devq || dev->devminor != devminor) {
		newdev = (dev_entry_t *)kmem_alloc(kernel_map,size + sizeof (dev_entry_t));
                if (newdev == NULL)
                        error = ENOMEM;
                else {
                        newdev->devminor = devminor;
                        newdev->head = devlist;
			newdev->size = size + sizeof(dev_entry_t);
                        insque(newdev, dev);            /* Insert after "dev" */
                        dev = newdev;
                        if (size)
                                bzero((caddr_t)(dev + 1), size);
                }
        }
out:
        simple_unlock(&devlist->lock);

        if (error == 0) {
                *dpp = (void *)(dev + 1);
                if (mode & O_DOCLONE)
                        *devp = makedev(devmajor, devminor);
        }
        return error;
}

int
cdevsw_open_ocomm(
        dev_t           otherdev,
        int             mode,
        dev_t           *devp,
        unsigned        size,
        void **         dpp)
{
        long    othermaj;
        int     error;

        /*
         * Used when one major shares another's dev_entries (e.g. pty's)
         */
        if ((unsigned)(othermaj = major(otherdev)) >= nchrdev)
                return ENODEV;
        if (devp == NULL || *devp == NODEV) {
                /* Just go via cdevsw_open_comm for anonymous */
                return cdevsw_open_comm(mode, devp, size, dpp);
        }
        otherdev = makedev(othermaj, minor(*devp));
        CDEVSW_READ_LOCK(othermaj);
        error = cdevsw_open_comm(mode, &otherdev, size, dpp);

        CDEVSW_READ_UNLOCK(othermaj);
        if (error == 0)
                *devp = makedev(major(*devp), minor(otherdev));
        return error;
}

int
cdevsw_close_comm(
        void *  dp)
{
        dev_entry_t *dev;
        dev_list_t *devlist;

        if (dp) {
                dev = ((dev_entry_t *)dp) - 1;
#if     MACH_ASSERT
                if (devlist = dev->head) {
                        if ((unsigned)dev->devminor <= minorlimit) {
                                simple_lock(&devlist->lock);
                                remque(dev);
                                simple_unlock(&devlist->lock);
                                dev->devminor = -1L;
                        } else {
                                panic("cdevsw_close_comm 2");
                                return ENXIO;
                        }
                } else if (dev->devminor != -1L) {
                        panic("cdevsw_close_comm 3");
                        return ENXIO;
                }
#else
                if (devlist = dev->head) {
                        simple_lock(&devlist->lock);
                        remque(dev);
                        simple_unlock(&devlist->lock);
                }
#endif
		kmem_free(kernel_map,dev,dev->size);
        }
        return 0;
}


