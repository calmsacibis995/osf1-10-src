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
 * @(#)$RCSfile: vpage.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 08:58:18 $
 */
#ifndef	__VP_PAGE__
#define	__VP_PAGE__	1

/*
 * The vpage structure should have any information
 * the is of value for pushes a page out.  This
 * must be true for non K map operations.
 * It is possible that vpage might become pageable
 * in the future.  Care must be taken in acquiring
 * vpage information before taking any spin locks
 * in map entry handlers that require the vpage
 * information.
 */

struct vpage {

	/*
	 * Union of kernel and user vpage fields.  Identical ones
	 * must be in the same bit field position.
	 */

	union {
		struct {
		unsigned int
		_uvp_prot	: 3,		/* Keep protection here */
		_uvp_plock	: 1,		/* Page locked */
				: 28;
		} _uvp;
		struct {
		unsigned int
		_kvp_prot	: 3,		/* Same as user */
				: 5,
		_kvp_kwire	: 8,		/* Wiring count */
				:16;
		} _kvp;
	} _uvpage;
};

#define vp_prot		_uvpage._uvp._uvp_prot
#define vp_plock	_uvpage._uvp._uvp_plock
#define	vp_kwire	_uvpage._kvp._kvp_kwire

#ifdef	KERNEL
extern kern_return_t u_vpage_alloc(/* struct vm_map *map,
		vm_size_t size, struct vpage **vpp, boolean_t canwait */);
extern void u_vpage_free(/* struct vm_map *map, struct vpage *vp, 
		vm_size_t size, boolean_t canwait */);
extern kern_return_t u_vpage_reserve(/* struct vm_map *map, int increase */);
extern void u_vpage_free_reserve(/* struct vm_map *map, int decrease */);
extern void u_vpage_expand(/* struct vpage **vpp, vm_size_t oldsize,
		vm_size_t increase, int offset */);
#endif	/* KERNEL */

#endif	/* !__VP_PAGE__ */
