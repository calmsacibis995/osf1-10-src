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
 *	@(#)$RCSfile: s5mount.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:52:09 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * 20-aug-1991: vipul patel
 *	OSF/1 Release 1.0.1. s5fsmount size change.
 *	with fields for locks.
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * All rights reserved.
 *

 */

#ifndef _S5_MOUNT_H_
#define	_S5_MOUNT_H_

/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */

struct	s5fsmount {
	struct	mount *um_mountp;	/* vfs structure for this fs */
	dev_t	um_dev;			/* device mounted */
	struct	vnode *um_devvp;	/* vnode for block device mounted */
	struct	filsys *um_fs;		/* pointer to superblock */
	lock_data_t um_fsflock;		/* lock over superblock free list */
	lock_data_t um_fsilock;		/* lock over superblock inode list */
	char	um_mntname[MNAMELEN];	/* mounted filesystem */
};
#ifdef KERNEL
/*
 * Convert mount ptr to s5fsmount ptr.
 */
#define VFSTOS5FS(mp)	((struct s5fsmount *)((mp)->m_data))

/*
 * mount table
 */
extern struct s5fsmount	s5fs_mounttab[NMOUNT];

#endif	/* KERNEL */

#endif /* _S5_MOUNT_H_ */








