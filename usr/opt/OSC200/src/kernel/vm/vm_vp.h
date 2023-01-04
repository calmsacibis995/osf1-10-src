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
 * @(#)$RCSfile: vm_vp.h,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/10/30 15:30:40 $
 */
#ifndef	__VM_VP__
#define	__VM_VP__	1
#include <vm/vm_object.h>
#include <vm/vm_mmap.h>

	

/*
 * A regular vnode points to this object.
 */

struct vm_vp_object {
	struct vm_object vo_object;		/* Object common part */
	struct vnode 	*vo_vp;			/* Vnode */
	struct vm_page	*vo_cleanpl;		/* Clean page list */
	struct vm_page	*vo_cleanwpl;		/* Clean wired page list */
	struct vm_page	*vo_dirtywpl;		/* Dirty wired page list */
	int		vo_wirecnt;		/* Wired pages */
	int		vo_nsequential;		/* Number of contig lastpage */
	vm_offset_t	vo_lastpage;		/* Last page allocated */
	u_long		vo_stamp;		/* Allocation stamp */
	udecl_simple_lock_data(, vo_seglock)	/* Segment list lock */
	struct vm_seg	*vo_seglist;		/* Segment list */
};

#define	vo_dirtypl vo_object.ob_memq
#define vo_orefcnt vo_object.ob_ref_count
#define vo_npages  vo_object.ob_resident_pages

/*
 * An mmaper points to this object which in turn
 * points to a vm_vp_object.  
 */

struct vm_uvp_object {
	struct vm_object uvo_object;		/* Object common part */
	struct vm_vp_object *uvo_vop;		/* Pointer to vop */
	vm_offset_t uvo_end;			/* End of region mapped */
};


#define uvo_refcnt	uvo_object.ob_ref_count
#define	uvo_flags	uvo_object.ob_flags 

#ifdef	KERNEL
extern int vop_lock_size, vop_lock_pages, vop_lock_mask;
#endif	/* KERNEL */

#endif	/* !__VM_VP__ */
