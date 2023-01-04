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
 * @(#)$RCSfile: vm_vppage.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/08/02 14:56:40 $
 */
#ifndef	__VM_VPPAGE__
#define	__VM_VPPAGE__ 1

/*
 * Vnode specific information for page structure.
 */

struct vm_vppage {
	vm_offset_t vp_addr;			/* Address to access page */
	vm_offset_t vp_pfs;			/* File system private data */
			
};

/*
 * Reserved flag fields
 */

#define	VPP_REFBIT	0x1			/* Mem mapped reference bit */
#define	VPP_INVALID	0x2			/* Page is being invalidated */
#define	VPP_UBCIO	0x4			/* Page setup for I/O by UBC */
#define	VPP_DEAD	0x8			/*
					         * Page committed gone in
						 * in ubc_invalidate
						 */


#define	pg_addr		_upg._vppg.vp_addr
#define	pg_pfs		_upg._vppg.vp_pfs


#define	VP_PAGELIST	(12)
#define	VP_PAGELISTSIZE	(ptoa(VP_PAGELIST))

/*
 * returned by vm_ubc_memhigh
 */

#define	VM_UBC_PAGE_ALLOC	0x00			/* Allocate a page */
#define	VM_UBC_PAGE_FREE	0x01			/* Free dirty pages */
#define	VM_UBC_PAGE_LRU		0x02			/* LRU pages */

#endif	/* !__VM_VPPAGE__ */
