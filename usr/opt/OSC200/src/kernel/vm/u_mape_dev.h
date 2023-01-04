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
 * @(#)$RCSfile: u_mape_dev.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 18:57:49 $
 */
#ifndef	__U_MAPE_DEV__
#define	__U_MAPE_DEV__ 1


/*
 * mmap object for character device.
 */

struct u_dev_object {
	struct vm_object ud_object;		/* Object common part */
	int ud_flags;				/* Flags */
	struct vnode *ud_vp;			/* Vnode backing this dev */
	dev_t ud_dev;				/* Device we're mapped to */
	int (*ud_mapfunc)();			/* Device map function */
	vm_offset_t ud_offset;			/* Offset passed to map */
};

/*
 * What's of value in the object structure
 */

#define ud_size	 ud_object.ob_size
#define	ud_refcnt ud_object.ob_ref_count

#define	UDF_NOWIRING	0x1			/* Wiring not supported */

#endif	/* !__U_MAPE_DEV__ */
