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
 * @(#)$RCSfile: vp_swap.h,v $ $Revision: 1.1.7.3 $ (DEC) $Date: 1993/09/21 22:15:07 $
 */
#ifndef	__VP_SWAP__
#define __VP_SWAP__ 1
#include <mach/vm_param.h>
#include <vm/vmparam.h>
#include <sys/types.h>

/*
 * Operations to VOP_SWAP
 */

typedef enum {VPS_OPEN, VPS_CLOSE, VPS_MAP} vp_swap_op_t;


/*
 * Information returned by VP swap open and sent back on close
 */

struct vps_info {
	char	vps_flags;		/* Flags see below */
	short	vps_shift; 		/* shift to apply for VTOL */
	u_long	vps_size;		/* size of swap space */
	struct	vnode *vps_vp;		/* vnode */
	struct	vnode *vps_rvp;		/* rvnode */
	dev_t	vps_dev;		/* dev_t */
	vm_offset_t	
		vps_private;		/* vnode private pointer for vps_map */
};

#define	VPS_NOMAP	0x01		/* 
					 * swap offset to logical is shift 
					 * and don't call map function
					 * ie., single I/O operation by
					 * strategy routine can make the full
					 * page transfer
					 */
	

struct vps_map {
	vm_offset_t	vps_offset;	/* offset in file */
	short		vps_nblkno;	/* number of blocks in */
	vm_offset_t 	vps_private;	/* private date from open */
					/* block on the device */
#ifdef	PAGE_SIZE_FIXED
	daddr_t		vps_blkno[PAGE_SIZE/MIN_SWAPFSBLOCKSIZE];	
#else
	daddr_t		vps_blkno[MAX_PAGE_SIZE/MIN_SWAPFSBLOCKSIZE];
#endif	/*PAGE_SIZE_FIXED*/
};
	

#endif /* !__VP_SWAP__ */
