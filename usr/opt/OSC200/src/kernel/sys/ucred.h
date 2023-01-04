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
 *	@(#)$RCSfile: ucred.h,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/05/21 20:40:54 $
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

 */

#ifndef _SYS_UCRED_H_
#define	_SYS_UCRED_H_

#ifdef	_KERNEL
#include <sys/unix_defs.h>
#endif

/*
 * Credentials.
 *
 * By convention, credentials are never modified, only copied.
 * Thus, it is always possible to examine a credential entry
 * without holding the structure locked.
 *
 * Credentials in the u-area are changed by building a new
 * credentials structure and substituting it for the old one
 * pointed to by the proc structure's p_rcred field.
 */
struct ucred {
#if	MACH_ASSERT
	long	cr_dummy;		/* the zone free element ptr. */
#endif
	u_short	cr_ref;			/* reference count */
	short	cr_ngroups;		/* number of groups */
	uid_t	cr_uid;			/* effective user id */
	gid_t	cr_gid;			/* effective group id */
	gid_t	cr_groups[NGROUPS];	/* groups */
	uid_t   cr_ruid;		/* Copy of real user id from p_ruid */
	long	cr_pag;			/* AFS & DCE process authentication group */
	long	cr_sia_proc_cred_val;	/* DASS proc-wide auth value */
/*
 * The next four fields were added to support SVR4 style credentials.
 */
	uid_t	_cr_ruid;		/* real user id*/
	gid_t	_cr_rgid;		/* real group id*/
	uid_t	_cr_suid;		/* saved user id*/
	gid_t	_cr_sgid;		/* saved group id*/
#ifdef	_KERNEL
	udecl_simple_lock_data(,cr_lock)
#endif
};
#define NOCRED	((struct ucred *)-1)
#define	NOUID	((uid_t) -1)
#define	NOGID	((gid_t) -1)


#ifdef	_KERNEL

#define	CR_LOCK(cr)		usimple_lock(&(cr)->cr_lock)
#define	CR_UNLOCK(cr)		usimple_unlock(&(cr)->cr_lock)
#define	CR_LOCK_INIT(cr)	usimple_lock_init(&(cr)->cr_lock)

/*
 * Inline references for non-debug kernels.
 * Could inline crfree, too.
 */
#if	MACH_ASSERT
#define crhold(cr) cr_ref(cr)
#else
#define	crhold(cr)							\
MACRO_BEGIN								\
	CR_LOCK(cr);							\
	(cr)->cr_ref++;							\
	CR_UNLOCK(cr);							\
MACRO_END
#endif	/* MACH_ASSERT */

struct ucred *crget();
struct ucred *crcopy();
struct ucred *crdup();

#endif

#endif /* _SYS_UCRED_H_ */
