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
 * @(#)$RCSfile: ffmmount.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/12 19:28:20 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */
#ifndef _FFM_FFMMOUNT_
#define	_FFM_FFMMOUNT_

#include <sys/secdefines.h>
#ifdef	_KERNEL
#include <kern/lock.h>
#endif

/*
 * Mount structure.
 * One allocated on every mount.
 */
struct	ffmmount {
	struct	mount *fm_mountp;	/* vfs structure for this filesystem */
	struct	vnode *fm_vp;		/* vnode for file mounted */
	int	fm_flags;		/* flags */
};

#ifdef	_KERNEL
#define FFM_RDONLY 0x00000004		/* mount is read-only */
/*
 * Convert mount ptr to ffmmount ptr.
 */
#define VFSTOFFM(mp)	((struct ffmmount *)((mp)->m_data))

struct ffm_node {       /* a file-on-file vnode */
  struct vnode *fn_vnode;
  struct vnode *fn_shadow;
};

#define VTOF(vp)	((struct ffm_node *)(vp)->v_data)

#if SEC_FSCHANGE
extern struct vnsecops ffm_secops;
#endif /* SEC_FSCHANGE */

#endif /* _KERNEL */

#endif /* _FFM_FFMMOUNT_ */
