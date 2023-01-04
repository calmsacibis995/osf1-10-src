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
 *	@(#)$RCSfile: cdfsmount.h,v $ $Revision: 4.3.9.2 $ (DEC) $Date: 1993/07/16 13:05:54 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
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

 */

#ifndef _CDFSMOUNT_
#define	_CDFSMOUNT_

/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the primary volume descriptor.
 */
struct	cdfsmount {
	int	um_flag;		/* Flags for perm/version... */
	struct	mount *um_mountp;	/* vfs structure for this filesystem */
	dev_t	um_dev;			/* device mounted */
	struct	vnode *um_devvp;	/* vnode for block device mounted */
	struct	fs *um_fs;		/* pointer to pvd */
#ifdef	_KERNEL
	udecl_simple_lock_data(,um_lock)/* multiprocessor exclusion */
#endif
};

/*
 * low 5 bits are taken by pan-filesystem flags: RDONLY, SYNCHRONOUS,
 * NOEXEC, NOSUID, NODEV.
 */

#define M_NODEFPERM	0x20		/* No default permissions */
#define M_PRIMARY	0x80		/* Use Primary Vol Desc */
#define	M_EXTENDED_ARGS	0x80000		/* Use extended cdfs_args */
    /* M_NOVERSION is backward compat for old mount commands */
#define M_NOVERSION	0x00040		/* No version,lowercase,drop "." ext */
#define	M_LOWERCASE	0x00100		/* convert to lowercase,drop "." ext */
#define M_DROPVERS	0x00200		/* No version numbers */
#define	M_RRIP		0x00400		/* use RRIP */
#define RELAX_NAMES(flags) (((flags)&M_RRIP) == 0 && \
			    ((flags)&(M_DROPVERS|M_LOWERCASE)) == (M_DROPVERS|M_LOWERCASE))

#ifdef	_KERNEL

#define	CDFSMOUNT_LOCK(nmp)		usimple_lock(&(nmp)->um_lock)
#define	CDFSMOUNT_UNLOCK(nmp)		usimple_unlock(&(nmp)->umlock)
#define	CDFSMOUNT_LOCK_TRY(nmp)		usimple_lock_try(&(nmp)->umlock)
#define	CDFSMOUNT_LOCK_INIT(nmp)	usimple_lock_init(&(nmp)->umlock)
#define	CDFSMOUNT_LOCK_HOLDER(nmp)	SLOCK_HOLDER(&(nmp)->umlock)

/*
 * Convert mount ptr to cdfsmount ptr.
 */

#define VFSTOCDFS(mp)	((struct cdfsmount *)((mp)->m_data))

#endif
#endif /* _CDFS_CDFSMOUNT_ */
